#pragma once
#include "net_shared.h"

class ENGINE_API IClient
{
public:
	struct Flags
	{
		u32		bConnected	: 1;
	};
				IClient( CTimer* timer );
	virtual		~IClient() = default;

	ClientID	ID;
	Flags		flags;	// local/host/normal
};


IC bool operator== (IClient const* pClient, ClientID const& ID) { return pClient->ID == ID; }

//==============================================================================

class CServerInfo;

class ENGINE_API IPureServer
{
public:
	enum EConnect
	{
		ErrConnect,
		ErrMax,
		ErrNoError = ErrMax,
	};
protected:
	shared_str				connect_options;
	IClient*				SV_Client;
	std::recursive_mutex	csMessage;
	
	// Statistic
	CTimer*					device_timer;

	IClient*				ID_to_client		(ClientID ID, bool ScanAll = false);
public:
							IPureServer (CTimer* timer);
	virtual					~IPureServer		();
	
	virtual EConnect		Connect				(LPCSTR session_name);

	// extended functionality
	virtual void			OnMessage			(NET_Packet& P) = 0;
	virtual void			client_Destroy		(IClient* C)	= 0;			// destroy client info

	IClient*				GetServerClient		()			{ return SV_Client; };

	const shared_str&		GetConnectOptions	() const {return connect_options;}
};

