#include "stdafx.h"
#include "game_sv_event_queue.h"


// 
GameEventQueue::GameEventQueue()		
{
	unused.reserve	(128);
	for (int i=0; i<16; i++)
		unused.push_back	(xr_new<GameEvent>());
}
GameEventQueue::~GameEventQueue()
{
    std::lock_guard<decltype(cs)> lock(cs);
	u32				it;
	for				(it=0; it<unused.size(); it++)	xr_delete(unused[it]);
	for				(it=0; it<ready.size(); it++)	xr_delete(ready[it]);
}

static u32 LastTimeCreate = 0;
GameEvent*		GameEventQueue::Create	()
{
	GameEvent*	ge			= 0;
    std::lock_guard<decltype(cs)> lock(cs);
	if (unused.empty())	
	{
		ready.push_back		(xr_new<GameEvent> ());
		ge					= ready.back	();
		LastTimeCreate = GetTickCount();
		//---------------------------------------------
	} else {
		ready.push_back		(unused.back());
		unused.pop_back		();
		ge					= ready.back	();
	}
	return	ge;
}

GameEvent* GameEventQueue::CreateSafe(NET_Packet& P, u16 type, u32 time)
{
	return Create(P, type, time);
}

GameEvent*		GameEventQueue::Create	(NET_Packet& P, u16 type, u32 time)
{
	GameEvent*	ge			= 0;
    std::lock_guard<decltype(cs)> lock(cs);
	if (unused.empty())	
	{
		ready.push_back		(xr_new<GameEvent> ());
		ge					= ready.back	();
		LastTimeCreate = GetTickCount();
		//---------------------------------------------
	} else {
		ready.push_back		(unused.back());
		unused.pop_back		();
		ge					= ready.back	();
	}
    std::memcpy(&(ge->P),&P,sizeof(NET_Packet));
	ge->time	= time;
	ge->type	= type;

	return			ge;
}
GameEvent*		GameEventQueue::Retreive	()
{
	GameEvent*	ge			= 0;
    std::lock_guard<decltype(cs)> lock(cs);
	if (!ready.empty())		ge = ready.front();
	//---------------------------------------------	
	else
	{
		u32 tmp_time = GetTickCount()-60000;
		u32 size = unused.size();
		if ((LastTimeCreate < tmp_time) &&  (size > 32))
		{
			xr_delete(unused.back());
			unused.pop_back();
		}		
	}
	//---------------------------------------------	
	return	ge;
}

void			GameEventQueue::Release	()
{
    std::lock_guard<decltype(cs)> lock(cs);
	R_ASSERT		(!ready.empty());
	//---------------------------------------------
	u32 tmp_time = GetTickCount()-60000;
	u32 size = unused.size();
	if ((LastTimeCreate < tmp_time) &&  (size > 32))
	{
		xr_delete(ready.front());
#ifdef _DEBUG
//		Msg ("GameEventQueue::Release - ready %d, unused %d", ready.size(), unused.size());
#endif
	}
	else
		unused.push_back(ready.front());
	//---------------------------------------------		
	ready.pop_front	();
}

void GameEventQueue::SetIgnoreEventsFor(bool ignore, ClientID clientID)
{
}

u32 GameEventQueue::EraseEvents(event_predicate to_del)
{
	u32 ret_val = 0;
    std::lock_guard<decltype(cs)> lock(cs);
	if (ready.empty())	//read synchronization...
		return 0;
	
	typedef xr_deque<GameEvent*>	event_queue;
	typedef event_queue::iterator	eq_iterator;
	
	eq_iterator need_to_erase = std::find_if(ready.begin(), ready.end(), to_del);
	while (need_to_erase != ready.end())
	{
		//-----
		u32 tmp_time = GetTickCount() - 60000;
		u32 size = unused.size();
		if ((LastTimeCreate < tmp_time) &&  (size > 32))
		{
			xr_delete(*need_to_erase);
		} else
		{
			unused.push_back(*need_to_erase);
		}
		ready.erase(need_to_erase);
		++ret_val;
		need_to_erase = std::find_if(ready.begin(), ready.end(), to_del);
	}

	return ret_val;
}
