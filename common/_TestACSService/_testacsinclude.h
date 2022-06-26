#pragma once

#ifndef _VS_INCLUDE_H
#define _VS_INCLUDE_H

typedef TCHAR* PTCHAR; // avoid wxdebug.h error

#include "../acs/lib/VS_AcsLib.h"
#include "../acs/connectionmanager/VS_ConnectionManager.h"
#include "../std/cpplib/VS_Utils.h"
#include "../std/cpplib/VS_Endpoint.h"
#include "../std/cpplib/VS_Container.h"
#include "../std/cpplib/VS_Protocol.h"
#include "../transport/Client/VS_TransportClient.h"
#include "../transport/Lib/VS_TransportLib.h"
#include "../SecureLib/VS_CryptoInit.h"
#include "../SecureLib/VS_Certificate.h"
#include "../SecureLib/VS_SecureHandshake.h"
#include "../SecureLib/VS_SymmetricCrypt.h"
#include "../VSClient/VSClientBase.h"

#endif