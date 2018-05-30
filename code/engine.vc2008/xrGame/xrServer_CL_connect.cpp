#include "stdafx.h"
#include "xrserver.h"
#include "xrmessages.h"
#include "xrserver_objects.h"
#include "xrServer_Objects_Alife_Monsters.h"
#include "Level.h"
#include "GamePersistent.h"


void xrServer::Perform_connect_spawn(CSE_Abstract* E, xrClientData* CL, NET_Packet& P)
{
	P.B.count = 0;
	if(std::find(conn_spawned_ids.begin(), conn_spawned_ids.end(), E->ID) != conn_spawned_ids.end())
		return;
	
	conn_spawned_ids.push_back(E->ID);
	
	if (E->net_Processed)						return;
	if (E->s_flags.is(M_SPAWN_OBJECT_PHANTOM))	return;


	// Connectivity order
	CSE_Abstract* Parent = ID_to_entity	(E->ID_Parent);
	if (Parent)		Perform_connect_spawn	(Parent,CL,P);

	// Process
	Flags16			save = E->s_flags;
	//-------------------------------------------------
	E->s_flags.set	(M_SPAWN_UPDATE,TRUE);
	if (!E->owner)	
	{
		// PROCESS NAME; Name this entity
		if (E->s_flags.is(M_SPAWN_OBJECT_ASPLAYER))
		{
			CL->owner			= E;
			E->set_name_replace	("");
		}

		// Associate
		E->owner		= CL;
		E->Spawn_Write	(P,TRUE	);
		E->UPDATE_Write	(P);

		CSE_ALifeObject*	object = smart_cast<CSE_ALifeObject*>(E);
		VERIFY				(object);
		if (!object->keep_saved_data_anyway())
			object->client_data.clear	();
	}
	else				
	{
		E->Spawn_Write	(P, FALSE);
		E->UPDATE_Write	(P);
	}
	//-----------------------------------------------------
	E->s_flags			= save;
	SendTo_LL(P.B.data, (u32)P.B.count);
	E->net_Processed	= TRUE;
}

void xrServer::SendConnectionData()
{
	conn_spawned_ids.clear();
	NET_Packet		P;
	// Replicate current entities on to this client

	for (auto &xrSe_it: entities)
		xrSe_it.second->net_Processed = FALSE;

	for (auto &xrSe_it : entities)
		Perform_connect_spawn(xrSe_it.second, (xrClientData*)SV_Client, P);

	// Start to send server logo and rules
	NET_Packet Packet2;
	P.w_begin(M_SV_CONFIG_FINISHED);
	SendTo_LL(Packet2.B.data, (u32)Packet2.B.count);
};

void xrServer::OnCL_Connected()
{
	// Export Game Type
	NET_Packet P;
	P.w_begin(M_SV_CONFIG_NEW_CLIENT);
	P.w_stringZ(game->type_name());
	SendTo_LL(P.B.data, (u32)P.B.count);
	// end

	Perform_game_export();
	SendConnectionData();
	game->OnPlayerConnect();	
}
