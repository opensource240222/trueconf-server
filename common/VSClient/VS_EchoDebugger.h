#ifndef VS_ECHO_DEBUGGER_H
#define	VS_ECHO_DEBUGGER_H


#include <mutex>
#include <fstream>

class VS_EchoDebugger final
{
public:

	enum DataType
	{
		DT_ECHO,
		DT_NEAREND,
		DT_FAREND,
		DT_LOG,
		DT_NUM
	};

	VS_EchoDebugger(const VS_EchoDebugger&) = delete;
	VS_EchoDebugger(VS_EchoDebugger&&) = delete;
	~VS_EchoDebugger();

	bool Init(const std::string& prefix);
	void Clear();
	void WriteData(DataType dt, const void* buf, size_t size);
	std::ofstream& Log();

	static VS_EchoDebugger& GetInstance();

private:
	explicit VS_EchoDebugger();

	bool				m_Enabled;
	std::ofstream		m_DataFiles[DT_NUM];
	std::mutex			m_LogMutex;
};



#endif /* VS_ECHO_DEBUGGER_H */
