#ifndef VS_ROUTING_SERVICE_H
#define VS_ROUTING_SERVICE_H

#include "transport/Router/VS_TransportRouterServiceHelper.h"
#include "std/cpplib/VS_Map.h"
#include "ServerServices/VS_SubscriptionHub.h"
#include "std/cpplib/VS_Lock.h"


class VS_RoutingService :
	public VS_TransportRouterServiceReplyHelper,
	public VS_SubscriptionHub,
	public VS_Lock
{
	class RegisterItem
	{
	public:
		VS_SimpleStr m_server;
		VS_SimpleStr m_temp_id;
		VS_SimpleStr m_seq;
		long		 m_type;
		std::string	 m_displayName;

		RegisterItem(const char* server,const char* temp_id, const char* seq, long type, const std::string& displayName)
			: m_server(server), m_temp_id(temp_id), m_seq(seq), m_type(type), m_displayName(displayName)
		{};
	};
	VS_SimpleStr	m_SM;
	unsigned long	m_SmLastCheckTime;

	typedef VST_StrIMap<RegisterItem> RegisterQueue;

	typedef VST_Map<VS_FullID, std::chrono::system_clock::time_point> IDTimeMap;
	VS_Map			m_connserv;

	VS_SyncPool		m_sub_sync;

	RegisterQueue	m_reg_queue;
	VS_Lock			m_reg_lock;


	IDTimeMap       m_reset_time;
    IDTimeMap		m_sub_reset_time;

	void SendRegResult(VS_UserLoggedin_Result result, const RegisterItem& ri);
	virtual bool IsRoamingAllowed(const char *for_server_name) override;
	virtual bool IsUplink() const override { return true; }

public:
    VS_RoutingService(void);
    ~VS_RoutingService(void);

	bool Init(const char* our_endpoint, const char* our_service, const bool permitAll) override;
	bool Timer(unsigned long tickcount) override;
	bool Processing(std::unique_ptr<VS_RouterMessage>&& recvMess) override;

    // VS_EndpointConditions implementation
	bool OnPointDisconnected_Event(const VS_PointParams* prm) override;
	bool OnPointConnected_Event(const VS_PointParams* prm) override;

	// Helper(s)
	void UpdateCallID(const VS_SimpleStr& call_id,VS_CallIDInfo& ci,long type,unsigned char hops,const VS_SimpleStr& gate);
	void PushWithCheck(const VS_SimpleStr& call_id, VS_CallIDInfo& ci);
	void CleanServer(const char* server);

	int AddConnectedServer(const char* server);
	bool IsConnectedServer(const char* server);
	void DelConnectedServer(const char* server);

    // Service implementation

        /// RESOLVE_METHOD(CALLID_PARAM)
    void Resolve_Method(const VS_SimpleStr& call_id);
	void ResolveAll_Method(VS_Container& cnt);
        /// UDPATESTATUS_METHOD(CALLID_PARAM,USERPRESSTATUS_PARAM,ENDPOINT_PARAM)
    void UpdateStatus_Method(VS_Container& cnt,const VS_FullID& src_id);
        /// SUBSCRIBE_METHOD((CALLID_PARAM,ENDPOINT_PARAM) [])
    void Subscribe_Method(VS_Container& cnt);
        /// UNSUBSCRIBE_METHOD((CALLID_PARAM,ENDPOINT_PARAM) [])
    void Unsubscribe_Method(VS_Container& cnt);
        /// REGISTERSTATUS_METHOD((CALLID_PARAM, ENDPOINT_PARAM, SEQUENCE_PARAM) [])
	bool RegisterStatus			(const VS_SimpleStr& call_id, const VS_SimpleStr& serverID,const char* temp_id, const char* seq, long user_type, const std::string& displayName);
	void RegisterStatus_Method (VS_Container &cnt);
        /// UNREGISTERSTATUS_METHOD((CALLID_PARAM,ENDPOINT_PARAM) [])
	void UnregisterStatus_Method(const VS_SimpleStr& call_id);

        /// GETALLUSERSTATUS_METHOD()
    void GetAllUserStatus_Method(const VS_FullID& id);
};

#endif