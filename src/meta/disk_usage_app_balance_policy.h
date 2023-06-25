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

#pragma once

#include <gtest/gtest_prod.h>
#include <memory>
#include <unordered_map>
#include <vector>

#include "load_balance_policy.h"
#include "meta/meta_data.h"
#include "utils/command_manager.h"

namespace dsn {
class gpid;
class rpc_address;

namespace replication {
class meta_service;

class disk_usage_app_balance_policy : public load_balance_policy
{
public:
    disk_usage_app_balance_policy(meta_service *svc);
    ~disk_usage_app_balance_policy() = default;

    void balance(bool checker, const meta_view *global_view, migration_list *list) override;
    bool init(const dsn::replication::meta_view *global_view, dsn::replication::migration_list *list,bool checker);

    //todo: add score() override func to make disk usage balance info can show in pegasus-shell

private:
    bool primary_balance(const std::shared_ptr<app_state> &app,
                                              bool only_move_primary);
    bool need_balance_secondaries(bool balance_checker);
    bool copy_primary(const std::shared_ptr<app_state> &app,bool still_have_less_than_average);
    bool copy_secondary(const std::shared_ptr<app_state> &app, bool place_holder);

    bool still_have_replicas_lower_than_average( const std::shared_ptr<app_state> &app,node_mapper nodes,replica_disk_usage_mapper replicas);


    std::vector<std::unique_ptr<command_deregister>> _cmds;

    std::string ctrl_assgin_balance_threshold(const std::vector<std::string> &args);

    // options
    bool _balancer_in_turn;
    bool _only_primary_balancer;
    bool _only_move_primary;

    //new add
    int _balance_threshold;
    int _min_replica_disk_usage;
};

class copy_operation_by_disk : public copy_replica_operation
{
public:
    copy_operation_by_disk(const std::shared_ptr<app_state> app,
                             const app_mapper &apps,
                             node_mapper &nodes,
                           const replica_disk_usage_mapper &replicas,
                           const disk_total_usage_mapper &disks,
                             const std::vector<dsn::rpc_address> &address_vec,
                             const std::unordered_map<dsn::rpc_address, int> &address_id,
                             int replicas_low);
    ~copy_operation_by_disk() = default;

    bool start(migration_list *result);
    void init_ordered_address_by_disk();
    void copy_once(gpid selected_pid, migration_list *result);
    void update_ordered_address_by_disk(dsn::gpid selected_gpid);
    const partition_set* get_all_partitions();
    dsn::gpid select_partition(migration_list *result);
    std::string get_max_load_disk(dsn::rpc_address addr);
    dsn::gpid select_max_load_gpid(const partition_set *partitions,migration_list *result);

    virtual int get_node_disk_usage(const node_state &ns) const = 0;
    dsn::gpid  get_max_replica_disk_usage_smaller_than_expectation_residual(dsn::rpc_address addr,int expectation_residual,bool only_primary);

    int get_partition_count(const node_state &ns) const{return 0;}; //无用的纯虚函数实现

protected:
    //virtual int get_disk_usage(const node_state &ns) const = 0;

    std::vector<int> _disk_usage;
    std::set<int, std::function<bool(int left, int right)>> _ordered_address_by_disk;
    replica_disk_usage_mapper _replicas;
    disk_total_usage_mapper _disks;
    int _replicas_low;

    FRIEND_TEST(copy_operation_by_disk, misc);
};


class copy_primary_operation_by_disk : public copy_operation_by_disk
{
public:
    copy_primary_operation_by_disk(const std::shared_ptr<app_state> app,
                           const app_mapper &apps,
                           node_mapper &nodes,
                                   const replica_disk_usage_mapper &replicas,
                                   const disk_total_usage_mapper &disks,
                           const std::vector<dsn::rpc_address> &address_vec,
                           const std::unordered_map<dsn::rpc_address, int> &address_id,
                           bool have_lower_than_average,
                           int replicas_low,
                                   int balance_threshold);
    ~copy_primary_operation_by_disk() = default;

private:
    bool only_copy_primary() { return true; }
    bool can_select(gpid pid, migration_list *result);
    bool can_continue();
    int get_node_disk_usage(const dsn::replication::node_state &ns) const;

    enum balance_type get_balance_type();


    bool _have_lower_than_average;
    int _replicas_low;
    int _disk_usage_balance_threshold;

    FRIEND_TEST(copy_primary_operation_by_disk, primary_can_select);
    FRIEND_TEST(copy_primary_operation_by_disk, only_copy_primary);
    FRIEND_TEST(copy_primary_operation_by_disk, misc);

};

class copy_secondary_operation_by_disk : public copy_operation_by_disk
{
public:
    copy_secondary_operation_by_disk(const std::shared_ptr<app_state> app,
                             const app_mapper &apps,
                             node_mapper &nodes,
                                     const replica_disk_usage_mapper &replicas,
                                     const disk_total_usage_mapper &disks,
                             const std::vector<dsn::rpc_address> &address_vec,
                             const std::unordered_map<dsn::rpc_address, int> &address_id,
                             int replicas_low,
                                     int balance_threshold);
    ~copy_secondary_operation_by_disk() = default;

private:
    bool can_continue();
    bool can_select(gpid pid, migration_list *result);
    int get_node_disk_usage(const dsn::replication::node_state &ns) const;
    balance_type get_balance_type();
    bool only_copy_primary() { return false; }


    int _replicas_low;
    int _disk_usage_balance_threshold;

    FRIEND_TEST(copy_secondary_operation_by_disk, secondary_can_select);
    FRIEND_TEST(copy_secondary_operation_by_disk, misc);
};




} // namespace replication
} // namespace dsn
