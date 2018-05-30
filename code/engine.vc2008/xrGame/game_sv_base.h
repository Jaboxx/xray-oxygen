#pragma once

#include "game_base.h"
#include "alife_space.h"
#include "../xrScripts/export/script_export_space.h"
#include "../xrCore/client_id.h"
#include "game_sv_event_queue.h"
#include "../xrEngine/ClS/NET_Server.h"
#include "alife_simulator.h"

class CSE_Abstract;
class xrServer;
class GameEventQueue;

class game_sv_GameState	: public game_GameState
{
	typedef game_GameState inherited;

public:
	BOOL							sv_force_sync;
protected:
	xrServer*						m_server;
	GameEventQueue*					m_event_queue;
	CALifeSimulator					*m_alife_simulator;

	//Events
	virtual		void				OnEvent					(NET_Packet &tNetPacket, u16 type, u32 time);

public:
	virtual		void				OnPlayerConnect			();

public:
									game_sv_GameState		();
	virtual							~game_sv_GameState		();
	// Main accessors
	virtual		void*				get_client				(u16 id);
				CSE_Abstract*		get_entity_from_eid		(u16 id);
	// Signals
	virtual		void				signal_Syncronize		();

	virtual		void				OnRender				();
	
				CSE_Abstract*		spawn_begin				(LPCSTR N);
				CSE_Abstract*		spawn_end				(CSE_Abstract* E, ClientID id);

	// Utilities
	s32								get_option_i			(LPCSTR lst, LPCSTR name, s32 def = 0);
	virtual		xr_vector<u16>*		get_children			(ClientID id_who);
	void							u_EventGen				(NET_Packet& P, u16 type, u16 dest	);
	void							u_EventSend				(NET_Packet& P);

	// Events
	virtual		void				OnCreate				(u16 id_who);
	virtual		void				OnTouch					(u16 eid_who, u16 eid_target, BOOL bForced = FALSE); // TRUE=allow ownership, FALSE=denied
	virtual		void				OnDetach				(u16 eid_who, u16 eid_target);

	virtual		void				OnDestroyObject			(u16 eid_who);			

	// Main
	virtual		void				Create					(shared_str& options);
	virtual		void				Update					();
	virtual		void				net_Export_State		(NET_Packet& P);				// full state
	virtual		void				net_Export_GameTime		(NET_Packet& P);				// update GameTime only for remote clients

	virtual		bool				change_level			(NET_Packet &net_packet);
	virtual		void				save_game				(NET_Packet &net_packet);
	virtual		bool				load_game				(NET_Packet &net_packet);
	virtual		void				switch_distance			(NET_Packet &net_packet);

				void				AddDelayedEvent			(NET_Packet &tNetPacket, u16 type, u32 time);
				void				ProcessDelayedEvent		();
				//this method will delete all events for entity that already not exist (in case when player was kicked)
				void				CleanDelayedEventFor	(u16 id_entity_victim);

	virtual		void				teleport_object			(NET_Packet &packet, u16 id);
	virtual		void				add_restriction			(NET_Packet &packet, u16 id);
	virtual		void				remove_restriction		(NET_Packet &packet, u16 id);
	virtual		void				remove_all_restrictions	(NET_Packet &packet, u16 id);
	virtual		bool				custom_sls_default		() {return !!m_alife_simulator;};
	virtual		void				sls_default				();
	virtual		shared_str			level_name				(const shared_str &server_options) const;
	
	static		shared_str			parse_level_version		(const shared_str &server_options);

	virtual		void				on_death				(CSE_Abstract *e_dest, CSE_Abstract *e_src);
	
	// Single State
	IC			xrServer			&server() const 		{ return (*m_server); }
	IC			CALifeSimulator		&alife() const			{ return (*m_alife_simulator); }
	void		restart_simulator							(LPCSTR saved_game_name);
	
	// Times
	virtual		ALife::_TIME_ID		GetStartGameTime();
	virtual		ALife::_TIME_ID		GetGameTime();
	virtual		float				GetGameTimeFactor();
	virtual		void				SetGameTimeFactor(const float fTimeFactor);

	virtual		ALife::_TIME_ID		GetEnvironmentGameTime();
	virtual		float				GetEnvironmentGameTimeFactor();
	virtual		void				SetEnvironmentGameTimeFactor(const float fTimeFactor);
};
