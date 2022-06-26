#pragma once
#include <cstdint>
#include <cassert>

#include "std-generic/attributes.h"

enum class RegStatus: int32_t {
	failed                       = 0, // Reg. Server
	succeeded                    = 1, // Reg. Server
	changingHardwareIsNotAllowed = 2, // Reg. Server
	serverNameIsInUse            = 3, // Reg. Server
	certIsNotYetValid            = 4, // TCS
	certHasExpired               = 5, // TCS
	certSignatureIsInvalid       = 6, // TCS
	certIsInvalid                = 7, // TCS
	brokerIsNotAvailable         = 8, // TCS
	validLicenseIsNotAvailable   = 9, // Reg. Server
	certIsAbsent                 = 10, // TCS
	cannotWriteCert              = 11, // TCS
	unknownError                 = 12, // TCS
	serverNameIsTooLong          = 13, // TCS
	cannotGenerateCertRequest    = 14, // TCS
	fileAccessError              = 15, // RFF
	regFileGenError              = 16, // RFF
	badRegFile                   = 17, // RFF
	badPrivateKey                = 18, // RFF
	regFileEncryptError          = 19, // RFF
	regFileDecryptError          = 20, // RFF
	registryAccessError          = 21, // RFF
};

namespace vs
{
inline const char* ErrorCodeToString(RegStatus status)
{
	switch(status)
	{
	case RegStatus::failed:
		return "Invalid server ID or serial number";// Not sure about that, but no info is available
	case RegStatus::succeeded:
		return "Success";
	case RegStatus::changingHardwareIsNotAllowed:
		return "Changing hardware is not allowed for this server";
	case RegStatus::serverNameIsInUse:
		return "Server name is already in use";
	case RegStatus::brokerIsNotAvailable:
		return "Registry server is unavailable";
	case RegStatus::validLicenseIsNotAvailable:
		return "No valid licenses available for this server";
	case RegStatus::certIsAbsent:
		return "Registry server reply contained no certificate";
	case RegStatus::cannotWriteCert:
		return "Can't write the certificate to the registry";
	case RegStatus::serverNameIsTooLong:
		return "Server name is too long";
	case RegStatus::cannotGenerateCertRequest:
		return "Unable to generate the certificate request";
	case RegStatus::fileAccessError:
		return "File access error";
	case RegStatus::regFileGenError:
		return "Cannot generate the registration file";
	case RegStatus::badRegFile:
		return "Registration file is missing / corrupt";
	case RegStatus::badPrivateKey:
		return "Server's private key is invalid";
	case RegStatus::regFileEncryptError:
		return "Failed to encrypt the registration file";
	case RegStatus::regFileDecryptError:
		return "Failed to decrypt the registration file";
	case RegStatus::registryAccessError:
		return "Registry access failure";
	case RegStatus::certIsNotYetValid:
	case RegStatus::certHasExpired:
	case RegStatus::certSignatureIsInvalid:
	case RegStatus::certIsInvalid:
		assert(false);
		break;
	case RegStatus::unknownError:
	default:
		break;
	}
	return "Unknown error";
}
}
