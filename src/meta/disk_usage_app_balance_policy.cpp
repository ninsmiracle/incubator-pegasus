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

#include <dsn/tool-api/command_manager.h>
#include <dsn/dist/fmt_logging.h>
#include <dsn/utility/fail_point.h>

namespace dsn {
namespace replication {
DSN_DEFINE_bool(meta_server, balancer_in_turn, false, "balance the apps one-by-one/concurrently");
DSN_DEFINE_bool(meta_server, only_primary_balancer, false, "only try to make the primary balanced");
DSN_DEFINE_bool(meta_server,
                only_move_primary,
                false,
                "only try to make the primary balanced by move");

DSN_DECLARE_uint64(min_live_node_count_for_unfreeze);
DSN_DECLARE_uint64(balance_threshold);

const std::string &get_disk_tag(const app_mapper &apps, const rpc_address &node, const gpid &pid)
{
    const config_context &cc = *get_config_context(apps, pid);
    auto iter = cc.find_from_serving(node);
    CHECK(iter != cc.serving.end(),
            "can't find disk tag of gpid(%d.%d) for %s",
            pid.get_app_id(),
            pid.get_partition_index(),
            node.to_string());
    return iter->disk_tag;
}


bool calc_disk_usage(node_mapper &nodes,
                    const app_mapper &apps,
                     replica_disk_usage_mapper &replicas,
                    app_id id,
                    const rpc_address &node,
                    bool only_primary,
                    /*out*/ std::map<rpc_address,int> &disk_usage)
{
    disk_usage.clear();
    const node_state *ns = get_node_state(nodes, node, false);
    CHECK(ns != nullptr, "can't find node(%s) from node_state", node.to_string());

    auto add_one_replica_to_disk_usage = [&](const gpid &pid) {
        LOG_INFO("add gpid(%d.%d) to node(%s) disk usage",
              pid.get_app_id(),
              pid.get_partition_index(),
              node.to_string());
        const config_context &cc = *get_config_context(apps, pid);

        auto iter = cc.find_from_serving(node);
        if (iter == cc.serving.end()) {
            LOG_WARNING("can't collect gpid(%d.%d)'s info from %s, which should be primary",
                  pid.get_app_id(),
                  pid.get_partition_index(),
                  node.to_string());
            return false;
        } else {
            //node_addr -> disk_usage
            disk_usage[iter->node] += replicas[iter->node][pid];
            return true;
        }
    };

    if (only_primary) {
        bool result = ns->for_each_primary(id, add_one_replica_to_disk_usage);
        return result;
    } else {
        bool result = ns->for_each_partition(id, add_one_replica_to_disk_usage);
        return result;
    }
}

//const meta_view *global_view
int get_min_replica_disk_usage(const meta_view *global_view){
    int min_replica_usage = INT32_MAX;
    for(auto node_pair : *global_view->replicas){
        for(auto gpid_pair : node_pair.second){
            if(gpid_pair.second < min_replica_usage){
                min_replica_usage = gpid_pair.second;
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
    _cmds.emplace_back(dsn::command_manager::instance().register_command(
        {"meta.lb.balancer_in_turn"},
        "meta.lb.balancer_in_turn <true|false>",
        "control whether do app balancer in turn",
        [this](const std::vector<std::string> &args) {
            return remote_command_set_bool_flag(_balancer_in_turn, "lb.balancer_in_turn", args);
        }));

    _cmds.emplace_back(dsn::command_manager::instance().register_command(
        {"meta.lb.only_primary_balancer"},
        "meta.lb.only_primary_balancer <true|false>",
        "control whether do only primary balancer",
        [this](const std::vector<std::string> &args) {
            return remote_command_set_bool_flag(
                _only_primary_balancer, "lb.only_primary_balancer", args);
        }));

    _cmds.emplace_back(dsn::command_manager::instance().register_command(
        {"meta.lb.only_move_primary"},
        "meta.lb.only_move_primary <true|false>",
        "control whether only move primary in balancer",
        [this](const std::vector<std::string> &args) {
            return remote_command_set_bool_flag(_only_move_primary, "lb.only_move_primary", args);
        }));
    //register disk balance threshold : meta.lb.disk_balance_threshold
    _cmds.emplace_back(dsn::command_manager::instance().register_command(
        {"meta.lb.disk_balance_threshold"},
        "meta.lb.disk_balance_threshold <number MB>)",
        "control the disk usage balance threshold to continue or finish balance",
        [this](const std::vector<std::string> &args) {
            return ctrl_assgin_balance_threshold(args);
        }));
}

bool disk_usage_app_balance_policy::init(const dsn::replication::meta_view *global_view, dsn::replication::migration_list *list)
{
    _global_view = global_view;
    _migration_result = list;
    const node_mapper &nodes = *_global_view->nodes;
    _alive_nodes = nodes.size();
    //number_nodes(nodes);

    //阈值赋值
    _min_replica_disk_usage = get_min_replica_disk_usage(global_view);
    if(_min_replica_disk_usage > _balance_threshold){
        LOG_DEBUG("Balance threshold is not larger than min replica disk usage,adjust it.");
        return false;
    }

    return true;
}


void disk_usage_app_balance_policy::balance(bool checker, const meta_view *global_view, migration_list *list)
{
    //when min_replica_disk_usage > balance_threshold will not do balance
    if(!init(global_view, list)){
        return;
    }
    const app_mapper &apps = *_global_view->apps;
    if (!execute_balance(apps,
                         checker,
                         _balancer_in_turn,
                         _only_move_primary,
                         //todo:在这里primary_balance可能要重写，这里的多态不知道是否起作用
                         std::bind(&disk_usage_app_balance_policy::primary_balance,
                                   this,
                                   std::placeholders::_1,
                                   std::placeholders::_2))) {
        return;
    }

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

//没改
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
    node_mapper &nodes = *(_global_view->nodes);
    const app_mapper &apps = *_global_view->apps;
    replica_disk_usage_mapper &replicas = *_global_view->replicas;
    disk_total_usage_mapper &disks =  *_global_view->disks;

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

    //todo:优雅实现copy_secondary_operation_by_disk类
    std::unique_ptr<copy_replica_operation> operation = std::make_unique<copy_secondary_operation_by_disk>(
        app, apps, nodes, address_vec, address_id, replicas_low);
    return operation->start(_migration_result);
}

bool disk_usage_app_balance_policy::copy_primary(const std::shared_ptr<app_state> &app,bool still_have_less_than_average){
    node_mapper &nodes = *(_global_view->nodes);
    const app_mapper &apps = *_global_view->apps;
    replica_disk_usage_mapper &replicas = *_global_view->replicas;
    disk_total_usage_mapper &disks =  *_global_view->disks;
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

    std::unique_ptr<copy_replica_operation> operation = std::make_unique<copy_primary_operation_by_disk>(
        app, apps, nodes,replicas,disks, address_vec, address_id, still_have_less_than_average, replicas_low);
    return operation->start(_migration_result);
}








copy_operation_by_disk::copy_operation_by_disk(const std::shared_ptr<app_state> app,
                                                                     const app_mapper &apps,
                                                                     node_mapper &nodes,
                                                                     const replica_disk_usage_mapper &replicas,
                                                                     const disk_total_usage_mapper &disks,
                                                                     const std::vector<dsn::rpc_address> &address_vec,
                                                                     const std::unordered_map<dsn::rpc_address, int> &address_id,
                                                                     int replicas_low)
    : _app(app), _apps(apps), _nodes(nodes), _address_vec(address_vec), _address_id(address_id),_replicas(replicas),_disks(disks)
{
}

bool copy_operation_by_disk::start(migration_list *result)
{
    init_ordered_address_by_disk();

    while (true) {
        if (!can_continue()) {
            break;
        }
        gpid selected_pid = select_partition(result);
        if (selected_pid.get_app_id() != -1) {
            copy_once(selected_pid, result);
            update_ordered_address_by_disk(selected_pid);
        } else {
            _ordered_address_by_disk.erase(--_ordered_address_by_disk.end());
        }
    }
    return true;
}

int copy_operation_by_disk::get_node_disk_usage(const dsn::replication::node_state &ns) const
{
    int app_relica_disk_usage = 0;
    for(auto iter : _replicas.at(ns.addr())){
        if(iter.first.get_app_id() == _app->app_id){
            app_relica_disk_usage += iter.second;
        }
    }
    return app_relica_disk_usage;
}


void copy_operation_by_disk::init_ordered_address_by_disk()
{
    _disk_usage.resize(_address_vec.size(), 0);
    for (const auto &iter : _nodes) {
        auto id = _address_id.at(iter.first);
        _disk_usage[id] = get_node_disk_usage(iter.second);
    }

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
    _ordered_address_by_disk.swap(ordered_queue);
}

void copy_operation_by_disk::update_ordered_address_by_disk(dsn::gpid selected_gpid)
{
    int id_min = *_ordered_address_by_disk.begin();
    int id_max = *_ordered_address_by_disk.rbegin();
    auto from = _address_vec[id_max];
    int gpid_disk = _replicas[from][selected_gpid];

    _disk_usage[id_max] -= gpid_disk;
    _disk_usage[id_min] += gpid_disk;

    _ordered_address_by_disk.erase(_ordered_address_by_disk.begin());
    _ordered_address_by_disk.erase(--_ordered_address_by_disk.end());

    _ordered_address_by_disk.insert(id_max);
    _ordered_address_by_disk.insert(id_min);
}

gpid copy_operation_by_disk::select_partition(migration_list *result)
{
    const partition_set *partitions = get_all_partitions();

    int id_max = *_ordered_address_ids.rbegin();
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

dsn::gpid copy_operation_by_disk::select_max_load_gpid(const partition_set *partitions,migration_list *result)
{
    int id_max = *_ordered_address_by_disk.rbegin();
    const node_state &ns = _nodes.find(_address_vec[id_max])->second;
    std::string max_disk_tag = get_max_load_disk(ns.addr());

    int max_replica_disk_usage = -1;
    gpid selected_pid(-1, -1);
    for (const gpid &pid : *partitions) {
        if (!can_select(pid, result)) {
            continue;
        }

        const std::string &disk_tag = get_disk_tag(_apps, _address_vec[id_max], pid);
        int cur_replica_disk_usage = _replicas[ns.addr()][pid];
        //this gpid is in max load disk && chose the max load replica on max load disk
        if(disk_tag == max_disk_tag && cur_replica_disk_usage > max_replica_disk_usage){
            max_replica_disk_usage = cur_replica_disk_usage;
            selected_pid = pid;
        }
    }

    return selected_pid;
}


copy_primary_operation_by_disk::copy_primary_operation_by_disk(const std::shared_ptr<app_state> app,
                                                               const app_mapper &apps,
                                                               node_mapper &nodes,
                                                               const replica_disk_usage_mapper &replicas,
                                                               const disk_total_usage_mapper &disks,
                                                               const std::vector<dsn::rpc_address> &address_vec,
                                                               const std::unordered_map<dsn::rpc_address, int> &address_id,
                                                               bool have_lower_than_average,
                                                               int replicas_low)
    : copy_operation_by_disk(app, apps, nodes, replicas,disks,address_vec, address_id,replicas_low)
{
    _have_lower_than_average = have_lower_than_average;
    _replicas_low = replicas_low;
}





bool copy_primary_operation_by_disk::can_select(gpid pid, migration_list *result)
{
    return pid.get_app_id() == _app->app_id && result->find(pid) == result->end();
}

bool copy_primary_operation_by_disk::can_continue()
{
    int id_min = *_ordered_address_by_disk.begin();
    if (_have_lower_than_average && _disk_usage[id_min] >= _replicas_low) {
       LOG_DEBUG("{}: stop the copy due to primaries on all nodes will reach low later.",
                _app->get_logname());
       return false;
    }

    int id_max = *_ordered_address_by_disk.rbegin();

    //todo: 6657是平衡阈值的代写
    if (!_have_lower_than_average && _disk_usage[id_max] - _disk_usage[id_min] <= 6657) {
       LOG_DEBUG("{}: stop the copy due to the primary will be balanced later.",
                _app->get_logname());
       return false;
    }
    return true;
}


















}
}
