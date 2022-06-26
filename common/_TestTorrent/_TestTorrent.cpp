// _TestTorrent.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../LibTorrent/VS_LibTorrent.h"
#include <iostream>
#include <Windows.h>
#include <string>

class TestTorrent : public VS_LibTorrent
{
	void onReadyToSend(const std::string &magnet, const std::string &to)
	{
		std::cout << magnet << std::endl;		
	}

	void onAskUser(const std::string &info_hash, const std::string &filename, const std::string &from)
	{
	//	printf("download %s from %s? d = download, n = don't download\n");
		//this->ConfirmDownload(info_hash, true, "D:\\tmp\\");
	}

	void onPeerConnected(const std::string &info_hash, const Endpoint &endpoint)
	{
		std::cout << endpoint.ip << std::endl;
		GetTorrent(info_hash).Connect(endpoint);
	}

	void onAskUser(const std::string &info_hash, const std::string &filename, const std::string &from, bool already_present)
	{
		std::cout << info_hash << " " << filename << std::endl;
		GetTorrent(info_hash).ConfirmDownload(true, "D:\\tmp\\");
	}
};

int _tmain(int argc, _TCHAR* argv[])
{
	TestTorrent test;
	std::cout << "port: " << test.ListenPort() << std::endl;
	char buffer[1025];
	while( 1 )
	{
		gets(buffer);
		if (buffer[0] == 'd')
		{
			printf("url: ");		
			gets(buffer);
			test.StartDownload( std::string(buffer), "1@test.server.name" );
			continue;
		} 
		
		if (buffer[0] == 'u')
		{
			printf("file: ");
			char buffer[1025];
			gets(buffer);
			test.SendFile(std::string( buffer ), "1@test.server.name" );
			continue;	
		}

		if (buffer[0] == 'q')
		{
			break;
		}

		if (buffer[0] == 's')
		{
			std::vector<VS_LibTorrent::TorrentInfo> info;
			test.GetStatistics( info );
			for (unsigned i = 0; i < info.size(); i++)
			{
				VS_LibTorrent::TorrentInfo & a = info[i];
				printf("-----------------------------------------------\n");
				printf("name = %s\n", a.name.c_str());
				printf("info_hash = %s\n", a.info_hash.c_str());
				printf("last_error = %s\n", a.last_error.c_str());

				printf("total_wanted = %I64d\n", a.total_wanted);
				printf("total_download = %I64d\n", a.total_download);
				printf("total_upload = %I64d\n", a.total_upload);
				printf("total_payload_download = %I64d\n", a.total_payload_download);
				printf("total_payload_upload = %I64d\n", a.total_payload_upload);

				printf("download_rate = %d\n", a.download_rate);
				printf("upload_rate = %d\n", a.upload_rate);

				printf("paused = %s\n", a.paused ? "true" : "false");
				printf("auto_managed = %s\n", a.auto_managed ? "true" : "false");
				printf("sequential_download = %s\n", a.sequential_download ? "true" : "false");
				printf("is_seeding = %s\n", a.is_seeding ? "true" : "false");
				printf("is_finished = %s\n", a.is_finished ? "true" : "false");
				printf("-----------------------------------------------\n");
			}
			continue;
		}

		if (buffer[0] == 'e') break;
	}

	test.Shutdown();
	return 0;
}

