#pragma once

#include <string>
#include <functional>
#include <stdint.h>

enum class ServerNameByAddressResult
{
	res_ok,
	err_connect_faild,
	err_write_faild,
	err_read_faild,
	err_unknow

};

bool VS_GetServerNameByAddress(const std::string& addr, const uint16_t port, const std::function<void(const std::string&, const ServerNameByAddressResult&)>&func, const bool isAsync = false);