 /**
 ****************************************************************************
 * (c) 2002-2003 Visicron Inc.  http://www.visicron.net/
 *
 * Project: Visicron server services
 *
 * VS_Testable interface definition file
 *
 * $Revision: 1 $
 * $History: VS_Testable.h $
 *
 * *****************  Version 1  *****************
 * User: Avlaskin     Date: 3.10.07    Time: 14:51
 * Created in $/VSNA/Servers/Watchdog
 *
 * *****************  Version 1  *****************
 * User: Mushakov     Date: 2.02.07    Time: 19:52
 * Created in $/VS2005/Servers/Watchdog
 *
 * *****************  Version 1  *****************
 * User: Slavetsky    Date: 6/02/03    Time: 6:44p
 * Created in $/VS/Servers/Watchdog
 *
 * *****************  Version 1  *****************
 * User: Stass        Date: 27.05.03   Time: 17:46
 * Created in $/VS/Servers/ServerServices
 * added VS_Testable - interface for checking service state
 ****************************************************************************/


#pragma once

class VS_Testable
{
public:
	virtual bool	Test( void ) = 0;
};
// end VS_Testable class

