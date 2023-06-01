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

// IWYU pragma: no_include <gtest/gtest-message.h>
// IWYU pragma: no_include <gtest/gtest-test-part.h>
#include <gtest/gtest.h>
#include <stdint.h>
#include <algorithm>
#include <iterator>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "common/gpid.h"
#include "dsn.layer2_types.h"
#include "meta/disk_usage_app_balance_policy.h"
#include "meta/load_balance_policy.h"
#include "meta/meta_data.h"
#include "runtime/rpc/rpc_address.h"
#include "utils/fail_point.h"

namespace dsn {
namespace replication {


TEST(copy_primary_operation_by_disk, misc)
{
    int32_t app_id = 1;
    dsn::app_info info;
    info.app_id = app_id;
    info.partition_count = 4;
    std::shared_ptr<app_state> app = app_state::create(info);
    app_mapper apps;
    apps[app_id] = app;

    auto addr1 = rpc_address(1, 1);
    auto addr2 = rpc_address(1, 2);
    auto addr3 = rpc_address(1, 3);

    node_mapper nodes;
    node_state ns1;
    ns1.address = addr1;
    //put_partition的时候已经维护了app_primaries
    ns1.put_partition(gpid(app_id, 2), true);
    ns1.put_partition(gpid(app_id, 0), false);
    nodes[addr1] = ns1;

    node_state ns2;
    ns2.address = addr2;
    ns2.put_partition(gpid(app_id, 0), true);
    ns2.put_partition(gpid(app_id, 1), true);
    nodes[addr2] = ns2;

    node_state ns3;
    ns3.address = addr3;
    ns3.put_partition(gpid(app_id, 2), false);
    nodes[addr3] = ns3;

    LOG_DEBUG("gns,old code is ok");

    std::vector<dsn::rpc_address> address_vec{addr1, addr2, addr3};
    std::unordered_map<dsn::rpc_address, int> address_id;
    address_id[addr1] = 0;
    address_id[addr2] = 1;
    address_id[addr3] = 2;
    int balance_threshold = 20;
    //old: replicas_low = app->partition_count / _alive_nodes;
    //new: primary_disk_replicas_low = total_primary_disk_usage_of_this_app / _alive_nodes;
    //in this case: primary_disk_replicas_low = 10+70+0 / 3 = 26 MB
    int primary_disk_replicas_low = 26;

    LOG_DEBUG("gns,address_id code is ok");

    //assume disk situation
    replica_disk_usage_mapper replicas;
    disk_total_usage_mapper disks;
    //app_id@0 40M
    //app_id@1 30M
    //app_id@2 10M
    replicas[addr1] = std::map<dsn::gpid,int32_t>{{gpid(app_id, 2),10} , {gpid(app_id, 0),40}};
    replicas[addr2] = std::map<dsn::gpid,int32_t>{{gpid(app_id, 0),40} , {gpid(app_id, 1),30}};
    replicas[addr3] = std::map<dsn::gpid,int32_t>{{gpid(app_id, 2),10}};

    LOG_DEBUG("gns,replicas code is ok");

    //we did not test hybrid deployment of different pegasus cluster
    disks[addr1] = std::map<std::string,int32_t>{{"ssd1",10} ,{"ssd2",40}};
    disks[addr2] = std::map<std::string,int32_t>{{"ssd1",40} ,{"ssd2",30}};
    disks[addr3] = std::map<std::string,int32_t>{{"ssd1",10} ,{"ssd2",0}};

    LOG_DEBUG("gns,disks code is ok");


    copy_primary_operation_by_disk op(app, apps, nodes, replicas,disks,address_vec, address_id, false,primary_disk_replicas_low,balance_threshold);


    /**
     * Test copy_operation_by_disk::init_ordered_address_by_disk
     */
    op.init_ordered_address_by_disk();
    ASSERT_EQ(op._ordered_address_by_disk.size(), 3);
    // min node disk usage [_ordered_address_by_disk store the number of node(address_id)]
    ASSERT_EQ(*op._ordered_address_by_disk.begin(), 2);
    ASSERT_EQ(*(++op._ordered_address_by_disk.begin()), 0);
    // max node disk usage
    ASSERT_EQ(*op._ordered_address_by_disk.rbegin(),1);

    ASSERT_EQ(op._disk_usage[0], 10);
    ASSERT_EQ(op._disk_usage[1], 70);
    ASSERT_EQ(op._disk_usage[2], 0);

    /**
     * Test copy_operation_by_disk::get_all_partitions
     */
     //get all primary partitions from max disk usage load node
    auto partitions = op.get_all_partitions();
    //addr2 with id 1 has 2 primary partition
    ASSERT_EQ(partitions->size(), 2);
    ASSERT_EQ(*partitions->begin(), gpid(app_id, 0));
    ASSERT_EQ(*partitions->rbegin(), gpid(app_id, 1));

    /**
     * Test select_partition
     */
    std::string disk1 = "ssd1", disk2 = "ssd2";
    disk_load load;
    load[disk1] = 40;
    load[disk2] = 30;
    //最大磁盘用量节点的磁盘负载信息
    op._node_loads[addr2] = load;

    migration_list list;
    auto res_gpid = op.select_partition(&list);
    //gpid(app_id, 0),40} is largest
    ASSERT_EQ(res_gpid.get_partition_index(), 0);



    /**
     * Test can_continue
     **/
    op._have_lower_than_average = true;
    //now _disk_usage[id_min]= 0, _replicas_low = 26
    //expectation_residual = 70 - 26 = 44
    ASSERT_TRUE(op.can_continue());

    op._replicas_low = 0;
    //_have_lower_than_average = true,_disk_usage[id_min] = 0 >= _replicas_low = 0
    ASSERT_FALSE(op.can_continue());


    op._replicas_low = 26;
    ASSERT_TRUE(op.can_continue());

    //_disk_usage[id_min]= 0 ;_disk_usage[id_max]= 70 ;_disk_usage_balance_threshold = 100
    op._disk_usage_balance_threshold  =100;
    ASSERT_FALSE(op.can_continue());

    op._disk_usage_balance_threshold  =20;
    op._replicas_low = 65;
    ASSERT_FALSE(op.can_continue());


    //now only app_id@0 in addr2
    nodes[addr2].remove_partition(gpid(app_id, 1), true);
    op._replicas[addr2].erase(gpid(app_id, 1));
    //init 并不会更新_replicas_low 实际上一段时间内计算的磁盘总量不变，也不需要一直变expectation
    op.init_ordered_address_by_disk();
    ASSERT_EQ(op._disk_usage[0], 10);
    ASSERT_EQ(op._disk_usage[1], 40);
    ASSERT_EQ(op._disk_usage[2], 0);
    //reset base feature
    op._disk_usage_balance_threshold  =30;
    op._replicas_low = 26;
    //addr1 10; addr2 40; addr3 0, balance_threshold 30 , expectation_residual = 14 没有符合预期的可调整分片了
    ASSERT_FALSE(op.can_continue());
    nodes[addr3].put_partition(gpid(app_id, 1), true);
    op._replicas[addr3].insert({gpid(app_id, 1),30});
    op.init_ordered_address_by_disk();
    //addr1 10; addr2 40; addr3 30  , balance_threshold 30
    ASSERT_EQ(op._disk_usage[0], 10);
    ASSERT_EQ(op._disk_usage[1], 40);
    ASSERT_EQ(op._disk_usage[2], 30);
    ASSERT_FALSE(op.can_continue());

    /**
     * Test update_ordered_address_by_disk
     */
    //batch update
    nodes[addr1].put_partition(gpid(app_id, 3), true);
    op._replicas[addr1].insert({gpid(app_id, 3),30});
    nodes[addr1].put_partition(gpid(app_id, 4), true);
    op._replicas[addr1].insert({gpid(app_id, 4),40});
    nodes[addr3].put_partition(gpid(app_id, 5), true);
    op._replicas[addr3].insert({gpid(app_id, 5),50});
    op.init_ordered_address_by_disk();


    //addr1 80; addr2 40; addr3 80
    ASSERT_EQ(op._ordered_address_by_disk.size(), 3);

    ASSERT_EQ(op._disk_usage[0], 80);
    ASSERT_EQ(op._disk_usage[1], 40);
    ASSERT_EQ(op._disk_usage[2], 80);

    //id_min
    ASSERT_EQ(*op._ordered_address_by_disk.begin(), 1);
    ASSERT_EQ(*(++op._ordered_address_by_disk.begin()), 0);
    //id_max
    ASSERT_EQ(*op._ordered_address_by_disk.rbegin(), 2);

    //singel update
    //select app_id@5 from addr2 to addr3
    op.update_ordered_address_by_disk(gpid(app_id,5));
    ASSERT_EQ(op._disk_usage[0], 80); //(2,10) (3,30) (4,40)
    ASSERT_EQ(op._disk_usage[1], 90);
    ASSERT_EQ(op._disk_usage[2], 30); //(2,10) (1,30)


    /**
     * Test copy_once
     */
    fail::setup();
    fail::cfg("generate_balancer_request", "return()");
    gpid gpid1 = gpid(app_id,5);
    gpid gpid2 = gpid(app_id,4);

    list.clear();
    op.copy_once(gpid1, &list);
    ASSERT_EQ(list.size(), 1);
    ASSERT_EQ(list.count(gpid1), 1);
    ASSERT_EQ(list.count(gpid2), 0);
    fail::teardown();
}

TEST(copy_primary_operation_by_disk, primary_can_select)
{
    app_mapper apps;
    node_mapper nodes;
    std::vector<dsn::rpc_address> address_vec;
    std::unordered_map<dsn::rpc_address, int> address_id;
    replica_disk_usage_mapper replicas;
    disk_total_usage_mapper disks;
    int replica_low = 60;
    int balance_threshold = 20;

    copy_primary_operation_by_disk op(nullptr, apps, nodes, replicas,disks,address_vec, address_id, false, replica_low,balance_threshold);

    gpid cannot_select_gpid(1, 1);
    gpid can_select_gpid(1, 2);
    migration_list list;
    //put cannot_select_gpid in list firstly
    list[cannot_select_gpid] = nullptr;

    ASSERT_FALSE(op.can_select(cannot_select_gpid, &list));
    ASSERT_TRUE(op.can_select(can_select_gpid, &list));
}

TEST(copy_primary_operation_by_disk, only_copy_primary)
{
    app_mapper apps;
    node_mapper nodes;
    std::vector<dsn::rpc_address> address_vec;
    std::unordered_map<dsn::rpc_address, int> address_id;
    replica_disk_usage_mapper replicas;
    disk_total_usage_mapper disks;
    int replica_low = 60;
    int balance_threshold = 20;
    copy_primary_operation_by_disk op(nullptr, apps, nodes, replicas,disks,address_vec, address_id, false, replica_low,balance_threshold);


    ASSERT_TRUE(op.only_copy_primary());
}


TEST(copy_secondary_operation_by_disk, misc)
{
    int32_t app_id = 2;
    dsn::app_info info;
    info.app_id = app_id;
    info.partition_count = 4;
    std::shared_ptr<app_state> app = app_state::create(info);
    app_mapper apps;
    apps[app_id] = app;

    auto addr1 = rpc_address(1, 1);
    auto addr2 = rpc_address(1, 2);
    auto addr3 = rpc_address(1, 3);

    node_mapper nodes;
    node_state ns1;
    ns1.address = addr1;
    //put_partition的时候已经维护了app_primaries
    ns1.put_partition(gpid(app_id, 2), true);
    ns1.put_partition(gpid(app_id, 0), true);
    nodes[addr1] = ns1;

    node_state ns2;
    ns2.address = addr2;
    ns2.put_partition(gpid(app_id, 0), false);
    ns2.put_partition(gpid(app_id, 1), true);
    ns2.put_partition(gpid(app_id, 3), false);
    nodes[addr2] = ns2;

    node_state ns3;
    ns3.address = addr3;
    ns3.put_partition(gpid(app_id, 2), false);
    ns3.put_partition(gpid(app_id, 1), false);
    ns3.put_partition(gpid(app_id, 3), true);
    nodes[addr3] = ns3;


    std::vector<dsn::rpc_address> address_vec{addr1, addr2, addr3};
    std::unordered_map<dsn::rpc_address, int> address_id;
    address_id[addr1] = 0;
    address_id[addr2] = 1;
    address_id[addr3] = 2;
    int balance_threshold = 20;

    //in this case: secondary_disk_replicas_low = 50+75+45 / 3 = 56 MB
    int secondary_disk_replicas_low = 56;

    //assume disk situation
    replica_disk_usage_mapper replicas;
    disk_total_usage_mapper disks;
    //app_id@0 40M
    //app_id@1 30M
    //app_id@2 10M
    replicas[addr1] = std::map<dsn::gpid,int32_t>{{gpid(app_id, 2),10} , {gpid(app_id, 0),40}};
    replicas[addr2] = std::map<dsn::gpid,int32_t>{{gpid(app_id, 0),40} , {gpid(app_id, 1),30} , {gpid(app_id, 3),5}};
    replicas[addr3] = std::map<dsn::gpid,int32_t>{{gpid(app_id, 2),10} , {gpid(app_id, 1),30} , {gpid(app_id, 3),5}};


    //we did not test hybrid deployment of different pegasus cluster
    disks[addr1] = std::map<std::string,int32_t>{{"ssd1",10} ,{"ssd2",40}};
    disks[addr2] = std::map<std::string,int32_t>{{"ssd1",45} ,{"ssd2",30}};
    disks[addr3] = std::map<std::string,int32_t>{{"ssd1",15} ,{"ssd2",30}};

    copy_secondary_operation_by_disk op(app, apps, nodes, replicas,disks,address_vec, address_id, secondary_disk_replicas_low,balance_threshold);
    LOG_DEBUG("nins,secondary init secondary is ok");
    //=======================init done ==========================

    /**
     * Test copy_operation_by_disk::init_ordered_address_by_disk
     */
    op.init_ordered_address_by_disk();
    LOG_DEBUG("nins,secondary init_ordered_address_by_disk is ok");

    ASSERT_EQ(op._ordered_address_by_disk.size(), 3);
    // min node disk usage [_ordered_address_by_disk store the number of node(address_id)]
    ASSERT_EQ(*op._ordered_address_by_disk.begin(), 2);
    ASSERT_EQ(*(++op._ordered_address_by_disk.begin()), 0);
    // max node disk usage
    ASSERT_EQ(*op._ordered_address_by_disk.rbegin(),1);

    ASSERT_EQ(op._disk_usage[0], 50);
    ASSERT_EQ(op._disk_usage[1], 75);
    ASSERT_EQ(op._disk_usage[2], 45);


    /**
     * Test copy_operation_by_disk::get_all_partitions
     */
    //get all primary partitions from max disk usage load node
    auto partitions = op.get_all_partitions();
    //addr2 has 3 all partitions
    ASSERT_EQ(partitions->size(), 3);
    //all partition be seen as secondary,cause copy primary already done
    ASSERT_EQ(*partitions->begin(), gpid(app_id, 0));
    ASSERT_EQ(*partitions->rbegin(), gpid(app_id, 3));
    LOG_DEBUG("nins,secondary get_all_partitions is ok");


    migration_list list;
    /**
     * Test can_select
     */
     //1,2,3 is in id_min node
    ASSERT_FALSE(op.can_select( gpid(app_id, 1),&list));
    ASSERT_FALSE(op.can_select( gpid(app_id, 2),&list));
    ASSERT_FALSE(op.can_select( gpid(app_id, 3),&list));

    //0 is not a primary replica
    ASSERT_TRUE(op.can_select( gpid(app_id, 0),&list));
    //7 is fake replica
    ASSERT_TRUE(op.can_select( gpid(app_id, 7),&list));




    /**
     * Test select_partition
     */
    std::string disk1 = "ssd1", disk2 = "ssd2";
    disk_load load;
    load[disk1] = 45;
    load[disk2] = 30;
    //最大磁盘用量节点的磁盘负载信息
    op._node_loads[addr2] = load;


    auto res_gpid = op.select_partition(&list);
    //gpid(app_id, 0),40} is largest   (0,40),(1,30),(2,5)
    ASSERT_EQ(res_gpid.get_partition_index(), 0);
    LOG_DEBUG("nins,secondary select_partition is ok");

    /**
     * Test get_node_disk_usage
     */
    ASSERT_EQ(op.get_node_disk_usage(ns1),50);
    ASSERT_EQ(op.get_node_disk_usage(ns2),75);
    ASSERT_EQ(op.get_node_disk_usage(ns3),45);
    LOG_DEBUG("nins,secondary get_node_disk_usage is ok");

}


TEST(copy_secondary_operation_by_disk, secondary_can_select)
{
    int32_t app_id = 2;
    dsn::app_info info;
    info.app_id = app_id;
    info.partition_count = 4;
    std::shared_ptr<app_state> app = app_state::create(info);
    app_mapper apps;
    apps[app_id] = app;

    node_mapper nodes;
    std::vector<dsn::rpc_address> address_vec;
    std::unordered_map<dsn::rpc_address, int> address_id;
    replica_disk_usage_mapper replicas;
    disk_total_usage_mapper disks;
    int replica_low = 60;
    int balance_threshold = 20;

    copy_secondary_operation_by_disk op(app, apps, nodes, replicas,disks,address_vec, address_id, replica_low,balance_threshold);


    migration_list list;
    //if find a gpid not belong to this app
    gpid not_this_app(1,1);
    ASSERT_FALSE(op.can_select(not_this_app,&list));

    //if pid has been used
    gpid cannot_select_gpid(1, 1);
    gpid can_select_gpid(1, 2);
    list[cannot_select_gpid] = nullptr;

    ASSERT_FALSE(op.can_select(cannot_select_gpid, &list));
    ASSERT_TRUE(op.can_select(can_select_gpid, &list));

    ASSERT_FALSE(op.only_copy_primary());


}

}
}