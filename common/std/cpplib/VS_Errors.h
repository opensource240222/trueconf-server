/**
 **************************************************************************
 * \file VS_Errors.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Main idea of this file is to keep all errors in one place
 * (so it is enough to delete one files to make error-free server)
 *
 * \b Project Standart Libraries
 * \author StasS
 * \date 05.09.03
 *
 * $Revision: 2 $
 *
 * $History: VS_Errors.h $
 *
 * *****************  Version 2  *****************
 * User: Mushakov     Date: 7.08.09    Time: 15:14
 * Updated in $/VSNA/std/cpplib
 * - SBSv3 added by Matvey (SBSv3_m)
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/std/cpplib
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/std/cpplib
 *
 * *****************  Version 17  *****************
 * User: Stass        Date: 30.03.06   Time: 17:12
 * Updated in $/VS/std/cpplib
 * added switchable AB to reg storage
 *
 * *****************  Version 16  *****************
 * User: Stass        Date: 6.07.05    Time: 14:29
 * Updated in $/VS/std/cpplib
 * moved MC storage to separate classs
 *
 * *****************  Version 15  *****************
 * User: Smirnov      Date: 30.12.04   Time: 14:12
 * Updated in $/VS/std/cpplib
 * files headers
 *
 * *****************  Version 14  *****************
 * User: Smirnov      Date: 29.09.04   Time: 14:24
 * Updated in $/VS/std/cpplib
 * pragma_once removed
 *
 * *****************  Version 13  *****************
 * User: Stass        Date: 19.08.04   Time: 19:51
 * Updated in $/VS/std/cpplib
 * LDAP storage
 *
 * *****************  Version 12  *****************
 * User: Stass        Date: 6.04.04    Time: 19:39
 * Updated in $/VS/std/cpplib
 * Insert to MConf
 *
 * *****************  Version 11  *****************
 * User: Smirnov      Date: 22.12.03   Time: 16:40
 * Updated in $/VS/std/cpplib
 * license limits reported to client
 *
 * *****************  Version 10  *****************
 * User: Stass        Date: 20.12.03   Time: 14:22
 * Updated in $/VS/std/cpplib
 * added new license fields and ID control
 *
 * *****************  Version 9  *****************
 * User: Stass        Date: 25.11.03   Time: 18:59
 * Updated in $/VS/std/cpplib
 * added licensing
 *
 * *****************  Version 8  *****************
 * User: Stass        Date: 24.11.03   Time: 13:07
 * Updated in $/VS/std/cpplib
 * RC license support
 *
 * *****************  Version 7  *****************
 * User: Stass        Date: 18.11.03   Time: 17:06
 * Updated in $/VS/std/cpplib
 * added first version licensing
 *
 * *****************  Version 6  *****************
 * User: Smirnov      Date: 5.11.03    Time: 18:45
 * Updated in $/VS/std/cpplib
 * butifull code
 *
 * *****************  Version 5  *****************
 * User: Smirnov      Date: 24.10.03   Time: 17:39
 * Updated in $/VS/std/cpplib
 * MULTI test client and other files
 *
 * *****************  Version 4  *****************
 * User: Smirnov      Date: 12.10.03   Time: 16:18
 * Updated in $/VS/std/cpplib
 * adress bok corrected for id = email
*
* *****************  Version 3  *****************
* User: Smirnov      Date: 9.10.03    Time: 19:44
* Updated in $/VS/std/cpplib
* new methods in client
* new files in std...
*
* *****************  Version 2  *****************
* User: Stass        Date: 7.10.03    Time: 19:21
* Updated in $/VS/std/cpplib
* fixed C++ id syntax
*
* *****************  Version 1  *****************
* User: Stass        Date: 5.09.03    Time: 14:40
* Created in $/VS/std/cpplib
* Moved types out of BrokerServices project
*
* *****************  Version 1  *****************
* User: Slavetsky    Date: 9/04/03    Time: 4:38p
* Created in $/VS/Servers/MediaBroker/BrokerServices
*
* *****************  Version 8  *****************
* User: Stass        Date: 25.07.03   Time: 14:27
* Updated in $/VS/Servers/ServerServices
* printf -> cprintf
*
* *****************  Version 7  *****************
* User: Stass        Date: 25.07.03   Time: 13:13
* Updated in $/VS/Servers/ServerServices
* added debug print 0, currently printing to ASC log
*
* *****************  Version 6  *****************
* User: Stass        Date: 10.07.03   Time: 20:17
* Updated in $/VS/Servers/ServerServices
* configuration dependant define
*
* *****************  Version 5  *****************
* User: Stass        Date: 10.07.03   Time: 19:13
* Updated in $/VS/Servers/ServerServices
* added DEBUG_LEVEL define
*
* *****************  Version 4  *****************
* User: Stass        Date: 10.07.03   Time: 13:47
* Updated in $/VS/Servers/ServerServices
* errror codes cleanup
*
* *****************  Version 3  *****************
* User: Smirnov      Date: 3.07.03    Time: 14:41
* Updated in $/VS/Servers/ServerServices
* debug print warning level = 2
*
* *****************  Version 2  *****************
* User: Stass        Date: 20.06.03   Time: 13:12
* Updated in $/VS/servers/serverservices
* added endpoint key support, added first broker config functions
*
* *****************  Version 1  *****************
* User: Stass        Date: 30.05.03   Time: 12:50
* Created in $/VS/Servers/ServerServices
* moved storage errors to separate file
****************************************************************************/
#ifndef VS_ERRORS_H
#define VS_ERRORS_H

//@{ \name users errors
const int VSS_USER_NOT_FOUND       =201;
const int VSS_USER_NOT_VALID       =204;
const int VSS_USER_EXISTS          =205;
const int VSS_CANT_MAKE_USER_ID    =209;
const int VSS_USER_ACCESS_DENIED   =210;
//@}

//@{ \name endpoints errors
const int VSS_EP_ID_NOT_FOUND      =301;
const int VSS_CANT_MAKE_EP_ID      =302;
const int VSS_EP_NOT_VALID         =303;
const int VSS_EP_NOT_FOUND         =304;
const int VSS_EP_ACCESS_DENIED     =310;
//@}

//@{ \name conferences erorrs
const int VSS_CONF_NOT_FOUND       =401;
const int VSS_CANT_MAKE_CONF_ID    =402;
const int VSS_CONF_NOT_VALID       =403;
const int VSS_CONF_ACCESS_DENIED   =405;
const int VSS_CONF_NO_MONEY        =406;
const int VSS_CONF_MAX_PART_NUMBER =407;
const int VSS_CONF_NOT_STARTED     =408;
const int VSS_CONF_EXPIRED         =409;
const int VSS_CONF_ROUTER_DENIED   =410;
const int VSS_CONF_LIC_LIMITED     =411;
const int VSS_CONF_EXISTS          =412;
//@}

/// problems with registry load
const int VSS_REGISTRY_ERROR       =1001;

//@{ \name db storage specific error codes
const int VSS_DB_ERROR             =1101;
const int VSS_DB_COM_ERROR         =1102;
const int VSS_DB_ADO_ERROR         =1103;
const int VSS_DB_MEM_ERROR         =1104;
//@}

/// inernal server error
const int VSS_BROKER_LOGIC_ERROR   =2001;

//@{ \name files errors
const int VSS_FILE_READ_ERROR      =101;
const int VSS_FILE_WRITE_ERROR     =102;
//@}

//@{ \name licenses erorrs
const int VSS_LICENSE_NOT_VALID    =3001;
const int VSS_LICENSE_TYPE_UNKNOWN =3002;
const int VSS_LICENSE_VERIFY_ERROR =3003;
const int VSS_LICENSE_DUPLICATE    =3004;
const int VSS_LICENSE_KEY_EMTY	   =3005;
const int VSS_LICENSE_KEY_FAILED   =3006;
const int VSS_LICENSE_NO_HWKEY     =3007;
//@}

//@{ \name ldap storage specific error codes
const int VSS_DNS_ERROR            		=1201;
const int VSS_LDAP_INIT_ERROR      		=1202;
const int VSS_LDAP_ERROR           		=1203;
const int VSS_LDAP_CONNECT_SERVER_ERROR	=1204;
//@}

//@{ \name multiconference storage specific error codes
const int VSS_MC_STORAGE_ERROR     =1301;
//@}

//@{ \name addressbook storage specific error codes
const int VSS_AB_STORAGE_ERROR     =1401;
//@}

#endif
