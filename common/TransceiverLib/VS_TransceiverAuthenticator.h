#pragma once

#include "VS_AuthConnectionInterface.h"
#include <string>
#include "std-generic/cpplib/string_view.h"

namespace auth {
	class Transceiver : public VS_AuthConnectionInterface {
		std::string m_authenticatedName;
	public:
		bool AuthConnection(const unsigned char *data, const unsigned long data_sz) override;
		const std::string& GetAuthenticatedName() const override;
		static std::string ReadSharedKey(string_view transceiverName);
		static bool SaveSharedKey(string_view transceiverName, const char* key);
		static void CleanSharedKey(string_view transceiverName);
		static string_view GetLoginFromHandshake(const string_view input);
	};
}