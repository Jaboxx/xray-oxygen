// xrServer.cpp: implementation of the xrServer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "xrServer.h"
#include "xrMessages.h"
#include "xrServer_Objects_ALife_All.h"
#include "level.h"
#include "game_cl_base.h"
#include "ai_space.h"
#include "../xrEngine/IGame_Persistent.h"
#include "string_table.h"
#include "object_broker.h"

#include "../xrEngine/XR_IOConsole.h"
#include "ui/UIInventoryUtilities.h"

#include "../FrayBuildConfig.hpp"

#pragma warning(push)
#pragma warning(disable:4995)
#include <malloc.h>
#include <functional>
#pragma warning(pop)

xrClientData::xrClientData(): IClient(Device.GetTimerGlobal())
{
	Clear();
}

void xrClientData::Clear()
{
	owner = nullptr;
	net_Ready = FALSE;
	net_Accepted = FALSE;
};

xrServer::xrServer() : IPureServer(Device.GetTimerGlobal())
{
	m_aDelayedPackets.clear();
}

xrServer::~xrServer()
{
	struct ClientDestroyer
	{
		static bool true_generator(IClient*)
		{
			return true;
		}
	};
	IClient* tmp_client = net_players.GetFoundClient(&ClientDestroyer::true_generator);
	while (tmp_client)
	{
		client_Destroy(tmp_client);
		tmp_client = net_players.GetFoundClient(&ClientDestroyer::true_generator);
	}
	m_aDelayedPackets.clear();
	entities.clear();
}

//--------------------------------------------------------------------

CSE_Abstract* xrServer::ID_to_entity(u16 ID)
{
	if (0xffff != ID)
	{
		xrS_entities::iterator	I = entities.find(ID);
		if (entities.end() != I)		return I->second;
	}
	return nullptr;
}

//--------------------------------------------------------------------
void xrServer::client_Destroy(IClient* C)
{
	// Delete assosiated entity
	IClient* alife_client =	net_players.FindAndEraseClient(std::bind(std::equal_to<IClient*>(), std::placeholders::_1,C));

	if (alife_client)
	{
		CSE_Abstract* pOwner	= static_cast<xrClientData*>(alife_client)->owner;
		CSE_Spectator* pS		= smart_cast<CSE_Spectator*>(pOwner);
		if (pS)
		{
			NET_Packet			P;
			P.w_begin			(M_EVENT);
			P.w_u32				(Level().timeServer());
			P.w_u16				(GE_DESTROY);
			P.w_u16				(pS->ID);
			SendBroadcast		(C->ID,P);
		};

		DelayedPacket pp;
		pp.SenderID = alife_client->ID;
		xr_deque<DelayedPacket>::iterator it;
		do{
			it						=std::find(m_aDelayedPackets.begin(),m_aDelayedPackets.end(),pp);
			if(it!=m_aDelayedPackets.end())
			{
				m_aDelayedPackets.erase	(it);
				Msg("removing packet from delayed event storage");
			}else
				break;
		}while(true);
		
		if (pOwner)
		{
			game->CleanDelayedEventFor(pOwner->ID);
		}
	}
}

//--------------------------------------------------------------------
#ifdef DEBUG
INT g_sv_SendUpdate = 0;
#endif

void xrServer::Update	()
{
	VERIFY(verify_entities());

	ProceedDelayedPackets();
	// game update
	game->ProcessDelayedEvent();
	game->Update();

	if (game->sv_force_sync)	Perform_game_export();
#ifdef DEBUG
    verify_entities();
#endif
}

void xrServer::OnDelayedMessage(NET_Packet& P, ClientID sender)
{
	u16 type;
	P.r_begin(type);

	if (type == M_CLIENT_REQUEST_CONNECTION_DATA)
		OnCL_Connected(SV_Client);

#ifdef DEBUG
    verify_entities();
#endif
}

extern	float	g_fCatchObjectTime;
u32 xrServer::OnMessage(NET_Packet& P, ClientID sender)			// Non-Zero means broadcasting with "flags" as returned
{
	u16 type;
	P.r_begin(type);

	xrClientData* CL = ID_to_client(sender);

	switch (type)
	{
	case M_UPDATE:	
		{
			Process_update			(P,sender);						// No broadcast
		}break;
	case M_SPAWN:	
		{
			Process_spawn			(P,sender);
		}break;
	case M_EVENT:	
		{
			Process_event			(P,sender);
		}break;
	case M_EVENT_PACK:
		{
			NET_Packet	tmpP;
			while (!P.r_eof())
			{
				tmpP.B.count		= P.r_u8();
				P.r					(&tmpP.B.data, tmpP.B.count);

				OnMessage			(tmpP, sender);
			};			
		}break;
	//-------------------------------------------------------------------
	case M_SWITCH_DISTANCE:
		{
			game->switch_distance	(P,sender);
		}break;
	case M_CHANGE_LEVEL:
		{
			if (game->change_level(P,sender))
			{
				SendBroadcast		(BroadcastCID,P);
			}
		}break;
	case M_SAVE_GAME:
		{
			game->save_game			(P,sender);
		}break;
	case M_LOAD_GAME:
		{
			game->load_game			(P,sender);
			SendBroadcast			(BroadcastCID,P);
		}break;
	case M_SAVE_PACKET:
		{
			Process_save			(P,sender);
		}break;
	case M_CLIENT_REQUEST_CONNECTION_DATA:
		{
			AddDelayedPacket(P, sender);
		}break;
	}
	VERIFY (verify_entities());

	return 0;
}

void xrServer::SendBroadcast(ClientID exclude, NET_Packet& P)
{
    if (!SV_Client)
        return;
    if (SV_Client->ID == exclude)
        return;
    if (!SV_Client->flags.bConnected)
        return;

    xrClientData* tmp_client = static_cast<xrClientData*>(SV_Client);
	if (tmp_client->net_Accepted)
	{
		Level().OnMessage(P.B.data, (u32)P.B.count);
	}
}
//--------------------------------------------------------------------
CSE_Abstract*	xrServer::entity_Create		(LPCSTR name)
{
	return F_entity_Create(name);
}

void xrServer::entity_Destroy(CSE_Abstract *&P)
{
	R_ASSERT					(P);
	entities.erase				(P->ID);
	m_tID_Generator.vfFreeID	(P->ID,Device.TimerAsync());

	if(P->owner && P->owner->owner==P)
		P->owner->owner		= nullptr;

	P->owner = nullptr;
	if (!ai().get_alife() || !P->m_bALifeControl)
	{
		F_entity_Destroy		(P);
	}
}

//--------------------------------------------------------------------
CSE_Abstract* xrServer::GetEntity(u32 Num)
{
	xrS_entities::iterator I = entities.begin(), E = entities.end();
	for (u32 C = 0; I != E; ++I, ++C)
	{
		if (C == Num) return I->second;
	}
	return nullptr;
};

#ifdef DEBUG

static	bool _ve_initialized	= false;
static	bool _ve_use			= true;

bool xrServer::verify_entities() const
{
#ifdef SLOW_VERIFY_ENTITIES
	if (!_ve_initialized) 
	{
		_ve_initialized = false;
		if (strstr(Core.Params, "-~ve"))	_ve_use = false;
	}
	if (!_ve_use)						return true;

	xrS_entities::const_iterator		I = entities.begin();
	xrS_entities::const_iterator		E = entities.end();
	for ( ; I != E; ++I) {
		VERIFY2							((*I).first != 0xffff,"SERVER : Invalid entity id as a map key - 0xffff");
		VERIFY2							((*I).second,"SERVER : Null entity object in the map");
		VERIFY3							((*I).first == (*I).second->ID,"SERVER : ID mismatch - map key doesn't correspond to the real entity ID",(*I).second->name_replace());
		verify_entity					((*I).second);
	}
#endif
	return								(true);
}

void xrServer::verify_entity				(const CSE_Abstract *entity) const
{
	VERIFY(entity->m_wVersion!=0);
	if (entity->ID_Parent != 0xffff) {
		xrS_entities::const_iterator	J = entities.find(entity->ID_Parent);
		VERIFY2							(J != entities.end(),
			make_string("SERVER : Cannot find parent in the map [%s][%s]",entity->name_replace(),
			entity->name()).c_str());
		VERIFY3							((*J).second,"SERVER : Null entity object in the map",entity->name_replace());
		VERIFY3							((*J).first == (*J).second->ID,"SERVER : ID mismatch - map key doesn't correspond to the real entity ID",(*J).second->name_replace());
		VERIFY3							(std::find((*J).second->children.begin(),(*J).second->children.end(),entity->ID) != (*J).second->children.end(),"SERVER : Parent/Children relationship mismatch - Object has parent, but corresponding parent doesn't have children",(*J).second->name_replace());
	}

	xr_vector<u16>::const_iterator		I = entity->children.begin();
	xr_vector<u16>::const_iterator		E = entity->children.end();
	for ( ; I != E; ++I) {
		VERIFY3							(*I != 0xffff,"SERVER : Invalid entity children id - 0xffff",entity->name_replace());
		xrS_entities::const_iterator	J = entities.find(*I);
		VERIFY3							(J != entities.end(),"SERVER : Cannot find children in the map",entity->name_replace());
		VERIFY3							((*J).second,"SERVER : Null entity object in the map",entity->name_replace());
		VERIFY3							((*J).first == (*J).second->ID,"SERVER : ID mismatch - map key doesn't correspond to the real entity ID",(*J).second->name_replace());
		VERIFY3							((*J).second->ID_Parent == entity->ID,"SERVER : Parent/Children relationship mismatch - Object has children, but children doesn't have parent",(*J).second->name_replace());
	}
}

#endif // DEBUG

shared_str xrServer::level_name(const shared_str &server_options) const
{
	return	(game->level_name(server_options));
}
shared_str xrServer::level_version(const shared_str &server_options) const
{
	return	(game_sv_GameState::parse_level_version(server_options));
}

void xrServer::createClient()
{
	SV_Client = xr_new<xrClientData>();
	SV_Client->ID.set(1);
    net_players.AddNewClient(SV_Client);
	SV_Client->flags.bConnected = TRUE;
}

void xrServer::ProceedDelayedPackets()
{
    std::lock_guard<decltype(DelayedPackestCS)> lock(DelayedPackestCS);
	while (!m_aDelayedPackets.empty())
	{
		DelayedPacket& DPacket = *m_aDelayedPackets.begin();
		OnDelayedMessage(DPacket.Packet, DPacket.SenderID);
		m_aDelayedPackets.pop_front();
	}
};

void xrServer::AddDelayedPacket	(NET_Packet& Packet, ClientID Sender)
{
    std::lock_guard<decltype(DelayedPackestCS)> lock(DelayedPackestCS);

	m_aDelayedPackets.push_back(DelayedPacket());
	DelayedPacket* NewPacket = &(m_aDelayedPackets.back());
	NewPacket->SenderID = Sender;
    std::memcpy(&(NewPacket->Packet),&Packet,sizeof(NET_Packet));
}

void xrServer::Disconnect()
{
	SLS_Clear();
	xr_delete(game);
}

#include "GameObject.h"
bool is_object_valid_on_svclient(u16 id_entity)
{
	CObject* tmp_obj		= Level().Objects.net_Find(id_entity);
	if (!tmp_obj)
		return false;
	
	CGameObject* tmp_gobj	= smart_cast<CGameObject*>(tmp_obj);
	if (!tmp_gobj)
		return false;

	if (tmp_obj->getDestroy())
		return false;

	if (tmp_gobj->object_removed())
		return false;
	
	return true;
};
