/**
 * Autogenerated by Thrift Compiler (0.9.3)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef replica_client_H
#define replica_client_H

#include <thrift/TDispatchProcessor.h>
#include <thrift/async/TConcurrentClientSyncInfo.h>
#include "replica_admin_types.h"

namespace dsn { namespace replication {

#ifdef _WIN32
  #pragma warning( push )
  #pragma warning (disable : 4250 ) //inheriting methods via dominance 
#endif

class replica_clientIf {
 public:
  virtual ~replica_clientIf() {}
  virtual void query_disk_info(query_disk_info_response& _return, const query_disk_info_request& req) = 0;
  virtual void disk_migrate(replica_disk_migrate_response& _return, const replica_disk_migrate_request& req) = 0;
  virtual void add_disk(add_new_disk_response& _return, const add_new_disk_request& req) = 0;
};

class replica_clientIfFactory {
 public:
  typedef replica_clientIf Handler;

  virtual ~replica_clientIfFactory() {}

  virtual replica_clientIf* getHandler(const ::apache::thrift::TConnectionInfo& connInfo) = 0;
  virtual void releaseHandler(replica_clientIf* /* handler */) = 0;
};

class replica_clientIfSingletonFactory : virtual public replica_clientIfFactory {
 public:
  replica_clientIfSingletonFactory(const boost::shared_ptr<replica_clientIf>& iface) : iface_(iface) {}
  virtual ~replica_clientIfSingletonFactory() {}

  virtual replica_clientIf* getHandler(const ::apache::thrift::TConnectionInfo&) {
    return iface_.get();
  }
  virtual void releaseHandler(replica_clientIf* /* handler */) {}

 protected:
  boost::shared_ptr<replica_clientIf> iface_;
};

class replica_clientNull : virtual public replica_clientIf {
 public:
  virtual ~replica_clientNull() {}
  void query_disk_info(query_disk_info_response& /* _return */, const query_disk_info_request& /* req */) {
    return;
  }
  void disk_migrate(replica_disk_migrate_response& /* _return */, const replica_disk_migrate_request& /* req */) {
    return;
  }
  void add_disk(add_new_disk_response& /* _return */, const add_new_disk_request& /* req */) {
    return;
  }
};

typedef struct _replica_client_query_disk_info_args__isset {
  _replica_client_query_disk_info_args__isset() : req(false) {}
  bool req :1;
} _replica_client_query_disk_info_args__isset;

class replica_client_query_disk_info_args {
 public:

  replica_client_query_disk_info_args(const replica_client_query_disk_info_args&);
  replica_client_query_disk_info_args& operator=(const replica_client_query_disk_info_args&);
  replica_client_query_disk_info_args() {
  }

  virtual ~replica_client_query_disk_info_args() throw();
  query_disk_info_request req;

  _replica_client_query_disk_info_args__isset __isset;

  void __set_req(const query_disk_info_request& val);

  bool operator == (const replica_client_query_disk_info_args & rhs) const
  {
    if (!(req == rhs.req))
      return false;
    return true;
  }
  bool operator != (const replica_client_query_disk_info_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const replica_client_query_disk_info_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class replica_client_query_disk_info_pargs {
 public:


  virtual ~replica_client_query_disk_info_pargs() throw();
  const query_disk_info_request* req;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _replica_client_query_disk_info_result__isset {
  _replica_client_query_disk_info_result__isset() : success(false) {}
  bool success :1;
} _replica_client_query_disk_info_result__isset;

class replica_client_query_disk_info_result {
 public:

  replica_client_query_disk_info_result(const replica_client_query_disk_info_result&);
  replica_client_query_disk_info_result& operator=(const replica_client_query_disk_info_result&);
  replica_client_query_disk_info_result() {
  }

  virtual ~replica_client_query_disk_info_result() throw();
  query_disk_info_response success;

  _replica_client_query_disk_info_result__isset __isset;

  void __set_success(const query_disk_info_response& val);

  bool operator == (const replica_client_query_disk_info_result & rhs) const
  {
    if (!(success == rhs.success))
      return false;
    return true;
  }
  bool operator != (const replica_client_query_disk_info_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const replica_client_query_disk_info_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _replica_client_query_disk_info_presult__isset {
  _replica_client_query_disk_info_presult__isset() : success(false) {}
  bool success :1;
} _replica_client_query_disk_info_presult__isset;

class replica_client_query_disk_info_presult {
 public:


  virtual ~replica_client_query_disk_info_presult() throw();
  query_disk_info_response* success;

  _replica_client_query_disk_info_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

typedef struct _replica_client_disk_migrate_args__isset {
  _replica_client_disk_migrate_args__isset() : req(false) {}
  bool req :1;
} _replica_client_disk_migrate_args__isset;

class replica_client_disk_migrate_args {
 public:

  replica_client_disk_migrate_args(const replica_client_disk_migrate_args&);
  replica_client_disk_migrate_args& operator=(const replica_client_disk_migrate_args&);
  replica_client_disk_migrate_args() {
  }

  virtual ~replica_client_disk_migrate_args() throw();
  replica_disk_migrate_request req;

  _replica_client_disk_migrate_args__isset __isset;

  void __set_req(const replica_disk_migrate_request& val);

  bool operator == (const replica_client_disk_migrate_args & rhs) const
  {
    if (!(req == rhs.req))
      return false;
    return true;
  }
  bool operator != (const replica_client_disk_migrate_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const replica_client_disk_migrate_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class replica_client_disk_migrate_pargs {
 public:


  virtual ~replica_client_disk_migrate_pargs() throw();
  const replica_disk_migrate_request* req;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _replica_client_disk_migrate_result__isset {
  _replica_client_disk_migrate_result__isset() : success(false) {}
  bool success :1;
} _replica_client_disk_migrate_result__isset;

class replica_client_disk_migrate_result {
 public:

  replica_client_disk_migrate_result(const replica_client_disk_migrate_result&);
  replica_client_disk_migrate_result& operator=(const replica_client_disk_migrate_result&);
  replica_client_disk_migrate_result() {
  }

  virtual ~replica_client_disk_migrate_result() throw();
  replica_disk_migrate_response success;

  _replica_client_disk_migrate_result__isset __isset;

  void __set_success(const replica_disk_migrate_response& val);

  bool operator == (const replica_client_disk_migrate_result & rhs) const
  {
    if (!(success == rhs.success))
      return false;
    return true;
  }
  bool operator != (const replica_client_disk_migrate_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const replica_client_disk_migrate_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _replica_client_disk_migrate_presult__isset {
  _replica_client_disk_migrate_presult__isset() : success(false) {}
  bool success :1;
} _replica_client_disk_migrate_presult__isset;

class replica_client_disk_migrate_presult {
 public:


  virtual ~replica_client_disk_migrate_presult() throw();
  replica_disk_migrate_response* success;

  _replica_client_disk_migrate_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

typedef struct _replica_client_add_disk_args__isset {
  _replica_client_add_disk_args__isset() : req(false) {}
  bool req :1;
} _replica_client_add_disk_args__isset;

class replica_client_add_disk_args {
 public:

  replica_client_add_disk_args(const replica_client_add_disk_args&);
  replica_client_add_disk_args& operator=(const replica_client_add_disk_args&);
  replica_client_add_disk_args() {
  }

  virtual ~replica_client_add_disk_args() throw();
  add_new_disk_request req;

  _replica_client_add_disk_args__isset __isset;

  void __set_req(const add_new_disk_request& val);

  bool operator == (const replica_client_add_disk_args & rhs) const
  {
    if (!(req == rhs.req))
      return false;
    return true;
  }
  bool operator != (const replica_client_add_disk_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const replica_client_add_disk_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class replica_client_add_disk_pargs {
 public:


  virtual ~replica_client_add_disk_pargs() throw();
  const add_new_disk_request* req;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _replica_client_add_disk_result__isset {
  _replica_client_add_disk_result__isset() : success(false) {}
  bool success :1;
} _replica_client_add_disk_result__isset;

class replica_client_add_disk_result {
 public:

  replica_client_add_disk_result(const replica_client_add_disk_result&);
  replica_client_add_disk_result& operator=(const replica_client_add_disk_result&);
  replica_client_add_disk_result() {
  }

  virtual ~replica_client_add_disk_result() throw();
  add_new_disk_response success;

  _replica_client_add_disk_result__isset __isset;

  void __set_success(const add_new_disk_response& val);

  bool operator == (const replica_client_add_disk_result & rhs) const
  {
    if (!(success == rhs.success))
      return false;
    return true;
  }
  bool operator != (const replica_client_add_disk_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const replica_client_add_disk_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _replica_client_add_disk_presult__isset {
  _replica_client_add_disk_presult__isset() : success(false) {}
  bool success :1;
} _replica_client_add_disk_presult__isset;

class replica_client_add_disk_presult {
 public:


  virtual ~replica_client_add_disk_presult() throw();
  add_new_disk_response* success;

  _replica_client_add_disk_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

class replica_clientClient : virtual public replica_clientIf {
 public:
  replica_clientClient(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
    setProtocol(prot);
  }
  replica_clientClient(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, boost::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
    setProtocol(iprot,oprot);
  }
 private:
  void setProtocol(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
  setProtocol(prot,prot);
  }
  void setProtocol(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, boost::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
    piprot_=iprot;
    poprot_=oprot;
    iprot_ = iprot.get();
    oprot_ = oprot.get();
  }
 public:
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> getInputProtocol() {
    return piprot_;
  }
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> getOutputProtocol() {
    return poprot_;
  }
  void query_disk_info(query_disk_info_response& _return, const query_disk_info_request& req);
  void send_query_disk_info(const query_disk_info_request& req);
  void recv_query_disk_info(query_disk_info_response& _return);
  void disk_migrate(replica_disk_migrate_response& _return, const replica_disk_migrate_request& req);
  void send_disk_migrate(const replica_disk_migrate_request& req);
  void recv_disk_migrate(replica_disk_migrate_response& _return);
  void add_disk(add_new_disk_response& _return, const add_new_disk_request& req);
  void send_add_disk(const add_new_disk_request& req);
  void recv_add_disk(add_new_disk_response& _return);
 protected:
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> piprot_;
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> poprot_;
  ::apache::thrift::protocol::TProtocol* iprot_;
  ::apache::thrift::protocol::TProtocol* oprot_;
};

class replica_clientProcessor : public ::apache::thrift::TDispatchProcessor {
 protected:
  boost::shared_ptr<replica_clientIf> iface_;
  virtual bool dispatchCall(::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, const std::string& fname, int32_t seqid, void* callContext);
 private:
  typedef  void (replica_clientProcessor::*ProcessFunction)(int32_t, ::apache::thrift::protocol::TProtocol*, ::apache::thrift::protocol::TProtocol*, void*);
  typedef std::map<std::string, ProcessFunction> ProcessMap;
  ProcessMap processMap_;
  void process_query_disk_info(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_disk_migrate(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_add_disk(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
 public:
  replica_clientProcessor(boost::shared_ptr<replica_clientIf> iface) :
    iface_(iface) {
    processMap_["query_disk_info"] = &replica_clientProcessor::process_query_disk_info;
    processMap_["disk_migrate"] = &replica_clientProcessor::process_disk_migrate;
    processMap_["add_disk"] = &replica_clientProcessor::process_add_disk;
  }

  virtual ~replica_clientProcessor() {}
};

class replica_clientProcessorFactory : public ::apache::thrift::TProcessorFactory {
 public:
  replica_clientProcessorFactory(const ::boost::shared_ptr< replica_clientIfFactory >& handlerFactory) :
      handlerFactory_(handlerFactory) {}

  ::boost::shared_ptr< ::apache::thrift::TProcessor > getProcessor(const ::apache::thrift::TConnectionInfo& connInfo);

 protected:
  ::boost::shared_ptr< replica_clientIfFactory > handlerFactory_;
};

class replica_clientMultiface : virtual public replica_clientIf {
 public:
  replica_clientMultiface(std::vector<boost::shared_ptr<replica_clientIf> >& ifaces) : ifaces_(ifaces) {
  }
  virtual ~replica_clientMultiface() {}
 protected:
  std::vector<boost::shared_ptr<replica_clientIf> > ifaces_;
  replica_clientMultiface() {}
  void add(boost::shared_ptr<replica_clientIf> iface) {
    ifaces_.push_back(iface);
  }
 public:
  void query_disk_info(query_disk_info_response& _return, const query_disk_info_request& req) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->query_disk_info(_return, req);
    }
    ifaces_[i]->query_disk_info(_return, req);
    return;
  }

  void disk_migrate(replica_disk_migrate_response& _return, const replica_disk_migrate_request& req) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->disk_migrate(_return, req);
    }
    ifaces_[i]->disk_migrate(_return, req);
    return;
  }

  void add_disk(add_new_disk_response& _return, const add_new_disk_request& req) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->add_disk(_return, req);
    }
    ifaces_[i]->add_disk(_return, req);
    return;
  }

};

// The 'concurrent' client is a thread safe client that correctly handles
// out of order responses.  It is slower than the regular client, so should
// only be used when you need to share a connection among multiple threads
class replica_clientConcurrentClient : virtual public replica_clientIf {
 public:
  replica_clientConcurrentClient(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
    setProtocol(prot);
  }
  replica_clientConcurrentClient(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, boost::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
    setProtocol(iprot,oprot);
  }
 private:
  void setProtocol(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
  setProtocol(prot,prot);
  }
  void setProtocol(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, boost::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
    piprot_=iprot;
    poprot_=oprot;
    iprot_ = iprot.get();
    oprot_ = oprot.get();
  }
 public:
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> getInputProtocol() {
    return piprot_;
  }
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> getOutputProtocol() {
    return poprot_;
  }
  void query_disk_info(query_disk_info_response& _return, const query_disk_info_request& req);
  int32_t send_query_disk_info(const query_disk_info_request& req);
  void recv_query_disk_info(query_disk_info_response& _return, const int32_t seqid);
  void disk_migrate(replica_disk_migrate_response& _return, const replica_disk_migrate_request& req);
  int32_t send_disk_migrate(const replica_disk_migrate_request& req);
  void recv_disk_migrate(replica_disk_migrate_response& _return, const int32_t seqid);
  void add_disk(add_new_disk_response& _return, const add_new_disk_request& req);
  int32_t send_add_disk(const add_new_disk_request& req);
  void recv_add_disk(add_new_disk_response& _return, const int32_t seqid);
 protected:
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> piprot_;
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> poprot_;
  ::apache::thrift::protocol::TProtocol* iprot_;
  ::apache::thrift::protocol::TProtocol* oprot_;
  ::apache::thrift::async::TConcurrentClientSyncInfo sync_;
};

#ifdef _WIN32
  #pragma warning( pop )
#endif

}} // namespace

#endif
