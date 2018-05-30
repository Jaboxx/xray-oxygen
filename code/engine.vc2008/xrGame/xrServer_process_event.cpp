#include "stdafx.h"
#include "xrServer.h"
#include "alife_simulator.h"
#include "xrserver_objects.h"
#include "game_base.h"
#include "game_cl_base.h"
#include "ai_space.h"
#include "alife_object_registry.h"
#include "xrServer_Objects_ALife_Items.h"
#include "xrServer_Objects_ALife_Monsters.h"

void xrServer::Process_event(NET_Packet& P)
{
	// correct timestamp with server-unique-time (note: direct message correction)
	u32 timestamp = P.r_u32();
	// read generic info
	u16 type = P.r_u16();
	u16 destination = P.r_u16();

	CSE_Abstract* receiver = game->get_entity_from_eid(destination);
	if (receiver)
	{
		R_ASSERT(receiver->owner);
		receiver->OnEvent(P, type, timestamp);
	}

	switch (type)
	{
	case GE_GAME_EVENT:
	{
		u16		game_event_type;
		P.r_u16(game_event_type);
		game->AddDelayedEvent(P, game_event_type, timestamp);
	}break;

	case GEG_PLAYER_ACTIVATEARTEFACT:	Process_event_activate(P, timestamp, destination, P.r_u16(), true); break;
	case GE_INV_ACTION:					if (SV_Client) SendTo_LL(P.B.data, (u32)P.B.count); break;
	case GE_TRADE_BUY:
	case GE_OWNERSHIP_TAKE:				Process_event_ownership(P, timestamp, destination); break;
	case GE_OWNERSHIP_TAKE_MP_FORCED:	Process_event_ownership(P, timestamp, destination, TRUE); break;
	case GE_TRADE_SELL:
	case GE_OWNERSHIP_REJECT:
	case GE_LAUNCH_ROCKET:				Process_event_reject(P, timestamp, destination, P.r_u16()); break;
	case GE_DESTROY:					Process_event_destroy(P, timestamp, destination, nullptr); break;
	case GE_CHANGE_POS:					SendTo_LL(P.B.data, (u32)P.B.count); break;
	case GEG_PLAYER_DISABLE_SPRINT:
	case GEG_PLAYER_WEAPON_HIDE_STATE:	SendTo_LL(P.B.data, (u32)P.B.count); break;
	case GEG_PLAYER_ACTIVATE_SLOT:
	case GEG_PLAYER_ITEM_EAT:			SendTo_LL(P.B.data, (u32)P.B.count); break;
	case GE_TELEPORT_OBJECT:			game->teleport_object(P, destination); break;
	case GE_ADD_RESTRICTION:			game->add_restriction(P, destination); break;
	case GE_REMOVE_RESTRICTION:			game->remove_restriction(P, destination); break;
	case GE_REMOVE_ALL_RESTRICTIONS:	game->remove_all_restrictions(P, destination); break;
	case GE_MONEY:						smart_cast<CSE_ALifeTraderAbstract*>(receiver)->m_dwMoney = P.r_u32(); break;

	case GEG_PLAYER_USE_BOOSTER:
	{
		if (receiver && receiver->owner && (receiver->owner != SV_Client))
		{
			NET_Packet tmp_packet;
			CGameObject::u_EventGen(tmp_packet, GEG_PLAYER_USE_BOOSTER, receiver->ID);
			SendTo_LL(P.B.data, (u32)P.B.count);
		}
	}break;

	case GE_INSTALL_UPGRADE:
	{
		shared_str				upgrade_id;
		P.r_stringZ(upgrade_id);
		CSE_ALifeInventoryItem* iitem = smart_cast<CSE_ALifeInventoryItem*>(receiver);
		if (!iitem)
		{
			break;
		}
		iitem->add_upgrade(upgrade_id);
	}break;

	case GE_INV_BOX_STATUS:
	{
		u8 can_take, closed;
		P.r_u8(can_take);
		P.r_u8(closed);
		shared_str tip_text;
		P.r_stringZ(tip_text);

		CSE_ALifeInventoryBox* box = smart_cast<CSE_ALifeInventoryBox*>(receiver);
		if (!box)
		{
			break;
		}
		box->m_can_take = (can_take == 1);
		box->m_closed = (closed == 1);
		box->m_tip_text._set(tip_text);
	}break;

	case GE_INV_OWNER_STATUS:
	{
		u8 can_take, closed;
		P.r_u8(can_take);
		P.r_u8(closed);

		CSE_ALifeTraderAbstract* iowner = smart_cast<CSE_ALifeTraderAbstract*>(receiver);
		if (!iowner)
		{
			break;
		}
		iowner->m_deadbody_can_take = (can_take == 1);
		iowner->m_deadbody_closed = (closed == 1);
	}break;

	case GE_TRANSFER_AMMO:
	{
		u16 id_entity;
		P.r_u16(id_entity);
		CSE_Abstract* e_parent = receiver;	// кто забирает (для своих нужд)
		CSE_Abstract* e_entity = game->get_entity_from_eid(id_entity);	// кто отдает

		if (!e_entity) break;
		// this item already taken
		if (0xffff != e_entity->ID_Parent)	
			break;						
		xrClientData* c_parent = e_parent->owner;
		
		// assure client ownership of event
		R_ASSERT(GetServerClient() == c_parent);

		// Perfrom real destroy
		entity_Destroy(e_entity);
	}break;

	case GE_HIT:
	case GE_HIT_STATISTIC:
	{
		P.r_pos -= 2;
		if (type == GE_HIT_STATISTIC)
		{
			P.B.count -= 4;
			P.w_u32(GetServerClient()->ID.value());
		};
		game->AddDelayedEvent(P, GAME_EVENT_ON_HIT, 0);
	} break;

	case GE_ASSIGN_KILLER:
	{
		u16							id_src;
		P.r_u16(id_src);

		CSE_Abstract				*e_dest = receiver;	// кто умер
														// this is possible when hit event is sent before destroy event
		if (!e_dest)
			break;

		CSE_ALifeCreatureAbstract	*creature = smart_cast<CSE_ALifeCreatureAbstract*>(e_dest);
		if (creature)
			creature->set_killer_id(id_src);

		break;
	}

	case GE_CHANGE_VISUAL:
	{
		CSE_Visual* visual = smart_cast<CSE_Visual*>(receiver); VERIFY(visual);
		string256 tmp;
		P.r_stringZ(tmp);
		visual->set_visual(tmp);
	}break;

	case GE_DIE:
	{
		// Parse message
		u16 id_dest = destination, id_src;
		P.r_u16(id_src);
		VERIFY(game && GetServerClient);

		CSE_Abstract* e_dest = receiver;	// кто умер
		// this is possible when hit event is sent before destroy event
		if (!e_dest)
			break;

		CSE_Abstract* e_src = game->get_entity_from_eid(id_src);	// кто убил
		if (!e_src)
		{
			xrClientData* C = (xrClientData*)game->get_client(id_src);
			if (C) e_src = C->owner;
		}

		if (!e_src)
		{
			Msg("! ERROR: SV: src killer not exist.");
			return;
		}

		game->on_death(e_dest, e_src);

		xrClientData* c_src = e_src->owner;				// клиент, чей юнит убил

		if (c_src->owner->ID == id_src)
		{
			// Main unit
			P.w_begin(M_EVENT);
			P.w_u32(timestamp);
			P.w_u16(type);
			P.w_u16(destination);
			P.w_u16(id_src);
			P.w_clientID(c_src->ID);
		}

		//////////////////////////////////////////////////////////////////////////
		P.w_begin(M_EVENT);
		P.w_u32(timestamp);
		P.w_u16(GE_KILL_SOMEONE);
		P.w_u16(id_src);
		P.w_u16(destination);
		SendTo_LL(P.B.data, (u32)P.B.count);
		//////////////////////////////////////////////////////////////////////////
	}break;

	default:
		VERIFY(0, "Game Event not implemented!!!");
		break;
	}
	VERIFY(verify_entities());
}