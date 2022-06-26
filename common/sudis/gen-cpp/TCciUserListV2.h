/**
 * Autogenerated by Thrift Compiler (0.9.1)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef TCciUserListV2_H
#define TCciUserListV2_H

#include <thrift/TDispatchProcessor.h>
#include "cciUserListV2_types.h"

namespace sudis {

class TCciUserListV2If {
 public:
  virtual ~TCciUserListV2If() {}
  virtual void cciUserListV2(TCciUserListV2Response& _return, const TCciUserListV2Request& request) = 0;
  virtual void cciUserListV2Echo1(TCciUserListV2Request& _return, const TCciUserListV2Request& request) = 0;
  virtual void cciUserListV2Echo2(TCciUserListV2Response& _return, const TCciUserListV2Response& response) = 0;
};

class TCciUserListV2IfFactory {
 public:
  typedef TCciUserListV2If Handler;

  virtual ~TCciUserListV2IfFactory() {}

  virtual TCciUserListV2If* getHandler(const ::apache::thrift::TConnectionInfo& connInfo) = 0;
  virtual void releaseHandler(TCciUserListV2If* /* handler */) = 0;
};

class TCciUserListV2IfSingletonFactory : virtual public TCciUserListV2IfFactory {
 public:
  TCciUserListV2IfSingletonFactory(const boost::shared_ptr<TCciUserListV2If>& iface) : iface_(iface) {}
  virtual ~TCciUserListV2IfSingletonFactory() {}

  virtual TCciUserListV2If* getHandler(const ::apache::thrift::TConnectionInfo&) {
    return iface_.get();
  }
  virtual void releaseHandler(TCciUserListV2If* /* handler */) {}

 protected:
  boost::shared_ptr<TCciUserListV2If> iface_;
};

class TCciUserListV2Null : virtual public TCciUserListV2If {
 public:
  virtual ~TCciUserListV2Null() {}
  void cciUserListV2(TCciUserListV2Response& /* _return */, const TCciUserListV2Request& /* request */) {
    return;
  }
  void cciUserListV2Echo1(TCciUserListV2Request& /* _return */, const TCciUserListV2Request& /* request */) {
    return;
  }
  void cciUserListV2Echo2(TCciUserListV2Response& /* _return */, const TCciUserListV2Response& /* response */) {
    return;
  }
};

typedef struct _TCciUserListV2_cciUserListV2_args__isset {
  _TCciUserListV2_cciUserListV2_args__isset() : request(false) {}
  bool request;
} _TCciUserListV2_cciUserListV2_args__isset;

class TCciUserListV2_cciUserListV2_args {
 public:

  TCciUserListV2_cciUserListV2_args() {
  }

  virtual ~TCciUserListV2_cciUserListV2_args() throw() {}

  TCciUserListV2Request request;

  _TCciUserListV2_cciUserListV2_args__isset __isset;

  void __set_request(const TCciUserListV2Request& val) {
    request = val;
  }

  bool operator == (const TCciUserListV2_cciUserListV2_args & rhs) const
  {
    if (!(request == rhs.request))
      return false;
    return true;
  }
  bool operator != (const TCciUserListV2_cciUserListV2_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const TCciUserListV2_cciUserListV2_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class TCciUserListV2_cciUserListV2_pargs {
 public:


  virtual ~TCciUserListV2_cciUserListV2_pargs() throw() {}

  const TCciUserListV2Request* request;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _TCciUserListV2_cciUserListV2_result__isset {
  _TCciUserListV2_cciUserListV2_result__isset() : success(false) {}
  bool success;
} _TCciUserListV2_cciUserListV2_result__isset;

class TCciUserListV2_cciUserListV2_result {
 public:

  TCciUserListV2_cciUserListV2_result() {
  }

  virtual ~TCciUserListV2_cciUserListV2_result() throw() {}

  TCciUserListV2Response success;

  _TCciUserListV2_cciUserListV2_result__isset __isset;

  void __set_success(const TCciUserListV2Response& val) {
    success = val;
  }

  bool operator == (const TCciUserListV2_cciUserListV2_result & rhs) const
  {
    if (!(success == rhs.success))
      return false;
    return true;
  }
  bool operator != (const TCciUserListV2_cciUserListV2_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const TCciUserListV2_cciUserListV2_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _TCciUserListV2_cciUserListV2_presult__isset {
  _TCciUserListV2_cciUserListV2_presult__isset() : success(false) {}
  bool success;
} _TCciUserListV2_cciUserListV2_presult__isset;

class TCciUserListV2_cciUserListV2_presult {
 public:


  virtual ~TCciUserListV2_cciUserListV2_presult() throw() {}

  TCciUserListV2Response* success;

  _TCciUserListV2_cciUserListV2_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

typedef struct _TCciUserListV2_cciUserListV2Echo1_args__isset {
  _TCciUserListV2_cciUserListV2Echo1_args__isset() : request(false) {}
  bool request;
} _TCciUserListV2_cciUserListV2Echo1_args__isset;

class TCciUserListV2_cciUserListV2Echo1_args {
 public:

  TCciUserListV2_cciUserListV2Echo1_args() {
  }

  virtual ~TCciUserListV2_cciUserListV2Echo1_args() throw() {}

  TCciUserListV2Request request;

  _TCciUserListV2_cciUserListV2Echo1_args__isset __isset;

  void __set_request(const TCciUserListV2Request& val) {
    request = val;
  }

  bool operator == (const TCciUserListV2_cciUserListV2Echo1_args & rhs) const
  {
    if (!(request == rhs.request))
      return false;
    return true;
  }
  bool operator != (const TCciUserListV2_cciUserListV2Echo1_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const TCciUserListV2_cciUserListV2Echo1_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class TCciUserListV2_cciUserListV2Echo1_pargs {
 public:


  virtual ~TCciUserListV2_cciUserListV2Echo1_pargs() throw() {}

  const TCciUserListV2Request* request;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _TCciUserListV2_cciUserListV2Echo1_result__isset {
  _TCciUserListV2_cciUserListV2Echo1_result__isset() : success(false) {}
  bool success;
} _TCciUserListV2_cciUserListV2Echo1_result__isset;

class TCciUserListV2_cciUserListV2Echo1_result {
 public:

  TCciUserListV2_cciUserListV2Echo1_result() {
  }

  virtual ~TCciUserListV2_cciUserListV2Echo1_result() throw() {}

  TCciUserListV2Request success;

  _TCciUserListV2_cciUserListV2Echo1_result__isset __isset;

  void __set_success(const TCciUserListV2Request& val) {
    success = val;
  }

  bool operator == (const TCciUserListV2_cciUserListV2Echo1_result & rhs) const
  {
    if (!(success == rhs.success))
      return false;
    return true;
  }
  bool operator != (const TCciUserListV2_cciUserListV2Echo1_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const TCciUserListV2_cciUserListV2Echo1_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _TCciUserListV2_cciUserListV2Echo1_presult__isset {
  _TCciUserListV2_cciUserListV2Echo1_presult__isset() : success(false) {}
  bool success;
} _TCciUserListV2_cciUserListV2Echo1_presult__isset;

class TCciUserListV2_cciUserListV2Echo1_presult {
 public:


  virtual ~TCciUserListV2_cciUserListV2Echo1_presult() throw() {}

  TCciUserListV2Request* success;

  _TCciUserListV2_cciUserListV2Echo1_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

typedef struct _TCciUserListV2_cciUserListV2Echo2_args__isset {
  _TCciUserListV2_cciUserListV2Echo2_args__isset() : response(false) {}
  bool response;
} _TCciUserListV2_cciUserListV2Echo2_args__isset;

class TCciUserListV2_cciUserListV2Echo2_args {
 public:

  TCciUserListV2_cciUserListV2Echo2_args() {
  }

  virtual ~TCciUserListV2_cciUserListV2Echo2_args() throw() {}

  TCciUserListV2Response response;

  _TCciUserListV2_cciUserListV2Echo2_args__isset __isset;

  void __set_response(const TCciUserListV2Response& val) {
    response = val;
  }

  bool operator == (const TCciUserListV2_cciUserListV2Echo2_args & rhs) const
  {
    if (!(response == rhs.response))
      return false;
    return true;
  }
  bool operator != (const TCciUserListV2_cciUserListV2Echo2_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const TCciUserListV2_cciUserListV2Echo2_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class TCciUserListV2_cciUserListV2Echo2_pargs {
 public:


  virtual ~TCciUserListV2_cciUserListV2Echo2_pargs() throw() {}

  const TCciUserListV2Response* response;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _TCciUserListV2_cciUserListV2Echo2_result__isset {
  _TCciUserListV2_cciUserListV2Echo2_result__isset() : success(false) {}
  bool success;
} _TCciUserListV2_cciUserListV2Echo2_result__isset;

class TCciUserListV2_cciUserListV2Echo2_result {
 public:

  TCciUserListV2_cciUserListV2Echo2_result() {
  }

  virtual ~TCciUserListV2_cciUserListV2Echo2_result() throw() {}

  TCciUserListV2Response success;

  _TCciUserListV2_cciUserListV2Echo2_result__isset __isset;

  void __set_success(const TCciUserListV2Response& val) {
    success = val;
  }

  bool operator == (const TCciUserListV2_cciUserListV2Echo2_result & rhs) const
  {
    if (!(success == rhs.success))
      return false;
    return true;
  }
  bool operator != (const TCciUserListV2_cciUserListV2Echo2_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const TCciUserListV2_cciUserListV2Echo2_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _TCciUserListV2_cciUserListV2Echo2_presult__isset {
  _TCciUserListV2_cciUserListV2Echo2_presult__isset() : success(false) {}
  bool success;
} _TCciUserListV2_cciUserListV2Echo2_presult__isset;

class TCciUserListV2_cciUserListV2Echo2_presult {
 public:


  virtual ~TCciUserListV2_cciUserListV2Echo2_presult() throw() {}

  TCciUserListV2Response* success;

  _TCciUserListV2_cciUserListV2Echo2_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

class TCciUserListV2Client : virtual public TCciUserListV2If {
 public:
  TCciUserListV2Client(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) :
    piprot_(prot),
    poprot_(prot) {
    iprot_ = prot.get();
    oprot_ = prot.get();
  }
  TCciUserListV2Client(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, boost::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) :
    piprot_(iprot),
    poprot_(oprot) {
    iprot_ = iprot.get();
    oprot_ = oprot.get();
  }
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> getInputProtocol() {
    return piprot_;
  }
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> getOutputProtocol() {
    return poprot_;
  }
  void cciUserListV2(TCciUserListV2Response& _return, const TCciUserListV2Request& request);
  void send_cciUserListV2(const TCciUserListV2Request& request);
  void recv_cciUserListV2(TCciUserListV2Response& _return);
  void cciUserListV2Echo1(TCciUserListV2Request& _return, const TCciUserListV2Request& request);
  void send_cciUserListV2Echo1(const TCciUserListV2Request& request);
  void recv_cciUserListV2Echo1(TCciUserListV2Request& _return);
  void cciUserListV2Echo2(TCciUserListV2Response& _return, const TCciUserListV2Response& response);
  void send_cciUserListV2Echo2(const TCciUserListV2Response& response);
  void recv_cciUserListV2Echo2(TCciUserListV2Response& _return);
 protected:
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> piprot_;
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> poprot_;
  ::apache::thrift::protocol::TProtocol* iprot_;
  ::apache::thrift::protocol::TProtocol* oprot_;
};

class TCciUserListV2Processor : public ::apache::thrift::TDispatchProcessor {
 protected:
  boost::shared_ptr<TCciUserListV2If> iface_;
  virtual bool dispatchCall(::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, const std::string& fname, int32_t seqid, void* callContext);
 private:
  typedef  void (TCciUserListV2Processor::*ProcessFunction)(int32_t, ::apache::thrift::protocol::TProtocol*, ::apache::thrift::protocol::TProtocol*, void*);
  typedef std::map<std::string, ProcessFunction> ProcessMap;
  ProcessMap processMap_;
  void process_cciUserListV2(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_cciUserListV2Echo1(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_cciUserListV2Echo2(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
 public:
  TCciUserListV2Processor(boost::shared_ptr<TCciUserListV2If> iface) :
    iface_(iface) {
    processMap_["cciUserListV2"] = &TCciUserListV2Processor::process_cciUserListV2;
    processMap_["cciUserListV2Echo1"] = &TCciUserListV2Processor::process_cciUserListV2Echo1;
    processMap_["cciUserListV2Echo2"] = &TCciUserListV2Processor::process_cciUserListV2Echo2;
  }

  virtual ~TCciUserListV2Processor() {}
};

class TCciUserListV2ProcessorFactory : public ::apache::thrift::TProcessorFactory {
 public:
  TCciUserListV2ProcessorFactory(const ::boost::shared_ptr< TCciUserListV2IfFactory >& handlerFactory) :
      handlerFactory_(handlerFactory) {}

  ::boost::shared_ptr< ::apache::thrift::TProcessor > getProcessor(const ::apache::thrift::TConnectionInfo& connInfo);

 protected:
  ::boost::shared_ptr< TCciUserListV2IfFactory > handlerFactory_;
};

class TCciUserListV2Multiface : virtual public TCciUserListV2If {
 public:
  TCciUserListV2Multiface(std::vector<boost::shared_ptr<TCciUserListV2If> >& ifaces) : ifaces_(ifaces) {
  }
  virtual ~TCciUserListV2Multiface() {}
 protected:
  std::vector<boost::shared_ptr<TCciUserListV2If> > ifaces_;
  TCciUserListV2Multiface() {}
  void add(boost::shared_ptr<TCciUserListV2If> iface) {
    ifaces_.push_back(iface);
  }
 public:
  void cciUserListV2(TCciUserListV2Response& _return, const TCciUserListV2Request& request) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->cciUserListV2(_return, request);
    }
    ifaces_[i]->cciUserListV2(_return, request);
    return;
  }

  void cciUserListV2Echo1(TCciUserListV2Request& _return, const TCciUserListV2Request& request) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->cciUserListV2Echo1(_return, request);
    }
    ifaces_[i]->cciUserListV2Echo1(_return, request);
    return;
  }

  void cciUserListV2Echo2(TCciUserListV2Response& _return, const TCciUserListV2Response& response) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->cciUserListV2Echo2(_return, response);
    }
    ifaces_[i]->cciUserListV2Echo2(_return, response);
    return;
  }

};

} // namespace

#endif
