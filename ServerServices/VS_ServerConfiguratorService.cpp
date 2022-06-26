#include "VS_ServerConfiguratorService.h"
#include "ServerServices/VS_ConfRestrictInterface.h"
#include "../common/std/cpplib/json/writer.h"


#define DEBUG_CURRENT_MODULE VS_DM_SERVCONFIG

VS_ServerConfiguratorService::VS_ServerConfiguratorService()
{
}

VS_ServerConfiguratorService::~VS_ServerConfiguratorService()
{}

const VS_PushDataSignalSlot VS_ServerConfiguratorService::GetPushDataSignalSlot()
{
	return boost::bind(&VS_ServerConfiguratorService::SendConfiguratorCommand,this,_1,_2);
}
bool VS_ServerConfiguratorService::Init(const char *our_endpoint, const char *our_service, const bool permittedAll)
{
	return true;
}
bool VS_ServerConfiguratorService::Processing(std::unique_ptr<VS_RouterMessage>&&recvMess)
{
	return true;
}

bool VS_ServerConfiguratorService::SendConfiguratorCommand(const char *buf, uint32_t buf_len)
{
	if (!buf || buf_len == 0) {
		dstream2 <<"SRVConfig\nError Empty param \n";
		return false;
	}
	dstream4 << "SRVConfig: received content len = " << buf_len;
	if (!buf) {
		dstream2 << "SRVConfig\nError Empty param \n";
		return false;
	}
	std::stringstream post_data;
	post_data << string_view(buf,buf_len);
	try
	{
		json::Object rootObj;
		json::Reader::Read(rootObj, post_data);
		VS_Container cnt;
		auto service_i = rootObj.Find("Service");
		if(service_i==rootObj.End())
			return false;
		const json::String dst_service = service_i->element;
		rootObj.Erase(service_i);
		if(!JsonToContainer(rootObj,cnt))
			return false;
		return SendCommandToService(dst_service,cnt);
	}
	catch(json::Exception &e)
	{
		dstream1 << "SRVConfig\nError Parsing of post request: " << e.what();
	}
	return false;
}

bool VS_ServerConfiguratorService::JsonToContainer(json::Object &src, VS_Container &dst)
{
	std::string method = src.Begin()->name;
	dst.AddValue(METHOD_PARAM,method.c_str());
	json::Object params = src[method];
	for(json::Object::const_iterator i = params.Begin();i!=params.End();i++)
	{
		try //boolean
		{
			json::Boolean num_value = i->element;
			dst.AddValue(i->name, num_value);
			continue;
		}
		catch(json::Exception &)
		{
			// bad cast;
		}

		try //numeric
		{
			json::Number num_value = i->element;
			dst.AddValueI32(i->name, num_value);
			continue;
		}
		catch(json::Exception &)
		{
			// bad cast;
		}

		try //string
		{
			json::String str_value = i->element;
			dst.AddValue(i->name, str_value.Value().c_str());
			continue;
		}
		catch(json::Exception &)
		{
			//bad cast;
		}

		try // array
		{
			json::Array arr = i->element;
			for(json::Array::const_iterator arr_i = arr.Begin();arr_i!=arr.End();arr_i++)
			{
				try
				{

					json::Number num_value = *arr_i;
					dst.AddValueI32(i->name, num_value);
					continue;
				}
				catch(json::Exception &)
				{}
				try
				{
					json::String str_value = *arr_i;
					dst.AddValue(i->name, str_value.Value().c_str());
					continue;
				}
				catch(json::Exception &)
				{}
			}
		}
		catch(json::Exception &)
		{}

		try // Object
		{
			json::Object obj(i->element);
			std::stringstream ss;
			json::Writer::Write(obj, ss);
			if (ss.str().length() > 0)
				dst.AddValue(i->name, ss.str().c_str());
			continue;
		}
		catch (json::Exception &)
		{
		}

		try // Null
		{
			json::Null obj(i->element);
			dst.AddValue(i->name, "");
			continue;
		}
		catch (json::Exception &)
		{
		}

	}
	return !dst.IsEmpty();
}
void VS_ServerConfiguratorService::SetConfRestrict(const boost::shared_ptr<VS_ConfRestrictInterface>& confRestrict)
{
	m_confRestrict = confRestrict;
}

bool VS_ServerConfiguratorService::SendCommandToService(const std::string& dst_srv,VS_Container &cnt)
{
	if(!m_confRestrict)
		return false;
	if(!m_confRestrict->CheckSessionID(cnt.GetStrValueRef(SESSION_PARAM)))
		return false;
	return PostRequest(OurEndpoint(),0,cnt,0,dst_srv.c_str());
}