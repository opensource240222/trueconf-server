#pragma once

#define INITGUID
#include "netfw.h"

#include <list>

enum VS_IP_PROTOCOL
{
	e_IP_PROTOCOL_TCP,
	e_IP_PROTOCOL_UDP
};
/**
\brief Интерфейс манипуляции фаерволом.
*/
class VS_FirewallInterface
{
public:
	virtual ~VS_FirewallInterface(){}
	virtual bool	IsValid()		= 0;
	virtual int		IsFirewallOn()	= 0;
	virtual int		IsPortEnabled(const unsigned long port, const VS_IP_PROTOCOL protocol)	= 0;
	virtual int		EnableFirewall(const bool enable) = 0;
	virtual int		OpenPort(const unsigned long port, VS_IP_PROTOCOL protocol,
								const wchar_t *registerName) = 0;
	virtual int		ClosePort(const unsigned long port, VS_IP_PROTOCOL protocol) = 0;
	virtual int		AddApplication(const wchar_t *fileName,const wchar_t * registerName, const wchar_t *scope) = 0;
	virtual int		RemoveApplication(const wchar_t *fileName) = 0;
};

/**
\brief Позволяет управлять несколькими фаерволами (если, например, в системе стоит несколько фаерволов)
*/
class VS_FirewallMgr
{
protected:
	VS_FirewallMgr();
	std::list<VS_FirewallInterface*> m_Firewalls;
private:
	static VS_FirewallMgr *instance;
public:
	virtual				~VS_FirewallMgr();
	static VS_FirewallMgr	*Instance();

	bool	AddFirewall(VS_FirewallInterface *);
	bool	IsValid();
	bool	OpenPort(const unsigned long port, VS_IP_PROTOCOL protocol, const wchar_t *registerName);
	bool	ClosePort(const unsigned long port, VS_IP_PROTOCOL protocol);
	bool	AddApplication(const wchar_t *fileName,const wchar_t * registerName, const wchar_t *scope);
	bool	RemoveApplication(const wchar_t *fileName);
};

/**
Класс управления WinXPFirewall.
*/
class VS_WinXPFirewall : public VS_FirewallInterface
{
private:
	INetFwProfile* m_FireWallProfile;
	VS_WinXPFirewall();
	static VS_WinXPFirewall *instance;
public:
	virtual ~VS_WinXPFirewall();
/**
\brief Метод, управляющий созданием экземпляра класса (может быть только один).
*/
	static VS_WinXPFirewall *Instance();
/**
\brief Проверяет,  проверяет, установлен ли fw
*/
	virtual bool	IsValid();
/**
\brief Включен ли firewall.

@return 0 - fw не включен, 1 - fw включен, -1 - fw не установлен, -2 - ошибка COM
*/
	virtual int		IsFirewallOn();
/**
\brief Открыт ли поределенный порт.

@return 0 - порт заблокирован, 1 - порт открыт для входящих соединений, -1 - fw не установлен, -2 - ошибка COM
*/
	virtual int		IsPortEnabled(const unsigned long port, const VS_IP_PROTOCOL protocol);
/**
\brief Включтиь/Выключить фаервол.
*/
	virtual int		EnableFirewall(const bool enable);
/**
\brief Открыть порт.

 @param[in]	port		- открываемый порт;
 @param[in] protсol		- протокол, для которого открывается порт. Может принимать значения e_IP_PROTOCOL_TCP или e_IP_PROTOCOL_UDP
 @param[in]	registerName  - имя порта.
*/
	virtual int		OpenPort(const unsigned long port, VS_IP_PROTOCOL protocol, const wchar_t *registerName);
/**
\brief Закрыть порт.

 @porom[in]	port		- закрываемй порт
 @param[in]	protocol	- протокол, для которого закрывается порт.
*/
	virtual int		ClosePort(const unsigned long port, VS_IP_PROTOCOL protocol);
/**
\brief Разрешить приложению принимать соединения(быть сервером)

 @param[in]	fileName		- путь к файлу программы;
 @param[in]	registerName	- имя записи;
 @param[in] scoe			- определяет множество адресов, с которых будет разрешено подключаться.
							Формат:
							- 0 - доступно со всех адресов;
							- L"LocalSubnet" -
							- можно задать network mask или сетевой префикс. Примеры валидных значений: "10.0.0.2/255.0.0.0", "10.0.0.2/8", "10.0.0.2";
*/
	virtual int		AddApplication(const wchar_t *fileName, const wchar_t *registerName, const wchar_t *scope);
/**
\brief Удалить приложние из списка разрешенных
*/
	virtual int		RemoveApplication(const wchar_t *fileName);
};