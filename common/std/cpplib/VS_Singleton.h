#pragma once

#include <memory>
#include <mutex>

template <class T>
class VS_Singleton
{
public:
	static T& Instance()
	{
		// Once MSVC supports magic statics (MSVS 2015) we can simply do this:
		//     static T instance;
		//     return instance;

		std::call_once(of, []() { instance.reset(new T); });
		return *instance;
	}
private:
	VS_Singleton();
	static std::unique_ptr<T> instance;
	static std::once_flag of;
};

template <class T> std::unique_ptr<T> VS_Singleton<T>::instance;
template <class T> std::once_flag VS_Singleton<T>::of;

