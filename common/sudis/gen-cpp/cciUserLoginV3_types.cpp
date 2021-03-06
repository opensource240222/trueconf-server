/**
 * Autogenerated by Thrift Compiler (0.9.1)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#include "cciUserLoginV3_types.h"

#include <algorithm>



const char* TCciUserAttributeV3::ascii_fingerprint = "09A67A266242E872217E8BB1F6E483B3";
const uint8_t TCciUserAttributeV3::binary_fingerprint[16] = {0x09,0xA6,0x7A,0x26,0x62,0x42,0xE8,0x72,0x21,0x7E,0x8B,0xB1,0xF6,0xE4,0x83,0xB3};

uint32_t TCciUserAttributeV3::read(::apache::thrift::protocol::TProtocol* iprot) {

  uint32_t xfer = 0;
  std::string fname;
  ::apache::thrift::protocol::TType ftype;
  int16_t fid;

  xfer += iprot->readStructBegin(fname);

  using ::apache::thrift::protocol::TProtocolException;

  bool isset_name = false;

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
          xfer += iprot->readString(this->name);
          isset_name = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 2:
        if (ftype == ::apache::thrift::protocol::T_LIST) {
          {
            this->values.clear();
            uint32_t _size0;
            ::apache::thrift::protocol::TType _etype3;
            xfer += iprot->readListBegin(_etype3, _size0);
            this->values.resize(_size0);
            uint32_t _i4;
            for (_i4 = 0; _i4 < _size0; ++_i4)
            {
              xfer += iprot->readString(this->values[_i4]);
            }
            xfer += iprot->readListEnd();
          }
          this->__isset.values = true;
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

  if (!isset_name)
    throw TProtocolException(TProtocolException::INVALID_DATA);
  return xfer;
}

uint32_t TCciUserAttributeV3::write(::apache::thrift::protocol::TProtocol* oprot) const {
  uint32_t xfer = 0;
  xfer += oprot->writeStructBegin("TCciUserAttributeV3");

  xfer += oprot->writeFieldBegin("name", ::apache::thrift::protocol::T_STRING, 1);
  xfer += oprot->writeString(this->name);
  xfer += oprot->writeFieldEnd();

  if (this->__isset.values) {
    xfer += oprot->writeFieldBegin("values", ::apache::thrift::protocol::T_LIST, 2);
    {
      xfer += oprot->writeListBegin(::apache::thrift::protocol::T_STRING, static_cast<uint32_t>(this->values.size()));
      std::vector<std::string> ::const_iterator _iter5;
      for (_iter5 = this->values.begin(); _iter5 != this->values.end(); ++_iter5)
      {
        xfer += oprot->writeString((*_iter5));
      }
      xfer += oprot->writeListEnd();
    }
    xfer += oprot->writeFieldEnd();
  }
  xfer += oprot->writeFieldStop();
  xfer += oprot->writeStructEnd();
  return xfer;
}

void swap(TCciUserAttributeV3 &a, TCciUserAttributeV3 &b) {
  using ::std::swap;
  swap(a.name, b.name);
  swap(a.values, b.values);
  swap(a.__isset, b.__isset);
}

const char* UserLoginV3Request::ascii_fingerprint = "2C2B1E52F2C21E880657F61DE06A92A5";
const uint8_t UserLoginV3Request::binary_fingerprint[16] = {0x2C,0x2B,0x1E,0x52,0xF2,0xC2,0x1E,0x88,0x06,0x57,0xF6,0x1D,0xE0,0x6A,0x92,0xA5};

uint32_t UserLoginV3Request::read(::apache::thrift::protocol::TProtocol* iprot) {

  uint32_t xfer = 0;
  std::string fname;
  ::apache::thrift::protocol::TType ftype;
  int16_t fid;

  xfer += iprot->readStructBegin(fname);

  using ::apache::thrift::protocol::TProtocolException;

  bool isset_messageIdentifier = false;
  bool isset_ticketId = false;
  bool isset_signedTicketBody = false;

  while (true)
  {
    xfer += iprot->readFieldBegin(fname, ftype, fid);
    if (ftype == ::apache::thrift::protocol::T_STOP) {
      break;
    }
    switch (fid)
    {
      case 1:
        if (ftype == ::apache::thrift::protocol::T_STRUCT) {
          xfer += this->messageIdentifier.read(iprot);
          isset_messageIdentifier = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 2:
        if (ftype == ::apache::thrift::protocol::T_STRING) {
          xfer += iprot->readString(this->ticketId);
          isset_ticketId = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 3:
        if (ftype == ::apache::thrift::protocol::T_STRING) {
          xfer += iprot->readBinary(this->signedTicketBody);
          isset_signedTicketBody = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 4:
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

  if (!isset_messageIdentifier)
    throw TProtocolException(TProtocolException::INVALID_DATA);
  if (!isset_ticketId)
    throw TProtocolException(TProtocolException::INVALID_DATA);
  if (!isset_signedTicketBody)
    throw TProtocolException(TProtocolException::INVALID_DATA);
  return xfer;
}

uint32_t UserLoginV3Request::write(::apache::thrift::protocol::TProtocol* oprot) const {
  uint32_t xfer = 0;
  xfer += oprot->writeStructBegin("UserLoginV3Request");

  xfer += oprot->writeFieldBegin("messageIdentifier", ::apache::thrift::protocol::T_STRUCT, 1);
  xfer += this->messageIdentifier.write(oprot);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("ticketId", ::apache::thrift::protocol::T_STRING, 2);
  xfer += oprot->writeString(this->ticketId);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("signedTicketBody", ::apache::thrift::protocol::T_STRING, 3);
  xfer += oprot->writeBinary(this->signedTicketBody);
  xfer += oprot->writeFieldEnd();

  if (this->__isset.spCode) {
    xfer += oprot->writeFieldBegin("spCode", ::apache::thrift::protocol::T_STRING, 4);
    xfer += oprot->writeString(this->spCode);
    xfer += oprot->writeFieldEnd();
  }
  xfer += oprot->writeFieldStop();
  xfer += oprot->writeStructEnd();
  return xfer;
}

void swap(UserLoginV3Request &a, UserLoginV3Request &b) {
  using ::std::swap;
  swap(a.messageIdentifier, b.messageIdentifier);
  swap(a.ticketId, b.ticketId);
  swap(a.signedTicketBody, b.signedTicketBody);
  swap(a.spCode, b.spCode);
  swap(a.__isset, b.__isset);
}

const char* UserLoginV3Response::ascii_fingerprint = "6506B68E78C2FF1104D42C94EB1C2AFC";
const uint8_t UserLoginV3Response::binary_fingerprint[16] = {0x65,0x06,0xB6,0x8E,0x78,0xC2,0xFF,0x11,0x04,0xD4,0x2C,0x94,0xEB,0x1C,0x2A,0xFC};

uint32_t UserLoginV3Response::read(::apache::thrift::protocol::TProtocol* iprot) {

  uint32_t xfer = 0;
  std::string fname;
  ::apache::thrift::protocol::TType ftype;
  int16_t fid;

  xfer += iprot->readStructBegin(fname);

  using ::apache::thrift::protocol::TProtocolException;

  bool isset_messageIdentifier = false;
  bool isset_result = false;
  bool isset_userTokenId = false;
  bool isset_attributes = false;

  while (true)
  {
    xfer += iprot->readFieldBegin(fname, ftype, fid);
    if (ftype == ::apache::thrift::protocol::T_STOP) {
      break;
    }
    switch (fid)
    {
      case 1:
        if (ftype == ::apache::thrift::protocol::T_STRUCT) {
          xfer += this->messageIdentifier.read(iprot);
          isset_messageIdentifier = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 2:
        if (ftype == ::apache::thrift::protocol::T_STRUCT) {
          xfer += this->result.read(iprot);
          isset_result = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 3:
        if (ftype == ::apache::thrift::protocol::T_STRING) {
          xfer += iprot->readString(this->userTokenId);
          isset_userTokenId = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 4:
        if (ftype == ::apache::thrift::protocol::T_LIST) {
          {
            this->attributes.clear();
            uint32_t _size6;
            ::apache::thrift::protocol::TType _etype9;
            xfer += iprot->readListBegin(_etype9, _size6);
            this->attributes.resize(_size6);
            uint32_t _i10;
            for (_i10 = 0; _i10 < _size6; ++_i10)
            {
              xfer += this->attributes[_i10].read(iprot);
            }
            xfer += iprot->readListEnd();
          }
          isset_attributes = true;
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

  if (!isset_messageIdentifier)
    throw TProtocolException(TProtocolException::INVALID_DATA);
  if (!isset_result)
    throw TProtocolException(TProtocolException::INVALID_DATA);
  if (!isset_userTokenId)
    throw TProtocolException(TProtocolException::INVALID_DATA);
  if (!isset_attributes)
    throw TProtocolException(TProtocolException::INVALID_DATA);
  return xfer;
}

uint32_t UserLoginV3Response::write(::apache::thrift::protocol::TProtocol* oprot) const {
  uint32_t xfer = 0;
  xfer += oprot->writeStructBegin("UserLoginV3Response");

  xfer += oprot->writeFieldBegin("messageIdentifier", ::apache::thrift::protocol::T_STRUCT, 1);
  xfer += this->messageIdentifier.write(oprot);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("result", ::apache::thrift::protocol::T_STRUCT, 2);
  xfer += this->result.write(oprot);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("userTokenId", ::apache::thrift::protocol::T_STRING, 3);
  xfer += oprot->writeString(this->userTokenId);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("attributes", ::apache::thrift::protocol::T_LIST, 4);
  {
    xfer += oprot->writeListBegin(::apache::thrift::protocol::T_STRUCT, static_cast<uint32_t>(this->attributes.size()));
    std::vector<TCciUserAttributeV3> ::const_iterator _iter11;
    for (_iter11 = this->attributes.begin(); _iter11 != this->attributes.end(); ++_iter11)
    {
      xfer += (*_iter11).write(oprot);
    }
    xfer += oprot->writeListEnd();
  }
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldStop();
  xfer += oprot->writeStructEnd();
  return xfer;
}

void swap(UserLoginV3Response &a, UserLoginV3Response &b) {
  using ::std::swap;
  swap(a.messageIdentifier, b.messageIdentifier);
  swap(a.result, b.result);
  swap(a.userTokenId, b.userTokenId);
  swap(a.attributes, b.attributes);
}


