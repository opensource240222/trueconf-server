/**
 ****************************************************************************
 * (c) 2007 Visicron Systems, Inc.  http://www.visicron.net/
 *
 * Project: Visicron server services
 *
 * $Revision: 139 $
 * $History: VS_DBStorage.cpp $
 *
 * *****************  Version 139  *****************
 * User: Ktrushnikov  Date: 16.08.12   Time: 18:06
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - AUTO_PROF add to all funcs
 *
 * *****************  Version 138  *****************
 * User: Ktrushnikov  Date: 14.08.12   Time: 18:13
 * Updated in $/VSNA/Servers/BaseServer/Services
 * #12325
 * - more simple logic at server & db: don't check server property (or app
 * property) for valid params from client
 *
 * *****************  Version 137  *****************
 * User: Ktrushnikov  Date: 13.08.12   Time: 13:20
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - flag value fixed (UR_COMM_CHANGEPASSWORD=0x00200000)
 * - return type fixed
 * - dprint fixed
 *
 * *****************  Version 136  *****************
 * User: Ktrushnikov  Date: 8.08.12    Time: 19:16
 * Updated in $/VSNA/Servers/BaseServer/Services
 * #12325
 * - use vs_update_person stored procedure
 *
 * *****************  Version 135  *****************
 * User: Ktrushnikov  Date: 8.08.12    Time: 13:45
 * Updated in $/VSNA/Servers/BaseServer/Services
 * sp_change_profile stored procedure name fixed
 *
 * *****************  Version 134  *****************
 * User: Ktrushnikov  Date: 8.08.12    Time: 13:12
 * Updated in $/VSNA/Servers/BaseServer/Services
 * #12325
 * - ChangeProfile support added
 *
 * *****************  Version 133  *****************
 * User: Ktrushnikov  Date: 7.08.12    Time: 17:11
 * Updated in $/VSNA/Servers/BaseServer/Services
 * #12322
 * - change password from client supported
 * - return result from ADODB as Parameter (not as Recordset)
 *
 * *****************  Version 132  *****************
 * User: Ktrushnikov  Date: 3.08.12    Time: 18:12
 * Updated in $/VSNA/Servers/BaseServer/Services
 * #12823
 * - after AddToAddressBook call OnAddressBookChange to update AB_GROUP
 * for owner
 *
 * *****************  Version 131  *****************
 * User: Mushakov     Date: 16.07.12   Time: 16:29
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - g_dbStorage was wrapped to shared_ptr
 *
 * *****************  Version 130  *****************
 * User: Smirnov      Date: 5.07.12    Time: 12:46
 * Updated in $/VSNA/Servers/BaseServer/Services
 * -fix bug in search (from terminal)
 *
 * *****************  Version 129  *****************
 * User: Ktrushnikov  Date: 17.04.12   Time: 19:05
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - #11610: Result and Cause params at BS as at VCS
 *
 * *****************  Version 128  *****************
 * User: Ktrushnikov  Date: 11.04.12   Time: 14:45
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - #11566: BS crash at 1st call prop with null length and 2nd call prop
 * not null
 * - ep_prop_set value size set 8192 (OpenGL prop at android can be long)
 * - ep_group_prop_set Processor value size set 8192 (long string from
 * Android)
 *
 * *****************  Version 127  *****************
 * User: Ktrushnikov  Date: 3.04.12    Time: 12:58
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - my aliases support added (for Android) AB_MY_EXTERNAL_CONTACTS=18
 * spec: http://projects.trueconf.ru/bin/view/Projects/MobilePhoneNumbers
 *
 * *****************  Version 126  *****************
 * User: Ktrushnikov  Date: 20.03.12   Time: 17:17
 * Updated in $/VSNA/Servers/BaseServer/Services
 * #11179: requested search params in response
 *
 * *****************  Version 125  *****************
 * User: Ktrushnikov  Date: 24.02.12   Time: 14:27
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - vs_bak_add_user supported (Add & Remove)
 *
 * *****************  Version 124  *****************
 * User: Ktrushnikov  Date: 16.02.12   Time: 17:10
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - #10663: set endpoint property value size = 2048
 *
 * *****************  Version 123  *****************
 * User: Ktrushnikov  Date: 21.12.11   Time: 18:46
 * Updated in $/VSNA/Servers/BaseServer/Services
 *
 * *****************  Version 122  *****************
 * User: Ktrushnikov  Date: 11/10/11   Time: 6:58p
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - Groups: GTYPE_PARAM instead of TYPE_PARAM
 *
 * *****************  Version 121  *****************
 * User: Ktrushnikov  Date: 26.10.11   Time: 17:59
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - quotes deleted from template (because are added when called)
 *
 * *****************  Version 120  *****************
 * User: Ktrushnikov  Date: 26.10.11   Time: 14:18
 * Updated in $/VSNA/Servers/BaseServer/Services
 * NamedConf
 * - set MaxCast for NamedConf with value from DB (edit server property
 * named_conf_max_cast)
 *
 * *****************  Version 119  *****************
 * User: Ktrushnikov  Date: 20.10.11   Time: 19:03
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - quotes
 *
 * *****************  Version 118  *****************
 * User: Ktrushnikov  Date: 20.10.11   Time: 16:19
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - % mistake
 *
 * *****************  Version 117  *****************
 * User: Ktrushnikov  Date: 20.10.11   Time: 13:47
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - FindUsersGroups, Rename, Delete: with no RecordSet
 * - Groups: if no hash - then hash=1 (2000 year)
 *
 * *****************  Version 116  *****************
 * User: Ktrushnikov  Date: 19.10.11   Time: 11:02
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - dprint4 at Init() for each procedure
 *
 * *****************  Version 115  *****************
 * User: Ktrushnikov  Date: 17.10.11   Time: 17:25
 * Updated in $/VSNA/Servers/BaseServer/Services
 * ManageGroups:
 * - pass GID_PARAM to client as String (not as Integer)
 *
 * *****************  Version 114  *****************
 * User: Ktrushnikov  Date: 10.10.11   Time: 13:42
 * Updated in $/VSNA/Servers/BaseServer/Services
 * #8948: GetBSEvents(): check value for null
 *
 * *****************  Version 113  *****************
 * User: Ktrushnikov  Date: 5.10.11    Time: 12:34
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - handle no RecordSet at FindUser() at PostreSQL
 *
 * *****************  Version 112  *****************
 * User: Mushakov     Date: 8.08.11    Time: 19:37
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - comments removed
 * - AddEndpointConditions(PRESENCE_SRV, &upss) in AS
 *
 * *****************  Version 111  *****************
 * User: Ktrushnikov  Date: 26.07.11   Time: 19:51
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - Hash added for AB_GROUPS
 * - ManageGroups moved to base interface class
 *
 * *****************  Version 110  *****************
 * User: Ktrushnikov  Date: 15.07.11   Time: 15:21
 * Updated in $/VSNA/Servers/BaseServer/Services
 * #8535, #8917
 * - user groups supported in Visicron.dll and BS server
 * (VS_AddressBookService, VS_DBStorage)
 *
 * *****************  Version 109  *****************
 * User: Ktrushnikov  Date: 11.07.11   Time: 21:51
 * Updated in $/VSNA/Servers/BaseServer/Services
 * [#8917]
 * - get AB_GROUPS from DB and send to client
 *
 * *****************  Version 108  *****************
 * User: Ktrushnikov  Date: 9.06.11    Time: 12:34
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - GetNamedConfInfo(): use VS_FileTime::FromVariant_NoTZ (not operator=)
 *
 * *****************  Version 107  *****************
 * User: Ktrushnikov  Date: 20.05.11   Time: 17:09
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - checkin mistake fix
 *
 * *****************  Version 106  *****************
 * User: Ktrushnikov  Date: 18.05.11   Time: 9:23
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - support conf_status in GetNamedConfInfo()
 *
 * *****************  Version 105  *****************
 * User: Ktrushnikov  Date: 28.03.11   Time: 23:05
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - email offline participant at VS_InvitesStorage
 * - new template added
 *
 * *****************  Version 104  *****************
 * User: Ktrushnikov  Date: 17.03.11   Time: 19:35
 * Updated in $/VSNA/Servers/BaseServer/Services
 * new style of system groups
 * - implementation (alfa)
 * - FindUserExtended() renamed to overloaded FindUser()
 * - OnUserChange(): type==4 supported
 *
 * *****************  Version 103  *****************
 * User: Ktrushnikov  Date: 1.03.11    Time: 22:53
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - pass current stream_conf_id of a named_conf_id from AS to BS->DB
 *
 * *****************  Version 102  *****************
 * User: Ktrushnikov  Date: 23.11.10   Time: 16:51
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - restart BS on any connection error (don't do reconnects or Init())
 *
 * *****************  Version 101  *****************
 * User: Ktrushnikov  Date: 13.11.10   Time: 14:50
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - sp_get_properties_list: appName param added
 * - pass appName from client to AS->BS->DB
 *
 * *****************  Version 100  *****************
 * User: Ktrushnikov  Date: 3.11.10    Time: 12:03
 * Updated in $/VSNA/Servers/BaseServer/Services
 *
 * *****************  Version 99  *****************
 * User: Ktrushnikov  Date: 1.11.10    Time: 13:14
 * Updated in $/VSNA/Servers/BaseServer/Services
 * [#7967]
 * - don't set LocalTime for call history stored procedures
 *
 * *****************  Version 98  *****************
 * User: Ktrushnikov  Date: 26.10.10   Time: 18:21
 * Updated in $/VSNA/Servers/BaseServer/Services
 *
 * *****************  Version 97  *****************
 * User: Ktrushnikov  Date: 11.10.10   Time: 16:41
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - offline chat messages support in PostgreSQL
 *
 * *****************  Version 96  *****************
 * User: Ktrushnikov  Date: 27.09.10   Time: 18:14
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - use_hash in try-catch()
 *
 * *****************  Version 95  *****************
 * User: Ktrushnikov  Date: 27.09.10   Time: 17:46
 * Updated in $/VSNA/Servers/BaseServer/Services
 *
 * *****************  Version 94  *****************
 * User: Ktrushnikov  Date: 27.09.10   Time: 14:23
 * Updated in $/VSNA/Servers/BaseServer/Services
 * delete test users
 *
 * *****************  Version 93  *****************
 * User: Ktrushnikov  Date: 26.09.10   Time: 14:06
 * Updated in $/VSNA/Servers/BaseServer/Services
 * VS_DBStorage::FindUsers()
 * - Get hash in PostgreSQL (no second RecordSet)
 * - if no hash in DB, return 1;
 * - skip "hash" field
 *
 * *****************  Version 92  *****************
 * User: Ktrushnikov  Date: 22.09.10   Time: 20:51
 * Updated in $/VSNA/Servers/BaseServer/Services
 * _wcsicmp() for key comparing in VS_DBStorage::FindUsers() [#7726]
 *
 * *****************  Version 91  *****************
 * User: Ktrushnikov  Date: 22.09.10   Time: 16:43
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - new search from client by [name, call_id, email]
 * - sp_set_group_endpoint_properties: directX param size fixed for
 * PostgreSQL
 * - VS_WideStr::Trim() function
 *
 * *****************  Version 90  *****************
 * User: Ktrushnikov  Date: 20.09.10   Time: 15:18
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - Aliases support for PostgreSQL
 *
 * *****************  Version 89  *****************
 * User: Ktrushnikov  Date: 6.09.10    Time: 17:19
 * Updated in $/VSNA/Servers/BaseServer/Services
 * BS Event "USER" supported [#6978]
 * - when user deleted from DB - logout him from AS
 * - cleanup m_loginData at this AS
 *
 * *****************  Version 88  *****************
 * User: Ktrushnikov  Date: 3.09.10    Time: 15:53
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - restart server if DB Reocnnects more than MAX_DB_RECONNECTS=20 times
 * (over 50 MB memory leaks)
 *
 * *****************  Version 87  *****************
 * User: Ktrushnikov  Date: 1.09.10    Time: 15:01
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - check DB connection at ADO COM Error only
 * - re Init DBStorage, not one DB Object (like in MS SQL)
 * - NativeError: driver error code for PostgreSQL connection problem
 * added
 *
 * *****************  Version 86  *****************
 * User: Ktrushnikov  Date: 1.09.10    Time: 13:07
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - delete dbo pool removed: cause memory leak, but working.
 *
 * *****************  Version 85  *****************
 * User: Ktrushnikov  Date: 31.08.10   Time: 17:18
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - db_ping before each procedure
 * - Reconnect if DB connection dead
 * - small cleanup at db::init()
 *
 * *****************  Version 84  *****************
 * User: Ktrushnikov  Date: 30.08.10   Time: 20:09
 * Updated in $/VSNA/Servers/BaseServer/Services
 * [#7571]
 * - New BS Event "TARIFF_CHANGED": generate UPDATEACCOUNT with TarifName,
 * TarifRestrictions, UserRights
 *
 * *****************  Version 83  *****************
 * User: Ktrushnikov  Date: 30.08.10   Time: 14:17
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - GetAllPropertiesList(): user properties (all the same in one DB now)
 *
 * *****************  Version 82  *****************
 * User: Ktrushnikov  Date: 25.08.10   Time: 20:31
 * Updated in $/VSNA/Servers/BaseServer/Services
 * physical restrictions on tarif (creating confs from client)
 *
 * *****************  Version 81  *****************
 * User: Ktrushnikov  Date: 18.08.10   Time: 18:49
 * Updated in $/VSNA/Servers/BaseServer/Services
 * Arch3.1:Tarif
 * - get TarifName & TarifRestictions from DB
 * - dummy test code with tarifs
 *
 * *****************  Version 80  *****************
 * User: Ktrushnikov  Date: 18.08.10   Time: 13:01
 * Updated in $/VSNA/Servers/BaseServer/Services
 * GetLoginSessionSecret: trap removed
 *
 * *****************  Version 79  *****************
 * User: Ktrushnikov  Date: 10.08.10   Time: 22:29
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - vs_get_login_session_secret (dummy call)
 *
 * *****************  Version 78  *****************
 * User: Ktrushnikov  Date: 10.08.10   Time: 21:28
 * Updated in $/VSNA/Servers/BaseServer/Services
 * [#7561]: GenerateSessionKey() at LoginUser and at ReqUpdateAccount()
 * [#7562]: update login session key at timer timeout in Visicron.dll
 *
 * *****************  Version 77  *****************
 * User: Mushakov     Date: 29.07.10   Time: 15:52
 * Updated in $/VSNA/Servers/BaseServer/Services
 *  - login UTF8->WideChar
 *
 * *****************  Version 76  *****************
 * User: Ktrushnikov  Date: 15.07.10   Time: 22:47
 * Updated in $/VSNA/Servers/BaseServer/Services
 * set parametr size for update_conf_info (for PostgreSQL only)
 *
 * *****************  Version 75  *****************
 * User: Ktrushnikov  Date: 14.07.10   Time: 21:56
 * Updated in $/VSNA/Servers/BaseServer/Services
 * Arch 3.1: NamedConf
 * - support for one-time invitation at conf start_time
 *
 * *****************  Version 74  *****************
 * User: Ktrushnikov  Date: 12.07.10   Time: 13:11
 * Updated in $/VSNA/Servers/BaseServer/Services
 * GetBSEvents:
 * - обход путого RecordSet дл PostgreSQL
 * - совместимост с MS SQL
 *
 * *****************  Version 73  *****************
 * User: Ktrushnikov  Date: 8.07.10    Time: 22:22
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - BS Event об изменених в именованной конфе
 * - передача owner_param
 * - передача Connect информации, если зер сидит не на AS дл конференции
 * - задание m_timeExp значением end_time из БД
 * - назначат AS дл конфы первым зашедшим
 *
 * *****************  Version 72  *****************
 * User: Ktrushnikov  Date: 7.07.10    Time: 21:12
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - named confs procedures & parameters fixed
 * - init_named_conf_invitations processing added
 *
 * *****************  Version 71  *****************
 * User: Ktrushnikov  Date: 1.07.10    Time: 22:33
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - test code removed
 *
 * *****************  Version 70  *****************
 * User: Ktrushnikov  Date: 1.07.10    Time: 20:56
 * Updated in $/VSNA/Servers/BaseServer/Services
 * Arch 3.1: NamedConfs
 * - DB interfaces created (with dummy & untested imp)
 * - Processing of InvitationUpdate_Method added
 * - BS Event "NAMED_CONF" rewritten
 * - bug fix with storing changed in struct (in map)
 * - some reject messages added
 *
 * *****************  Version 69  *****************
 * User: Ktrushnikov  Date: 29.06.10   Time: 12:21
 * Updated in $/VSNA/Servers/BaseServer/Services
 * Arch 3.1: NamedConfs invitation:
 * - RESOLVEALL_METHOD to get server & status of users
 * - start resolve in separate thread (PoolThreads)
 * - ask SM for RS in BS
 * - access to RS name from all BS services via locks
 * - reconnect to RS
 * - calc is_time fixed
 *
 * *****************  Version 68  *****************
 * User: Mushakov     Date: 31.05.10   Time: 15:01
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - param name corrected
 *
 * *****************  Version 67  *****************
 * User: Ktrushnikov  Date: 25.05.10   Time: 20:07
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - IsPostgreSQL flag added
 * - some dummy funcs for PostgreSQL
 *
 * *****************  Version 66  *****************
 * User: Mushakov     Date: 19.03.10   Time: 17:59
 * Updated in $/VSNA/Servers/BaseServer/Services
 *  - ClientType is received from client
 * - Endpoint function removed from code
 * - arr_key logged inRegistryServer
 *
 * *****************  Version 65  *****************
 * User: Smirnov      Date: 21.01.10   Time: 16:44
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - profiling for find_users extended
 *
 * *****************  Version 64  *****************
 * User: Ktrushnikov  Date: 21.10.09   Time: 17:20
 * Updated in $/VSNA/Servers/BaseServer/Services
 * [#6623] pass rating in login_user(): DB -> BS -> AS
 * - AB_INVERSE event support
 *
 * *****************  Version 63  *****************
 * User: Ktrushnikov  Date: 16.09.09   Time: 18:48
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - merge
 *
 * *****************  Version 62  *****************
 * User: Mushakov     Date: 15.09.09   Time: 21:15
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - some changes are canceled
 *
 * *****************  Version 58  *****************
 * User: Mushakov     Date: 7.08.09    Time: 15:14
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - SBSv3 added by Matvey (SBSv3_m)
 *
 * *****************  Version 57  *****************
 * User: Ktrushnikov  Date: 4.08.09    Time: 13:24
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - BS Event: IS_FRIEND: two events when 2 users become friends (each to
 * one of users)
 *
 * *****************  Version 56  *****************
 * User: Smirnov      Date: 8.05.09    Time: 19:50
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - some auto_prof removed
 *
 * *****************  Version 55  *****************
 * User: Ktrushnikov  Date: 8.05.09    Time: 17:53
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - cache for OfflineChatMessages
 *
 * *****************  Version 54  *****************
 * User: Smirnov      Date: 27.04.09   Time: 16:55
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - log part stat on other server
 *
 * *****************  Version 53  *****************
 * User: Mushakov     Date: 17.03.09   Time: 17:18
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - sp_update_conf_info call moved to second DB
 *
 * *****************  Version 52  *****************
 * User: Mushakov     Date: 26.02.09   Time: 18:56
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - get_missed/placed/received calls moved in 2d DB
 *
 * *****************  Version 51  *****************
 * User: Mushakov     Date: 20.02.09   Time: 19:26
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - AppId added in ParticipantDescr (invite, join log)
 * - 2 DBs supported
 *
 * *****************  Version 50  *****************
 * User: Ktrushnikov  Date: 3.12.08    Time: 14:18
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - COMError text fixed
 *
 * *****************  Version 49  *****************
 * User: Smirnov      Date: 3.12.08    Time: 13:08
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - unban added
 *
 * *****************  Version 48  *****************
 * User: Ktrushnikov  Date: 1.12.08    Time: 13:49
 * Updated in $/VSNA/Servers/BaseServer/Services
 * [#5231] param CallId added
 *
 * *****************  Version 47  *****************
 * User: Mushakov     Date: 24.11.08   Time: 17:49
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - small bug fix
 *
 * *****************  Version 46  *****************
 * User: Ktrushnikov  Date: 13.11.08   Time: 18:32
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - BS Events added
 *
 * *****************  Version 45  *****************
 * User: Mushakov     Date: 30.10.08   Time: 20:33
 * Updated in $/VSNA/Servers/BaseServer/Services
 *  - Alias status sending removed (AS ->RS )
 * - Logging of subType added
 *
 * *****************  Version 44  *****************
 * User: Smirnov      Date: 8.10.08    Time: 21:39
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - fixed bug with variant safearray
 *
 * *****************  Version 43  *****************
 * User: Smirnov      Date: 3.10.08    Time: 18:09
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - correct wrong parametrs in conference picture update
 *
 * *****************  Version 42  *****************
 * User: Mushakov     Date: 26.09.08   Time: 20:03
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - New Group Conf's atributes supported;
 *
 * *****************  Version 41  *****************
 * User: Mushakov     Date: 17.09.08   Time: 16:55
 * Updated in $/VSNA/Servers/BaseServer/Services
 *  - properties encoding corrected
 *
 * *****************  Version 40  *****************
 * User: Mushakov     Date: 10.09.08   Time: 19:08
 * Updated in $/VSNA/Servers/BaseServer/Services
 *  - sending missed call mail for email of existing user (call_id ==
 * email)
 *
 * *****************  Version 39  *****************
 * User: Mushakov     Date: 9.09.08    Time: 16:19
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - bug 4603 fixed
 *
 * *****************  Version 38  *****************
 * User: Mushakov     Date: 25.07.08   Time: 14:51
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - logging app_ID added
 * - logging multi_conf
 * - bug 4602 fixed
 *
 * *****************  Version 37  *****************
 * User: Mushakov     Date: 15.07.08   Time: 18:12
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - LogConfStart LogConfEnd time corrected
 * - SrvRespond added to registry
 *
 * *****************  Version 36  *****************
 * User: Mushakov     Date: 1.07.08    Time: 20:04
 * Updated in $/VSNA/Servers/BaseServer/Services
 *  - Profiler was realized
 *
 * *****************  Version 35  *****************
 * User: Mushakov     Date: 11.06.08   Time: 18:45
 * Updated in $/VSNA/Servers/BaseServer/Services
 * additional logging ProcessCOMError added
 *
 * *****************  Version 34  *****************
 * User: Mushakov     Date: 26.05.08   Time: 19:37
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - params added to SetAllEpProperties method
 *
 * *****************  Version 33  *****************
 * User: Smirnov      Date: 19.05.08   Time: 18:32
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - removed ep_update_sc
 *
 * *****************  Version 32  *****************
 * User: Stass        Date: 15.05.08   Time: 18:20
 * Updated in $/VSNA/Servers/BaseServer/Services
 * silent reject login in case of ADO/COM error
 *
 * *****************  Version 31  *****************
 * User: Mushakov     Date: 15.05.08   Time: 15:04
 * Updated in $/VSNA/Servers/BaseServer/Services
 * missed call mail corrcted
 *
 * *****************  Version 30  *****************
 * User: Stass        Date: 25.04.08   Time: 17:10
 * Updated in $/VSNA/Servers/BaseServer/Services
 * bs fix
 *
 * *****************  Version 29  *****************
 * User: Ktrushnikov  Date: 16.04.08   Time: 21:28
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - "wan_ip" param is set alone, not with SetAllEpProperties
 * (sp_set_group_edpoint_properties)
 *
 * *****************  Version 28  *****************
 * User: Stass        Date: 15.04.08   Time: 13:10
 * Updated in $/VSNA/Servers/BaseServer/Services
 * offline chat
 *
 * *****************  Version 27  *****************
 * User: Ktrushnikov  Date: 14.04.08   Time: 19:56
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - bugfix with languages in GetOfflineChatMessages()
 *
 * *****************  Version 26  *****************
 * User: Stass        Date: 2.04.08    Time: 21:44
 * Updated in $/VSNA/Servers/BaseServer/Services
 * added set server to BS presense
 *
 * *****************  Version 25  *****************
 * User: Stass        Date: 1.04.08    Time: 20:24
 * Updated in $/VSNA/Servers/BaseServer/Services
 * added ban list support
 *
 * *****************  Version 24  *****************
 * User: Ktrushnikov  Date: 16.03.08   Time: 12:24
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - Server Property functions commented (cause they are not used)
 * - set_user_status uses stored procedure in DB (not a hardcoded
 * SQL-string)
 *
 * *****************  Version 23  *****************
 * User: Ktrushnikov  Date: 15.03.08   Time: 12:07
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - std::vector now used to pass return data from VS_DBStorage
 * - get data from DB by char-constant, not by index
 *
 * *****************  Version 22  *****************
 * User: Smirnov      Date: 13.03.08   Time: 16:47
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - memory leaks
 *
 * *****************  Version 21  *****************
 * User: Ktrushnikov  Date: 12.03.08   Time: 20:01
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - BS: CHAT_SRV added
 * - Pass offline chat messages from AS::RESOLVE_SRV to BS::CHAT_SRV
 * - Save offline chat messages in DB via sp_friend_im_save stored
 * procedure
 *
 * *****************  Version 20  *****************
 * User: Ktrushnikov  Date: 11.03.08   Time: 15:40
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - call PostRequest for each message (not one call)
 *
 * *****************  Version 19  *****************
 * User: Ktrushnikov  Date: 9.03.08    Time: 15:00
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - added GetOfflineChatMessages() using sp_notify_login stored procedure
 * from VS_DBStorage
 *
 * *****************  Version 18  *****************
 * User: Stass        Date: 27.02.08   Time: 21:09
 * Updated in $/VSNA/Servers/BaseServer/Services
 * new parameters in addtoAB
 *
 * *****************  Version 17  *****************
 * User: Mushakov     Date: 19.02.08   Time: 18:54
 * Updated in $/VSNA/Servers/BaseServer/Services
 * ReleaseDBO added
 *
 * *****************  Version 16  *****************
 * User: Mushakov     Date: 19.02.08   Time: 17:42
 * Updated in $/VSNA/Servers/BaseServer/Services
 *  - some comments were deleted
 *
 * *****************  Version 15  *****************
 * User: Ktrushnikov  Date: 19.02.08   Time: 15:32
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - build errors fix
 *
 * *****************  Version 14  *****************
 * User: Mushakov     Date: 18.02.08   Time: 12:26
 * Updated in $/VSNA/Servers/BaseServer/Services
 * for compiling
 *
 * *****************  Version 13  *****************
 * User: Ktrushnikov  Date: 17.02.08   Time: 10:58
 * Updated in $/VSNA/Servers/BaseServer/Services
 * - HardwareTest: fix output cause of funcs have other names
 * - Stored Procedure: call SetAllEpProperties() for known params
 * - split Capabilities into: is_audio, is_mic, is_camera
 *
 * *****************  Version 12  *****************
 * User: Mushakov     Date: 15.02.08   Time: 20:21
 * Updated in $/VSNA/Servers/BaseServer/Services
 * sending missed call mail and invate mail added
 *
 * *****************  Version 11  *****************
 * User: Stass        Date: 13.02.08   Time: 15:53
 * Updated in $/VSNA/Servers/BaseServer/Services
 * fixed release
 *
 * *****************  Version 10  *****************
 * User: Stass        Date: 12.02.08   Time: 20:31
 * Updated in $/VSNA/Servers/BaseServer/Services
 * passing aliases to as
 *
 * *****************  Version 9  *****************
 * User: Mushakov     Date: 12.02.08   Time: 19:13
 * Updated in $/VSNA/Servers/BaseServer/Services
 * GetAppProperties method realized
 *
 * *****************  Version 8  *****************
 * User: Stass        Date: 1.02.08    Time: 21:51
 * Updated in $/VSNA/Servers/BaseServer/Services
 * debug printout
 *
 * *****************  Version 7  *****************
 * User: Mushakov     Date: 21.01.08   Time: 19:55
 * Updated in $/VSNA/Servers/BaseServer/Services
 * BaseUserPresence added
 *
 * *****************  Version 6  *****************
 * User: Stass        Date: 13.12.07   Time: 16:40
 * Updated in $/VSNA/Servers/BaseServer/Services
 * fixed send login
 *
 * *****************  Version 5  *****************
 * User: Stass        Date: 10.12.07   Time: 16:24
 * Updated in $/VSNA/Servers/BaseServer/Services
 * fixed init
 *
 * *****************  Version 4  *****************
 * User: Stass        Date: 7.12.07    Time: 13:21
 * Updated in $/VSNA/Servers/BaseServer/Services
 * fixed link problems
 *
 * *****************  Version 3  *****************
 * User: Stass        Date: 6.12.07    Time: 18:51
 * Updated in $/VSNA/Servers/BaseServer/Services
 * base services done
 *
 * *****************  Version 2  *****************
 * User: Stass        Date: 5.12.07    Time: 11:09
 * Updated in $/VSNA/Servers/BaseServer/Services
 * BS - new iteration
 *
 * *****************  Version 1  *****************
 * User: Stass        Date: 28.11.07   Time: 18:33
 * Created in $/VSNA/Servers/BaseServer/Services
 * first bs ver
 *
 ****************************************************************************/

/**
 * \file VS_DBStorage.cpp
 * \brief Server Database Storage class implementation functions
 *
 */

#include "VS_DBStorageInterface.h"
std::shared_ptr<VS_DBStorageInterface>	g_dbStorage;

#ifdef _WIN32 // not ported

#include "VS_DBStorage.h"
#include "../../../common/std/cpplib/VS_RegistryKey.h"
#include "../../../common/std/cpplib/VS_RegistryConst.h"
#include "../../../common/std/cpplib/VS_Replace.h"
#include "std/cpplib/StrFromVariantT.h"
#include "std/cpplib/VS_CallIDUtils.h"
#include "../../../common/std/VS_ProfileTools.h"
#include "std-generic/cpplib/string_view.h"
#include "../../../common/statuslib/VS_ExtendedStatus.h"
#include "std-generic/cpplib/scope_exit.h"
#include "../../../common/std/cpplib/base64.h"
#include "../../../common/std/cpplib/VS_FileTime.h"
#include "std/CallLog/VS_ConferenceDescription_io.h"
#include "tlb_import/msado26.tli"
#include "std/cpplib/json/elements.h"
#include "std/cpplib/json/writer.h"

#include "std-generic/compat/memory.h"
#include <sstream>
#include <ctime>
#include <time.h>
#include <boost/algorithm/string/iter_find.hpp>
#include <boost/algorithm/string/finder.hpp>

#define DEBUG_CURRENT_MODULE VS_DM_DBSTOR
namespace
{
const int reconnect_max			=2;
const int reconnect_timeout	=10000;

class status_value_to_string : public boost::static_visitor<std::pair<bool, std::string>>
{
public:
	/**
		first - value has been converted;
		second - conversion result;
	*/
	template<typename T>
	std::pair<bool,std::string> operator()(const T &val) const
	{
		return std::make_pair(true,std::to_string(val));
	}
	template<>
	std::pair<bool, std::string> operator()(const std::string &val) const
	{
		return std::make_pair(true,val);
	}
	template<>
	std::pair<bool, std::string> operator ()(const std::vector<uint8_t> &val) const
	{
		return std::make_pair(false, std::string());
	}
};
}

std::string VS_DBStorage::STRING_SEPARATOR;

VS_DBObjects* VS_DBStorage::GetDBO(const VS_Pool::Item* &item) {
	int n = 0;
	while(state==STATE_RECONNECT) {
		dprint1("POOL: Reconnect state, sleeping\n");
		Sleep(10000);
		n++;
		if (n >= 3*6)		// 3 minutes (3*6*10000)
		{
			state = STATE_FAILED;
		}
	};

	n=0;
	while((item=m_dbo_pool->Get())==NULL) {
		dprint1("POOL: no free db objects, sleeping\n");
		Sleep(500);
		n++;
		if (n >= 3*6*20)	// 3 minutes (3*6*20*500)
		{
			state = STATE_FAILED;
		}
	};

	return (VS_DBObjects*)item->m_data;
}

void VS_DBObjects::ProcessCOMError(const _com_error &e, ADODB::_ConnectionPtr cur_db=0, VS_DBStorage* dbs=NULL,const char *add_descr = 0)
{
AUTO_PROF

	ADODB::_ConnectionPtr	err_db = !cur_db? db: cur_db;

	_bstr_t bstrSource(e.Source());
	_bstr_t bstrDescription(e.Description());

	error_code=VSS_DB_ADO_ERROR;

	VS_FileTime t;
	t.Now();
	char buf[256];

	// Print COM errors.
	int count=err_db?err_db->Errors->Count:0;
	dprint1("#at %s\n",t.ToNStr(buf));
	if(count==0)
	{
		dprint1("#COM Error [%08lx] \n",e.Error());
		dprint1(" Msg         = %s \n", e.ErrorMessage());
		dprint1(" Source      = %s \n", (LPCSTR) bstrSource);
		dprint1(" Description = %s \n", (LPCSTR) bstrDescription);
		if(add_descr)
			dprint1(" Add descr = %s \n",add_descr);
		error_code=VSS_DB_COM_ERROR;
		if(dbs)
			dbs->error_code=error_code;
		dprint1(" DB State: %02lx \n", err_db?err_db->State:-1);
	}
	else
	{
		ADODB::ErrorPtr ae= err_db->Errors->GetItem((short)0);

		dprint1("#ADO Error [%08lx] native(%08lx) (total:%d) SQLState(%s)\n",ae->Number,ae->NativeError,count,(LPCSTR)ae->SQLState);
		dprint1(" Source = %s \n", (char*)(_bstr_t)ae->Source);
		dprint1(" Description = %s \n", (char*)(_bstr_t)ae->Description);
		if(add_descr)
			dprint1(" Add descr = %s \n",add_descr);
		error_code=VSS_DB_ADO_ERROR;
		if(dbs)
			dbs->error_code=error_code;
		dprint1(" DB State: %02lx \n", err_db?err_db->State:-1);

		if ((!!ae->SQLState && _strnicmp(ae->SQLState,"08",2)==0) ||		// SQLState error: Class 08 - Connection Exception (http://www.postgresql.org/docs/current/static/errcodes-appendix.html)
			(ae->Number==0x80004005 && (ae->NativeError==0x0 || ae->NativeError==0x11 || (IsPostgreSQL && (ae->NativeError==0x1a)))) )
		{
			if (IsPostgreSQL){
				dprint1("#DB connection error, set state=STATE_FAILED\n");
				if (dbs)
					dbs->state=dbs->STATE_FAILED;
			}else {		// MS SQL reconnects handling

				if(dbs && dbs->state==dbs->STATE_RUNNING)
				{
					dprint0("#DB connection error, trying to reconnect\n");
					dbs->state=dbs->STATE_RECONNECT;
					if(!Init(dbs))
					{
						dbs->state=dbs->STATE_FAILED;
						dprint0("#reconnect failed, setting storage state to FAIL\n");
					}/*else
						dbs->state=dbs->STATE_RUNNING;*/
				}
				else
				{
					dprint1("db connection error\n");
				};

			}
			//error_code=VSS_DB_CONN_ERROR
		}
		else
		{
			dprint1("other error\n");
		};
	};

	fflush(stdout);
};

VS_DBObjects::VS_DBObjects()
	:db(0),db1(0),
	ab_add_user(0),ab_remove_user(0),
	ab_get(0),ab_missed_calls(0),ab_received_calls(0),ab_placed_calls(0),ab_get_person_details(0),
	ab_update_user(0),ab_update_details(0),ab_update_picture(0),ab_delete_picture(0),ab_get_missed_call_mail(0),
	user_login(0), sp_login_temp_set_param(0), user_find(0),
	//log_conf_start(0),log_conf_end(0),log_part_join(0),log_part_leave(0),log_part_invite(0),log_part_stats(0),
	app_prop_get(0),app_prop_get_all(0),/*srv_prop_get(0),srv_prop_get_all(0),srv_prop_set(0),*/ep_prop_get(0),ep_prop_set(0),user_prop_get(0),
	error_code(0),
	user_set_status(0),
	get_bs_events(0),
	get_users_with_offline_messages(0),
	get_named_conf_info(0),
	set_named_conf_server(0),
	get_named_conf_participants(0),
	init_named_conf_invitations(0),
	get_aliases(0), search_users(0),
	change_password(0), update_person(0),
	ab_phones_add(0), ab_phones_update(0), ab_phones_delete(0),
	user_set_ext_status(0), ext_statuses_get_allowed(0), user_get_ext_status(0)
{
	IsPostgreSQL = false;
}

bool VS_DBObjects::Init(VS_DBStorage* dbs)
{
AUTO_PROF
	char buff[512];

	////////////////////////////////////////////////////////////
	// database init
	VS_RegistryKey    cfg_root(false, CONFIGURATION_KEY, false, true);
	if (!cfg_root.IsValid()) {
	  error_code=VSS_REGISTRY_ERROR;
	  return false;
	};

	VS_SimpleStr conn,username,password;

	if (cfg_root.GetValue(buff, sizeof(buff), VS_REG_STRING_VT,DB_CONNECTIONSTRING_TAG ) > 0)
	  conn = buff;
	if (cfg_root.GetValue(buff, sizeof(buff), VS_REG_STRING_VT,DB_PASSWORD_TAG) > 0)
	  password = buff;
	if (cfg_root.GetValue(buff, sizeof(buff), VS_REG_STRING_VT,DB_USER_TAG ) > 0)
	  username = buff;

	if (conn && *conn && strstr(conn, "PostgreSQL")!=0)
		IsPostgreSQL = true;

	fflush(stdout);

	try
	{db.CreateInstance("ADODB.Connection");}
	catch(_com_error e)
	{
		ProcessCOMError(e,db,dbs);
		return false;
	}

	bool init=false;
	for (int i = 0; i < ::reconnect_max; i++)
	{
	  try
	  {
		db->Open((char *)conn,(char *)username,(char *)password,ADODB::adConnectUnspecified);
		init=true;
		break;
	  }
	  catch(_com_error e)
	  {
		  dprint1(".%d.\n",::reconnect_max-i);
		  ProcessCOMError(e,db,dbs);
		  fflush(stdout);
		  SleepEx(::reconnect_timeout,false);

	  }
	};
	if(!init)
		return false;

	VS_SimpleStr conn1,username1,password1;

	if (cfg_root.GetValue(buff, sizeof(buff), VS_REG_STRING_VT,DB_CONNECTIONSTRING_TAG_1 ) > 0)
	  conn1 = buff;
	if (cfg_root.GetValue(buff, sizeof(buff), VS_REG_STRING_VT,DB_PASSWORD_TAG_1) > 0)
	  password1 = buff;
	if (cfg_root.GetValue(buff, sizeof(buff), VS_REG_STRING_VT,DB_USER_TAG_1 ) > 0)
	  username1 = buff;

	fflush(stdout);

	try
	{db1.CreateInstance("ADODB.Connection");}
	catch(_com_error e)
	{
		ProcessCOMError(e);
		return false;
	}
	for (int i = 0; i < ::reconnect_max; i++)
	{
	  try
	  {
		db1->Open((char *)conn1,(char *)username1,(char *)password1,ADODB::adConnectUnspecified);
		init=true;
		break;
	  }
	  catch(_com_error e)
	  {
		  init = false;
		  dprint1(".%d.\n",::reconnect_max-i);
		  ProcessCOMError(e,db1,dbs);
		  fflush(stdout);
		  SleepEx(::reconnect_timeout,false);
	  }
	};


	  if(!init)
			return false;

	  printf(".");
	  try
		{
			//create procedures
			ADODB::_ParameterPtr par;

			dprint4("sp_login_user\n");
			user_login.CreateInstance("ADODB.Command");
			user_login->ActiveConnection=db;
			user_login->CommandText="sp_login_user";
			user_login->CommandType=ADODB::adCmdStoredProc;
			user_login->Parameters->Refresh();

			dprint4("sp_login_temp_set_param\n");
			sp_login_temp_set_param.CreateInstance("ADODB.Command");
			sp_login_temp_set_param->ActiveConnection = db;

			dprint4("sp_find_user\n");
			user_find_template = "select * from sp_find_user('%call_id%', '%search_type%');";
			user_find.CreateInstance("ADODB.Command");
			user_find->ActiveConnection=db;
			//user_find->CommandText="sp_find_user";
			user_find->CommandType=ADODB::adCmdText;
			//user_find->Parameters->Refresh();

			dprint4("sp_add_to_address_book\n");
			ab_add_user.CreateInstance("ADODB.Command");
			ab_add_user->ActiveConnection=db;
			ab_add_user->CommandText="sp_add_to_address_book";
			ab_add_user->CommandType=ADODB::adCmdStoredProc;
			ab_add_user->Parameters->Refresh();

			dprint4("sp_ban_user\n");
			ban_user.CreateInstance("ADODB.Command");
			ban_user->ActiveConnection=db;
			ban_user->CommandText="sp_ban_user";
			ban_user->CommandType=ADODB::adCmdStoredProc;
			ban_user->Parameters->Refresh();

			dprint4("sp_unban_user\n");
			unban_user.CreateInstance("ADODB.Command");
			unban_user->ActiveConnection=db;
			unban_user->CommandText="sp_unban_user";
			unban_user->CommandType=ADODB::adCmdStoredProc;
			unban_user->Parameters->Refresh();

			dprint4("sp_ab_remove_user\n");
			ab_remove_user.CreateInstance("ADODB.Command");
			ab_remove_user->ActiveConnection=db;
			ab_remove_user->CommandText="sp_ab_remove_user";
			ab_remove_user->CommandType=ADODB::adCmdStoredProc;
			ab_remove_user->Parameters->Refresh();

			dprint4("sp_get_person_details\n");
			ab_get_person_details.CreateInstance("ADODB.Command");
			ab_get_person_details->ActiveConnection=db;
			ab_get_person_details->CommandText="sp_get_person_details";
			ab_get_person_details->CommandType=ADODB::adCmdStoredProc;
			ab_get_person_details->Parameters->Refresh();

			dprint4("sp_get_address_book\n");
			ab_get.CreateInstance("ADODB.Command");
			ab_get->ActiveConnection=db;
			ab_get->CommandText="sp_get_address_book";
			ab_get->CommandType=ADODB::adCmdStoredProc;
			ab_get->Parameters->Refresh();

			dprint4("sp_get_received_calls\n");
			ab_received_calls.CreateInstance("ADODB.Command");
			ab_received_calls->ActiveConnection=db1;
			ab_received_calls->CommandText="sp_get_received_calls";
			ab_received_calls->CommandType=ADODB::adCmdStoredProc;
			ab_received_calls->Parameters->Refresh();

			dprint4("sp_get_missed_calls\n");
			ab_missed_calls.CreateInstance("ADODB.Command");
			ab_missed_calls->ActiveConnection=db1;
			ab_missed_calls->CommandText="sp_get_missed_calls";
			ab_missed_calls->CommandType=ADODB::adCmdStoredProc;
			ab_missed_calls->Parameters->Refresh();

			dprint4("sp_get_placed_calls\n");
			ab_placed_calls.CreateInstance("ADODB.Command");
			ab_placed_calls->ActiveConnection=db1;
			ab_placed_calls->CommandText="sp_get_placed_calls";
			ab_placed_calls->CommandType=ADODB::adCmdStoredProc;
			ab_placed_calls->Parameters->Refresh();

			dprint4("sp_ab_update_user\n");
			ab_update_user.CreateInstance("ADODB.Command");
			ab_update_user->ActiveConnection=db;
			ab_update_user->CommandText="sp_ab_update_user";
			ab_update_user->CommandType=ADODB::adCmdStoredProc;
			ab_update_user->Parameters->Refresh();

			// TODO: implement for PostgreSQL
			if (!IsPostgreSQL)
			{
				dprint4("sp_ab_update_details\n");
				ab_update_details.CreateInstance("ADODB.Command");
				ab_update_details->ActiveConnection=db;
				ab_update_details->CommandText="sp_ab_update_details";
				ab_update_details->CommandType=ADODB::adCmdStoredProc;
				ab_update_details->Parameters->Refresh();
			}

			dprint4("sp_ab_update_picture\n");
			ab_update_picture.CreateInstance("ADODB.Command");
			ab_update_picture->ActiveConnection=db;
			ab_update_picture->CommandText="sp_ab_update_picture";
			ab_update_picture->CommandType=ADODB::adCmdStoredProc;
			ab_update_picture->Parameters->Refresh();

			dprint4("vs_user_delete_avatar\n");
			ab_delete_picture.CreateInstance("ADODB.Command");
			ab_delete_picture->ActiveConnection = db;
			ab_delete_picture->CommandText = "vs_user_delete_avatar";
			ab_delete_picture->CommandType = ADODB::adCmdStoredProc;
			ab_delete_picture->Parameters->Refresh();

			dprint4("sp_get_missed_call_mail\n");
			ab_get_missed_call_mail.CreateInstance("ADODB.Command");
			ab_get_missed_call_mail->ActiveConnection = db;
			ab_get_missed_call_mail->CommandText="sp_get_missed_call_mail";
			ab_get_missed_call_mail->CommandType=ADODB::adCmdStoredProc;
			ab_get_missed_call_mail->Parameters->Refresh();

			//dprint4("sp_log_conf_start\n");
			//log_conf_start.CreateInstance("ADODB.Command");
			//log_conf_start->ActiveConnection=db1; //
			//log_conf_start->CommandText="sp_log_conf_start";
			//log_conf_start->CommandType=ADODB::adCmdStoredProc;
			//log_conf_start->Parameters->Refresh();

			//dprint4("sp_log_conf_end\n");
			//log_conf_end.CreateInstance("ADODB.Command");
			//log_conf_end->ActiveConnection=db1;
			//log_conf_end->CommandText="sp_log_conf_end";
			//log_conf_end->CommandType=ADODB::adCmdStoredProc;
			//log_conf_end->Parameters->Refresh();

			dprint4("sp_update_conf_info\n");
			update_conf_info.CreateInstance("ADODB.Command");
			update_conf_info->ActiveConnection=db1;
			update_conf_info->CommandText = "sp_update_conf_info";
			update_conf_info->CommandType=ADODB::adCmdStoredProc;
			update_conf_info->Parameters->Refresh();
			if (IsPostgreSQL)
				update_conf_info->Parameters->Item[DB_CONFERENCE_PIC_PARAM]->Size = 10000;

			//dprint4("sp_log_participant_join\n");
			//log_part_join.CreateInstance("ADODB.Command");
			//log_part_join->ActiveConnection=db1;
			//log_part_join->CommandText="sp_log_participant_join";
			//log_part_join->CommandType=ADODB::adCmdStoredProc;
			//log_part_join->Parameters->Refresh();

			//dprint4("sp_log_participant_leave\n");
			//log_part_leave.CreateInstance("ADODB.Command");
			//log_part_leave->ActiveConnection=db1;
			//log_part_leave->CommandText="sp_log_participant_leave";
			//log_part_leave->CommandType=ADODB::adCmdStoredProc;
			//log_part_leave->Parameters->Refresh();

			//dprint4("sp_log_participant_invite\n");
			//log_part_invite.CreateInstance("ADODB.Command");
			//log_part_invite->ActiveConnection=db1;
			//log_part_invite->CommandText="sp_log_participant_invite";
			//log_part_invite->CommandType=ADODB::adCmdStoredProc;
			//log_part_invite->Parameters->Refresh();

			//dprint4("sp_log_participant_statistics\n");
			//log_part_stats.CreateInstance("ADODB.Command");
			//log_part_stats->ActiveConnection=db1;
			//log_part_stats->CommandText="sp_log_participant_statistics";
			//log_part_stats->CommandType=ADODB::adCmdStoredProc;
			//log_part_stats->Parameters->Refresh();

			dprint4("sp_get_app_property\n");
			app_prop_get.CreateInstance("ADODB.Command");
			app_prop_get->ActiveConnection=db;
			app_prop_get->CommandText="sp_get_app_property";
			app_prop_get->CommandType=ADODB::adCmdStoredProc;
			app_prop_get->Parameters->Refresh();

			dprint4("sp_get_app_properties\n");
			app_prop_get_all.CreateInstance("ADODB.Command");
			app_prop_get_all->ActiveConnection=db;
			app_prop_get_all->CommandText="sp_get_app_properties";
			app_prop_get_all->CommandType=ADODB::adCmdStoredProc;
			app_prop_get_all->Parameters->Refresh();

			dprint4("sp_get_apps\n");
			app_prop_get_all_props.CreateInstance("ADODB.Command");
			app_prop_get_all_props->ActiveConnection=db;
			app_prop_get_all_props->CommandText="sp_get_apps";
			app_prop_get_all_props->CommandType=ADODB::adCmdStoredProc;
			app_prop_get_all_props->Parameters->Refresh();

			dprint4("sp_get_server_property\n");
			srv_prop_get.CreateInstance("ADODB.Command");
			srv_prop_get->ActiveConnection=db;
			srv_prop_get->CommandText="sp_get_server_property";
			srv_prop_get->CommandType=ADODB::adCmdStoredProc;
			srv_prop_get->Parameters->Refresh();

			//srv_prop_get_all.CreateInstance("ADODB.Command");
			//srv_prop_get_all->ActiveConnection=db;
			//srv_prop_get_all->CommandText="sp_get_srv_properties";
			//srv_prop_get_all->CommandType=ADODB::adCmdStoredProc;
			//srv_prop_get_all->Parameters->Refresh();

			//srv_prop_set.CreateInstance("ADODB.Command");
			//srv_prop_set->ActiveConnection=db;
			//srv_prop_set->CommandText="sp_set_server_property";
			//srv_prop_set->CommandType=ADODB::adCmdStoredProc;
			//srv_prop_set->Parameters->Refresh();

			dprint4("sp_get_endpoint_property\n");
			ep_prop_get.CreateInstance("ADODB.Command");
			ep_prop_get->ActiveConnection=db;
			ep_prop_get->CommandText="sp_get_endpoint_property";
			ep_prop_get->CommandType=ADODB::adCmdStoredProc;
			ep_prop_get->Parameters->Refresh();

			dprint4("sp_set_endpoint_property\n");
			ep_prop_set.CreateInstance("ADODB.Command");
			ep_prop_set->ActiveConnection=db;
			ep_prop_set->CommandText="sp_set_endpoint_property";
			ep_prop_set->CommandType=ADODB::adCmdStoredProc;
			ep_prop_set->Parameters->Refresh();
			if (IsPostgreSQL)
				ep_prop_set->Parameters->Item[PROP_VALUE_PARAM]->Size = 8192;

			dprint4("sp_set_group_endpoint_properties\n");
			ep_group_prop_set.CreateInstance("ADODB.Command");
			ep_group_prop_set->ActiveConnection=db;
			ep_group_prop_set->CommandText="sp_set_group_endpoint_properties";
			ep_group_prop_set->CommandType=ADODB::adCmdStoredProc;
			ep_group_prop_set->Parameters->Refresh();
			if (IsPostgreSQL)
			{
				ep_group_prop_set->Parameters->Item[EP_DIRECTX_PARAM]->Size = 4096;
				ep_group_prop_set->Parameters->Item[EP_PROCESSOR_PARAM]->Size = 8192;
				ep_group_prop_set->Parameters->Item[EP_HARDWARE_CONFIG_PARAM]->Size = 8192;
				ep_group_prop_set->Parameters->Item[EP_AUDIO_CAPTURE_PARAM]->Size = 2048;
			}

			dprint4("sp_get_user_property\n");
			user_prop_get.CreateInstance("ADODB.Command");
			user_prop_get->ActiveConnection=db;
			user_prop_get->CommandText="sp_get_user_property";
			user_prop_get->CommandType=ADODB::adCmdStoredProc;
			user_prop_get->Parameters->Refresh();


			dprint4("sp_notify_login\n");
			notify_login.CreateInstance("ADODB.Command");
			notify_login->ActiveConnection=db;
			notify_login->CommandText="sp_notify_login";
			notify_login->CommandType=ADODB::adCmdStoredProc;
			notify_login->Parameters->Refresh();
			if (IsPostgreSQL)
				notify_login->Parameters->Item["container"]->Size = 8192;

			dprint4("sp_friend_im_save\n");
			friend_im_save.CreateInstance("ADODB.Command");
			friend_im_save->ActiveConnection=db;
			friend_im_save->CommandText="sp_friend_im_save";
			friend_im_save->CommandType=ADODB::adCmdStoredProc;
			friend_im_save->Parameters->Refresh();

			dprint4("sp_set_user_status\n");
			user_set_status.CreateInstance("ADODB.Command");
			user_set_status->ActiveConnection=db;
			user_set_status->CommandText="sp_set_user_status";
			user_set_status->CommandType=ADODB::adCmdStoredProc;
			user_set_status->Parameters->Refresh();

			dprint4("sp_ext_statuses_set\n");
			user_set_ext_status.CreateInstance("ADODB.Command");
			user_set_ext_status->ActiveConnection=db;
			user_set_ext_status->CommandText="sp_ext_statuses_set";
			user_set_ext_status->CommandType=ADODB::adCmdStoredProc;
			{
				auto param_ptr = user_set_ext_status->CreateParameter(CALL_ID_PARAM_SP, ADODB::DataTypeEnum::adBSTR, ADODB::ParameterDirectionEnum::adParamInput,0,NULL);
				user_set_ext_status->Parameters->Append(param_ptr);
				param_ptr = user_set_ext_status->CreateParameter(NAME_PARAM_SP, ADODB::DataTypeEnum::adBSTR, ADODB::ParameterDirectionEnum::adParamInput, 0, NULL);
				user_set_ext_status->Parameters->Append(param_ptr);
				param_ptr = user_set_ext_status->CreateParameter(VALUE_PARAM_SP, ADODB::DataTypeEnum::adBSTR, ADODB::ParameterDirectionEnum::adParamInput, 0, NULL);
				user_set_ext_status->Parameters->Append(param_ptr);
			}
			dprint4("sp_ext_statuses_get\n");
			user_get_ext_status.CreateInstance("ADODB.Command");
			user_get_ext_status->ActiveConnection = db;
			user_get_ext_status->CommandText = "sp_ext_statuses_get";
			user_get_ext_status->CommandType = ADODB::adCmdStoredProc;
			user_get_ext_status->Parameters->Refresh();

			dprint4("sp_ext_statuses_get_allowed\n");
			ext_statuses_get_allowed.CreateInstance("ADODB.Command");
			ext_statuses_get_allowed->ActiveConnection=db;
			ext_statuses_get_allowed->CommandText="sp_ext_statuses_get_allowed";
			ext_statuses_get_allowed->CommandType=ADODB::adCmdStoredProc;

			dprint4("sp_clear_statuses\n");
			user_clear_statuses.CreateInstance("ADODB.Command");
			user_clear_statuses->ActiveConnection=db;
			user_clear_statuses->CommandText="sp_clear_statuses";
			user_clear_statuses->CommandType=ADODB::adCmdStoredProc;
			{
				auto param_ptr = user_clear_statuses->CreateParameter(DB_SERVER_PARAM, ADODB::DataTypeEnum::adBSTR, ADODB::ParameterDirectionEnum::adParamInput, 0, NULL);
				user_clear_statuses->Parameters->Append(param_ptr);
			}

			dprint4("sp_cleanup\n");
			cleanup.CreateInstance("ADODB.Command");
			cleanup->ActiveConnection=db;
			cleanup->CommandText="sp_cleanup";
			cleanup->CommandType=ADODB::adCmdStoredProc;
			cleanup->Parameters->Refresh();

			dprint4("sp_get_bs_events\n");
			get_bs_events.CreateInstance("ADODB.Command");
			get_bs_events->ActiveConnection=db;
			if (IsPostgreSQL) {
				get_bs_events->CommandType=ADODB::adCmdText;
			}else{
				get_bs_events->CommandText="sp_get_bs_events";
				get_bs_events->CommandType=ADODB::adCmdStoredProc;
				get_bs_events->Parameters->Refresh();
			}

			dprint4("sp_get_users_with_offline_messages\n");
			get_users_with_offline_messages.CreateInstance("ADODB.Command");
			get_users_with_offline_messages->ActiveConnection=db;
			if (IsPostgreSQL) {
				get_users_with_offline_messages->CommandText="select * from sp_get_users_with_offline_messages();";
				get_users_with_offline_messages->CommandType=ADODB::adCmdText;
			} else {
				get_users_with_offline_messages->CommandText="sp_get_users_with_offline_messages";
				get_users_with_offline_messages->CommandType=ADODB::adCmdStoredProc;
				get_users_with_offline_messages->Parameters->Refresh();
			}
			if (!dbs->IsConferendoBS())
			{
				dprint4("vs_sip_get_provider_by_call_id\n");
				user_get_external_accounts.CreateInstance("ADODB.Command");
				user_get_external_accounts->ActiveConnection=db;
				user_get_external_accounts->CommandType=ADODB::adCmdText;
				user_get_external_accounts->Parameters->Refresh();
			}
			dprint4("vs_get_server_by_login\n");
			get_server_by_login.CreateInstance("ADODB.Command");
			get_server_by_login->ActiveConnection = db;
			get_server_by_login->CommandType = ADODB::adCmdText;

			dprint4("vs_get_server_by_call_id\n");
			get_server_by_call_id.CreateInstance("ADODB.Command");
			get_server_by_call_id->ActiveConnection = db;
			get_server_by_call_id->CommandText = "vs_get_server_by_call_id";
			get_server_by_call_id->CommandType = ADODB::adCmdStoredProc;
			get_server_by_call_id->Parameters->Refresh();

			if (IsPostgreSQL)
			{
				dprint4("sp_get_named_conf_info\n");
				get_named_conf_info_template = "select * from sp_get_named_conf_info('%conf_id%');";
				get_named_conf_info.CreateInstance("ADODB.Command");
				get_named_conf_info->ActiveConnection=db;
				//get_named_conf_info->CommandText="sp_get_named_conf_info";
				get_named_conf_info->CommandType=ADODB::adCmdText;
				//get_named_conf_info->Parameters->Refresh();

				dprint4("sp_set_named_conf_server\n");
				set_named_conf_server.CreateInstance("ADODB.Command");
				set_named_conf_server->ActiveConnection=db;
				set_named_conf_server->CommandText="sp_set_named_conf_server";
				set_named_conf_server->CommandType=ADODB::adCmdStoredProc;
				set_named_conf_server->Parameters->Refresh();

				dprint4("sp_get_named_conf_participants\n");
				get_named_conf_participants_template = "select * from sp_get_named_conf_participants('%conf_id%');";
				get_named_conf_participants.CreateInstance("ADODB.Command");
				get_named_conf_participants->ActiveConnection=db;
				//get_named_conf_participants->CommandText="sp_get_named_conf_participants";
				get_named_conf_participants->CommandType=ADODB::adCmdText;
				//get_named_conf_participants->Parameters->Refresh();

				dprint4("sp_init_named_conf_invitations\n");
				init_named_conf_invitations_template = "select * from sp_init_named_conf_invitations();";
				init_named_conf_invitations.CreateInstance("ADODB.Command");
				init_named_conf_invitations->ActiveConnection=db;
				//get_named_conf_participants->CommandText="sp_get_named_conf_participants";
				init_named_conf_invitations->CommandType=ADODB::adCmdText;
				//get_named_conf_participants->Parameters->Refresh();

				dprint4("vs_get_login_session_secret\n");
				get_login_session_secret.CreateInstance("ADODB.Command");
				get_login_session_secret->ActiveConnection=db;
				get_login_session_secret->CommandText="vs_get_login_session_secret";
				get_login_session_secret->CommandType=ADODB::adCmdStoredProc;
				get_login_session_secret->Parameters->Refresh();

				dprint4("sp_get_properties_list\n");
				get_properties_list_template = "select * from sp_get_properties_list('%call_id%', '', '%app_name%');";
				get_properties_list.CreateInstance("ADODB.Command");
				get_properties_list->ActiveConnection=db;
//				get_properties_list->CommandText="sp_get_properties_list";
				get_properties_list->CommandType=ADODB::adCmdText;
//				get_properties_list->Parameters->Refresh();

				dprint4("sp_get_aliases\n");
				get_aliases_template = "select * from sp_get_aliases('%call_id%');";
				get_aliases.CreateInstance("ADODB.Command");
				get_aliases->ActiveConnection=db;
				get_aliases->CommandText="sp_get_aliases";
				get_aliases->CommandType=ADODB::adCmdStoredProc;
				get_aliases->Parameters->Refresh();

				dprint4("sp_find_users\n");
				search_users.CreateInstance("ADODB.Command");
				search_users->ActiveConnection=db;
				search_users->CommandText="vs_find_users";
				search_users->CommandType=ADODB::adCmdStoredProc;
				search_users->Parameters->Refresh();

				if (!dbs->IsConferendoBS())
				{
					dprint4("sp_ab_group_get_list_users\n");
					ab_group_get_list_users_template = "select * from sp_ab_group_get_list_users('%call_id%');";
					ab_group_get_list_users.CreateInstance("ADODB.Command");
					ab_group_get_list_users->ActiveConnection=db;

					dprint4("sp_ab_group_create\n");
					ab_group_create.CreateInstance("ADODB.Command");
					ab_group_create->ActiveConnection=db;
					ab_group_create->CommandText="sp_ab_group_create";
					ab_group_create->CommandType=ADODB::adCmdStoredProc;
					ab_group_create->Parameters->Refresh();

					dprint4("sp_ab_group_delete\n");
					ab_group_delete_template = "select * from sp_ab_group_delete(%gid%);";
					ab_group_delete.CreateInstance("ADODB.Command");
					ab_group_delete->ActiveConnection=db;

					dprint4("sp_ab_group_edit\n");
					ab_group_edit_template = L"select * from sp_ab_group_edit(%gid%, '%gname%');";
					ab_group_edit.CreateInstance("ADODB.Command");
					ab_group_edit->ActiveConnection=db;

					dprint4("sp_ab_group_add_user\n");
					ab_group_add_user.CreateInstance("ADODB.Command");
					ab_group_add_user->ActiveConnection=db;
					ab_group_add_user->CommandText="sp_ab_group_add_user";
					ab_group_add_user->CommandType=ADODB::adCmdStoredProc;
					ab_group_add_user->Parameters->Refresh();

					dprint4("sp_ab_group_delete_user\n");
					ab_group_delete_user.CreateInstance("ADODB.Command");
					ab_group_delete_user->ActiveConnection=db;
					ab_group_delete_user->CommandText="sp_ab_group_delete_user";
					ab_group_delete_user->CommandType=ADODB::adCmdStoredProc;
					ab_group_delete_user->Parameters->Refresh();
				}

				dprint4("sp_bak_add_user\n");
				ab_external_add_user.CreateInstance("ADODB.Command");
				ab_external_add_user->ActiveConnection=db;
				ab_external_add_user->CommandText="sp_bak_add_user";
				ab_external_add_user->CommandType=ADODB::adCmdStoredProc;
				ab_external_add_user->Parameters->Refresh();

				dprint4("vs_bak_delete_user\n");
				ab_external_delete_user.CreateInstance("ADODB.Command");
				ab_external_delete_user->ActiveConnection=db;
				ab_external_delete_user->CommandText="vs_bak_delete_user";
				ab_external_delete_user->CommandType=ADODB::adCmdStoredProc;
				ab_external_delete_user->Parameters->Refresh();

				dprint4("sp_add_alias\n");
				sp_add_alias.CreateInstance("ADODB.Command");
				sp_add_alias->ActiveConnection=db;
				sp_add_alias->CommandText="sp_add_alias";
				sp_add_alias->CommandType=ADODB::adCmdStoredProc;
				sp_add_alias->Parameters->Refresh();

				dprint4("vs_delete_alias\n");
				vs_delete_alias.CreateInstance("ADODB.Command");
				vs_delete_alias->ActiveConnection=db;
				vs_delete_alias->CommandText="vs_delete_alias";
				vs_delete_alias->CommandType=ADODB::adCmdStoredProc;
				vs_delete_alias->Parameters->Refresh();

				dprint4("sp_change_password\n");
				change_password.CreateInstance("ADODB.Command");
				change_password->ActiveConnection=db;
				change_password->CommandText="sp_change_password";
				change_password->CommandType=ADODB::adCmdStoredProc;
				change_password->Parameters->Refresh();

				dprint4("sp_update_person\n");
				update_person.CreateInstance("ADODB.Command");
				update_person->ActiveConnection=db;
				update_person->CommandText="sp_update_person";
				update_person->CommandType=ADODB::adCmdStoredProc;
				update_person->Parameters->Refresh();

				if (!dbs->IsConferendoBS())
				{
					dprint4("vs_add_dial_number\n");
					ab_phones_add.CreateInstance("ADODB.Command");
					ab_phones_add->ActiveConnection=db;
					ab_phones_add->CommandText="vs_add_dial_number";
					ab_phones_add->CommandType=ADODB::adCmdStoredProc;
					ab_phones_add->Parameters->Refresh();

					dprint4("sp_update_dial_number\n");
					ab_phones_update.CreateInstance("ADODB.Command");
					ab_phones_update->ActiveConnection=db;
					ab_phones_update->CommandText="sp_update_dial_number";
					ab_phones_update->CommandType=ADODB::adCmdStoredProc;
					ab_phones_update->Parameters->Refresh();

					dprint4("sp_delete_dial_number\n");
					ab_phones_delete.CreateInstance("ADODB.Command");
					ab_phones_delete->ActiveConnection=db;
					ab_phones_delete->CommandText="sp_delete_dial_number";
					ab_phones_delete->CommandType=ADODB::adCmdStoredProc;
					ab_phones_delete->Parameters->Refresh();


					dprint4("sp_get_dial_numbers_list\n");
					ab_phones_find_template = "select * from sp_get_dial_numbers_list('%call_id%');";
					ab_phones_find.CreateInstance("ADODB.Command");
					ab_phones_find->ActiveConnection=db;
				}

				dprint4("sp_ntf_web_log_insert\n");
				sp_ntf_web_log_insert.CreateInstance("ADODB.Command");
				sp_ntf_web_log_insert->ActiveConnection=db;
				sp_ntf_web_log_insert->CommandText="sp_ntf_web_log_insert";
				sp_ntf_web_log_insert->CommandType=ADODB::adCmdStoredProc;
				sp_ntf_web_log_insert->Parameters->Refresh();

				dprint4("sp_ntf_update_reg_id\n");
				sp_ntf_update_reg_id.CreateInstance("ADODB.Command");
				sp_ntf_update_reg_id->ActiveConnection=db;
				sp_ntf_update_reg_id->CommandText="sp_ntf_update_reg_id";
				sp_ntf_update_reg_id->CommandType=ADODB::adCmdStoredProc;
				sp_ntf_update_reg_id->Parameters->Refresh();

				/**************************/

				dprint4("sp_ft_create_file_transfer\n");
				create_file_transfer.CreateInstance("ADODB.Command");
				create_file_transfer->ActiveConnection = db;
				create_file_transfer->CommandText = "sp_ft_create_file_transfer";
				create_file_transfer->CommandType = ADODB::adCmdStoredProc;
				create_file_transfer->Parameters->Refresh();

				dprint4("sp_ft_add_file_to_transfer\n");
				add_file_to_transfer.CreateInstance("ADODB.Command");
				add_file_to_transfer->ActiveConnection = db;
				add_file_to_transfer->CommandText = "sp_ft_add_file_to_transfer";
				add_file_to_transfer->CommandType = ADODB::adCmdStoredProc;
				add_file_to_transfer->Parameters->Refresh();

				dprint4("sp_ft_add_file_transfer_stat\n");
				add_file_transfer_stat.CreateInstance("ADODB.Command");
				add_file_transfer_stat->ActiveConnection = db;
				add_file_transfer_stat->CommandText = "sp_ft_add_file_transfer_stat";
				add_file_transfer_stat->CommandType = ADODB::adCmdStoredProc;
				add_file_transfer_stat->Parameters->Refresh();

				dprint4("sp_ft_add_file_transfer_files_stat\n");
				add_file_transfer_files_stat.CreateInstance("ADODB.Command");
				add_file_transfer_files_stat->ActiveConnection = db;
				add_file_transfer_files_stat->CommandText = "sp_ft_add_file_transfer_files_stat";
				add_file_transfer_files_stat->CommandType = ADODB::adCmdStoredProc;
				add_file_transfer_files_stat->Parameters->Refresh();

				dprint4("sp_ft_delete_file_transfer\n");
				delete_file_transfer.CreateInstance("ADODB.Command");
				delete_file_transfer->ActiveConnection = db;
				delete_file_transfer->CommandText = "sp_ft_delete_file_transfer";
				delete_file_transfer->CommandType = ADODB::adCmdStoredProc;
				delete_file_transfer->Parameters->Refresh();

				dprint4("sp_ft_delete_file_transfer_owner\n");
				delete_file_transfer_owner.CreateInstance("ADODB.Command");
				delete_file_transfer_owner->ActiveConnection = db;
				delete_file_transfer_owner->CommandText = "sp_ft_delete_file_transfer_owner";
				delete_file_transfer_owner->CommandType = ADODB::adCmdStoredProc;
				delete_file_transfer_owner->Parameters->Refresh();

				dprint4("sp_ft_set_file_transfer_delete_files\n");
				set_file_transfer_delete_files.CreateInstance("ADODB.Command");
				set_file_transfer_delete_files->ActiveConnection = db;
				set_file_transfer_delete_files->CommandText = "sp_ft_set_file_transfer_delete_files";
				set_file_transfer_delete_files->CommandType = ADODB::adCmdStoredProc;
				set_file_transfer_delete_files->Parameters->Refresh();

				dprint4("sp_ft_set_file_transfer_delete_files_by_days\n");
				set_file_transfer_delete_files_by_days.CreateInstance("ADODB.Command");
				set_file_transfer_delete_files_by_days->ActiveConnection = db;
				set_file_transfer_delete_files_by_days->CommandText = "sp_ft_set_file_transfer_delete_files_by_days";
				set_file_transfer_delete_files_by_days->CommandType = ADODB::adCmdStoredProc;
				set_file_transfer_delete_files_by_days->Parameters->Refresh();

				dprint4("sp_ft_get_file_transfer\n");
				get_file_transfer.CreateInstance("ADODB.Command");
				get_file_transfer->ActiveConnection = db;
				get_file_transfer->CommandText = "sp_ft_get_file_transfer";
				get_file_transfer->CommandType = ADODB::adCmdStoredProc;
				get_file_transfer->Parameters->Refresh();

				dprint4("sp_ft_set_file_transfer_owner_cnt\n");
				set_file_transfer_owner_cnt.CreateInstance("ADODB.Command");
				set_file_transfer_owner_cnt->ActiveConnection = db;
				set_file_transfer_owner_cnt->CommandText = "sp_ft_set_file_transfer_owner_cnt";
				set_file_transfer_owner_cnt->CommandType = ADODB::adCmdStoredProc;
				set_file_transfer_owner_cnt->Parameters->Refresh();
			}

			if (!VS_DBObjects_CallLog::Init(conn1, username1, password1))
				return false;

printf(". ");
		}
		catch(_com_error e)
		{
			ProcessCOMError(e);
			return false;
		}


		error_code=0;
		return true;
/////////////////////////////

};

VS_DBStorage::VS_DBStorage()
		:state(STATE_CREATED),
		m_dbo_pool(0)
  {

  };

  bool VS_DBStorage::Init(const VS_SimpleStr& broker_id)
  {
AUTO_PROF
	// CleanUp
	//delete m_dbo_pool;
	{
		VS_AutoLock lock(&m_usersWithOfflineMessages_lock);
		m_usersWithOfflineMessages.Clear();
	}
	// ----

	  m_serverID=broker_id;

	  // call base
	  VS_RegistryKey    cfg(false, CONFIGURATION_KEY, false, true);
	  if (!cfg.IsValid()) {
		  error_code=VSS_REGISTRY_ERROR;
		  return false;
	  };

	int connections;

	if (cfg.GetValue(&connections, sizeof(int), VS_REG_INTEGER_VT,DB_CONNECTIONS_TAG ) != 0)
	{
		if(connections>DB_CONNECTIONS_MAX)
			 connections=DB_CONNECTIONS_MAX;
		if(connections<DB_CONNECTIONS_MIN)
			 connections=DB_CONNECTIONS_MIN;
	}
	else
		connections=DB_CONNECTIONS_INIT;

	  fflush(stdout);

	  error_code=VSS_DB_ERROR;

	  m_dbo_pool=std::make_shared<VS_SharedPool>(std::make_unique<VS_DBOFactory>(this), connections, true);

	  if(m_dbo_pool->m_error)
	  {
			error_code=VSS_DB_ERROR;
			return false;
	  };

	//////////////////////////////////////////////////////////
	// sucessful end of init
	  error_code = 0;
	  state = STATE_RUNNING;


	  const VS_Pool::Item* dboitem;
	  VS_DBObjects* dbo = GetDBO(dboitem);
	  if (!dbo || dbo->error_code != 0)
	  {
		  error_code = dbo ? dbo->error_code : VSS_DB_ERROR;
		  return false;
	  };
	  ReleaseDBO(dboitem);

	  InitUsersWithOfflineMessages();
	  this->GetServerProperty("string_separator", STRING_SEPARATOR);
	  UpdateListAllowedExtStatuses();

	  printf("*");
	  return true;
  };

VS_DBStorage::~VS_DBStorage()
{
}



void VS_DBStorage::InitUsersWithOfflineMessages()
{
AUTO_PROF
	const VS_Pool::Item* dboitem;
	VS_DBObjects* dbo=GetDBO(dboitem);

	try
	{
		dprint4("$InitUsersWithOfflineMessages\n");

		ADODB::_RecordsetPtr rs = dbo->get_users_with_offline_messages->Execute(NULL,NULL,ADODB::adCmdUnspecified);

		VS_SimpleStr	user;

		while( !rs->ADOEOF )
		{
			ADODB::FieldsPtr f = rs->Fields;

			user = vs::StrFromVariantT(f->Item[CALL_ID_TO]->Value);

			{
				VS_AutoLock lock(&m_usersWithOfflineMessages_lock);
				m_usersWithOfflineMessages.Insert(user, user);
			}

			rs->MoveNext();
		}
	}
	catch(_com_error e)
	{
		_bstr_t	add_str(dbo->get_users_with_offline_messages->GetCommandText());
		dbo->ProcessCOMError(e,dbo->get_users_with_offline_messages->ActiveConnection,this,add_str);
	}

	ReleaseDBO(dboitem);
}

////////////////////////////////////////////////////
// User management

void VS_DBStorage::FetchUser(ADODB::FieldsPtr f,VS_UserData& user)
{
	user.m_name = vs::StrFromVariantT(f->Item[CALL_ID]->Value);
	auto dn = vs::WStrFromVariantT(f->Item[DB_DISPLAYNAME]->Value);
	if (dn) user.m_displayName = vs::UTF16toUTF8Convert(dn.m_str);
	user.m_type			=f->Item[DB_TYPE]->Value;
	user.m_rights		=f->Item[DB_RIGHTS]->Value;

	dstream3 << " found. id:" << user.m_name.m_str << " name: '" << user.m_displayName << "' t=" << user.m_type << " r=" << std::ios::hex << user.m_rights<<"\n";
}

bool VS_DBStorage::FindUser(const vs_user_id& id,VS_UserData& user,bool find_by_call_id_only)
  {
AUTO_PROF
	dprint3("$FindUser with id %s \n",(const char*)id);

	if(id==0)
	{
		error_code=VSS_USER_NOT_FOUND;
		dprint1("$FindUser failed: zero id.\n");
		return false;
	};

	const VS_Pool::Item* dboitem;
	VS_DBObjects* dbo=GetDBO(dboitem);
	bool result=false;

	try
	{
		std::string cmd(dbo->user_find_template);
		VS_ReplaceAll(cmd, "%call_id%", (const char*)id);
		VS_ReplaceAll(cmd, "%search_type%", !find_by_call_id_only ? "alias" : "");
		dbo->user_find->CommandText = cmd.c_str();

		ADODB::_RecordsetPtr rs=dbo->user_find->Execute(0,0,ADODB::adCmdText);

		if(rs->ADOEOF)
		{
			dprint2("$FindUser failed: id (%s) not found\n",( const char*)id);
			throw VSS_USER_NOT_FOUND;
		}

		FetchUser(rs->Fields,user);
		result=true;

	}
	catch(_com_error e)
	{
		_bstr_t	add_str(dbo->user_find->GetCommandText());
		dbo->ProcessCOMError(e,dbo->user_find->ActiveConnection,this,add_str);
	}
	catch(int e)
	{
		error_code=e;
	}

	ReleaseDBO(dboitem);
	return result;
};
void VS_DBStorage::UpdateListAllowedExtStatuses()
{
	const VS_Pool::Item* dboitem;
	VS_DBObjects* dbo=GetDBO(dboitem);
	decltype(m_allowed_ext_statuses) tmp_set;
	try
	{
		dprint4("$UpdateListAllowedExtStatuses\n");
		ADODB::_RecordsetPtr rs = dbo->ext_statuses_get_allowed->Execute(NULL, NULL, ADODB::adCmdUnspecified);
		for(;!rs->ADOEOF;rs->MoveNext())
		{
			ADODB::FieldsPtr f= rs->Fields;
			ADODB::FieldPtr p = f->GetItem(short(0));
			_bstr_t status_name(p->GetValue());
			tmp_set.insert(static_cast<const char*>(status_name));
			dstream4 << "status_name = " << static_cast<const char*>(status_name) << ";\n";
		}
	}
	catch(const _com_error &e)
	{
		_bstr_t	add_str(dbo->ext_statuses_get_allowed->GetCommandText());
		dbo->ProcessCOMError(e,dbo->ext_statuses_get_allowed->ActiveConnection,this,add_str);
		tmp_set.clear();
	}
	ReleaseDBO(dboitem);
	VS_ExtendedStatusStorage::SetStickyStatusesNames(tmp_set);
	VS_AutoLock lock(&m_allowed_ext_statuses_lock);
	m_allowed_ext_statuses = std::move(tmp_set);
}
void VS_DBStorage::SetUserStatus(const VS_SimpleStr& call_id,int status, const VS_ExtendedStatusStorage &extStatus, bool set_server, const VS_SimpleStr& server)
{
AUTO_PROF
  const VS_Pool::Item* dboitem;
  VS_DBObjects* dbo=GetDBO(dboitem);
	try
	{
		dprint3("$SetUserStatus for %s at %s ,status %d\n",(const char*)call_id, (const char*)server, status);

		ADODB::ParametersPtr p=dbo->user_set_status->Parameters;

		p->Item[CALL_ID_PARAM]->Value		= call_id.m_str;
		p->Item[USER_STATUS_PARAM]->Value	= status;
		if(set_server)
			p->Item[DB_SERVER_PARAM]->Value		= server.m_str;
		else
			p->Item[DB_SERVER_PARAM]->Value		= db_null;

		dbo->user_set_status->Execute(NULL,NULL,ADODB::adCmdStoredProc|ADODB::adExecuteNoRecords);
	}
	catch(_com_error e)
	{
		_bstr_t	add_str(dbo->user_set_status->GetCommandText());
		dbo->ProcessCOMError(e,dbo->user_set_status->ActiveConnection,this,add_str);
	};
	SetExtStatus(dbo,call_id,extStatus);
	ReleaseDBO(dboitem);
}
void VS_DBStorage::SetExtStatus(VS_DBObjects *dbo, const char* call_id, const VS_ExtendedStatusStorage &extStatus)
{
	AUTO_PROF
	if (!dbo)
		return;
	auto changed_st = extStatus.GetChangedStatuses();
	auto extStMap = std::move(changed_st.first);
	auto reset = changed_st.second;
	auto deletedStatuses = extStatus.GetDeletedStatuses();
	if(extStMap.empty() && deletedStatuses.empty())
		return;
	decltype(m_allowed_ext_statuses) allowed_ext_st;
	{
		VS_AutoLock lock(&m_allowed_ext_statuses_lock);
		allowed_ext_st = m_allowed_ext_statuses;
	}
	dprint3("$SetUserExtStatus for %s. Count updated ext statuses = %zu. %s\n", call_id, extStMap.size(), reset ? "Reset ext statuses" : "");
	if (reset)
	{
		for (const auto &i : allowed_ext_st)
		{
			if (!extStatus.IsStatusExist(i))
				deletedStatuses.insert(i);
		}
	}
	for(const auto &i : extStMap)
	{
		try
		{
			const auto &ext_st_descr = allowed_ext_st.find(i.first);
			if(ext_st_descr==allowed_ext_st.end())
			{
				dprint3(" ext status = %s is not allowed. Skip\n",i.first.c_str());
				continue;
			}
			std::pair<bool,std::string> status_val_in_str = boost::apply_visitor(status_value_to_string(), i.second);
			assert(status_val_in_str.first);
			if (!status_val_in_str.first)
				continue;

			ADODB::ParametersPtr p = dbo->user_set_ext_status->Parameters;
			p->Item[CALL_ID_PARAM_SP]->Value = call_id;
			p->Item[NAME_PARAM_SP]->Value = i.first.c_str();
			p->Item[VALUE_PARAM_SP]->Value = status_val_in_str.second.c_str();
			dbo->user_set_ext_status->Execute(NULL, NULL, ADODB::adCmdStoredProc | ADODB::adExecuteNoRecords);
		}
		catch(const _com_error &e)
		{
			_bstr_t	add_str(dbo->user_set_status->GetCommandText());
			dbo->ProcessCOMError(e,dbo->user_set_status->ActiveConnection,this,add_str);
		}
	}
	dprint3("$SetUserExtStatus for %s. Count deleted ext statuses = %zu \n", call_id, deletedStatuses.size());
	for (const auto &i : deletedStatuses)
	{
		try
		{
			const auto &ext_st_descr = allowed_ext_st.find(i);
			if (ext_st_descr == allowed_ext_st.end())
			{
				dprint3(" ext status = %s is not allowed. Skip\n", i.c_str());
				continue;
			}
			ADODB::ParametersPtr p = dbo->user_set_ext_status->Parameters;
			p->Item[CALL_ID_PARAM_SP]->Value = call_id;
			p->Item[NAME_PARAM_SP]->Value = i.c_str();
			p->Item[VALUE_PARAM_SP]->Value = NULL;
			dbo->user_set_ext_status->Execute(NULL, NULL, ADODB::adCmdStoredProc | ADODB::adExecuteNoRecords);
		}
		catch (const _com_error &e)
		{
			_bstr_t	add_str(dbo->user_set_status->GetCommandText());
			dbo->ProcessCOMError(e, dbo->user_set_status->ActiveConnection, this, add_str);
		}
	}
}
bool VS_DBStorage::GetExtendedStatus(VS_DBObjects *dbo, const char *call_id, VS_ExtendedStatusStorage &ext_st)
{
	try
	{
		auto params = dbo->user_get_ext_status->Parameters;
		params->Item[CALL_ID_PARAM_SP]->Value = call_id;
		dprint3("$GetExtendedStatus for call_id = %s\n",call_id);
		auto rs = dbo->user_get_ext_status->Execute(nullptr, nullptr, ADODB::adCmdStoredProc);
		if (rs == 0 || rs->State == ADODB::adStateClosed || rs->ADOEOF)
		{
			dprint3("ext status not found\n");
			return false;
		}
		for (; !rs->ADOEOF; rs->MoveNext())
		{
			VS_Container status;
			auto fields = rs->Fields;
			auto status_name = fields->Item[USER_EXT_STATUS_NAME_PARAM]->Value;
			if (status_name.vt == VT_EMPTY || status_name.vt == VT_NULL)
				continue;
			std::string st_name = static_cast<const char*>(static_cast<const _bstr_t>(status_name));
			auto ss = dstream4;
			ss << "Extended status name = " << st_name.c_str() << "; value = ";
			variant_t val;
			if ((val = fields->Item[INTVAL_PARAM]->GetValue()).vt != VT_NULL)
			{
				auto int_val = static_cast<int32_t>(val);
				ss << int_val;
				ext_st.UpdateStatus(st_name.c_str(), int_val);
			}
			else if ((val = fields->Item[STRVAL_PARAM]->GetValue()).vt != VT_NULL)
			{
				std::string str_val = static_cast<const char*>(static_cast<const bstr_t>(val));
				ss << str_val;
				ext_st.UpdateStatus(st_name.c_str(), std::move(str_val));
			}
			else if ((val = fields->Item[TIMESTAMPVAL_PARAM]->GetValue()).vt != VT_NULL)
			{
				VS_FileTime ft;
				ft.FromVariant(val, false);
				auto t_point = ft.chrono_system_clock_time_point();
				int64_t secs_since_epoch = std::chrono::duration_cast<std::chrono::seconds>(t_point.time_since_epoch()).count();
				ext_st.UpdateStatus(st_name.c_str(), secs_since_epoch);
				auto t = std::chrono::system_clock::to_time_t(t_point);
				struct tm tm_buf;
				char buf[100] = { 0 };
				if (!gmtime_s(&tm_buf, &t))
					std::strftime(buf, 100, "%Y-%m-%d %H:%M:%S", &tm_buf);
				else
					strcpy(buf, "<time conversion error>");
				ss << buf;
			}
			else
			{
				ext_st.DeleteStatus(st_name);
				ss << "<null>, delete status";
			}
			ss << "\n";
		}
	}
	catch (const _com_error &e)
	{
		auto add_str = dbo->user_get_ext_status->GetCommandText();
		dbo->ProcessCOMError(e, dbo->user_get_ext_status->ActiveConnection, this, add_str);
		return false;
	}
	return true;
}
bool VS_DBStorage::GetExtendedStatus(const char *call_id, VS_ExtendedStatusStorage &ext_status)
{
AUTO_PROF
	if (!call_id || !*call_id)
	{
		dprint2("$GetExtendedStatus call_id is empty! Skip processing.\n");
		return false;
	}
	const VS_Pool::Item* dboitem;
	const auto dbo = GetDBO(dboitem);
	VS_SCOPE_EXIT{
		ReleaseDBO(dboitem);
	};
	dprint3("$GetExtendedStatus for %s\n", call_id);
	return GetExtendedStatus(dbo, call_id, ext_status);
}

void VS_DBStorage::ClearStatuses(const VS_SimpleStr& server)
{
AUTO_PROF
	const VS_Pool::Item* dboitem;
	VS_DBObjects* dbo=GetDBO(dboitem);

	try
	{
		dprint3("$ClearStatuses for %s\n", server.m_str);

		dbo->user_clear_statuses->Parameters->Item[DB_SERVER_PARAM]->Value		= server.m_str;

		dbo->user_clear_statuses->Execute(NULL,NULL,ADODB::adCmdStoredProc|ADODB::adExecuteNoRecords);
	}
	catch(const _com_error &e)
	{
		_bstr_t	add_str(dbo->user_clear_statuses->GetCommandText());
		dbo->ProcessCOMError(e,dbo->user_clear_statuses->ActiveConnection, this,add_str);
	};

	ReleaseDBO(dboitem);
}

void VS_DBStorage::CleanUp()
{
AUTO_PROF
	const VS_Pool::Item* dboitem;
	VS_DBObjects* dbo=GetDBO(dboitem);

	try
	{
		dprint3("$CleanUp\n");

		dbo->cleanup->Execute(NULL,NULL,ADODB::adCmdStoredProc|ADODB::adExecuteNoRecords);
	}
	catch(_com_error e)
	{
		_bstr_t	add_str(dbo->cleanup->GetCommandText());
		dbo->ProcessCOMError(e,dbo->cleanup->ActiveConnection,this,add_str);
	};

	ReleaseDBO(dboitem);
}

int VS_DBStorage::LoginUser(const VS_SimpleStr& login,const VS_SimpleStr& password, const VS_SimpleStr& appID, VS_SimpleStr& key, const VS_SimpleStr& appServer, VS_UserData& user, VS_Container& prop_cnt, const VS_ClientType& client_type)
{
AUTO_PROF
	const VS_Pool::Item* dboitem;
	VS_DBObjects* dbo=GetDBO(dboitem);
    int result=-1;

	try
	{
		dprint2("$LoginUser '%s' app:%s srv:%s\n", login.m_str, appID.m_str, appServer.m_str);

		//WaitForCommand(user_login);
		ADODB::ParametersPtr p=dbo->user_login->Parameters;
		VS_WideStr	w_login;
		w_login.AssignUTF8(login);

		p->Item[DB_LOGIN_PARAM]->Value			= w_login.m_str;
		p->Item[DB_PASSWORD_PARAM]->Value		= password.m_str;
		p->Item[APP_ID_PARAM]->Value			= appID.m_str;
		p->Item[DB_AUTO_KEY_PARAM]->Value		= key.m_str;
		p->Item[DB_SERVER_PARAM]->Value			= appServer.m_str;
		try{
			p->Item[DB_CLIENT_TYPE_PARAM]->Value	=(long)client_type;
		}
		catch(...){
			dprint2("$LoginUser: set 'client_type' param error\n");
		}

			//last_logged_in updated in DB


		ADODB::_RecordsetPtr rs=dbo->user_login->Execute(0,0,ADODB::adCmdStoredProc);


		if(rs==0 || rs->State==ADODB::adStateClosed || rs->ADOEOF)
		{
			dprint2(" not found\n");
			throw ACCESS_DENIED;
		}

		ADODB::FieldsPtr f=rs->Fields;

		VS_UserLoggedin_Result res(USER_LOGGEDIN_OK);
		try{
			res = (VS_UserLoggedin_Result)(int)f->Item[DB_RESULT]->Value;
		}
		catch(...){
			dprint2("$LoginUser: get 'result' param error\n");
		}
		if (res!=USER_LOGGEDIN_OK)
		{
			dprint2("$LoginUser: result=%d\n", res);
			throw res;
		}

		_variant_t id=f->Item[CALL_ID]->Value;
		vs_user_id xid = vs::StrFromVariantT(id);
		if(id.vt==VT_EMPTY || id.vt==VT_NULL || xid.IsEmpty())
		{
			dprint2("$LoginUser: access denied\n");
			throw ACCESS_DENIED;
		}

		int status=f->Item[DB_STATUS]->Value;
		if(!(status&user.US_LOGIN))
		{
			dprint2("$LoginUser: user is disabled\n");
			throw USER_DISABLED;
		}

		key = vs::StrFromVariantT(f->Item[DB_AUTO_KEY]->Value);

		user.m_rating = (long) f->Item["sys_rating"]->Value;
		// todo: add user.m_SeparationGroup here

		FetchUser(f,user);
		user.m_tarif_name = vs::StrFromVariantT(f->Item["trf_name"]->Value);
		user.m_tarif_restrictions = f->Item["conf_rights"]->Value;

		GetPropertiesList(user.m_name, user.m_appName, prop_cnt);
//		this->GetAllUserProps("", prop_cnt);

		if (dbo->IsPostgreSQL) {
			std::string cmd(dbo->get_aliases_template);
			VS_ReplaceAll(cmd, "%call_id%", user.m_name.m_str);
			dbo->get_aliases->CommandText = cmd.c_str();

			rs=dbo->get_aliases->Execute(0,0,ADODB::adCmdText);
		} else {
			_variant_t r((long)0);
			rs=rs->NextRecordset(&r);
		}

		auto ss = dstream3;
		ss << "aliases: ";
		if(rs!=0 && rs->State!=ADODB::adStateClosed )
		{
			for(;!rs->ADOEOF;rs->MoveNext())
			{
				user.m_aliases.Insert((char const*)(_bstr_t)rs->Fields->Item[(short)0]->Value,0);
				ss << (char const*)(_bstr_t)rs->Fields->Item[(short)0]->Value << " ";
			};
		}
		ss << '\n';

		GetSipProviderByCallId(user.m_name.m_str, user.m_external_accounts, dbo);

		result=0;

		}
		catch(VS_UserLoggedin_Result err)
		{
			result=err;
		}
		catch(_com_error err)
		{
			_bstr_t	add_str(dbo->user_login->GetCommandText());
			dbo->ProcessCOMError(err,dbo->user_login->ActiveConnection,this,add_str);
  			result = SILENT_REJECT_LOGIN;
		}

		ReleaseDBO(dboitem);
		return result;
  };

  int VS_DBStorage::FindUsers(VS_Container& cnt, int& entries, VS_AddressBook ab, const vs_user_id& owner, const std::string& query, long client_hash, VS_Container& in_cnt)
  {
	  dstream2 << "$FindUsers in ab=" << ab <<" Q=" << query << ", hash=" << client_hash << "\n";

	  const VS_Pool::Item* dboitem;
	  VS_DBObjects* dbo=GetDBO(dboitem);
	  VS_SCOPE_EXIT{ ReleaseDBO(dboitem); };
	  int result=SEARCH_FAILED;
	  entries=0;

	  if (dbo->IsPostgreSQL && ab==AB_PERSONS)
	  {
		  entries = SearchUsers(cnt, in_cnt, owner);
		  return SEARCH_DONE;
	  }

	  bool add_local = true;

	  long hash=0;
	  ADODB::_CommandPtr cmd;
	  try
	  {
		  int max=5000;

		  _bstr_t key_field("UserName");
		  ADODB::_ConnectionPtr	cur_db(dbo->db);

		  cmd.CreateInstance(__uuidof(ADODB::Command));
		  switch(ab)
		  {
		  case AB_PERSONS:
			  cmd->CommandText="sp_find_users";
			  break;
		  case AB_COMMON:
			  cmd->CommandText="sp_get_address_book";
			  break;
		  case AB_BAN_LIST:
			  cmd->CommandText="sp_get_ban_list";
			  break;
		  case AB_RECEIVED_CALLS:
			  cmd->CommandText="sp_get_received_calls";
			  cur_db=dbo->db1;
			  add_local = false;
			  break;
		  case AB_MISSED_CALLS:
			  cmd->CommandText="sp_get_missed_calls";
			  cur_db=dbo->db1;
			  add_local = false;
			  break;
		  case AB_PLACED_CALLS:
			  cmd->CommandText="sp_get_placed_calls";
			  cur_db=dbo->db1;
			  add_local = false;
			  break;
		  case AB_PERSON_DETAILS:
			  cmd->CommandText="sp_get_person_details";
			  break;
		  case AB_PERSON_DETAILS_EXTRA:
			  cmd->CommandText="sp_get_user_details";
			  break;
		  case AB_USER_PICTURE:
			  cmd->CommandText="sp_get_user_picture";
			  break;
		  case AB_CONF_LIST:
			  key_field="CallId";
			  cmd->CommandText="sp_get_user_conf_list";
			  break;
		  case AB_SP_GROUPS_:
			  key_field="CallId";
			  cmd->CommandText="sp_get_user_groups";
			  break;
		  case AB_DIRECTORY:
			  max=250000;
			  key_field="CallId";
			  cmd->CommandText="sp_get_directory";
			  break;
		  case AB_INVERSE:
			  key_field="CallId";
			  cmd->CommandText="sp_get_ab_inverse_address_book";
			  break;
		  default:
			  return 0;
		  };

		  // bug#38925c14
		  VS_SCOPE_EXIT{
			  if (ab == AB_USER_PICTURE && cnt.GetStrValueRef(USERNAME_PARAM) == nullptr)
				cnt.AddValue(USERNAME_PARAM, query);
		  };

		  // profiling
		  VS_AutoCountTime ___autoCountPerf(cmd->CommandText);

		  //cmd->Parameters->Refresh();
		  cmd->CommandType=ADODB::adCmdStoredProc;

		  cmd->ActiveConnection=cur_db;
		  ADODB::ParametersPtr p=cmd->Parameters;


		  p->Append(
			  cmd->CreateParameter(CALL_ID_PARAM,ADODB::adVarChar,ADODB::adParamInput,256)
			  );
		  p->Append(
			  cmd->CreateParameter(BOOK_PARAM,ADODB::adInteger,ADODB::adParamInput,4)
			  );
		  p->Append(
			  cmd->CreateParameter(DB_QUERY_PARAM,ADODB::adVarWChar,ADODB::adParamInput,256)
			  );

		  p->Item[CALL_ID_PARAM]->Value = owner.m_str;
		  p->Item[BOOK_PARAM]->Value					=(int)ab;

		  VS_WideStr wquery; wquery.AssignUTF8(query.c_str());
		  p->Item[DB_QUERY_PARAM]->Value = wquery.m_str;

		  ADODB::_RecordsetPtr rs = cmd->Execute(NULL,NULL,ADODB::adCmdStoredProc);

		  static const _bstr_t hname("hash");

		  ADODB::FieldPtr hfield;
		  bool use_hash=false;
		  try {
			  hfield = rs->Fields->GetItem((dbo->IsPostgreSQL)? "hash": (short)0);
			  use_hash=true;
		  }catch(_com_error e){};

		  if (use_hash)
		  {
			  if(hfield->Name==hname)
			  {
				  if(!rs->ADOEOF)
				  {
					  _variant_t val(hfield->Value);

					  if(val.vt!=VT_NULL && val.vt!=VT_EMPTY)
						  hash=val;

					  if(VS_CompareHash(client_hash,hash))
					  {
						  entries=-1;
						  throw SEARCH_NOT_MODIFIED;
					  };
				  };

				  if (!dbo->IsPostgreSQL)
				  {
					  _variant_t r((long)0);
					  rs=rs->NextRecordset(&r);

					  if (rs==0)
					  {
						  entries=0;
						  throw SEARCH_DONE;
					  }
				  }else{
					  if (hash==0 && dbo->IsPostgreSQL) {		// Set HASH = 2000 year
						  cnt.AddValueI32(HASH_PARAM, 1);
						  entries=0;
						  throw SEARCH_DONE;
					  }
				  }
			  }

			  if(hash!=0)
				  cnt.AddValueI32(HASH_PARAM, hash);
		  }

		  __int64 count=rs->State?rs->RecordCount:0;
		  if(count==0)
		  {
			  if(rs->State)
				  rs->Close();
			  throw VSS_USER_NOT_FOUND;
		  }
		  if(count==-1)
		  {
			  count=max;
		  }

		  ADODB::FieldsPtr f=rs->Fields;
		  ADODB::FieldPtr fld;
		  _bstr_t			name;
		  const char* cname;
		  ADODB::DataTypeEnum ftype;
		  int fcount;
		  long long_val;
		  double	double_val;
		  void HUGEP* bin_val;

		  SYSTEMTIME stime;
		  FILETIME ftime1,ftime2;

		  VS_SCOPE_EXIT{ rs->Close(); };
		  int i=0;
		  for(;i<max && !rs->ADOEOF;i++,rs->MoveNext())
		  {
			  auto key_val = f->Item[key_field]->Value;
			  if (key_val == db_null)
				  throw SEARCH_DONE;
			  cnt.AddValue(static_cast<const char*>(key_field), (const wchar_t *)(_bstr_t)key_val);
			  ADODB::FieldsPtr f= rs->Fields;

			  fcount=f->Count;

			  for(short j=0;j<fcount;j++)
			  {
				  fld= f->GetItem(j);
				  name=fld->Name;
				  cname=(const char*)name;
				  ftype=fld->Type;
				  _variant_t val(fld->Value);

				  if(!_wcsicmp(name, key_field) || (name==hname))
				  {
					  //dprint4("\t skipped key field\n");
				  }
				  else if(val.vt!=VT_NULL && val.vt!=VT_EMPTY)
				  {
					  //dprint4("\t adding field %s='%s' type(%d)\n",(char*)name,(char*)(_bstr_t)val,val.vt);
					  switch(ftype)
					  {
						  //VT_DATE
					  case ADODB::adDate:
					  case ADODB::adFileTime:
					  case ADODB::adDBTimeStamp:
					  case ADODB::adDBDate:
					  case ADODB::adDBTime:
						  {
							  VariantTimeToSystemTime(val.dblVal, &stime);
							  if (add_local)
							  {
								  SystemTimeToFileTime(&stime,&ftime1);
								  LocalFileTimeToFileTime(&ftime1,&ftime2);
								  FileTimeToSystemTime(&ftime2,&stime);
							  }
							  cnt.AddValue(cname,&stime,sizeof(stime));
						  }
						  break;

					  case ADODB::adDecimal:
					  case ADODB::adNumeric:
						  {
							  double_val=val;
							  cnt.AddValue(cname,double_val);
						  }
						  break;

						  // VT_UI1 VT_UI2 VT_UI4
						  // VT_INT VT_I2 VT_I4
					  case ADODB::adBigInt:
					  case ADODB::adTinyInt:
					  case ADODB::adSmallInt:
					  case ADODB::adInteger:
					  case ADODB::adUnsignedTinyInt:
					  case ADODB::adUnsignedSmallInt:
					  case ADODB::adUnsignedInt:
						  {
							  long_val=val;
							  cnt.AddValueI32(cname, long_val);
						  }
						  break;

						  //VT_EMPTY
					  case ADODB::adEmpty:
						  break; //nothing

						  //VT_ARRAY|VT_UI1:
					  case ADODB::adArray|ADODB::adTinyInt:
					  case ADODB::adArray|ADODB::adUnsignedTinyInt:
					  case ADODB::adVarBinary:
					  case ADODB::adLongVarBinary:
						  {
							  SAFEARRAY* sa=val.parray;
							  if (FAILED(SafeArrayAccessData(sa,&bin_val)))
								  throw VSS_DB_ERROR;
							  cnt.AddValue(cname, bin_val, sa->rgsabound[0].cElements);
							  SafeArrayUnaccessData(sa);
							  bin_val=0;
						  }
						  break;
						  //case VT_BSTR:
					  case ADODB::adWChar:
					  case ADODB::adVarWChar:
					  case ADODB::adVarChar:
					  case ADODB::adChar:
					  case ADODB::adLongVarChar:
					  case ADODB::adLongVarWChar:
						  cnt.AddValue(cname,(wchar_t*)(_bstr_t)fld->Value);
						  break;
					  default:
						  dprint3("unknown type: %x\n",ftype);
					  };
				  }; // if not null value
			  };//for all fields

		  }; //for all records

		  entries = i;
		  dprint2(" found %d = %lld\n", i, count);
		  result= SEARCH_DONE;
	  }
	  catch(_com_error e)
	  {
		  _bstr_t	add_str(cmd->GetCommandText());
		  dbo->ProcessCOMError(e,cmd->ActiveConnection, this,add_str);
	  }
	  catch(int e)
	  {
		  error_code=e;
	  }
	  catch(VS_Search_Result res)
	  {
		  result=res;
	  };

	  return result;
  };

//user presence

int VS_DBStorage::AddToAddressBook(VS_AddressBook ab,const vs_user_id& user_id1, VS_Container& cnt, long& hash, VS_Container& rCnt, VS_SimpleStr& add_call_id, std::string& add_display_name, VS_TransportRouterServiceHelper* srv)
{
//	AUTO_PROF see below
	const VS_Pool::Item* dboitem;
	VS_DBObjects* dbo=GetDBO(dboitem);
	int error=-1;
	int	error_code_out=-1;

	hash=0;
	ADODB::_CommandPtr cmd;
	ADODB::ParametersPtr p;

	dprint2("$AddToAddressBook %d of %s\n", ab,(const char*)user_id1);
	const char* call_id1_param = CALL_ID_PARAM;
	const char* call_id2_param = CALL_ID2_PARAM;
	bool is_displayname = true;
	bool check_error(true);

	try {
		switch(ab)
		{
		case AB_COMMON:
			cmd=dbo->ab_add_user;
			break;
		case AB_BAN_LIST:
			cmd=dbo->ban_user;
			p=cmd->Parameters;
			if (IsConferendoBS() && !!cnt.GetLongValueRef(TYPE_PARAM))		// #13417: abuse flag
			{
				dprint3("abuse=%d\n",*cnt.GetLongValueRef(TYPE_PARAM));
				p->Item[DB_TYPE_PARAM]->Value=*(cnt.GetLongValueRef(TYPE_PARAM));
			}
			break;
		case AB_EXTERNAL_CONTACTS:
			cmd=dbo->ab_external_add_user;
			call_id1_param = "@call_id";
			call_id2_param = "@alias2";
			check_error = false;
			error = 0;
			break;
		case AB_MY_EXTERNAL_CONTACTS:
			cmd=dbo->sp_add_alias;
			call_id1_param = "c_call_id";
			call_id2_param = "c_alias";
			is_displayname = false;
			//cmd->Parameters->Item["i_alias_type"]->Value = (int)0;
			check_error = false;
			error = 0;
			break;
		default:
			ReleaseDBO(dboitem);
			return false;
		};

		// profiling
		VS_AutoCountTime ___autoCountPerf(cmd->CommandText);

		p=cmd->Parameters;
		p->Item[call_id1_param]->Value				= user_id1.m_str;
		p->Item[call_id2_param]->Value				= db_null;
		if (is_displayname)
			p->Item[DB_DISPLAYNAME_PARAM]->Value	= db_null;

		VS_SimpleStr call_id2;
		VS_WideStr dn;
		std::map<VS_SimpleStr, VS_WideStr> m;
		cnt.Reset();
		while (cnt.Next())
		{
			const char * Name = cnt.GetName();
			if (!_stricmp(Name, NAME_PARAM) || !_stricmp(Name, CALLID_PARAM)) {
				if (!!call_id2)
				{
					m.emplace(call_id2, dn);
				}
				call_id2 = cnt.GetStrValueRef();
			} else if (_stricmp(Name, DISPLAYNAME_PARAM) == 0) {
				dn.AssignUTF8(cnt.GetStrValueRef());
			}
		}
		if (!!call_id2)
			m.emplace(call_id2, dn);

		// process vice versa to make add_display_name for 1st user (for backward compability with TrueConf Client)
		for(std::map<VS_SimpleStr, VS_WideStr>::reverse_iterator it=m.rbegin(); it!=m.rend(); ++it)
		{
			VS_WideStr wstr; wstr.AssignUTF8(it->first);
			p->Item[call_id2_param]->Value = wstr.m_str;
			if (is_displayname)
			{
				if (it->second && !!it->second)
					p->Item[DB_DISPLAYNAME_PARAM]->Value = it->second.m_str;
				else
					p->Item[DB_DISPLAYNAME_PARAM]->Value = db_null;
			}

			dstream3 << "AddToAB(" << ab << "," << user_id1.m_str << "), id2=" << wstr.m_str << ", dn2=" << ((is_displayname)? it->second.m_str: L"no_dn") << "\n";
			ADODB::_RecordsetPtr rs;
			try {
				rs=cmd->Execute(NULL,NULL,ADODB::adCmdUnspecified);
			} catch(_com_error e) {
				ADODB::_ConnectionPtr cur_db = cmd->ActiveConnection;
				ADODB::ErrorPtr ae= cur_db->Errors->GetItem((short)0);
				if (_stricmp("S1000",(LPCSTR)ae->SQLState)!=0)	// other than "SC_fetch to get a Procedure return failed"
					throw e;
			}
			if (ab==AB_EXTERNAL_CONTACTS && (rs!=0 && rs->State!=ADODB::adStateClosed && !rs->ADOEOF))	// bug #19050
			{
				VS_SimpleStr alias2 = vs::StrFromVariantT(rs->Fields->Item["alias"]->Value);
				VS_SimpleStr call_id2 = vs::StrFromVariantT(rs->Fields->Item["call_id"]->Value);
				rCnt.AddValue(ALIAS_PARAM, alias2.m_str);
				rCnt.AddValue(CALLID2_PARAM, call_id2.m_str);
				dstream4 << "ExtAB: " << user_id1.m_str << " invited " << wstr.m_str << ", alias2=" << alias2.m_str << ", call_id2=" << call_id2.m_str << "\n";
			}
			if (!check_error)
				continue;
			error_code_out = rs->Fields->Item["error_code"]->Value;
			if(0 == error_code_out)
			{
				if(rs!=0 && rs->State!=ADODB::adStateClosed && !rs->ADOEOF)
				{
					static const _bstr_t hname("hash");
					ADODB::FieldPtr hfield=rs->Fields->GetItem((short)0);

					if(hfield->Name==hname)
					{
						_variant_t val(hfield->Value);

						if(val.vt!=VT_NULL && val.vt!=VT_EMPTY)
							hash=val;

					};

					const char* out_param = dbo->IsPostgreSQL? CALL_ID_OUT: CALL_ID;
					VS_WideStr wstr = (wchar_t*)(_bstr_t) rs->Fields->Item[out_param]->Value;
					add_call_id.Attach(wstr.ToUTF8());

					out_param = dbo->IsPostgreSQL? DB_DISPLAYNAME_OUT: DB_DISPLAYNAME;
					VS_WideStr dn = vs::WStrFromVariantT(rs->Fields->Item[out_param]->Value);
					if (dn) add_display_name = vs::WideCharToUTF8Convert(dn.m_str);
				}
				error = 0;
			}
			switch(error_code_out)
			{
			case 0:
				error = 0;
				break;
			case 1:
				error = VSS_USER_EXISTS;
				break;
			case 2:
				error = VSS_DB_ERROR;
				break;
			}
		}
	}
	catch(_com_error e)
	{
		ADODB::_ConnectionPtr cur_db = cmd->ActiveConnection;
		if(cur_db==0)
		{
			_bstr_t	add_str(cmd->GetCommandText());
			dbo->ProcessCOMError(e,cur_db,this,cmd->CommandText);
		}
		if(cur_db->Errors->Count==0)
		{
			error=VSS_USER_NOT_VALID;
			dprint2("$AddToAddressBook:  call_id not valid %s\n",(PCSTR)(_bstr_t)e.Description());
		}
		else
		{
			ADODB::ErrorPtr ae= cur_db->Errors->GetItem((short)0);
			if(ae->Number==0x80040e2f && ae->NativeError==0x00000a43 )
			{
				dprint3(" user already exists in address book(%s)\n",(PCSTR)(_bstr_t)e.Description());
				error=VSS_USER_EXISTS;
			}
			else if(ae->Number==0x80040e2f && ae->NativeError==0x00000203 )
			{
				dprint3(" user not found in address book(%s)\n",(PCSTR)(_bstr_t)e.Description());
				error=VSS_USER_NOT_FOUND;
			}
			else
			{
				error=VSS_DB_ERROR;
				_bstr_t	add_str(cmd->GetCommandText());
				dbo->ProcessCOMError(e,cur_db,this,add_str);
			}
		};
	}

	ReleaseDBO(dboitem);
	return error;
};

int VS_DBStorage::UpdateAddressBook(VS_AddressBook ab,const vs_user_id& user_id1,const char* call_id2, VS_Container& cnt, long& hash, VS_Container& rCnt)
{
AUTO_PROF
	const VS_Pool::Item* dboitem;
	VS_DBObjects* dbo=GetDBO(dboitem);
	int error=-1;

	hash=0;
	ADODB::_CommandPtr cmd;
	try
	{
		dprint2("$UpdateAddressBook %d of %s\n", ab,(const char*)user_id1);


		switch(ab)
		{
		case AB_COMMON:
			cmd=dbo->ab_update_user;
			break;
		case AB_PERSON_DETAILS:

			// TODO: implement for PostgreSQL
			if (!dbo->IsPostgreSQL)
				cmd=dbo->ab_update_details;
			else
				printf("not ab_update_details");

			break;
		case AB_USER_PICTURE:
			cmd=dbo->ab_update_picture;
			break;
		default:
			ReleaseDBO(dboitem);
			return false;
		};

		ADODB::ParametersPtr p=cmd->Parameters;

		short c=(short)p->Count;
		for(short i=0;i<c;i++)
			p->Item[i]->Value=db_null;

		p->Item[CALL_ID_PARAM]->Value = user_id1.m_str;
		VS_WideStr wstr; wstr.AssignUTF8(call_id2);
		p->Item[CALL_ID2_PARAM]->Value = wstr.m_str;

		char pname[256]="@";pname[sizeof(pname)-1]=0;

		int32_t lval; bool bval; double dval;

		cnt.Reset();
		while (cnt.Next()) {
			const char * Name = cnt.GetName();
			if (_stricmp(Name, METHOD_PARAM) == 0
				|| _stricmp(Name, ADDRESSBOOK_PARAM) == 0
				|| _stricmp(Name, CALLID_PARAM) == 0
				|| _stricmp(Name, USERNAME_PARAM) == 0
				|| _stricmp(Name, TRANSPORT_SRCUSER_PARAM) == 0
				)
			{}
			else
			{
				strncpy(pname+1,Name,sizeof(pname)-2);
				switch(cnt.GetType())
				{
				case VS_CNT_STRING_VT:
				default:
					wstr.AssignUTF8(cnt.GetStrValueRef());
					p->Item[pname]->Value = wstr.m_str;
					break;
				case VS_CNT_INTEGER_VT:
					cnt.GetValue(lval);
					p->Item[pname]->Value=lval;
					break;
				case VS_CNT_BOOL_VT:
					cnt.GetValue(bval);
					p->Item[pname]->Value=bval;
					break;
				case VS_CNT_DOUBLE_VT:
					cnt.GetValue(dval);
					p->Item[pname]->Value=dval;
					break;
				case VS_CNT_BINARY_VT:
					{

						size_t size;
						const void* data=cnt.GetBinValueRef(size);

						if(size>0)
						{
							SAFEARRAY sa;
							VARIANT	v2;
							sa.rgsabound[0].cElements = size;
							sa.rgsabound[0].lLbound = 0;
							sa.cbElements	=	1;
							sa.cDims			=	1;
							sa.fFeatures	=	128;
							sa.cLocks			=	0;
							sa.pvData			=	(void*)data;

							v2.parray=&sa;
							v2.vt=VT_UI1|VT_ARRAY;

							_variant_t v3(v2);

							p->Item[pname]->Value=v3;
						}
						else
							p->Item[pname]->Value=db_null;
						break;
					}
				}
			}
		}
		ADODB::_RecordsetPtr rs=cmd->Execute(NULL,NULL,ADODB::adCmdUnspecified);

		if(rs!=0 && rs->State!=ADODB::adStateClosed && !rs->ADOEOF)
		{
			static const _bstr_t hname("hash");
			ADODB::FieldPtr hfield=rs->Fields->GetItem((short)0);

			if(hfield->Name==hname)
			{
				_variant_t val(hfield->Value);

				if(val.vt!=VT_NULL && val.vt!=VT_EMPTY)
					hash=val;

			};
		}

		error=0;
	}
	catch(_com_error e)
	{
		ADODB::_ConnectionPtr cur_db = cmd->ActiveConnection;
		if(dbo->db==0)
		{
			_bstr_t	add_str(cmd->GetCommandText());
			dbo->ProcessCOMError(e,cur_db,this,add_str);
		}
		if(cur_db->Errors->Count==0)
		{
			error=VSS_USER_NOT_VALID;
			dprint2("$UpdateAddressBook:  call_id not valid %s\n",(PCSTR)(_bstr_t)e.Description());
		}
		else
		{
			ADODB::ErrorPtr ae= cur_db->Errors->GetItem((short)0);
			if(ae->Number==0x80040e2f && ae->NativeError==0x00000203 )
			{
				dprint3(" user not found in address book(%s)\n",(PCSTR)(_bstr_t)e.Description());
				error=VSS_USER_NOT_FOUND;
			}
			else
			{
				error=VSS_DB_ERROR;
				_bstr_t	add_str(cmd->GetCommandText());
				dbo->ProcessCOMError(e,cur_db,this,add_str);
			}
		};
	}

	ReleaseDBO(dboitem);
	return error;
};


int VS_DBStorage::RemoveFromAddressBook(VS_AddressBook ab, const vs_user_id& user_id1,const vs_user_id& user_id2, VS_Container& cnt, long& hash, VS_Container& rCnt)
{
AUTO_PROF
	const VS_Pool::Item* dboitem;
	VS_DBObjects* dbo=GetDBO(dboitem);

	hash=0;
	int error=-1;
	ADODB::_CommandPtr cmd;
	try
	{
		dprint1("$RemoveFromAddressBook %d of %s ,removing %s\n", ab, user_id1.m_str, user_id2.m_str);
		const char* call_id1_param = CALL_ID_PARAM;
		const char* call_id2_param = CALL_ID2_PARAM;
		switch(ab)
		{
		case AB_COMMON:
			cmd=dbo->ab_remove_user;
			break;
		case AB_BAN_LIST:
			cmd=dbo->unban_user;
			break;
		case AB_EXTERNAL_CONTACTS:
			cmd=dbo->ab_external_delete_user;
			break;
		case AB_USER_PICTURE:
			cmd = dbo->ab_delete_picture;
			call_id1_param = "@c_call_id";
			call_id2_param = 0;						// not used
			break;
		case AB_MY_EXTERNAL_CONTACTS:
			cmd=dbo->vs_delete_alias;
			call_id1_param = "c_call_id";
			call_id2_param = "c_alias";
			cmd->Parameters->Item["i_alias_type"]->Value = (int)0;
			break;
		default:
			ReleaseDBO(dboitem);
			return false;
		};

		//WaitForCommand(cmd);
		ADODB::ParametersPtr p=cmd->Parameters;

		if (call_id1_param && *call_id1_param)
			p->Item[call_id1_param]->Value = user_id1.m_str;
		if (call_id2_param && *call_id2_param) {
			if (!!user_id2){
				VS_WideStr wstr;	wstr.AssignUTF8(user_id2.m_str);
				p->Item[call_id2_param]->Value = wstr.m_str;
			} else {
				p->Item[call_id2_param]->Value = db_null;
			}
		}

		ADODB::_RecordsetPtr rs=cmd->Execute(NULL,NULL,ADODB::adCmdUnspecified);

		if(rs!=0 && rs->State!=ADODB::adStateClosed && !rs->ADOEOF)
		{
			static const _bstr_t hname("hash");
			ADODB::FieldPtr hfield=rs->Fields->GetItem((short)0);

			if(hfield->Name==hname)
			{
				_variant_t val(hfield->Value);

				if(val.vt!=VT_NULL && val.vt!=VT_EMPTY)
					hash=val;

			};
		}

		error=0;
	}
	catch(_com_error e)
	{
		_bstr_t	add_str(cmd->GetCommandText());
		dbo->ProcessCOMError(e,cmd->ActiveConnection,this,add_str);
		error=-1;
	}

	ReleaseDBO(dboitem);
	return error;
};

////////////////////////////////////////////////////
// Logging

bool	VS_DBStorage::LogParticipantInvite	(const vs_conf_id& conf_id,const VS_SimpleStr& call_id1, const VS_SimpleStr& app_id, const VS_SimpleStr& call_id2,const std::chrono::system_clock::time_point time,VS_ParticipantDescription::Type type)
{
AUTO_PROF
	const VS_Pool::Item* dboitem;
	VS_DBObjects* dbo=GetDBO(dboitem);
	bool ret = dbo->LogParticipantInvite(conf_id, call_id1, app_id, call_id2, time, type);
	ReleaseDBO(dboitem);
	return ret;
/*
	try
	{

		char buf[256];
		dprint2("$LogPartInvite from '%s' to '%s', conf(%s) time(%s)\n",(const char*)call_id1,(const char*)call_id2,(const char*)conf_id,time==0?"current":time.ToNStr(buf));

		ADODB::ParametersPtr p=dbo->log_part_invite->Parameters;

		int conference;VS_SimpleStr broker_id;
		SplitConfID(conf_id,conference,broker_id);

		p->Item[DB_CONFERENCE_PARAM]->Value		=conference;
		p->Item[DB_SERVER_PARAM]->Value			=broker_id;
		p->Item[CALL_ID_PARAM]->Value			=call_id1;
		p->Item[CALL_ID2_PARAM]->Value			=call_id2;
		p->Item[APP_ID_PARAM]->Value			=app_id;
		if(time!=0)
			p->Item[DB_TIME_PARAM]->Value		=time;
		else
			p->Item[DB_TIME_PARAM]->Value		=db_null;

		p->Item[DB_TYPE_PARAM]->Value			=type;


		dbo->log_part_invite->Execute(NULL,NULL,ADODB::adCmdStoredProc|ADODB::adExecuteNoRecords);

	}
	catch(_com_error e)
	{
		_bstr_t	add_str(dbo->log_part_invite->GetCommandText());
		dbo->ProcessCOMError(e,dbo->log_part_invite->ActiveConnection,this,add_str);
		ReleaseDBO(dboitem);
		return false;
	}
	ReleaseDBO(dboitem);
	return true;
*/
};



bool	VS_DBStorage::LogConferenceStart(const VS_ConferenceDescription& conf,bool remote)
{
AUTO_PROF
	const VS_Pool::Item* dboitem;
/**
	m_MaxParticipants
	topic
	curr_users
*/
	VS_DBObjects* dbo=GetDBO(dboitem);
	bool ret = dbo->LogConferenceStart(conf, remote);
	ReleaseDBO(dboitem);
	return ret;
};
bool	VS_DBStorage::LogConferenceEnd  (const VS_ConferenceDescription& conf)
{
AUTO_PROF
	  const VS_Pool::Item* dboitem;
	  VS_DBObjects* dbo=GetDBO(dboitem);
	  bool ret = dbo->LogConferenceEnd(conf);
	  ReleaseDBO(dboitem);
	  return ret;
	  /*
		try
		{
			char buf[256];
			dprint2("$LogConfEnd conf(%s) end time (%s) \n",(const char*)conf.m_name,(const char*)conf.m_logEnded.ToNStr(buf));

			//WaitForCommand(log_conf_end);
			ADODB::ParametersPtr p=dbo->log_conf_end->Parameters;

			int conference;VS_SimpleStr broker_id;
			SplitConfID(conf.m_name,conference,broker_id);

			p->Item[DB_CONFERENCE_PARAM]->Value		=conference;
			p->Item[DB_SERVER_PARAM]->Value			=broker_id;
			p->Item[DB_OWNER_PARAM]->Value			=conf.m_owner;
			p->Item[CONF_TERM_PARAM]->Value			=conf.m_logCause;
			p->Item[DB_TYPE_PARAM]->Value				=conf.m_type;
			p->Item[DB_SUBTYPE_PARAM]->Value		=conf.m_SubType;
			p->Item[DB_TIME_PARAM]->Value				=conf.m_logEnded==NULL?db_null:conf.m_logEnded;

			dbo->log_conf_end->Execute(0,0,ADODB::adCmdUnspecified|ADODB::adExecuteNoRecords);

		}
		catch(_com_error e)
		{
			_bstr_t	add_str(dbo->log_conf_end->GetCommandText());
			dbo->ProcessCOMError(e,dbo->log_conf_end->ActiveConnection,this,add_str);
			ReleaseDBO(dboitem);
			return false;
		}
		ReleaseDBO(dboitem);
		return true;
*/
};

bool	VS_DBStorage::LogParticipantJoin (const VS_ParticipantDescription& pd,const VS_SimpleStr& call_id2)
{
AUTO_PROF
	  const VS_Pool::Item* dboitem;
	  VS_DBObjects* dbo=GetDBO(dboitem);
	  bool ret = dbo->LogParticipantJoin(pd, call_id2);
	  ReleaseDBO(dboitem);
	  return ret;
/*
		try
		{
			char buf[256];
			dprint2("$LogPartJoin conf(%s) user(%s) join time(%s)\n",(const char*)pd.m_conf_id,(const char*)pd.m_user_id,pd.m_joinTime.ToNStr(buf));

			//WaitForCommand(log_part_join);
			ADODB::ParametersPtr p=dbo->log_part_join->Parameters;

			int conference;VS_SimpleStr broker_id;
			SplitConfID(pd.m_conf_id,conference,broker_id);

			p->Item[DB_CONFERENCE_PARAM]->Value		=conference;
			p->Item[DB_SERVER_PARAM]->Value			=broker_id;
			p->Item[CALL_ID_PARAM]->Value			=pd.m_user_id;
			p->Item[DB_TYPE_PARAM]->Value			=pd.m_type;
			p->Item[APP_ID_PARAM]->Value			=pd.m_appID;

			if(pd.m_joinTime!=NULL)
			{	p->Item[PART_JOIN_DB_TIME_PARAM]->Value	=pd.m_joinTime;	}
			else
				p->Item[PART_JOIN_DB_TIME_PARAM]->Value	=db_null;

			if(call_id2!=NULL)
			{	p->Item[CALL_ID2_PARAM]->Value	=call_id2;	}
			else
				p->Item[CALL_ID2_PARAM]->Value	=db_null;
			//leave time - here
			//p->Item[PART_LEAVE_DB_TIME_PARAM]->Value	=leaveTime;
			p->Item[PART_PRICE_PARAM]->Value						=pd.m_decLimit;
			p->Item[PART_CHARGE1_PARAM]->Value					=pd.m_charge1;
			p->Item[PART_CHARGE2_PARAM]->Value					=pd.m_charge2;
			p->Item[PART_CHARGE3_PARAM]->Value					=pd.m_charge3;



			//app_id should be taken internally

			dbo->log_part_join->Execute(NULL,NULL,ADODB::adCmdUnspecified|ADODB::adExecuteNoRecords);

		}
		catch(_com_error e)
		{
			_bstr_t	add_str(dbo->log_part_join->GetCommandText());
			dbo->ProcessCOMError(e,dbo->log_part_join->ActiveConnection,this,add_str);
			ReleaseDBO(dboitem);
			return false;
		}
		ReleaseDBO(dboitem);
		return true;
*/
};

bool	VS_DBStorage::LogParticipantReject(const vs_conf_id& conf_id, const vs_user_id& user, const vs_user_id& invited_from, const VS_Reject_Cause cause)
{
	// todo(kt): implement
	return false;
}


bool	VS_DBStorage::LogParticipantLeave(const VS_ParticipantDescription& pd)
{
AUTO_PROF
	  const VS_Pool::Item* dboitem;
	  VS_DBObjects* dbo=GetDBO(dboitem);
	  bool ret = dbo->LogParticipantLeave(pd);
	  ReleaseDBO(dboitem);
	  return ret;
/*
		try
		{
			char buf[256];
			dprint2("$LogPartLeave conf(%s) user(%s) leave time(%s)\n",(const char*)pd.m_conf_id,(const char*)pd.m_user_id,pd.m_leaveTime.ToNStr(buf));

			//WaitForCommand(log_part_leave);
			ADODB::ParametersPtr p=dbo->log_part_leave->Parameters;

			int conference;VS_SimpleStr broker_id;
			SplitConfID(pd.m_conf_id,conference,broker_id);

			p->Item[DB_CONFERENCE_PARAM]->Value			=conference;
			p->Item[DB_SERVER_PARAM]->Value				=broker_id;
			p->Item[CALL_ID_PARAM]->Value				=pd.m_user_id;
			p->Item[DB_TYPE_PARAM]->Value				=pd.m_type;

			p->Item[PART_LEAVE_REASON_PARAM]->Value		=pd.m_cause;
			p->Item[PART_BYTES_SENT_PARAM]->Value		=pd.m_bytesSnd;
			p->Item[PART_BYTES_RECEIVED_PARAM]->Value	=pd.m_bytesRcv;
			p->Item[PART_RECON_SND_PARAM]->Value		=pd.m_reconSnd;
			p->Item[PART_RECON_RCV_PARAM]->Value		=pd.m_reconRcv;

			//p->Item[PART_JOIN_DB_TIME_PARAM]->Value	=pd.m_joinTime;
			//leave time - here
			if(pd.m_leaveTime!=NULL)
			{	p->Item[PART_LEAVE_DB_TIME_PARAM]->Value	=pd.m_leaveTime;}
			else
				p->Item[PART_LEAVE_DB_TIME_PARAM]->Value	=db_null;



			//app_id should be taken internally

			dbo->log_part_leave->Execute(NULL,NULL,ADODB::adCmdUnspecified|ADODB::adExecuteNoRecords);

		}
		catch(_com_error e)
		{
			_bstr_t	add_str(dbo->log_part_leave->GetCommandText());
			dbo->ProcessCOMError(e,dbo->log_part_leave->ActiveConnection,this,add_str);
			ReleaseDBO(dboitem);
			return false;
		}
		ReleaseDBO(dboitem);
		return true;
*/
};

bool	VS_DBStorage::LogParticipantStatistics (const VS_ParticipantDescription& pd)
{
AUTO_PROF
	const VS_Pool::Item* dboitem;
	VS_DBObjects* dbo=GetDBO(dboitem);
	bool ret = dbo->LogParticipantStatistics(pd);
	ReleaseDBO(dboitem);
	return ret;
/*
	bool result=false;

	try
	{
		dprint3("$LogPartStats conf(%s) user(%s)\n",(const char*)pd.m_conf_id,(const char*)pd.m_user_id);

		//WaitForCommand(log_part_leave);
		ADODB::ParametersPtr p=dbo->log_part_stats->Parameters;

		int conference;VS_SimpleStr broker_id;
		SplitConfID(pd.m_conf_id,conference,broker_id);

		p->Item[DB_CONFERENCE_PARAM]->Value			=conference;
		p->Item[DB_SERVER_PARAM]->Value			  =broker_id;
		p->Item[CALL_ID_PARAM]->Value					=pd.m_user_id;

		p->Item[PART_BYTES_SENT_PARAM]->Value			=pd.m_bytesSnd;
		p->Item[PART_BYTES_RECEIVED_PARAM]->Value	=pd.m_bytesRcv;
		p->Item[PART_RECON_SND_PARAM]->Value			=pd.m_reconSnd;
		p->Item[PART_RECON_RCV_PARAM]->Value			=pd.m_reconRcv;

		dbo->log_part_stats->Execute(NULL,NULL,ADODB::adCmdUnspecified|ADODB::adExecuteNoRecords);

		result=true;

	}
	catch(_com_error e)
	{
		_bstr_t	add_str(dbo->log_part_stats->GetCommandText());
		dbo->ProcessCOMError(e,dbo->log_part_stats->ActiveConnection,this,add_str);
	}

	ReleaseDBO(dboitem);
	return result;
	*/
};

bool VS_DBStorage::LogSlideShowCmd(const char *confId, const char *from, const char *url, const char *mimeType, size_t slideIndex, size_t slidesCount, const char *about,
								   size_t width, size_t height, size_t size)
{
	return false;
}

bool VS_DBStorage::LogSlideShowEnd(const char *confId, const char *from)
{
	return false;
}

bool VS_DBStorage::LogGroupChat(const char *confId, const char *from, const char *text)
{
	return false;
}

bool VS_DBStorage::UpdateConferencePic(VS_Container &cnt)
{
AUTO_PROF
	const VS_Pool::Item* dboitem;
	VS_DBObjects* dbo=GetDBO(dboitem);
	bool result=false;

	vs_conf_id conf_id = cnt.GetStrValueRef(CONFERENCE_PARAM);
	int conference(0);
	VS_SimpleStr	server_id;
	dbo->SplitConfID(conf_id,conference,server_id);
	size_t size(0);
	const void * data = cnt.GetBinValueRef(PICTURE_PARAM, size);

	_variant_t v3;
	if(size>0) {
		SAFEARRAY sa;
		sa.rgsabound[0].cElements = size;
		sa.rgsabound[0].lLbound = 0;
		sa.cbElements	=	1;
		sa.cDims		=	1;
		sa.fFeatures	=	128;
		sa.cLocks		=	0;
		sa.pvData		=	(void*)data;

		VARIANT	v2;
		v2.parray = &sa;
		v2.vt = VT_UI1|VT_ARRAY;
		v3 = v2;
	}
	try
	{
		dprint3("$UpdateConferencePic: conf_id = %s; pic_size = %zu\n", conf_id.m_str, size);
		ADODB::ParametersPtr p=dbo->update_conf_info->Parameters;
		p->Item[DB_CONFERENCE_PARAM]->Value = conference;
		p->Item[DB_SERVER_PARAM]->Value = server_id.m_str;
		p->Item[DB_CONFERENCE_PIC_PARAM]->Value = v3;

		dbo->update_conf_info->Execute(NULL,NULL,ADODB::adCmdUnspecified|ADODB::adExecuteNoRecords);

		result=true;
	}
	catch(_com_error e)
	{
		_bstr_t	add_str(dbo->update_conf_info->GetCommandText());
		dbo->ProcessCOMError(e,dbo->update_conf_info->ActiveConnection,this,add_str);
	}

	ReleaseDBO(dboitem);
	return result;
}

#pragma warning( disable : 4312 4311)
////////////////////////////////////////////////////
// Endpoint management
	//applications
int VS_DBStorage::GetAllAppProperties(VS_AppPropertiesMap &prop_map)
{
AUTO_PROF
	const VS_Pool::Item* dboitem;
	VS_DBObjects* dbo=GetDBO(dboitem);
	int count(0);
	try
	{
		dprint4("$GetAllAppProperties\n");

		ADODB::_RecordsetPtr rs = dbo->app_prop_get_all_props->Execute(NULL,NULL,ADODB::adCmdUnspecified);

		for(count =0;!rs->ADOEOF;rs->MoveNext() , count++)
		{
			VS_Container	*prop = new VS_Container;
			ADODB::FieldsPtr f= rs->Fields;
			ADODB::FieldPtr p;
			int n = f->Count;

			_bstr_t		name;
			_variant_t	val;

			VS_SimpleStr key_name = vs::StrFromVariantT(f->Item[APP_NAME]->Value);
			for(short i = 0;i<n;i++)
			{
				p = f->GetItem(i);
				name = p->Name;
				val = p->Value;
				if(val.vt!=VT_EMPTY && val.vt !=VT_NULL)
				{
					dstream4 << "\tadding property " << (char*)name << "='" << (wchar_t*)(_bstr_t)val << "'\n";
					prop->AddValue((char*)name,(wchar_t*)(_bstr_t)val);
				}

			}
			prop_map[key_name] = prop;
			prop = 0;
		}


	}
	catch(_com_error e)
	{
		_bstr_t	add_str(dbo->app_prop_get_all_props->GetCommandText());
		dbo->ProcessCOMError(e,dbo->app_prop_get_all_props->ActiveConnection,this,add_str);
		count=0;
	}

	ReleaseDBO(dboitem);
	return count;
}

int VS_DBStorage::GetAppProperties(VS_Container& prop,const VS_SimpleStr& app_name)
{
AUTO_PROF
	const VS_Pool::Item* dboitem;
	VS_DBObjects* dbo=GetDBO(dboitem);
	int count=0;

	try
	{

		dprint4("$GetAppProperties: for app '%s'\n",(const char*)app_name);
		dbo->app_prop_get_all->Parameters->Item[DB_APP_NAME_PARAM]->Value = app_name.m_str;

		ADODB::_RecordsetPtr rs=dbo->app_prop_get_all->Execute(NULL,NULL,ADODB::adCmdUnspecified);

		if(rs->ADOEOF)
		{
			error_code=0;
			dprint1("$GetAppProperties: props for app '%s' not found\n",(const char*)app_name);
			ReleaseDBO(dboitem);
			return 0;
		}

		ADODB::FieldsPtr f= rs->Fields;

		ADODB::FieldPtr p;
		_bstr_t			name;
		_variant_t	val;

		count=f->Count;


		for(short i=0;i<count;i++)
		{
			p= f->GetItem(i);
			name=p->Name;
			val=p->Value;
			if(val.vt!=VT_EMPTY && val.vt !=VT_NULL)
			{
				dstream4 << "\tadding property " << (char*)name << "='" << (wchar_t*)(_bstr_t)val << "'\n";
				prop.AddValue((char*)name,(wchar_t*)(_bstr_t)val);
			};
		};

		dprint4("\tfound %d properties\n",count);
	}
	catch(_com_error e)
	{
		_bstr_t	add_str(dbo->app_prop_get_all->GetCommandText());
		dbo->ProcessCOMError(e,dbo->app_prop_get_all->ActiveConnection,this,add_str);
		count=0;
	}

	ReleaseDBO(dboitem);
	return count;
};

//bool VS_DBStorage::GetServerProperty(const VS_SimpleStr& name,VS_SimpleStr& value)
//{
//	const VS_Pool::Item* dboitem;
//	VS_DBObjects* dbo=GetDBO(dboitem);
//	bool result=false;
//	try
//	{
//		dprint4("$Get Server Property '%s' \n",(const char*)name);
//
//		//WaitForCommand(srv_prop_get);
//		dbo->srv_prop_get->Parameters->Item[PROP_NAME_PARAM]->Value=name;
//
//		ADODB::_RecordsetPtr rs=dbo->srv_prop_get->Execute(NULL,NULL,ADODB::adCmdUnspecified);
//
//		if(rs->ADOEOF)
//		{
//			error_code=0;
//			dprint1("$ Server Property '%s' not found \n",(const char*)name);
//			ReleaseDBO(dboitem);
//			return false;
//		}
//
//		value=rs->Fields->Item[(short)0]->Value;
//
//
//		dprint4("\t='%s' \n",(const char*)value);
//		result=true;
//	}
//	catch(_com_error e)
//	{
//		dbo->ProcessCOMError(e,this);
//		result=false;
//	}
//
//	ReleaseDBO(dboitem);
//	return result;
//}
//
//bool VS_DBStorage::GetServerProperty(const VS_SimpleStr& name,VS_WideStr& value)
//{
//	  const VS_Pool::Item* dboitem;
//	  VS_DBObjects* dbo=GetDBO(dboitem);
//		try
//		{
//			dprint4("$Get Server Property '%s' \n",(const char*)name);
//
//			//WaitForCommand(srv_prop_get);
//			dbo->srv_prop_get->Parameters->Item[PROP_NAME_PARAM]->Value=name;
//
//			ADODB::_RecordsetPtr rs=dbo->srv_prop_get->Execute(NULL,NULL,ADODB::adCmdUnspecified);
//
//			if(rs->ADOEOF)
//			{
//				error_code=0;
//				dprint1("$ Server Property '%s' not found \n",(const char*)name);
//				ReleaseDBO(dboitem);
//				return false;
//			}
//
//			value=rs->Fields->Item[(short)0]->Value;
//
//
//			dprint4("\t='%S' \n",(const wchar_t*)value);
//		}
//		catch(_com_error e)
//		{
//			dbo->ProcessCOMError(e,this);
//			ReleaseDBO(dboitem);
//			return false;
//		}
//		ReleaseDBO(dboitem);
//		return true;
//}
//
//int VS_DBStorage::GetServerProperties(VS_Container& prop,SrvPropertyType type)
//{
//	  const VS_Pool::Item* dboitem;
//	  VS_DBObjects* dbo=GetDBO(dboitem);
//		int	count=0;
//
//
//		try
//		{
//			dprint3("$Get Server Properties of type '%d' \n",type);
//
//			//WaitForCommand(srv_prop_get);
//			dbo->srv_prop_get_all->Parameters->Item[DB_TYPE_PARAM]->Value=(int)type;
//
//			ADODB::_RecordsetPtr rs=dbo->srv_prop_get_all->Execute(NULL,NULL,ADODB::adCmdUnspecified);
//
//			ADODB::FieldsPtr f=rs->Fields;
//			_variant_t name,val;
//
//			for(count=0;!rs->ADOEOF;rs->MoveNext(),count++)
//			{
//				name=f->Item[(short)0]->Value;
//				val =f->Item[(short)1]->Value;
//				if(name.vt!=VT_NULL && name.vt!=VT_EMPTY &&
//					 val.vt!=VT_EMPTY && val.vt !=VT_NULL)
//				{
//					dprint4("\tadding property %s='%S'",(char*)(_bstr_t)name,(wchar_t*)(_bstr_t)val);
//					prop.AddValue((char*)(_bstr_t)name,(wchar_t*)(_bstr_t)val);
//				};
//
//			}
//			dprint3("\tfound %d \n",count);
//		}
//		catch(_com_error e)
//		{
//			dbo->ProcessCOMError(e,this);
//			//count=0;
//		}
//
//		ReleaseDBO(dboitem);
//		return count;
//}

bool VS_DBStorage::GetUserProperty(const vs_user_id& user_id,const std::string& name, std::string& value)
{
AUTO_PROF
	  const VS_Pool::Item* dboitem;
	  VS_DBObjects* dbo=GetDBO(dboitem);
		bool result=false;


		try
		{
			dprint3("$Get User Property '%s'\n",name.c_str());

			dbo->user_prop_get->Parameters->Item[CALL_ID_PARAM]->Value = user_id.m_str;
			dbo->user_prop_get->Parameters->Item[PROP_NAME_PARAM]->Value=name.c_str();

			ADODB::_RecordsetPtr rs=dbo->user_prop_get->Execute(NULL,NULL,ADODB::adCmdUnspecified);

			if(rs->ADOEOF)
			{
				dprint3(" not found \n");
				throw 0;
			}

			VS_WideStr val = vs::WStrFromVariantT(rs->Fields->Item[(short)0]->Value);
			if (val) value = vs::WideCharToUTF8Convert(val.m_str);

			dstream3 << "\t='" << value << "' \n";
			result=true;
		}
		catch(_com_error e)
		{
			_bstr_t	add_str(dbo->user_prop_get->GetCommandText());
			dbo->ProcessCOMError(e,dbo->user_prop_get->ActiveConnection,this,add_str);
			result=false;
		}
		catch(int err)
		{
			error_code=err;
			result=false;
		}

		ReleaseDBO(dboitem);
		return result;
}

int  VS_DBStorage::SaveTempLoginData(const char* login, const char* app_id, VS_Container &data){
	if (!login || !app_id) return 0;

AUTO_PROF

	int result = 0;
	const VS_Pool::Item* dboitem;
	VS_DBObjects* dbo = GetDBO(dboitem);
	VS_SCOPE_EXIT{ ReleaseDBO(dboitem); };

	void* buff = nullptr;
	size_t size = 0;
	if (!data.SerializeAlloc(buff, size))
		return 0;
	VS_SCOPE_EXIT{ ::free(buff); };

	size_t new_size;
	base64_encode(buff, size, nullptr, new_size);
	auto encoded_buff = vs::make_unique_default_init<char[]>(new_size);
	if (!base64_encode(buff, size, encoded_buff.get(), new_size))
		return 0;
	string_view data_to_store(encoded_buff.get(), new_size);

	ADODB::_CommandPtr cmd = dbo->sp_login_temp_set_param;
	try
	{
		dprint3("$SaveTempLoginData for '%s' app_id = '%s'\n", login, app_id);

		std::string templ = "select sp_login_temp_set_param('%login%','%app_id%','%data%')";
		VS_ReplaceAll(templ, "%login%", login);
		VS_ReplaceAll(templ, "%app_id%", app_id);
		VS_ReplaceAll(templ, "%data%", data_to_store);
		cmd->CommandText = templ.c_str();

		ADODB::_RecordsetPtr rs = cmd->Execute(NULL, NULL, ADODB::adCmdText);
		if (!rs->ADOEOF)
			result = rs->Fields->Item[(short)0]->Value;
	}
	catch (_com_error &e)
	{
		_bstr_t	add_str(cmd->GetCommandText());
		dbo->ProcessCOMError(e, cmd->ActiveConnection, this, add_str);
		result = 0;
	}
	catch (int err)
	{
		error_code = err;
		result = 0;
	}
	return result;
}


bool VS_DBStorage::GetServerTime(std::chrono::system_clock::time_point& ftime)
{
AUTO_PROF

	  const VS_Pool::Item* dboitem;
	  VS_DBObjects* dbo=GetDBO(dboitem);
		try
		{
			dprint4("$Get Server Time \n");

			ADODB::_RecordsetPtr rs;
			rs.CreateInstance("ADODB.Recordset");
			rs->Open("SELECT GETDATE() as time ",_variant_t((IDispatch*)dbo->db,true),ADODB::adOpenDynamic, ADODB::adLockOptimistic, ADODB::adCmdText);
			VS_SCOPE_EXIT{ rs->Close(); };

			if(rs->ADOEOF)
			{
				error_code=0;
				dprint1("$ Error while fetching time\n");
				ReleaseDBO(dboitem);
				return false;
			}

			_variant_t val=rs->Fields->Item[(short)0]->Value;

			FILETIME ftime2, ftime1;
			SYSTEMTIME stime;

			VariantTimeToSystemTime(val.dblVal, &stime);
			SystemTimeToFileTime(&stime,&ftime2);
			LocalFileTimeToFileTime(&ftime2, &ftime1);
			ftime = tu::WindowsTickToUnixSeconds(*reinterpret_cast<const int64_t *>(&ftime1));

		}
		catch(_com_error e)
		{
			dbo->ProcessCOMError(e,dbo->db,this,"GetServerTime");
			ReleaseDBO(dboitem);
			return false;
		}
		ReleaseDBO(dboitem);
		return true;
};

bool VS_DBStorage::SetAppProperty(const VS_SimpleStr& prop_name, const VS_SimpleStr &value) {
	VS_AutoLock _(&m_additional_app_properties_lock);
	m_additional_app_properties[prop_name] = value;
	return true;
}

bool VS_DBStorage::GetAppProperty(const VS_SimpleStr& app_name,const VS_SimpleStr& prop_name,VS_SimpleStr& value)
{
AUTO_PROF
	  const VS_Pool::Item* dboitem;
	  VS_DBObjects* dbo=GetDBO(dboitem);
		try
		{
			dprint3("$Get Application %s Property '%s' \n",(const char*)app_name,(const char*)prop_name);

			//WaitForCommand(app_prop_get);
			dbo->app_prop_get->Parameters->Item[DB_APP_NAME_PARAM]->Value = app_name.m_str;
			dbo->app_prop_get->Parameters->Item[DB_QUERY_PARAM]->Value = prop_name.m_str;

			ADODB::_RecordsetPtr rs=dbo->app_prop_get->Execute(NULL,NULL,ADODB::adCmdUnspecified);

			if(rs->ADOEOF)
			{
				error_code=0;
				dprint1("$Application %s or app property '%s' not found \n",(const char*)app_name,(const char*)prop_name);
				ReleaseDBO(dboitem);
				return false;
			}

			value = vs::StrFromVariantT(rs->Fields->Item[(short)0]->Value);


			dprint4("\t='%s' \n",(const char*)value);
		}
		catch(_com_error e)
		{
			_bstr_t	add_str(dbo->app_prop_get->GetCommandText());
			dbo->ProcessCOMError(e,dbo->app_prop_get->ActiveConnection,this,add_str);
			ReleaseDBO(dboitem);
			return false;
		}
		ReleaseDBO(dboitem);
		return true;
}

bool VS_DBStorage::GetAppProperty(const VS_SimpleStr& app_name,const VS_SimpleStr& prop_name,VS_WideStr& value)
{
AUTO_PROF
	  const VS_Pool::Item* dboitem;
	  VS_DBObjects* dbo=GetDBO(dboitem);
		try
		{
			dprint3("$Get Application %s Property '%s' \n",(const char*)app_name,(const char*)prop_name);

			//WaitForCommand(app_prop_get);
			dbo->app_prop_get->Parameters->Item[DB_APP_NAME_PARAM]->Value = app_name.m_str;
			dbo->app_prop_get->Parameters->Item[DB_QUERY_PARAM]->Value = prop_name.m_str;

			ADODB::_RecordsetPtr rs=dbo->app_prop_get->Execute(NULL,NULL,ADODB::adCmdUnspecified);

			if(rs->ADOEOF)
			{
				error_code=0;
				dprint1("$Application %s or app property '%s' not found \n",(const char*)app_name,(const char*)prop_name);
				ReleaseDBO(dboitem);
				return false;
			}

			value = vs::WStrFromVariantT(rs->Fields->Item[(short)0]->Value);


			dstream4 << "\t='" << (const wchar_t*)value << "' \n";
		}
		catch(_com_error e)
		{
			_bstr_t	add_str(dbo->app_prop_get->GetCommandText());
			dbo->ProcessCOMError(e,dbo->app_prop_get->ActiveConnection,this,add_str);
			ReleaseDBO(dboitem);
			return false;
		}
		ReleaseDBO(dboitem);
		return true;
}

bool VS_DBStorage::GetServerProperty(const std::string &name, std::string &value)
{
AUTO_PROF
	const VS_Pool::Item* dboitem;
	VS_DBObjects* dbo=GetDBO(dboitem);
	try
	{
		dprint3("$Get Server Property '%s' \n",name.c_str());
		dbo->srv_prop_get->Parameters->Item["@c_type"]->Value = name.c_str();
		ADODB::_RecordsetPtr rs=dbo->srv_prop_get->Execute(NULL,NULL,ADODB::adCmdUnspecified);
		if(rs->ADOEOF)
		{
			error_code=0;
			dprint1("$Server property '%s' not found \n",name.c_str());
			ReleaseDBO(dboitem);
			return false;
		}
		try {
			value = static_cast<const char*>(static_cast<const bstr_t>(rs->Fields->Item[(short)0]->Value));
		}
		catch (...) { dstream4 << "Error\tFailed to assing value!\n"; }
		dprint4("\t='%s' \n", value.c_str());
	}
	catch(_com_error e)
	{
		_bstr_t	add_str(dbo->srv_prop_get->GetCommandText());
		dbo->ProcessCOMError(e,dbo->srv_prop_get->ActiveConnection,this,add_str);
		ReleaseDBO(dboitem);
		return false;
	}
	ReleaseDBO(dboitem);
	return true;

}

bool VS_DBStorage::SetServerProperty(const VS_SimpleStr& /*name*/,const VS_SimpleStr& /*value*/)
{
	return false;
}

bool VS_DBStorage::GetEndpointProperty(const VS_SimpleStr& app_id,const VS_SimpleStr& name,VS_SimpleStr& value)
{
AUTO_PROF
	  const VS_Pool::Item* dboitem;
	  VS_DBObjects* dbo=GetDBO(dboitem);
		try
		{
			dprint3("$Get Endpoint %s Property '%s' \n",(const char*)app_id,(const char*)name);

			//WaitForCommand(ep_prop_get);
			dbo->ep_prop_get->Parameters->Item[APP_ID_PARAM]->Value = app_id.m_str;
			dbo->ep_prop_get->Parameters->Item[PROP_NAME_PARAM]->Value = name.m_str;

			ADODB::_RecordsetPtr rs=dbo->ep_prop_get->Execute(NULL,NULL,ADODB::adCmdUnspecified);

			if(rs->ADOEOF)
			{
				error_code=0;
				dprint3("$ Endpoint %s or endpoint property '%s' not found \n",(const char*)app_id,(const char*)name);
				ReleaseDBO(dboitem);
				return false;
			}

			value = vs::StrFromVariantT(rs->Fields->Item[(short)0]->Value);


			dprint3("\t='%s' \n",(const char*)value);
		}
		catch(_com_error e)
		{
			_bstr_t	add_str(dbo->ep_prop_get->GetCommandText());
			dbo->ProcessCOMError(e,dbo->ep_prop_get->ActiveConnection,this,add_str);
			ReleaseDBO(dboitem);
			return false;
		}
		ReleaseDBO(dboitem);
		return true;
}

bool VS_DBStorage::SetEndpointProperty(const char* app_id, const char* name, const char* value)
{
AUTO_PROF
	  if (!value) return false;
	  const VS_Pool::Item* dboitem;
	  VS_DBObjects* dbo=GetDBO(dboitem);
		try
		{
			dprint4("$Set Endpoint %s Property '%s' \n", app_id, name);

			//WaitForCommand(ep_prop_set);
			dbo->ep_prop_set->Parameters->Item[APP_ID_PARAM]->Value = app_id;
			dbo->ep_prop_set->Parameters->Item[PROP_NAME_PARAM]->Value = name;
			dbo->ep_prop_set->Parameters->Item[PROP_VALUE_PARAM]->Value			=value;

			dbo->ep_prop_set->Execute(NULL,NULL,ADODB::adCmdStoredProc|ADODB::adExecuteNoRecords);

		}
		catch(_com_error e)
		{
			_bstr_t	add_str(dbo->ep_prop_set->GetCommandText());
			dbo->ProcessCOMError(e,dbo->ep_prop_set->ActiveConnection,this,add_str);
			ReleaseDBO(dboitem);
			return false;
		}
		ReleaseDBO(dboitem);
		return true;
}

bool VS_DBStorage::SetEndpointProperty(const char* app_id, const char* name, const wchar_t* value)
{
AUTO_PROF
	  const VS_Pool::Item* dboitem;
	  VS_DBObjects* dbo=GetDBO(dboitem);
		try
		{
			dprint4("$WSet Endpoint %s Property '%s' \n", app_id, name);

			dbo->ep_prop_set->Parameters->Item[APP_ID_PARAM]->Value = app_id;
			dbo->ep_prop_set->Parameters->Item[PROP_NAME_PARAM]->Value = name;
			dbo->ep_prop_set->Parameters->Item[PROP_VALUE_PARAM]->Value = value;

			dbo->ep_prop_set->Execute(NULL,NULL,ADODB::adCmdStoredProc|ADODB::adExecuteNoRecords);

		}
		catch(_com_error e)
		{
			_bstr_t	add_str(dbo->ep_prop_set->GetCommandText());
			dbo->ProcessCOMError(e,dbo->ep_prop_set->ActiveConnection,this,add_str);
			ReleaseDBO(dboitem);
			return false;
		}
		ReleaseDBO(dboitem);
		return true;
}

bool VS_DBStorage::SetAllEpProperties(const char* app_id, const int prot_version, const short int type, const wchar_t* version,
									const /*char*/wchar_t* app_name, const /*char*/wchar_t* sys_conf, const /*char**/wchar_t* processor, const /*char*/wchar_t *directX,
									const wchar_t* hardwareConfig, const wchar_t* AudioCapture, const wchar_t *VideoCapture, const /*char*/wchar_t* AudioRender, const char* call_id)
{
AUTO_PROF
	const VS_Pool::Item* dboitem;
	VS_DBObjects* dbo=GetDBO(dboitem);
	try
	{
		dprint4("$Set Group Endpoint %s Properties \n", (const char*)app_id);

		dbo->ep_group_prop_set->Parameters->Item[APP_ID_PARAM]->Value						= app_id? app_id: "";
		dbo->ep_group_prop_set->Parameters->Item[EP_PROTOCOL_VERSION_PARAM]->Value			= prot_version;
		dbo->ep_group_prop_set->Parameters->Item[DB_TYPE_PARAM]->Value						= type;
		dbo->ep_group_prop_set->Parameters->Item[EP_VERSION_PARAM]->Value					= version? version: L"";
		dbo->ep_group_prop_set->Parameters->Item[DB_APP_NAME_PARAM]->Value					= app_name? app_name: L"";

		dbo->ep_group_prop_set->Parameters->Item[EP_SYS_CONF_PARAM]->Value					= sys_conf? sys_conf: L"";
		dbo->ep_group_prop_set->Parameters->Item[EP_PROCESSOR_PARAM]->Value					= processor? processor: L"";
		dbo->ep_group_prop_set->Parameters->Item[EP_DIRECTX_PARAM]->Value					= directX? directX: L"";
		dbo->ep_group_prop_set->Parameters->Item[EP_HARDWARE_CONFIG_PARAM]->Value			= hardwareConfig? hardwareConfig: L"";
		dbo->ep_group_prop_set->Parameters->Item[EP_AUDIO_CAPTURE_PARAM]->Value				= AudioCapture? AudioCapture: L"";
		dbo->ep_group_prop_set->Parameters->Item[EP_VIDEO_CAPTURE_PARAM]->Value				= VideoCapture? VideoCapture: L"";
		dbo->ep_group_prop_set->Parameters->Item[EP_AUDIO_RENDER_PARAM]->Value				= AudioRender? AudioRender: L"";
		dbo->ep_group_prop_set->Parameters->Item[CALL_ID_PARAM]->Value						= call_id? call_id: "";

		dbo->ep_group_prop_set->Execute(NULL,NULL,ADODB::adCmdStoredProc|ADODB::adExecuteNoRecords);
	}
	catch(_com_error e)
	{
		_bstr_t	add_str(dbo->ep_group_prop_set->GetCommandText());
		dbo->ProcessCOMError(e,dbo->ep_group_prop_set->ActiveConnection,this,add_str);
		ReleaseDBO(dboitem);
		return false;
	}
	ReleaseDBO(dboitem);
	return true;
}


void VS_DBStorage::MakeConfID(const int conference, const VS_SimpleStr& broker_id, vs_conf_id& conf_id)
{
	if(!conference)
		conf_id.Empty();
	else {
		conf_id.Resize(25);
		sprintf(conf_id,"%08x@%s", conference, (const char*)broker_id);
	}
}

bool VS_DBStorage::GetMissedCallMailTemplate(const std::chrono::system_clock::time_point missed_call_time, const char* fromId, std::string& inOutfromDn, const char* toId, std::string& inOutToDn,
											 VS_SimpleStr &from_email, VS_SimpleStr &to_email, std::string &subj_templ, std::string &body_templ)
{
AUTO_PROF
	const VS_Pool::Item	*dboitem;
	VS_DBObjects *dbo = GetDBO(dboitem);
	try
	{
		///dprint3("$Get Missed Call Mail Template; fromEm %s; fromDn %s; toEm %s; toDn %s\n",fromEm,from
		VS_WideStr fromDN;	fromDN.AssignUTF8(inOutfromDn.c_str());
		VS_WideStr toDN;	toDN.AssignUTF8(inOutToDn.c_str());
		dbo->ab_get_missed_call_mail->Parameters->Item[CALL_ID_FROM]->Value = fromId;
		dbo->ab_get_missed_call_mail->Parameters->Item[DISPLAY_NAME_FROM]->Value = fromDN.m_str;
		dbo->ab_get_missed_call_mail->Parameters->Item[CALL_ID_TO]->Value = toId;
		dbo->ab_get_missed_call_mail->Parameters->Item[DISPLAY_NAME_TO]->Value = toDN.m_str;

		dbo->ab_get_missed_call_mail->Parameters->Item[TIME_MISSED_CALL]->Value = VS_FileTime(missed_call_time);
		dbo->ab_get_missed_call_mail->Parameters->Item[TYPE_MAIL_TEMPLATE]->Value = "OLD";

		ADODB::_RecordsetPtr rs=dbo->ab_get_missed_call_mail->Execute(NULL,NULL,ADODB::adCmdUnspecified);
		if(rs->State==ADODB::adStateClosed || rs->ADOEOF)
		{
			error_code=0;
			///// не найден шаблон
			ReleaseDBO(dboitem);
			return false;
		}

		to_email = vs::StrFromVariantT(rs->Fields->Item[EMAIL]->Value);
		from_email = vs::StrFromVariantT(rs->Fields->Item[EMAIL_FROM]->Value);

		vs::UnicodeConverter<wchar_t, char> converter;
		VS_WideStr wsubj_templ = vs::WStrFromVariantT(rs->Fields->Item[MAIL_SUBJECT_TEMPLATE]->Value);
		if (wsubj_templ) subj_templ = converter.Convert(wsubj_templ.m_str);

		VS_WideStr wbody_templ = vs::WStrFromVariantT(rs->Fields->Item[MAIL_BODY_TEMPLATE]->Value);
		if(wbody_templ) body_templ = converter.Convert(wbody_templ.m_str);
	}
	catch(_com_error e)
	{
		_bstr_t	add_str(dbo->ab_get_missed_call_mail->GetCommandText());
		dbo->ProcessCOMError(e,dbo->ab_get_missed_call_mail->ActiveConnection,this,add_str);
		ReleaseDBO(dboitem);
		return false;
	}
	ReleaseDBO(dboitem);
	return true;
}
bool VS_DBStorage::GetInviteCallMailTemplate(const std::chrono::system_clock::time_point missed_call_time, const char *fromId, std::string& inOutfromDn, const char* toId,
												VS_SimpleStr &from_email, VS_SimpleStr &to_email, std::string &subj_templ, std::string &body_templ)
{
AUTO_PROF
	const VS_Pool::Item	*dboitem;
	VS_DBObjects	*dbo = GetDBO(dboitem);
	try
	{
		dbo->ab_get_missed_call_mail->Parameters->Item[CALL_ID_FROM]->Value = fromId;
		VS_WideStr dn; dn.AssignUTF8(inOutfromDn.c_str());
		dbo->ab_get_missed_call_mail->Parameters->Item[DISPLAY_NAME_FROM]->Value = dn.m_str;
		dbo->ab_get_missed_call_mail->Parameters->Item[CALL_ID_TO]->Value = toId;
		dbo->ab_get_missed_call_mail->Parameters->Item[DISPLAY_NAME_TO]->Value = NULL;
		dbo->ab_get_missed_call_mail->Parameters->Item[TIME_MISSED_CALL]->Value = VS_FileTime(missed_call_time);
		dbo->ab_get_missed_call_mail->Parameters->Item[TYPE_MAIL_TEMPLATE]->Value = "NEW";
		ADODB::_RecordsetPtr rs = dbo->ab_get_missed_call_mail->Execute(NULL,NULL,ADODB::adCmdUnspecified);


		if(rs->State==ADODB::adStateClosed || rs->ADOEOF)
		{
			error_code=0;
			///// не найден шаблон
			ReleaseDBO(dboitem);
			return false;
		}

		to_email = vs::StrFromVariantT(rs->Fields->Item[EMAIL]->Value);
		from_email = vs::StrFromVariantT(rs->Fields->Item[EMAIL_FROM]->Value);

		vs::UnicodeConverter<wchar_t, char> converter;
		VS_WideStr wsubj_templ = vs::WStrFromVariantT(rs->Fields->Item[MAIL_SUBJECT_TEMPLATE]->Value);
		if (wsubj_templ) subj_templ = converter.Convert(wsubj_templ.m_str);

		VS_WideStr wbody_templ = vs::WStrFromVariantT(rs->Fields->Item[MAIL_BODY_TEMPLATE]->Value);
		if (wbody_templ) body_templ = converter.Convert(wbody_templ.m_str);

	}
	catch(_com_error e)
	{
		_bstr_t	add_str(dbo->ab_get_missed_call_mail->GetCommandText());
		dbo->ProcessCOMError(e,dbo->ab_get_missed_call_mail->ActiveConnection,this,add_str);
		ReleaseDBO(dboitem);
		return false;
	}
	ReleaseDBO(dboitem);
	return true;
}

bool VS_DBStorage::GetMissedNamedConfMailTemplate(const char* /*fromId*/, std::string& /*inOutfromDn*/, const char* /*toId*/, std::string& /*inOutToDn*/,
											 VS_SimpleStr &/*from_email*/, VS_SimpleStr &/*to_email*/, std::string &/*subj_templ*/, std::string &/*body_templ*/)
{
//	return GetMissedCallMailTemplate(missed_call_time, fromId, fromDn, toId, toDn, from_email, to_email, subj_templ, body_templ);
	return false;
}

int VS_DBStorage::GetOfflineChatMessages(const char* call_id, std::vector<VS_Container> &vec)
{
AUTO_PROF
	if ( !call_id )
		return false;

	// cache
	{
		VS_AutoLock lock(&m_usersWithOfflineMessages_lock);
		VS_StrIStrMap::Iterator it = m_usersWithOfflineMessages[call_id];
		if (!it)
			return false;
	}

	const int n_max = 100;

	const VS_Pool::Item* dboitem;
	VS_DBObjects* dbo=GetDBO(dboitem);
	try
	{
		dprint4("$GetOfflineChatMessages for %s\n", call_id);

		dbo->notify_login->Parameters->Item[CALL_ID_PARAM]->Value = call_id;

		ADODB::_RecordsetPtr rs = dbo->notify_login->Execute(NULL,NULL,ADODB::adCmdStoredProc);

		//_bstr_t		type;		// we don't need this
		VS_SimpleStr	from;
		VS_WideStr		from_display_name;
		VS_SimpleStr	to;
		VS_WideStr	body;

		while( !rs->ADOEOF && (vec.size() < n_max) )
		{
			ADODB::FieldsPtr f = rs->Fields;

			//type				= f->Item[(short)0]->Value;		// we don't need this
			from				= vs::StrFromVariantT(f->Item[CALL_ID_FROM]->Value);
			from_display_name	= vs::WStrFromVariantT(f->Item[DISPLAY_NAME_FROM]->Value);
			to					= vs::StrFromVariantT(f->Item[CALL_ID_TO]->Value);
			body				= vs::WStrFromVariantT(f->Item[BODY]->Value);

			VS_FileTime		ft;

			try // case TIME_MESSAGE wasn't returned
			{
				ft.FromVariant( f->Item[TIME_MESSAGE]->Value, false);
			}
			catch (_com_error )
			{
				dprint4("TIME_MESSAGE wasn't returned from sp_notify_login %s\n", call_id);
			}

			VS_Container c;

			if (f->Item["container"]->Type == ADODB::adVarBinary ||
				f->Item["container"]->Type == ADODB::adLongVarBinary)
			{
				void HUGEP* bin_val = nullptr;
				_variant_t val(f->Item["container"]->Value);
				SAFEARRAY* sa = val.parray;
				if (!FAILED(SafeArrayAccessData(sa, &bin_val)))
				{
					c.Deserialize(bin_val, sa->rgsabound[0].cElements);
					SafeArrayUnaccessData(sa);
					bin_val = 0;
				}
			}
			else {
				c.AddValue(FILETIME_PARAM, &ft.m_filetime, sizeof(ft.m_filetime));
				c.AddValue(METHOD_PARAM, SENDMESSAGE_METHOD);
				c.AddValue(FROM_PARAM, from);
				c.AddValue(DISPLAYNAME_PARAM, from_display_name);
				c.AddValue(TO_PARAM, to);
				c.AddValue(MESSAGE_PARAM, body);
			}
			vec.push_back(std::move(c));

			rs->MoveNext();
		}

		// clear cache
		{
			VS_AutoLock lock(&m_usersWithOfflineMessages_lock);
			m_usersWithOfflineMessages.Erase(call_id);
		}
	}
	catch(_com_error e)
	{
		_bstr_t	add_str(dbo->notify_login->GetCommandText());
		dbo->ProcessCOMError(e,dbo->notify_login->ActiveConnection,this,add_str);
	}

	ReleaseDBO(dboitem);
	return (int) vec.size();
}

bool VS_DBStorage::SetOfflineChatMessage(const VS_SimpleStr& from_call_id, const VS_SimpleStr& to_call_id, const VS_SimpleStr& body_utf8, const std::string& from_dn, const VS_Container& cnt)
{
AUTO_PROF
	dprint4("$SetOfflineChatMessage from:'%s' to:'%s' body:'%s'\n",
		(const char*)from_call_id, (const char*)to_call_id, (const char*)body_utf8);

	if (!to_call_id || !body_utf8)
		return false;

	if (!IsValidUserSID(SimpleStrToStringView(to_call_id)))
		return false;

	bool result=false;

	VS_WideStr body;
	body.AssignUTF8(body_utf8);

	const VS_Pool::Item* dboitem;
	VS_DBObjects* dbo=GetDBO(dboitem);
	try
	{

		dbo->friend_im_save->Parameters->Item[CALL_ID_FROM]->Value		= from_call_id.m_str;
		dbo->friend_im_save->Parameters->Item[CALL_ID_TO]->Value		= to_call_id.m_str;
		dbo->friend_im_save->Parameters->Item["@i_msg_type"]->Value		= 2;			// пока двойка (договорилис Кост Т. и Анна)
		dbo->friend_im_save->Parameters->Item["@c_subject"]->Value		= "";
		dbo->friend_im_save->Parameters->Item[BODY]->Value				= body.m_str;
		VS_WideStr dn; dn.AssignUTF8(from_dn.c_str());
		dbo->friend_im_save->Parameters->Item[DISPLAY_NAME_FROM]->Value	= dn.m_str;
		void *ptr = nullptr;
		VS_SCOPE_EXIT{ if (ptr) free(ptr); };
		size_t size = 0;
		if (cnt.SerializeAlloc(ptr, size) && size > 0)
		{
			SAFEARRAY sa;
			VARIANT	v2;
			sa.rgsabound[0].cElements = size;
			sa.rgsabound[0].lLbound = 0;
			sa.cbElements = 1;
			sa.cDims = 1;
			sa.fFeatures = 128;
			sa.cLocks = 0;
			sa.pvData = (void*)ptr;

			v2.parray = &sa;
			v2.vt = VT_UI1 | VT_ARRAY;

			_variant_t v3(v2);

			dbo->friend_im_save->Parameters->Item["_container"]->Value = v3;
		}
		else
			dbo->friend_im_save->Parameters->Item["_container"]->Value = db_null;

		dbo->friend_im_save->Execute(NULL,NULL,ADODB::adCmdStoredProc|ADODB::adExecuteNoRecords);

		result=true;
	}
	catch(_com_error e)
	{
		_bstr_t	add_str(dbo->friend_im_save->GetCommandText());
		dbo->ProcessCOMError(e,dbo->friend_im_save->ActiveConnection,this,add_str);
	}

	// cache
	if (result)
	{
		VS_AutoLock lock(&m_usersWithOfflineMessages_lock);
		m_usersWithOfflineMessages.Insert(to_call_id, to_call_id);
	}

	ReleaseDBO(dboitem);
	return result;
}

void VS_DBStorage::GetBSEvents(std::vector<BSEvent> &vec)
{
AUTO_PROF
	const int n_max = 1000;

	const VS_Pool::Item* dboitem;
	VS_DBObjects* dbo=GetDBO(dboitem);
	try
	{
		dprint4("$GetBSEvents\n");

		ADODB::_RecordsetPtr rs;
		if (dbo->IsPostgreSQL) {
			dbo->get_bs_events->CommandText = "select type, call_id, obj_type, obj_id, CAST(value as text) as value, last_server, Hash from sp_get_bs_events(0);";
			rs = dbo->get_bs_events->Execute(NULL,NULL,ADODB::adCmdText);
		}else{
			dbo->get_bs_events->Parameters->Item["@i_cnt"]->Value = 0;
			rs = dbo->get_bs_events->Execute(NULL,NULL,ADODB::adCmdUnspecified);
		}

		while( !rs->ADOEOF && (vec.size() < n_max) )
		{
			ADODB::FieldsPtr f = rs->Fields;

			VS_SimpleStr	type				= vs::StrFromVariantT(f->Item["type"]->Value);
			VS_WideStr		call_id				= vs::WStrFromVariantT(f->Item[CALL_ID]->Value);
			VS_SimpleStr	obj_type			= vs::StrFromVariantT(f->Item["obj_type"]->Value);
			VS_WideStr		obj_id				= vs::WStrFromVariantT(f->Item["obj_id"]->Value);
			VS_WideStr		value				= vs::WStrFromVariantT(f->Item["value"]->Value);
			VS_SimpleStr	broker_id			= vs::StrFromVariantT(f->Item["last_server"]->Value);
			long			hash				= (long) f->Item[HASH_PARAM]->Value;

			if (!value)
			{
				rs->MoveNext();
				continue;
			}
/*
type = "UPD";
obj_type = "TARIFF_CHANGED";
obj_id = "NEW_TARIF";
value = "12345~#54321";

call_id = "kt1@kt.pca.ru";
broker_id = "kt.pca.ru#as";
hash = true;	// fake
*/
			const char* method = 0;
			long ab = AB_NONE;
			bool is_friend = false;

			BSEvent ev;
			ev.cnt = new VS_Container;

			obj_type.ToUpper();
			if (obj_type == "BAN_LIST") {
				ev.cnt->AddValueI32(ADDRESSBOOK_PARAM, AB_BAN_LIST);
				ev.cnt->AddValue(DISPLAYNAME_PARAM, value);
				ev.to_service = ADDRESSBOOK_SRV;

			} else if (obj_type == "ADDRESS_BOOK") {
				ev.cnt->AddValueI32(ADDRESSBOOK_PARAM, AB_COMMON);
				ev.cnt->AddValue(DISPLAYNAME_PARAM, value);
				ev.to_service = ADDRESSBOOK_SRV;

				if (type == "ABU")		// send to BS OurEndpoint (not to AS of user)
				{
					if (!!broker_id)
						ev.cnt->AddValue(SERVER_PARAM, broker_id);		// last_server - AS of user
					broker_id = m_serverID;								// send OnABChange to my BS (need to homeBS of user)
					ev.cnt->AddValue(USERNAME_PARAM, call_id);
				}

			} else if (obj_type == "ADDRESS_BOOK_NEWUSER") {
				ev.cnt->AddValueI32(ADDRESSBOOK_PARAM, AB_COMMON);
				ev.cnt->AddValue(DISPLAYNAME_PARAM, value);
				ev.cnt->AddValue("NewUser", (bool)true);
				ev.to_service = ADDRESSBOOK_SRV;

			} else if (obj_type == "AB_INVERSE") {
				ev.cnt->AddValueI32(ADDRESSBOOK_PARAM, AB_INVERSE);
				ev.cnt->AddValue(DISPLAYNAME_PARAM, value);
				ev.to_service = ADDRESSBOOK_SRV;

			} else if (obj_type == "AB_USER_PICTURE") {
				ev.cnt->AddValueI32(ADDRESSBOOK_PARAM, AB_USER_PICTURE);
				ev.to_service = ADDRESSBOOK_SRV;

				if (type == "UPD"){
					ev.cnt->AddValue(USERNAME_PARAM, call_id);
					ev.cnt->AddValue(METHOD_PARAM, ONADDRESSBOOKCHANGE_METHOD);
					ev.cnt->AddValue(QUERY_PARAM, call_id);

					if (!!broker_id)	ev.cnt->AddValue(SERVER_PARAM, broker_id);		// last_server - AS of user
					else dprint4("$GetBSEvents:AB_USER_PICTURE: AS of user wasn't found\n");
					broker_id = m_serverID;
				}
			} else if (obj_type == "AB_PHONES") {
				if (!broker_id)		// user is offline; skip this bs-event
				{
					if (ev.cnt) { delete ev.cnt; ev.cnt = 0; }
					rs->MoveNext();
					continue;
				}
				ev.cnt->AddValue(METHOD_PARAM, ONADDRESSBOOKCHANGE_METHOD);
				ev.cnt->AddValueI32(ADDRESSBOOK_PARAM, AB_PHONES);
				ev.cnt->AddValue(USERNAME_PARAM, call_id);
				ev.to_service = ADDRESSBOOK_SRV;
				obj_id = call_id;
				ev.cnt->AddValue(SERVER_PARAM, broker_id);		// last_server - AS of user
				broker_id = m_serverID;							// send OnABChange to my BS (need to homeBS of user)

			} else if (obj_type == "DISPLAY_NAME") {
				ev.cnt->AddValue(DISPLAYNAME_PARAM, value);
				ev.to_service = AUTH_SRV;

			} else if (obj_type == "USR_RIGHTS") {
				ev.cnt->AddValueI32(RIGHTS_PARAM, _wtoi(value));
				ev.to_service = AUTH_SRV;

			} else if (obj_type == "SYS_RATING") {
				ev.cnt->AddValueI32(RATING_PARAM, _wtoi(value));
				ev.to_service = AUTH_SRV;
				// todo: add SEPARATIONGROUP_PARAM here

			} else if (obj_type == "WEB_MSG") {
				if (!!call_id)
				{
					std::string call_id_utf8; call_id.ToUTF8(call_id_utf8);
					VS_AutoLock lock(&m_usersWithOfflineMessages_lock);
					m_usersWithOfflineMessages.Insert(call_id_utf8.c_str(), call_id_utf8.c_str());
				}

				ev.broker_id = m_serverID;
				ev.to_service = OFFLINECHAT_SRV;

				ev.cnt->AddValue(METHOD_PARAM, "NewOfflineChat");
				ev.cnt->AddValue(CALLID_PARAM, call_id);

				vec.push_back(ev);

				rs->MoveNext();
				continue;

			} else if (obj_type == "NAMED_CONF") {

				//ev.broker_id = OurEndpoint();
				ev.to_service = CONFERENCE_SRV;

				ev.cnt->AddValue(METHOD_PARAM, INVITATIONUPDATE_METHOD);
				ev.cnt->AddValue(CONFERENCE_PARAM, call_id);

				ev.broker_id = m_serverID;
				//ev.to = call_id;

				vec.push_back(ev);

//				if (ev.cnt) { delete ev.cnt; ev.cnt = 0; }
				rs->MoveNext();
				continue;

			} else if (obj_type == "TARIFF_CHANGED") {
				ev.to_service = AUTH_SRV;
				ev.cnt->AddValue(TARIFNAME_PARAM, obj_id);

				long user_rights(0), tarif_restr(0);
				VS_WideStr wsep; wsep.AssignUTF8(STRING_SEPARATOR.c_str());
				std::wstring wstr = value.m_str;
				VS_ReplaceAll(wstr, wsep.m_str, L"\n");
				std::wstringstream wss;
				wss << wstr;
				wss >> user_rights;
				wss >> tarif_restr;
				ev.cnt->AddValueI32(RIGHTS_PARAM, user_rights);
				ev.cnt->AddValueI32(TARIFRESTR_PARAM, tarif_restr);

				dstream3 << "UpdatePeerCfg_Method for " << call_id.m_str;
				BSEvent ev2;
				ev2.broker_id = m_serverID;
				ev2.to_service = AUTH_SRV;

				ev2.cnt = new VS_Container;
				ev2.cnt->AddValue(METHOD_PARAM, UPDATE_PEERCFG_METHOD);
				ev2.cnt->AddValue(CALLID_PARAM, call_id);
				ev2.cnt->AddValue(SERVER_PARAM, broker_id);

				vec.push_back(ev2);

			} else if (obj_type == "USER") {		// user deleted
				ev.to_service = AUTH_SRV;

				if (!!broker_id)
					ev.cnt->AddValue(SERVER_PARAM, broker_id);
/*
				if (broker_id)
				{
					send to one AS server
				}else{
					send to all AS servers
				}

*/

			}
			else if (obj_type == "SESSION_WAIT"){
				if (STRING_SEPARATOR.empty()) STRING_SEPARATOR = "~#";
				// expect values in 'value' string in order: $6code+string stored by sp_set_session_wait_temp_param
				VS_SimpleStr value1 = vs::StrFromVariantT(f->Item["value"]->Value);
				string_view params_for_login = value1.m_str;

				std::vector<std::string> arr;
				boost::algorithm::iter_split(arr, params_for_login, boost::first_finder(STRING_SEPARATOR));
				if (arr.size() != 2) continue;

				std::string& passwd = arr[0];
				std::string& cnt_data = arr[1];

				size_t len;
				base64_decode(cnt_data.c_str(), cnt_data.length(), nullptr, len);
				auto cnt_buff = vs::make_unique_default_init<unsigned char[]>(len);
				base64_decode(cnt_data.c_str(), cnt_data.length(), cnt_buff.get(), len);

				VS_Container cnt_from_db;
				cnt_from_db.Deserialize(cnt_buff.get(), len);

				ev.to_service = AUTH_SRV;
				broker_id = m_serverID;
				ev.cnt->AddValue(PASSWORD_PARAM, passwd.c_str());
				ev.cnt->AddValueI32(LOGINSTRATEGY_PARAM, eLoginStrategy::LS_FROM_BSEVENT);
				cnt_from_db.AttachToCnt(*ev.cnt);
			} else {
				if (ev.cnt) { delete ev.cnt; ev.cnt = 0; }
				rs->MoveNext();
				continue;
			}

			type.ToUpper();
			if (type == "INS")
				method = ADDTOADDRESSBOOK_METHOD;
			else if (type == "DEL")
				method = REMOVEFROMADDRESSBOOK_METHOD;
			else if (type == "UPD")
				method = UPDATEACCOUNT_METHOD;
			else if (type == "OUT")
				method = ONUSERCHANGE_METHOD;
			else if (type == "ABU")
				method = ONADDRESSBOOKCHANGE_METHOD;
			else {
				if (ev.cnt) { delete ev.cnt; ev.cnt = 0; }
				rs->MoveNext();
				continue;
			}

			if (!method || !call_id || !broker_id || !obj_id || !hash)
			{
				if (ev.cnt) { delete ev.cnt; ev.cnt = 0; }
				rs->MoveNext();
				continue;
			}


			ev.cnt->AddValue(METHOD_PARAM, method);
			ev.cnt->AddValueI32(RESULT_PARAM, 1);		// Always only one
			ev.cnt->AddValueI32(CAUSE_PARAM, 0);		// No Error
			ev.cnt->AddValueI32(HASH_PARAM, hash);
			ev.cnt->AddValue(CALLID_PARAM, obj_id);			// add_call_id


			// go to AS
			ev.cnt->AddValue(TRANSPORT_SRCUSER_PARAM, call_id);

			ev.broker_id = broker_id;
			ev.to = call_id.ToUTF8();

			vec.push_back(ev);

			rs->MoveNext();
		}
	}
	catch(_com_error e)
	{
		_bstr_t	add_str(dbo->get_bs_events->GetCommandText());
		dbo->ProcessCOMError(e,dbo->get_bs_events->ActiveConnection,this,add_str);
	}

	ReleaseDBO(dboitem);
	return ;
}
void VS_DBStorage::GetUpdatedExtStatuses(std::map<std::string,VS_ExtendedStatusStorage>& out)
{
AUTO_PROF
	const VS_Pool::Item* dboitem;
	VS_DBObjects* dbo = GetDBO(dboitem);
	VS_SCOPE_EXIT{ReleaseDBO(dboitem);};
	std::set<std::string> call_idx_with_upd_ext_st;
	try
	{
		dprint4("$GetUpdatedExtStatuses\n");
		dbo->get_bs_events->CommandText = "select * from sp_get_bs_events(0,'bs');";
		auto rs = dbo->get_bs_events->Execute(NULL, NULL, ADODB::adCmdText);
		while (!rs->ADOEOF)
		{
			call_idx_with_upd_ext_st.insert(static_cast<const char*>(static_cast<_bstr_t>(rs->Fields->Item[CALL_ID]->Value)));
			rs->MoveNext();
		}
	}
	catch (const _com_error &e)
	{
		_bstr_t	add_str(dbo->get_bs_events->GetCommandText());
		dbo->ProcessCOMError(e, dbo->get_bs_events->ActiveConnection, this, add_str);
	}
	for (const auto &i : call_idx_with_upd_ext_st)
		GetExtendedStatus(dbo, i.c_str(), out[i]);
}
int VS_DBStorage::GetNamedConfInfo(const char* conf_id, VS_ConferenceDescription& cd, ConferenceInvitation& ci, VS_SimpleStr& as_server, long& scope)
{
AUTO_PROF
	const VS_Pool::Item* dboitem;
	VS_DBObjects* dbo=GetDBO(dboitem);
	bool b = dbo->IsPostgreSQL;
	try
	{
		dprint4("$GetNamedConfInfo\n");

		std::string cmd(dbo->get_named_conf_info_template);
		VS_ReplaceAll(cmd, "%conf_id%", conf_id);
		dbo->get_named_conf_info->CommandText = cmd.c_str();

		ADODB::_RecordsetPtr rs;
		rs=dbo->get_named_conf_info->Execute(NULL,NULL,ADODB::adCmdText);

		if(rs->ADOEOF)
			throw VSS_CONF_NOT_FOUND;

		cd.m_type				= rs->Fields->Item[GetParam(DB_TYPE_PARAM,b)]->Value;
		cd.m_SubType			= rs->Fields->Item[GetParam(DB_SUBTYPE_PARAM,b)]->Value;
		cd.m_MaxParticipants	= rs->Fields->Item[GetParam(DB_MAX_PARTICIPANTS_PARAM,b)]->Value;
		cd.m_MaxCast			= rs->Fields->Item[GetParam(DB_MAX_CAST_PARAM,b)]->Value;
		scope					= rs->Fields->Item[GetParam(DB_IS_PUBLIC_PARAM,b)]->Value;
		cd.m_owner				= vs::StrFromVariantT(rs->Fields->Item[GetParam(DB_OWNER_PARAM,b)]->Value);
		ci.m_owner				= cd.m_owner;
		cd.m_topic				= vs::WideCharToUTF8Convert((wchar_t*)(_bstr_t)rs->Fields->Item[GetParam(DB_TOPIC_PARAM,b)]->Value);
		ci.m_topic				= cd.m_topic;
		as_server				= vs::StrFromVariantT(rs->Fields->Item[GetParam(DB_SERVER_PARAM,b)]->Value);

		// invite
		ci.m_invitation_time	= rs->Fields->Item[GetParam(DB_INVITE_TIME_PARAM,b)]->Value;
		ci.m_invitation_day		= rs->Fields->Item[GetParam(DB_INVITE_DAYS_PARAM,b)]->Value;
		ci.m_email_minutes		= rs->Fields->Item[GetParam(DB_INVITE_EMAIL_PARAM,b)]->Value;
		VS_FileTime db_time;	db_time.FromVariant_NoTZ(rs->Fields->Item[DB_TIME]->Value);
		VS_FileTime start_time;	start_time.FromVariant_NoTZ(rs->Fields->Item[DB_START_TIME]->Value);
		VS_FileTime end_time;	end_time.FromVariant_NoTZ(rs->Fields->Item[DB_END_TIME]->Value);

		ci.m_invitation_start_time = start_time.chrono_system_clock_time_point();

		cd.m_name = conf_id;
		cd.m_timeExp = static_cast<std::chrono::system_clock::time_point>(end_time);
		error_code=0;

		VS_SimpleStr conf_status = vs::StrFromVariantT(rs->Fields->Item[DB_CONF_STATUS]->Value);
		if (!conf_status || ((conf_status != "trial") && (conf_status != "on")))
			throw VSS_CONF_NOT_STARTED;
		if (db_time < start_time)
			throw VSS_CONF_NOT_STARTED;
		if (db_time > end_time)
			throw VSS_CONF_EXPIRED;
		if (conf_id)
			ci.m_conf_id = conf_id;
	}
	catch(_com_error e)
	{
		_bstr_t	add_str(dbo->get_named_conf_info->GetCommandText());
		dbo->ProcessCOMError(e,dbo->get_named_conf_info->ActiveConnection,this,add_str);
	}
	catch(int e)
	{
		error_code = e;
		dprint3("$NamedConf '%s' failed. Error=%d\n", conf_id, error_code);
	}
	ReleaseDBO(dboitem);
	auto ds = dstream4;
	ds << "GetNamedConfInfo: " << cd << ",as=";
	if (!as_server.IsEmpty())
		ds << as_server.m_str;
	ds << ",ci.m_invitation_start_time=" << std::chrono::duration_cast<std::chrono::seconds>(ci.m_invitation_start_time.time_since_epoch()).count();
	return error_code;
}

void VS_DBStorage::SetNamedConfServer(const char* named_conf_id, const char* stream_conf_id)
{
AUTO_PROF
	const VS_Pool::Item* dboitem;
	VS_DBObjects* dbo=GetDBO(dboitem);
	try
	{
		dprint4("$SetNamedConfServer: %s, %s\n", named_conf_id, stream_conf_id);

		int conference;VS_SimpleStr broker_id;
		dbo->SplitConfID(stream_conf_id,conference,broker_id);

		dbo->set_named_conf_server->Parameters->Item[CALL_ID_PARAM]->Value=named_conf_id;
		dbo->set_named_conf_server->Parameters->Item["@conf_id"]->Value=conference;
		dbo->set_named_conf_server->Parameters->Item[DB_SERVER_PARAM]->Value = broker_id.m_str;

		dbo->set_named_conf_server->Execute(NULL,NULL,ADODB::adCmdStoredProc|ADODB::adExecuteNoRecords);
	}
	catch(_com_error e)
	{
		_bstr_t	add_str(dbo->set_named_conf_server->GetCommandText());
		dbo->ProcessCOMError(e,dbo->set_named_conf_server->ActiveConnection,this,add_str);
	}
	ReleaseDBO(dboitem);
}

int VS_DBStorage::GetNamedConfParticipants(const char* conf_id, ConferenceInvitation& ci)
{
AUTO_PROF
	const VS_Pool::Item* dboitem;
	VS_DBObjects* dbo=GetDBO(dboitem);
	try
	{
		dprint4("$GetNamedConfParticipants\n");

		std::string cmd(dbo->get_named_conf_participants_template);
		VS_ReplaceAll(cmd, "%conf_id%", conf_id);
		dbo->get_named_conf_participants->CommandText = cmd.c_str();

		ADODB::_RecordsetPtr rs=dbo->get_named_conf_participants->Execute(NULL,NULL,ADODB::adCmdText);

		while( !rs->ADOEOF )
		{
			VS_SimpleStr user = vs::StrFromVariantT(rs->Fields->Item[CALL_ID_PARAM]->Value);
			ci.m_invitation_parts[VS_RealUserLogin(user.m_str)] = make_pair(std::string(""), std::string(""));

			rs->MoveNext();
		}

		error_code = 0;
	}
	catch(_com_error e)
	{
		_bstr_t	add_str(dbo->get_named_conf_participants->GetCommandText());
		dbo->ProcessCOMError(e,dbo->get_named_conf_participants->ActiveConnection,this,add_str);
	}
	catch(int e)
	{
		error_code = e;
		dprint3("$GetNamedConfParticipants '%s' failed. Error=%d\n", conf_id, error_code);
	}
	ReleaseDBO(dboitem);

	auto ds = dstream4;
	ds << "ci.m_invitation_parts=";
	for (auto const& p : ci.m_invitation_parts)
		ds << p.first << ",";

	return error_code;
}

int VS_DBStorage::InitNamedConfInvitaions(std::vector<std::string> &v/*const char* conf_id, ConferenceInvitation& ci*/)
{
AUTO_PROF
	const VS_Pool::Item* dboitem;
	VS_DBObjects* dbo=GetDBO(dboitem);

	if (!dbo->IsPostgreSQL)
	{
		ReleaseDBO(dboitem);
		return 0;
	}

	try
	{
		dprint4("$InitNamedConfInvitaions\n");

		dbo->init_named_conf_invitations->CommandText = dbo->init_named_conf_invitations_template.c_str();

		ADODB::_RecordsetPtr rs=dbo->init_named_conf_invitations->Execute(NULL,NULL,ADODB::adCmdText);

		if(rs->ADOEOF)
			throw VSS_CONF_NOT_FOUND;

		VS_SimpleStr conf_id;
		while( !rs->ADOEOF )
		{
			ADODB::FieldsPtr f = rs->Fields;

			conf_id = vs::StrFromVariantT(f->Item["@conf_id"]->Value);
			v.push_back((std::string) conf_id);
//			ci.m_invitation_parts.Insert(user,"");

			rs->MoveNext();
		}

		error_code = 0;
	}
	catch(_com_error e)
	{
		_bstr_t	add_str(dbo->get_named_conf_participants->GetCommandText());
		dbo->ProcessCOMError(e,dbo->get_named_conf_participants->ActiveConnection,this,add_str);
	}
	catch(int e)
	{
		error_code = e;
		dprint3("$InitNamedConfInvitaions failed. Error=%d\n", error_code);
	}

	ReleaseDBO(dboitem);
	return error_code;
}

const char* VS_DBStorage::GetParam(const char* param, bool IsPosgresSQL)
{
	return (IsPosgresSQL && param && *param && *(param+1))? param + 1: param;
}

VS_SimpleStr VS_DBStorage::GetLoginSessionSecret()
{
AUTO_PROF
	const VS_Pool::Item* dboitem;
	VS_DBObjects* dbo=GetDBO(dboitem);

	if (!dbo->IsPostgreSQL)
	{
		ReleaseDBO(dboitem);
		return 0;
	}

	VS_SimpleStr secret;

	try
	{
		dprint4("$GetLoginSessionSecret\n");

		ADODB::_RecordsetPtr rs=dbo->get_login_session_secret->Execute(NULL,NULL,ADODB::adCmdStoredProc);

		if(!rs->ADOEOF)
		{
			secret = vs::StrFromVariantT(rs->Fields->Item["secret"]->Value);
		}
	}
	catch(_com_error e)
	{
		_bstr_t	add_str(dbo->get_login_session_secret->GetCommandText());
		dbo->ProcessCOMError(e,dbo->get_login_session_secret->ActiveConnection,this,add_str);
	}

	ReleaseDBO(dboitem);
	return secret;
}

int VS_DBStorage::GetPropertiesList(const char* call_id, const char* appName, VS_Container& prop_cnt)
{
AUTO_PROF
	const VS_Pool::Item* dboitem;
	VS_DBObjects* dbo=GetDBO(dboitem);
	try
	{
		dprint3("$GetPropertiesList\n");

		std::string cmd(dbo->get_properties_list_template);
		VS_ReplaceAll(cmd, "%call_id%", call_id ? call_id : "");
		VS_ReplaceAll(cmd, "%app_name%", appName ? appName : "");
		dbo->get_properties_list->CommandText = cmd.c_str();

		ADODB::_RecordsetPtr rs = dbo->get_properties_list->Execute(NULL,NULL,ADODB::adCmdText);

		while( !rs->ADOEOF )
		{
			VS_SimpleStr name = vs::StrFromVariantT(rs->Fields->Item[PROP_NAME_PARAM]->Value);
			VS_WideStr value = vs::WStrFromVariantT(rs->Fields->Item[PROP_VALUE_PARAM]->Value);

			//dprint4("PropertyList: name=%s, ", name); dprint3("value=%S\n", value);
			if (name.m_str)
				prop_cnt.AddValue(name.m_str, value);

			rs->MoveNext();
		}

		{	VS_AutoLock _(&m_additional_app_properties_lock);
			for (auto &p : m_additional_app_properties) {
				if (p.first.m_str && prop_cnt.GetStrValueRef(p.first.m_str) == NULL)
					prop_cnt.AddValue(p.first.m_str, p.second);
			}
		}

		error_code = 0;
	}
	catch(_com_error e)
	{
		_bstr_t	add_str(dbo->get_properties_list->GetCommandText());
		dbo->ProcessCOMError(e,dbo->get_properties_list->ActiveConnection,this,add_str);
	}
	catch(int e)
	{
		error_code = e;
		dprint3("$GetPropertiesList '%s' failed. Error=%d\n", call_id, error_code);
	}

	ReleaseDBO(dboitem);
	return error_code;
}

int VS_DBStorage::SearchUsers(VS_Container& rCnt, VS_Container& cnt, const vs_user_id& owner)
{
	int users(0);
	const char* call_id = cnt.GetStrValueRef(CALLID_PARAM);
	const char* email = cnt.GetStrValueRef(EMAIL_PARAM);
	const char* name = cnt.GetStrValueRef(NAME_PARAM);

	const char* str = 0;
	if (name && *name)
		str = name;
	else if (email && *email)
		str = email;
	else if (call_id && *call_id)
		str = call_id;
	else
		return users;

AUTO_PROF
	const VS_Pool::Item* dboitem;
	VS_DBObjects* dbo=GetDBO(dboitem);
	if (!dbo->IsPostgreSQL)
	{
		ReleaseDBO(dboitem);
		return users;
	}

	try
	{
		dprint4("$SearchUsers\n");

		VS_WideStr wstr;	wstr.AssignUTF8(str).Trim();
		ADODB::ParametersPtr p=dbo->search_users->Parameters;
		p->Item["@search_str"]->Value = (!!wstr)? (_variant_t)wstr: db_null;
		p->Item["@ab_call_id"]->Value = (!!owner)? (_variant_t)owner: db_null;

		ADODB::_RecordsetPtr rs;
		try {
			rs = dbo->search_users->Execute(NULL,NULL,ADODB::adCmdStoredProc);
		} catch(_com_error e) {
			ADODB::_ConnectionPtr cur_db = dbo->search_users->ActiveConnection;
			ADODB::ErrorPtr ae= cur_db->Errors->GetItem((short)0);
			if (_stricmp("S1000",(LPCSTR)ae->SQLState)!=0)	// other than "SC_fetch to get a Procedure return failed"
				throw e;
		}

		while( rs!=0 && !rs->ADOEOF )
		{
			rCnt.AddValue(USERNAME_PARAM, (wchar_t*)(_bstr_t) rs->Fields->Item[CALL_ID]->Value);
			rCnt.AddValue(CALLID_PARAM, (wchar_t*)(_bstr_t) rs->Fields->Item[CALL_ID]->Value);
			rCnt.AddValue(DISPLAYNAME_PARAM, (wchar_t*)(_bstr_t) rs->Fields->Item[DB_DISPLAYNAME]->Value);
			users++;
			rs->MoveNext();
		}

		error_code = 0;
	}
	catch(_com_error e)
	{
		_bstr_t	add_str(dbo->search_users->GetCommandText());
		dbo->ProcessCOMError(e,dbo->search_users->ActiveConnection,this,add_str);
	}
	catch(int e)
	{
		error_code = e;
		dprint3("$SearchUsers '%s' failed. Error=%d\n", call_id, error_code);
	}

	rCnt.AddValue(REQUEST_CALL_ID_PARAM, call_id);
	rCnt.AddValue(REQUEST_EMAIL_PARAM, email);
	rCnt.AddValue(REQUEST_NAME_PARAM, name);

	ReleaseDBO(dboitem);
	return users;
}

long VS_DBStorage::ChangePassword(const char* call_id, const char* old_pass, const char* new_pass, const VS_SimpleStr& from_app_id)
{
AUTO_PROF
	if (!call_id||!*call_id||
		!old_pass||!*old_pass||
		!new_pass||!*new_pass)
		return -1;
	long result(-1);
	const VS_Pool::Item* dboitem;
	VS_DBObjects* dbo=GetDBO(dboitem);
	try
	{
		dprint2("$ChangePassword call_id %s old(%s) new(%s)\n", call_id, old_pass, new_pass);

		ADODB::ParametersPtr p=dbo->change_password->Parameters;
		p->Item["@c_call_id"]->Value = call_id;
		p->Item["c_old_password"]->Value = old_pass;
		p->Item["c_new_password"]->Value = new_pass;
		if (from_app_id.Length())
			p->Item["@app_id"]->Value = from_app_id.m_str;
		else
			p->Item["@app_id"]->Value = db_null;

		ADODB::_RecordsetPtr rs = dbo->change_password->Execute(0,0,ADODB::adExecuteNoRecords);

		result = p->Item[(short)0]->Value;
	}
	catch(_com_error e)
	{
		_bstr_t	add_str(dbo->change_password->GetCommandText());
		dbo->ProcessCOMError(e,dbo->change_password->ActiveConnection,this,add_str);
		ReleaseDBO(dboitem);
		return false;
	}
	ReleaseDBO(dboitem);
	return result;
}

long VS_DBStorage::UpdatePerson(const char* call_id, VS_Container& cnt)
{
AUTO_PROF
	if (!call_id||!*call_id)
		return -1;
	long result(-1);
	const VS_Pool::Item* dboitem;
	VS_DBObjects* dbo=GetDBO(dboitem);
	try
	{
		dprint2("$UpdatePerson call_id %s\n", call_id);

		ADODB::ParametersPtr p=dbo->update_person->Parameters;
		p->Item[CALL_ID_PARAM]->Value = call_id;

		VS_WideStr wstr;
		wstr.AssignUTF8(cnt.GetStrValueRef(DISPLAYNAME_PARAM));
		p->Item["@display_name"]->Value = (wstr.m_str!=NULL)? (_variant_t)wstr: db_null;

		wstr.AssignUTF8(cnt.GetStrValueRef(FIRSTNAME_PARAM));
		p->Item["@first_name"]->Value = (wstr.m_str != NULL) ? (_variant_t)wstr : db_null;

		wstr.AssignUTF8(cnt.GetStrValueRef(LASTNAME_PARAM));
		p->Item["@last_name"]->Value = (wstr.m_str != NULL) ? (_variant_t)wstr : db_null;

		wstr.AssignUTF8(cnt.GetStrValueRef(USERCOMPANY_PARAM));
		p->Item["@company"]->Value = (wstr.m_str != NULL) ? (_variant_t)wstr : db_null;

		ADODB::_RecordsetPtr rs = dbo->update_person->Execute(0,0,ADODB::adExecuteNoRecords);

		result = p->Item[(short)0]->Value;
	}
	catch(_com_error e)
	{
		_bstr_t	add_str(dbo->update_person->GetCommandText());
		dbo->ProcessCOMError(e,dbo->update_person->ActiveConnection,this,add_str);
		ReleaseDBO(dboitem);
		return false;
	}
	ReleaseDBO(dboitem);
	return result;
}

long VS_DBStorage::SetRegID(const char* call_id, const char* reg_id, VS_RegID_Register_Type reg_type)
{
AUTO_PROF
	long result(-1);
	const VS_Pool::Item* dboitem;
	VS_DBObjects* dbo=GetDBO(dboitem);
	try
	{
		dprint2("$SetRegID %s, %s\n", call_id, reg_id);

		ADODB::ParametersPtr p=dbo->sp_ntf_update_reg_id->Parameters;
		p->Item["@c_call_id"]->Value = (call_id&&*call_id)? call_id: db_null;
		p->Item["@c_reg_id"]->Value = (reg_id&&*reg_id)? reg_id: db_null;
		p->Item["@b_flag"]->Value = (reg_type==REGID_REGISTER)? true: false;

		ADODB::_RecordsetPtr rs;
		try {
			rs = dbo->sp_ntf_update_reg_id->Execute(0,0,ADODB::adCmdUnspecified);
		} catch(_com_error e) {
			ADODB::_ConnectionPtr cur_db = dbo->sp_ntf_update_reg_id->ActiveConnection;
			ADODB::ErrorPtr ae= cur_db->Errors->GetItem((short)0);
			if (_stricmp("S1000",(LPCSTR)ae->SQLState)!=0)	// other than "SC_fetch to get a Procedure return failed"
				throw e;
		}
		result = p->Item[(short)0]->Value;
	}
	catch(_com_error e)
	{
		_bstr_t	add_str(dbo->sp_ntf_update_reg_id->GetCommandText());
		dbo->ProcessCOMError(e,dbo->sp_ntf_update_reg_id->ActiveConnection,this,add_str);
		ReleaseDBO(dboitem);
		return false;
	}
	ReleaseDBO(dboitem);
	return result;
}


long VS_DBStorage::NotifyWeb_MissedCall(VS_Container& cnt)
{
AUTO_PROF
	long result(-1);
	const VS_Pool::Item* dboitem;
	VS_DBObjects* dbo=GetDBO(dboitem);
	try
	{
		dprint2("$NotifyWeb_MissedCall\n");
		ADODB::ParametersPtr p=dbo->sp_ntf_web_log_insert->Parameters;

		const char* call_id_from = cnt.GetStrValueRef(CALLID_PARAM);
		p->Item["@c_call_id_from"]->Value = db_null;
		if (call_id_from&&*call_id_from)
			p->Item["@c_call_id_from"]->Value = call_id_from;

		VS_WideStr fromDn;		fromDn.AssignUTF8(cnt.GetStrValueRef(DISPLAYNAME_PARAM));
		p->Item["@c_display_name_from"]->Value = db_null;
		if (!!fromDn)
			p->Item["@c_display_name_from"]->Value = fromDn.m_str;

		const char* toId = cnt.GetStrValueRef(CALLID2_PARAM);
		p->Item["@c_call_id_to"]->Value = db_null;
		if (toId&&*toId)
			p->Item["@c_call_id_to"]->Value = toId;

		VS_FileTime missed_call_time;
		size_t size = 0;
		const void * time = cnt.GetBinValueRef(TIME_PARAM, size);
		if (time && (size==sizeof(FILETIME)))
			memcpy(&missed_call_time, time, size);
		p->Item["@d_time_stamp"]->Value = missed_call_time;

		const char* app_name = cnt.GetStrValueRef("app_name");
		const char *conf = cnt.GetStrValueRef(CONFERENCE_PARAM);

		p->Item["@c_value"]->Value = db_null;

		if (!conf || !*conf) {
			p->Item["@c_type"]->Value = "call";
			if (app_name && *app_name)
				p->Item["@c_value"]->Value = app_name;
		}
		else {
			p->Item["@c_type"]->Value = "conf_invite";

			const char *cid = cnt.GetStrValueRef(NAME_PARAM);
			const char *topic = cnt.GetStrValueRef(TOPIC_PARAM);
			json::Object obj;
			obj["stream_id"] = json::String(conf);
			if (cid) obj["cid"] = json::String(cid);
			if (topic) obj["topic"] = json::String(topic);
			if (app_name) obj["app_name"] = json::String(app_name);

			std::stringstream jstream;
			json::Writer::Write(obj, jstream);
			VS_WideStr tows;
			tows.AssignUTF8(jstream.str().c_str());
			p->Item["@c_value"]->Value = tows.m_str;
		}

		ADODB::_RecordsetPtr rs = dbo->sp_ntf_web_log_insert->Execute(0,0,ADODB::adExecuteNoRecords);
		result = p->Item[(short)0]->Value;
	}
	catch(_com_error e)
	{
		_bstr_t	add_str(dbo->sp_ntf_web_log_insert->GetCommandText());
		dbo->ProcessCOMError(e,dbo->sp_ntf_web_log_insert->ActiveConnection,this,add_str);
		ReleaseDBO(dboitem);
		return false;
	}
	ReleaseDBO(dboitem);
	return result;
}

#endif