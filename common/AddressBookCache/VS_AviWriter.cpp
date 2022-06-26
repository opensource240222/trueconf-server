#include "VS_AviWriter.h"
#include "VS_OleProtoHelper.h"

VS_AviWriter::VS_AviWriter(VS_OleProtoHelper *parent)
	: m_parent(parent)
{
}

VS_AviWriter::~VS_AviWriter(void)
{
	Close();
}

bool VS_AviWriter::Create(const char *callId, const char *filename)
{
	if (m_parent) return m_parent->AviCreate(callId, filename);
		else return false;
}

bool VS_AviWriter::Resume()
{
	if (m_parent) return m_parent->AviResume();
		else return false;
}

bool VS_AviWriter::Pause()
{
	if (m_parent) return m_parent->AviPause();
		else return false;
}

bool VS_AviWriter::Close()
{
	if (m_parent) return m_parent->AviClose();
		else return false;
}
