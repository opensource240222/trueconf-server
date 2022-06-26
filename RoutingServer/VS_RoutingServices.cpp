#include "VS_RoutingServices.h"

#include "Services/VS_RoutingService.h"
#include "Services/VS_ManagerService.h"

#include "../common/transport/Router/VS_PoolThreadsService.h"
#include "../ServerServices/VS_VerificationService.h"
#include "../ServerServices/VS_PingService.h"

#include "../common/std/cpplib/VS_RegistryKey.h"
#include "../common/std/cpplib/VS_RegistryConst.h"
#include "std-generic/compat/memory.h"

struct VS_RoutingServices_Implementation
{
	enum StartUpSequence {
		NONE_SS = 0,
		POOLTHREADS_SRV_SS,
		LOG_SRV_SS,
		PRESENCE_SRV_SS,
		MANAGER_SRV_SS,
		AUTH_SRV_SS,
		CONFIGURATION_SRV_SS,
		CONFERENCE_SRV_SS,
		CHAT_SRV_SS,
		VERIFY_SRV_SS,
		PING_SRV_SS
	};
	long m_srv_start_mode;

	static const int RESTART_TIME	=5*60;//5 minutes


  VS_RoutingServices_Implementation( void )
    : tr(0), ss(NONE_SS), m_srv_start_mode(0)
	{}

	~VS_RoutingServices_Implementation( void )
	{Destroy();}


	VS_TransportRouter *tr;
	StartUpSequence	ss;
	VS_PoolThreadsService			pts;		//sync
	VS_RoutingService				rs;
	VS_PingService					pngs;
	VS_ManagerService				ms;

	std::unique_ptr<VS_VerificationService>	verifys;


	inline bool Init(vs::RegistrationParams&& rp, VS_TransportRouter *tr, VS_RoutersWatchdog *watchdog, const char* ver)
	{
		if (ss != NONE_SS)		return false;
		VS_RoutingServices_Implementation::tr = tr;
		puts( "SRV: Services are starting..." );
		const char   sSd[] = "\t started successfully.", sFd[] = "\t failed.";
#define STARTMESS(x) printf("\t %-20s: ", (x))

		m_srv_start_mode = rp.mode;

		VS_RegistryKey    cfg_root(false, CONFIGURATION_KEY, false, true);
		if (!cfg_root.IsValid()) {
			puts("Registry Configuration key not found");
			return false;
		}

		verifys = vs::make_unique<VS_VerificationService>(std::move(rp));

		if(m_srv_start_mode == 1 || m_srv_start_mode == 2)
		{
// --------------------------------------------------------------------------------
			STARTMESS(VERIFY_SRV);
			verifys->SetComponents(watchdog, MANAGER_SRV);
			if (!tr->AddService(VERIFY_SRV, verifys.get()) || !verifys->SetThread()){
				puts( sFd ); return false;
			}
			ss = VERIFY_SRV_SS; puts(sSd);

// --------------------------------------------------------------------------------
		}
		else
		{
			// --------------------------------------------------------------------------------
			STARTMESS(VERIFY_SRV);
			verifys->SetComponents(watchdog, MANAGER_SRV);
			if (!tr->AddService(VERIFY_SRV, verifys.get()) || !verifys->SetThread()){
				puts( sFd ); return false;
			}
			ss = VERIFY_SRV_SS; puts(sSd);

			// --------------------------------------------------------------------------------

			//cs.SetStreamRouter(sr);
			//wcs.SetService(&cs);

			STARTMESS(VS_POOL_THREADS_SERVICE_NAME);
			if (!tr->AddService(VS_POOL_THREADS_SERVICE_NAME, &pts) || !pts.SetThread()) {
				puts( sFd );	return false;
			}
			ss = POOLTHREADS_SRV_SS;	puts( sSd );

			// --------------------------------------------------------------------------------
			STARTMESS(PRESENCE_SRV);
			if (!tr->AddService(PRESENCE_SRV, &rs) || !rs.SetThread()) {
					puts(sFd); return false;
			}
			ss = PRESENCE_SRV_SS; puts(sSd);

			STARTMESS(MANAGER_SRV);
			ms.SetComponents(watchdog, ver);
			if (!tr->AddService(MANAGER_SRV, &ms) || !ms.SetThread() || !watchdog->AddTestable(&ms, 11)) {
				puts(sFd);
				watchdog->Restart(RESTART_TIME);
				return false;
			}
			ss = MANAGER_SRV_SS; puts(sSd);

			STARTMESS(PING_SRV);
			if (!tr->AddCallService(PING_SRV, &pngs,1))
			{	puts( sFd );	return false;	}
			ss = PING_SRV_SS;	puts( sSd );
		}
		return true;
	}

	inline void Destroy( void )
	{
		const char   sSl[] = " successful.";
#define ENDSEQ(x) printf("\t Destroying %-16s: ", (x)); tr->RemoveService((x)); puts(sSl);
		puts( "SRV: Services are exiting..." );
		if(1==m_srv_start_mode)
		{
			if (verifys)
				verifys->ResetThread();
			ENDSEQ(VERIFY_SRV);
		}
		else
		{
			switch (ss)
			{
			case PING_SRV_SS:
				ENDSEQ(PING_SRV)

			case VERIFY_SRV_SS:
				if (verifys)
					verifys->ResetThread();
				ENDSEQ(VERIFY_SRV);

			case MANAGER_SRV_SS:
				ms.ResetThread();
				ENDSEQ(MANAGER_SRV)
			case PRESENCE_SRV_SS:
				//upss.ResetThread();
				ENDSEQ(PRESENCE_SRV)
			case POOLTHREADS_SRV_SS:
				//pts.ResetThread();
				ENDSEQ(VS_POOL_THREADS_SERVICE_NAME)
			default:
				if (pts.IsValid())
				{
					printf( "\tDestroying Phread was" );
					pts.ResetThread();
					puts( sSl );
				}
				/*if (g_storage) {
				printf( "\tDestroying Storage was" );
				delete g_storage; g_storage = 0;
				puts( sSl );
				}*/
			}
		}
		ss = NONE_SS; tr = 0;

	}

	bool Test( void )
	{
		return true;//g_storage!=0 && g_storage->Test();
	};

};

//////////////////////////////////////////////////////////////////////////////////////////

VS_RoutingServices::VS_RoutingServices( void )
{	imp = new VS_RoutingServices_Implementation;		}

VS_RoutingServices::~VS_RoutingServices( void ) {		if (imp)	delete imp;		}

bool VS_RoutingServices::IsValid( void ) const
{	return imp;	}

//stream::ConferencesConditions* VS_RoutingServices::GetConferencesConditions()
//{	return !imp ? 0 : /*&imp->cs;*/	nullptr;	}

bool VS_RoutingServices::Init(vs::RegistrationParams&& rp, VS_TransportRouter *tr, /*VS_StreamsRouter *sr,*/ VS_RoutersWatchdog *watchdog, const char* ver)
{	return !imp ? false : imp->Init(std::move(rp), tr, /*sr,*/ watchdog, ver);	}

void VS_RoutingServices::Destroy( void ) {	if (imp)	imp->Destroy();		}

bool VS_RoutingServices::Test( void)
{	return imp?imp->Test():false; };

//////////////////////////////////////////////////////////////////////////////////////////
