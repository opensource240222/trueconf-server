#ifndef VS_AVIWRITER
#define VS_AVIWRITER

class VS_OleProtoHelper;
class VS_AviWriter
{
private:
	VS_OleProtoHelper *m_parent;

public:
	VS_AviWriter(VS_OleProtoHelper *parent);
	virtual ~VS_AviWriter();
	bool Create(const char *callId, const char *filename);
	bool Resume();
	bool Pause();
	bool Close();
};

#endif