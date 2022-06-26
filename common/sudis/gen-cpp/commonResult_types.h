/**
 * Autogenerated by Thrift Compiler (0.9.1)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef commonResult_TYPES_H
#define commonResult_TYPES_H

#include <thrift/Thrift.h>
#include <thrift/TApplicationException.h>
#include <thrift/protocol/TProtocol.h>
#include <thrift/transport/TTransport.h>

#include <thrift/cxxfunctional.h>





class KeyValue {
 public:

  static const char* ascii_fingerprint; // = "07A9615F837F7D0A952B595DD3020972";
  static const uint8_t binary_fingerprint[16]; // = {0x07,0xA9,0x61,0x5F,0x83,0x7F,0x7D,0x0A,0x95,0x2B,0x59,0x5D,0xD3,0x02,0x09,0x72};

  KeyValue() : key(), value() {
  }

  virtual ~KeyValue() throw() {}

  std::string key;
  std::string value;

  void __set_key(const std::string& val) {
    key = val;
  }

  void __set_value(const std::string& val) {
    value = val;
  }

  bool operator == (const KeyValue & rhs) const
  {
    if (!(key == rhs.key))
      return false;
    if (!(value == rhs.value))
      return false;
    return true;
  }
  bool operator != (const KeyValue &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const KeyValue & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(KeyValue &a, KeyValue &b);

typedef struct _Result__isset {
  _Result__isset() : attributes(false) {}
  bool attributes;
} _Result__isset;

class Result {
 public:

  static const char* ascii_fingerprint; // = "9D6B6F48C320512C9C15AA1AE0AED099";
  static const uint8_t binary_fingerprint[16]; // = {0x9D,0x6B,0x6F,0x48,0xC3,0x20,0x51,0x2C,0x9C,0x15,0xAA,0x1A,0xE0,0xAE,0xD0,0x99};

  Result() : successful(0) {
  }

  virtual ~Result() throw() {}

  bool successful;
  std::vector<KeyValue>  attributes;

  _Result__isset __isset;

  void __set_successful(const bool val) {
    successful = val;
  }

  void __set_attributes(const std::vector<KeyValue> & val) {
    attributes = val;
    __isset.attributes = true;
  }

  bool operator == (const Result & rhs) const
  {
    if (!(successful == rhs.successful))
      return false;
    if (__isset.attributes != rhs.__isset.attributes)
      return false;
    else if (__isset.attributes && !(attributes == rhs.attributes))
      return false;
    return true;
  }
  bool operator != (const Result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(Result &a, Result &b);


class MessageIdentifier {
 public:

  static const char* ascii_fingerprint; // = "07A9615F837F7D0A952B595DD3020972";
  static const uint8_t binary_fingerprint[16]; // = {0x07,0xA9,0x61,0x5F,0x83,0x7F,0x7D,0x0A,0x95,0x2B,0x59,0x5D,0xD3,0x02,0x09,0x72};

  MessageIdentifier() : timestamp(), nonce() {
  }

  virtual ~MessageIdentifier() throw() {}

  std::string timestamp;
  std::string nonce;

  void __set_timestamp(const std::string& val) {
    timestamp = val;
  }

  void __set_nonce(const std::string& val) {
    nonce = val;
  }

  bool operator == (const MessageIdentifier & rhs) const
  {
    if (!(timestamp == rhs.timestamp))
      return false;
    if (!(nonce == rhs.nonce))
      return false;
    return true;
  }
  bool operator != (const MessageIdentifier &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const MessageIdentifier & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(MessageIdentifier &a, MessageIdentifier &b);



#endif