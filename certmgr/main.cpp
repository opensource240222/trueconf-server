#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>

#include "parg.h"

#include "../common/tools/Server/VS_ApplicationInfo.h"
#include "../common/std/cpplib/VS_RegistryKey.h"
#include "../common/std/cpplib/VS_RegistryConst.h"

/*
 * It's program purpose is to manage TrueConf Server TLS certificate and key.
 * author: Artem Boldarev
 * date: 2015.03.31
 */

// imports

// constants
static const size_t     VERSION = 2; /*!!! program version !!!*/
static const char *KEY_VALNAME  = "TLSPrivateKey";
static const char *CERT_VALNAME = "TLSCert";
static const size_t BUFSZ = 64 * 1024; // I/O buffer size: 64Kb

typedef struct _Config {
	bool dump;
	char *key_file;
	char *cert_file;

	_Config()
		: dump(true), key_file(NULL), cert_file(NULL)
	{}
} Config;

// application configuration
static Config cfg;

// executable name
static char *argv0;

// show usage end exit application
static void usage(void)
{
	cerr << "TrueConf TLS certificate manager version " << VERSION << ".\n\n"
		"Usage: \n"
		<< argv0 << " [-dsR -k <key_file> -c <cert_file>]\n"
		"Options:\n" <<
		" -d - dump mode;\n"
		" -s - store mode;\n"
		" -k - key file;\n"
		" -c - certificate file;\n"
		" -R - remove TLS certificate and key from registry and exit program;\n"
		" -h - show this help and exit." << endl;

	exit(EXIT_FAILURE);
}

void dump_data(const char *value_name, char *file_name)
{
	unsigned char buf[BUFSZ];
	//unsigned long type;
	int size;

	if (!file_name)
		return;

	VS_RegistryKey key(false, CONFIGURATION_KEY, true, false);

	size = key.GetValue(buf, BUFSZ, VS_REG_BINARY_VT, value_name);
	if (size == 0)
	{
		cerr << "Registry value " << "\"" << value_name << "\"" << " not found." << endl;
		return;
	}
	else if (size < 0)
	{
		cerr << "Size of read buffer is not sufficient enough." << endl;
		return;
	}

	// dump data
	ofstream fout(file_name, ios_base::out|ios_base::binary);
	fout.write((char *)&buf[0], size);
	fout.close();
}

void dump_key()
{
	dump_data(KEY_VALNAME, cfg.key_file);
}


void dump_cert()
{
	dump_data(CERT_VALNAME, cfg.cert_file);
}

void set_data(const char *value_name, char *file_name)
{
	unsigned char buf[BUFSZ];
	int size;
	if (!file_name)
		return;

	VS_RegistryKey key(false, CONFIGURATION_KEY, false, true);

	// read data from file
	ifstream fin(file_name, ios_base::out|ios_base::binary);
	fin.read((char *)&buf[0], BUFSZ);
	size = fin.gcount();
	fin.close();

	if (!key.SetValue((char* )&buf[0], size, VS_REG_BINARY_VT, value_name))
	{
		cerr << "Can\'t store data into registry value \"" << value_name << "\"" << endl;
		return;
	}
}

void set_key()
{
	set_data(KEY_VALNAME, cfg.key_file);
}

void set_cert()
{
	set_data(CERT_VALNAME, cfg.cert_file);
}

// remove certificate and key from regestry
void remove()
{
	VS_RegistryKey key(false, CONFIGURATION_KEY, false, false);

	if (!key.RemoveValue(KEY_VALNAME))
	{
		cerr << "Can\'t remove TLS key from registry." << endl;
	}

	if (!key.RemoveValue(CERT_VALNAME))
	{
		cerr << "Can\'t remove TLS certificate from registry." << endl;
	}
}

int main(int argc, char **argv)
{
	// Manage keys only for TCS
	VS_RegistryKey::SetRoot(VS_TRUECONF_WS_ROOT_KEY_NAME);

	ARGBEGIN {
		case 'd':
			cfg.dump = true;
			break;
		case 's':
			cfg.dump = false;
			break;
		case 'k':
			cfg.key_file = EARGF(usage() );
			break;
		case 'c':
			cfg.cert_file = EARGF(usage() );
			break;
		case 'R':
			remove();
			return EXIT_SUCCESS;
			break;
		case 'h':
			usage();
			break;
		default:
			cerr << "unexpected option: -" << ARGC() << endl;
			usage();
			break;
	} ARGEND;

	if (cfg.cert_file == NULL && cfg.key_file == NULL)
	{
		cerr << "Nothing to do: no files specified." << endl;
		usage();
		return EXIT_SUCCESS;
	}

	try {
		if (cfg.dump)
		{
			dump_key();
			dump_cert();
		}
		else
		{
			set_key();
			set_cert();
		}
	}
	catch (exception &e)
	{
		cerr << "Error occured: " << e.what() << endl;
	}

	return EXIT_SUCCESS;
}
