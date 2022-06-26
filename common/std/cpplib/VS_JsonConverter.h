#pragma once

#include "std-generic/cpplib/VS_Container.h"
#include "json/writer.h"

#include <set>
#include <string>

json::Object ConvertToJson(const VS_Container &cnt, const std::set<std::string> &exclude_params = {});
std::string ConvertToJsonStr(const VS_Container &cnt, const std::set<std::string> &exclude_params = {});
