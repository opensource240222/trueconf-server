#include "VS_SIPInstantMessage.h"

VS_SIPInstantMessage::VS_SIPInstantMessage(){}
VS_SIPInstantMessage::~VS_SIPInstantMessage(){}

TSIPErrorCodes VS_SIPInstantMessage::Decode(VS_SIPBuffer &inBuffer){

	Clear();

	std::unique_ptr<char[]> ptr;
	std::size_t ptr_sz = 0;


	const TSIPErrorCodes err = inBuffer.GetAllDataAllocConst(ptr, ptr_sz);
	if (TSIPErrorCodes::e_ok != err)
	{
		SetValid(false);
		SetError(err);
		return err;
	}

	if (!ptr || !ptr_sz)
	{
		SetValid(false);
		SetError(TSIPErrorCodes::e_buffer);
		return TSIPErrorCodes::e_buffer;
	}

	m_message = ptr.get();

	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);
	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SIPInstantMessage::Encode(VS_SIPBuffer &outBuffer) const{
	return outBuffer.AddData(m_message.c_str(), m_message.length() + 1);
}

TSIPErrorCodes VS_SIPInstantMessage::Init(const VS_SIPGetInfoInterface& call){

	return TSIPErrorCodes::e_ok;
}

TSIPErrorCodes VS_SIPInstantMessage::Init(const VS_SIPGetInfoInterface& info, string_view message){
	if (Init(info) != TSIPErrorCodes::e_ok)
		return TSIPErrorCodes::e_InputParam;

	m_message = std::string(message);
	SetValid(true);
	SetError(TSIPErrorCodes::e_ok);

	return TSIPErrorCodes::e_ok;
}

void VS_SIPInstantMessage::Clear(){
	m_message.clear();
	SetValid(false);
	SetError(TSIPErrorCodes::e_null);
}

const std::string &VS_SIPInstantMessage::GetMessageText() const
{
	return m_message;
}