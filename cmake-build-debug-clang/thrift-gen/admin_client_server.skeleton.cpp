// This autogenerated skeleton file illustrates how to build a server.
// You should copy it to another filename to avoid overwriting it.

#include "admin_client.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;

using namespace  ::dsn::replication;

class admin_clientHandler : virtual public admin_clientIf {
 public:
  admin_clientHandler() {
    // Your initialization goes here
  }

  void create_app(configuration_create_app_response& _return, const configuration_create_app_request& req) {
    // Your implementation goes here
    printf("create_app\n");
  }

  void drop_app(configuration_drop_app_response& _return, const configuration_drop_app_request& req) {
    // Your implementation goes here
    printf("drop_app\n");
  }

  void recall_app(configuration_recall_app_response& _return, const configuration_recall_app_request& req) {
    // Your implementation goes here
    printf("recall_app\n");
  }

  void list_apps(configuration_list_apps_response& _return, const configuration_list_apps_request& req) {
    // Your implementation goes here
    printf("list_apps\n");
  }

  void add_duplication( ::dsn::replication::duplication_add_response& _return, const  ::dsn::replication::duplication_add_request& req) {
    // Your implementation goes here
    printf("add_duplication\n");
  }

  void query_duplication( ::dsn::replication::duplication_query_response& _return, const  ::dsn::replication::duplication_query_request& req) {
    // Your implementation goes here
    printf("query_duplication\n");
  }

  void modify_duplication( ::dsn::replication::duplication_modify_response& _return, const  ::dsn::replication::duplication_modify_request& req) {
    // Your implementation goes here
    printf("modify_duplication\n");
  }

  void query_app_info(query_app_info_response& _return, const query_app_info_request& req) {
    // Your implementation goes here
    printf("query_app_info\n");
  }

  void update_app_env(configuration_update_app_env_response& _return, const configuration_update_app_env_request& req) {
    // Your implementation goes here
    printf("update_app_env\n");
  }

  void list_nodes(configuration_list_nodes_response& _return, const configuration_list_nodes_request& req) {
    // Your implementation goes here
    printf("list_nodes\n");
  }

  void query_cluster_info(configuration_cluster_info_response& _return, const configuration_cluster_info_request& req) {
    // Your implementation goes here
    printf("query_cluster_info\n");
  }

  void meta_control(configuration_meta_control_response& _return, const configuration_meta_control_request& req) {
    // Your implementation goes here
    printf("meta_control\n");
  }

  void query_backup_policy( ::dsn::replication::configuration_query_backup_policy_response& _return, const  ::dsn::replication::configuration_query_backup_policy_request& req) {
    // Your implementation goes here
    printf("query_backup_policy\n");
  }

  void balance(configuration_balancer_response& _return, const configuration_balancer_request& req) {
    // Your implementation goes here
    printf("balance\n");
  }

  void start_backup_app( ::dsn::replication::start_backup_app_response& _return, const  ::dsn::replication::start_backup_app_request& req) {
    // Your implementation goes here
    printf("start_backup_app\n");
  }

  void query_backup_status( ::dsn::replication::query_backup_status_response& _return, const  ::dsn::replication::query_backup_status_request& req) {
    // Your implementation goes here
    printf("query_backup_status\n");
  }

  void restore_app(configuration_create_app_response& _return, const  ::dsn::replication::configuration_restore_request& req) {
    // Your implementation goes here
    printf("restore_app\n");
  }

  void start_partition_split( ::dsn::replication::start_partition_split_response& _return, const  ::dsn::replication::start_partition_split_request& req) {
    // Your implementation goes here
    printf("start_partition_split\n");
  }

  void query_split_status( ::dsn::replication::query_split_response& _return, const  ::dsn::replication::query_split_request& req) {
    // Your implementation goes here
    printf("query_split_status\n");
  }

  void control_partition_split( ::dsn::replication::control_split_response& _return, const  ::dsn::replication::control_split_request& req) {
    // Your implementation goes here
    printf("control_partition_split\n");
  }

  void start_bulk_load( ::dsn::replication::start_bulk_load_response& _return, const  ::dsn::replication::start_bulk_load_request& req) {
    // Your implementation goes here
    printf("start_bulk_load\n");
  }

  void query_bulk_load_status( ::dsn::replication::query_bulk_load_response& _return, const  ::dsn::replication::query_bulk_load_request& req) {
    // Your implementation goes here
    printf("query_bulk_load_status\n");
  }

  void control_bulk_load( ::dsn::replication::control_bulk_load_response& _return, const  ::dsn::replication::control_bulk_load_request& req) {
    // Your implementation goes here
    printf("control_bulk_load\n");
  }

  void clear_bulk_load( ::dsn::replication::clear_bulk_load_state_response& _return, const  ::dsn::replication::clear_bulk_load_state_request& req) {
    // Your implementation goes here
    printf("clear_bulk_load\n");
  }

  void start_manual_compact(start_app_manual_compact_response& _return, const start_app_manual_compact_request& req) {
    // Your implementation goes here
    printf("start_manual_compact\n");
  }

  void query_manual_compact(query_app_manual_compact_response& _return, const query_app_manual_compact_request& req) {
    // Your implementation goes here
    printf("query_manual_compact\n");
  }

};

int main(int argc, char **argv) {
  int port = 9090;
  shared_ptr<admin_clientHandler> handler(new admin_clientHandler());
  shared_ptr<TProcessor> processor(new admin_clientProcessor(handler));
  shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
  server.serve();
  return 0;
}

