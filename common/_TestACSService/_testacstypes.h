#pragma once

#ifndef _VS_TESTACSTYPES_H
#define _VS_TESTACSTYPES_H

#include <map>
#include <list>
#include <vector>
#include <string>

/* common types and defines */

struct _TestMode {
	std::string address;		// target address		//exp. -amsk1:5050
	std::string endpoint;		// target endpoint		//exp. -eMSK1:1
	signed long users;			// testusers count		//exp. -u1000
	signed long type_service;	// testservice type		//exp. -t1
	signed long sleep_time;		// message send pause	//exp. -s250
	signed long reconnect;		// reconnects count		//exp. -r3
	signed long sleep_connect;	// next connect pause	//exp. -n100
	signed long flood_sleep;	// flood pause			//exp.
	signed long flood_mult;		// flood messages		//exp. ^^^lineup^^^
	signed long kill_sleep;		// kill pause			//exp.
	signed long kill_mult;		// kill users			//exp. ^^^lineup^^^
	signed long reboot_time;	// app restart time		//exp. -b1800000
	signed long nThreads;		// threads number		//exp. -p2
	bool ssl;					// use ssl				//exp. -c
	bool trash;					// garbage FIN_WAIT2	//exp. -g
	bool syncwait_;				// press "w" to start	//exp. -w
	bool onesocket;				// one socket transport	//exp. -1
	signed long _TranslateFlood	(const char *flood_str);
	signed long _TranslateKill	(const char *kill_str);
};

enum vs_packet_mode {
	VS_UNDEFINED		= -1,
	VS_TRANSPORT_PING	=  0,
	VS_SERVICE_PING		=  1,
	VS_SERVICE_CHAT		=  2
};

enum vs_iocp_op {
	iocp_Read	= -1,
	iocp_Nop	=  0,
	iocp_Write	=  1,
	iocp_Reconnect	= 0xFF + 1, // 0xFF + = socket control codes
	iocp_Delete		= 0xFF + 2, // this codes is for transfer socket to control IOCP thread
	iocp_Flood		= 0xFF + 3
};

enum vs_modifier_type {
	mod_Nop		= -1,	// mod undefined - bad modifier
	mod_Sock	=  0,	// mod sock - change socket parameters (exp. QoS, KeepAlive)
	mod_Data	=  1	// mod data - change data (exp. De/Crypter)
};

#define BUF_SIZE			0x1600
#define WM_DOREPAIR			WM_USER+11
#define WM_DONEXTMSG		WM_USER+12
#define WM_DOFLOOD			WM_USER+13
#define SEND_TIMEOUT		20*1000
#define MAX_SSL_RECONNECT	10

#endif