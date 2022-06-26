/**
**************************************************************************
* \file VS_Container.h
* (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
* \brief Container class definition
*
* \b Project Standart Libraries
* \author PetrovichevD
* \date 22.11.02
*
* $Revision: 6 $
*
* $History: VS_Container.h $
 *
 * *****************  Version 6  *****************
 * User: Ktrushnikov  Date: 14.03.11   Time: 12:10
 * Updated in $/VSNA/std/cpplib
 * - added VS_Container::IsEmpty()
 * - Gateway: WaitProps() after conneted to server, but before try to
 * login (bacuse we need Prop: authentication_method)
 *
 * *****************  Version 5  *****************
 * User: Ktrushnikov  Date: 2.02.11    Time: 0:51
 * Updated in $/VSNA/std/cpplib
 * vcs 3.2:
 * - AddToAB in a PoolThreadTask
 * - AddToAB user with full reallogin in roaming
 * - VS_Container::AttachToCnt()
 *
 * *****************  Version 4  *****************
 * User: Ktrushnikov  Date: 3.09.09    Time: 11:38
 * Updated in $/VSNA/std/cpplib
 * Save abooks at AS:
 *   - VS_Container: GetLongValueRef() added
 *   - TRANSPORT_SRCUSER_PARAM: pass user from Transport to Container
 *
 * *****************  Version 3  *****************
 * User: Dront78      Date: 14.06.09   Time: 10:49
 * Updated in $/VSNA/std/cpplib
 * - VZOchat7 merge. see VZOchat7.h for details
 *
 * *****************  Version 2  *****************
 * User: Mushakov     Date: 29.04.08   Time: 18:45
 * Updated in $/VSNA/std/cpplib
 * unprotected container copy constructor
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/std/cpplib
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/std/cpplib
 *
 * *****************  Version 16  *****************
 * User: Smirnov      Date: 16.08.06   Time: 15:43
 * Updated in $/VS/std/cpplib
 * - AddVAlue speeding up
 *
 * *****************  Version 15  *****************
 * User: Stass        Date: 28.07.06   Time: 16:20
 * Updated in $/VS/std/cpplib
 * added __int64 pass
 *
 * *****************  Version 14  *****************
 * User: Smirnov      Date: 2.06.06    Time: 15:34
 * Updated in $/VS/std/cpplib
 * - added Nhp video and audio buffers
 *
 * *****************  Version 13  *****************
 * User: Stass        Date: 28.12.05   Time: 15:25
 * Updated in $/VS/std/cpplib
 * comment
 *
 * *****************  Version 12  *****************
 * User: Stass        Date: 21.12.05   Time: 19:06
 * Updated in $/VS/std/cpplib
 * added get type for current cnt value
 *
 * *****************  Version 11  *****************
 * User: Stass        Date: 26.07.05   Time: 18:56
 * Updated in $/VS/std/cpplib
 * fixed copy constructor
 *
 * *****************  Version 10  *****************
 * User: Smirnov      Date: 29.12.04   Time: 15:11
 * Updated in $/VS/std/cpplib
 * files headers
****************************************************************************/
#ifndef VS_STD_CONTAINER_H
#define VS_STD_CONTAINER_H


/****************************************************************************
* Includes
****************************************************************************/
#include "TimeUtils.h"
#include "string_view.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <chrono>

/**
**************************************************************************
* \brief Container values types
****************************************************************************/
enum ContainerVT : uint8_t
{
	VS_CNT_BOOL_VT = 0,	///< bool
	VS_CNT_INTEGER_VT,	///< int32_t
	VS_CNT_DOUBLE_VT,	///< double
	VS_CNT_STRING_VT,	///< char[], zero terminated
	VS_CNT_BINARY_VT,	///< unsigned char[]
	VS_CNT_INT64_VT,	///< int64_t
};

#pragma pack(push)
#pragma pack(1)
//@{ \name Container Internal Structures
typedef struct PascalStr_tag {
	uint8_t			len;
	char			szStr[1];
} PascalStr;

typedef struct LPascalStr_tag {
	uint32_t		len;
	char			szStr[1];
} LPascalStr;

typedef struct RawValue_tag {
	ContainerVT type;
	union {
		unsigned char	bVal;
		int32_t			i32Val;
		double			dVal;
		LPascalStr		sVal;
		int64_t			i64Val;
	} data;
} RawValue;
//@}
#pragma pack(pop)

/**
**************************************************************************
* \brief Container for pair of data and correspondent names
****************************************************************************/
class VS_Container
{
public:
	//@{ \name Constructors
	VS_Container() noexcept;
	VS_Container(const VS_Container &cnt);
	VS_Container(VS_Container&& src) noexcept;
	//@}

	VS_Container &operator= (const VS_Container &cnt);
	VS_Container& operator=(VS_Container&& src) noexcept;

	/// destructor
	~VS_Container();
	/// check empty case
	bool IsValid() const;
	/// Copy current container to cnt
	bool CopyTo(VS_Container& cnt) const;
	/// Add my data to other container
	void AttachToCnt(VS_Container& cnt) const;
	/// Clear internal buffer
	void Clear();
	//@{ \name Work wtih values
	bool AddValue(string_view name, const bool val)
	{
		return AddValue(name, VS_CNT_BOOL_VT, &val, 0);
	}
	bool AddValue(string_view name, const int32_t val)
	{
		return AddValue(name, VS_CNT_INTEGER_VT, &val, 0);
	}
	bool AddValue(string_view name, const uint32_t val)
	{
		return AddValue(name, VS_CNT_INTEGER_VT, &val, 0);
	}
	bool AddValue(string_view name, const char* val)
	{
		if (!val)
			return false;
		return AddValue(name, VS_CNT_STRING_VT, val, ::strlen(val));
	}
	bool AddValue(string_view name, string_view val)
	{
		return AddValue(name, VS_CNT_STRING_VT, val.data(), val.size());
	}
	bool AddValue(string_view name, const wchar_t* val);
	bool AddValue(string_view name, const double val)
	{
		return AddValue(name, VS_CNT_DOUBLE_VT, &val, 0);
	}
	bool AddValue(string_view name, const void* val, const size_t size)
	{
		if (!val || size == 0)
			return false;
		return AddValue(name, VS_CNT_BINARY_VT, val, size);
	}
	bool AddValue(string_view name, const std::chrono::system_clock::time_point val) {
		int64_t windowsTicks = 0;
		if (val != std::chrono::system_clock::time_point())
		{
			windowsTicks = tu::UnixSecondsToWindowsTicks(val);
		}
		return  AddValue(name, VS_CNT_BINARY_VT, &windowsTicks, sizeof windowsTicks);
	}
	bool AddValue(string_view name, std::tm& time);
	bool AddValue(string_view name, const VS_Container &val);

	template <class T>
	bool AddValueI32(string_view name, const T& val)
	{
		auto val32 = static_cast<int32_t>(val);
		return AddValue(name, VS_CNT_INTEGER_VT, &val32, 0);
	}
	template <class T>
	bool AddValueI64(string_view name, const T& val)
	{
		auto val64 = static_cast<int64_t>(val);
		return AddValue(name, VS_CNT_INT64_VT, &val64, 0);
	}

	// If you got "use of a deleted function" error here it's because adding
	// long to containers is banned to enforce consistent behaviour on all
	// systems:
	// On Linux64 long is a 64 bit number and would be added as VS_CNT_INT64_T,
	// and on others it is a 32 bit number and would be added as
	// VS_CNT_INTEGER_VT.
	// Overload for long long is also deleted to prevent writing code that
	// would compile on one platform but not on others (by using int64_t or
	// size_t).
	// If you need to add 64 bit value to the container use AddValueI64().
	// NOTE: Because int64_t is a typedef to long on Linux, it is impossible to
	// have AddValue overload for int64_t and delete overload for long.
	bool AddValue(string_view name, long val) = delete;
	bool AddValue(string_view name, unsigned long val) = delete;
	bool AddValue(string_view name, long long val) = delete;
	bool AddValue(string_view name, unsigned long long val) = delete;

	bool DeleteValue(string_view name);

	bool GetValue(string_view name, std::chrono::system_clock::time_point& tp) const
	{
		size_t sz = 0;
		const auto valid_until_ft = static_cast<const int64_t *>(GetBinValueRef(name, sz));
		if (valid_until_ft == nullptr || sz != sizeof(int64_t))
		{
			return false;
		}
		if ((*valid_until_ft) == 0)
		{
			tp = std::chrono::system_clock::time_point();
		}
		else
		{
			tp = tu::WindowsTickToUnixSeconds(*valid_until_ft);
		}
		return true;
	}

	bool GetValue(string_view name, bool& val) const
	{
		size_t size = 0;
		return GetValue(name, VS_CNT_BOOL_VT, &val, size);
	}
	bool GetValue(string_view name, int32_t& val) const
	{
		size_t size = 0;
		return GetValue(name, VS_CNT_INTEGER_VT, &val, size);
	}
	bool GetValue(string_view name, char* val, size_t& size) const
	{
		return GetValue(name, VS_CNT_STRING_VT, val, size);
	}
	bool GetValue(string_view name, double& val) const
	{
		size_t size = 0;
		return GetValue(name, VS_CNT_DOUBLE_VT, &val, size);
	}
	bool GetValue(string_view name, void* val, size_t& size) const
	{
		return GetValue(name, VS_CNT_BINARY_VT, val, size);
	}
	bool GetValue(string_view name, int64_t& val) const
	{
		size_t size = 0;
		return GetValue(name, VS_CNT_INT64_VT, &val, size);
	}
	bool GetValue(string_view name, std::tm& time) const;
	bool GetValue(string_view name, VS_Container& val) const;
	bool GetValue(string_view name, std::string& val) const;

	template <class T>
	bool GetValueI32(string_view name, T& val) const
	{
		int32_t val32;
		if (!GetValue(name, val32))
			return false;
		val = static_cast<T>(val32);
		return true;
	}
	template <class T>
	bool GetValueI64(string_view name, T& val) const
	{
		int64_t val64;
		if (!GetValue(name, val64))
			return false;
		val = static_cast<T>(val64);
		return true;
	}
	//@}
	//@{\name Get reference to values
	const char* GetStrValueRef(string_view name) const
	{
		size_t size;
		return static_cast<const char*>(GetValueRef(name, VS_CNT_STRING_VT, size));
	}
	const void* GetBinValueRef(string_view name, size_t& size) const
	{
		return GetValueRef(name, VS_CNT_BINARY_VT, size);
	}
	int32_t* GetLongValueRef(string_view name)
	{
		size_t size;
		return static_cast<int32_t*>(const_cast<void*>(GetValueRef(name, VS_CNT_INTEGER_VT, size)));
	}
	int64_t* GetInt64ValueRef(string_view name)
	{
		size_t size;
		return static_cast<int64_t*>(const_cast<void*>(GetValueRef(name, VS_CNT_INT64_VT, size)));
	}

	// Get a view to the stored string.
	// Returns empty view both when the string is empty and when it is not found in the container.
	cstring_view GetStrValueView(string_view name) const
	{
		size_t size = 1; // This is here to ensure that result will have correct (zero) length when value is not found.
		const auto data = GetValueRef(name, VS_CNT_STRING_VT, size);
		return { null_terminated, static_cast<const char*>(data), size - 1 };
	}

	size_t Count() const
	{
		return m_valueCnt;
	}
	//@}
	//@{\name Serialization/deserialization
	bool Serialize(void* buff, size_t &size) const;
	bool SerializeAlloc(void* &buff, size_t &size) const;
	bool Deserialize(const void* buff, const size_t size);
	uint32_t GetAllocSize() const;
	//@}
	//@{\name Sequential access
	void Reset() const;
	bool Next() const;
	const char* GetName() const;
	ContainerVT GetType() const;
	bool GetValue(bool &val) const;
	bool GetValue(int32_t &val) const;
	bool GetValue(double &val) const;
	bool GetValue(int64_t &val) const;
	bool GetValue(VS_Container &val) const;
	bool GetValue(std::chrono::system_clock::time_point &tp) const;

	template <class T>
	bool GetValueI32(T& val) const
	{
		int32_t val32;
		if (!GetValue(val32))
			return false;
		val = static_cast<T>(val32);
		return true;
	}
	template <class T>
	bool GetValueI64(T& val) const
	{
		int64_t val64;
		if (!GetValue(val64))
			return false;
		val = val64;
		return true;
	}
	bool GetValue(std::string &val) const;
	std::string GetStrValue() const;

	//Add current name value to cnt
	bool AttachCurrentTo(VS_Container &cnt) const;

	// Get reference to values
	const char* GetStrValueRef() const;
	const void* GetBinValueRef(size_t &size) const;

	// Get a view to the stored string.
	// Returns empty view both when the string is empty and when the current value is not a string.
	cstring_view GetStrValueView() const;

	bool IsEmpty() const;

	friend uint32_t CalcHashOnMemory(VS_Container& cnt);
	//@}
protected:
	//@{\name Internal helpers
	bool AddValue(string_view name, const ContainerVT type, const void* val, const size_t size);
	bool GetValue(string_view name, const ContainerVT type, void* val, size_t& size) const;
	const void* GetValueRef(string_view name, const ContainerVT type, size_t& size) const;
	void* GetMemory(size_t size);
	//@}
	mutable PascalStr* m_currKey;
	mutable RawValue* m_currValue;
	void* m_value;
	size_t m_valueSize;
	size_t m_MemorySize;
	unsigned long m_valueCnt;

	friend struct VS_ContainerData;
};

/**
 **************************************************************************
 * \brief Container for binary data
 ****************************************************************************/
class VS_BinBuff {
	void* m_buff;
	size_t m_size;
public:
	VS_BinBuff() noexcept;
	VS_BinBuff(const void* buff, size_t size);
	VS_BinBuff(const VS_BinBuff &src);
	VS_BinBuff(VS_BinBuff&& src) noexcept;
	~VS_BinBuff();
	void Empty();
	void Set(const void* buff, size_t size);
	void Set(const VS_Container & cnt);
	void SetSize(size_t size);
	bool IsValid() const { return m_buff != nullptr && m_size != 0; }
	void* Buffer() { return m_buff; }
	const void* Buffer() const { return m_buff; }
	size_t Size() const { return m_size; }
	VS_BinBuff &operator= (const VS_BinBuff &src);
	VS_BinBuff& operator=(VS_BinBuff&& src) noexcept;

	friend bool operator==(const VS_BinBuff& l, const VS_BinBuff& r);
	friend bool operator!=(const VS_BinBuff& l, const VS_BinBuff& r) { return !(l == r); }
};

#endif // VS_STD_CONTAINER_H
