#include "FilesystemUtils.h"
#include <cassert>

#ifdef _WIN32
#include <Windows.h>
#include <ctime>
#else
#include <unistd.h>
#include <stdlib.h>
#endif //_WIN32

#include <boost/filesystem/operations.hpp>


FILE* vs::GetTempFile(string_view nameFile, const char *mode, std::string &newFileName, boost::system::error_code &ec) noexcept
{
	assert(mode);
	
#ifdef _WIN32

	unsigned posix_time;
	const unsigned thread_id = ::GetCurrentThreadId();
	unsigned rand_number;

	newFileName.reserve(nameFile.length() + (std::numeric_limits<unsigned>::digits10 + 1) * 3);
	newFileName += nameFile;

	static_assert(
		std::is_same<unsigned, std::decay<decltype(posix_time)>::type>::value &&
		std::is_same<unsigned, std::decay<decltype(thread_id)>::type>::value &&
		std::is_same<unsigned, std::decay<decltype(rand_number)>::type>::value
		);

	char buffer[std::numeric_limits<unsigned>::digits10 + 1 + 1 /*0-terminator*/];

	std::size_t len = ::sprintf(buffer, "%u", thread_id);

	newFileName.append(buffer, len);

	len += nameFile.length();

	unsigned count = 5;
	do 
	{
		posix_time = static_cast<decltype(posix_time)>(::time(NULL));
		rand_number = ::rand();

		::sprintf(buffer, "%u", posix_time);
		newFileName.append(buffer);

		::sprintf(buffer, "%u", rand_number);
		newFileName.append(buffer);

		auto f = ::fopen(newFileName.c_str(), mode);
		if(f)
		{
			ec.clear();
			return f;
		}

		newFileName.erase(len);

	} while (--count > 0);

	ec = boost::system::error_code(errno, boost::system::system_category());
	return NULL;

#else
	newFileName.reserve(nameFile.length() + 6);
	newFileName += nameFile;
	newFileName.append("XXXXXX");
	
	auto res_mks = ::mkstemp((char *)newFileName.c_str());
	if(res_mks == -1)
	{
		ec = boost::system::error_code(errno, boost::system::system_category());
		return NULL;
	}

	auto res = ::fdopen(res_mks, mode);
	if(res == 0)
	{
		ec = boost::system::error_code(errno, boost::system::system_category());
		::close(res_mks);
	}
	else
	{
		ec.clear();
	}

	return res;

#endif //_WIN32
}

void vs::RenameFile(string_view oldFile, string_view newFile, boost::system::error_code &ec) noexcept
{
#ifdef _WIN32
	RemoveFile(newFile, ec);

	if (ec) //if error
		return;

	for (unsigned count = 5;
		((void)boost::filesystem::rename({ oldFile.cbegin(), oldFile.cend() }, { newFile.cbegin(), newFile.cend() }, ec), ec == boost::system::errc::permission_denied || ec == boost::system::errc::device_or_resource_busy) && count > 0;
		--count)
	{
		::Sleep(0); //yield 
	}
#else 
	boost::filesystem::rename({ oldFile.cbegin(), oldFile.cend() }, { newFile.cbegin(), newFile.cend() }, ec);
#endif //_WIN32
}

bool vs::RemoveFile(string_view file, boost::system::error_code &ec) noexcept
{
#ifdef _WIN32
	bool res = false;
	for (unsigned count = 5;
		((res=boost::filesystem::remove({ file.cbegin(), file.cend() }, ec)), ec == boost::system::errc::permission_denied || ec == boost::system::errc::device_or_resource_busy) && count > 0;
		--count)
	{
		::Sleep(0); //yield 
	}
	return res;
#else
	return boost::filesystem::remove({ file.cbegin(), file.cend() }, ec);
#endif //_WIN32
	return false;
}
