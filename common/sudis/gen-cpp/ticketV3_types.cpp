/**
 * Autogenerated by Thrift Compiler (0.9.1)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#include "ticketV3_types.h"

#include <algorithm>



const char* TicketV3Request::ascii_fingerprint = "A756D3DBE614FB13F70BF7F7B6EB3D73";
const uint8_t TicketV3Request::binary_fingerprint[16] = {0xA7,0x56,0xD3,0xDB,0xE6,0x14,0xFB,0x13,0xF7,0x0B,0xF7,0xF7,0xB6,0xEB,0x3D,0x73};

uint32_t TicketV3Request::read(::apache::thrift::protocol::TProtocol* iprot) {

  uint32_t xfer = 0;
  std::string fname;
  ::apache::thrift::protocol::TType ftype;
  int16_t fid;

  xfer += iprot->readStructBegin(fname);

  using ::apache::thrift::protocol::TProtocolException;

  bool isset_messageIdentifier = false;

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
      default:
        xfer += iprot->skip(ftype);
        break;
    }
    xfer += iprot->readFieldEnd();
  }

  xfer += iprot->readStructEnd();

  if (!isset_messageIdentifier)
    throw TProtocolException(TProtocolException::INVALID_DATA);
  return xfer;
}

uint32_t TicketV3Request::write(::apache::thrift::protocol::TProtocol* oprot) const {
  uint32_t xfer = 0;
  xfer += oprot->writeStructBegin("TicketV3Request");

  xfer += oprot->writeFieldBegin("messageIdentifier", ::apache::thrift::protocol::T_STRUCT, 1);
  xfer += this->messageIdentifier.write(oprot);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldStop();
  xfer += oprot->writeStructEnd();
  return xfer;
}

void swap(TicketV3Request &a, TicketV3Request &b) {
  using ::std::swap;
  swap(a.messageIdentifier, b.messageIdentifier);
}

const char* TicketV3Response::ascii_fingerprint = "B06764ADB7C15B97FD4C304ABD820216";
const uint8_t TicketV3Response::binary_fingerprint[16] = {0xB0,0x67,0x64,0xAD,0xB7,0xC1,0x5B,0x97,0xFD,0x4C,0x30,0x4A,0xBD,0x82,0x02,0x16};

uint32_t TicketV3Response::read(::apache::thrift::protocol::TProtocol* iprot) {

  uint32_t xfer = 0;
  std::string fname;
  ::apache::thrift::protocol::TType ftype;
  int16_t fid;

  xfer += iprot->readStructBegin(fname);

  using ::apache::thrift::protocol::TProtocolException;

  bool isset_messageIdentifier = false;
  bool isset_result = false;
  bool isset_ticketId = false;
  bool isset_ticketBody = false;

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
          xfer += iprot->readString(this->ticketId);
          isset_ticketId = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 4:
        if (ftype == ::apache::thrift::protocol::T_STRING) {
          xfer += iprot->readBinary(this->ticketBody);
          isset_ticketBody = true;
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
  if (!isset_ticketId)
    throw TProtocolException(TProtocolException::INVALID_DATA);
  if (!isset_ticketBody)
    throw TProtocolException(TProtocolException::INVALID_DATA);
  return xfer;
}

uint32_t TicketV3Response::write(::apache::thrift::protocol::TProtocol* oprot) const {
  uint32_t xfer = 0;
  xfer += oprot->writeStructBegin("TicketV3Response");

  xfer += oprot->writeFieldBegin("messageIdentifier", ::apache::thrift::protocol::T_STRUCT, 1);
  xfer += this->messageIdentifier.write(oprot);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("result", ::apache::thrift::protocol::T_STRUCT, 2);
  xfer += this->result.write(oprot);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("ticketId", ::apache::thrift::protocol::T_STRING, 3);
  xfer += oprot->writeString(this->ticketId);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("ticketBody", ::apache::thrift::protocol::T_STRING, 4);
  xfer += oprot->writeBinary(this->ticketBody);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldStop();
  xfer += oprot->writeStructEnd();
  return xfer;
}

void swap(TicketV3Response &a, TicketV3Response &b) {
  using ::std::swap;
  swap(a.messageIdentifier, b.messageIdentifier);
  swap(a.result, b.result);
  swap(a.ticketId, b.ticketId);
  swap(a.ticketBody, b.ticketBody);
}


