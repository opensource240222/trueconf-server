/**
 * Autogenerated by Thrift Compiler (0.9.1)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#include "cciUserLoginV1_types.h"

#include <algorithm>

namespace sudis {

const char* TCciUserLoginV1Request::ascii_fingerprint = "93A1111D3E071522253C5379C504D11B";
const uint8_t TCciUserLoginV1Request::binary_fingerprint[16] = {0x93,0xA1,0x11,0x1D,0x3E,0x07,0x15,0x22,0x25,0x3C,0x53,0x79,0xC5,0x04,0xD1,0x1B};

uint32_t TCciUserLoginV1Request::read(::apache::thrift::protocol::TProtocol* iprot) {

  uint32_t xfer = 0;
  std::string fname;
  ::apache::thrift::protocol::TType ftype;
  int16_t fid;

  xfer += iprot->readStructBegin(fname);

  using ::apache::thrift::protocol::TProtocolException;

  bool isset_requestDateTime = false;
  bool isset_requestNonce = false;
  bool isset_login = false;
  bool isset_password = false;

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
          isset_login = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 4:
        if (ftype == ::apache::thrift::protocol::T_STRING) {
          xfer += iprot->readString(this->password);
          isset_password = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 5:
        if (ftype == ::apache::thrift::protocol::T_STRING) {
          xfer += iprot->readString(this->spCode);
          this->__isset.spCode = true;
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
  if (!isset_login)
    throw TProtocolException(TProtocolException::INVALID_DATA);
  if (!isset_password)
    throw TProtocolException(TProtocolException::INVALID_DATA);
  return xfer;
}

uint32_t TCciUserLoginV1Request::write(::apache::thrift::protocol::TProtocol* oprot) const {
  uint32_t xfer = 0;
  xfer += oprot->writeStructBegin("TCciUserLoginV1Request");

  xfer += oprot->writeFieldBegin("requestDateTime", ::apache::thrift::protocol::T_STRING, 1);
  xfer += oprot->writeString(this->requestDateTime);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("requestNonce", ::apache::thrift::protocol::T_STRING, 2);
  xfer += oprot->writeBinary(this->requestNonce);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("login", ::apache::thrift::protocol::T_STRING, 3);
  xfer += oprot->writeString(this->login);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("password", ::apache::thrift::protocol::T_STRING, 4);
  xfer += oprot->writeString(this->password);
  xfer += oprot->writeFieldEnd();

  if (this->__isset.spCode) {
    xfer += oprot->writeFieldBegin("spCode", ::apache::thrift::protocol::T_STRING, 5);
    xfer += oprot->writeString(this->spCode);
    xfer += oprot->writeFieldEnd();
  }
  xfer += oprot->writeFieldStop();
  xfer += oprot->writeStructEnd();
  return xfer;
}

void swap(TCciUserLoginV1Request &a, TCciUserLoginV1Request &b) {
  using ::std::swap;
  swap(a.requestDateTime, b.requestDateTime);
  swap(a.requestNonce, b.requestNonce);
  swap(a.login, b.login);
  swap(a.password, b.password);
  swap(a.spCode, b.spCode);
  swap(a.__isset, b.__isset);
}

const char* TCciUserLoginV1Response::ascii_fingerprint = "566EA6FBDAAA5A3C9631E9B70D390B7D";
const uint8_t TCciUserLoginV1Response::binary_fingerprint[16] = {0x56,0x6E,0xA6,0xFB,0xDA,0xAA,0x5A,0x3C,0x96,0x31,0xE9,0xB7,0x0D,0x39,0x0B,0x7D};

uint32_t TCciUserLoginV1Response::read(::apache::thrift::protocol::TProtocol* iprot) {

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
        if (ftype == ::apache::thrift::protocol::T_STRING) {
          xfer += iprot->readString(this->userTokenId);
          this->__isset.userTokenId = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 5:
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

  if (!isset_responseDateTime)
    throw TProtocolException(TProtocolException::INVALID_DATA);
  if (!isset_responseNonce)
    throw TProtocolException(TProtocolException::INVALID_DATA);
  if (!isset_resultMessage)
    throw TProtocolException(TProtocolException::INVALID_DATA);
  return xfer;
}

uint32_t TCciUserLoginV1Response::write(::apache::thrift::protocol::TProtocol* oprot) const {
  uint32_t xfer = 0;
  xfer += oprot->writeStructBegin("TCciUserLoginV1Response");

  xfer += oprot->writeFieldBegin("responseDateTime", ::apache::thrift::protocol::T_STRING, 1);
  xfer += oprot->writeString(this->responseDateTime);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("responseNonce", ::apache::thrift::protocol::T_STRING, 2);
  xfer += oprot->writeBinary(this->responseNonce);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("resultMessage", ::apache::thrift::protocol::T_STRUCT, 3);
  xfer += this->resultMessage.write(oprot);
  xfer += oprot->writeFieldEnd();

  if (this->__isset.userTokenId) {
    xfer += oprot->writeFieldBegin("userTokenId", ::apache::thrift::protocol::T_STRING, 4);
    xfer += oprot->writeString(this->userTokenId);
    xfer += oprot->writeFieldEnd();
  }
  if (this->__isset.attributes) {
    xfer += oprot->writeFieldBegin("attributes", ::apache::thrift::protocol::T_LIST, 5);
    {
      xfer += oprot->writeListBegin(::apache::thrift::protocol::T_STRUCT, static_cast<uint32_t>(this->attributes.size()));
      std::vector< ::sudis::TCciUserAttributeV1> ::const_iterator _iter5;
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

void swap(TCciUserLoginV1Response &a, TCciUserLoginV1Response &b) {
  using ::std::swap;
  swap(a.responseDateTime, b.responseDateTime);
  swap(a.responseNonce, b.responseNonce);
  swap(a.resultMessage, b.resultMessage);
  swap(a.userTokenId, b.userTokenId);
  swap(a.attributes, b.attributes);
  swap(a.__isset, b.__isset);
}

} // namespace
