#pragma once

#include "SecureLib/VS_Certificate.h"

struct VS_TlsContext
{
	enum ConnectionType
	{
		ct_undefined = 0,
		ct_am_client,
		ct_am_server
	} connectionType;

	enum CertCheckStatus
	{
		ccs_didnt_happen = 0,
		ccs_failure,
		ccs_success
	} certCheckStatus;

	VS_Certificate cert;

	VS_TlsContext(ConnectionType type = ct_undefined, CertCheckStatus certCheck = ccs_didnt_happen) :
		connectionType(type), certCheckStatus(certCheck) {}
};