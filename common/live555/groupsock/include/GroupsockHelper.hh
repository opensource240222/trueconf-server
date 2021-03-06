/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// "mTunnel" multicast access service
// Copyright (c) 1996-2015 Live Networks, Inc.  All rights reserved.
// Helper routines to implement 'group sockets'
// C++ header

#ifndef _GROUPSOCK_HELPER_HH
#define _GROUPSOCK_HELPER_HH

#ifndef _LIVE_GLOBALS_HH
#include "LiveGlobals.hh"
#endif

#ifndef _NET_ADDRESS_HH
#include "NetAddress.hh"
#endif

LIVE_API int setupDatagramSocket(UsageEnvironment& env, Port port);
LIVE_API int setupStreamSocket(UsageEnvironment& env,
		      Port port, Boolean makeNonBlocking = True);

LIVE_API int readSocket(UsageEnvironment& env,
	       int socket, unsigned char* buffer, unsigned bufferSize,
	       struct sockaddr_in& fromAddress);

LIVE_API Boolean writeSocket(UsageEnvironment& env,
		    int socket, struct in_addr address, Port port,
		    u_int8_t ttlArg,
		    unsigned char* buffer, unsigned bufferSize);

LIVE_API Boolean writeSocket(UsageEnvironment& env,
		    int socket, struct in_addr address, Port port,
		    unsigned char* buffer, unsigned bufferSize);
    // An optimized version of "writeSocket" that omits the "setsockopt()" call to set the TTL.

LIVE_API unsigned getSendBufferSize(UsageEnvironment& env, int socket);
LIVE_API unsigned getReceiveBufferSize(UsageEnvironment& env, int socket);
LIVE_API unsigned setSendBufferTo(UsageEnvironment& env,
			 int socket, unsigned requestedSize);
LIVE_API unsigned setReceiveBufferTo(UsageEnvironment& env,
			    int socket, unsigned requestedSize);
LIVE_API unsigned increaseSendBufferTo(UsageEnvironment& env,
			      int socket, unsigned requestedSize);
LIVE_API unsigned increaseReceiveBufferTo(UsageEnvironment& env,
				 int socket, unsigned requestedSize);

LIVE_API Boolean makeSocketNonBlocking(int sock);
LIVE_API Boolean makeSocketBlocking(int sock, unsigned writeTimeoutInMilliseconds = 0);
  // A "writeTimeoutInMilliseconds" value of 0 means: Don't timeout

LIVE_API Boolean socketJoinGroup(UsageEnvironment& env, int socket,
			netAddressBits groupAddress);
LIVE_API Boolean socketLeaveGroup(UsageEnvironment&, int socket,
			 netAddressBits groupAddress);

// source-specific multicast join/leave
LIVE_API Boolean socketJoinGroupSSM(UsageEnvironment& env, int socket,
			   netAddressBits groupAddress,
			   netAddressBits sourceFilterAddr);
LIVE_API Boolean socketLeaveGroupSSM(UsageEnvironment&, int socket,
			    netAddressBits groupAddress,
			    netAddressBits sourceFilterAddr);

LIVE_API Boolean getSourcePort(UsageEnvironment& env, int socket, Port& port);

LIVE_API netAddressBits ourIPAddress(UsageEnvironment& env); // in network order

// IP addresses of our sending and receiving interfaces.  (By default, these
// are INADDR_ANY (i.e., 0), specifying the default interface.)
extern LIVE_API netAddressBits SendingInterfaceAddr;
extern LIVE_API netAddressBits ReceivingInterfaceAddr;

// Allocates a randomly-chosen IPv4 SSM (multicast) address:
LIVE_API netAddressBits chooseRandomIPv4SSMAddress(UsageEnvironment& env);

// Returns a simple "hh:mm:ss" string, for use in debugging output (e.g.)
LIVE_API char const* timestampString();


#ifdef HAVE_SOCKADDR_LEN
#define SET_SOCKADDR_SIN_LEN(var) var.sin_len = sizeof var
#else
#define SET_SOCKADDR_SIN_LEN(var)
#endif

#define MAKE_SOCKADDR_IN(var,adr,prt) /*adr,prt must be in network order*/\
    struct sockaddr_in var;\
    var.sin_family = AF_INET;\
    var.sin_addr.s_addr = (adr);\
    var.sin_port = (prt);\
    SET_SOCKADDR_SIN_LEN(var);


// By default, we create sockets with the SO_REUSE_* flag set.
// If, instead, you want to create sockets without the SO_REUSE_* flags,
// Then enclose the creation code with:
//          {
//            NoReuse dummy;
//            ...
//          }
class LIVE_API NoReuse {
public:
  NoReuse(UsageEnvironment& env);
  ~NoReuse();

private:
  UsageEnvironment& fEnv;
};


// Define the "UsageEnvironment"-specific "groupsockPriv" structure:

struct _groupsockPriv { // There should be only one of these allocated
  HashTable* socketTable;
  int reuseFlag;
};
LIVE_API _groupsockPriv* groupsockPriv(UsageEnvironment& env); // allocates it if necessary
LIVE_API void reclaimGroupsockPriv(UsageEnvironment& env);


#if defined(__WIN32__) || defined(_WIN32)
// For Windoze, we need to implement our own gettimeofday()
LIVE_API int gettimeofday(struct timeval*, int*);
#endif

// The following are implemented in inet.c:
extern "C" LIVE_API netAddressBits our_inet_addr(char const*);
extern "C" LIVE_API void our_srandom(int x);
extern "C" LIVE_API long our_random();
extern "C" LIVE_API u_int32_t our_random32(); // because "our_random()" returns a 31-bit number

#endif
