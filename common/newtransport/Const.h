#pragma once

namespace transport {

static const unsigned c_version = 9;
static const unsigned c_version_min = 8;
static const unsigned c_version_old = 7;
static const unsigned c_ssl_support_mask = 0x10;

static const char c_ping_opcode       = 'p';
static const char c_connect_opcode    = 'c';
static const char c_disconnect_opcode = 'd';

static const unsigned c_ping_time_limit       = 5000;  // milliseconds
static const unsigned c_connect_time_limit    = 30000; // milliseconds
static const unsigned c_disconnect_time_limit = 30000; // milliseconds

}
