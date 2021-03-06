/**
 * Autogenerated by Thrift Compiler (0.9.1)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#include "commonResult_types.h"

#include <algorithm>



const char* KeyValue::ascii_fingerprint = "07A9615F837F7D0A952B595DD3020972";
const uint8_t KeyValue::binary_fingerprint[16] = {0x07,0xA9,0x61,0x5F,0x83,0x7F,0x7D,0x0A,0x95,0x2B,0x59,0x5D,0xD3,0x02,0x09,0x72};

uint32_t KeyValue::read(::apache::thrift::protocol::TProtocol* iprot) {

  uint32_t xfer = 0;
  std::string fname;
  ::apache::thrift::protocol::TType ftype;
  int16_t fid;

  xfer += iprot->readStructBegin(fname);

  using ::apache::thrift::protocol::TProtocolException;

  bool isset_key = false;
  bool isset_value = false;

  while (true)
  {
    xfer += iprot->readFieldBegin(fname, ftype, fid);
    if (ftype == ::apache::thrift::protocol::T_STOP) {
      break;
    }
    switch (fid)
    {
      case 1:
        if (ftype == ::apache::thrift::protocol::T_STRING) {
          xfer += iprot->readString(this->key);
          isset_key = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 2:
        if (ftype == ::apache::thrift::protocol::T_STRING) {
          xfer += iprot->readString(this->value);
          isset_value = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      default:
        xfer += iprot->skip(ftype);
        break;
    }
    xfer += iprot->readFieldEnd();
  }

  xfer += iprot->readStructEnd();

  if (!isset_key)
    throw TProtocolException(TProtocolException::INVALID_DATA);
  if (!isset_value)
    throw TProtocolException(TProtocolException::INVALID_DATA);
  return xfer;
}

uint32_t KeyValue::write(::apache::thrift::protocol::TProtocol* oprot) const {
  uint32_t xfer = 0;
  xfer += oprot->writeStructBegin("KeyValue");

  xfer += oprot->writeFieldBegin("key", ::apache::thrift::protocol::T_STRING, 1);
  xfer += oprot->writeString(this->key);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("value", ::apache::thrift::protocol::T_STRING, 2);
  xfer += oprot->writeString(this->value);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldStop();
  xfer += oprot->writeStructEnd();
  return xfer;
}

void swap(KeyValue &a, KeyValue &b) {
  using ::std::swap;
  swap(a.key, b.key);
  swap(a.value, b.value);
}

const char* Result::ascii_fingerprint = "9D6B6F48C320512C9C15AA1AE0AED099";
const uint8_t Result::binary_fingerprint[16] = {0x9D,0x6B,0x6F,0x48,0xC3,0x20,0x51,0x2C,0x9C,0x15,0xAA,0x1A,0xE0,0xAE,0xD0,0x99};

uint32_t Result::read(::apache::thrift::protocol::TProtocol* iprot) {

  uint32_t xfer = 0;
  std::string fname;
  ::apache::thrift::protocol::TType ftype;
  int16_t fid;

  xfer += iprot->readStructBegin(fname);

  using ::apache::thrift::protocol::TProtocolException;

  bool isset_successful = false;

  while (true)
  {
    xfer += iprot->readFieldBegin(fname, ftype, fid);
    if (ftype == ::apache::thrift::protocol::T_STOP) {
      break;
    }
    switch (fid)
    {
      case 1:
        if (ftype == ::apache::thrift::protocol::T_BOOL) {
          xfer += iprot->readBool(this->successful);
          isset_successful = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 2:
        if (ftype == ::apache::thrift::protocol::T_LIST) {
          {
            this->attributes.clear();
            uint32_t _size0;
            ::apache::thrift::protocol::TType _etype3;
            xfer += iprot->readListBegin(_etype3, _size0);
            this->attributes.resize(_size0);
            uint32_t _i4;
            for (_i4 = 0; _i4 < _size0; ++_i4)
            {
              xfer += this->attributes[_i4].read(iprot);
            }
            xfer += iprot->readListEnd();
          }
          this->__isset.attributes = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      default:
        xfer += iprot->skip(ftype);
        break;
    }
    xfer += iprot->readFieldEnd();
  }

  xfer += iprot->readStructEnd();

  if (!isset_successful)
    throw TProtocolException(TProtocolException::INVALID_DATA);
  return xfer;
}

uint32_t Result::write(::apache::thrift::protocol::TProtocol* oprot) const {
  uint32_t xfer = 0;
  xfer += oprot->writeStructBegin("Result");

  xfer += oprot->writeFieldBegin("successful", ::apache::thrift::protocol::T_BOOL, 1);
  xfer += oprot->writeBool(this->successful);
  xfer += oprot->writeFieldEnd();

  if (this->__isset.attributes) {
    xfer += oprot->writeFieldBegin("attributes", ::apache::thrift::protocol::T_LIST, 2);
    {
      xfer += oprot->writeListBegin(::apache::thrift::protocol::T_STRUCT, static_cast<uint32_t>(this->attributes.size()));
      std::vector<KeyValue> ::const_iterator _iter5;
      for (_iter5 = this->attributes.begin(); _iter5 != this->attributes.end(); ++_iter5)
      {
        xfer += (*_iter5).write(oprot);
      }
      xfer += oprot->writeListEnd();
    }
    xfer += oprot->writeFieldEnd();
  }
  xfer += oprot->writeFieldStop();
  xfer += oprot->writeStructEnd();
  return xfer;
}

void swap(Result &a, Result &b) {
  using ::std::swap;
  swap(a.successful, b.successful);
  swap(a.attributes, b.attributes);
  swap(a.__isset, b.__isset);
}

const char* MessageIdentifier::ascii_fingerprint = "07A9615F837F7D0A952B595DD3020972";
const uint8_t MessageIdentifier::binary_fingerprint[16] = {0x07,0xA9,0x61,0x5F,0x83,0x7F,0x7D,0x0A,0x95,0x2B,0x59,0x5D,0xD3,0x02,0x09,0x72};

uint32_t MessageIdentifier::read(::apache::thrift::protocol::TProtocol* iprot) {

  uint32_t xfer = 0;
  std::string fname;
  ::apache::thrift::protocol::TType ftype;
  int16_t fid;

  xfer += iprot->readStructBegin(fname);

  using ::apache::thrift::protocol::TProtocolException;

  bool isset_timestamp = false;
  bool isset_nonce = false;

  while (true)
  {
    xfer += iprot->readFieldBegin(fname, ftype, fid);
    if (ftype == ::apache::thrift::protocol::T_STOP) {
      break;
    }
    switch (fid)
    {
      case 1:
        if (ftype == ::apache::thrift::protocol::T_STRING) {
          xfer += iprot->readString(this->timestamp);
          isset_timestamp = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 2:
        if (ftype == ::apache::thrift::protocol::T_STRING) {
          xfer += iprot->readBinary(this->nonce);
          isset_nonce = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      default:
        xfer += iprot->skip(ftype);
        break;
    }
    xfer += iprot->readFieldEnd();
  }

  xfer += iprot->readStructEnd();

  if (!isset_timestamp)
    throw TProtocolException(TProtocolException::INVALID_DATA);
  if (!isset_nonce)
    throw TProtocolException(TProtocolException::INVALID_DATA);
  return xfer;
}

uint32_t MessageIdentifier::write(::apache::thrift::protocol::TProtocol* oprot) const {
  uint32_t xfer = 0;
  xfer += oprot->writeStructBegin("MessageIdentifier");

  xfer += oprot->writeFieldBegin("timestamp", ::apache::thrift::protocol::T_STRING, 1);
  xfer += oprot->writeString(this->timestamp);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("nonce", ::apache::thrift::protocol::T_STRING, 2);
  xfer += oprot->writeBinary(this->nonce);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldStop();
  xfer += oprot->writeStructEnd();
  return xfer;
}

void swap(MessageIdentifier &a, MessageIdentifier &b) {
  using ::std::swap;
  swap(a.timestamp, b.timestamp);
  swap(a.nonce, b.nonce);
}


