#include "VS_AMFObjectsContainer.h"
#include "VS_AMFString.h"
#include "VS_AMFNumber.h"
#include "VS_AMFFactory.h"

VS_AMFObjectsContainer::VS_AMFObjectsContainer()
{

}

VS_AMFObjectsContainer::~VS_AMFObjectsContainer()
{
	for(unsigned int i=0; i < m_objs.size(); i++)
		if (m_objs[i].second) { delete m_objs[i].second; m_objs[i].second = 0; }

	m_objs.clear();
}

void VS_AMFObjectsContainer::Add(const char* key, VS_AMFBase* value)
{
	m_objs.push_back( TPair(key, value) );
}

void VS_AMFObjectsContainer::Add(const char* key, const char* value)
{
	VS_AMFString* amf_str = new VS_AMFString;
	amf_str->SetStr(value);

	this->Add(key, amf_str);
}

void VS_AMFObjectsContainer::Add(const char* key, const double value)
{
	VS_AMFNumber* amf_num = new VS_AMFNumber;
	amf_num->Init(value);

	this->Add(key, amf_num);
}

unsigned int VS_AMFObjectsContainer::Encode(void* out)
{
	if ( !out )
		return 0;

	void* ptr = out;
	unsigned int n = 0;
	unsigned int n_total = 0;

	// start
	((char*)ptr)[0] = 0x03;
	ptr = (void*) (((char*)ptr)+1);

	TContainer::iterator it;
	for(it = m_objs.begin(); it != m_objs.end(); it++)
	{
		VS_AMFString str;
		str.SetStr(it->first.c_str());
		str.WithFirst(false);

		n = str.Encode(ptr);
		if (n)
		{
			ptr = (void*) (((char*)ptr)+n);
			n_total += n;
		}

		n = it->second->Encode(ptr);
		if (n)
		{
			ptr = (void*) (((char*)ptr)+n);
			n_total += n;
		}
	}		

	// end
	((char*)ptr)[0] = 0x00;
	((char*)ptr)[1] = 0x00;
	((char*)ptr)[2] = 0x09;

	return 1 + n_total + 3;
}

unsigned int VS_AMFObjectsContainer::Decode(const void* in, const unsigned long in_sz)
{
	if ( !in )
		return 0;

	unsigned int n_total = 0;
	unsigned int n = 0;
	char* ptr = (char*) in;

	if (ptr[0] != 0x03)		// start byte is 0x03
		return 0;

	ptr++;
	n_total++;

	VS_AMFFactory* factory = VS_AMFFactory::Instance();
	VS_AMFString key;	key.WithFirst(false);
	VS_AMFBase* value = 0;
	while( !IsEnd(ptr) )
	{
		n = key.Decode((void*) ptr);
		if ( !n )
			return 0;

		ptr += n;
		n_total += n;

		VS_AMFBase* value = factory->CreateDecoder(ptr);
		if ( !value )
			return 0;

		int n = value->Decode(ptr);
		if ( !n )
			return 0;

		this->Add(key.GetStr(), value);

		ptr += n;
		n_total += n;
	}

	n_total += 3;			// three stop bytes
	return n_total;
}

bool VS_AMFObjectsContainer::IsEnd(const char* in)
{
	if ( !in )
		return false;

	if ( (in[0] != 0x00) ||
		(in[1] != 0x00) ||
		(in[2] != 0x09) )
		return false;

	return true;
}