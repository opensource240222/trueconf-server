#include "VS_RegOfflineClient.h"
#include "SecureLib/VS_UtilsLib.h"
#define REGISTRATION_TIMEOUT	60000
int main(int argc, char* argv[])
{

	if(argc <3)
		return 0;
	VS_RegOfflineClient client;
	HANDLE hRegCompleteEvent = CreateEvent(0,FALSE,FALSE,0);
	client.Init(argv[1]);
	if(client.MakeRegistration(hRegCompleteEvent,argv[2]))
	{
		switch(WaitForSingleObject(hRegCompleteEvent,REGISTRATION_TIMEOUT))
		{
		case WAIT_OBJECT_0:
			if(client.IsRegSuccess())
				printf("==Registration is success! Look file %s==\n",argv[2]);
			break;
		case WAIT_TIMEOUT:
			printf("==Registration is failed. Time expired==\n");
			break;
		default:
			break;
		};
	}
	if(!client.IsRegSuccess())
	{
		VS_SimpleStr str = client.ErrorDescr();
		printf("==Registration is failed. %s==\n", str.m_str);
	}
	printf("ServerID = '%s'; ServerName = '%s'", client.m_server_id.m_str, client.m_server_name.m_str);
	client.Release();
	return 0;
}