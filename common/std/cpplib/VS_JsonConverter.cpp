#include "VS_JsonConverter.h"

json::Object ConvertToJson(const VS_Container &cnt, const std::set<std::string> &exclude_params)
{
	json::Object result;

	cnt.Reset();
	while (cnt.Next()) {
		// skip excluded params
		if (exclude_params.find(cnt.GetName()) != exclude_params.end()) {
			continue;
		}
		switch (cnt.GetType()) {
		case VS_CNT_BOOL_VT: {
			bool value;
			if (cnt.GetValue(value)) {
				result.Insert(json::Object::Member(cnt.GetName(), json::Boolean(value)));
			}
		} break;
		case VS_CNT_INTEGER_VT: {
			int32_t value;
			if (cnt.GetValue(value)) {
				result.Insert(json::Object::Member(cnt.GetName(), json::Number(value)));
			}
		} break;
		case VS_CNT_DOUBLE_VT: {
			double value;
			if (cnt.GetValue(value)) {
				result.Insert(json::Object::Member(cnt.GetName(), json::Number(value)));
			}
		} break;
		case VS_CNT_STRING_VT: {
			const char* value = cnt.GetStrValueRef();
			if (value) {
				result.Insert(json::Object::Member(cnt.GetName(), json::String(value)));
			}
		} break;
		case VS_CNT_INT64_VT: {
			int64_t value;
			if (cnt.GetValue(value)) {
				result.Insert(json::Object::Member(cnt.GetName(), json::Number(value)));
			}
		} break;
		}
	}

	return result;
}

std::string ConvertToJsonStr(const VS_Container &cnt, const std::set<std::string> &exclude_params)
{
	json::Object resultJson = ConvertToJson(cnt, exclude_params);
	std::stringstream resultStream;
	json::Writer::Write(resultJson, resultStream);
	return resultStream.str();
}
