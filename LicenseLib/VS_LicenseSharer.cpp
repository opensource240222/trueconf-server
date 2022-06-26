#include "VS_LicenseSharer.h"
#include "std/cpplib/VS_Protocol.h"
#include "std/cpplib/VS_RegistryKey.h"
#include "std/cpplib/VS_RegistryConst.h"
#include "std/cpplib/VS_Utils.h"
#include "ProtectionLib/Protection.h"
#include "std/debuglog/VS_Debug.h"
#include "statuslib/VS_ResolveServerFinder.h"
#include "net/Address.h"
#include "std/cpplib/VS_JsonConverter.h"

#include "std-generic/compat/chrono.h"
#include <climits>
#include <random>

#define DEBUG_CURRENT_MODULE VS_DM_REGS

const char LICENSE_TAG[]			= "License";
const char SHARED_LIC_KEY_NAME[]	= "SharedLicenses";
const char LAST_CHECK_TAG[]			= "LastCheck";
const char LAST_CONN_TAG[]			= "LastConnect";
const char LAST_DISCONN_TAG[]		= "LastDisconnect";
const char RESTRICT_TAG[]			= "Restrict";
const char LICENSE_MASTER_TAG[]		= "LicenseMasterServer";
const char ALLOW_ANY_SLAVE_TAG[]	= "AllowAnySlave";
const char MASTER_STATUS_TAG[]		= "LicenseMasterServerStatus";
const char RESOLVED_MASTER_NAME_TAG[] = "LicenseMasterResolvedServerName";
const char SLAVE_SERVER_OBJ[]		= "Slave Server";

const std::chrono::minutes  lic::Sharer::LAST_CHECK_TIMEOUT = std::chrono::minutes(60);
const std::chrono::minutes  CHECK_SUPPRESSING_TIMEOUT(5);
const std::chrono::seconds	CHECK_RESPONSE_TIMEOUT(10);
const std::chrono::minutes RETURN_LIC_OVERHEAD_TIMEOUT(1);
bool lic::Sharer::s_resolvedMasterActive = false;

namespace lic {

const unsigned char c_sharer_private_key[] = {
	0xc6, 0x96, 0xda, 0x25, 0xe5, 0xd6, 0xa2, 0x9e, 0xec, 0xe1, 0x2e, 0xb2, 0x07, 0xfe, 0x4e, 0x27,
	0xd0, 0x2d, 0x84, 0xff, 0xd4, 0xbe, 0x5e, 0x62, 0x15, 0xfb, 0xd6, 0x70, 0x9a, 0xda, 0x76, 0x11,
	0x2e, 0xd1, 0xd2, 0x3c, 0x24, 0x41, 0xd3, 0x42, 0x11, 0x90, 0x05, 0x69, 0x43, 0xd1, 0x32, 0x3f,
	0x98, 0xe4, 0x61, 0xeb, 0x97, 0x51, 0xf7, 0x1c, 0x5c, 0x46, 0xcc, 0xfc, 0x97, 0x12, 0x30, 0xba,
	0x38, 0xd2, 0xc5, 0x5b, 0xd7, 0x09, 0x89, 0xd2, 0x4d, 0x46, 0x15, 0x7c, 0xd5, 0x78, 0x16, 0x36,
	0x77, 0x0a, 0x3a, 0xd9, 0x35, 0xb4, 0x10, 0x99, 0x66, 0xc5, 0x80, 0x82, 0x76, 0x65, 0x94, 0xea,
	0x7f, 0xb4, 0xb4, 0x27, 0x41, 0x6e, 0xcd, 0xec, 0x24, 0x74, 0xbd, 0xf3, 0xba, 0xb5, 0x3a, 0x8a,
	0x85, 0x0e, 0xf3, 0x02, 0x47, 0x77, 0xbb, 0x14, 0x24, 0x0e, 0x6c, 0xff, 0x35, 0xa0, 0xd1, 0xa2,
	0x16, 0xa0, 0x86, 0x89, 0xa2, 0x30, 0x13, 0xb5, 0xf2, 0x60, 0x84, 0x93, 0x6a, 0xe7, 0xe3, 0xb9,
	0x7e, 0xb8, 0xe3, 0xd1, 0xb1, 0xde, 0x69, 0x1a, 0xeb, 0x6d, 0x04, 0xb4, 0x47, 0x4b, 0x6b, 0xad,
	0xc8, 0xca, 0xa5, 0x38, 0x23, 0x5a, 0xe3, 0x6c, 0xb8, 0x05, 0x18, 0xae, 0xfb, 0x59, 0xc5, 0xca,
	0x02, 0xae, 0x9b, 0x33, 0xdc, 0x4c, 0x75, 0x16, 0x52, 0x0e, 0x4b, 0x92, 0x40, 0x11, 0xea, 0xe9,
	0x00, 0x36, 0xb7, 0xd2, 0x31, 0x7c, 0x0b, 0x63, 0xf4, 0x58, 0x3e, 0x9c, 0x87, 0xb8, 0xd1, 0x17,
	0xc7, 0x78, 0x02, 0xe9, 0x8d, 0x75, 0x45, 0xe1, 0x8c, 0x8f, 0x54, 0x5b, 0xa9, 0x1f, 0x0d, 0x2f,
	0x3f, 0x75, 0xaf, 0xd7, 0x6c, 0x3f, 0x3a, 0x26, 0x4a, 0x08, 0xc6, 0x2b, 0x59, 0x8c, 0x4f, 0x2c,
	0xa7, 0x53, 0xaf, 0x1b, 0x8e, 0xbe, 0x5d, 0x07, 0x4f, 0xf4, 0x54, 0xdb, 0x10, 0xff, 0x30, 0x9e,
	0x60, 0xc6, 0xb4, 0x6d, 0x32, 0x77, 0xf1, 0x6b, 0xab, 0xea, 0xd9, 0x7a, 0x89, 0x66, 0xf0, 0x28,
	0x64, 0x72, 0x51, 0xe2, 0x62, 0x63, 0x06, 0x35, 0xe2, 0xde, 0x71, 0x6e, 0xf8, 0x28, 0xa3, 0x60,
	0xec, 0x1e, 0x1f, 0xf7, 0x02, 0x04, 0x65, 0x3b, 0x92, 0xe4, 0x05, 0x39, 0x51, 0x27, 0x64, 0x78,
	0x0e, 0x3f, 0x9a, 0xce, 0xe5, 0x0b, 0xc4, 0x35, 0x14, 0xeb, 0x30, 0x7d, 0xc5, 0x41, 0xb6, 0xd6,
	0x8a, 0x4f, 0xf3, 0xb9, 0x4e, 0xb2, 0x83, 0x26, 0x4d, 0x5d, 0x0a, 0xc2, 0xff, 0x78, 0x2b, 0xc0,
	0xdd, 0xc8, 0x41, 0x4b, 0xdd, 0xa8, 0x98, 0xa5, 0x33, 0xe3, 0xc8, 0x97, 0xb3, 0x83, 0xde, 0xf1,
	0xda, 0xea, 0x4f, 0xb5, 0xee, 0xb7, 0x5d, 0x64, 0xb6, 0x63, 0x54, 0xac, 0x6b, 0x12, 0xb6, 0xa2,
	0x67, 0x5f, 0x0e, 0xfa, 0x8f, 0x7b, 0x6a, 0xbf, 0x84, 0xe9, 0x88, 0xb3, 0x30, 0xe3, 0xef, 0x2a,
	0x4d, 0x83, 0x00, 0xcb, 0x0c, 0x1a, 0xfb, 0xb6, 0x63, 0x37, 0x81, 0x48, 0x53, 0xb3, 0xec, 0xfa,
	0x7b, 0xdd, 0x4b, 0x89, 0xba, 0x1e, 0xca, 0x4f, 0xf2, 0xa8, 0x09, 0xd5, 0xeb, 0x00, 0xfc, 0xaf,
	0x62, 0xad, 0x12, 0x33, 0x92, 0x3c, 0x47, 0xf9, 0xeb, 0xfe, 0xa7, 0x2b, 0x57, 0x92, 0x51, 0xe4,
	0xf0, 0xdc, 0x97, 0x8c, 0x6f, 0x97, 0x35, 0x5c, 0x25, 0xc2, 0x7a, 0xc5, 0x2a, 0xc6, 0xdc, 0x57,
	0x76, 0xc0, 0x8a, 0x67, 0x82, 0x4b, 0x0e, 0x35, 0x44, 0x00, 0x61, 0x61, 0xe0, 0x7d, 0xb7, 0xe4,
	0x2c, 0x6e, 0x30, 0x53, 0xa1, 0x30, 0x27, 0x75, 0x45, 0x4a, 0x38, 0x80, 0x3e, 0xd3, 0xbe, 0x21,
	0x82, 0xc2, 0x8a, 0x56, 0xf7, 0x88, 0xfa, 0x39, 0x0a, 0xcc, 0x95, 0x64, 0x9a, 0xd4, 0x77, 0x61,
	0xf6, 0x4e, 0x9f, 0x95, 0xd4, 0xf3, 0x4a, 0x51, 0x11, 0xc7, 0xd5, 0x9c, 0x20, 0x59, 0x38, 0x79,
	0xc8, 0x40, 0x1e, 0x0d, 0x84, 0x44, 0xf9, 0x4d, 0xfd, 0x45, 0xde, 0xbb, 0x35, 0x99, 0xfb, 0xec,
	0x6f, 0x9e, 0x62, 0x90, 0x76, 0xa5, 0x3f, 0xcd, 0x6c, 0x0f, 0x35, 0xe3, 0x20, 0x5d, 0x27, 0x51,
	0x57, 0x63, 0x5f, 0xc9, 0x2e, 0x86, 0x4f, 0x58, 0x12, 0xec, 0x93, 0x13, 0xdb, 0xac, 0x7b, 0x77,
	0x02, 0x88, 0x6f, 0xc5, 0x8e, 0x87, 0x28, 0x97, 0xc2, 0x42, 0xaf, 0xbf, 0x54, 0x5d, 0xea, 0x3e,
	0xd9, 0xe0, 0x0f, 0xc8, 0x62, 0x33, 0x52, 0xfa, 0x63, 0x2c, 0xf5, 0x9d, 0xa8, 0x04, 0x55, 0x34,
	0x8a, 0xed, 0x3b, 0x3c, 0x78, 0x40, 0x90, 0x69, 0xea, 0x55, 0x37, 0x68, 0x25, 0x73, 0xfa, 0xc6,
	0x51, 0x56, 0xd6, 0x33, 0x47, 0x46, 0x3f, 0x40, 0xd5, 0x00, 0xd7, 0xf1, 0xf6, 0x84, 0xf4, 0x17,
	0x32, 0xa0, 0x88, 0xcd, 0x80, 0xf6, 0xf9, 0x85, 0xed, 0x05, 0xa7, 0x44, 0x61, 0x55, 0xac, 0x6b,
	0x10, 0x0d, 0x5e, 0x33, 0xa6, 0xf9, 0x16, 0x51, 0xd2, 0x3d, 0xc7, 0x0c, 0xd8, 0xc9, 0x82, 0xa8,
	0x45, 0x00, 0x21, 0xfe, 0x89, 0xa5, 0x07, 0xc2, 0x3f, 0x68, 0xd1, 0x9e, 0x07, 0x8e, 0x4d, 0x40,
	0xfb, 0xc5, 0x82, 0x09, 0x26, 0x55, 0x67, 0x2c, 0xec, 0x3f, 0x1f, 0xa7, 0xeb, 0xff, 0xda, 0x96,
	0xa2, 0xd4, 0x5b, 0x3a, 0x6b, 0x29, 0xfc, 0xe5, 0xff, 0x6a, 0xcf, 0x95, 0xf4, 0x48, 0xd6, 0xbc,
	0x22, 0xff, 0x9c, 0x05, 0x32, 0x49, 0x12, 0x25, 0x93, 0x79, 0x17, 0xd3, 0x04, 0xfb, 0x45, 0x90,
	0x5c, 0x71, 0xe9, 0x55, 0xfa, 0x04, 0x3b, 0x90, 0xdc, 0x32, 0x7f, 0xf1, 0xfb, 0x5d, 0x67, 0xa1,
	0x83, 0x1a, 0xbd, 0x45, 0xde, 0x01, 0x49, 0x25, 0xae, 0x79, 0x3d, 0xbf, 0xeb, 0x82, 0xff, 0xe2,
	0xf0, 0x8f, 0xad, 0x7f, 0xdd, 0x7a, 0x91, 0x02, 0xc0, 0x69, 0x85, 0xaa, 0x35, 0x7e, 0x31, 0x71,
	0x07, 0x1d, 0xec, 0xb9, 0x04, 0x69, 0xf4, 0x6d, 0xe0, 0x74, 0x3d, 0x4d, 0xe2, 0x1b, 0x50, 0x13,
	0xda, 0x18, 0xcf, 0xe2, 0x2e, 0x57, 0xe2, 0x76, 0xc0, 0x11, 0xb6, 0x28, 0x0a, 0x31, 0xf7, 0xde,
	0x0e, 0x09, 0x21, 0xd5, 0x24, 0x0a, 0x9a, 0xe2, 0x25, 0x02, 0x4b, 0x3f, 0x35, 0x50, 0x23, 0xbf,
	0xfe, 0xc9, 0x25, 0xf1, 0x32, 0xf6, 0x73, 0xc8, 0x24, 0x63, 0x2e, 0x48, 0x2c, 0x23, 0xa8, 0x55,
	0xdf, 0x5e, 0x80, 0x59, 0xef, 0xd3, 0x07, 0x07, 0x7e, 0x16, 0x67, 0x80, 0xcd, 0xe2, 0x76, 0x0a,
	0x96, 0xc2, 0x6d, 0x71, 0xd0, 0x46, 0xd7, 0x31, 0x89, 0xa0, 0xa7, 0xd4, 0x50, 0x9e, 0xb4, 0x40,
	0x22, 0xbd, 0xb3, 0xf1, 0xe7, 0x98, 0x87, 0xa3, 0xeb, 0x62, 0xdd, 0x29, 0xbe, 0x14, 0x01, 0xf0,
	0xd4, 0xb9, 0x8a, 0xf4, 0x77, 0x53, 0x3b, 0x7a, 0x15, 0xd2, 0x24, 0x9d, 0xcf, 0xc0, 0x2f, 0xab,
	0x5e, 0xb8, 0xc2, 0xf2, 0xac, 0x08, 0xc7, 0x8a, 0xcc, 0x00, 0x1f, 0x68, 0x30, 0x4f, 0x5a, 0x1e,
	0x41, 0xda, 0xbb, 0x8e, 0xba, 0x37, 0x65, 0x55, 0xca, 0x2e, 0x4c, 0x19, 0xb8, 0x69, 0x14, 0x6c,
	0x2a, 0x24, 0x8e, 0x91, 0x5c, 0xb1, 0x98, 0x96, 0x86, 0xb7, 0xb2, 0x3d, 0x81, 0x72, 0xad, 0x3d,
	0x7c, 0x26, 0x01, 0x61, 0x90, 0x40, 0x88, 0xbf, 0xcd, 0x33, 0x12, 0x26, 0x9e, 0x50, 0x5d, 0xb5,
	0x51, 0x17, 0x75, 0x6e, 0x10, 0xb2, 0x10, 0x99, 0x17, 0x5d, 0xc5, 0x4a, 0x13, 0xc4, 0x64, 0x9e,
	0x64, 0x0d, 0x14, 0x8c, 0xb2, 0x3c, 0xb8, 0x13, 0xdf, 0xed, 0xde, 0xeb, 0x1f, 0xb5, 0xe5, 0x7d,
	0xc2, 0xfa, 0xfd, 0x88, 0x3c, 0xfb, 0xf8, 0xce, 0xa2, 0x8f, 0x39, 0xba, 0x3c, 0xd5, 0x52, 0x3a,
	0xcf, 0x34, 0xd1, 0x06, 0x24, 0x06, 0x6e, 0xc7, 0x1e, 0x32, 0x5f, 0x59, 0xc8, 0x08, 0xe0, 0xa0,
	0x42, 0xe3, 0x45, 0x7b, 0xfa, 0x25, 0xbe, 0x88, 0x64, 0x20, 0x07, 0xa7, 0x39, 0x93, 0x66, 0x4a,
	0x0c, 0xeb, 0x41, 0x54, 0x13, 0x4c, 0x03, 0xd7, 0x13, 0x5a, 0x93, 0x32, 0x67, 0xff, 0xe4, 0xe0,
	0x84, 0xaa, 0x14, 0xff, 0x39, 0x62, 0xba, 0x5d, 0xcc, 0xfe, 0xf3, 0x6e, 0x36, 0x75, 0x25, 0x3e,
	0x66, 0x07, 0x01, 0x9e, 0x6f, 0x0f, 0xca, 0xa6, 0xc2, 0xf0, 0x6c, 0x4c, 0x03, 0x78, 0x75, 0x13,
	0xbe, 0x7d, 0x44, 0x1f, 0x62, 0x1c, 0x92, 0xbc, 0xfe, 0xd3, 0x2f, 0x34, 0x54, 0x43, 0x31, 0x03,
	0xbd, 0x88, 0x92, 0x0d, 0xab, 0x2a, 0xad, 0x59, 0x08, 0x81, 0xf0, 0x92, 0xf2, 0xf6, 0xda, 0xa8,
	0x0d, 0x64, 0x9f, 0x13, 0x19, 0xd4, 0xcd, 0x6f, 0x05, 0xa5, 0x1f, 0x1b, 0x71, 0x7e, 0x64, 0xe4,
	0x31, 0x07, 0xd6, 0xe9, 0x52, 0xc6, 0x07, 0xdb, 0x7c, 0xd3, 0xbe, 0xe1, 0x11, 0xd6, 0x33, 0x38,
	0x5a, 0xbd, 0xf7, 0xcc, 0x88, 0x71, 0xa6, 0x44, 0x2b, 0xf1, 0x4c, 0xe3, 0xa7, 0x9e, 0x12, 0x55,
	0x34, 0x99, 0xaa, 0x06, 0xdc, 0xd3, 0x45, 0xbf, 0x16, 0x20, 0xa4, 0xd0, 0xd4, 0x04, 0xb9, 0xbb,
	0x34, 0xdf, 0x8e, 0xb2, 0x4c, 0xb1, 0x41, 0x01, 0xd7, 0x62, 0x2a, 0xc7, 0x8d, 0xfd, 0x79, 0x79,
	0xd5, 0x99, 0x67, 0xb7, 0x83, 0x4b, 0x78, 0x11, 0x8d, 0x01, 0x49, 0x13, 0x5c, 0x4e, 0x94, 0xa0,
	0x9e, 0xe4, 0xf5, 0x04, 0x24, 0xa6, 0x6c, 0x37, 0x5a, 0x11, 0xc0, 0x43, 0x05, 0x19, 0x46, 0xba,
	0x4d, 0x1f, 0x2f, 0x38, 0xf7, 0xdb, 0xcc, 0x95, 0x0a, 0xc3, 0xc3, 0xa1, 0xb4, 0xfe, 0x55, 0xe8,
	0x23, 0xd7, 0xbf, 0xea, 0x4e, 0xd8, 0xf0, 0x44, 0x36, 0x4b, 0x68, 0xf2, 0xb9, 0xc2, 0x2f, 0x32,
	0x15, 0x37, 0x8b, 0xb5, 0x65, 0xcf, 0x07, 0x24, 0x7e, 0x1b, 0x78, 0x48, 0x12, 0x83, 0x49, 0xda,
	0x00, 0xd7, 0x9d, 0xfb, 0x0c, 0xd4, 0x9b, 0x22, 0xf2, 0x6a, 0x62, 0x60, 0xc7, 0x6b, 0xf4, 0x3d,
	0x5e, 0xce, 0x94, 0xee, 0x09, 0x23, 0x2a, 0x34, 0xd0, 0xb2, 0x94, 0xd9, 0x63, 0xd2, 0xf6, 0x9a,
	0x03, 0x17, 0xf0, 0x46, 0xd7, 0x9d, 0x31, 0xb1, 0x57, 0xeb, 0x4a, 0x73, 0x85, 0x52, 0x0f, 0x42,
	0x20, 0xfd, 0xde, 0x1b, 0xc1, 0x3c, 0x6a, 0x73, 0x56, 0x8e, 0x9e, 0x0c, 0x2c, 0x19, 0x78, 0xae,
	0x04, 0x7f, 0x72, 0xe7, 0x2d, 0x73, 0x40, 0xa7, 0xe7, 0x8c, 0x2d, 0x5d, 0x18, 0xed, 0x22, 0xd2,
	0x33, 0x32, 0x39, 0x25, 0xcd, 0x73, 0x87, 0xcc, 0xca, 0x70, 0xf0, 0xc7, 0x30, 0xed, 0x4d, 0xce,
	0x05, 0xc7, 0x9a, 0x5f, 0x97, 0xb9, 0x78, 0x5c, 0xea, 0xb9, 0x64, 0xbb, 0x21, 0x20, 0x48, 0x98,
	0x45, 0xa2, 0x74, 0x29, 0x01, 0x98, 0x42, 0x88, 0x7d, 0x71, 0xfe, 0x98, 0xf4, 0x1b, 0xa6, 0x00,
	0xd9, 0xb7, 0xd8, 0xde, 0x64, 0x2d, 0xec, 0x9f, 0xa6, 0x56, 0xba, 0xbf, 0x92, 0xaf, 0x5d, 0x53,
	0x12, 0x5b, 0xe8, 0x76, 0x3f, 0x8e, 0x2a, 0xb7, 0x6a, 0x95, 0xf6, 0x40, 0x50, 0xdc, 0xc6, 0x32,
	0xf8, 0xde, 0x74, 0x1a, 0xa7, 0x15, 0x16, 0x5c, 0xc0, 0x8a, 0xa9, 0xb0, 0xbc, 0x40, 0x98, 0x85,
	0xc4, 0x31, 0xbd, 0x51, 0x36, 0xf5, 0xd8, 0x89, 0x50, 0x23, 0x57, 0xf6, 0x9a, 0x26, 0xf9, 0x59,
	0xce, 0x11, 0x05, 0x67, 0xc5, 0x84, 0x77, 0x9b, 0x06, 0x9c, 0xe3, 0xeb, 0xb3, 0x19, 0x47, 0x28,
	0xe1, 0xfc, 0x97, 0x1d, 0xf4, 0x3e, 0xb8, 0xda, 0x48, 0x6a, 0x0e, 0x61, 0xdf, 0x29, 0x3b, 0x36,
	0x50, 0xba, 0x0d, 0x54, 0xe3, 0x03, 0xe5, 0x57, 0x5d, 0x3d, 0xb8, 0xac, 0x2d, 0xdd, 0x10, 0x33,
	0xc2, 0x88, 0x20, 0xfa, 0x97, 0xd9, 0x6c, 0xc1, 0x71, 0xa0, 0xd3, 0x32, 0x39, 0xe0, 0xf3, 0x89,
	0x45, 0x43, 0xdd, 0x09, 0x37, 0x73, 0x90, 0xa5, 0x09, 0x34, 0xc9, 0x5e, 0x66, 0x8c, 0x38, 0xb8,
	0x93, 0xbc, 0x4a, 0x81, 0x32, 0xcf, 0xec, 0x0e, 0x6c, 0x28, 0x11, 0x75, 0xe7, 0xb0, 0x53, 0x9f,
	0xed, 0x3a, 0x3e, 0x2d, 0x06, 0xf5, 0xb0, 0x6b, 0x50, 0x68, 0xf5, 0x2a, 0x36, 0x50, 0x50, 0x64,
	0x96, 0x7a, 0xeb, 0xc5, 0x83, 0x48, 0x3c, 0xed, 0x35, 0xba, 0x76, 0xc0, 0x11, 0x77, 0x15, 0x07,
	0x73, 0x57, 0x6d, 0x88, 0x9b, 0x22, 0x9d, 0xf5, 0x11, 0x0c, 0x3c, 0x15, 0x69, 0xe8, 0x58, 0x81,
	0x12, 0x0f, 0x79, 0x20, 0xba, 0xde, 0xa4, 0x51, 0x8f, 0x89, 0x78, 0x4f, 0x5a, 0x80, 0xff, 0xca,
	0x52, 0x86, 0xc6, 0xc6, 0x90, 0x97, 0x11, 0x0d, 0x12, 0xa2, 0xf5, 0xaf, 0x9a, 0x93, 0x4f, 0xb7,
	0xea, 0xa8, 0xd2, 0x3f, 0x7b, 0x84, 0xd0, 0x90, 0x62, 0xe9, 0xbc, 0x51, 0x90, 0x38, 0xde, 0xcf,
	0x1b, 0x57, 0xff, 0x5a, 0xc7, 0x05, 0x49, 0xd6, 0xa9, 0xb2, 0x9f,
};
const uint32_t c_sharer_private_key_random_seed = 0x7abe39e2;
const char c_sharer_public_key[] =
	"-----BEGIN PUBLIC KEY-----\n"
	"MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA3YEOFI+K8VEwPkpCNHC9\n"
	"xkL4EqlSvMKAw/g72HXWnxmyqqAaPSeZ29UuBoWAlUKz9LBIjHDh4JYGJVNFyT5w\n"
	"CAlkCk8gmxR7OfZkRJC7+Wrq1Hv3nVOzbNvUvkHYiFfb2TQ1J+kgGV9rG2BfZYyQ\n"
	"ZlIfsiWXqTm419cnaJWa5Z819MTng9D6fIsLwBeiDiF/mWUu2NK1pzBf5BIfm58I\n"
	"FU9igx8zjGEBeo3gezdL5sAq3++VDYyIeqvwGwNATOh6HZ9h4l4HTfvhfaJ2IwiN\n"
	"dkk3pxp2icJpFJWH09zQO50uC1L2oSpqDvpRXdtpOZN6TuaD5l8v/JP4adIHjLxu\n"
	"ewIDAQAB\n"
	"-----END PUBLIC KEY-----\n";

	int32_t REQ_LIMIT = 20; // can't request more that 20 units of license resource
}

#include "ProtectionLib/OptimizeDisable.h"

bool SetLastCheck(std::chrono::system_clock::time_point t, VS_RegistryKey& key)
{
	bool res(false);
	char str[128] = { 0 };
	if (tu::TimeToNStr(t, str, 128) > 0)
		res = key.SetString(str, LAST_CHECK_TAG);
	return res;
}

std::chrono::system_clock::time_point GetLastCheck(const VS_RegistryKey& key)
{
	std::chrono::system_clock::time_point res;
	std::string time_str;
	if (key.GetString(time_str, LAST_CHECK_TAG)) {
		auto t = tu::NStrToTimeT(time_str.c_str());
		if (t != static_cast<time_t>(-1))
			res = std::chrono::system_clock::from_time_t(t);
	}
	return res;
}

bool lic::Sharer::SaveLicense(const VS_License & l, const std::string & reg_path)
{
	bool res(false);
SECUREBEGIN_E_ENTERPRISE
	VS_License::SignedHWLicense signed_l;
	l.ConvertToSigned(signed_l);
	signed_l.SetHWKey();

	// TODO: Remove this cheap in-memory obfuscation after migrating to VMProtect (replace with VMProtectDecryptStringA).
	char private_key[sizeof(lic::c_sharer_private_key) + 1];
	std::mt19937 rnd(lic::c_sharer_private_key_random_seed);
	for (size_t i = 0; i < sizeof(lic::c_sharer_private_key); ++i)
		private_key[i] = lic::c_sharer_private_key[i] ^ static_cast<unsigned char>(rnd());
	private_key[sizeof(lic::c_sharer_private_key)] = '\0';

	if (signed_l.MakeSign(private_key))
	{
		size_t signed_lic_buff_len(0);
		std::unique_ptr<uint8_t[]> signed_lic_buff(nullptr);
		if (signed_l.Encode(signed_lic_buff, signed_lic_buff_len) && signed_lic_buff_len > 0)
		{
			if (VS_RegistryKey(false, reg_path, false, true).SetValue(signed_lic_buff.get(), signed_lic_buff_len, VS_REG_BINARY_VT, LICENSE_TAG))
				res = true;
			else
				dstream3 << "lic::Sharer::SaveLicense: Can't store license to registry";
		}
		else
			dstream3 << "lic::Sharer::SaveLicense: Can't encode license";
	}
	else
		dstream3 << "lic::Sharer::SaveLicense: Can't sign license";
SECUREEND_E_ENTERPRISE
	if (res) {
		VS_RegistryKey k(false, reg_path, false, true);
		l.SaveToRegistry(k);	// human readable saving for web, has own SECURE sections
	}
	return res;
}

bool lic::Sharer::LoadLicense(const std::string & reg_path, VS_License & OUTlic)
{
	VS_RegistryKey k(false, reg_path);
	return LoadLicense(k, OUTlic);
}


bool lic::Sharer::LoadLicense(VS_RegistryKey & k, VS_License & OUTlic)
{
	std::unique_ptr<void, free_deleter> signed_l_buff;
	const auto bsize = k.GetValue(signed_l_buff, VS_REG_BINARY_VT, LICENSE_TAG);
	assert(bsize >= 0);
	if (bsize <= 0)
	{
		dstream3 << "lic::Sharer::LoadLicense: Can't find '" << k.GetName() << '\\' << LICENSE_TAG << "' in registry";
		return false;
	}

	bool res(false);
SECUREBEGIN_E_ENTERPRISE
	VS_License::SignedHWLicense signed_l;
	if (signed_l.Decode(signed_l_buff.get(), bsize))
	{
		if (signed_l.VerifySign(lic::c_sharer_public_key))
		{
			if (signed_l.VerifyHWKey())
			{
				OUTlic.ConvertFromSigned(signed_l);
				res =  true;
			}
			else
				dstream3 << "lic::Sharer::LoadLicense: HWKey verification faield";
		}
		else
			dstream3 << "lic::Sharer::LoadLicense: License verification failed";
	}
	else
		dstream3 << "lic::Sharer::LoadLicense: License decoding failed";
SECUREEND_E_ENTERPRISE
	return res;
}

bool SaveTimeForSlave(const std::string & slave_name, const char* time_tag, const std::chrono::system_clock::time_point t) {
	if (!time_tag) return false;

	std::string sh_lic_key_name = SHARED_LIC_KEY_NAME; sh_lic_key_name += "\\"; sh_lic_key_name += slave_name;
	bool res(false);
	VS_RegistryKey sh_lic_key(false, sh_lic_key_name, false);

	char str[128] = { 0 };
	if (tu::TimeToNStr(t, str, 128) > 0)
		res = sh_lic_key.SetString(str, time_tag);

	return res;
}

bool lic::Sharer::SaveSlavesSharedLicence(const lic::Sharer::ShareLicenseInfo & info, const std::string & server, bool save_connection_time)
{
	std::string sh_lic_key_name = SHARED_LIC_KEY_NAME; sh_lic_key_name += "\\"; sh_lic_key_name += server;
	bool res(true);

	res = SaveLicense(info.lic, sh_lic_key_name);	// has secure tags
	if (!res) dstream3 << "Error\tSlaves shared license wasn't saved!\n";

	res = res && SaveTimeForSlave(server, LAST_CHECK_TAG, info.last_check);
	if(save_connection_time) res = res && SaveTimeForSlave(server, LAST_CONN_TAG, info.last_connection);

	return res;
}

bool lic::Sharer::SaveReceivedSharedLicence(const VS_License & l)
{
	bool res(false);

	res = SaveLicense(l, SHARED_LIC_KEY_NAME);
	if (!res) dstream3 << "Error\tReceived shared license wasn't saved!\n";

	VS_RegistryKey sh_lic_key(false, SHARED_LIC_KEY_NAME, false, true);
	res = res && SetLastCheck(std::chrono::system_clock::now(), sh_lic_key);
	return res;
}

void lic::Sharer::LogEvent(string_view object_type, string_view object_name, string_view event_type, string_view message)
{
	assert(m_transport != nullptr);

	VS_Container msg;
	msg.AddValue("Message", message);
	std::string payload(ConvertToJsonStr(msg));

	VS_Container log_cnt;
	log_cnt.AddValue(OBJECT_TYPE_PARAM, object_type);
	log_cnt.AddValue(OBJECT_NAME_PARAM, object_name);
	log_cnt.AddValue(EVENT_TYPE_PARAM, event_type);
	log_cnt.AddValue(PAYLOAD_PARAM, payload);

	dstream3 << "lic::Sharer Event: object type='" << object_type << "', object_name='" << object_name << "', event_type='" << event_type << "', message='" << message << "'\n";
	m_transport->PostRequest(m_transport->OurEndpoint(), 0, log_cnt, 0, LOG_SRV);
}

unsigned lic::Sharer::GetLicID()
{
	assert(p_licWrap->IsSlave());	// id counting only on slave side
	return ++m_licID == 0 ? ++m_licID : m_licID;
}

void SaveResolvedServerName(const char* name) {
	if (!name) return;

	VS_RegistryKey(false, SHARED_LIC_KEY_NAME, false, true).SetString(name, RESOLVED_MASTER_NAME_TAG);
	dstream3 << "lic::Sharer Resolved master server name '" << name << "'\n";
	lic::Sharer::s_resolvedMasterActive = false;
}

lic::Sharer::Sharer()
{
}

bool lic::Sharer::Init()
{
	if (p_licWrap->IsSlave())
		SaveResolvedServerName(""); // empty resolved name on Start
	return true;
}

void lic::Sharer::SetTransport(VS_TransportRouterServiceHelper * transport)
{
	m_transport = transport;
}

bool AddLicense(VS_Container& cnt, const VS_License & l, const char* funcName) {
	if (!funcName)
		return false;

	size_t licBuffLen(0);
	std::unique_ptr<uint8_t[]> licBuff(nullptr);
	l.Serialize(licBuff, licBuffLen);
	if (licBuffLen == 0) {
		dstream3 << funcName << " Fail to serialize license!\n";
		return false;
	}

	return cnt.AddValue(LICENSE_TAG, licBuff.get(), licBuffLen);
}

bool lic::Sharer::GetLicense(const VS_Container&cnt, const char* funcName, VS_License& res) {
	if (!funcName)
		return false;

	size_t size(0);
	auto pLicBuff = cnt.GetBinValueRef(LICENSE_TAG, size);
	if (!pLicBuff || size == 0){
		dstream3 << funcName << " Fail to find license in container!\n";
		return false;
	}

	if (!res.Deserialize(pLicBuff, size)) {
		dstream3 << funcName << " Fail to deserealize license!\n";
		return false;
	}
	return true;
}

void lic::Sharer::RequestLicense(const VS_License & l, const std::string &master_server)
{
	assert(m_transport != nullptr);
	assert(p_licWrap != nullptr);

	if (!p_licWrap->IsSlave()) return;

	dstream3 << "lic::Sharer RequestLicense from master='" << master_server << "'\n";
	VS_Container to_master;
	to_master.AddValue(METHOD_PARAM, REQUEST_LICENSE_METHOD);
	AddLicense(to_master, l, __FUNCTION__);
	++changeLic_requests_to_master;
	m_transport->PostRequest(master_server.c_str(), nullptr, to_master, nullptr, m_transport->OurService());
}

void ClearSlaveSharedLicense(const std::string& server) {
	std::string key_name = SHARED_LIC_KEY_NAME; key_name += "\\"; key_name += server;
	VS_RegistryKey sh_lic_key(false, key_name, false);
	if (!sh_lic_key.IsValid()) return;

	sh_lic_key.RemoveValue(LICENSE_TAG);
	sh_lic_key.RemoveValue(LAST_CHECK_TAG);

	VS_License dummy; dummy.SaveToRegistry(sh_lic_key); // clear human readable values for web
}


void ClearMyLicense() {
	VS_RegistryKey sh_lic_key(false, SHARED_LIC_KEY_NAME, false);
	if (!sh_lic_key.IsValid()) return;

	sh_lic_key.RemoveValue(LICENSE_TAG);
	sh_lic_key.RemoveValue(LAST_CHECK_TAG);
	VS_License dummy; dummy.SaveToRegistry(sh_lic_key); // clear human readable values for web
}

void UpdateTimeOnMySharedLicense(std::chrono::system_clock::time_point t) {
	VS_RegistryKey sh_lic_key(false, SHARED_LIC_KEY_NAME,false,true);
	SetLastCheck(t, sh_lic_key);
}

std::chrono::system_clock::time_point GetMySharedLicenseUpdateTime() {
	VS_RegistryKey sh_lic_key(false, SHARED_LIC_KEY_NAME);
	return GetLastCheck(sh_lic_key);
}

void lic::Sharer::ShareLicense(const ShareResult& r, const std::string &slave_server, const VS_License* l)
{
	assert(m_transport != nullptr);
	assert(p_licWrap != nullptr);
	if (p_licWrap->IsMaster() && !slave_server.empty()) {
		dstream3 << "lic::Sharer Share license for slave='" << slave_server << "', res= '" << static_cast<uint32_t>(r) << "'\n";

		VS_Container to_slave;
		to_slave.AddValue(METHOD_PARAM, SHARE_LICENSE_METHOD);
		to_slave.AddValueI32(RESULT_PARAM, r);

		if (r == ShareResult::OK && l != nullptr) {
			auto it = m_shared_licenses.find(slave_server);
SECUREBEGIN_E_ENTERPRISE
			AddLicense(to_slave, *l, __FUNCTION__);
			if(!l->HasSharedResources()){
				LogEvent(SLAVE_SERVER_OBJ, slave_server, "license sharing", "Empty license sending. License sharing limit is reached\n");
			}

			if (it == m_shared_licenses.end()) it = m_shared_licenses.emplace(slave_server, ShareLicenseInfo()).first;
			it->second.lic.AddLicence(*l);
			it->second.last_check = std::chrono::system_clock::now();
			if (it->second.last_connection == std::chrono::system_clock::time_point()) it->second.last_connection = it->second.last_check;
SECUREEND_E_ENTERPRISE
			SaveSlavesSharedLicence(it->second, slave_server, true);	// has secure tags
		}
		else {
			dstream3 << "lic::Sharer Share license for slave='" << slave_server << "' failed!\n";
		}

		m_transport->PostRequest(slave_server.c_str(), nullptr, to_slave, nullptr, m_transport->OurService());
	}
}

bool SlaveIsInAllowedList(const string_view slave_id) {
	VS_RegistryKey shared_lic(false, SHARED_LIC_KEY_NAME);
	if (!shared_lic.IsValid()) return false;

	VS_RegistryKey slave_sub_key;
	shared_lic.ResetKey();
	while (shared_lic.NextKey(slave_sub_key)) {
		auto pName = slave_sub_key.GetName();
		if (!pName) continue;
		if (slave_id == pName) return true;
	}

	return false;
}

bool AllowAnySlave() {
	VS_RegistryKey shared_lic(false, SHARED_LIC_KEY_NAME);
	if (!shared_lic.IsValid()) return false;

	int32_t allow_any_slave(0);
	shared_lic.GetValue(&allow_any_slave, sizeof(allow_any_slave), VS_REG_INTEGER_VT, ALLOW_ANY_SLAVE_TAG);
	return allow_any_slave == 1;
}

VS_License GetLicenseRestriction(const std::string & slave_server)
{
	std::string sh_lic_key_name = SHARED_LIC_KEY_NAME; sh_lic_key_name += "\\"; sh_lic_key_name += slave_server; sh_lic_key_name += "\\"; sh_lic_key_name += RESTRICT_TAG;
	VS_RegistryKey sh_lic_key(false,sh_lic_key_name);
	VS_License res;
	if (!sh_lic_key.IsValid()) {
		res.m_error = VSS_LICENSE_NOT_VALID;
		return res;
	}
	res.ReadFromRegistry(sh_lic_key);	// has secure tags
	res.m_error = 0;
	return res;
}

void lic::Sharer::ProcessShareLicenseRequest(const VS_Container & cnt, const std::string & from)
{
	assert(p_licWrap != nullptr);
	assert(m_transport != nullptr);

	if (p_licWrap->IsMaster()) {
		if(AllowAnySlave() || SlaveIsInAllowedList(from)){
			VS_License arrivedLic;
			if (GetLicense(cnt, __FUNCTION__, arrivedLic)) {
				VS_License sharedLic, requestedLic, *pRequested_license = &arrivedLic;
				auto restriction = GetLicenseRestriction(from);	// has secure tags
SECUREBEGIN_E_ENTERPRISE
				if (restriction.m_error == 0) {
					dstream4 << "lic::Sharer slave server='" << from << "' has license restrictions.\n";
					auto shared_lic_it = m_shared_licenses.find(from);
					if (shared_lic_it != m_shared_licenses.end()) {
						restriction.DeductLicense(shared_lic_it->second.lic);
					}

					requestedLic = restriction.ShareAvailable(*pRequested_license);
					pRequested_license = &requestedLic;
				}

				sharedLic = p_licWrap->ShareMyLicense(*pRequested_license);
				sharedLic.m_id = arrivedLic.m_id;
SECUREEND_E_ENTERPRISE
				if (sharedLic.m_error == VSS_LICENSE_NOT_VALID) {
					ShareLicense(ShareResult::NO_SHARED, from);
				}
				else {
					ShareLicense(ShareResult::OK, from, &sharedLic);
				}
			}
		}
		else {
			dstream2 << "lic::Sharer license share request from not allowed slave '" << from << "'. Reject licence share.\n";
			ShareLicense(ShareResult::NO_SHARED, from);
		}
	}
}

bool lic::Sharer::ReceiveLicenseShare(const VS_Container & cnt, uint64_t& recvLicId)
{
	assert(p_licWrap != nullptr);
	assert(m_transport != nullptr);

	bool res(false);
	if (p_licWrap->IsSlave()) {

		--changeLic_requests_to_master;
		ShareResult result = ShareResult::NO_SHARED;
		cnt.GetValueI32(RESULT_PARAM, result);
		if (result != ShareResult::NO_SHARED) {
			VS_License arrivedLic;
			GetLicense(cnt, __FUNCTION__, arrivedLic);
			recvLicId = arrivedLic.m_id;
			bool save_license(false);
SECUREBEGIN_E_ENTERPRISE
			save_license = arrivedLic.HasSharedResources();
SECUREEND_E_ENTERPRISE
			if (save_license) {
SECUREBEGIN_E_ENTERPRISE
				p_licWrap->AddSharedLicense(arrivedLic);
SECUREEND_E_ENTERPRISE
				res = SaveReceivedSharedLicence(p_licWrap->GetSharedLicSum());
			}
			else{
				dstream3 << "Warning\tlic::Sharer Received empty license share!\n'";
			}
		}
	}

	return res;
}

// Temporary hack to fix applying VMProtect on Windows, until VMProtect is integrated properly.
#if defined(ENABLE_VMPROTECT_BUILD) && defined(_MSC_VER)
#pragma optimize("", off)
#endif
void lic::Sharer::RestoreSharedLicenses()
{
	assert(p_licWrap != nullptr);
	if (p_licWrap->IsMaster()) {

		VS_RegistryKey sh_lic_key(false, SHARED_LIC_KEY_NAME);

		VS_RegistryKey lic_key;
		sh_lic_key.ResetKey();
		while (sh_lic_key.NextKey(lic_key)) {	// deduct all shared licenses from license of master
			auto server = lic_key.GetName();
			if (!server) server = "empty";

			VS_License license;
			if (LoadLicense(lic_key, license)) {
SECUREBEGIN_E_ENTERPRISE
				std::chrono::system_clock::time_point last_check = GetLastCheck(lic_key);
				if (last_check != std::chrono::system_clock::time_point()) {
					m_shared_licenses.emplace(server, lic::Sharer::ShareLicenseInfo(license, last_check));
					p_licWrap->ShareMyLicense(license);
					dstream3 << "Restored shared license for slave '" << server << "' \n";
					license.Print();
				}
SECUREEND_E_ENTERPRISE
			}
			else {
				dstream3 << "Warning\tMaster didn't share any resources yet for " << server << ".\n";
			}

		}
	}

	if (p_licWrap->IsSlave()) {	// add shared license to license of slave
		VS_License license;
		if (LoadLicense(SHARED_LIC_KEY_NAME, license)) {
SECUREBEGIN_E_ENTERPRISE
			VS_RegistryKey sh_lic_key(false, SHARED_LIC_KEY_NAME);
			std::chrono::system_clock::time_point last_conn = GetLastCheck(sh_lic_key);
			if (last_conn != std::chrono::system_clock::time_point()) {
				p_licWrap->AddSharedLicense(license);
				dstream3 << "lic::Sharer Shared license was restored\n";
				license.Print();
			}
SECUREEND_E_ENTERPRISE
		}
		else {
			dstream3 << "Warning\tSlave didn't requested any shared resources yet.\n";
		}
	}
}
#if defined(ENABLE_VMPROTECT_BUILD) && defined(_MSC_VER)
#pragma optimize("", on)
#endif

void lic::Sharer::ClearSharedLicenseOnMasterSide(const std::string & master)
{
	assert(p_licWrap != nullptr);
	assert(m_transport != nullptr);

	if (p_licWrap->IsSlave()) {
		VS_License empty;
		VS_Container to_master;
		to_master.AddValue(METHOD_PARAM, SHARED_LICENSE_CHECK_METHOD);
		AddLicense(to_master, empty, __FUNCTION__);

		m_transport->PostRequest(master.c_str(), nullptr, to_master, nullptr, m_transport->OurService());
	}
}

bool SuppressLicenseCheck(unsigned short &changeLic_requests_to_master) {
	static std::chrono::steady_clock::time_point last_supperessed_check_time;

	if (changeLic_requests_to_master == 0) {
		last_supperessed_check_time = decltype(last_supperessed_check_time)();
		return false;
	}

	if (last_supperessed_check_time != decltype(last_supperessed_check_time)() && vs::chrono::abs(std::chrono::steady_clock::now() - last_supperessed_check_time) > CHECK_SUPPRESSING_TIMEOUT) {
		dstream2 << "Warning License check suppressing timeout!\n";
		last_supperessed_check_time = decltype(last_supperessed_check_time)();
		changeLic_requests_to_master = 0;
		return false;
	}

	if (changeLic_requests_to_master != 0) {
		last_supperessed_check_time = std::chrono::steady_clock::now();
		return true;
	}

	return false;
}

void SetLicenseMasterStatus(lic::LicenseCheckStatus st) {
	VS_RegistryKey sh_lic(false, SHARED_LIC_KEY_NAME, false);
	sh_lic.SetValue(&st, sizeof(st), VS_REG_INTEGER_VT, MASTER_STATUS_TAG);

	dstream3 << "lic::Sharer Set master server status to '" << static_cast<uint32_t>(st) << "'\n";
}

void lic::Sharer::SendSharedLicenseCheck(const std::string &master)
{
	assert(p_licWrap != nullptr);
	assert(m_transport != nullptr);

SECUREBEGIN_E_ENTERPRISE
	auto suppress_check = SuppressLicenseCheck(changeLic_requests_to_master);
	if(suppress_check) dstream2 << "License check to master '" << master << "' is suppressed! Do not have responses on " << changeLic_requests_to_master << " requests\n";

	if (p_licWrap->IsSlave() && !suppress_check) {
		VS_RegistryKey sh_lic(false, SHARED_LIC_KEY_NAME);
		if (sh_lic.IsValid()) {
			auto shared_lic = p_licWrap->GetSharedLicSum();
			VS_Container to_master;
			to_master.AddValue(METHOD_PARAM, SHARED_LICENSE_CHECK_METHOD);
			AddLicense(to_master, shared_lic, __FUNCTION__);

			m_transport->PostRequest(master.c_str(), nullptr, to_master, nullptr, m_transport->OurService());

			SetLicenseMasterStatus(LicenseCheckStatus::checking);
			m_last_check_request = std::chrono::steady_clock::now();
		}
	}
SECUREEND_E_ENTERPRISE
}

void lic::Sharer::ReceiveSharedLicenseCheck(const VS_Container& cnt, const std::string &slave_server)
{
	assert(p_licWrap != nullptr);
	assert(m_transport != nullptr);
	if (!p_licWrap->IsMaster()) return;

	VS_Container to_slave;
	VS_License arrivedLic;
	if (GetLicense(cnt, __FUNCTION__, arrivedLic)) {
		bool slave_is_allowed = SlaveIsInAllowedList(slave_server) || AllowAnySlave();
		if (!slave_is_allowed) {
			dstream3 << "lic::Sharer Received license check from not allowed slave '" << slave_server << "'\n";
			LogEvent(SLAVE_SERVER_OBJ, slave_server, "LicenseCheck", "Received license check from not allowed slave");
		}

		auto it = m_shared_licenses.find(slave_server);
		if (it != m_shared_licenses.end() && slave_is_allowed) {
			it->second.last_check = std::chrono::system_clock::now();

			if (it->second.lic.CompareCountableResources(arrivedLic)) {
				SaveTimeForSlave(slave_server, LAST_CHECK_TAG, it->second.last_check);

				to_slave.AddValue(METHOD_PARAM, SHARED_LICENSE_CHECK_METHOD_RESP);
				to_slave.AddValueI32(RESULT_PARAM, LicenseCheckStatus::allowed);
			}
			else {
				dstream3 << "Error\tLicense check from '" << slave_server << "' failed. Licenses are not equal!\n";
				p_licWrap->ReturnBackSharedResourses(it->second.lic);				// return slaves license back
				it->second.lic.DeductLicense(it->second.lic);						// clear slaves license
				SaveSlavesSharedLicence(it->second, slave_server);					// save

				to_slave.AddValue(METHOD_PARAM, SHARED_LICENSE_CHECK_METHOD_RESP);
				to_slave.AddValueI32(RESULT_PARAM, LicenseCheckStatus::check_failed);			// require to clear own license for slave
				LogEvent(SLAVE_SERVER_OBJ, slave_server, "LicenseCheck", "Check failure. License are not equal!");
			}
		}
		else {
			dstream3 << "Warning\tLicense check from '" << slave_server << "' failed. Master has no information about this license!\n";
			to_slave.AddValue(METHOD_PARAM, SHARED_LICENSE_CHECK_METHOD_RESP);
			to_slave.AddValueI32(RESULT_PARAM, LicenseCheckStatus::not_allowed);
		}
	}
	else {
		dstream3 << "Warning\tLicense check from '" << slave_server << "' failed. License not present in check!\n";
		to_slave.AddValue(METHOD_PARAM, SHARED_LICENSE_CHECK_METHOD_RESP);
		to_slave.AddValueI32(RESULT_PARAM, LicenseCheckStatus::not_allowed);
	}

	m_transport->PostRequest(slave_server.c_str(), nullptr, to_slave, nullptr, m_transport->OurService());
}

VS_License CalculateNotFittedInMyLicense() {
	auto used_by_me = p_licWrap->GetMyUsedLicenseResources();
	auto i_have = p_licWrap->GetLicSum();

	return used_by_me.DeductLicenseCopy(i_have);
}

void lic::Sharer::ReceiveSharedLicenseCheckResponse(const VS_Container & cnt, VS_License& overhead)
{
	assert(p_licWrap != nullptr);
	if (!p_licWrap->IsSlave()) return;

	s_resolvedMasterActive = true;
	auto result = LicenseCheckStatus::not_allowed;
	cnt.GetValueI32(RESULT_PARAM, result);
	if (result == LicenseCheckStatus::not_allowed || result == LicenseCheckStatus::check_failed) {
		dstream3 << "Warning\tLicense check is failed with result '" << static_cast<uint32_t>(result) << "'. License will be cleared!\n";
		LogEvent(SERVER_OBJECT_TYPE, m_transport->OurEndpoint(), "LicenseCheckResponse", "Check failure");
		SetStartLicenseForSlave();
		ClearMyLicense();
		overhead = CalculateNotFittedInMyLicense();
	}
	else {
		UpdateTimeOnMySharedLicense(std::chrono::system_clock::now());
	}

	if (result != LicenseCheckStatus::check_failed)	SetLicenseMasterStatus(result);	// Save to registry all statuses except license check failure
}

int32_t CalculateOverhead(int32_t what_i_have, int32_t what_i_use) {
	int32_t res =  what_i_have - what_i_use * 1;
	res = std::max(res, 0);
	return res;
}

VS_License CalculateLicenseOverhead(const VS_License& what_i_have, const VS_License& what_i_use) {
	VS_License res;
	res.m_onlineusers = CalculateOverhead(what_i_have.m_onlineusers, what_i_use.m_onlineusers);
	res.m_gateways = CalculateOverhead(what_i_have.m_gateways, what_i_use.m_gateways);
	res.m_max_guests = CalculateOverhead(what_i_have.m_max_guests, what_i_use.m_max_guests);
	res.m_terminal_pro_users = CalculateOverhead(what_i_have.m_terminal_pro_users, what_i_use.m_terminal_pro_users);
	return res;
}

void lic::Sharer::ObserveSharedResourses()
{
	assert(p_licWrap != nullptr);
SECUREBEGIN_E_ENTERPRISE
	if (p_licWrap->IsMaster()) {
		auto now = std::chrono::system_clock::now();
		for (auto it = m_shared_licenses.begin(); it != m_shared_licenses.end(); ++it) {
			if (it->second.last_check == std::chrono::system_clock::time_point()) continue;

			bool in_allowed_list = SlaveIsInAllowedList(it->first) || AllowAnySlave();
			const bool check_timeout = vs::chrono::abs(now - it->second.last_check) > LAST_CHECK_TIMEOUT;
			if (!in_allowed_list) dstream2 << "lic::Sharer Slave='" << it->first << "' is not present in allowed list. Return license back.\n";
			else if (check_timeout)
				dstream3 << "lic::Sharer Check timeout for slave='" << it->first << "'. Return license back.\n";

			if (check_timeout || !in_allowed_list) {
				p_licWrap->ReturnBackSharedResourses(it->second.lic);
				ClearSlaveSharedLicense(it->first);
				it->second.lic = decltype(it->second.lic)();
				it->second.last_check = decltype(it->second.last_check)();
			}
			else {
				ObserveRestriction(*it);
			}
		}
	}
SECUREEND_E_ENTERPRISE
}

VS_License& MaximizeInfinity(VS_License& what) {
	if (what.m_onlineusers == VS_License::TC_INFINITY) what.m_onlineusers = INT_MAX;
	if (what.m_gateways == VS_License::TC_INFINITY) what.m_gateways = INT_MAX;
	if (what.m_max_guests == VS_License::TC_INFINITY) what.m_max_guests = INT_MAX;
	if (what.m_terminal_pro_users == VS_License::TC_INFINITY) what.m_terminal_pro_users = INT_MAX;
	return what;
}

VS_License& SubstractLicense(VS_License& from, const VS_License& to_sub) {
	from.m_onlineusers -= to_sub.m_onlineusers;
	from.m_gateways -= to_sub.m_gateways;
	from.m_max_guests -= to_sub.m_max_guests;
	from.m_terminal_pro_users -= to_sub.m_terminal_pro_users;
	return from;
}

VS_License& RevertNegative(VS_License& what) {
	what.m_onlineusers = what.m_onlineusers < 0? -what.m_onlineusers: 0;
	what.m_gateways = what.m_gateways < 0 ? -what.m_gateways : 0;
	what.m_max_guests = what.m_max_guests < 0 ? -what.m_max_guests : 0;
	what.m_terminal_pro_users = what.m_terminal_pro_users < 0 ? -what.m_terminal_pro_users : 0;
	return what;
}

void lic::Sharer::ObserveRestriction(const std::pair<std::string /* server */, ShareLicenseInfo>& slave_info)
{
	assert(p_licWrap != nullptr);

	auto restriction = GetLicenseRestriction(slave_info.first);	// has secure tags
	if (restriction.m_error == 0 && p_licWrap->IsMaster()) {
		dstream4 << "lic::Sharer check restrictions for slave server='" << slave_info.first << "'\n";
		VS_License to_return = restriction;
		const auto &curr_license = slave_info.second.lic;

		to_return = MaximizeInfinity(to_return);
		to_return = SubstractLicense(to_return, curr_license);			// get difference
		to_return = RevertNegative(to_return);							// make differense positive

		if (to_return.HasSharedResources()) {
			dstream4 << "Request license returning by force!\n";
			ReturnLicense(slave_info.first, to_return);
		}
	}
}

void lic::Sharer::ReturnLicense(const std::string & slave, const VS_License& to_return)
{
	assert(p_licWrap != nullptr);
	assert(to_return.m_id != 0u);
SECUREBEGIN_E_ENTERPRISE
	if (p_licWrap->IsMaster()) {
		VS_Container to_slave;
		to_slave.AddValue(METHOD_PARAM, RETURN_LICENSE_FORCE_TAG);
		AddLicense(to_slave, to_return, __FUNCTION__);
		m_transport->PostRequest(slave.c_str(), nullptr, to_slave, nullptr, m_transport->OurService());
	}
SECUREEND_E_ENTERPRISE
}

bool lic::Sharer::ObserveReceivedSharedResources(VS_License& OUT_to_free)
{
	assert(p_licWrap != nullptr);

	bool res(false);
SECUREBEGIN_E_ENTERPRISE
	if (p_licWrap->IsSlave()) {
		auto last_succsessfull_check = GetMySharedLicenseUpdateTime();
		std::chrono::duration<std::chrono::system_clock::rep, std::chrono::system_clock::period> check_period;
		if (last_succsessfull_check == std::chrono::system_clock::time_point()) {
			res = false;
		}
		else if ((check_period = vs::chrono::abs(std::chrono::system_clock::now() - last_succsessfull_check)) > LAST_CHECK_TIMEOUT) {
			dstream3 << "Warning\tConnection timeout! License will be cleared!\n";

			char mbstr[128] = {0};
			tu::TimeToGStr(last_succsessfull_check, mbstr, 128);
			dstream3 << "Last succsessfull check time is " << mbstr;
			dstream3 << "Difference is '" << std::chrono::duration_cast<std::chrono::minutes>(check_period).count() << "' minutes\n";

			SetStartLicenseForSlave();	// in this case slave will clear received license and must request new license from master if it have such need
			ClearMyLicense();
			ClearSharedLicenseOnMasterSide(GetMasterServer());
			OUT_to_free = CalculateNotFittedInMyLicense();
			res = false;
		}
		else {
			res = true;
		}
	}
SECUREEND_E_ENTERPRISE
	return res;
}

void lic::Sharer::ObserveLicenseOverhead()
{
	assert(p_licWrap != nullptr);
	if (p_licWrap->IsSlave()) {
SECUREBEGIN_E_ENTERPRISE
		for (auto it = m_licOverhead.begin(); it != m_licOverhead.end();) {
			auto& licRequest = it->second;

			// when master is not answering, clear request and restore licenses back
			if ((std::chrono::steady_clock::now() - licRequest.reqTime) > RETURN_LIC_OVERHEAD_TIMEOUT) {
				p_licWrap->AddSharedLicense(licRequest.lic);
				SaveReceivedSharedLicence(p_licWrap->GetSharedLicSum());
				it = m_licOverhead.erase(it);
				continue;
			}
			++it;
		}
SECUREEND_E_ENTERPRISE
	}
}

void lic::Sharer::ReturnLicenseOverhead(const string_view master_server)
{
	assert(p_licWrap != nullptr);
	if (p_licWrap->IsSlave()) {
SECUREBEGIN_E_ENTERPRISE
		auto overhead = CalculateLicenseOverhead(p_licWrap->GetLicSum(), p_licWrap->GetMyUsedLicenseResources());
		auto res = p_licWrap->GetSharedLicSum().ShareAvailable(overhead);
		if (res.HasSharedResources()) {
			ReturnSharedResoursesToMaster(res, master_server);
		}
SECUREEND_E_ENTERPRISE
	}
}

void lic::Sharer::SetStartLicenseForSlave()
{
	assert(p_licWrap != nullptr);
	if (!p_licWrap->IsSlave()) return;

	p_licWrap->ClearSharedLicense();
SECUREBEGIN_E_ENTERPRISE
	auto &received_sh_licecnse = (VS_License&)p_licWrap->GetSharedLicSum();
	received_sh_licecnse.m_error = 0;
	received_sh_licecnse.m_conferences = VS_License::TC_INFINITY;
SECUREEND_E_ENTERPRISE
}

void lic::Sharer::ReturnSharedResoursesToMaster(VS_License & l, const string_view master_server)
{
	assert(p_licWrap != nullptr);
	assert(m_transport != nullptr);
	if (!p_licWrap->IsSlave()) return;

	l.m_id = GetLicID();
	VS_Container to_master;
	to_master.AddValue(METHOD_PARAM, RETURN_SHARED_LICENSE_METHOD);
	AddLicense(to_master, l, __FUNCTION__);

	ReturnLicenseReq r{ l, std::chrono::steady_clock::now() };
	m_licOverhead.emplace(l.m_id, std::move(r));
	p_licWrap->DeductSharedLicense(l);
	SaveReceivedSharedLicence(p_licWrap->GetSharedLicSum());
	++changeLic_requests_to_master;
	m_transport->PostRequest(std::string(master_server).c_str(), nullptr, to_master, nullptr, m_transport->OurService());
}

void lic::Sharer::ReceiveReturnedSharedResources(const VS_Container & cnt, const std::string& slave_server)
{
	assert(p_licWrap != nullptr);
	assert(m_transport != nullptr);
	if (!p_licWrap->IsMaster()) return;

	VS_Container to_slave;
	to_slave.AddValue(METHOD_PARAM, RETURN_SHARED_LICENSE_METHOD_RESP);

	VS_License arrivedLic;
	if (!GetLicense(cnt, __FUNCTION__, arrivedLic)) {
		dstream3 << "lic::Sharer Master received empty license to return back.\n";
		to_slave.AddValueI32(RESULT_PARAM, ShareResult::NO_SHARED);
		m_transport->PostRequest(slave_server.data(), nullptr, to_slave, nullptr, m_transport->OurService());
	}
	else {
		auto it = m_shared_licenses.find(slave_server);
		if (it == m_shared_licenses.end()) {
			dstream3 << "lic::Sharer Master can't find slave='" << slave_server << "' to accept return back license.\n";
			to_slave.AddValueI32(RESULT_PARAM, ShareResult::NO_SHARED);
			m_transport->PostRequest(slave_server.data(), nullptr, to_slave, nullptr, m_transport->OurService());
		}
		else {
			dstream3 << "lic::Sharer Master accepted returned back license from slave='" << slave_server << "'\n";
			it->second.lic.DeductLicense(arrivedLic);
			it->second.last_check = std::chrono::system_clock::now();
			SaveSlavesSharedLicence(it->second, slave_server);
			p_licWrap->ReturnBackSharedResourses(arrivedLic);

			to_slave.AddValueI32(RESULT_PARAM, ShareResult::OK);
			AddLicense(to_slave, arrivedLic, __FUNCTION__);
			m_transport->PostRequest(slave_server.data(), nullptr, to_slave, nullptr, m_transport->OurService());
		}
	}
}

void lic::Sharer::ReceiveReturnedSharedResourcesResp(const VS_Container & cnt, VS_License& overhead)
{
	assert(p_licWrap != nullptr);
	if (!p_licWrap->IsSlave()) return;

	ShareResult result = ShareResult::NO_SHARED;
	cnt.GetValueI32(RESULT_PARAM, result);

	--changeLic_requests_to_master;
	if (result != ShareResult::NO_SHARED) {
		dstream3 << "Master accepted returned back shared license!\n";
		VS_License arrivedLic;
		if (GetLicense(cnt, __FUNCTION__, arrivedLic)) {
			auto it = m_licOverhead.find(arrivedLic.m_id);
			if (it == m_licOverhead.end()) {
				dstream3 << "lic::Sharer::ReceiveReturnedSharedResourcesResp Error! Return license request id="<< arrivedLic.m_id << " wasn't found";
				return;
			}
			m_licOverhead.erase(it);
			overhead = CalculateNotFittedInMyLicense();
		}
	}
	else {
		dstream3 << "Master can't accept returned back shared license!\n";
	}
}

void SaveCurrentTimeForSlave(const string_view slave_id, const char* time_tag) {
	std::string key_name = SHARED_LIC_KEY_NAME; key_name += "\\"; key_name += slave_id;
	VS_RegistryKey sh_lic_key(false, key_name, false);
	if (!sh_lic_key.IsValid()) return;

	char str[128] = { 0 };
	if (tu::TimeToNStr(std::chrono::system_clock::now(), str, 128) > 0)
		sh_lic_key.SetString(str, time_tag);
}

void lic::Sharer::SlaveConnectedEvent(const string_view slave_id)
{
	assert(p_licWrap != nullptr);
	if (!p_licWrap->IsMaster()) return;
	if (VS_GetServerType(slave_id) != ST_VCS) return;

	bool allow_any_slave = AllowAnySlave();
	if (SlaveIsInAllowedList(slave_id))
		SaveCurrentTimeForSlave(slave_id, LAST_CONN_TAG);
	else if (!allow_any_slave)
		dstream3 << "lic::Sharer Connection from NOT allowed slave '" << slave_id << "'!\n";

	auto it = m_shared_licenses.find(static_cast<const std::string>(slave_id));
	if(it == m_shared_licenses.end()) it = m_shared_licenses.emplace(std::string(slave_id), ShareLicenseInfo()).first;
	it->second.last_connection = std::chrono::system_clock::now();

}
void lic::Sharer::SlaveDisconnectedEvent(const string_view slave_id)
{
	assert(p_licWrap != nullptr);
	if (!p_licWrap->IsMaster()) return;
	if (VS_GetServerType(slave_id) != ST_VCS) return;
	if (SlaveIsInAllowedList(slave_id))
		SaveCurrentTimeForSlave(slave_id, LAST_DISCONN_TAG);
	else if (!AllowAnySlave())
		dstream3 << "lic::Sharer Disconnect from NOT allowed slave '" << slave_id << "'!\n";
}

void lic::Sharer::MasterDisconnectedEvent(const string_view master_id)
{
	assert(p_licWrap != nullptr);
	if (!p_licWrap->IsSlave()) return;
	if (VS_GetServerType(master_id) != ST_VCS) return;

	dstream3 << "Master server '" << master_id << "' is disconnected\n";
	if (GetMasterServer() == master_id) {
		SaveResolvedServerName("");
		ResolveMasterServer();	// maybe we have new master, try to resolve it
	}
}

void lic::Sharer::VerifyMasterStatus()
{
	assert(p_licWrap != nullptr);
	if (!p_licWrap->IsSlave()) return;

	if (!s_resolvedMasterActive
		&& m_last_check_request != decltype(m_last_check_request)()
		&& std::chrono::steady_clock::now() - m_last_check_request > CHECK_RESPONSE_TIMEOUT)
	{
		dstream2 << "lic::Sharer Master status veryfication failed!\n";
		SetLicenseMasterStatus(LicenseCheckStatus::no_answer);
	}
}

std::string lic::Sharer::GetMasterServer(bool do_resolve)
{
	std::string result;
	assert(p_licWrap != nullptr);
	if (!p_licWrap->IsSlave())
		return result;

	VS_RegistryKey key(false, SHARED_LIC_KEY_NAME);
	if (key.GetString(result, RESOLVED_MASTER_NAME_TAG))
		return result;

	if (do_resolve) {
		dstream2 << "lic::Sharer Master server name wasn't found. Try to resolve a new one.\n";
		result = ResolveMasterServer();
	}

	return result;
}

std::string lic::Sharer::ResolveMasterServer()
{
	std::string result;
	assert(p_licWrap != nullptr);
	if (!p_licWrap->IsSlave())
		return result;

	VS_RegistryKey key(false, CONFIGURATION_KEY);
	if (!key.GetString(result, LICENSE_MASTER_TAG))
		return result;

	if (result.find_first_of('#') != result.npos) {
		SaveResolvedServerName(result.c_str());	// already have resolved name
		return result;
	}

	boost::system::error_code ec;
	int port(4307);
	auto address = net::address::from_string(result, ec);
	string_view master_server_sv(result);
	auto port_pos = string_view::npos;

	if (address.is_unspecified() || address.is_v4())
		port_pos = master_server_sv.find_last_of(':');	// hostname or ipv4 => search port by semicolon ':'
	else if (address.is_v6()) {
		port_pos = master_server_sv.find("]:");			// ipv6 => search port by "]:"
		if (port_pos != string_view::npos) ++port_pos;
	}

	if (port_pos != string_view::npos && port_pos != 0) {
		auto port_str = static_cast<std::string>(master_server_sv.substr(port_pos + 1));
		int parsed_port(0);
		if ((parsed_port = strtol(port_str.c_str(), nullptr, 10)) != 0) {
			port = parsed_port;
			master_server_sv = master_server_sv.substr(0, port_pos);
		}

	}

	//dns::VS_SRV_Endpoint ep(static_cast<std::string>(master_server_sv), port); //TODO: unused
	std::string resolved_server_name;
	VS_ResolveServerFinder::GetServerNameByHostPort(std::string(master_server_sv), port, resolved_server_name);
	SaveResolvedServerName(resolved_server_name.c_str());

	return resolved_server_name;
}

#include "ProtectionLib/OptimizeEnable.h"