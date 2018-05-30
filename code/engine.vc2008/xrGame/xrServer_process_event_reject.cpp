#include "stdafx.h"
#include "xrserver.h"
#include "xrserver_objects.h"

bool xrServer::Process_event_reject(NET_Packet& P, const u32 time, const u16 id_parent, const u16 id_entity, bool send_message)
{
	// Parse message
	CSE_Abstract*		e_parent	= game->get_entity_from_eid	(id_parent);
	CSE_Abstract*		e_entity	= game->get_entity_from_eid	(id_entity);
	VERIFY2  ( e_entity, make_string( "entity not found. parent_id = [%d], entity_id = [%d], frame = [%d]", id_parent, id_entity, Device.dwFrame ).c_str() );
	if ( !e_entity ) 
	{
		Msg                ( "! ERROR on rejecting: entity not found. parent_id = [%d], entity_id = [%d], frame = [%d].", id_parent, id_entity, Device.dwFrame );
		return false;
	}

	VERIFY2  ( e_parent, make_string( "parent not found. parent_id = [%d], entity_id = [%d], frame = [%d]", id_parent, id_entity, Device.dwFrame ).c_str() );
	if ( !e_parent ) {
		Msg                ( "! ERROR on rejecting: parent not found. parent_id = [%d], entity_id = [%d], frame = [%d].", id_parent, id_entity, Device.dwFrame );
		return false;
	}

	xr_vector<u16>& C		= e_parent->children;
	xr_vector<u16>::iterator c	= std::find	(C.begin(),C.end(),id_entity);
	if (c == C.end())
	{
		Msg("! ERROR: SV: can't find children [%d] of parent [%d]", id_entity, e_parent);
		return false;
	}

	if (0xffff == e_entity->ID_Parent) 
	{
		return			(false);
	}

	if (e_entity->ID_Parent != id_parent)
	{
		Msg("! ERROR: e_entity->ID_Parent = [%d]  parent = [%d][%s]  entity_id = [%d]  frame = [%d]",
			e_entity->ID_Parent, id_parent, e_parent->name_replace(), id_entity, Device.dwFrame);
		//it can't be !!!
	}

	game->OnDetach(id_parent,id_entity);

	e_entity->ID_Parent = 0xffff; 
	C.erase(c);
	
	if (send_message)
	{
		SendTo_LL(P.B.data, (u32)P.B.count);
	}
	return (true);
}
