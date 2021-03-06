/**
 * Autogenerated by Thrift Compiler (0.9.1)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef spsbCommon_TYPES_H
#define spsbCommon_TYPES_H

#include <thrift/Thrift.h>
#include <thrift/TApplicationException.h>
#include <thrift/protocol/TProtocol.h>
#include <thrift/transport/TTransport.h>

#include <thrift/cxxfunctional.h>


namespace spsb {

struct TSpSbAttributeType {
  enum type {
    BOOLEAN = 1,
    INT = 2,
    DATE_TIME_ISO_8601 = 3,
    TEXT = 4,
    DOUBLE = 5,
    BYTES = 6,
    LONG = 7
  };
};

extern const std::map<int, const char*> _TSpSbAttributeType_VALUES_TO_NAMES;

typedef struct _TSpsbEventAttribute__isset {
  _TSpsbEventAttribute__isset() : code(true), description(true), type(true) {}
  bool code;
  bool description;
  bool type;
} _TSpsbEventAttribute__isset;

class TSpsbEventAttribute {
 public:

  static const char* ascii_fingerprint; // = "09A4D2F732F4A181FD44C543B235CF07";
  static const uint8_t binary_fingerprint[16]; // = {0x09,0xA4,0xD2,0xF7,0x32,0xF4,0xA1,0x81,0xFD,0x44,0xC5,0x43,0xB2,0x35,0xCF,0x07};

  TSpsbEventAttribute() : code("undefined"), description("undefined"), type((TSpSbAttributeType::type)4) {
    type = (TSpSbAttributeType::type)4;

  }

  virtual ~TSpsbEventAttribute() throw() {}

  std::string code;
  std::string description;
  TSpSbAttributeType::type type;

  _TSpsbEventAttribute__isset __isset;

  void __set_code(const std::string& val) {
    code = val;
    __isset.code = true;
  }

  void __set_description(const std::string& val) {
    description = val;
    __isset.description = true;
  }

  void __set_type(const TSpSbAttributeType::type val) {
    type = val;
    __isset.type = true;
  }

  bool operator == (const TSpsbEventAttribute & rhs) const
  {
    if (__isset.code != rhs.__isset.code)
      return false;
    else if (__isset.code && !(code == rhs.code))
      return false;
    if (__isset.description != rhs.__isset.description)
      return false;
    else if (__isset.description && !(description == rhs.description))
      return false;
    if (__isset.type != rhs.__isset.type)
      return false;
    else if (__isset.type && !(type == rhs.type))
      return false;
    return true;
  }
  bool operator != (const TSpsbEventAttribute &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const TSpsbEventAttribute & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(TSpsbEventAttribute &a, TSpsbEventAttribute &b);

typedef struct _TSpsbEventAttributeValue__isset {
  _TSpsbEventAttributeValue__isset() : code(true), value(true), type(true) {}
  bool code;
  bool value;
  bool type;
} _TSpsbEventAttributeValue__isset;

class TSpsbEventAttributeValue {
 public:

  static const char* ascii_fingerprint; // = "09A4D2F732F4A181FD44C543B235CF07";
  static const uint8_t binary_fingerprint[16]; // = {0x09,0xA4,0xD2,0xF7,0x32,0xF4,0xA1,0x81,0xFD,0x44,0xC5,0x43,0xB2,0x35,0xCF,0x07};

  TSpsbEventAttributeValue() : code("undefined"), value("undefined"), type((TSpSbAttributeType::type)4) {
    type = (TSpSbAttributeType::type)4;

  }

  virtual ~TSpsbEventAttributeValue() throw() {}

  std::string code;
  std::string value;
  TSpSbAttributeType::type type;

  _TSpsbEventAttributeValue__isset __isset;

  void __set_code(const std::string& val) {
    code = val;
    __isset.code = true;
  }

  void __set_value(const std::string& val) {
    value = val;
    __isset.value = true;
  }

  void __set_type(const TSpSbAttributeType::type val) {
    type = val;
    __isset.type = true;
  }

  bool operator == (const TSpsbEventAttributeValue & rhs) const
  {
    if (__isset.code != rhs.__isset.code)
      return false;
    else if (__isset.code && !(code == rhs.code))
      return false;
    if (__isset.value != rhs.__isset.value)
      return false;
    else if (__isset.value && !(value == rhs.value))
      return false;
    if (__isset.type != rhs.__isset.type)
      return false;
    else if (__isset.type && !(type == rhs.type))
      return false;
    return true;
  }
  bool operator != (const TSpsbEventAttributeValue &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const TSpsbEventAttributeValue & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(TSpsbEventAttributeValue &a, TSpsbEventAttributeValue &b);

} // namespace

#endif
