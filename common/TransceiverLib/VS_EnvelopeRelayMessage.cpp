#include "VS_EnvelopeRelayMessage.h"
#include "std-generic/clib/strcasecmp.h"

VS_EnvelopeRelayMessage::VS_EnvelopeRelayMessage(const char* module_name)
{
	m_name = (char*)malloc(strlen(module_name)+1);
	strncpy(m_name, module_name, strlen(module_name)+1);
}

VS_EnvelopeRelayMessage::~VS_EnvelopeRelayMessage()
{
	free(m_name);
}

bool VS_EnvelopeRelayMessage::IsValid() const
{
	return ((GetMessageType()==e_envelope) &&
		(GetModuleName() && strncasecmp(m_name, GetModuleName(), strlen(m_name))==0))? true: false;
}

bool VS_EnvelopeRelayMessage::SetMessage(const boost::shared_ptr<std::vector<unsigned char> > &mess)
{
	m_mess = mess;
	m_mess_size = m_mess->size();
	m_isComplete = true;		// kt: set temporary, beause IsValid() need it to set correct value
	if(IsValid()){
		DeserializeContainer(GetModuleBody(),GetContainerLen());
	}
	return IsValid();
}

unsigned char *VS_EnvelopeRelayMessage::SerializeContainer(size_t &size) const
{
	void* buf;
	if(!m_cnt.SerializeAlloc(buf,size)) return 0;
	return (unsigned char*)buf;
}

bool VS_EnvelopeRelayMessage::DeserializeContainer(const unsigned char *buf, const size_t size)
{
	return m_cnt.Deserialize((void*)buf,size);
}

unsigned long VS_EnvelopeRelayMessage::GetModuleBodyIndex() const
{
	unsigned long pos = GetModuleNameIndex();
	if (pos==-1)
		return -1;
	return pos + strlen(reinterpret_cast<const char*>(&(*m_mess)[0]+pos)) + 1;
}

unsigned long VS_EnvelopeRelayMessage::GetBodyLen() const
{
	return reinterpret_cast<RelayMessFixedPart*>(&(*m_mess)[0])->body_len;
}

unsigned char *VS_EnvelopeRelayMessage::GetModuleBody() const
{

	unsigned long pos = GetModuleBodyIndex();
	if(pos==-1)
		return 0;
	return &((*m_mess)[0])+pos;
}
bool VS_EnvelopeRelayMessage::Make(){

	size_t body_len = 0;
	unsigned char* body = SerializeContainer(body_len);
	if(!body) return false;
	Clear();
	m_mess->reserve(sizeof(RelayMessFixedPart) + strlen(m_name) + 1 + body_len);
	m_mess->resize(sizeof(RelayMessFixedPart));
	RelayMessFixedPart *header = reinterpret_cast<RelayMessFixedPart*>(&(*m_mess)[0]);
	header->mess_type = e_envelope;
	header->body_len = body_len + strlen(m_name) + 1;
	m_mess->insert(m_mess->end(),reinterpret_cast<const unsigned char*>(m_name),&(reinterpret_cast<const unsigned char*>(m_name))[strlen(m_name)+1]);
	m_mess->insert(m_mess->end(),body,body+body_len);
	m_mess_size = m_mess->size();
	free(body);
	m_isComplete = true;
	return true;
}

unsigned long VS_EnvelopeRelayMessage::GetContainerLen() const
{
	return GetBodyLen()-strlen(GetModuleName())-1;
}