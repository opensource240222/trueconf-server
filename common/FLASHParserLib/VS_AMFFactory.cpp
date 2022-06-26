#include "VS_AMFFactory.h"

#include "VS_AMFNumber.h"
#include "VS_AMFString.h"
#include "VS_AMFNull.h"
#include "VS_AMFBool.h"
#include "VS_AMFObjectsContainer.h"

#include "VS_AMFVideoFrame.h"
#include "VS_AMFAudioFrame.h"

VS_AMFFactory* VS_AMFFactory::m_factory = 0;

VS_AMFFactory::VS_AMFFactory()
{

}

VS_AMFFactory::~VS_AMFFactory()
{
	if (m_factory) { delete m_factory; m_factory = 0; }
}

VS_AMFFactory* VS_AMFFactory::Instance()
{
	if ( !m_factory )
		m_factory = new VS_AMFFactory;

	return m_factory;
}

VS_AMFBase* VS_AMFFactory::CreateDecoder(const void* in)
{
	char tmp = ((char*)in)[0];
	if (tmp == 0x00)
		return new VS_AMFNumber;
	else if (tmp == 0x01)
		return new VS_AMFBool;
	else if (tmp == 0x02)
		return new VS_AMFString;
	else if (tmp == 0x03)
		return new VS_AMFObjectsContainer;
	else if ((tmp == 0x05) || (tmp == 0x06))
		return new VS_AMFNull;
	else
		return 0;
}
VS_AMFBase* VS_AMFFactory::CreateDecoder(const unsigned int content_type)
{
	if ( content_type == VIDEO_DATA )
		return new VS_AMFVideoFrame;
	else if ( content_type == AUDIO_DATA )
		return new VS_AMFAudioFrame;
	else
		return 0;
}