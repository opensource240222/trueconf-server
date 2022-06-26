/**
 * Autogenerated by Thrift Compiler (0.9.1)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef cciUserLogoutV1_TYPES_H
#define cciUserLogoutV1_TYPES_H

#include <thrift/Thrift.h>
#include <thrift/TApplicationException.h>
#include <thrift/protocol/TProtocol.h>
#include <thrift/transport/TTransport.h>

#include <thrift/cxxfunctional.h>
#include "cciCommonV1_types.h"


namespace sudis {


class TCciUserLogoutV1Request {
 public:

  static const char* ascii_fingerprint; // = "AB879940BD15B6B25691265F7384B271";
  static const uint8_t binary_fingerprint[16]; // = {0xAB,0x87,0x99,0x40,0xBD,0x15,0xB6,0xB2,0x56,0x91,0x26,0x5F,0x73,0x84,0xB2,0x71};

  TCciUserLogoutV1Request() : requestDateTime(), requestNonce(), userTokenId() {
  }

  virtual ~TCciUserLogoutV1Request() throw() {}

  std::string requestDateTime;
  std::string requestNonce;
  std::string userTokenId;

  void __set_requestDateTime(const std::string& val) {
    requestDateTime = val;
  }

  void __set_requestNonce(const std::string& val) {
    requestNonce = val;
  }

  void __set_userTokenId(const std::string& val) {
    userTokenId = val;
  }

  bool operator == (const TCciUserLogoutV1Request & rhs) const
  {
    if (!(requestDateTime == rhs.requestDateTime))
      return false;
    if (!(requestNonce == rhs.requestNonce))
      return false;
    if (!(userTokenId == rhs.userTokenId))
      return false;
    return true;
  }
  bool operator != (const TCciUserLogoutV1Request &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const TCciUserLogoutV1Request & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(TCciUserLogoutV1Request &a, TCciUserLogoutV1Request &b);


class TCciUserLogoutV1Response {
 public:

  static const char* ascii_fingerprint; // = "1AF705D5F455F35C4A25DA3E505892A5";
  static const uint8_t binary_fingerprint[16]; // = {0x1A,0xF7,0x05,0xD5,0xF4,0x55,0xF3,0x5C,0x4A,0x25,0xDA,0x3E,0x50,0x58,0x92,0xA5};

  TCciUserLogoutV1Response() : responseDateTime(), responseNonce() {
  }

  virtual ~TCciUserLogoutV1Response() throw() {}

  std::string responseDateTime;
  std::string responseNonce;
   ::sudis::TCciResultMessage resultMessage;

  void __set_responseDateTime(const std::string& val) {
    responseDateTime = val;
  }

  void __set_responseNonce(const std::string& val) {
    responseNonce = val;
  }

  void __set_resultMessage(const  ::sudis::TCciResultMessage& val) {
    resultMessage = val;
  }

  bool operator == (const TCciUserLogoutV1Response & rhs) const
  {
    if (!(responseDateTime == rhs.responseDateTime))
      return false;
    if (!(responseNonce == rhs.responseNonce))
      return false;
    if (!(resultMessage == rhs.resultMessage))
      return false;
    return true;
  }
  bool operator != (const TCciUserLogoutV1Response &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const TCciUserLogoutV1Response & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(TCciUserLogoutV1Response &a, TCciUserLogoutV1Response &b);

} // namespace

#endif