/* space_misc.c */

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
#include "confmagic.h"
#include "space.h"
#include "log.h"
#include "attrib.h"
#include "dbdefs.h"
#include "flags.h"
//FOR MYSQL SUPPORT
 #include <mysql.h>
 #include <errmsg.h>
//END MYSQL SUPPORT


/* ------------------------------------------------------------------------ */
//#define SpaceObj(x) (IS(x, TYPE_THING, THING_SPACE_OBJECT)) THIS IS THE OLD FLAG SYSTEM
#define SpaceObj(x)       (IS(x, TYPE_THING, "SPACE_OBJECT"))

extern time_t mudtime;
/* ------------------------------------------------------------------------ */
//Define the Functions
int check_inside _((float aax, float aay, float aaz, float myrad,int secondship));
void do_selfdestruct _((int x, int silent, int destruction));
void do_abortselfdestruct _((int x));
void do_eject_warpcore _((int x));
int get_selfdestruct_timer _((int x));
void check_component _((int x, char *query,dbref enactor,char *buff,char *bp));
void damage_component _((x));

// END FUNCTION DEFINE


void check_component(int x, char *query,dbref enactor,char *buff,char *bp) {
	char *queryreturn; 


queryreturn=sql_query_internal(enactor, query, buff, bp);

	//notify(enactor, tprintf("%s",queryreturn));
	//notify(enactor, tprintf("object %s",unparse_dbref(sdb[x].object)));









}

void damage_structure (int x, double damage)
{
	register int i;
	int inside,t;
	double s = sdb[x].structure.superstructure;

	if (sdb[x].structure.superstructure == -sdb[x].structure.max_structure)
		return;
	sdb[x].structure.superstructure -= damage;
	if (sdb[x].structure.superstructure < -sdb[x].structure.max_structure)
		sdb[x].structure.superstructure = -sdb[x].structure.max_structure;
	do_console_notify(x, console_engineering, console_damage, 0,
	  ansi_alert(tprintf("%s%s%s: %s %s",
	  ANSI_CYAN, system_name[0], ANSI_WHITE,
	  unparse_percent(sdb[x].structure.superstructure / sdb[x].structure.max_structure),
	  unparse_damage(sdb[x].structure.superstructure / sdb[x].structure.max_structure))));
	// This is the death loop
	if ((sdb[x].structure.superstructure <= -sdb[x].structure.max_structure) &&
	  (s > -sdb[x].structure.max_structure)) {
		/* Mordak Aspace v1.0.0p1 - Removed Silly KayBye! */
		do_ship_notify(x, tprintf("%s%s%s%s explodes into white hot vapor.%s%s",
		  ANSI_HILITE, ANSI_INVERSE, ANSI_RED, Name(sdb[x].object), ANSI_WHITE, ANSI_NORMAL));
		/* End Aspace v1.0.0p1*/
		do_space_notify_one(x, console_helm, console_tactical, console_science,
		  "has been destroyed");


			//This appears to be the loop where we kill things
			//that are close to the dead ship let me fix 
			for (t = MIN_SPACE_OBJECTS ; t <= max_space_objects ; ++t) {
				do_ship_notify(1, tprintf("%i X%f Y %f Z%f",t,sdb[t].coords.x,sdb[t].coords.y,sdb[t].coords.z));
				do_ship_notify(1, tprintf("ID %i\nStructure Type:%i\nSpace %i\n",t,sdb[t].structure.type,sdb[t].space));
				
						//We only want to kill ships or bases!
				if (((sdb[t].structure.type == 1 || sdb[t].structure.type ==2)) && sdb[t].space == sdb[x].space && t != x) {
							inside= check_inside(sdb[x].coords.x,sdb[x].coords.y,sdb[x].coords.z,SELF_DESTRUCT_RANGE,t);
							
							if (inside == 1) {
									//do_ship_notify(1, tprintf("YUP %i",t));
								do_space_notify_one(t, console_helm, console_tactical, console_science,
									"has been crippled");
									do_ship_notify(t, tprintf("%s%s%s%s hit by the shockwave and crippled.%s%s",
									  ANSI_HILITE, ANSI_INVERSE, ANSI_RED, Name(sdb[t].object), ANSI_WHITE, ANSI_NORMAL));
									do_log(LT_SPACE, sdb[t].object, sdb[t].object, tprintf("LOG: Crippled in blast, Shields %.6f GHz",
									  sdb[t].shield.freq));
									//Lets just cripple them for now
									//sdb[t].space = -1;
									//sdb[t].status.active = 0;
									sdb[t].structure.superstructure = -50;
									sdb[t].status.crippled = 1;	
							}
						}
					}

		//Now that we've killed ships close lets kill the original ship
		do_log(LT_SPACE, sdb[x].object, sdb[x].object, tprintf("LOG: Destroyed, Shields %.6f GHz",
		sdb[x].shield.freq));
		sdb[x].space = -1;
		sdb[x].status.active = 0;
		sdb[x].status.crippled = 2;
		

	} else if ((sdb[x].structure.superstructure <= 0.0) && (s > 0.0)) {
		do_all_console_notify(x, ansi_warn("Excessive damage. All systems shutting down"));
		do_ship_notify(x, tprintf("%s experiences total systems failure.",Name(sdb[x].object)));
		sdb[x].status.crippled = 1;
		do_space_notify_one(x, console_helm, console_tactical, console_science,
			"has been disabled");
		do_log(LT_SPACE, sdb[x].object, sdb[x].object, tprintf("LOG: Disabled, Shields %.6f GHz",
		  sdb[x].shield.freq));
	}
	if (((sdb[x].structure.superstructure <= 0.0) && (s > 0.0)) ||
	  ((sdb[x].structure.superstructure <= -sdb[x].structure.max_structure) &&
	  (s > -sdb[x].structure.max_structure))) {
	  	if (sdb[x].main.damage > 0.0)
			sdb[x].main.in = 0.0;
	  	if (sdb[x].aux.damage > 0.0)
			sdb[x].aux.in = 0.0;
		for (i = 0; i < MAX_SHIELD_NAME; ++i)
			sdb[x].shield.active[i] = 0;
		sdb[x].beam.in = 0.0;
		sdb[x].beam.out = 0.0;
		for (i = 0; i < sdb[x].beam.banks; ++i) {
			sdb[x].blist.lock[i] = 0;
			sdb[x].blist.active[i] = 0;
		}
		sdb[x].missile.in = 0.0;
		sdb[x].missile.out = 0.0;
		for (i = 0; i < sdb[x].missile.tubes; ++i) {
			sdb[x].mlist.lock[i] = 0;
			sdb[x].mlist.active[i] = 0;
		}
		sdb[x].batt.in = 0.0;
		sdb[x].batt.out = 0.0;
		sdb[x].move.in = 0.0;
		sdb[x].move.out = 0.0;
		sdb[x].move.v = 0.0;
		sdb[x].engine.warp_max = 0.0;
		sdb[x].engine.impulse_max = 0.0;
		sdb[x].power.batt = 0.0;
		sdb[x].sensor.lrs_active = 0;
		sdb[x].sensor.srs_active = 0;
		sdb[x].sensor.ew_active = 0;
		sdb[x].cloak.active = 0;
		sdb[x].trans.active = 0;
		sdb[x].trans.d_lock = 0;
		sdb[x].trans.s_lock = 0;
		sdb[x].tract.active = 0;
		sdb[x].tract.lock = 0;
		if (GoodSDB(sdb[x].status.tractoring)) {
			sdb[sdb[x].status.tractoring].status.tractored = 0;
			sdb[x].status.tractoring = 0;
		}
		up_cochranes();
		up_empire();
		up_quadrant();
		up_vectors();
		up_resolution();
		up_signature(x);
		up_visibility();
		debug_space(x);
	}
	return;
}

/* ------------------------------------------------------------------------ */

void damage_aux (int x, double damage, int mydmg)
{
	if (!sdb[x].aux.exist || sdb[x].aux.damage == -1.0)
		return;
	if (sdb[x].power.aux != 0.0
	  && sdb[x].aux.damage > 0.0
	  && (sdb[x].aux.damage - damage / sdb[x].aux.gw <= 0.0)) {
		alert_aux_overload(x);
	}
	sdb[x].aux.damage -= damage / sdb[x].aux.gw;
	
	if (sdb[x].aux.damage < -1.0) {
		sdb[x].aux.damage = -1.0;
		if (sdb[x].power.aux != 0.0) {
			do_all_console_notify(x, tprintf(ansi_warn("%s core breach."), system_name[1]));
			damage_structure(x, sdb[x].power.aux * (getrandom(10) + 1.0));
		}
	} 

		/* Lets set the last weapon hit MOdified by bill 4-12-2002*/
		sdb[x].aux.lasthit=mydmg;

	

	do_console_notify(x, console_engineering, console_damage, 0,
	  ansi_alert(tprintf("%s%s%s: %s %s",
	  ANSI_CYAN, system_name[1], ANSI_WHITE,
	  unparse_percent(sdb[x].aux.damage),
	  unparse_damage(sdb[x].aux.damage))));

	return;
}

/* ------------------------------------------------------------------------ */

void damage_batt (int x, double damage, int mydmg)
{
	if (!sdb[x].batt.exist || sdb[x].batt.damage == -1.0)
		return;
	sdb[x].batt.damage -= damage / sdb[x].batt.gw;
	if (sdb[x].batt.damage < -1.0)
		sdb[x].batt.damage = -1.0;
		
	do_console_notify(x, console_engineering, console_damage, 0,
	  ansi_alert(tprintf("%s%s%s: %s %s",
	  ANSI_CYAN, system_name[2], ANSI_WHITE,
	  unparse_percent(sdb[x].batt.damage),
	  unparse_damage(sdb[x].batt.damage))));

		/* Lets set the last weapon hit MOdified by bill 4-12-2002*/
		sdb[x].batt.lasthit=mydmg;


	if (sdb[x].batt.damage <= 0.0) {
		sdb[x].batt.in = 0.0;
		sdb[x].batt.out = 0.0;
		sdb[x].power.batt = 0.0;
		sdb[x].power.version = 1;
	}
	return;
}

/* ------------------------------------------------------------------------ */

void damage_beam (int x, int beam, double damage, int mydmg)
{
	if (!sdb[x].beam.exist || sdb[x].blist.damage[beam] == -1.0)
		return;
	sdb[x].blist.damage[beam] -= damage
	  / ((sdb[x].blist.cost[beam] + sdb[x].blist.bonus[beam]) / 10.0);
	if (sdb[x].blist.damage[beam] < -1.0)
		sdb[x].blist.damage[beam] = -1.0;
		
	do_console_notify(x, console_tactical, console_damage, 0,
	  ansi_alert(tprintf("%s%s %d%s: %s %s",
	  ANSI_CYAN, system_name[3], beam + 1, ANSI_WHITE,
	  unparse_percent(sdb[x].blist.damage[beam]),
	  unparse_damage(sdb[x].blist.damage[beam]))));

		/* Lets set the last weapon hit MOdified by bill 4-12-2002*/
		sdb[x].beam.lasthit=mydmg;


	if (sdb[x].blist.damage[beam] <= 0.0)
		if (sdb[x].blist.active[beam]) {
			sdb[x].beam.in -= 10.0 * sdb[x].blist.cost[beam];
			sdb[x].blist.active[beam] = 0;
			sdb[x].blist.lock[beam] = 0;
		}
	return;
}

/* ------------------------------------------------------------------------ */

void damage_cloak (int x, double damage, int mydmg)
{
	if (!sdb[x].cloak.exist || sdb[x].cloak.damage == -1.0)
		return;
	sdb[x].cloak.damage -= damage / (1.0 + (sdb[x].structure.max_structure / 100.0));
	if (sdb[x].cloak.damage < -1.0)
		sdb[x].cloak.damage = -1.0;
		

		
	do_console_notify(x, console_helm, console_damage, 0,
	  ansi_alert(tprintf("%s%s%s: %s %s",
	  ANSI_CYAN, system_name[4], ANSI_WHITE,
	  unparse_percent(sdb[x].cloak.damage),
	  unparse_damage(sdb[x].cloak.damage))));
	if (sdb[x].cloak.damage <= 0.0)
		if (sdb[x].cloak.active) {
			sdb[x].cloak.active = 0;
			sdb[x].engine.version = 1;
			alert_ship_cloak_offline(x);
		}

		/* Lets set the last weapon hit MOdified by bill 4-12-2002*/
		sdb[x].cloak.lasthit=mydmg;


	sdb[x].sensor.version = 1;
	return;
}

/* ------------------------------------------------------------------------ */

void damage_ew (int x, double damage, int mydmg)
{
	if (!sdb[x].sensor.ew_exist || sdb[x].sensor.ew_damage == -1.0)
		return;
	sdb[x].sensor.ew_damage -= damage / (1.0 + (sdb[x].structure.max_structure / 10.0));
	if (sdb[x].sensor.ew_damage < -1.0)
		sdb[x].sensor.ew_damage = -1.0;
	do_console_notify(x, console_tactical, console_science, console_damage,
	  ansi_alert(tprintf("%s%s%s: %s %s",
	  ANSI_CYAN, system_name[5], ANSI_WHITE,
	  unparse_percent(sdb[x].sensor.ew_damage),
	  unparse_damage(sdb[x].sensor.ew_damage))));
	if (sdb[x].sensor.ew_damage <= 0.0)
		if (sdb[x].sensor.ew_active)
			sdb[x].sensor.ew_active = 0;
	sdb[x].sensor.version = 1;
		/* Lets set the last weapon hit MOdified by bill 4-12-2002*/
		sdb[x].sensor.lasthit=mydmg;
	return;
}

/* ------------------------------------------------------------------------ */

void damage_impulse (int x, double damage, int mydmg)
{
	if (!sdb[x].engine.impulse_exist || sdb[x].engine.impulse_damage == -1.0)
		return;
	sdb[x].engine.impulse_damage -= damage / (1.0 + (sdb[x].structure.max_structure / 10.0));
	if (sdb[x].engine.impulse_damage < -1.0)
		sdb[x].engine.impulse_damage = -1.0;
	do_console_notify(x, console_engineering, console_damage, 0,
	  ansi_alert(tprintf("%s%s%s: %s %s",
	  ANSI_CYAN, system_name[6], ANSI_WHITE,
	  unparse_percent(sdb[x].engine.impulse_damage),
	  unparse_damage(sdb[x].engine.impulse_damage))));
	sdb[x].engine.version = 1;
		/* Lets set the last weapon hit MOdified by bill 4-12-2002*/
		sdb[x].engine.lasthit=mydmg;
	return;
}




/* ------------------------------------------------------------------------ */

void damage_lrs (int x, double damage, int mydmg)
{
	if (!sdb[x].sensor.lrs_exist || sdb[x].sensor.lrs_damage == -1.0)
		return;
	sdb[x].sensor.lrs_damage -= damage / (1.0 + (sdb[x].structure.max_structure / 10.0));
	if (sdb[x].sensor.lrs_damage < -1.0)
		sdb[x].sensor.lrs_damage = -1.0;
	do_console_notify(x, console_tactical, console_science, console_damage,
	  ansi_alert(tprintf("%s%s%s: %s %s",
	  ANSI_CYAN, system_name[7], ANSI_WHITE,
	  unparse_percent(sdb[x].sensor.lrs_damage),
	  unparse_damage(sdb[x].sensor.lrs_damage))));
	if (sdb[x].sensor.lrs_damage <= 0.0)
		if (sdb[x].sensor.lrs_active)
			sdb[x].sensor.lrs_active = 0;
	sdb[x].sensor.version = 1;
		/* Lets set the last weapon hit MOdified by bill 4-12-2002*/
		sdb[x].sensor.lasthit=mydmg;
	return;
}




/* ------------------------------------------------------------------------ */


void damage_main (int x, double damage, int mydmg)
{
	if (!sdb[x].main.exist || sdb[x].main.damage == -1.0)
		return;
	if (sdb[x].power.main != 0.0
	  && sdb[x].main.damage > 0.0
  	  && (sdb[x].main.damage - damage / sdb[x].main.gw <= 0.0)) {
		alert_main_overload(x);
	}
	sdb[x].main.damage -= damage / sdb[x].main.gw;
	if (sdb[x].main.damage < -1.0) {
		sdb[x].main.damage = -1.0;
		if (sdb[x].power.main != 0.0) {
			do_all_console_notify(x, tprintf(ansi_warn("%s core breach."), system_name[8]));
			damage_structure(x, sdb[x].power.main * (getrandom(100) + 1.0));
		}
	}
	do_console_notify(x, console_engineering, console_damage, 0,
	  ansi_alert(tprintf("%s%s%s: %s %s",
	  ANSI_CYAN, system_name[8], ANSI_WHITE,
	  unparse_percent(sdb[x].main.damage),
	  unparse_damage(sdb[x].main.damage))));
		/* Lets set the last weapon hit MOdified by bill 4-12-2002*/
		sdb[x].main.lasthit=mydmg;
	return;
}


/* ------------------------------------------------------------------------ */

void damage_missile (int x, int missile, double damage, int mydmg)
{
	if (!sdb[x].missile.exist || sdb[x].mlist.damage[missile] == -1.0)
		return;
	sdb[x].mlist.damage[missile] -= damage
	  / (sdb[x].mlist.warhead[missile] / 10.0);
	if (sdb[x].mlist.damage[missile] < -1.0)
		sdb[x].mlist.damage[missile] = -1.0;
	do_console_notify(x, console_tactical, console_damage, 0,
	  ansi_alert(tprintf("%s%s %d%s: %s %s",
	  ANSI_CYAN, system_name[9], missile + 1, ANSI_WHITE,
	  unparse_percent(sdb[x].mlist.damage[missile]),
	  unparse_damage(sdb[x].mlist.damage[missile]))));
	if (sdb[x].mlist.damage[missile] <= 0.0)
		if (sdb[x].mlist.active[missile]) {
			sdb[x].missile.in -= sdb[x].mlist.cost[missile];
			sdb[x].mlist.active[missile] = 0;
			sdb[x].mlist.lock[missile] = 0;
		}
		/* Lets set the last weapon hit MOdified by bill 4-12-2002*/
		sdb[x].missile.lasthit=mydmg;
	return;
}


/* ------------------------------------------------------------------------ */

void damage_shield (int x, int shield, double damage, int mydmg)
{
	if (!sdb[x].shield.exist || sdb[x].shield.damage[shield] == -1.0)
		return;
	sdb[x].shield.damage[shield] -= damage / (1.0 + (sdb[x].structure.max_structure / 10.0));
	if (sdb[x].shield.damage[shield] < -1.0)
		sdb[x].shield.damage[shield] = -1.0;
	do_console_notify(x, console_helm, console_damage, 0,
	  ansi_alert(tprintf("%s%s%s: %s %s",
	  ANSI_CYAN, unparse_shield(shield), ANSI_WHITE,
	  unparse_percent(sdb[x].shield.damage[shield]),
	  unparse_damage(sdb[x].shield.damage[shield]))));
	if (sdb[x].shield.damage[shield] <= 0.0) {
		sdb[x].shield.active[shield] = 0;
		sdb[x].engine.version = 1;
	}
		/* Lets set the last weapon hit MOdified by bill 4-12-2002*/
		sdb[x].shield.lasthit=mydmg;
	return;
}


/* ------------------------------------------------------------------------ */

void damage_srs (int x, double damage, int mydmg)
{
	if (!sdb[x].sensor.srs_exist || sdb[x].sensor.srs_damage == -1.0)
		return;
	sdb[x].sensor.srs_damage -= damage / (1.0 + (sdb[x].structure.max_structure / 10.0));
	if (sdb[x].sensor.srs_damage < -1.0)
		sdb[x].sensor.srs_damage = -1.0;
	do_console_notify(x, console_tactical, console_science, console_damage,
	  ansi_alert(tprintf("%s%s%s: %s %s",
	  ANSI_CYAN, system_name[11], ANSI_WHITE,
	  unparse_percent(sdb[x].sensor.srs_damage),
	  unparse_damage(sdb[x].sensor.srs_damage))));
	if (sdb[x].sensor.srs_damage <= 0.0)
		if (sdb[x].sensor.srs_active)
			sdb[x].sensor.srs_active = 0;
	sdb[x].sensor.version = 1;
		/* Lets set the last weapon hit MOdified by bill 4-12-2002*/
		sdb[x].sensor.lasthit=mydmg;
	return;
}

/* ------------------------------------------------------------------------ */

void damage_tract (int x, double damage, int mydmg)
{
	if (!sdb[x].tract.exist || sdb[x].tract.damage == -1.0)
		return;
	sdb[x].tract.damage -= damage / (1.0 + (sdb[x].structure.max_structure / 10.0));
	if (sdb[x].tract.damage < -1.0)
		sdb[x].tract.damage = -1.0;
	do_console_notify(x, console_operation, console_damage, 0,
	  ansi_alert(tprintf("%s%s%s: %s %s",
	  ANSI_CYAN, system_name[12], ANSI_WHITE,
	  unparse_percent(sdb[x].tract.damage),
	  unparse_damage(sdb[x].tract.damage))));
	if (sdb[x].tract.damage <= 0.0)
		if (sdb[x].tract.active) {
			if (sdb[x].status.tractoring) {
				alert_tract_lost(x, sdb[x].status.tractoring);
				sdb[x].tract.lock = 0;
				sdb[sdb[x].status.tractoring].status.tractored = 0;
				sdb[sdb[x].status.tractoring].power.version = 1;
				sdb[x].status.tractoring = 0;
			}
			sdb[x].tract.active = 0;
		}
	sdb[x].power.version = 1;
		/* Lets set the last weapon hit MOdified by bill 4-12-2002*/
		sdb[x].tract.lasthit=mydmg;
	return;
}


/* ------------------------------------------------------------------------ */

void damage_trans (int x, double damage, int mydmg)
{
	if (!sdb[x].trans.exist || sdb[x].trans.damage == -1.0)
		return;
	sdb[x].trans.damage -= damage / (1.0 + (sdb[x].structure.max_structure / 10.0));
	if (sdb[x].trans.damage < -1.0)
		sdb[x].trans.damage = -1.0;
	do_console_notify(x, console_operation, console_damage, console_transporter,
	  ansi_alert(tprintf("%s%s%s: %s %s",
	  ANSI_CYAN, system_name[13], ANSI_WHITE,
	  unparse_percent(sdb[x].trans.damage),
	  unparse_damage(sdb[x].trans.damage))));
	if (sdb[x].trans.damage <= 0.0)
		if (sdb[x].trans.active) {
			sdb[x].trans.s_lock = 0;
			sdb[x].trans.d_lock = 0;
			sdb[x].trans.active = 0;
		}
		/* Lets set the last weapon hit MOdified by bill 4-12-2002*/
		sdb[x].trans.lasthit=mydmg;
	return;
}


/* ---------------Modified by bill 4-10-2002 -------------------------------------------- */

void damage_lifesupport (int x, double damage, int mydmg)
{

	if (!sdb[x].lifesupport.active || sdb[x].lifesupport.damage == -1.0)
		return;
	sdb[x].lifesupport.damage -= damage / (1.0 + (sdb[x].structure.max_structure / 10.0));
	if (sdb[x].lifesupport.damage < -1.0)
		sdb[x].lifesupport.damage = -1.0;
	do_console_notify(x, console_operation, console_damage, console_transporter,
	  ansi_alert(tprintf("%s%s%s: %s %s",
	  ANSI_CYAN, system_name[15], ANSI_WHITE,
	  unparse_percent(sdb[x].lifesupport.damage),
	  unparse_damage(sdb[x].lifesupport.damage))));
	if (sdb[x].lifesupport.damage <= .26 && sdb[x].lifesupport.damage >= .12)
		if (sdb[x].lifesupport.active) {
		/* Here we would place the you're going to die code ;) */
			sdb[x].lifesupport.active = 1;	
			/*sdb[x].object*/
			/*do_zemit("#5", "#1132",  "Life Support Offlne in 5 minutes");*/
	
	
		do_ship_notify(x, tprintf("%s%s%sComputer Says: \"Warning! Life Support at 25%% and falling.\"%s%s",
		  ANSI_HILITE, ANSI_RED, ANSI_BLINK, ANSI_WHITE, ANSI_NORMAL));	
		}
	if (sdb[x].lifesupport.damage <= .11 && sdb[x].lifesupport.damage <= .01)
		if (sdb[x].lifesupport.active) {
		/* Here we would place the you're going to die code ;) */
			sdb[x].lifesupport.active = 1;	
			/*sdb[x].object*/
		do_ship_notify(x, tprintf("%s%s%sComputer Says: \"Warning! Life Support at 10%% and falling.\"%s%s",
		  ANSI_HILITE, ANSI_RED, ANSI_BLINK, ANSI_WHITE, ANSI_NORMAL));		
		}
	if (sdb[x].lifesupport.damage <= 0.0)
		if (sdb[x].lifesupport.active) {
		/* Here we would place the you're going to die code ;) */
			sdb[x].lifesupport.active = 0;	
			sdb[x].lifesupport.time = 0;	
			/*sdb[x].object*/
		/* Let everyone know they have 5 minutes of life left ;) */
		do_ship_notify(x, tprintf("%s%s%sComputer Says: \"Warning! Life Support Destroyed. Oyxgen Levels at 95%%. Ambient Room Tempature at 70 degress and falling.\"%s%s",
		  ANSI_HILITE, ANSI_RED, ANSI_BLINK, ANSI_WHITE, ANSI_NORMAL));		
			
		/* Death count is in space_iterate to start the count*/
			

		}
		
		
		
			/* Lets set the last weapon hit MOdified by bill 4-12-2002*/
		sdb[x].lifesupport.lasthit=mydmg;	
		
	return;
}



/* ------------------------------------------------------------------------ */

void damage_warp (int x, double damage, int mydmg)
{
	if (!sdb[x].engine.warp_exist || sdb[x].engine.warp_damage == -1.0)
		return;
	sdb[x].engine.warp_damage -= damage / (1.0 + (sdb[x].structure.max_structure / 10.0));
	if (sdb[x].engine.warp_damage < -1.0)
		sdb[x].engine.warp_damage = -1.0;
	do_console_notify(x, console_engineering, console_damage, 0,
	  ansi_alert(tprintf("%s%s%s: %s %s",
	  ANSI_CYAN, system_name[14], ANSI_WHITE,
	  unparse_percent(sdb[x].engine.warp_damage),
	  unparse_damage(sdb[x].engine.warp_damage))));
	sdb[x].engine.version = 1;
			/* Lets set the last weapon hit MOdified by bill 4-12-2002*/
		sdb[x].engine.lasthit=mydmg;	
	return;
}

/* ---------------FIRE CODE modified by bill 4-9-2002------------------- */

int do_set_fire (int first, int last, int weapon, int mode, dbref enactor)
{
	static char buff_n[1000], buff_x[MAX_SENSOR_CONTACTS][1000];
	register int a, b, i, j, k, x, flag = 0;
	int is_b_active = 0, is_b_lock = 0, is_b_load = 0, is_b_arm = 0, is_b_arc = 0, is_b_range = 0,is_b_nebula = 0;
	int is_m_active = 0, is_m_lock = 0, is_m_load = 0, is_m_arm = 0, is_m_arc = 0, is_m_range = 0,is_m_nebula = 0;
	double range, prob, dmg_b[MAX_BEAM_BANKS], dmg_m[MAX_MISSILE_TUBES];
	double ss, dis, pdmg, d_system[16], d_shield[MAX_SHIELD_NAME];
	double d_beam[MAX_BEAM_BANKS], d_missile[MAX_MISSILE_TUBES];
	int dmg_weap;
				
	if (error_on_console(enactor)) {
		return 0;
	} else if (weapon < 0 || weapon > 2 || mode < 0 || mode > 6) {
		return 0;
	} else if (weapon == 1 && !sdb[n].beam.exist) {
		notify(enactor, ansi_red(tprintf("%s has no %ss.", Name(sdb[n].object), system_name[3])));
		return 0;
	} else if (weapon == 2 && !sdb[n].missile.exist) {
		notify(enactor, ansi_red(tprintf("%s has no %ss.", Name(sdb[n].object), system_name[9])));
		return 0;
	} else if (!weapon && !sdb[n].beam.exist && !sdb[n].missile.exist) {
		notify(enactor, ansi_red(tprintf("%s has no weapons.", Name(sdb[n].object))));
		return 0;
	} else if (sdb[n].status.docked) {
		notify(enactor, ansi_red(tprintf("%s is in dock.", Name(sdb[n].object))));
		return 0;
	} else if (sdb[n].status.landed) {
		notify(enactor, ansi_red(tprintf("%s is on a landing pad.", Name(sdb[n].object))));
		return 0;
	} else if (sdb[n].cloak.active && sdb[n].tech.cloak < 2.0) {
		notify(enactor, ansi_red(tprintf("%s cannot fire weapons while cloaked.", Name(sdb[n].object))));
		return 0;
	}


	
	strncpy(buff_n, "", sizeof(buff_n) - 1);
	for (i = 0; i < sdb[n].sensor.contacts; ++i)
		strncpy(buff_x[i], "", sizeof(buff_x[i]) - 1);

	/* check beam weapon list */

	if (weapon == 1 || (!weapon && sdb[n].beam.exist)) {
		a = first;
		b = last;
		if (a < 1 || a > sdb[n].beam.banks) {
			a = 0;
			b = sdb[n].beam.banks - 1;
		} else {
			--a;
			--b;
		 	if (b >= sdb[n].beam.banks)
				b = sdb[n].beam.banks - 1;
			if (b < a)
				b = a;
		}
		for (i = 0; i < sdb[n].beam.banks; ++i)
			dmg_b[i] = 0.0;
		for (i = a; i <= b ; ++i) {
			if (sdb[n].blist.damage[i] <= 0.0)
				continue;
			if (!sdb[n].blist.active[i])
				continue;
			++is_b_active;
			x = sdb[n].blist.lock[i];
			if (!GoodSDB(x))
				continue;
			if (sdb[x].tmp.i0 != n) {
			    sdb[x].tmp.i0 = n;               /* initial marker */
			    sdb[x].tmp.i1 = sdb2slist(n, x); /* slist number */
			    sdb[x].tmp.i2 = 0;               /* firing arc to target */
			    sdb[x].tmp.i3 = 0;               /* facing shield of target */
			    sdb[x].tmp.i4 = 0;               /* multiple-hit flag for target */
			    sdb[x].tmp.i5 = 0;               /* hit flag of target */
			    sdb[x].tmp.d0 = 0.0;             /* range to target & shield damage */
			    sdb[x].tmp.d1 = 0.0;             /* target shield GW */
			    sdb[x].tmp.d2 = 0.0;             /* internal damage */
			}

			if (sdb[x].structure.type==6)
				continue;
			++is_b_nebula;				
			if (sdb[x].tmp.i1 == SENSOR_FAIL)
				continue;
			++is_b_lock;
			if (sdb[n].beam.out < sdb[n].blist.cost[i])
				continue;
			++is_b_arm;
			if (!sdb[x].tmp.i2)
				sdb[x].tmp.i2 = sdb2arc(n, x);
			if (arc_check(sdb[x].tmp.i2, sdb[n].blist.arcs[i]) == ARC_FAIL)
				continue;
			if (sdb[n].status.tractored)
				if (sdb[n].status.tractored != x)
					continue;
			++is_b_arc;
			/*  RANGE MODIFICATION CODE range = sdb[n].blist.range[i] * PARSEC / 10000.0; */
			if (sdb[x].tmp.d0 == 0.0)
				sdb[x].tmp.d0 = sdb2range(n, x);
			if (fabs(sdb[n].move.out) < 1.0) {
				/*range = sdb[n].blist.range[i];  Commented out by bill*/
				range = sdb[n].blist.range[i] * PARSEC / 8000.0;
			} else
				range = sdb[n].blist.range[i] * PARSEC / 8000.0;
			if (sdb[x].tmp.d0 > range * 10.0)
				continue;
			/* Modified by bill - to allow firing on ships that are stationary or moving */
			/*if (fabs(sdb[n].move.out) >= 1.0 && fabs(sdb[x].move.out) < 1.0)
			*	if (sdb[n].status.tractoring != x && sdb[n].status.tractored != x)
			*		continue;*/
					
			/*if (fabs(sdb[n].move.out) < 1.0 && fabs(sdb[x].move.out) >= 1.0)
			*	if (sdb[n].status.tractoring != x && sdb[n].status.tractored != x)
			*		continue; */
			++is_b_range;
			
			if (sdb[n].blist.load[i] + (sdb[n].blist.recycle[i] / sdb[n].tech.firing) > time(NULL)) {
				continue; }
			++is_b_load;
			sdb[n].blist.load[i] = time(NULL);
			sdb[n].beam.out -= sdb[n].blist.cost[i];
			prob = sdb[n].slist.lev[sdb[x].tmp.i1];
			prob *= sdb[n].blist.damage[i] * sdb[n].tech.firing;
			if (sdb[x].tmp.d0 > range)
				prob *= 0.01 + 0.99 * range / sdb[x].tmp.d0;
			prob /= 1.0 + (sdb2angular(n, x) * 10.0 * (1.0 + sdb[x].move.ratio) / sdb[x].move.ratio);
			prob = (prob > 1.0) ? 1.0 : (prob < 0.01) ? 0.01 : prob;
			if (sdb[x].tmp.i4)
				strncat(buff_x[sdb[x].tmp.i1], " ", sizeof(buff_x[sdb[x].tmp.i1]) - 1);
			++sdb[x].tmp.i4;
			if (getrandom(101) < prob * 100.0) {
				dmg_b[i] = (sdb[n].blist.cost[i] + sdb[n].blist.bonus[i]);
				dmg_weap=sdb[n].blist.name[i];
				if (sdb[x].tmp.d0 > range)
					dmg_b[i] *= (1.0 - (sdb[x].tmp.d0 - range) / (18.0 * range));
				strncat(buff_x[sdb[x].tmp.i1], tprintf("B%d:%s%s%d%s%s",
				  i + 1, ANSI_HILITE, ANSI_RED, (int) (dmg_b[i] + 0.5),
				  ANSI_NORMAL, ANSI_CYAN), sizeof(buff_x[sdb[x].tmp.i1]) - 1);
				if (!sdb[x].tmp.i5)
					++sdb[x].tmp.i5;
			} else {
				strncat(buff_x[sdb[x].tmp.i1], tprintf("B%d:--",
				  i + 1), sizeof(buff_x[sdb[x].tmp.i1]) - 1);
			}
		}
	}

	/* check missile weapon list */

	if (weapon == 2 || (!weapon && sdb[n].missile.exist)) {
		a = first;
		b = last;
		if (a < 1 || a > sdb[n].missile.tubes) {
			a = 0;
			b = sdb[n].missile.tubes - 1;
		} else {
			--a;
			--b;
		 	if (b >= sdb[n].missile.tubes)
				b = sdb[n].missile.tubes - 1;
			if (b < a)
				b = a;
		}
		for (i = 0; i < sdb[n].missile.tubes; ++i)
			dmg_m[i] = 0.0;
		for (i = a; i <= b ; ++i) {
			if (sdb[n].mlist.damage[i] <= 0.0)
				continue;
			if (!sdb[n].mlist.active[i])
				continue;
			++is_m_active;
			x = sdb[n].mlist.lock[i];
			if (!GoodSDB(x))
				continue;
			if (sdb[x].tmp.i0 != n) {
			    sdb[x].tmp.i0 = n;               /* initial marker */
			    sdb[x].tmp.i1 = sdb2slist(n, x); /* slist number */
			    sdb[x].tmp.i2 = 0;               /* firing arc to target */
			    sdb[x].tmp.i3 = 0;               /* facing shield of target */
			    sdb[x].tmp.i4 = 0;               /* multiple-hit flag for target */
			    sdb[x].tmp.i5 = 0;               /* hit flag of target */
			    sdb[x].tmp.d0 = 0.0;             /* range to target & shield damage */
			    sdb[x].tmp.d1 = 0.0;             /* target shield GW */
			    sdb[x].tmp.d2 = 0.0;             /* internal damage */
			}
			if (sdb[x].structure.type==6)
				continue;
			++is_m_nebula;
			if (sdb[x].tmp.i1 == SENSOR_FAIL)
				continue;
			++is_m_lock;
			if (sdb[n].missile.out < sdb[n].mlist.cost[i])
				continue;
			++is_m_arm;
			if (!sdb[x].tmp.i2)
				sdb[x].tmp.i2 = sdb2arc(n, x);
			if (arc_check(sdb[x].tmp.i2, sdb[n].mlist.arcs[i]) == ARC_FAIL)
				continue;
			if (sdb[n].status.tractored)
				if (sdb[n].status.tractored != x)
					continue;
			++is_m_arc;
			/* Missile Code check - Modified by bill 4-10-2002 */
			if (sdb[x].tmp.d0 == 0.0)
				sdb[x].tmp.d0 = sdb2range(n, x);
			if ((fabs(sdb[n].move.out) >= 1.0) || ((sdb[n].structure.type > 1) && (fabs(sdb[x].move.out) >= 1.0))) { /* * PARSEC / 10000.0;*/
			range = sdb[n].mlist.range[i] * PARSEC / 8000.0;
			} else
				range = sdb[n].mlist.range[i] * PARSEC / 8000.0;				
			if (sdb[x].tmp.d0 > range * 10.0)
				continue;
			/*if (fabs(sdb[n].move.out) >= 1.0 && fabs(sdb[x].move.out) < 1.0)
			*	if (sdb[n].status.tractoring != x && sdb[n].status.tractored != x)
			*		continue;
			*if (fabs(sdb[n].move.out) < 1.0 && fabs(sdb[x].move.out) >= 1.0 && sdb[n].structure.type == 1)
			*	if (sdb[n].status.tractoring != x && sdb[n].status.tractored != x)
			*		continue; */
			++is_m_range;
			if (sdb[n].mlist.load[i] + (sdb[n].mlist.recycle[i] / sdb[n].tech.firing) > time(NULL))
				continue;
			++is_m_load;
			sdb[n].mlist.load[i] = time(NULL);
			sdb[n].missile.out -= sdb[n].mlist.cost[i];
			prob = sdb[n].slist.lev[sdb[x].tmp.i1];
			prob *= sdb[n].mlist.damage[i] * sdb[n].tech.firing;
			if (sdb[x].tmp.d0 > range)
				prob *= 0.01 + 0.99 * range / sdb[x].tmp.d0;
			prob /= 1.0 + (sdb2angular(n, x) * 10.0 * (1.0 + sdb[x].move.ratio) / sdb[x].move.ratio);
			prob = (prob > 1.0) ? 1.0 : (prob < 0.01) ? 0.01 : prob;
			if (sdb[x].tmp.i4)
				strncat(buff_x[sdb[x].tmp.i1], " ", sizeof(buff_x[sdb[x].tmp.i1]) - 1);
			++sdb[x].tmp.i4;
			if (getrandom(101) < prob * 100.0) {
				dmg_m[i] = sdb[n].mlist.warhead[i];
				if (sdb[n].mlist.name[i] == 3) /* plasma kludge */
					if (sdb[x].tmp.d0 > range)
						dmg_m[i] *= (1.0 - (sdb[x].tmp.d0 - range) / (18.0 * range));
				strncat(buff_x[sdb[x].tmp.i1], tprintf("M%d:%s%s%d%s%s",
				  i + 1, ANSI_HILITE, ANSI_RED, (int) (dmg_m[i] + 0.5),
				  ANSI_NORMAL, ANSI_CYAN), sizeof(buff_x[sdb[x].tmp.i1]) - 1);
				if (!sdb[x].tmp.i5)
					++sdb[x].tmp.i5;
			} else {
				strncat(buff_x[sdb[x].tmp.i1], tprintf("M%d:--", i + 1), sizeof(buff_x[sdb[x].tmp.i1]) - 1);
			}
		}
	}

	/* report weapon status */

	if (!weapon && !is_b_load && !is_m_load) {
		if (!is_b_active) {
			notify(enactor, ansi_red(tprintf("No %ss are online.", system_name[3])));
		} else if (!is_b_nebula) {
			notify(enactor, ansi_red(tprintf("No %ss may shoot at a nebula!", system_name[3])));
		} else if (!is_b_lock) {
			notify(enactor, ansi_red(tprintf("No %ss are locked.", system_name[3])));
		} else if (!is_b_arm) {
			notify(enactor, ansi_red(tprintf("No %ss are powered.", system_name[3])));
		} else if (!is_b_arc) {
			notify(enactor, ansi_red(tprintf("No %ss have targets in firing arc.", system_name[3])));
		} else if (!is_b_range) {
			/*notify(enactor, ansi_red(tprintf("%f  RANGE AND SHIELD",sdb[x].tmp.d0)));
			*notify(enactor, ansi_red(tprintf("%f RANGE",range * 10)));*/
		notify(enactor, ansi_red(tprintf("No %ss have targets in range.", system_name[3])));
		} else if (!is_b_load) {
			notify(enactor, ansi_red(tprintf("No %ss are recycled.", system_name[3])));
		}
		if (!is_m_active) {
			notify(enactor, ansi_red(tprintf("No %ss are online.", system_name[9])));
		} else if (!is_m_nebula) {
			notify(enactor, ansi_red(tprintf("No %ss may shoot at a nebula!", system_name[9])));
		} else if (!is_m_lock) {
			notify(enactor, ansi_red(tprintf("No %ss are locked.", system_name[9])));
		} else if (!is_m_arm) {
			notify(enactor, ansi_red(tprintf("No %ss are powered.", system_name[9])));
		} else if (!is_m_arc) {
			notify(enactor, ansi_red(tprintf("No %ss have targets in firing arc.", system_name[9])));
		} else if (!is_m_range) {
			notify(enactor, ansi_red(tprintf("No %ss have targets in range.", system_name[9])));
		} else if (!is_m_load) {
			notify(enactor, ansi_red(tprintf("No %ss are recycled.", system_name[9])));
		}
		for (i = 0; i < sdb[n].sensor.contacts; ++i) {
			x = sdb[n].slist.sdb[i];
			if (sdb[x].tmp.i0 == n) {
				sdb[x].tmp.i0 = 0;
				sdb[x].tmp.i1 = 0;
				sdb[x].tmp.i2 = 0;
				sdb[x].tmp.i3 = 0;
				sdb[x].tmp.i4 = 0;
				sdb[x].tmp.i5 = 0;
				sdb[x].tmp.d0 = 0.0;
				sdb[x].tmp.d1 = 0.0;
				sdb[x].tmp.d2 = 0.0;
			}
		}
		return 0;
	} else if (weapon == 1 && !is_b_load) {
		if (!is_b_active) {
			notify(enactor, ansi_red(tprintf("No %ss are online.", system_name[3])));
		} else if (!is_b_lock) {
			notify(enactor, ansi_red(tprintf("No %ss are locked.", system_name[3])));
		} else if (!is_b_arm) {
			notify(enactor, ansi_red(tprintf("No %ss are powered.", system_name[3])));
		} else if (!is_b_arc) {
			notify(enactor, ansi_red(tprintf("No %ss have targets in firing arc.", system_name[3])));
		} else if (!is_b_range) {
			notify(enactor, ansi_red(tprintf("No %ss have targets in range.", system_name[3])));
		} else if (!is_b_load) {
			notify(enactor, ansi_red(tprintf("No %ss are recycled.", system_name[3])));
		}
		for (i = 0; i < sdb[n].sensor.contacts; ++i) {
			x = sdb[n].slist.sdb[i];
			if (sdb[x].tmp.i0 == n) {
				sdb[x].tmp.i0 = 0;
				sdb[x].tmp.i1 = 0;
				sdb[x].tmp.i2 = 0;
				sdb[x].tmp.i3 = 0;
				sdb[x].tmp.i4 = 0;
				sdb[x].tmp.i5 = 0;
				sdb[x].tmp.d0 = 0.0;
				sdb[x].tmp.d1 = 0.0;
				sdb[x].tmp.d2 = 0.0;
			}
		}
		return 0;
	} else if (weapon == 2 && !is_m_load) {
		if (!is_m_active) {
			notify(enactor, ansi_red(tprintf("No %ss are online.", system_name[9])));
		} else if (!is_m_lock) {
			notify(enactor, ansi_red(tprintf("No %ss are locked.", system_name[9])));
		} else if (!is_m_arm) {
			notify(enactor, ansi_red(tprintf("No %ss are powered.", system_name[9])));
		} else if (!is_m_arc) {
			notify(enactor, ansi_red(tprintf("No %ss have targets in firing arc.", system_name[9])));
		} else if (!is_m_range) {
			notify(enactor, ansi_red(tprintf("No %ss have targets in range.", system_name[9])));
		} else if (!is_m_load) {
			notify(enactor, ansi_red(tprintf("No %ss are recycled.", system_name[9])));
		}
		for (i = 0; i < sdb[n].sensor.contacts; ++i) {
			x = sdb[n].slist.sdb[i];
			if (sdb[x].tmp.i0 == n) {
				sdb[x].tmp.i0 = 0;
				sdb[x].tmp.i1 = 0;
				sdb[x].tmp.i2 = 0;
				sdb[x].tmp.i3 = 0;
				sdb[x].tmp.i4 = 0;
				sdb[x].tmp.i5 = 0;
				sdb[x].tmp.d0 = 0.0;
				sdb[x].tmp.d1 = 0.0;
				sdb[x].tmp.d2 = 0.0;
			}
		}
		return 0;
	}

	/* report firing messages */

	flag = 0;
	for (i = 0; i < sdb[n].sensor.contacts; ++i) {
		x = sdb[n].slist.sdb[i];
		if (sdb[x].tmp.i0 == n) {
			if (sdb[x].tmp.i4) {
				if (flag)
					strncat(buff_n, "\n", sizeof(buff_n) - 1);
				++flag;
				strncat(buff_n, ansi_cmd(enactor,
				  tprintf("Firing at %s: %s", unparse_identity(n, x), buff_x[i])), sizeof(buff_n) - 1);
				if (sdb[x].tmp.i5) {
					do_space_notify_two(n, x, console_helm, console_tactical, console_science,
					  "fires and hits");
					do_log(LT_SPACE, enactor, sdb[n].object, tprintf("LOG: Fired and hit, Beam %.6f GHz, Missile %.6f GHz",
					  sdb[n].beam.freq, sdb[n].missile.freq));
				} else {
					do_space_notify_two(n, x, console_helm, console_tactical, console_science,
					  "fires and misses");
				}
				if (sdb[x].status.active && !sdb[x].status.crippled) {
					do_console_notify(x, console_helm, console_science, console_tactical,
					  ansi_warn(tprintf("%s firing: %s", unparse_identity(x, n), buff_x[i])));
					do_fed_shield_bug_check(x);
					sdb[x].tmp.i3 = sdb2shield(x, n);
					sdb[x].tmp.d1 = sdb2dissipation(x, sdb[x].tmp.i3);
				}
			}
			sdb[x].tmp.d0 = 0.0; /* shield damage */
			sdb[x].tmp.d2 = 0.0; /* ship damage */
		}
	}
	do_console_notify(n, console_helm, console_science, console_tactical, buff_n);

	/* compute damage */

	if (!weapon || weapon == 1.0)
		for (i = 0; i < sdb[n].beam.banks; ++i) {
			x = sdb[n].blist.lock[i];
			if (sdb[x].tmp.i0 == n)
				if (dmg_b[i] > 0.0) {
					if (sdb[x].shield.exist) {
						if (sdb[x].tmp.d1 > 0.0) {
							if (sdb[x].shield.freq == sdb[n].beam.freq) {
								pdmg = dmg_b[i];
							} else
								pdmg = 0.0;
							dmg_b[i] -= pdmg;
							if (dmg_b[i] > 0.0)
								sdb[x].tmp.d0 += dmg_b[i] / sdb[x].tmp.d1 / 10.0;
							dmg_b[i] /= sdb[x].tmp.d1;
							dmg_b[i] += pdmg;
						}
					}
					/* dmg_b[i] -= sdb[x].tech.armor; */
					/* dmg_b[i] /= sdb[x].tech.armor; */
					if (dmg_b[i] > 0.0)
						sdb[x].tmp.d2 += dmg_b[i]; /* BILL NOTE */
				}
		}
	if (!weapon || weapon == 2.0)
		for (i = 0; i < sdb[n].missile.tubes; ++i) {
			x = sdb[n].mlist.lock[i];
			if (sdb[x].tmp.i0 == n)
				if (dmg_m[i] > 0.0) {
					if (sdb[x].shield.exist) {
						if (sdb[x].tmp.d1 > 0.0) {
							if (sdb[x].shield.freq == sdb[n].missile.freq) {
								pdmg = dmg_m[i];
							} else
								pdmg = 0.0;
							dmg_m[i] -= pdmg;
							if (dmg_m[i] > 0.0)
								sdb[x].tmp.d0 += dmg_m[i] / sdb[x].tmp.d1 / 10.0;
							dmg_m[i] /= sdb[x].tmp.d1;
							dmg_m[i] += pdmg;
						}
					}
					/* dmg_m[i] -= sdb[x].tech.armor; */
					/* dmg_m[i] /= sdb[x].tech.armor; */
					if (dmg_m[i] > 0.0)
						sdb[x].tmp.d2 += dmg_m[i];
				}
		}

	/* assess damage */

	for (i = 0; i < sdb[n].sensor.contacts; ++i) {
		x = sdb[n].slist.sdb[i];
		if (sdb[x].tmp.i0 == n) {
			for (j = 0; j < MAX_SHIELD_NAME; ++j)
				d_shield[j] = 0.0;
			for (j = 0; j < 15; ++j)
				d_system[j] = 0.0;
			for (j = 0; j < sdb[x].beam.banks; ++j)
				d_beam[j] = 0.0;
			for (j = 0; j < sdb[x].missile.tubes; ++j)
				d_missile[j] = 0.0;
			sdb[x].tmp.d2 -= sdb[x].tech.armor;	/* additive weapons */
			sdb[x].tmp.d2 /= sdb[x].tech.armor; /* additive weapons */
			if (sdb[x].tmp.d0 > 0.0) {
				d_shield[sdb[x].tmp.i3] += sdb[x].tmp.d0;
				if (sdb[x].tmp.d2 > 0.0) {
					alert_ship_hurt(x);
				} else
					alert_ship_hit(x);
			}
			if (sdb[x].tmp.d2 > 0.0) {
					if (mode == 0 || mode == 6) {
						d_system[0] += sdb[x].tmp.d2;  /* Bill Note */
					} else
						d_system[0] += sdb[x].tmp.d2 / 100.0;
				k = (int) (sdb[x].tmp.d2 / 5.0 + 1.0);
				for (j = 0; j < k; ++j) {
					pdmg = getrandom(500) / 100.0;
					if (mode == 0) /* normal damage */ {
						switch (getrandom(10) + getrandom(10) + 2) {
							case 2: d_shield[getrandom(MAX_SHIELD_NAME)] += pdmg; break;
							case 3: d_system[11] += pdmg; break;
							case 4: d_system[7] += pdmg; break;
							case 5: d_system[5] += pdmg; break;
							case 6: d_system[1] += pdmg; break;
							case 7: d_system[8] += pdmg; break;
							case 8: d_missile[getrandom(sdb[x].missile.tubes)] += pdmg; break;
							//Added to damage components in combat Added 12-13-2002
							case 9: damage_component(x); break;
							//case 10: damage_component(x); break;
							//End Component Damage
							case 13:  d_system[15] += pdmg/5; break;
							case 14: d_beam[getrandom(sdb[x].beam.banks)] += pdmg; break;
							case 15: d_system[14] += pdmg; break;
							case 16: d_system[6] += pdmg; break;
							case 17: d_system[13] += pdmg; break;
							case 18: d_system[12] += pdmg; break;
							case 19: d_system[4] += pdmg; break;
							case 20: d_system[2] += pdmg; break;
							default: break;
						}
					} else if (mode == 2) /* engine damage */ {
						switch (getrandom(10) + getrandom(10) + 2) {
							case 15: d_system[14] += pdmg; break;
							case 16: d_system[6] += pdmg; break;
							case 4: damage_component(x); break;
							default: break;
						}
					} else if (mode == 3) /* weapon damage */ {
						switch (getrandom(10) + getrandom(10) + 2) {
							case 2: d_shield[getrandom(MAX_SHIELD_NAME)] += pdmg; break;
							case 5: d_system[5] += pdmg; break;
							case 8: d_missile[getrandom(sdb[x].missile.tubes)] += pdmg; break;
							case 14: d_beam[getrandom(sdb[x].beam.banks)] += pdmg; break;
							case 17: d_system[13] += pdmg; break;
							case 18: d_system[12] += pdmg; break;
							case 4: damage_component(x); break;
							case 19: d_system[4] += pdmg; break;
							default: break;
						}
					} else if (mode == 4) /* sensor damage */ {
						switch (getrandom(10) + getrandom(10) + 2) {
							case 3: d_system[11] += pdmg; break;
							case 4: d_system[7] += pdmg; break;
							case 5: d_system[5] += pdmg; break;
							case 19: d_system[4] += pdmg; break;
							case 10: damage_component(x); break;
							default: break;
						}
					} else if (mode == 5) /* power damage */ {
					 	switch (getrandom(10) + getrandom(10) + 2) {
							case 6: d_system[1] += pdmg; break;
							case 9: d_system[8] += pdmg; break;
							case 20: d_system[2] += pdmg; break;
							case 8: d_system[1] += pdmg; break;
							case 7: d_system[8] += pdmg; break;
							case 10: damage_component(x); break;
							case 4: d_system[2] += pdmg; break;
							case 15: d_system[15] += pdmg/5; break;
							default: break;
						}
					}
				}
			}
			if (sdb[x].tmp.d0 > 0.0 || sdb[x].tmp.d2 > 0.0)
				if (sdb[x].shield.exist)
					for (j = 0; j < MAX_SHIELD_NAME; ++j)
						if (d_shield[j] > 0.0)
							damage_shield(x, j, d_shield[j], dmg_weap);
			if (sdb[x].tmp.d2 > 0.0) {
				ss = sdb[x].structure.max_structure;
				if (d_system[0] > 0.0) {
					pdmg = sdb[x].structure.superstructure;
					damage_structure(x, d_system[0]);
					if (sdb[x].structure.superstructure <= -ss) {
						atr_add(sdb[x].object, "COMMENT",
						  tprintf("%s(#%d) killed by %s(#%d) on %s(#%d) at %s",
						  Name(sdb[x].object), sdb[x].object,
						  Name(enactor), enactor,
						  Name(sdb[n].object), sdb[n].object,
						  (char *) ctime(&mudtime)), GOD,
						  (AF_MDARK + AF_WIZARD + AF_NOPROG));
						continue;
					}
					if (pdmg > 0.0 && sdb[x].structure.superstructure <= 0.0) {
						for (j = 0; j < sdb[n].beam.banks; ++j)
							if (sdb[n].blist.lock[j] == x)
								sdb[n].blist.lock[j] = 0;
						for (j = 0; j < sdb[n].missile.tubes; ++j)
							if (sdb[n].mlist.lock[j] == x)
								sdb[n].mlist.lock[j] = 0;
					}
				}
				if (d_system[1] > 0.0 && sdb[x].aux.exist) {
					damage_aux(x, d_system[1], dmg_weap);
					if (sdb[x].structure.superstructure <= -ss)
						continue;
				}
				if (d_system[8] > 0.0 && sdb[x].main.exist) {
					damage_main(x, d_system[8], dmg_weap);
					if (sdb[x].structure.superstructure <= -ss)
						continue;
				}
				if (d_system[2] > 0.0 && sdb[x].batt.exist)
					damage_batt(x, d_system[2], dmg_weap);
				if (sdb[x].beam.exist)
					for (j = 0; j < sdb[x].beam.banks; ++j)
						if (d_beam[j] > 0.0)
							damage_beam(x, j, d_beam[j], dmg_weap);
				if (d_system[4] > 0.0 && sdb[x].cloak.exist)
					damage_cloak(x, d_system[4], dmg_weap);
				if (d_system[5] > 0.0 && sdb[x].sensor.ew_exist)
					damage_ew(x, d_system[5], dmg_weap);
				if (d_system[6] > 0.0 && sdb[x].engine.impulse_exist)
					damage_impulse(x, d_system[6], dmg_weap);
				if (d_system[7] > 0.0 && sdb[x].sensor.lrs_exist)
					damage_lrs(x, d_system[7], dmg_weap);
				if (sdb[x].missile.exist)
					for (j = 0; j < sdb[x].missile.tubes; ++j)
						if (d_missile[j] > 0.0)
							damage_missile(x, j, d_missile[j], dmg_weap);
				if (d_system[11] > 0.0 && sdb[x].sensor.srs_exist)
					damage_srs(x, d_system[11], dmg_weap);
				if (d_system[12] > 0.0 && sdb[x].tract.exist)
					damage_tract(x, d_system[12], dmg_weap);
				if (d_system[13] > 0.0 && sdb[x].trans.exist)
					damage_trans(x, d_system[13], dmg_weap);
				if (d_system[14] > 0.0 && sdb[x].engine.warp_exist)
					damage_warp(x, d_system[14], dmg_weap);
				if (d_system[15] > 0.0 && sdb[x].lifesupport.active)
					damage_lifesupport(x, d_system[15], dmg_weap);
			}
			sdb[x].tmp.i0 = 0;
			sdb[x].tmp.i1 = 0;
			sdb[x].tmp.i2 = 0;
			sdb[x].tmp.i3 = 0;
			sdb[x].tmp.i4 = 0;
			sdb[x].tmp.i5 = 0;
			sdb[x].tmp.d0 = 0.0;
			sdb[x].tmp.d1 = 0.0;
			sdb[x].tmp.d2 = 0.0;
		}
	}
	return 1;
}

/* ------------------Modified by bill 4-9-2002-------------------------------------- */

int repair_everything (void)
{
	register int i;

	if (sdb[n].aux.exist)
		sdb[n].aux.damage = 1.0;
		sdb[n].aux.lasthit=999;

		/*Added for the life support stuff  - bill 4/9/2002 	*/
		sdb[n].lifesupport.damage = 1.0;
		sdb[n].lifesupport.active = 1;
		sdb[n].lifesupport.lasthit = 999;
		
	if (sdb[n].batt.exist)
		sdb[n].batt.damage = 1.0;
		sdb[n].batt.lasthit = 999;

	if (sdb[n].beam.exist)
	    sdb[n].beam.lasthit = 999;
		for (i = 0; i < sdb[n].beam.banks; ++i)
			sdb[n].blist.damage[i] = 1.0;

	if (sdb[n].missile.exist)
		sdb[n].missile.lasthit = 999;
		for (i = 0; i < sdb[n].missile.tubes; ++i)
			sdb[n].mlist.damage[i] = 1.0;

	if (sdb[n].cloak.exist)
		sdb[n].cloak.damage = 1.0;
		sdb[n].cloak.lasthit = 999;

	if (sdb[n].engine.warp_exist)
		sdb[n].engine.lasthit = 999;
		sdb[n].engine.warp_damage = 1.0;
	if (sdb[n].engine.impulse_exist)
		sdb[n].engine.impulse_damage = 1.0;

	if (sdb[n].main.exist)
		sdb[n].main.lasthit = 999;
		sdb[n].main.damage = 1.0;

		sdb[n].sensor.lasthit = 999;
	if (sdb[n].sensor.ew_exist)
		sdb[n].sensor.ew_damage = 1.0;
	if (sdb[n].sensor.lrs_exist)
		sdb[n].sensor.lrs_damage = 1.0;
	if (sdb[n].sensor.srs_exist)
		sdb[n].sensor.srs_damage = 1.0;

	if (sdb[n].shield.exist)
		sdb[n].shield.lasthit = 999;
		for (i = 0; i < MAX_SHIELD_NAME; ++i)
			sdb[n].shield.damage[i] = 1.0;

	sdb[n].status.crippled = 0;

	sdb[n].structure.superstructure = sdb[n].structure.max_structure;
	sdb[n].structure.repair = sdb[n].structure.max_repair;

	if (sdb[n].trans.exist)
		sdb[n].trans.lasthit = 999;
		sdb[n].trans.damage = 1.0;

	if (sdb[n].tract.exist)
		sdb[n].tract.lasthit = 999;
		sdb[n].tract.damage = 1.0;

	sdb[n].sensor.version = 1;
	sdb[n].engine.version = 1;
	sdb[n].power.version = 1;
	
	/* Repair Life support - Added by bill 4-10-2002 */
	sdb[n].lifesupport.damage=1.0;
	sdb[n].lifesupport.active=1.0;
	sdb[n].lifesupport.time=0;
	sdb[n].lifesupport.lasthit = 999;
	/*End life support code */
	
	
	sdb[n].cloak.version = 1;
	up_cochranes();
	up_empire();
	up_quadrant();
	up_vectors();
	up_resolution();
	up_signature(n);
	up_visibility();
	debug_space(n);

	return 1;
}

/* ------------------------------------------------------------------------ */

void dump_space (dbref executor) /* dumps the sdb[] struct into the space objects */
{
	int result;

	for (n = MIN_SPACE_OBJECTS ; n <= max_space_objects ; ++n)
		if (sdb[n].structure.type) {
			if (sdb[n].status.active && sdb[n].power.total == 0.0)
				result = do_set_inactive(sdb[n].object);
			do_space_db_write(sdb[n].object, executor);
		}

	return;
}

/* ------------------------------------------------------------------------ */
void do_eject_warpcore(int x) {


	//we only eject if they have a core!!!
	if (sdb[x].engine.warp_exist==1){
	//This is where we eject the warp core
	do_console_notify(x, console_tactical, console_science, console_helm,
	  ansi_alert(tprintf("Woman voice says \"Warp Core Critical. Recommend ejecting the warp core immediately.\"")));

	atr_add(sdb[x].object, "WARPCOREEJECT", tprintf("1"), GOD,(AF_MDARK + AF_WIZARD + AF_NOPROG));														

	}
	else {
	do_console_notify(x, console_tactical, console_science, console_helm,
	  ansi_alert(tprintf("NO WARP CORE TO EJECT!")));
	}

return;

}


/* ------------------------------------------------------------------------ */
void do_selfdestruct(int x, int silent, int destruction) {


	//we only self destruct if the ship is !destroyed
	if (sdb[x].status.crippled !=2) {
		//Only Zemit if silent eq 0 else its audible
		if (silent == 1) {
		do_ship_notify(x, tprintf("%s%sWoman voice says \"Self Destruct Sequence Started, Detonation in %i Seconds. Audible Alerts\"%s%s",
		  ANSI_HILITE, ANSI_RED, destruction, ANSI_WHITE, ANSI_NORMAL));
		}
		else if (silent == 0) {
		do_ship_notify(x, tprintf("%s%sWoman voice says \"Self Destruct Sequence Started, Detonation in %i Seconds. No Audible Alerts will be given.\"%s%s",
		  ANSI_HILITE, ANSI_RED, destruction, ANSI_WHITE, ANSI_NORMAL));
		}
		//Lets let all the other ships in the area know
		do_space_notify_one(x, console_helm, console_tactical, console_science,
			"has started a self destruct sequence");
		//Lets set the timer of death!
		atr_add(sdb[x].object, "SELFDESTRUCTTIMER", tprintf("%i",destruction), GOD,(AF_MDARK + AF_WIZARD + AF_NOPROG));
		atr_add(sdb[x].object, "SELFDESTRUCTSILENT", tprintf("%i",silent), GOD,(AF_MDARK + AF_WIZARD + AF_NOPROG));
	}

return;

}	
/* ------------------------------------------------------------------------ */
void do_abortselfdestruct(int x) {


	//we only self destruct if the ship is !destroyed
	if (sdb[x].status.active==1) {
		//Only Zemit if silent eq 0 else its audible
		do_ship_notify(x, tprintf("%s%sWoman voice says \"Self Destruct Sequence Aborted. Have a Nice Day\"%s%s",
		  ANSI_HILITE, ANSI_RED, ANSI_WHITE, ANSI_NORMAL));
		//Lets let all the other ships in the area know
		do_space_notify_one(x, console_helm, console_tactical, console_science,
			"has aborted a self destruct sequence");
		//Lets set the timer of death!
		atr_add(sdb[x].object, "SELFDESTRUCTTIMER", tprintf("0"), GOD,(AF_MDARK + AF_WIZARD + AF_NOPROG));
	}

return;

}

/* ------------------------------------------------------------------------ */

int debug_space (int x)
	{
	register int i, bug = 1;

/* --- OBJECT ------------------------------------------------------------- */

	if (sdb[x].structure.type <= 0) {
		sdb[x].structure.type = 0;
		bug = 0;
	}
	if (!SpaceObj(sdb[x].object) || !GoodObject(sdb[x].object)) {
		sdb[x].object = 0;
		sdb[x].structure.type = 0;
		bug = 0;
	}

/* --- LOCATION ----------------------------------------------------------- */

	sdb[x].location = 0;
	for (i = MIN_SPACE_OBJECTS ; i <= max_space_objects ; ++i)
		if (SpaceObj(sdb[i].object)) {
			if (Location(sdb[i].object) == sdb[x].object)
				sdb[i].location = x;
			if (Location(sdb[x].object) == sdb[i].object)
				sdb[x].location = i;
		}

/* --- MAIN --------------------------------------------------------------- */

	if (!sdb[x].main.exist || sdb[x].main.gw <= 0) {
		sdb[x].main.damage = 0.0;
		sdb[x].main.exist = 0;
		sdb[x].main.gw = 0.0;
		sdb[x].main.in = 0.0;
		sdb[x].main.out = 0.0;
		sdb[x].power.main = 0.0;
	}

/* --- AUX ---------------------------------------------------------------- */

	if (!sdb[x].aux.exist || sdb[x].aux.gw <= 0) {
		sdb[x].aux.damage = 0.0;
		sdb[x].aux.exist = 0;
		sdb[x].aux.gw = 0.0;
		sdb[x].aux.in = 0.0;
		sdb[x].aux.out = 0.0;
		sdb[x].power.aux = 0.0;
	}

/* --- BATT --------------------------------------------------------------- */

	if (!sdb[x].batt.exist || sdb[x].batt.gw <= 0) {
		sdb[x].batt.damage = 0.0;
		sdb[x].batt.exist = 0;
		sdb[x].batt.gw = 0.0;
		sdb[x].batt.in = 0.0;
		sdb[x].batt.out = 0.0;
		sdb[x].fuel.reserves = 0.0;
		sdb[x].power.batt = 0.0;
	}

/* --- ALLOCATE ----------------------------------------------------------- */

	if (!sdb[x].main.exist && !sdb[x].aux.exist && !sdb[x].batt.exist) {
		sdb[x].alloc.helm = 0.0;
		sdb[x].alloc.tactical = 0.0;
		sdb[x].alloc.operations = 1.0;
		sdb[x].alloc.movement = 0.0;
		sdb[x].alloc.shields = 0.0;
		for (i = 0 ; i < MAX_SHIELD_NAME ; ++i)
			sdb[x].alloc.shield[i] = 0.0;
		sdb[x].alloc.cloak = 0.0;
		sdb[x].alloc.beams = 0.0;
		sdb[x].alloc.missiles = 0.0;
		sdb[x].alloc.sensors = 0.0;
		sdb[x].alloc.ecm = 0.0;
		sdb[x].alloc.eccm = 0.0;
		sdb[x].alloc.transporters = 0.0;
		sdb[x].alloc.tractors = 0.0;
		sdb[x].alloc.miscellaneous = 1.0;
		sdb[x].power.total = 0.0;
		sdb[x].beam.in = 0.0;
		sdb[x].beam.out = 0.0;
		sdb[x].missile.in = 0.0;
		sdb[x].missile.out = 0.0;
	}

/* --- BEAM --------------------------------------------------------------- */

	if (!sdb[x].beam.exist || sdb[x].beam.banks <= 0) {
		sdb[x].beam.banks = 0;
		sdb[x].beam.exist = 0;
		sdb[x].beam.freq = 0.0;
		sdb[x].beam.in = 0.0;
		sdb[x].beam.out = 0.0;
		for (i = 0; i < MAX_BEAM_BANKS; ++i) {
			sdb[x].blist.name[i] = 0;
			sdb[x].blist.damage[i] = 0.0;
			sdb[x].blist.bonus[i] = 0;
			sdb[x].blist.cost[i] = 0;
			sdb[x].blist.range[i] = 0;
			sdb[x].blist.arcs[i] = 0;
			sdb[x].blist.active[i] = 0;
			sdb[x].blist.lock[i] = 0;
			sdb[x].blist.load[i] = 0;
		}
	} else {
		if (sdb[x].beam.in < 0.0) {
			sdb[x].beam.in = 0.0;
			bug = 0;
		}
		if (sdb[x].beam.out < 0.0) {
			sdb[x].beam.out = 0.0;
			bug = 0;
		}
		if (sdb[x].beam.banks > MAX_BEAM_BANKS) {
			sdb[x].beam.banks = MAX_BEAM_BANKS;
			bug = 0;
		}
		if (sdb[x].beam.freq <= 1.0 || sdb[x].beam.freq >= 1000.0) {
			sdb[x].beam.freq = getrandom(10000) / 100.0;
			bug = 0;
		}
		for (i = 0; i < MAX_BEAM_BANKS; ++i) {
			if (!(sdb[x].blist.arcs[i] & 1) && !(sdb[x].blist.arcs[i] & 4))
				sdb[x].blist.arcs[i] += 5;
			if (!(sdb[x].blist.arcs[i] & 2) && !(sdb[x].blist.arcs[i] & 8))
				sdb[x].blist.arcs[i] += 10;
			if (!(sdb[x].blist.arcs[i] & 16) && !(sdb[x].blist.arcs[i] & 32))
				sdb[x].blist.arcs[i] += 48;
		}
	}

/* --- MISSILE ------------------------------------------------------------ */

	if (!sdb[x].missile.exist || sdb[x].missile.tubes <= 0) {
		sdb[x].missile.exist = 0;
		sdb[x].missile.freq = 0.0;
		sdb[x].missile.tubes = 0;
		sdb[x].missile.in = 0.0;
		sdb[x].missile.out = 0.0;
		for (i = 0; i < MAX_MISSILE_TUBES; ++i) {
			sdb[x].mlist.name[i] = 0;
			sdb[x].mlist.damage[i] = 0.0;
			sdb[x].mlist.warhead[i] = 0;
			sdb[x].mlist.cost[i] = 0;
			sdb[x].mlist.range[i] = 0;
			sdb[x].mlist.arcs[i] = 0;
			sdb[x].mlist.active[i] = 0;
			sdb[x].mlist.lock[i] = 0;
			sdb[x].mlist.load[i] = 0;
		}
	} else {
		if (sdb[x].missile.out < 0.0) {
			sdb[x].missile.out = 0.0;
			bug = 0;
		}
		if (sdb[x].missile.in < 0.0) {
			sdb[x].missile.in = 0.0;
			bug = 0;
		}
		if (sdb[x].missile.tubes > MAX_MISSILE_TUBES) {
			sdb[x].missile.tubes = MAX_MISSILE_TUBES;
			bug = 0;
		}
		if (sdb[x].missile.freq <= 1.0 || sdb[x].missile.freq >= 1000.0) {
			sdb[x].missile.freq = getrandom(10000) / 100.0;
			bug = 0;
		}
		for (i = 0; i < MAX_MISSILE_TUBES; ++i) {
			if (!(sdb[x].mlist.arcs[i] & 1) && !(sdb[x].mlist.arcs[i] & 4))
				sdb[x].mlist.arcs[i] += 5;
			if (!(sdb[x].mlist.arcs[i] & 2) && !(sdb[x].mlist.arcs[i] & 8))
				sdb[x].mlist.arcs[i] += 10;
			if (!(sdb[x].mlist.arcs[i] & 16) && !(sdb[x].mlist.arcs[i] & 32))
				sdb[x].mlist.arcs[i] += 48;
		}
	}

/* --- ENGINE ------------------------------------------------------------- */

	if (!sdb[x].engine.impulse_exist) {
		sdb[x].engine.impulse_exist = 0;
		sdb[x].engine.impulse_damage = 0.0;
		sdb[x].engine.warp_max = 0.0;
		sdb[x].engine.warp_cruise = 0.0;
	}
	if (!sdb[x].engine.warp_exist) {
		sdb[x].engine.warp_exist = 0;
		sdb[x].engine.warp_damage = 0.0;
		sdb[x].engine.impulse_max = 0.0;
		sdb[x].engine.impulse_cruise = 0.0;
	}
	if (!sdb[x].engine.warp_exist && !sdb[x].engine.impulse_exist) {
		sdb[x].move.in = 0.0;
		sdb[x].move.out = 0.0;
	}

/* --- STRUCTURE ---------------------------------------------------------- */

	if (sdb[x].structure.displacement <= 0) {
		sdb[x].structure.displacement = 1;
		bug = 0;
	}
	if (sdb[x].structure.cargo_hold > sdb[x].structure.displacement) {
		sdb[x].structure.cargo_hold = sdb[x].structure.displacement;
		bug = 0;
	} else if (sdb[x].structure.cargo_hold < 0) {
		sdb[x].structure.cargo_hold = 0;
		bug = 0;
	}
	if (sdb[x].structure.max_structure <= 0) {
		sdb[x].structure.max_structure = 1;
		bug = 0;
	}
	if (sdb[x].structure.superstructure > sdb[x].structure.max_structure) {
		sdb[x].structure.superstructure = sdb[x].structure.max_structure;
		bug = 0;
	}
	if (sdb[x].structure.max_repair < 0) {
		sdb[x].structure.max_repair = 0;
		bug = 0;
	}
	if (sdb[x].structure.repair > sdb[x].structure.max_repair) {
		sdb[x].structure.repair = sdb[x].structure.max_repair;
		bug = 0;
	} else if (sdb[x].structure.repair < 0.0) {
		sdb[x].structure.repair = 0.0;
		bug = 0;
	}

/* --- SENSOR ------------------------------------------------------------- */

	if (!sdb[x].sensor.lrs_exist) {
		sdb[x].sensor.lrs_active = 0;
		sdb[x].sensor.lrs_damage = 0.0;
		sdb[x].sensor.lrs_resolution = 0.0;
	}
	if (!sdb[x].sensor.srs_exist) {
		sdb[x].sensor.srs_active = 0;
		sdb[x].sensor.srs_damage = 0.0;
		sdb[x].sensor.srs_resolution = 0.0;
	}
	if (!sdb[x].sensor.ew_exist) {
		sdb[x].sensor.ew_active = 0;
		sdb[x].sensor.ew_damage = 0.0;
	}
	if (!sdb[x].sensor.srs_exist && !sdb[x].sensor.lrs_exist) {
		sdb[x].sensor.contacts = 0;
		sdb[x].sensor.counter = 0;
		for (i = 0 ; i < MAX_SENSOR_CONTACTS ; ++i) {
			sdb[x].slist.sdb[i] = 0;
			sdb[x].slist.num[i] = 0;
			sdb[x].slist.lev[i] = 0.0;
		}
	}

/* --- SHIELD ------------------------------------------------------------- */
	if (!sdb[x].shield.exist || sdb[x].shield.ratio <= 0.0 || sdb[x].shield.maximum <= 0) {
		sdb[x].shield.ratio = 0.0;
		sdb[x].shield.maximum = 0;
		sdb[x].shield.freq = 0.0;
		sdb[x].shield.exist = 0;
		for (i = 0 ; i < MAX_SHIELD_NAME; ++i) {
			sdb[x].shield.damage[i] = 0.0;
			sdb[x].shield.active[i] = 0;
		}
	}

/* --- TECH --------------------------------------------------------------- */

	if (sdb[x].tech.firing <= 0.0) {
		sdb[x].tech.firing = 1.0;
		bug = 0;
	}
	if (sdb[x].tech.fuel <= 0.0) {
		sdb[x].tech.fuel = 1.0;
		bug = 0;
	}
	if (sdb[x].tech.stealth <= 0.0) {
		sdb[x].tech.stealth = 1.0;
		bug = 0;
	}
	if (sdb[x].tech.cloak <= 0.0) {
		sdb[x].tech.cloak = 1.0;
		bug = 0;
	}
	if (sdb[x].tech.sensors <= 0.0) {
		sdb[x].tech.sensors = 1.0;
		bug = 0;
	}
	if (sdb[x].tech.main_max <= 0.0) {
		sdb[x].tech.main_max = 1.0;
		bug = 0;
	}
	if (sdb[x].tech.aux_max <= 0.0) {
		sdb[x].tech.aux_max = 1.0;
		bug = 0;
	}
	if (sdb[x].tech.armor <= 0.0) {
		sdb[x].tech.armor = 1.0;
		bug = 0;
	}

/* --- MOVE --------------------------------------------------------------- */

	if (sdb[x].move.ratio <= 0.0) {
		sdb[x].move.ratio = 1.0;
		bug = 0;
	}

/* --- CLOAK -------------------------------------------------------------- */

	if (!sdb[x].cloak.exist || sdb[x].cloak.cost <= 0) {
		sdb[x].cloak.cost = 0;
		sdb[x].cloak.damage = 0.0;
		sdb[x].cloak.exist = 0;
		sdb[x].cloak.freq = 0.0;
		sdb[x].cloak.active = 0;
	} else {
		if (sdb[x].cloak.freq <= 1.0 || sdb[x].cloak.freq >= 1000.0) {
			sdb[x].cloak.freq = getrandom(10000) / 100.0;
			bug = 0;
		}
	}

/* --- TRANS -------------------------------------------------------------- */

	if (!sdb[x].trans.exist) {
		sdb[x].trans.cost = 0;
		sdb[x].trans.damage = 0;
		sdb[x].trans.freq = 0;
		sdb[x].trans.active = 0;
		sdb[x].trans.d_lock = 0;
		sdb[x].trans.s_lock = 0;
	} else if (sdb[x].trans.freq <= 1.0 || sdb[x].trans.freq >= 1000.0) {
		sdb[x].trans.freq = getrandom(10000) / 100.0;
		bug = 0;
	}
	if (sdb[x].trans.d_lock)
		if (sdb[x].trans.d_lock != x)
			if (sdb2contact(n, sdb[x].trans.d_lock) == SENSOR_FAIL) {
				sdb[x].trans.d_lock = 0;
			}
	if (sdb[x].trans.s_lock)
		if (sdb[x].trans.s_lock != x)
			if (sdb2contact(n, sdb[x].trans.s_lock) == SENSOR_FAIL) {
				sdb[x].trans.s_lock = 0;
			}

/* --- TRACT -------------------------------------------------------------- */

	if (!sdb[x].tract.exist) {
		sdb[x].tract.cost = 0;
		sdb[x].tract.damage = 0;
		sdb[x].tract.freq = 0;
		sdb[x].tract.active = 0;
		sdb[x].tract.lock = 0;
		sdb[x].status.tractoring = 0;
	} else if (sdb[x].tract.freq <= 1.0 || sdb[x].tract.freq >= 1000.0) {
		sdb[x].tract.freq = getrandom(10000) / 100.0;
		bug = 0;
	}
	/* if (sdb[x].status.tractoring)
		if (sdb2contact(n, sdb[x].status.tractoring) == SENSOR_FAIL) {
			sdb[sdb[x].status.tractoring].status.tractored = 0;
			sdb[x].tract.lock = 0;
			sdb[x].status.tractoring = 0;
		} */
	/* if (sdb[x].status.tractored)
		if (sdb2contact(sdb[x].status.tractored, x) == SENSOR_FAIL) {
			sdb[sdb[x].status.tractored].status.tractoring = 0;
			sdb[sdb[x].status.tractored].tract.lock = 0;
			sdb[x].status.tractored = 0;
		} */

/* --- FUEL --------------------------------------------------------------- */

	if (sdb[x].fuel.antimatter < 0.0)
		sdb[x].fuel.antimatter = 0.0;
	if (sdb[x].fuel.deuterium < 0.0)
		sdb[x].fuel.deuterium = 0.0;
	if (sdb[x].fuel.reserves < 0.0)
		sdb[x].fuel.reserves = 0.0;
	if (sdb[x].fuel.antimatter > sdb2max_antimatter(x))
		sdb[x].fuel.antimatter = sdb2max_antimatter(x);
	if (sdb[x].fuel.deuterium > sdb2max_deuterium(x))
		sdb[x].fuel.deuterium = sdb2max_deuterium(x);
	if (sdb[x].fuel.reserves > sdb2max_reserve(x))
		sdb[x].fuel.reserves = sdb2max_reserve(x);

/* --- STATUS ------------------------------------------------------------- */

	if (sdb[x].structure.superstructure <= -sdb[x].structure.max_structure) {
		sdb[x].status.crippled = 2;
	} else if (sdb[x].structure.superstructure <= 0.0) {
		sdb[x].status.crippled = 1;
	} else {
		sdb[x].status.crippled = 0;
	}

/* --- other updates ------------------------------------------------------ */

	up_cochranes();
	up_turn_rate();
	up_vectors();
	up_empire();
	up_quadrant();
	up_resolution();
	up_signature(x);
	up_visibility();

	return bug;
}

/* ------------------------------------------------------------------------ */

int get_selfdestruct_timer(int x) 
{
	int px;
	ATTR *a;
	char *q;


	
				a = atr_get(sdb[n].object, "SELFDESTRUCTTIMER");
				if (a != NULL) {
					//selfdestructsilent
					q = safe_uncompress(a->value);
					px=atoi(q);

		//Lets free up some ram
		free((Malloc_t) q);

				}
					//tprintf("%i X%f Y %f Z%f",i,sdb[i].coords.x,sdb[i].coords.y,sdb[i].coords.z));
					
	return px;
}
/* ------------------------------------------------------------------------ */

void cdb_add(dbref object, double range,dbref enactor,char *buff,char *bp) {

					  
				//sql_init(enactor);
				//lets run a query to insert this name
				//UNCOMMENT THIS LATER
				/*query=tprintf("insert into commdb(dbref,lowfreq,upfreq,range)
					values('%i','%i','%i','%d')"
					,executor,MIN_COMMS_FREQUENCY,MAX_COMMS_FREQUENCY,args[1]);*/

				//sql_query_internal(enactor, query, buff, bp);
				notify(enactor, tprintf("EXecutor %i\nMinFreq %d\nMaxFreq %d\nRange %d\nobject %i",
					enactor,MIN_COMMS_FREQUENCY,MAX_COMMS_FREQUENCY,range,object));			
				//notify(enactor, tprintf("Object added To Communications Database"));			

		return;
}

/************************Lets damage a random component in combat***************************************/
void damage_component(int x) {
MYSQL *conn;
MYSQL_RES *res_set;
MYSQL_ROW	row;
MYSQL_FIELD *field;
unsigned int	i;
int recordid;
char *componentname;


/* Lets	do the component check */
	conn = mysql_init (NULL);
	mysql_real_connect(conn,"localhost.localdomain","root","pw","db",0,NULL,0);

	if (!conn) { /*notify(executor, "No SQL database connection.");*/	}
   if (conn) {	/*notify(executor, "SQL database connection.");*/
	//Lets run the query to grab a random component
	   if (mysql_query (conn, tprintf("select * from components where shipdbref='%s' and active='1' order by rand() limit 1",unparse_dbref(sdb[x].object))) != 0){
			//notify(executor, "Query Failed");   
	   }
	else {
		res_set = mysql_store_result (conn); //Generate Result Set
		if (res_set==NULL) {
				//notify(executor, "Mysql_store_result() Failed");
		}
		else {
				//Start the actual processing of the errors
					while((row=mysql_fetch_row (res_set)) != NULL)
						{

							//do_ship_notify(1, tprintf("BANG!"));
							mysql_field_seek (res_set,0);
							for (i = 0; i <  mysql_num_fields (res_set); i++)
							{
								field=mysql_fetch_field (res_set);
									
								//Lets get the recordid
								if (strcmp(field->name,"recordid") ==0) {
									recordid=atoi(row[i]);
									//do_ship_notify(1, tprintf("Main power in %f out %f",sdb[x].main.in,sdb[x].main.out));
									mysql_query (conn, tprintf("update components set odometer=0,damaged=1,cf=cf-1,remove=1 where recordid='%i'",recordid));
									//Now lets make sure the component is removed
									do_space_db_write(sdb[x].object,GOD);

								}		
								//Lets get the recordid
								if (strcmp(field->name,"name") ==0) {
									componentname=row[i];
									do_console_notify(x, console_engineering, 0, 0,ansi_alert(tprintf("Component: %s has been disabled in a volley",componentname)));  //Let everyone know something broke	
								}								
									
							}

							
							componentname="";
							recordid="0";
							
							//notify(executor, tprintf("\n"));	
						}
					/*	if (mysql_errno (conn) !=0)
								notify(executor, "mysql_Fetch_row() Failed");
						else
								notify(executor, tprintf("%lu rows returned\n",(unsigned long) mysql_num_rows (res_set)));
					*/	
				//End the actual processing of the errors

			mysql_free_result(res_set);
		}

	}

   }	
		
	
	mysql_close(conn);
	/* End Component Check */
	
 }
/********************END COMPONENT ODOMETER CODE***************************************/
