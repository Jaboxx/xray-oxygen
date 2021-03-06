#pragma once
/* KERNEL */
#ifdef DEBUG
#define CHECK_MOUSE_STATE		// Sometimes, IDirectInputDevice8::GetDeviceData losses data (reason: unknown). 
								// By defining this, we force to double check mouse state by calling GetDeviceState. 
								// It's a very little performance penalty
//#define JIT_OXY_MEMPOOL_DEBUG	// Memory debbuging for 'X-Ray Mempool for LuaJIT'
#endif
//#define SLOW_VERIFY_ENTITIES	// Very slow verifing every entity on every game update

/* Memory */
#define LOW_TEXTURES_SIZE		// Reduces the size of textures in 2 times.

/* Threading */
#define NEW_TTAPI				// Enable new ttapi, based on WinAPI ThreadPool system
//#define TEST_TTAPI			// Perform a test of ttapi integrity

/* xrCore */
#define	LOG_TIME_PRECISE		// Optimization log writte (by alpet & RvP)

/* Scripts */
//#define SPAWN_ANTIFREEZE		// spread spawn of game objects thoughout multiple frames to prevent lags
//#define LUACP_API				// attaching luaicp.dll

/* xrRender */
#define R34_USE_FIRSTMIPLEVEL	// activate: LoadInfo.FirstMipLevel = img_loaded_lod;

/* xrGame */
#define WPN_AUTORELOAD			// Enable autoreload
//#define DYNAMIC_SUN_MOVEMENT 	// Use dynamic sun movement
//#define ASPAWN_CROW			// Spawn crows over actor
//#define NEW_ANIMS_WPN			// Enable anm_..._crouch, anm_reload_empty
#define DEAD_BODY_WEAPON		// Enable weapons in dead stalkers
#define POLTERGEIST_BUILD_DIE	// Old style polter dead. Need fixes for scripts and configs.

/* IUI states value */
//#define ARROW_VALUE_INV		// Adds arrows to inventory. Clear Sky style. For use unlock in actor_menu/_16.xml tags <arrow> and <arrow_shadow>
#define PROGRESS_VALUE_INV		// Adds progress bars value in the inventory. COP style. Used for default values.
//#define NUM_PARAMS_INV		// Adds progress bars in the inventory numeric value. Max value 100%. (%) - color="no_color"
//#define SHAPE_VALUE_INV		// Adds shape progress to inventory. 
//#define MINMAP_BAR_HUD		// Adds progress bars to luminosity and noise value
//#define HUD_DOSIMETER			// Adds dosimeter on hud. From Clear Sky.
//#define HUD_SHAPE_VALUE		// Adds shape progress to HUD. Health and Stamina value.

/* True Stalker */
//#define HOLD_PICKUP_OFF		// Selection of subjects one by one.
//#define ACTOR_RUCK			// Ruck slot to Actor Inventory.
//#define PSEUDOGIG_JUMP_NOHIT	// Disabling hit on pseudogigant jump.
//#define POLTER_SHOW_DEAD		// Showing poltergeist particle if polter dead.
#define MULTITRANSFER			// Multi transfer to actor
//#define SOC_TALK_WND			// Left and Right talkers icons
//#define MONSTER_INV			// Inventory on Monsted dead body
//#define VERTICAL_BELT			// Verticals belt slots