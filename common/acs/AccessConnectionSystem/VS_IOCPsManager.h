
#ifndef VS_ROUTER_IOCPS_MANAGER_H
#define VS_ROUTER_IOCPS_MANAGER_H

class VS_IOCPsManager
{
public:
			VS_IOCPsManager( void );
	~VS_IOCPsManager();
	bool	IsValid ( void ) const {	return imp != 0;	};
	bool		Init			( void );
	void		*HandleIocp		( void );
	void		Shutdown		( void );
	struct VS_IOCPsManager_Implementation   *imp;
};
// end VS_IOCPsManager class

#endif // VS_ROUTER_IOCPS_MANAGER_H
