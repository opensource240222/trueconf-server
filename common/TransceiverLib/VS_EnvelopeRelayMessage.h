#pragma once
#include "std-generic/cpplib/VS_Container.h"
#include "VS_MainRelayMessage.h"
#include <boost/shared_ptr.hpp>

class VS_EnvelopeRelayMessage : public VS_MainRelayMessage
{
private:
	VS_Container m_cnt;
public:
	char* m_name;
	explicit VS_EnvelopeRelayMessage(const char *module_name);
	virtual ~VS_EnvelopeRelayMessage();

	bool IsValid() const override;
	bool Make();
	bool SetMessage(const boost::shared_ptr<std::vector<unsigned char>>& mess) override;

	void ClearContainer() { m_cnt.Clear(); }
	bool SetParam(string_view name, const bool val)                                  { return m_cnt.AddValue(name, val); }
	bool SetParam(string_view name, const int32_t val)                               { return m_cnt.AddValue(name, val); }
	bool SetParam(string_view name, const uint32_t val)                              { return m_cnt.AddValue(name, val); }
	bool SetParam(string_view name, const double val)                                { return m_cnt.AddValue(name, val); }
	bool SetParam(string_view name, const char* val)                                 { return m_cnt.AddValue(name, val); }
	bool SetParam(string_view name, string_view val)                                 { return m_cnt.AddValue(name, val); }
	bool SetParam(string_view name, const void* val, const size_t size)              { return m_cnt.AddValue(name, val, size); }
	bool SetParam(string_view name, const VS_Container& val)                         { return m_cnt.AddValue(name, val); }
	bool SetParam(string_view name, const std::chrono::system_clock::time_point val) { return m_cnt.AddValue(name, val); }

	template<class T>
	bool SetParamI64(string_view name, const T& val)                                 { return m_cnt.AddValueI64(name, val); }

	// These are deliberately disabled, more info in comments for corresponding functions in VS_Container.
	bool SetParam(string_view name, long val) = delete;
	bool SetParam(string_view name, unsigned long val) = delete;
	bool SetParam(string_view name, long long val) = delete;
	bool SetParam(string_view name, unsigned long long val) = delete;

	bool GetParam(string_view name, bool& val) const                                  { return m_cnt.GetValue(name, val); }
	bool GetParam(string_view name, int32_t& val) const                               { return m_cnt.GetValue(name, val); }
	bool GetParam(string_view name, double& val) const                                { return m_cnt.GetValue(name, val); }
	bool GetParam(string_view name, char* val, size_t& size) const                    { return m_cnt.GetValue(name, val, size); }
	bool GetParam(string_view name, void* val, size_t& size) const                    { return m_cnt.GetValue(name, val, size); }
	bool GetParam(string_view name, int64_t& val) const                               { return m_cnt.GetValue(name, val); }
	bool GetParam(string_view name, VS_Container& val) const                          { return m_cnt.GetValue(name, val); }
	bool GetParam(string_view name, std::chrono::system_clock::time_point& val) const { return m_cnt.GetValue(name, val); }

	template<class T>
	bool GetParamI64(string_view name, T& val) const               { return m_cnt.GetValueI64(name, val); }

	const char* GetStrValRef(string_view name) const               { return m_cnt.GetStrValueRef(name); }
	const void* GetBinValRef(string_view name, size_t& size) const { return m_cnt.GetBinValueRef(name, size); }
	string_view GetStrValueView(string_view name) const            { return m_cnt.GetStrValueView(name); }

	unsigned char *SerializeContainer(size_t &size) const;
	bool DeserializeContainer(const unsigned char *buf, const size_t size);

	unsigned char *GetModuleBody() const;
	unsigned long GetBodyLen() const;

	unsigned long GetContainerLen() const;
protected:
	unsigned long GetModuleBodyIndex() const;
};
