/**
 * Autogenerated by Thrift Compiler (0.9.1)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#include "cciUserListV2_types.h"

#include <algorithm>

namespace sudis {

const char* TCciUserListV2Request::ascii_fingerprint = "FED0FBEAA0C90D1589E8B650561B7675";
const uint8_t TCciUserListV2Request::binary_fingerprint[16] = {0xFE,0xD0,0xFB,0xEA,0xA0,0xC9,0x0D,0x15,0x89,0xE8,0xB6,0x50,0x56,0x1B,0x76,0x75};

uint32_t TCciUserListV2Request::read(::apache::thrift::protocol::TProtocol* iprot) {

  uint32_t xfer = 0;
  std::string fname;
  ::apache::thrift::protocol::TType ftype;
  int16_t fid;

  xfer += iprot->readStructBegin(fname);

  using ::apache::thrift::protocol::TProtocolException;

  bool isset_requestDateTime = false;
  bool isset_requestNonce = false;

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
          xfer += iprot->readString(this->requestDateTime);
          isset_requestDateTime = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 2:
        if (ftype == ::apache::thrift::protocol::T_STRING) {
          xfer += iprot->readBinary(this->requestNonce);
          isset_requestNonce = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 3:
        if (ftype == ::apache::thrift::protocol::T_STRING) {
          xfer += iprot->readString(this->login);
          this->__isset.login = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 4:
        if (ftype == ::apache::thrift::protocol::T_STRING) {
          xfer += iprot->readString(this->fio);
          this->__isset.fio = true;
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

  if (!isset_requestDateTime)
    throw TProtocolException(TProtocolException::INVALID_DATA);
  if (!isset_requestNonce)
    throw TProtocolException(TProtocolException::INVALID_DATA);
  return xfer;
}

uint32_t TCciUserListV2Request::write(::apache::thrift::protocol::TProtocol* oprot) const {
  uint32_t xfer = 0;
  xfer += oprot->writeStructBegin("TCciUserListV2Request");

  xfer += oprot->writeFieldBegin("requestDateTime", ::apache::thrift::protocol::T_STRING, 1);
  xfer += oprot->writeString(this->requestDateTime);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("requestNonce", ::apache::thrift::protocol::T_STRING, 2);
  xfer += oprot->writeBinary(this->requestNonce);
  xfer += oprot->writeFieldEnd();

  if (this->__isset.login) {
    xfer += oprot->writeFieldBegin("login", ::apache::thrift::protocol::T_STRING, 3);
    xfer += oprot->writeString(this->login);
    xfer += oprot->writeFieldEnd();
  }
  if (this->__isset.fio) {
    xfer += oprot->writeFieldBegin("fio", ::apache::thrift::protocol::T_STRING, 4);
    xfer += oprot->writeString(this->fio);
    xfer += oprot->writeFieldEnd();
  }
  xfer += oprot->writeFieldStop();
  xfer += oprot->writeStructEnd();
  return xfer;
}

void swap(TCciUserListV2Request &a, TCciUserListV2Request &b) {
  using ::std::swap;
  swap(a.requestDateTime, b.requestDateTime);
  swap(a.requestNonce, b.requestNonce);
  swap(a.login, b.login);
  swap(a.fio, b.fio);
  swap(a.__isset, b.__isset);
}

const char* TCciUserListV2Response::ascii_fingerprint = "4BEC7B9E1C1CDD96248174F9D2F41C35";
const uint8_t TCciUserListV2Response::binary_fingerprint[16] = {0x4B,0xEC,0x7B,0x9E,0x1C,0x1C,0xDD,0x96,0x24,0x81,0x74,0xF9,0xD2,0xF4,0x1C,0x35};

uint32_t TCciUserListV2Response::read(::apache::thrift::protocol::TProtocol* iprot) {

  uint32_t xfer = 0;
  std::string fname;
  ::apache::thrift::protocol::TType ftype;
  int16_t fid;

  xfer += iprot->readStructBegin(fname);

  using ::apache::thrift::protocol::TProtocolException;

  bool isset_responseDateTime = false;
  bool isset_responseNonce = false;
  bool isset_resultMessage = false;

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
          xfer += iprot->readString(this->responseDateTime);
          isset_responseDateTime = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 2:
        if (ftype == ::apache::thrift::protocol::T_STRING) {
          xfer += iprot->readBinary(this->responseNonce);
          isset_responseNonce = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 3:
        if (ftype == ::apache::thrift::protocol::T_STRUCT) {
          xfer += this->resultMessage.read(iprot);
          isset_resultMessage = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 4:
        if (ftype == ::apache::thrift::protocol::T_LIST) {
          {
            this->userList.clear();
            uint32_t _size0;
            ::apache::thrift::protocol::TType _etype3;
            xfer += iprot->readListBegin(_etype3, _size0);
            this->userList.resize(_size0);
            uint32_t _i4;
            for (_i4 = 0; _i4 < _size0; ++_i4)
            {
              xfer += this->userList[_i4].read(iprot);
            }
            xfer += iprot->readListEnd();
          }
          this->__isset.userList = true;
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

  if (!isset_responseDateTime)
    throw TProtocolException(TProtocolException::INVALID_DATA);
  if (!isset_responseNonce)
    throw TProtocolException(TProtocolException::INVALID_DATA);
  if (!isset_resultMessage)
    throw TProtocolException(TProtocolException::INVALID_DATA);
  return xfer;
}

uint32_t TCciUserListV2Response::write(::apache::thrift::protocol::TProtocol* oprot) const {
  uint32_t xfer = 0;
  xfer += oprot->writeStructBegin("TCciUserListV2Response");

  xfer += oprot->writeFieldBegin("responseDateTime", ::apache::thrift::protocol::T_STRING, 1);
  xfer += oprot->writeString(this->responseDateTime);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("responseNonce", ::apache::thrift::protocol::T_STRING, 2);
  xfer += oprot->writeBinary(this->responseNonce);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("resultMessage", ::apache::thrift::protocol::T_STRUCT, 3);
  xfer += this->resultMessage.write(oprot);
  xfer += oprot->writeFieldEnd();

  if (this->__isset.userList) {
    xfer += oprot->writeFieldBegin("userList", ::apache::thrift::protocol::T_LIST, 4);
    {
      xfer += oprot->writeListBegin(::apache::thrift::protocol::T_STRUCT, static_cast<uint32_t>(this->userList.size()));
      std::vector< ::sudis::TCciUserV1> ::const_iterator _iter5;
      for (_iter5 = this->userList.begin(); _iter5 != this->userList.end(); ++_iter5)
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

void swap(TCciUserListV2Response &a, TCciUserListV2Response &b) {
  using ::std::swap;
  swap(a.responseDateTime, b.responseDateTime);
  swap(a.responseNonce, b.responseNonce);
  swap(a.resultMessage, b.resultMessage);
  swap(a.userList, b.userList);
  swap(a.__isset, b.__isset);
}

} // namespace