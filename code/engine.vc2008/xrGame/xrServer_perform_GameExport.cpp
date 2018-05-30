#include "stdafx.h"
#include "xrserver.h"
#include "xrmessages.h"

void xrServer::Perform_game_export	()
{
    if (!((xrClientData*)SV_Client)->net_Accepted)
        return;

    NET_Packet P;
    P.w_begin(M_SV_CONFIG_GAME);
    game->net_Export_State(P);
	SendTo_LL(P.B.data, (u32)P.B.count);

	game->sv_force_sync	= FALSE;
}