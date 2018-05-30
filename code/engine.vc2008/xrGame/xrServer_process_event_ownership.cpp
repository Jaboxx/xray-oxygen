#include "stdafx.h"
#include "xrserver.h"
#include "xrserver_objects.h"
#include "xrserver_objects_alife_monsters.h"

void xrServer::Process_event_ownership(NET_Packet& P, u32, u16 ID, BOOL bForced)
{
	u16 id_parent = ID, id_entity;
	P.r_u16(id_entity);
	CSE_Abstract* e_parent = game->get_entity_from_eid(id_parent);
	CSE_Abstract* e_entity = game->get_entity_from_eid(id_entity);

	if (!e_parent) 
	{
		Msg("! ERROR on ownership: parent not found. parent_id = [%d], entity_id = [%d], frame = [%d].", id_parent, id_entity, Device.dwFrame);
		return;
	}
	if (!e_entity)
	{
		return;
	}

	if (!is_object_valid_on_svclient(id_parent))
	{
		Msg("! ERROR on ownership: parent object is not valid on sv client. parent_id = [%d], entity_id = [%d], frame = [%d]", id_parent, id_entity, Device.dwFrame);
		return;
	}

	if (!is_object_valid_on_svclient(id_entity))
	{
		Msg("! ERROR on ownership: entity object is not valid on sv client. parent_id = [%d], entity_id = [%d], frame = [%d]", id_parent, id_entity, Device.dwFrame);
		return;
	}

	if (0xffff != e_entity->ID_Parent)	return;

	xrClientData* c_parent = e_parent->owner;

	if (c_parent != GetServerClient())
		return;

	CSE_ALifeCreatureAbstract* alife_entity = smart_cast<CSE_ALifeCreatureAbstract*>(e_parent);

	// Game allows ownership of entity
	game->OnTouch(id_parent, id_entity, bForced);

	// Rebuild parentness
	e_entity->ID_Parent = id_parent;
	e_parent->children.push_back(id_entity);

	if (bForced)
	{
		//  ����� ����� ������, �� �� ������ ������ ����� ������ ���. ������� ������� ���������...
		u16 NewType = GE_OWNERSHIP_TAKE;
		std::memcpy(&P.B.data[6], &NewType, 2);
		// TODO: ��������� �������: ������ ����� ��� 2 �����!!!
	}
	SendTo_LL(P.B.data, (u32)P.B.count);
}
