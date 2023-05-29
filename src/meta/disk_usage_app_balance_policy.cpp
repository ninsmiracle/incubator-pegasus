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
DSN_DECLARE_uint64(min_live_node_count_for_unfreeze);

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
        //在这里迭代执行
        bool result = ns->for_each_primary(id, add_one_replica_to_disk_usage);
        return result;
    } else {
        bool result = ns->for_each_partition(id, add_one_replica_to_disk_usage);
        return result;
    }
}


copy_operation_by_disk::copy_operation_by_disk(
    const std::shared_ptr<app_state> app,
    const app_mapper &apps,
    node_mapper &nodes,
    const replica_disk_usage_mapper &replicas,
    const disk_total_usage_mapper &disks,
    const std::vector<dsn::rpc_address> &address_vec,
    const std::unordered_map<dsn::rpc_address, int> &address_id)
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

void copy_operation_by_disk::init_ordered_address_by_disk()
{
    _disk_usage.resize(_address_vec.size(), 0);
    for (const auto &iter : _nodes) {
        auto id = _address_id.at(iter.first);
        _disk_usage[id] = get_partition_count(iter.second);
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
        if(iter->second > max_disk_load){
            max_disk_load = iter->second;
            max_disk_tag = iter->first;
        }
    }

    return max_disk_tag;
}

dsn::gpid copy_operation_by_disk::select_max_load_gpid(const partition_set *partitions,migration_list *result)
{
    int id_max = *_ordered_address_ids.rbegin();
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












bool copy_primary_operation_by_disk::can_continue()
{
    int id_min = *_ordered_address_by_disk.begin();
    //当前节点的最小replica数量都大于理论值，没有迁移replica的目标节点
    if (_have_lower_than_average && _partition_counts[id_min] >= _replicas_low) {
        LOG_DEBUG("{}: stop the copy due to primaries on all nodes will reach low later.",
                 _app->get_logname());
        return false;
    }

    int id_max = *_ordered_address_by_disk.rbegin();
    if (!_have_lower_than_average && _partition_counts[id_max] - _partition_counts[id_min] <= 1) {
        LOG_DEBUG("{}: stop the copy due to the primary will be balanced later.",
                 _app->get_logname());
        return false;
    }
    return true;
}
















}
}
