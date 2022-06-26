/**
 **************************************************************************
 * \file VS_RegServer.h
 * (c) 2002-2004 Visicron Inc.  http://www.visicron.net/
 * \brief Contain Direcory (Registration) Server endpoint connect parameters
 *
 * \b Project Standart Libraries
 * \author StasS
 * \date 05.01.04
 *
 * $Revision: 10 $
 *
 * $History: VS_RegServer.h $
 *
 * *****************  Version 10  *****************
 * User: Mushakov     Date: 23.05.11   Time: 14:56
 * Updated in $/VSNA/std
 *
 * *****************  Version 9  *****************
 * User: Mushakov     Date: 6.05.11    Time: 20:43
 * Updated in $/VSNA/std
 *  - new reg; new reg cert; cert chain supported in tc_server
 *
 * *****************  Version 8  *****************
 * User: Mushakov     Date: 5.05.11    Time: 20:01
 * Updated in $/VSNA/std
 *
 * *****************  Version 7  *****************
 * User: Mushakov     Date: 5.05.11    Time: 19:54
 * Updated in $/VSNA/std
 * - rollback
 *
 * *****************  Version 6  *****************
 * User: Mushakov     Date: 5.05.11    Time: 17:53
 * Updated in $/VSNA/std
 * - new reg, new certs
 *
 * *****************  Version 5  *****************
 * User: Mushakov     Date: 4.11.09    Time: 20:07
 * Updated in $/VSNA/std
 * - quick update license
 * - #regs suffix added
 *
 * *****************  Version 4  *****************
 * User: Mushakov     Date: 23.10.09   Time: 15:05
 * Updated in $/VSNA/std
 *  - VCS 3
 *
 * *****************  Version 3  *****************
 * User: Mushakov     Date: 19.08.09   Time: 14:17
 * Updated in $/VSNA/std
 * - removed links to vp64
 *
 * *****************  Version 2  *****************
 * User: Mushakov     Date: 7.08.09    Time: 15:14
 * Updated in $/VSNA/std
 * - SBSv3 added by Matvey (SBSv3_m)
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/std
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/std
 *
 * *****************  Version 4  *****************
 * User: Stass        Date: 13.05.05   Time: 17:33
 * Updated in $/VS/std
 * new domain name
 *
 * *****************  Version 3  *****************
 * User: Smirnov      Date: 30.12.04   Time: 14:12
 * Updated in $/VS/std
 * files headers
 ****************************************************************************/
#ifndef VS_REG_SERVER_H
#define VS_REG_SERVER_H

/****************************************************************************
 * Defines
 ****************************************************************************/
//@{ \name Direcory (Registration) Server endpoint connect parameters
#ifdef _SVKS_M_BUILD_
#define RegServerName			"reg.mvd#regs"
#define RegServerHost			"reg.mvd"
#else
#define RegServerName			"reg.trueconf.com#regs"
#define RegServerHost			"reg.trueconf.com"
#endif

#define RegServerPort			4310
#define RegServerProtocol "TCP"

//@}

#endif
