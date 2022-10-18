/**
 * Autogenerated by Thrift Compiler (0.9.3)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef fd_TYPES_H
#define fd_TYPES_H

#include <iosfwd>

#include <thrift/Thrift.h>
#include <thrift/TApplicationException.h>
#include <thrift/protocol/TProtocol.h>
#include <thrift/transport/TTransport.h>

#include <thrift/cxxfunctional.h>
#include "dsn_types.h"


namespace dsn { namespace fd {

class beacon_msg;

class beacon_ack;

class config_master_message;

typedef struct _beacon_msg__isset {
  _beacon_msg__isset() : time(false), from_addr(false), to_addr(false), start_time(false) {}
  bool time :1;
  bool from_addr :1;
  bool to_addr :1;
  bool start_time :1;
} _beacon_msg__isset;

class beacon_msg {
 public:

  beacon_msg(const beacon_msg&);
  beacon_msg& operator=(const beacon_msg&);
  beacon_msg() : time(0), start_time(0) {
  }

  virtual ~beacon_msg() throw();
  int64_t time;
   ::dsn::rpc_address from_addr;
   ::dsn::rpc_address to_addr;
  int64_t start_time;

  _beacon_msg__isset __isset;

  void __set_time(const int64_t val);

  void __set_from_addr(const  ::dsn::rpc_address& val);

  void __set_to_addr(const  ::dsn::rpc_address& val);

  void __set_start_time(const int64_t val);

  bool operator == (const beacon_msg & rhs) const
  {
    if (!(time == rhs.time))
      return false;
    if (!(from_addr == rhs.from_addr))
      return false;
    if (!(to_addr == rhs.to_addr))
      return false;
    if (__isset.start_time != rhs.__isset.start_time)
      return false;
    else if (__isset.start_time && !(start_time == rhs.start_time))
      return false;
    return true;
  }
  bool operator != (const beacon_msg &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const beacon_msg & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  virtual void printTo(std::ostream& out) const;
};

void swap(beacon_msg &a, beacon_msg &b);

inline std::ostream& operator<<(std::ostream& out, const beacon_msg& obj)
{
  obj.printTo(out);
  return out;
}

typedef struct _beacon_ack__isset {
  _beacon_ack__isset() : time(false), this_node(false), primary_node(false), is_master(false), allowed(false) {}
  bool time :1;
  bool this_node :1;
  bool primary_node :1;
  bool is_master :1;
  bool allowed :1;
} _beacon_ack__isset;

class beacon_ack {
 public:

  beacon_ack(const beacon_ack&);
  beacon_ack& operator=(const beacon_ack&);
  beacon_ack() : time(0), is_master(0), allowed(0) {
  }

  virtual ~beacon_ack() throw();
  int64_t time;
   ::dsn::rpc_address this_node;
   ::dsn::rpc_address primary_node;
  bool is_master;
  bool allowed;

  _beacon_ack__isset __isset;

  void __set_time(const int64_t val);

  void __set_this_node(const  ::dsn::rpc_address& val);

  void __set_primary_node(const  ::dsn::rpc_address& val);

  void __set_is_master(const bool val);

  void __set_allowed(const bool val);

  bool operator == (const beacon_ack & rhs) const
  {
    if (!(time == rhs.time))
      return false;
    if (!(this_node == rhs.this_node))
      return false;
    if (!(primary_node == rhs.primary_node))
      return false;
    if (!(is_master == rhs.is_master))
      return false;
    if (!(allowed == rhs.allowed))
      return false;
    return true;
  }
  bool operator != (const beacon_ack &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const beacon_ack & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  virtual void printTo(std::ostream& out) const;
};

void swap(beacon_ack &a, beacon_ack &b);

inline std::ostream& operator<<(std::ostream& out, const beacon_ack& obj)
{
  obj.printTo(out);
  return out;
}

typedef struct _config_master_message__isset {
  _config_master_message__isset() : master(false), is_register(false) {}
  bool master :1;
  bool is_register :1;
} _config_master_message__isset;

class config_master_message {
 public:

  config_master_message(const config_master_message&);
  config_master_message& operator=(const config_master_message&);
  config_master_message() : is_register(0) {
  }

  virtual ~config_master_message() throw();
   ::dsn::rpc_address master;
  bool is_register;

  _config_master_message__isset __isset;

  void __set_master(const  ::dsn::rpc_address& val);

  void __set_is_register(const bool val);

  bool operator == (const config_master_message & rhs) const
  {
    if (!(master == rhs.master))
      return false;
    if (!(is_register == rhs.is_register))
      return false;
    return true;
  }
  bool operator != (const config_master_message &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const config_master_message & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  virtual void printTo(std::ostream& out) const;
};

void swap(config_master_message &a, config_master_message &b);

inline std::ostream& operator<<(std::ostream& out, const config_master_message& obj)
{
  obj.printTo(out);
  return out;
}

}} // namespace

#endif
