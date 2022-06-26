#pragma once

#include "net/Handshake.h"

#define VS_CIRCUIT_MIN_VERSION 1

extern const char VS_Circuit_PrimaryField[net::HandshakeHeader::primary_field_size];
extern const char VS_FrameTransmit_PrimaryField[net::HandshakeHeader::primary_field_size];
