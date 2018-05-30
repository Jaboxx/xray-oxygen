#include "stdafx.h"
#include "net_server.h"

ENGINE_API ClientID BroadcastCID(0xffffffff);

IClient::IClient(CTimer* timer)
{
	flags.bConnected = FALSE;
}

//------------------------------------------------------------------------------
IClient* IPureServer::ID_to_client(ClientID ID, bool ScanAll)
{
	return SV_Client;
}

//==============================================================================
IPureServer::IPureServer(CTimer* timer)
{
	device_timer = timer;
	SV_Client = nullptr;
}

IPureServer::~IPureServer	()
{
	SV_Client = nullptr;
}

IPureServer::EConnect IPureServer::Connect(LPCSTR options)
{
	// Parse options
    string1024 session_name;
    xr_strcpy(session_name, options);
    if (strchr(session_name, '/'))	*strchr(session_name, '/') = 0;
    connect_options = options;
	return	ErrNoError;
}
