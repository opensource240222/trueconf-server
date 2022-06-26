/**
 * Autogenerated by Thrift Compiler (0.9.1)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef cciUserCommonV1_TYPES_H
#define cciUserCommonV1_TYPES_H

#include <thrift/Thrift.h>
#include <thrift/TApplicationException.h>
#include <thrift/protocol/TProtocol.h>
#include <thrift/transport/TTransport.h>

#include <thrift/cxxfunctional.h>


namespace sudis {

typedef struct _TCciUserV1__isset {
  _TCciUserV1__isset() : secondName(false), login(false), birthDate(false), position(false), department(false), isBlocked(false) {}
  bool secondName;
  bool login;
  bool birthDate;
  bool position;
  bool department;
  bool isBlocked;
} _TCciUserV1__isset;

class TCciUserV1 {
 public:

  static const char* ascii_fingerprint; // = "3B164AD3E007FCC6EFEE33DFC4975B96";
  static const uint8_t binary_fingerprint[16]; // = {0x3B,0x16,0x4A,0xD3,0xE0,0x07,0xFC,0xC6,0xEF,0xEE,0x33,0xDF,0xC4,0x97,0x5B,0x96};

  TCciUserV1() : firstName(), secondName(), lastName(), login(), birthDate(), position(), department(), isBlocked(0) {
  }

  virtual ~TCciUserV1() throw() {}

  std::string firstName;
  std::string secondName;
  std::string lastName;
  std::string login;
  std::string birthDate;
  std::string position;
  std::string department;
  bool isBlocked;

  _TCciUserV1__isset __isset;

  void __set_firstName(const std::string& val) {
    firstName = val;
  }

  void __set_secondName(const std::string& val) {
    secondName = val;
    __isset.secondName = true;
  }

  void __set_lastName(const std::string& val) {
    lastName = val;
  }

  void __set_login(const std::string& val) {
    login = val;
    __isset.login = true;
  }

  void __set_birthDate(const std::string& val) {
    birthDate = val;
    __isset.birthDate = true;
  }

  void __set_position(const std::string& val) {
    position = val;
    __isset.position = true;
  }

  void __set_department(const std::string& val) {
    department = val;
    __isset.department = true;
  }

  void __set_isBlocked(const bool val) {
    isBlocked = val;
    __isset.isBlocked = true;
  }

  bool operator == (const TCciUserV1 & rhs) const
  {
    if (!(firstName == rhs.firstName))
      return false;
    if (__isset.secondName != rhs.__isset.secondName)
      return false;
    else if (__isset.secondName && !(secondName == rhs.secondName))
      return false;
    if (!(lastName == rhs.lastName))
      return false;
    if (__isset.login != rhs.__isset.login)
      return false;
    else if (__isset.login && !(login == rhs.login))
      return false;
    if (__isset.birthDate != rhs.__isset.birthDate)
      return false;
    else if (__isset.birthDate && !(birthDate == rhs.birthDate))
      return false;
    if (__isset.position != rhs.__isset.position)
      return false;
    else if (__isset.position && !(position == rhs.position))
      return false;
    if (__isset.department != rhs.__isset.department)
      return false;
    else if (__isset.department && !(department == rhs.department))
      return false;
    if (__isset.isBlocked != rhs.__isset.isBlocked)
      return false;
    else if (__isset.isBlocked && !(isBlocked == rhs.isBlocked))
      return false;
    return true;
  }
  bool operator != (const TCciUserV1 &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const TCciUserV1 & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(TCciUserV1 &a, TCciUserV1 &b);

} // namespace

#endif