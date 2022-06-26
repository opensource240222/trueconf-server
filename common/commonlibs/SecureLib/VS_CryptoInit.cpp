#include "VS_CryptoInit.h"

#include "OpenSSLCompat/tc_ssl.h"
#include <openssl/err.h>

#if defined(_WIN32)
#	include <Windows.h>
#else
#	include <sys/syscall.h>
#	include <unistd.h>
#endif

#include <memory>
#include <mutex>

#if defined(_WIN32)
#	include <openssl/applink.c>
#endif


namespace vs {

static unsigned long OpenSSL_thread_id(void)
{
#if defined(_WIN32)
	return ::GetCurrentThreadId();
#else
	return ::syscall(SYS_gettid);
#endif
}

// Artem Boldarev (28.04.16):
// You *HAVE TO* protect OpenSSL internal data strcuctures in multithreaded environment.
// OpenSSL is *NOT* strictly thread safe. You need to provide and register some routines
// for multithreading support in your application. Enjoy random craches otherwise ;)
//
// References:
// [1] Network Security with OpenSSL, John Viega et al, 2009 - Section 4.1: Multithread Support

// static locking implementation
static std::unique_ptr<std::mutex[]> s_ssl_mutexes;
static void OpenSSL_lock(int mode, int n, const char*, int)
{
	if (!s_ssl_mutexes)
		return;

	if (mode & CRYPTO_LOCK)
		s_ssl_mutexes[n].lock();
	else
		s_ssl_mutexes[n].unlock();
}

// dynamic locking implementation (unused right now, but should be implemented for future versions of OpenSSL)
static void* OpenSSL_dyn_create_lock(const char*, int)
{
	return new std::mutex();
}

static void OpenSSL_dyn_destroy_lock(void* lock, const char*, int)
{
	if (lock == nullptr)
		return;
	auto mtx = static_cast<std::mutex*>(lock);
	delete mtx;
}

static void OpenSSL_dyn_lock(int mode, void *lock, const char*, int)
{
	if (lock == nullptr)
		return;
	auto mtx = static_cast<std::mutex*>(lock);
	if (mode & CRYPTO_LOCK)
		mtx->lock();
	else
		mtx->unlock();
}

void InitOpenSSL()
{
	// Leverage magic statics to do optional one-time initialization and destruction.
	static struct Once
	{
		Once()
		{
			OPENSSL_add_all_algorithms_noconf();
			SSL_library_init();
			SSL_load_error_strings();

			CRYPTO_set_id_callback(OpenSSL_thread_id);

			s_ssl_mutexes = std::make_unique<std::mutex[]>(CRYPTO_num_locks());
			CRYPTO_set_locking_callback(OpenSSL_lock);

			CRYPTO_set_dynlock_create_callback(
				reinterpret_cast<CRYPTO_dynlock_value* (*)(const char*, int)>(OpenSSL_dyn_create_lock));
			CRYPTO_set_dynlock_destroy_callback(
				reinterpret_cast<void(*) (CRYPTO_dynlock_value*, const char*, int)>(OpenSSL_dyn_destroy_lock));
			CRYPTO_set_dynlock_lock_callback(
				reinterpret_cast<void(*) (int, CRYPTO_dynlock_value*, const char*, int)>(OpenSSL_dyn_lock));
		}
		~Once()
		{
			CRYPTO_set_id_callback(nullptr);

			CRYPTO_set_locking_callback(nullptr);

			CRYPTO_set_dynlock_create_callback(nullptr);
			CRYPTO_set_dynlock_destroy_callback(nullptr);
			CRYPTO_set_dynlock_lock_callback(nullptr);

			ERR_free_strings();
			EVP_cleanup();
		}
	} init;
}

}
