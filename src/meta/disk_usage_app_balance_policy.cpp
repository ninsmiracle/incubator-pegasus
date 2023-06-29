// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#include "disk_usage_app_balance_policy.h"
#include "greedy_load_balancer.h"
#include "utils/string_conv.h"

#include <utils/command_manager.h>
#include <utils/fmt_logging.h>
#include <utils/fail_point.h>

namespace dsn {
namespace replication {

DSN_DECLARE_bool(balancer_in_turn);
DSN_DECLARE_bool(only_primary_balancer);
DSN_DECLARE_bool(only_move_primary);

DSN_DECLARE_uint64(min_live_node_count_for_unfreeze);

DSN_DEFINE_uint64(meta_server,
                  balance_threshold,
                   20,
                   "control the disk usage balance threshold to continue or finish balance policy,<number MB>");


//bool calculate_disk_usage(node_mapper &nodes,
//                    const app_mapper &apps,
//                     replica_disk_usage_mapper &replicas,
//                    app_id id,
//                    bool only_primary,
//                    /*out*/ std::map<rpc_address,int> &disk_usage)
//{
//    disk_usage.clear();
//    for (auto iter = nodes.begin(); iter != nodes.end(); ++iter) {
//        const rpc_address node = iter->first;
//        const node_state *ns = get_node_state(nodes, node, false);
//        CHECK(ns != nullptr, "can't find node(%s) from node_state", node.to_string());
//
//        auto add_one_replica_to_disk_usage = [&](const gpid &pid) {
//            LOG_INFO("add gpid(%d.%d) to node(%s) disk usage",
//                     pid.get_app_id(),
//                     pid.get_partition_index(),
//                     node.to_string());
//            const config_context &cc = *get_config_context(apps, pid);
//
//            auto iter = cc.find_from_serving(node);
//            if (iter == cc.serving.end()) {
//                LOG_WARNING("can't collect gpid(%d.%d)'s info from %s, which should be primary",
//                            pid.get_app_id(),
//                            pid.get_partition_index(),
//                            node.to_string());
//                return false;
//            } else {
//                // node_addr -> disk_usage
//                disk_usage[iter->node] += replicas[iter->node][pid];
//                LOG_DEBUG("calculate disk usage:now iter is {},node is {}",iter->node,node);
//                return true;
//            }
//        };
//
//        if (only_primary) {
//            bool result = ns->for_each_primary(id, add_one_replica_to_disk_usage);
//            return result;
//        } else {
//            bool result = ns->for_each_partition(id, add_one_replica_to_disk_usage);
//            return result;
//        }
//    }
//}

//const meta_view *global_view
int get_min_replica_disk_usage(const meta_view *global_view){
    int min_replica_usage = INT32_MAX;
    for(auto node_pair : *global_view->replicas){
        for(auto gpid_pair : node_pair.second){
            if(gpid_pair.second < min_replica_usage){
                min_replica_usage = gpid_pair.second;
                LOG_INFO("now min_replica_usage is {},current usage is {}",min_replica_usage, gpid_pair.second);
            }
        }
    }

    return min_replica_usage;
}



std::string disk_usage_app_balance_policy::ctrl_assgin_balance_threshold(const std::vector<std::string> &args){
    std::string result("OK");
    if (args.empty()) {
        result = std::to_string(_balance_threshold);
    } else {
        if (args[0] == "DEFAULT") {
            //get default value in config.ini,now default value is 20MB
            _balance_threshold = FLAGS_balance_threshold;
        } else {
            int32_t v = 0;
            if (!dsn::buf2int32(args[0], v) || v <= 0) {
                result = std::string("ERR: invalid arguments");
            } else {
                _balance_threshold = v;
            }
        }
    }

    return result;
}

disk_usage_app_balance_policy::disk_usage_app_balance_policy(meta_service *svc) : load_balance_policy(svc)
{
    if (_svc != nullptr) {
        _balancer_in_turn = FLAGS_balancer_in_turn;
        _only_primary_balancer = FLAGS_only_primary_balancer;
        _only_move_primary = FLAGS_only_move_primary;
    } else {
        _balancer_in_turn = false;
        _only_primary_balancer = false;
        _only_move_primary = false;
    }

    //register disk balance threshold : meta.lb.disk_balance_threshold
    _cmds.emplace_back(dsn::command_manager::instance().register_command(
        {"meta.lb.disk_balance_threshold"},
        "meta.lb.disk_balance_threshold <number MB>)",
        "control the disk usage balance threshold to continue or finish balance",
        [this](const std::vector<std::string> &args) {
            return ctrl_assgin_balance_threshold(args);
        }));
}

bool disk_usage_app_balance_policy::init(const dsn::replication::meta_view *global_view, dsn::replication::migration_list *list,bool checker)
{
    _global_view = global_view;
    _migration_result = list;
    const node_mapper &nodes = *_global_view->nodes;
    _alive_nodes = nodes.size();
    number_nodes(nodes);

    //阈值赋值
    _min_replica_disk_usage = get_min_replica_disk_usage(_global_view);
    if(_min_replica_disk_usage > _balance_threshold){
        if(checker){
            LOG_INFO("Some error happen in checking stage,disk load balance check will not run");
        }else{
            LOG_INFO("Some error happen in balance stage,disk load balance can not read disk infomation");
        }
        LOG_WARNING("Balance threshold {} is not larger than min replica disk usage {},need to adjust it.",_balance_threshold,_min_replica_disk_usage);
        return false;
    }

    return true;
}

void disk_usage_app_balance_policy::balance(bool checker, const meta_view *global_view, migration_list *list)
{
    //when min_replica_disk_usage > balance_threshold will not do balance
    if(!init(global_view, list,checker)){
        return;
    }
    const app_mapper &apps = *_global_view->apps;
    if (!execute_balance(apps,
                         checker,
                         _balancer_in_turn,
                         _only_move_primary,
                         std::bind(&disk_usage_app_balance_policy::primary_balance,
                                   this,
                                   std::placeholders::_1,
                                   std::placeholders::_2))) {
        return;
    }
    LOG_INFO("gns,begin to copy secondary");

    if (!need_balance_secondaries(checker)) {
        return;
    }

    execute_balance(apps,
                    checker,
                    _balancer_in_turn,
                    _only_move_primary,
                    std::bind(&disk_usage_app_balance_policy::copy_secondary,
                              this,
                              std::placeholders::_1,
                              std::placeholders::_2));
}

bool disk_usage_app_balance_policy::still_have_replicas_lower_than_average( const std::shared_ptr<app_state> &app,node_mapper &nodes,replica_disk_usage_mapper &replicas){
    //todo:考虑磁盘平衡阈值  控制 can_continue 因为磁盘负载均衡没有走最大流图了，所以需要有这个方法
    //LOG_INFO("gns,replicas size is {}",replicas.size());
    int total_primary_disk_usage_of_this_app = 0;
    std::vector<int> disk_usage_by_nodes;
    for(auto gpid_map_it : replicas){
        rpc_address addr = gpid_map_it.first;
        int nodes_sum = 0;

        LOG_INFO("gns, nodes size is {},addr is {}.appid {}",nodes.size(),addr,app->app_id);
        LOG_INFO("gns, nodes-addr primary count is {}.appid {}",nodes[addr].primary_count(),app->app_id);

        if(nodes.find(addr) == nodes.end()){
            LOG_ERROR("gns, can not find {} in nodes",addr);
        }
        //get all primary
        partition_set * primary_set = nodes[addr].partitions(app->app_id,true);
        if(primary_set == nullptr){
            LOG_INFO("There are no primary replica of on nodes {}.appid {}",addr,app->app_id);
            continue;
        }else{
            LOG_INFO("gns,in still_have_replicas_lower_than_average, primary_set size is {}",primary_set->size());
        }

        for(auto iter : gpid_map_it.second){
            //current gpid in primary_set
            if (primary_set->count(iter.first)){
                LOG_INFO("gns,in still_have_replicas_lower_than_average, gpidis {}.{},USAGE is {}",iter.first.get_app_id(),iter.first.get_partition_index(),iter.second);
                total_primary_disk_usage_of_this_app += iter.second;
                LOG_INFO("gns,now total_primary_disk_usage_of_this_app {}",total_primary_disk_usage_of_this_app);
            }
            nodes_sum += iter.second;
        }

        //LOG_INFO("gns,push back {} into vector",nodes_sum);
        disk_usage_by_nodes.push_back(nodes_sum);
    }

    int expect_replicas_disk_usage_low = total_primary_disk_usage_of_this_app / nodes.size();
    LOG_INFO("gns, expect_replicas_disk_usage_low is {}",expect_replicas_disk_usage_low);

    //int lower_count = 0;
    for (int i = 0; i < disk_usage_by_nodes.size(); i++) {
        if(disk_usage_by_nodes[i] < expect_replicas_disk_usage_low){
            return true;
        }
    }

    return false;
}

bool disk_usage_app_balance_policy::primary_balance(const std::shared_ptr<app_state> &app,
                                          bool only_move_primary)
{
    ///切角色对磁盘负载似乎没有作用
    //move primary is useless for disk usage
    LOG_INFO("begin copy primary to make disk usage balance.appid {}",app->app_id);
    if (!only_move_primary) {
        bool still_lower = still_have_replicas_lower_than_average(app,*(_global_view->nodes),*(_global_view->replicas));
        LOG_INFO("gns:disk_usage_app_balance_policy::copy_primary outside.appid {}",app->app_id);
        ///原始逻辑中，第二个参数graph->have_less_than_average() 决定了 have_less_than_average ，间接决定了 can_continue,实际就是查看是否还有节点小于期望值
        bool copy_result = disk_usage_app_balance_policy::copy_primary(app,still_lower);
        LOG_INFO("copy_result is ok.appid {}",app->app_id);
        LOG_INFO("copy_result*******,appid {}",app->app_id);
        return copy_result;
    } else {
            LOG_INFO("stop to copy primary for app({}) coz it is disabled", app->get_logname());
            return true;
    }
}

///标记位检查，没改 后续可提到基类中
bool disk_usage_app_balance_policy::need_balance_secondaries(bool balance_checker)
{
    if (!balance_checker && !_migration_result->empty()) {
        LOG_INFO("stop to do secondary balance coz we already have actions to do");
        return false;
    }
    if (_only_primary_balancer) {
        LOG_INFO("stop to do secondary balancer coz it is not allowed");
        return false;
    }
    return true;
}

bool disk_usage_app_balance_policy::copy_secondary(const std::shared_ptr<app_state> &app, bool place_holder)
{
    LOG_INFO("gns,begin to copy secondary");
    node_mapper &nodes = *(_global_view->nodes);
    const app_mapper &apps = *(_global_view->apps);
    replica_disk_usage_mapper &replicas = *(_global_view->replicas);
    disk_total_usage_mapper &disks =  *(_global_view->disks);

    //get all replica disk usage of current app
    int total_disk_usage_of_this_app = 0;
    for(auto gpid_map_it : replicas){
        for(auto iter : gpid_map_it.second){
            if (iter.first.get_app_id() == app->app_id){
                total_disk_usage_of_this_app += iter.second;
            }
        }
    }

    int replicas_low = total_disk_usage_of_this_app / _alive_nodes;
    LOG_INFO("gns,copy_secondary.replicas_low {} , total_disk_usage_of_this_app {}, _alive_nodes {}",replicas_low,total_disk_usage_of_this_app,_alive_nodes);


    std::unique_ptr<copy_operation_by_disk> operation = std::make_unique<copy_secondary_operation_by_disk>(
        app, apps, nodes, replicas,disks,address_vec, address_id, replicas_low,_balance_threshold);
    return operation->start(_migration_result);
}

bool disk_usage_app_balance_policy::copy_primary(const std::shared_ptr<app_state> &app,bool still_have_less_than_average){
    LOG_INFO("gns: begin copy_primary,appid {}",app->app_id);
    node_mapper &nodes = *(_global_view->nodes);
    const app_mapper &apps = *(_global_view->apps);
    replica_disk_usage_mapper &replicas = *(_global_view->replicas);
    disk_total_usage_mapper &disks =  *(_global_view->disks);
    LOG_INFO("gns: put meta_view success.appid {}",app->app_id);

    //get Primary disk usage
    int total_primary_disk_usage_of_this_app = 0;
    for(auto gpid_map_it : replicas){
        rpc_address addr = gpid_map_it.first;
        LOG_INFO("gns: addr is {}",addr);
        if(!nodes.count(addr)){
            LOG_INFO("gns,copy_primary can not find {} in nodes.appid {}",addr,app->app_id);
        }
        //get all primary
        partition_set * primary_set = nodes[addr].partitions(app->app_id,true);
        if(primary_set == nullptr){
            LOG_INFO("There are no primary replica of app_id {} on nodes {}",app->app_id,addr);
            continue;
        }else{
            LOG_INFO("gns,in copy_primary primary_set size is {},appid {}",primary_set->size(),app->app_id);
        }

        for(auto iter : gpid_map_it.second){
            //current gpid in primary_set
            if (primary_set->count(iter.first)){
                total_primary_disk_usage_of_this_app += iter.second;
            }
        }
    }

    LOG_INFO("copy_primary: total_primary_disk_usage_of_this_app is {},alive_nodes is {}.appid {}",total_primary_disk_usage_of_this_app,_alive_nodes,app->app_id);
    int primary_disk_replicas_low = total_primary_disk_usage_of_this_app / _alive_nodes;
    LOG_INFO("copy_primary: primary_disk_replicas_low is {}. appid {}",primary_disk_replicas_low,app->app_id);


    std::unique_ptr<copy_operation_by_disk> operation = std::make_unique<copy_primary_operation_by_disk>(
        app, apps, nodes,replicas,disks, address_vec, address_id, still_have_less_than_average, primary_disk_replicas_low,_balance_threshold);
    LOG_INFO("gns, copy_primary start.appid {}",app->app_id);
    bool start_result = operation->start(_migration_result);
    if(start_result){
        LOG_INFO("gns, copy_primary start is true,appid {}",app->app_id);
    }else{
        LOG_INFO("gns, copy_primary start is false,appid {}",app->app_id);
    }


    return start_result;
}


copy_operation_by_disk::copy_operation_by_disk(const std::shared_ptr<app_state> app,
                                                                     const app_mapper &apps,
                                                                     node_mapper &nodes,
                                                                     const replica_disk_usage_mapper &replicas,
                                                                     const disk_total_usage_mapper &disks,
                                                                     const std::vector<dsn::rpc_address> &address_vec,
                                                                     const std::unordered_map<dsn::rpc_address, int> &address_id,
                                                                     int replicas_low)
    : copy_replica_operation(app,apps,nodes,address_vec,address_id),_replicas(replicas),_disks(disks)
{
}

bool copy_operation_by_disk::start(migration_list *result)
{
    LOG_INFO("gns,begin to start.appid {}",_app->app_id);
    //calculate_disk_usage(_nodes,_apps,_replicas,_app->app_id,only_copy_primary(),/*out*/ _disk_usage);
    init_ordered_address_by_disk();
    //todo: 可能需要处理异常情况

    while (true) {
        LOG_INFO("gns,begin to can_continue.appid {}",_app->app_id);
        if (!can_continue()) {
            break;
        }
        //select_partition父类子类对replica_low的尺度不一致
        LOG_INFO("gns,begin to select_partition.appid {}",_app->app_id);
        gpid selected_pid = select_partition(result);
        if (selected_pid.get_app_id() != -1) {
            LOG_INFO("gns,begin to copy_once.appid {}",_app->app_id);
            copy_once(selected_pid, result);
            LOG_INFO("gns,begin to update_ordered_address_by_disk.appid {}",_app->app_id);
            update_ordered_address_by_disk(selected_pid);
        } else {
            LOG_INFO("gns,selected gpid < 0.appid {}",_app->app_id);
            _ordered_address_by_disk.erase(--_ordered_address_by_disk.end());
        }
    }

    LOG_INFO("gns,start return true.appid {}",_app->app_id);
    return true;
}


void copy_operation_by_disk::init_ordered_address_by_disk()
{
    LOG_INFO("gns,int oreder address vec begin");
    //init _disk_usage
    LOG_INFO("gns,_address_vec is {}",_address_vec.size());
    _disk_usage.resize(_address_vec.size(), 0);

    LOG_INFO("gns,init nodes size {}",_nodes.size());
    for (const auto &iter : _nodes) {
        //LOG_INFO("gns,init _address_id size {}",_address_id.size());
        auto id = _address_id.at(iter.first);

        _disk_usage[id] = get_node_disk_usage(iter.second);
        LOG_INFO("gns,now order id is {} disk_usage is {}",id,_disk_usage[id]);
    }
    LOG_INFO("gns,get_node_disk_usage is ok");

    std::set<int, std::function<bool(int left, int right)>> ordered_queue(
        [this](int left, int right) {
            return _disk_usage[left] != _disk_usage[right]
                       ? _disk_usage[left] < _disk_usage[right]
                       : left < right;
        });
    for (const auto &iter : _nodes) {
        auto id = _address_id.at(iter.first);
        ordered_queue.insert(id);
    }
    LOG_INFO("gns,ordered_queue is ok");

    _ordered_address_by_disk.swap(ordered_queue);
    LOG_INFO("gns,init_ordered_address_by_disk is ok");
}

void copy_operation_by_disk::copy_once(gpid selected_pid, migration_list *result)
{
    auto from = _address_vec[*_ordered_address_by_disk.rbegin()];
    auto to = _address_vec[*_ordered_address_by_disk.begin()];

    auto pc = _app->partitions[selected_pid.get_partition_index()];
    auto request = generate_balancer_request(_apps, pc, get_balance_type(), from, to);
    result->emplace(selected_pid, request);
}

void copy_operation_by_disk::update_ordered_address_by_disk(dsn::gpid selected_gpid)
{
    int id_min = *_ordered_address_by_disk.begin();
    int id_max = *_ordered_address_by_disk.rbegin();

    auto from = _address_vec[id_max];
    auto to = _address_vec[id_min];

    int gpid_disk = _replicas[from][selected_gpid];
    _disk_usage[id_max] -= gpid_disk;
    _disk_usage[id_min] += gpid_disk;

    //delect and insert,to reorder
    _ordered_address_by_disk.erase(_ordered_address_by_disk.begin());
    _ordered_address_by_disk.erase(--_ordered_address_by_disk.end());
    _ordered_address_by_disk.insert(id_max);
    _ordered_address_by_disk.insert(id_min);


    //update _replicas
    _replicas[from].erase(selected_gpid);
    _replicas[to].insert({selected_gpid,gpid_disk});

    //update_total_disks
    //std::string disk_tag = get_disk_tag(_apps,from,selected_gpid);
    //_disks[from].erase(selected_gpid);

}

//get all partitions from max disk usage load node
const partition_set *copy_operation_by_disk::get_all_partitions(){
    int id_max = *_ordered_address_by_disk.rbegin();
    const node_state &ns = _nodes.find(_address_vec[id_max])->second;
    const partition_set *partitions = ns.partitions(_app->app_id, only_copy_primary());
    return partitions;
}

gpid copy_operation_by_disk::select_partition(migration_list *result)
{
    const partition_set *partitions = get_all_partitions();

    int id_max = *_ordered_address_by_disk.rbegin();
    const node_state &ns = _nodes.find(_address_vec[id_max])->second;
    CHECK(partitions != nullptr && !partitions->empty(),
              "max load({}) shouldn't empty",
              ns.addr().to_string());

    return select_max_load_gpid(partitions, result);
}

std::string copy_operation_by_disk::get_max_load_disk(dsn::rpc_address addr)
{
    //disk_tag -> int32
    int max_disk_load = 0;
    std::string max_disk_tag;
    for(auto iter : _disks[addr]){
        if(iter.second > max_disk_load){
            max_disk_load = iter.second;
            max_disk_tag = iter.first;
        }
    }

    return max_disk_tag;
}

///partitions入参时就已经完成了primary或secondary的分类过滤
dsn::gpid copy_operation_by_disk::select_max_load_gpid(const partition_set *partitions,migration_list *result)
{
    int id_max = *_ordered_address_by_disk.rbegin();
    const node_state &ns = _nodes.find(_address_vec[id_max])->second;
    ///如果考虑真实磁盘用量，最大真实用量的磁盘上可能没有当前表的分片，故这里不考虑真实用量
    //std::string max_disk_tag = get_max_load_disk(ns.addr());

    int cur_replicas_low = get_replicas_low();
    int expectation_residual = _disk_usage[id_max] - cur_replicas_low;
    LOG_INFO("gns.in select_max_load_gpid.expectation_residual are {}",expectation_residual);
    int max_replica_disk_usage = -1;
    gpid selected_pid(-1, -1);
        //find a largest partition in max load node
        //if only_primary is ture, partition_set only contain primary replica
    for (const gpid &pid : *partitions) {
        if (!can_select(pid, result)) {
            continue;
        }

        int cur_replica_disk_usage = _replicas[ns.addr()][pid];
        if(cur_replica_disk_usage > max_replica_disk_usage && cur_replica_disk_usage <= expectation_residual){
            max_replica_disk_usage = cur_replica_disk_usage;
            selected_pid = pid;
        }
    }
    LOG_INFO("max node disk usage is {},select_max_load_gpid.replicas_low {}",_disk_usage[id_max],cur_replicas_low);

    return selected_pid;
}

dsn::gpid copy_operation_by_disk::get_max_replica_disk_usage_smaller_than_expectation_residual(dsn::rpc_address addr,int expectation_residual,bool only_primary)
{
    int max_smaller_than_expectation_residual = -1;
    dsn::gpid res_gpid(-1,-1);
    //get all primary or all replica
    partition_set * primary_set = _nodes[addr].partitions(_app->app_id,only_primary);
    if(primary_set == nullptr){
        LOG_INFO("can not find replica of appid {} in node {}",_app->app_id,addr);
        return res_gpid;
    }

    for(auto iter : _replicas[addr]){
        //current gpid in primary_set
        if (primary_set->count(iter.first)){
            if(iter.second <= expectation_residual && iter.second > max_smaller_than_expectation_residual ){
                max_smaller_than_expectation_residual = iter.second;
                res_gpid = iter.first;
            }
        }
    }

    return res_gpid;
}


copy_primary_operation_by_disk::copy_primary_operation_by_disk(const std::shared_ptr<app_state> app,
                                                               const app_mapper &apps,
                                                               node_mapper &nodes,
                                                               const replica_disk_usage_mapper &replicas,
                                                               const disk_total_usage_mapper &disks,
                                                               const std::vector<dsn::rpc_address> &address_vec,
                                                               const std::unordered_map<dsn::rpc_address, int> &address_id,
                                                               bool have_lower_than_average,
                                                               int replicas_low,
                                                               int balance_threshold)
    : copy_operation_by_disk(app, apps, nodes, replicas,disks,address_vec, address_id,replicas_low)
{
    _have_lower_than_average = have_lower_than_average;
    _replicas_low = replicas_low;
    LOG_INFO("gns: copy_primary_operation_by_disk replicas_low is {} ",_replicas_low);
    _disk_usage_balance_threshold = balance_threshold;
}


bool copy_primary_operation_by_disk::can_select(gpid pid, migration_list *result)
{
    //target node already have a member of this gpid
    int id_min = *_ordered_address_by_disk.begin();
    const node_state &min_ns = _nodes.at(_address_vec[id_min]);
    if (min_ns.served_as(pid) != partition_status::PS_INACTIVE) {
        LOG_INFO("{}: skip gpid({}.{}) coz it is already a member on the target node",
                  _app->get_logname(),
                  pid.get_app_id(),
                  pid.get_partition_index());
        return false;
    }


    LOG_INFO("{}: copy primary. gpid({}.{}) has be choose",
             _app->get_logname(),
             pid.get_app_id(),
             pid.get_partition_index());
    return pid.get_app_id() == _app->app_id && result->find(pid) == result->end();
}


bool copy_primary_operation_by_disk::can_continue()
{
    int id_min = *_ordered_address_by_disk.begin();
    //_replicas_low is Mathematical Expectation
    if (_have_lower_than_average && _disk_usage[id_min] >= _replicas_low) {
       LOG_INFO("{}: stop the copy due to primaries on all nodes will reach low later.can_continue replicas_low is {}",
                _app->get_logname(),_replicas_low);
       return false;
    }

    int id_max = *_ordered_address_by_disk.rbegin();

    LOG_INFO("max {}mb,min {}mb",_disk_usage[id_max],_disk_usage[id_min]);

    //when skew of every replica is smaller than threshold,do not do balance continue
    if (_disk_usage[id_max] - _disk_usage[id_min] <= _disk_usage_balance_threshold) {
       LOG_INFO("{}: stop the copy due to the primary skew between every replica is smaller than balance threshold.",
                _app->get_logname());
       return false;
    }

    int expectation_residual = _disk_usage[id_max] - _replicas_low;
    LOG_INFO("gns.primary can_continue,next step will copy at most {}MB in expectation,max disk usage node is {},replicas_low is {}",expectation_residual,_disk_usage[id_max],_replicas_low);

    gpid res_gpid = get_max_replica_disk_usage_smaller_than_expectation_residual(_address_vec[id_max],expectation_residual, true);
    //can not choose any replica in max disk load node in next step
    if(res_gpid.get_partition_index() < 0){
       LOG_INFO("In disk usage app balance policy,can not find a gpid which is smaller than expectation residual.");
       return false;
    }
    int max_disk_usage_smaller_than_expectation_residual = _replicas[_address_vec[id_max]][res_gpid];
    LOG_INFO("In disk usage app balance policy,max_disk_usage_smaller_than_expectation_residual is {}",max_disk_usage_smaller_than_expectation_residual);

    return true;
}

int copy_primary_operation_by_disk::get_node_disk_usage(const dsn::replication::node_state &ns) const
{
    LOG_INFO("gns,get_node_disk_usage begin.appid {}",_app->app_id);
    int app_relica_disk_usage = 0;

    if(_app == nullptr){
       LOG_ERROR("Something is wrong.Input app is null");
    }

    const partition_set * primary_set = ns.partitions(_app->app_id,true);
    if(nullptr == primary_set || primary_set->empty()){
       LOG_INFO("gns,get_node_disk_usage get primary set is null or empty, addr is {}.appid {}",ns.addr(),_app->app_id);
       return 0;
    }

    for(auto iter : _replicas.at(ns.addr())){
       //LOG_INFO("gns,get_node_disk_usage replicas has addr {}.",ns.addr());
       if(primary_set->count(iter.first)){
            app_relica_disk_usage += iter.second;
       }
    }
    return app_relica_disk_usage;
}



balance_type copy_primary_operation_by_disk::get_balance_type() { return balance_type::COPY_PRIMARY; }


copy_secondary_operation_by_disk::copy_secondary_operation_by_disk(  const std::shared_ptr<app_state> app,
                                                                   const app_mapper &apps,
                                                                   node_mapper &nodes,
                                                                   const replica_disk_usage_mapper &replicas,
                                                                   const disk_total_usage_mapper &disks,
                                                                   const std::vector<dsn::rpc_address> &address_vec,
                                                                   const std::unordered_map<dsn::rpc_address, int> &address_id,
                                                                   int replicas_low,
                                                                   int balance_threshold)
    : copy_operation_by_disk(app, apps, nodes,replicas,disks, address_vec, address_id,replicas_low), _replicas_low(replicas_low),_disk_usage_balance_threshold(balance_threshold)
{
};

bool copy_secondary_operation_by_disk::can_continue()
{
    int id_min = *_ordered_address_by_disk.begin();
    int id_max = *_ordered_address_by_disk.rbegin();
    if (_disk_usage[id_max] <= _replicas_low ||
        _disk_usage[id_max] - _disk_usage[id_min] <= _disk_usage_balance_threshold) {
       LOG_INFO("{}: stop copy secondary coz it will be balanced by disk usage later", _app->get_logname());
       return false;
    }

    int expectation_residual = _disk_usage[id_max] - _replicas_low;
    gpid res_gpid = get_max_replica_disk_usage_smaller_than_expectation_residual(_address_vec[id_max],expectation_residual, false);
    //can not choose any replica in max disk load node in next step
    if(res_gpid.get_partition_index() < 0){
       return false;
    }
    int max_disk_usage_smaller_than_expectation_residual = _replicas[_address_vec[id_max]][res_gpid];
    LOG_INFO("In disk usage app balance policy,max_disk_usage_smaller_than_expectation_residual is {}",max_disk_usage_smaller_than_expectation_residual);

    return true;
}

bool copy_secondary_operation_by_disk::can_select(dsn::gpid pid, dsn::replication::migration_list *result)
{
    if(pid.get_app_id() != _app->app_id){
       return false;
    }
    int id_max = *_ordered_address_by_disk.rbegin();
    const node_state &max_ns = _nodes.at(_address_vec[id_max]);
    if (max_ns.served_as(pid) == partition_status::PS_PRIMARY) {
       LOG_INFO("{}: do disk usage balance, skip gpid({}.{}) coz it is primary",
                 _app->get_logname(),
                 pid.get_app_id(),
                 pid.get_partition_index());
       return false;
    }

    // if the pid has been used
    if (result->find(pid) != result->end()) {
       LOG_INFO("{}: do disk usage balance, skip gpid({}.{}) coz it is already copyed",
                 _app->get_logname(),
                 pid.get_app_id(),
                 pid.get_partition_index());
       return false;
    }

    int id_min = *_ordered_address_by_disk.begin();
    const node_state &min_ns = _nodes.at(_address_vec[id_min]);
    if (min_ns.served_as(pid) != partition_status::PS_INACTIVE) {
       LOG_INFO("{}: skip gpid({}.{}) coz it is already a member on the target node",
                 _app->get_logname(),
                 pid.get_app_id(),
                 pid.get_partition_index());
       return false;
    }


    LOG_INFO("{}: copy secondary. gpid({}.{}) has be choose",
             _app->get_logname(),
             pid.get_app_id(),
             pid.get_partition_index());
    return true;
}

int copy_secondary_operation_by_disk::get_node_disk_usage(const dsn::replication::node_state &ns) const
{
    int app_relica_disk_usage = 0;
    for(auto iter : _replicas.at(ns.addr())){
       if(iter.first.get_app_id() == _app->app_id){
            app_relica_disk_usage += iter.second;
       }
    }
    return app_relica_disk_usage;
}

balance_type copy_secondary_operation_by_disk::get_balance_type() { return balance_type::COPY_SECONDARY; }



}
}
