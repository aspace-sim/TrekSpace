/* space_alert.c */
#include "config.h"

#include <ctype.h>
#include <string.h>
#include <math.h>
#ifdef I_SYS_TIME
#include <sys/time.h>
#else
#include <time.h>
#endif
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif

#include "copyrite.h"
#include "ansi.h"
#include "externs.h"
#include "intrface.h"
#include "parse.h"
#include "match.h"
#include "confmagic.h"
#include "space.h"
#include "dbdefs.h"

#include "function.h"
#include "attrib.h"


/* ------------------------------------------------------------------------ */

void alert_main_balance (int x)
{
	do_console_notify(x, console_engineering, 0, 0,
	  ansi_alert(tprintf("M/A reactor balanced at %.3f%%", sdb[x].main.out * 100.0)));
	return;
}

/* ------------------------------------------------------------------------ */

void alert_aux_balance (int x)
{
	do_console_notify(x, console_engineering, 0, 0,
	  ansi_alert(tprintf("Fusion reactor balanced at %.3f%%", sdb[x].aux.out * 100.0)));
	return;
}

/* ------------------------------------------------------------------------ */

void alert_batt_balance (int x)
{
	do_console_notify(x, console_engineering, 0, 0,
	  ansi_alert(tprintf("Batteries set at %.3f%%", sdb[x].batt.out * 100.0)));
	return;
}

/* ------------------------------------------------------------------------ */

void alert_main_overload (int x)
{
	do_all_console_notify(x, ansi_warn("M/A REACTOR CORE BREACH IN PROGRESS"));
	return;
}

/* ------------------------------------------------------------------------ */

void alert_aux_overload (int x)
{
	do_all_console_notify(x, ansi_warn("FUSION REACTOR CORE BREACH IN PROGRESS"));
	return;
}

/* ------------------------------------------------------------------------ */

void alert_enter_nebula (int x,int nn)
{
	//Notify the console of the details
	do_console_notify(x, console_helm, console_tactical, 0,
	  ansi_warn(tprintf("ENTERING NEBULA %s: Type: %s",Name(sdb[nn].object),unparse_class(nn))));
	//Notify the ship of the tremors
	do_ship_notify(x, tprintf("%s shakes a gentle bit as the inertial dampers kick in.", Name(sdb[x].object)));
	return;
}

/* ------------------------------------------------------------------------ */

void alert_exit_nebula (int x,int nn)
{
	//Notify the console of the details
	do_console_notify(x, console_helm, console_tactical, 0,
	  ansi_warn(tprintf("EXITING NEBULA %s: Type: %s",Name(sdb[nn].object),unparse_class(nn))));
	//Notify the ship of the tremors
	do_ship_notify(x, tprintf("%s shakes a gentle bit and then smooths out.", Name(sdb[x].object)));
	return;}

/* ------------------------------------------------------------------------ */

void alert_warp_overload (int x)
{
	do_console_notify(x, console_engineering, console_damage, 0,
	  ansi_warn("WARP DRIVE BURNOUT: Warp drive now offline"));
	return;
}

/* ------------------------------------------------------------------------ */

void alert_impulse_overload (int x)
{
	do_console_notify(x, console_engineering, console_damage, 0,
	  ansi_warn("IMPULSE DRIVE BURNOUT: Impulse drive now offline"));
	return;
}

/* ------------------------------------------------------------------------ */

void alert_anti_runout (int x)
{
	do_console_notify(x, console_engineering, 0, 0,
	  ansi_warn("ANTIMATTER DEPLETION: M/A reactor now offline"));
	return;
}

/* ------------------------------------------------------------------------ */

void alert_deut_runout (int x)
{
	do_console_notify(x, console_engineering, 0, 0,
	  ansi_warn("DEUTERIUM DEPLETION: All reactors now offline"));
	return;
}

/* ------------------------------------------------------------------------ */

void alert_batt_runout (int x)
{
	do_console_notify(x, console_engineering, 0, 0,
	  ansi_warn("BATTERY DEPLETION: Batteries now offline"));
	return;
}

/* ------------------------------------------------------------------------ */

void alert_cloak_failure (int x)
{
	do_console_notify(x, console_helm, console_tactical, 0,
	  ansi_warn("Insufficient power: Cloaking device disengaged"));
	return;
}

/* ------------------------------------------------------------------------ */

void alert_cloak_voided (int x)
{
	do_console_notify(x, console_helm, console_tactical, 0,
	  ansi_warn("Cloaking device voided. Deactivating"));
	return;
}

/* ------------------------------------------------------------------------ */

void alert_beam_balance (int x)
{
	do_console_notify(x, console_tactical, 0, 0,
	  ansi_alert("Beam capacitor power balanced"));
	return;
}

/* ------------------------------------------------------------------------ */

void alert_beam_charged (int x)
{
	do_console_notify(x, console_tactical, 0, 0,
	  ansi_alert("Beam capacitor fully charged"));
	return;
}

/* ------------------------------------------------------------------------ */

void alert_missile_balance (int x)
{
	do_console_notify(x, console_tactical, 0, 0,
	  ansi_alert("Missile capacitor power balanced"));
	return;
}

/* ------------------------------------------------------------------------ */

void alert_missile_charged (int x)
{
	do_console_notify(x, console_tactical, 0, 0,
	  ansi_alert("Missile capacitor fully charged"));
	return;
}

/* ------------------------------------------------------------------------ */

void alert_speed_warp (int x)
{
	do_console_notify(x, console_helm, console_engineering, 0,
	  ansi_alert(tprintf("Speed now warp %.6f", sdb[x].move.out)));
	return;
}

/* ------------------------------------------------------------------------ */

void alert_speed_impulse (int x)
{
	do_console_notify(x, console_helm, console_engineering, 0,
	  ansi_alert(tprintf("Speed now %.3f%% impulse", sdb[x].move.out * 100.0)));
	return;
}

/* ------------------------------------------------------------------------ */

void alert_speed_stop (int x)
{
	do_console_notify(x, console_helm, console_engineering, 0,
	  ansi_alert("Speed now full stop"));
	return;
}

/* ------------------------------------------------------------------------ */

void alert_enter_quadrant (int x)
{
	do_console_notify(x, console_helm, 0, 0,
	  ansi_alert(tprintf("Entering %s quadrant", unparse_quadrant(x))));
	return;
}

/* ------------------------------------------------------------------------ */

void alert_enter_empire (int x)
{
	do_console_notify(x, console_helm, 0, 0,
	  ansi_warn(tprintf("Entering %s space", unparse_empire(x))));
	return;
}

/* ------------------------------------------------------------------------ */

void alert_exit_empire (int x)
{
	do_console_notify(x, console_helm, 0, 0,
	  ansi_alert(tprintf("Departing %s space", unparse_empire(x))));
	return;
}

/* ------------------------------------------------------------------------ */

void alert_yaw (int x)
{
	do_console_notify(x, console_helm, 0, 0,
	  ansi_alert(tprintf("Yaw now %.3f degrees", sdb[x].course.yaw_out)));
	return;
}

/* ------------------------------------------------------------------------ */

void alert_pitch (int x)
{
	do_console_notify(x, console_helm, 0, 0,
	  ansi_alert(tprintf("Pitch now %.3f degrees", sdb[x].course.pitch_out)));
	return;
}

/* ------------------------------------------------------------------------ */

void alert_roll (int x)
{
	do_console_notify(x, console_helm, 0, 0,
	  ansi_alert(tprintf("Roll now %.3f degrees", sdb[x].course.roll_out)));
	return;
}

/* ------------------------------------------------------------------------ */

void alert_max_repair (int x)
{
	do_console_notify(x, console_damage, 0, 0,
		ansi_alert("Repair capacity maximized"));
	return;
}

/* ------------------------------------------------------------------------ */

void alert_border_cross (int x, int a, int way)
{
	register int i;
	double r;

	if (sdb[x].move.out != 0.0)
		for (i = MIN_SPACE_OBJECTS; i <= max_space_objects; ++i)
			if (sdb[i].status.active)
				 if (sdb[i].space == sdb[x].space)
					if (sdb[i].move.empire == a)
						if (i != x)
							if (sdb2range(x, i) < MAX_NOTIFICATION_DISTANCE){
								if (way) {
									do_console_notify(i, console_helm, console_science, console_security,
									  ansi_warn(tprintf("Inbound border crossing reported at %.3f %.3f %.3f",
									  su2pc(sdb[x].coords.x - sdb[i].coords.xo),
									  su2pc(sdb[x].coords.y - sdb[i].coords.yo),
									  su2pc(sdb[x].coords.z - sdb[i].coords.zo))));
								} else {
									do_console_notify(i, console_helm, console_science, console_security,
									  ansi_warn(tprintf("Outbound border crossing reported at %.3f %.3f %.3f",
									  su2pc(sdb[x].coords.x - sdb[i].coords.xo),
									  su2pc(sdb[x].coords.y - sdb[i].coords.yo),
									  su2pc(sdb[x].coords.z - sdb[i].coords.zo))));
									  }}

	return;
}

/* ------------------------------------------------------------------------ */

void alert_tract_unlock (int n1, int n2, dbref enactor)
{
	do_console_notify(n1, console_operation, console_helm, console_science,
	  ansi_cmd(enactor, tprintf("Tractor beam unlocked from %s",
	  unparse_identity(n1, n2))));
	do_console_notify(n2, console_operation, console_helm, console_science,
	  ansi_alert(tprintf("Tractor beam from %s unlocked",
	  unparse_identity(n2, n1))));
	
	do_space_notify_two(n1, n2, console_operation, console_helm, console_science,
	  "has unlocked tractor beam from");
	  
	do_ship_notify(n2, tprintf("%s is released from a tractor beam.", Name(sdb[n2].object)));

	return;
}

/* ------------------------------------------------------------------------ */

void alert_tract_lock (int n1, int n2, dbref enactor)
{
	do_console_notify(n1, console_operation, console_helm, console_science,
	  ansi_cmd(enactor, tprintf("Tractor beam locked on %s",
	  unparse_identity(n1, n2))));
	do_console_notify(n2, console_operation, console_helm, console_science,
	  ansi_alert(tprintf("Tractor beam locked from %s",
	  unparse_identity(n2, n1))));
	do_space_notify_two(n1, n2, console_operation, console_helm, console_science,
	  "has locked tractor beam on");
	do_ship_notify(n2, tprintf("%s is seized by a tractor beam.",
	  Name(sdb[n2].object)));
	return;
}

/* ------------------------------------------------------------------------ */

void alert_tract_attempt (int n1, int n2, dbref enactor)
{
	do_console_notify(n1, console_operation, console_helm, console_science,
	  ansi_cmd(enactor, tprintf("Tractor beam lock on %s attempted and failed",
	  unparse_identity(n1, n2))));
	do_console_notify(n2, console_operation, console_helm, console_science,
	  ansi_alert(tprintf("Tractor beam lock from %s attempted and failed",
	  unparse_identity(n2, n1))));
	do_space_notify_two(n1, n2, console_operation, console_helm, console_science,
	  "attempts and fails to lock tractor beam on");
	return;
}

/* ------------------------------------------------------------------------ */

void alert_tract_lost (int n1, int n2)
{
	do_console_notify(n1, console_operation, console_helm, console_science,
	  ansi_alert(tprintf("Tractor beam lock on %s lost",
	  unparse_identity(n1, n2))));
	do_console_notify(n2, console_operation, console_helm, console_science,
	  ansi_alert(tprintf("Tractor beam lock from %s lost",
	  unparse_identity(n2, n1))));
	do_space_notify_two(n1, n2, console_operation, console_helm, console_science,
	  "has lost tractor beam lock on");
	do_ship_notify(n2, tprintf("%s is released from a tractor beam.",
	  Name(sdb[n2].object)));
	return;
}

/* ------------------------------------------------------------------------ */

void alert_ship_cloak_online (int x)
{
	do_ship_notify(x, tprintf("%s engages its cloaking device.",
	  Name(sdb[x].object)));
	do_space_notify_one(x, console_helm, console_tactical, console_science,
	  "engages its cloaking device");
	return;
}

/* ------------------------------------------------------------------------ */

void alert_ship_cloak_offline (int x)
{
	do_ship_notify(x, tprintf("%s disengages its cloaking device.",
	  Name(sdb[x].object)));
	do_space_notify_one(x, console_helm, console_tactical, console_science,
	  "disengages its cloaking device");
	return;
}

/* ------------------------------------------------------------------------ */

void alert_ship_enter_warp (int x)
{
	do_ship_notify(x, tprintf("%s shifts into warp.", Name(sdb[x].object)));
	return;
}

/* ------------------------------------------------------------------------ */

void alert_ship_exit_warp (int x)
{
	do_ship_notify(x, tprintf("%s drops out of warp.", Name(sdb[x].object)));
	return;
}

/* ------------------------------------------------------------------------ */

void alert_ship_hit (int x)
{

	if (sdb[x].structure.type == 1 || sdb[x].structure.type == 2)
	
		switch (getrandom(10) + 1) {
					case 2: do_ship_notify(x, tprintf("%s rocks sharply from a weapons impact.",Name(sdb[x].object))); break;
					case 3: do_ship_notify(x, tprintf("%s rocks sharply from a weapons impact.",Name(sdb[x].object))); break;
					case 4: do_ship_notify(x, tprintf("%s shudders from a weapons impact.",Name(sdb[x].object))); break;
					case 5: do_ship_notify(x, tprintf("%s hull cracks as its struck from a blow.",Name(sdb[x].object))); break;
					case 6: do_ship_notify(x, tprintf("%s lights go dim for a moment and then come back up after a impact.",Name(sdb[x].object))); break;
					case 7: do_ship_notify(x, tprintf("%s lights go dim for a moment and then come back up after a impact.",Name(sdb[x].object))); break;
					case 8: do_ship_notify(x, tprintf("%s has panels fly off the wall with smoke after as an impact strikes home.",Name(sdb[x].object))); break;
					case 9: do_ship_notify(x, tprintf("%s shudders as another blast strikes home.",Name(sdb[x].object))); break;
					case 10: do_ship_notify(x, tprintf("%s feels the brunt of another beam attack.",Name(sdb[x].object))); break;
					default: do_ship_notify(x, tprintf("%s shudders from an impact.",Name(sdb[x].object))); break;
					}
		
	return;
}

/* ------------------------------------------------------------------------ */

void alert_ship_hurt (int x)
{
	if (sdb[x].structure.type == 1 || sdb[x].structure.type == 2) {
		switch (getrandom(10) + 1) {
					case 2: do_ship_notify(x, tprintf("%s rocks sharply from a weapons impact.",Name(sdb[x].object))); break;
					case 3: do_ship_notify(x, tprintf("%s rocks sharply from a weapons impact.",Name(sdb[x].object))); break;
					case 4: do_ship_notify(x, tprintf("%s shudders from a weapons impact.",Name(sdb[x].object))); break;
					case 5: do_ship_notify(x, tprintf("%s hull cracks as its struck from a blow.",Name(sdb[x].object))); break;
					case 6: do_ship_notify(x, tprintf("%s lights go dim for a moment and then come back up after a impact.",Name(sdb[x].object))); break;
					case 7: do_ship_notify(x, tprintf("%s lights go dim for a moment and then come back up after a impact.",Name(sdb[x].object))); break;
					case 8: do_ship_notify(x, tprintf("%s has panels fly off the wall with smoke after as an impact strikes home.",Name(sdb[x].object))); break;
					case 9: do_ship_notify(x, tprintf("%s shudders as another blast strikes home.",Name(sdb[x].object))); break;
					case 10: do_ship_notify(x, tprintf("%s feels the brunt of another beam attack.",Name(sdb[x].object))); break;
					default: do_ship_notify(x, tprintf("%s rocks violently from an impact.",Name(sdb[x].object))); break;
					}
		
	} else if (sdb[x].structure.type == 3 && getrandom(10) == 1)
		
		do_ship_notify(x, tprintf("%s trembles from a surface impact.",
	  	  Name(sdb[x].object)));
	return;
}

/* ------------------------------------------------------------------------ */
