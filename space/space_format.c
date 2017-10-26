/* space_format.c */

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
#include "flags.h"
#include "function.h"
#include "attrib.h"



/* ------------------------------------------------------------------------ */
//#define SpaceObj(x) (IS(x, TYPE_THING, THING_SPACE_OBJECT)) THIS IS THE OLD FLAG SYSTEM
#define SpaceObj(x)       (IS(x, TYPE_THING, "SPACE_OBJECT"))

char *format_Location (int x)
{
	static char buffer[50];
	int l = sdb[x].location;

	if (l == 0) {
		if (sdb[x].sensor.visibility < 0.1) {
			snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.20s",
			  ANSI_YELLOW, "Location", ANSI_WHITE, "Opaque Nebula");
		} else if (sdb[x].sensor.visibility < 0.25) {
			snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.20s",
			  ANSI_YELLOW, "Location", ANSI_WHITE, "Thick Nebula");
		} else if (sdb[x].sensor.visibility < 0.50) {
			snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.20s",
			  ANSI_YELLOW, "Location", ANSI_WHITE, "Moderate Nebula");
		} else if (sdb[x].sensor.visibility < 0.90) {
			snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.20s",
			  ANSI_YELLOW, "Location", ANSI_WHITE, "Light Nebula");
		} else {
			snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.20s",
			  ANSI_YELLOW, "Location", ANSI_WHITE, "Open Space");
		}
	} else if (l < MIN_SPACE_OBJECTS || l > max_space_objects) {
		snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.20s",
		  ANSI_YELLOW, "Location", ANSI_WHITE, "#-1 BAD SDB");
	} else if (!SpaceObj(sdb[l].object) || !GoodObject(sdb[l].object)) {
		snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.20s",
		  ANSI_YELLOW, "Location", ANSI_WHITE, "#-1 BAD DBREF");
	} else if (sdb[l].structure.type == 0) {
		snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.20s",
		  ANSI_YELLOW, "Location", ANSI_WHITE, "#-1 BAD TYPE");
	} else if (sdb[x].status.docked) {
		if (sdb[x].status.connected) {
			snprintf(buffer, sizeof(buffer), "%s%16.16s:%s D+C:%-17.17s",
			  ANSI_YELLOW, "Location", ANSI_WHITE, Name(sdb[l].object));
		} else
			snprintf(buffer, sizeof(buffer), "%s%16.16s:%s D:%-18.18s",
			  ANSI_YELLOW, "Location", ANSI_WHITE, Name(sdb[l].object));
	} else if (sdb[x].status.landed) {
		if (sdb[x].status.connected) {
			snprintf(buffer, sizeof(buffer), "%s%16.16s:%s L+C:%-17.17s",
			  ANSI_YELLOW, "Location", ANSI_WHITE, Name(sdb[l].object));
		} else
			snprintf(buffer, sizeof(buffer), "%s%16.16s:%s L:%-18.18s",
			  ANSI_YELLOW, "Location", ANSI_WHITE, Name(sdb[l].object));
	} else
		snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.20s",
		  ANSI_YELLOW, "Location", ANSI_WHITE, "#-1 BAD UNKNOWN");

	return (buffer);
}

/* -------------------------------------------------------------------- */

char *format_Name (int x)
{
	static char buffer[50];

	snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.20s",
	  ANSI_YELLOW, "Name", ANSI_WHITE, Name(sdb[x].object));

	return (buffer);
}

/* -------------------------------------------------------------------- */

char *format_Type (int x)
{
	static char buffer[50];

	snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.20s",
	  ANSI_YELLOW, "Type", ANSI_WHITE, unparse_type(x));

	return (buffer);
}

/* -------------------------------------------------------------------- */

char *format_Class (int x)
{
	static char buffer[50];

	snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.20s",
	  ANSI_YELLOW, "Class", ANSI_WHITE, unparse_class(x));

	return (buffer);
}

/* -------------------------------------------------------------------- */

char *format_Range (int n1, int n2)
{
	static char buffer[50];

	snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.20s",
	  ANSI_YELLOW, "Range", ANSI_WHITE, unparse_distance(sdb2range(n1, n2)));

	return (buffer);
}

/* -------------------------------------------------------------------- */

char *format_Firing_Arc (int n1, int n2)
{
	static char buffer[50];

	snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.20s",
	  ANSI_YELLOW, "Firing Arc", ANSI_WHITE, unparse_arc(sdb2arc(n1, n2)));

	return (buffer);
}

/* -------------------------------------------------------------------- */

char *format_Contact_Arc (int n1, int n2)
{
	static char buffer[50];

	snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.20s",
	  ANSI_YELLOW, "Contact Arc", ANSI_WHITE, unparse_arc(sdb2arc(n2, n1)));

	return (buffer);
}

/* -------------------------------------------------------------------- */

char *format_Facing_Shield (int n1, int n2)
{
	static char buffer[50];
	int i;

	if (!sdb[n1].shield.exist) {
		snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.20s",
		  ANSI_YELLOW, "Facing Shield", ANSI_WHITE, "None");
	} else {
		i = sdb2shield(n1, n2);
		if (sdb2dissipation(n1, i)) {
			snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-17.17s UP",
			  ANSI_YELLOW, "Facing Shield", ANSI_WHITE, unparse_shield(i));
		} else
			snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.20s",
			  ANSI_YELLOW, "Facing Shield", ANSI_WHITE, unparse_shield(i));
	}

	return (buffer);
}

/* -------------------------------------------------------------------- */

char *format_Contact_Shield (int n1, int n2)
{
	int i;
	static char buffer[50];

	if (!sdb[n2].shield.exist) {
		snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.20s",
		  ANSI_YELLOW, "Contact Shield", ANSI_WHITE, "None");
	} else {
		i = sdb2shield(n2, n1);
		if (sdb2dissipation(n2, i)) {
			snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-17.17s UP",
			  ANSI_YELLOW, "Contact Shield", ANSI_WHITE, unparse_shield(i));
		} else
			snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.20s",
			  ANSI_YELLOW, "Contact Shield", ANSI_WHITE, unparse_shield(i));
	}

	return (buffer);
}

/* ------------------------------------------------------------------------ */

char *format_Bearing (int n1, int n2)
{
	static char buffer[50];

	snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.20s",
	  ANSI_YELLOW, "Bearing", ANSI_WHITE, unparse_bearing(n1, n2));

	return (buffer);
}

/* ------------------------------------------------------------------------ */

char *format_Course (int x)
{
	static char buffer[50];

	snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.20s",
	  ANSI_YELLOW, "Course", ANSI_WHITE, unparse_course(x));

	return (buffer);
}

/* -------------------------------------------------------------------- */

char *format_Speed (int x)
{
	static char buffer[50];

	snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.20s",
	  ANSI_YELLOW, "Speed", ANSI_WHITE, unparse_movement(x));

	return (buffer);
}

/* ------------------------------------------------------------------------ */

char *format_Displacement (int x)
{
	static char buffer[50];

	snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.20s",
	  ANSI_YELLOW, "Displacement", ANSI_WHITE, unparse_cargo(sdb[x].structure.displacement));

	return (buffer);
}

/* ------------------------------------------------------------------------ */

char *format_Radius (int x)
{
	static char buffer[50];

	snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.20s",
	  ANSI_YELLOW, "Radius", ANSI_WHITE, unparse_radius(sdb[x].structure.displacement));

	return (buffer);
}

/* ------------------------------------------------------------------------ */

char *format_Cargo_Carried (int x)
{
	static char buffer[50];

	snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.20s",
	  ANSI_YELLOW, "Cargo Carried", ANSI_WHITE, unparse_cargo(sdb[x].structure.cargo_mass));

	return (buffer);
}

/* ------------------------------------------------------------------------ */

char *format_Cargo_Cap (int x)
{
	static char buffer[50];

	snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.20s",
	  ANSI_YELLOW, "Cargo Capacity", ANSI_WHITE, unparse_cargo(sdb[x].structure.cargo_hold));

	return (buffer);
}

/* -------------------------------------------------------------------- */

char *format_Resolution (double a)
{
	static char buffer[50];

	snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.20s",
	  ANSI_YELLOW, "Resolution", ANSI_WHITE, unparse_percent_3(a));

	return (buffer);
}

/* -------------------------------------------------------------------- */

char *format_Shield_Freq (int x)
{
	static char buffer[50];

	snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.20s",
	  ANSI_YELLOW, "Shield Freq", ANSI_WHITE, unparse_freq(sdb[x].shield.freq));

	return (buffer);
}

/* -------------------------------------------------------------------- */

char *format_Cloak_Freq (int x)
{
	static char buffer[50];

	snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.20s",
	  ANSI_YELLOW, "Cloak Freq", ANSI_WHITE, unparse_freq(sdb[x].cloak.freq));

	return (buffer);
}

/* -------------------------------------------------------------------- */

char *format_Beam_Freq (int x)
{
	static char buffer[50];

	snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.20s",
	  ANSI_YELLOW, "Beam Freq", ANSI_WHITE, unparse_freq(sdb[x].beam.freq));

	return (buffer);
}

/* -------------------------------------------------------------------- */

char *format_Missile_Freq (int x)
{
	static char buffer[50];

	snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.20s",
	  ANSI_YELLOW, "Missile Freq", ANSI_WHITE, unparse_freq(sdb[x].missile.freq));

	return (buffer);
}

/* -------------------------------------------------------------------- */

char *format_Trans_Freq (int x)
{
	static char buffer[50];

	snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.20s",
	  ANSI_YELLOW, "Transporter Freq", ANSI_WHITE, unparse_freq(sdb[x].trans.freq));

	return (buffer);
}

/* -------------------------------------------------------------------- */

char *format_Tract_Freq (int x)
{
	static char buffer[50];

	snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.20s",
	  ANSI_YELLOW, "Tractor Freq", ANSI_WHITE, unparse_freq(sdb[x].tract.freq));

	return (buffer);
}

/* -------------------------------------------------------------------- */

char *format_Quadrant (int x)
{
	static char buffer[50];

	snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.20s",
	  ANSI_YELLOW, "Quadrant", ANSI_WHITE, unparse_quadrant(x));

	return (buffer);
}

/* -------------------------------------------------------------------- */

char *format_Territory (int x)
{
	static char buffer[50];

	snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.20s",
	  ANSI_YELLOW, "Territory", ANSI_WHITE, unparse_empire(x));

	return (buffer);
}

/* -------------------------------------------------------------------- */

char *format_Velocity (int x)
{
	static char buffer[50];

	snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.20s",
	  ANSI_YELLOW, "Velocity", ANSI_WHITE, unparse_velocity(x));

	return (buffer);
}

/* ------------------------------------------------------------------------ */

char *format_Helm_Power (int x)
{
	static char buffer[50];

	snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.20s",
	  ANSI_YELLOW, "Helm Power", ANSI_WHITE, unparse_power(sdb[x].alloc.helm * sdb[x].power.total));

	return (buffer);
}

/* ------------------------------------------------------------------------ */

char *format_Operations_Power (int x)
{
	static char buffer[50];

	snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.20s",
	  ANSI_YELLOW, "Ops Power", ANSI_WHITE, unparse_power(sdb[x].alloc.operations * sdb[x].power.total));

	return (buffer);
}

/* ------------------------------------------------------------------------ */

char *format_Tactical_Power (int x)
{
	static char buffer[50];

	snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.20s",
	  ANSI_YELLOW, "Tactical Power", ANSI_WHITE, unparse_power(sdb[x].alloc.tactical * sdb[x].power.total));

	return (buffer);
}

/* ------------------------------------------------------------------------ */

char *format_Total_Power (int x)
{
	static char buffer[50];

	snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.20s",
	  ANSI_YELLOW, "Total Power", ANSI_WHITE, unparse_power(sdb[x].power.total));

	return (buffer);
}

/* ------------------------------------------------------------------------ */

char *format_Cochranes (int x)
{
	static char buffer[50];

	snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.3f",
	  ANSI_YELLOW, "Cochranes", ANSI_WHITE, sdb[x].move.cochranes);

	return (buffer);
}

/* ------------------------------------------------------------------------ */

char *format_Visibility (int x)
{
	static char buffer[50];

	snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.20s",
	  ANSI_YELLOW, "Visibility", ANSI_WHITE, unparse_percent_3(sdb[x].sensor.visibility));

	return (buffer);
}

/* -------------------------------------------------------------------- */

char *format_LRS_Res (int x)
{
	static char buffer[50];
	double res;

	if (sdb[x].sensor.lrs_active) {
		res = sdb[x].sensor.lrs_damage * sdb[x].sensor.visibility;
		if (sdb[x].sensor.ew_active)
			res *= sdb2eccm_lrs(x);
	} else
		res = 0.0;

	snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.20s",
	  ANSI_YELLOW, "LRS Resolution", ANSI_WHITE, unparse_percent_3(res));

	return (buffer);
}

/* ------------------------------------------------------------------------ */

char *format_SRS_Res (int x)
{
	static char buffer[50];
	double res;

	if (sdb[x].sensor.srs_active) {
		res = sdb[x].sensor.srs_damage * sdb[x].sensor.visibility;
		if (sdb[x].sensor.ew_active)
			res *= sdb2eccm_srs(x);
	} else
		res = 0.0;

	snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.20s",
	  ANSI_YELLOW, "SRS Resolution", ANSI_WHITE,  unparse_percent_3(res));

	return (buffer);
}

/* ------------------------------------------------------------------------ */

char *format_LRS_Sig (int x)
{
	static char buffer[50];
		double sig = 1.0;

	if (sdb[x].cloak.active)
		sig *= sdb[x].cloak.level;

	sig *= sdb[x].sensor.visibility;
	sig *= sdb[x].move.out * sdb[x].move.out + 1.0;
	if (sdb[x].sensor.ew_active) {
		sig /= sdb2ecm_lrs(x);
	}
	snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.20s",
	  ANSI_YELLOW, "LRS Signature", ANSI_WHITE,  unparse_percent_3(sig));

	return (buffer);
}

/* ------------------------------------------------------------------------ */

char *format_SRS_Sig (int x)
{
	static char buffer[50];
		double sig = 1.0;

	if (sdb[x].cloak.active)
		sig *= sdb[x].cloak.level;

	sig *= sdb[x].sensor.visibility;
	sig *= 1.0 + sdb[x].power.main + (sdb[x].power.aux / 10.0) + (sdb[x].power.batt / 100.0);
	if (sdb[x].sensor.ew_active) {
		sig /= sdb2ecm_srs(x);
	}
	snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.20s",
	  ANSI_YELLOW, "SRS Signature", ANSI_WHITE, unparse_percent_3(sig));

	return (buffer);
}

/* ------------------------------------------------------------------------ */

char *format_Warp_Cruise (int x)
{
	static char buffer[50];

	snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.6f",
	  ANSI_YELLOW, "Warp Cruise", ANSI_WHITE, sdb[x].engine.warp_cruise);

	return (buffer);
}

/* ------------------------------------------------------------------------ */

char *format_Impulse_Cruise (int x)
{
	static char buffer[50];

	snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.20s",
	  ANSI_YELLOW, "Impulse Cruise", ANSI_WHITE, unparse_percent_3(sdb[x].engine.impulse_cruise));

	return (buffer);
}

/* ------------------------------------------------------------------------ */

char *format_Warp_Maximum (int x)
{
	static char buffer[50];

	snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.6f",
	  ANSI_YELLOW, "Warp Maximum", ANSI_WHITE, sdb[x].engine.warp_max);

	return (buffer);
}

/* ------------------------------------------------------------------------ */

char *format_Impulse_Maximum (int x)
{
	static char buffer[50];

	snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %-20.20s",
	  ANSI_YELLOW, "Impulse Maximum", ANSI_WHITE, unparse_percent_3(sdb[x].engine.impulse_max));

	return (buffer);
}
/* ------------------------------------------------------------------------ */

char *format_CF (int x)
{
	static char buffer[120];
	ATTR *a;
	float py;
	char *q;
	
  	a = atr_get(sdb[x].object, "CF");
	if (a != NULL) {
		q = safe_uncompress(a->value);
		py=atof(q);
		}
		
	if (py >50){
	snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %s%s%-20.30s %s",
	  ANSI_YELLOW, "System Stability", ANSI_RED, q,"%",ANSI_RED,ANSI_WHITE);	
	}
	else if (py <= 50 && py > 25) {
	snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %s%s%-20.30s  Stability in doubt. %s",
	  ANSI_YELLOW, "System Stability", ANSI_RED, q,"%",ANSI_RED,ANSI_WHITE);	
	}
	else if (py <= 25 && py >= 10){
	snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %s%s%-20.30s  CHECK ENGINE. Return to Starbase%s",
	  ANSI_YELLOW, "System Stability", ANSI_RED, q,"%",ANSI_RED,ANSI_WHITE);	
	}
	else {
	snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %s%s%-20.30s  CHECK ENGINE. Refit needed IMMEDIATELY!%s",
	  ANSI_YELLOW, "System Stability", ANSI_RED, q,"%",ANSI_RED,ANSI_WHITE);	
	}
		  
	if (a !=NULL)  {
		//Lets free up some ram
		free((Malloc_t) q);
	}



	return (buffer);
}


/* ------------------------------------------------------------------------ */

char *format_l_line (void)
{
	static char buffer[120];

	snprintf(buffer, sizeof(buffer), "%s-------------------------------------------------------------------------------%s\n",
	  ANSI_BLUE, ANSI_WHITE);

	return (buffer);
}

/* ------------------------------------------------------------------------ */

char *format_l_end (void)
{
	static char buffer[120];

	snprintf(buffer, sizeof(buffer), "%s%s-------------------------------------------------------------------------------%s%s",
	  ANSI_BLUE, ANSI_HILITE, ANSI_WHITE, ANSI_NORMAL);

	return (buffer);
}

/* ------------------------------------------------------------------------ */

char *format_l_Antimatter (int x)
{
	static char buffer[200];
	double m = sdb2max_antimatter(x);

	snprintf(buffer, sizeof(buffer), "%s%11.11s:%s %15.2f/%-15.2f tons: %4.4s %s\n",
	  ANSI_YELLOW, "Antimatter", ANSI_WHITE,
	  sdb[x].fuel.antimatter / 1000000.0, m / 1000000.0,
	  unparse_percent(sdb[x].fuel.antimatter / m),
	  ansi_stoplight_scale(sdb[x].fuel.antimatter / m, 20));

	return (buffer);
}

/* ------------------------------------------------------------------------ */

char *format_l_Deuterium (int x)
{
	static char buffer[200];
	double m = sdb2max_deuterium(x);

	snprintf(buffer, sizeof(buffer), "%s%11.11s:%s %15.2f/%-15.2f tons: %4.4s %s\n",
	  ANSI_YELLOW, "Deuterium", ANSI_WHITE,
	  sdb[x].fuel.deuterium / 1000000.0, m / 1000000.0,
	  unparse_percent(sdb[x].fuel.deuterium / m),
	  ansi_stoplight_scale(sdb[x].fuel.deuterium / m, 20));

	return (buffer);
}

/* ------------------------------------------------------------------------ */

char *format_l_Reserves (int x)
{
	static char buffer[200];
	double m = sdb2max_reserve(x);

	snprintf(buffer, sizeof(buffer), "%s%11.11s:%s %15.2f/%-15.2f GW*H: %4.4s %s\n",
	  ANSI_YELLOW, "Reserve", ANSI_WHITE,
	  sdb[x].fuel.reserves / 3600.0, m / 3600.0,
	  unparse_percent(sdb[x].fuel.reserves / m),
	  ansi_stoplight_scale(sdb[x].fuel.reserves / m, 20));

	return (buffer);
}

/* ------------------------------------------------------------------------ */

char *format_l_Galactic (int x)
{
	static char buffer[120];

	snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %15.3f %15.3f %15.3f\n",
	  ANSI_YELLOW, "Galactic X Y Z", ANSI_WHITE,
	  su2pc(sdb[x].coords.x),
	  su2pc(sdb[x].coords.y),
	  su2pc(sdb[x].coords.z));

	return (buffer);
}

/* ------------------------------------------------------------------------ */

char *format_l_Relative (int x)
{
	static char buffer[120];

	snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %15.3f %15.3f %15.3f\n",
	  ANSI_YELLOW, "Relative X Y Z", ANSI_WHITE,
	  su2pc(sdb[x].coords.x - sdb[x].coords.xo),
	  su2pc(sdb[x].coords.y - sdb[x].coords.yo),
	  su2pc(sdb[x].coords.z - sdb[x].coords.zo));

	return (buffer);
}

/* ------------------------------------------------------------------------ */

char *format_l_Destination (int x)
{
	static char buffer[120];

	snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %15.3f %15.3f %15.3f\n",
	  ANSI_YELLOW, "Destinat X Y Z", ANSI_WHITE,
	  su2pc(sdb[x].coords.xd - sdb[x].coords.xo),
	  su2pc(sdb[x].coords.yd - sdb[x].coords.yo),
	  su2pc(sdb[x].coords.zd - sdb[x].coords.zo));

	return (buffer);
}

/* ------------------------------------------------------------------------ */

char *format_l_Beam_Cap (int x)
{
	static char buffer[200];

	if (sdb[x].beam.in == 0.0) {
		snprintf(buffer, sizeof(buffer), "%s%16.16s:%s (%s)\n",
		  ANSI_YELLOW, "Beam Power", ANSI_WHITE,
		  unparse_power(sdb[n].power.total * sdb[n].alloc.beams));
	} else {
		snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %15.15s/",
		  ANSI_YELLOW, "Beam Power", ANSI_WHITE,
		  unparse_power(sdb[x].beam.out));
		strncat(buffer, tprintf("%-15.15s: %5.5s %s\n",
		  unparse_power(sdb[x].beam.in),
		  unparse_percent(sdb[x].beam.out / sdb[x].beam.in),
		  ansi_rainbow_scale(sdb[x].beam.out / sdb[x].beam.in, 20)), sizeof(buffer) - 1);
	}

	return (buffer);
}

/* ------------------------------------------------------------------------ */

char *format_l_Missile_Cap (int x)
{
	static char buffer[200];

	if (sdb[x].missile.in == 0.0) {
		snprintf(buffer, sizeof(buffer), "%s%16.16s:%s (%s)\n",
		  ANSI_YELLOW, "Missile Power", ANSI_WHITE,
		  unparse_power(sdb[n].power.total * sdb[n].alloc.missiles));
	} else {
		snprintf(buffer, sizeof(buffer), "%s%16.16s:%s %15.15s/",
		  ANSI_YELLOW, "Missile Power", ANSI_WHITE,
		  unparse_power(sdb[x].missile.out));
		strncat(buffer, tprintf("%-15.15s: %5.5s %s\n",
		  unparse_power(sdb[x].missile.in),
		  unparse_percent(sdb[x].missile.out / sdb[x].missile.in),
		  ansi_rainbow_scale(sdb[x].missile.out / sdb[x].missile.in, 20)), sizeof(buffer) - 1);
	}

	return (buffer);
}

/* ------------------------------------------------------------------------ */

char *format_l_Docked (int x)
{
	static char buffer[800];
	register int i;
	int first = 1;

	snprintf(buffer, sizeof(buffer),
	  "%sDocked here:%s", ANSI_YELLOW, ANSI_WHITE);

	for (i = 1 ; i <= max_space_objects ; ++i)
		if (sdb[i].location == x)
			if (sdb[i].status.docked) {
				if (!sdb[i].cloak.active || sdb[i].status.connected) {
					if (first) {
						first = 0;
						strncat(buffer, "\n", sizeof(buffer) - 1);
					} else
						strncat(buffer, ", ", sizeof(buffer) - 1);
					strncat(buffer, Name(sdb[i].object), sizeof(buffer) - 1);
				}
			}
	strncat(buffer, "\n", sizeof(buffer) - 1);

	return (buffer);
}

/* ------------------------------------------------------------------------ */

char *format_l_Landed (int x)
{
	static char buffer[800];
		register int i;
	int first = 1;

	snprintf(buffer, sizeof(buffer),
	  "%sLanded here:%s", ANSI_YELLOW, ANSI_WHITE);

	for (i = 1 ; i <= max_space_objects ; ++i)
		if (sdb[i].location == x)
			if (sdb[i].status.landed) {
				if (!sdb[i].cloak.active || sdb[i].status.connected) {
					if (first) {
						first = 0;
						strncat(buffer, "\n", sizeof(buffer) - 1);
					} else
						strncat(buffer, ", ", sizeof(buffer) - 1);
					strncat(buffer, Name(sdb[i].object), sizeof(buffer) - 1);
				}
			}
	strncat(buffer, "\n", sizeof(buffer) - 1);

	return (buffer);
}

/* ------------------------------------------------------------------------ */

char *format_s_header (void)
{
	static char buffer[200];

	snprintf(buffer, sizeof(buffer),
	  "%s%sSystem Name      Funct Status          Specifics%s\n%s---------------- ----- --------------- ----------------------------------------%s\n",
	   ANSI_HILITE,ANSI_YELLOW,ANSI_NORMAL, ANSI_BLUE, ANSI_WHITE);

	return (buffer);
}

/* ------------------------------------------------------------------------ */

char *format_s_MA_Reactor (int x)
{
	static char buffer[120];

	snprintf(buffer, sizeof(buffer),
	  "%s%-16.16s%s %5.5s %-15.15s ",
	  ANSI_YELLOW, system_name[8], ANSI_WHITE,
	  unparse_percent(sdb[x].main.damage),
	  unparse_damage(sdb[x].main.damage));

	if (sdb[x].main.out == 0.0) {
		strncat(buffer, tprintf("Rating: %s",
		  unparse_power(sdb[x].main.gw)), sizeof(buffer) - 1);
		strncat(buffer, tprintf(" (%s) EPS\n",
		  unparse_power(sdb[x].main.gw * 100.0)), sizeof(buffer) - 1);
	} else if (sdb[x].main.out > sdb[x].main.damage) {
		strncat(buffer, tprintf("%s EPS: %4.4s %s%sOVERLOAD%s%s\n",
		  unparse_power(sdb[x].power.main),
		  unparse_percent(sdb[x].main.out),
		  ANSI_RED, ANSI_BLINK, ANSI_WHITE, ANSI_NORMAL), sizeof(buffer) - 1);
	} else {
		strncat(buffer, tprintf("%s EPS: %4.4s\n",
		  unparse_power(sdb[x].power.main),
		  unparse_percent(sdb[x].main.out)), sizeof(buffer) - 1);
	}

	return (buffer);
}

/* ------------------------------------------------------------------------ */

char *format_s_Fusion_Reactor (int x)
{
	static char buffer[120];

	snprintf(buffer, sizeof(buffer), "%s%-16.16s%s %5.5s %-15.15s ",
	  ANSI_YELLOW, system_name[1], ANSI_WHITE,
	  unparse_percent(sdb[x].aux.damage),
	  unparse_damage(sdb[x].aux.damage));

	if (sdb[x].aux.out == 0.0) {
		strncat(buffer, tprintf("Rating: %s",
		  unparse_power(sdb[x].aux.gw)), sizeof(buffer) - 1);
		strncat(buffer, tprintf(" (%s) EPS\n",
		  unparse_power(sdb[x].aux.gw * 10.0)), sizeof(buffer) - 1);
	} else if (sdb[x].aux.out > sdb[x].aux.damage) {
		strncat(buffer, tprintf("%s EPS: %4.4s %s%sOVERLOAD%s%s\n",
		  unparse_power(sdb[x].power.aux),
		  unparse_percent(sdb[x].aux.out),
		  ANSI_RED, ANSI_BLINK, ANSI_WHITE, ANSI_NORMAL), sizeof(buffer) - 1);
	} else {
		strncat(buffer, tprintf("%s EPS: %4.4s\n",
		  unparse_power(sdb[x].power.aux),
		  unparse_percent(sdb[x].aux.out)), sizeof(buffer) - 1);
	}

	return (buffer);
}

/* ------------------------------------------------------------------------ */

char *format_s_Batteries (int x)
{
	static char buffer[120];

	snprintf(buffer, sizeof(buffer), "%s%-16.16s%s %5.5s %-15.15s ",
	  ANSI_YELLOW, system_name[2], ANSI_WHITE,
	  unparse_percent(sdb[x].batt.damage),
	  unparse_damage(sdb[x].batt.damage));

	if (sdb[x].batt.out == 0.0) {
		strncat(buffer, tprintf("Rating: %s EPS\n",
		  unparse_power(sdb[x].batt.gw)), sizeof(buffer) - 1);
	} else {
		strncat(buffer, tprintf("%s EPS: %4.4s\n",
		  unparse_power(sdb[x].power.batt),
		  unparse_percent(sdb[x].batt.out)), sizeof(buffer) - 1);
	}

	return (buffer);
}

/* ------------------------------------------------------------------------ */

char *format_s_Warp_Drive (int x)
{
	static char buffer[120];

	snprintf(buffer, sizeof(buffer), "%s%-16.16s%s %5.5s %-15.15s ",
	  ANSI_YELLOW, system_name[14], ANSI_WHITE,
	  unparse_percent(sdb[x].engine.warp_damage),
	  unparse_damage(sdb[x].engine.warp_damage));

	if (fabs(sdb[x].move.out) < 1.0 && fabs(sdb[x].move.in) < 1.0) {
		strncat(buffer, tprintf("Rating: Warp %.3f (%.3f)\n",
		  sdb[x].engine.warp_cruise, sdb[x].engine.warp_max), sizeof(buffer) - 1);
	} else if (fabs(sdb[x].move.out) > sdb[x].engine.warp_cruise) {
		strncat(buffer, tprintf("Warp %.6f (%.6f) %s%sOVERLOAD%s%s\n",
		  sdb[x].move.out, sdb[x].engine.warp_max,
		  ANSI_RED, ANSI_BLINK, ANSI_WHITE, ANSI_NORMAL), sizeof(buffer) - 1);
	} else
		strncat(buffer, tprintf("Warp %.6f (%.6f)\n",
		  sdb[x].move.out, sdb[x].engine.warp_max), sizeof(buffer) - 1);

	return (buffer);
}

/* ------------------------------------------------------------------------ */

char *format_s_Impulse_Drive (int x)
{
	static char buffer[120];

	snprintf(buffer, sizeof(buffer), "%s%-16.16s%s %5.5s %-15.15s ",
	  ANSI_YELLOW, system_name[6], ANSI_WHITE,
	  unparse_percent(sdb[x].engine.impulse_damage),
	  unparse_damage(sdb[x].engine.impulse_damage));

	if ((fabs(sdb[x].move.out) > sdb[x].engine.impulse_max &&
	  fabs(sdb[x].move.in) > sdb[x].engine.impulse_max) || sdb[x].move.out == 0.0) {
		strncat(buffer, tprintf("Rating: %.3f%% (%.3f%%) Impulse\n",
		  sdb[x].engine.impulse_cruise * 100.0,
		  sdb[x].engine.impulse_max * 100.0), sizeof(buffer) - 1);
	} else if (fabs(sdb[x].move.out) > sdb[x].engine.impulse_cruise && fabs(sdb[x].move.in) < 1.0) {
		strncat(buffer, tprintf("%.3f%% (%.3f%%) Impulse %s%sOVERLOAD%s%s\n",
		  sdb[x].move.out * 100.0, sdb[x].engine.impulse_max * 100.0,
		  ANSI_RED, ANSI_BLINK, ANSI_WHITE, ANSI_NORMAL), sizeof(buffer) - 1);
	} else
		strncat(buffer, tprintf("%.3f%% (%.3f%%) Impulse\n",
		  sdb[x].move.out * 100.0, sdb[x].engine.impulse_max * 100.0), sizeof(buffer) - 1);

	return (buffer);
}

/* ------------------------------------------------------------------------ */

char *format_s_Structure (int x)
{
	static char buffer[120];

	snprintf(buffer, sizeof(buffer), "%s%-16.16s%s %5.5s %-15.15s Rating: %.3f (%d) SS\n",
	  ANSI_YELLOW, system_name[0], ANSI_WHITE,
	  unparse_percent(sdb[x].structure.superstructure / sdb[x].structure.max_structure),
	  unparse_damage(sdb[x].structure.superstructure / sdb[x].structure.max_structure),
	  sdb[x].structure.superstructure, sdb[x].structure.max_structure);

	return (buffer);
}

/* ------------Added by Bill 4-9-2002 to show life support--------------------- */

char *format_s_Life_Support (int x)
{
	static char buffer[120];
	
	snprintf(buffer, sizeof(buffer), "%s%-16.16s%s %5.5s %-15.15s ",
	  ANSI_YELLOW, system_name[15], ANSI_WHITE,
	  unparse_percent(sdb[x].lifesupport.damage),
	  unparse_damage(sdb[x].lifesupport.damage));

	  if (sdb[x].lifesupport.active) {
		strncat(buffer, tprintf("Status: (%s)\n",
		"ONLINE"), sizeof(buffer) - 1);	  	
	  } else {
		strncat(buffer, tprintf("Status: (%s%s%s%s%s)\n",ANSI_RED, ANSI_BLINK,"OFFLINE", ANSI_WHITE, ANSI_NORMAL), sizeof(buffer) - 1);	  	
		 }  	  
  
	return (buffer);
}

/* ------------Added by Bill 4-9-2002 to show life support--------------------- */

char *format_Damage_History (int x)
{
	static char buffer[120];
	char *dmghistory;

	
	/*Engine Damage Check*/
	
	if (sdb[x].engine.warp_exist || sdb[x].engine.impulse_exist) {
	if (sdb[x].engine.lasthit != 999)  {
		dmghistory=beam_name[sdb[x].engine.lasthit];
		snprintf(buffer, sizeof(buffer), "%s%-16.16s%s %sTraces of %s blasts\n",ANSI_YELLOW, "Engines", ANSI_WHITE,ANSI_RED,dmghistory);
		} 
	else {
		dmghistory="No Weapons Traces";	
		snprintf(buffer, sizeof(buffer), "%s%-16.16s%s %s\n",ANSI_YELLOW, "Engines", ANSI_WHITE,dmghistory);
	}
	}
	
	/*Sensor Damage Check*/
	if (sdb[x].sensor.lrs_exist || sdb[x].sensor.srs_exist || sdb[x].sensor.ew_exist){
	if (sdb[x].sensor.lasthit != 999)  {
		dmghistory=beam_name[sdb[x].sensor.lasthit];
		strncat(buffer, tprintf("%s%-16.16s%s %sTraces of %s blasts\n",ANSI_YELLOW, "Sensors", ANSI_WHITE,ANSI_RED,dmghistory), sizeof(buffer) - 1);
		}
	else {
		dmghistory="No Weapons Traces";	
		strncat(buffer, tprintf("%s%-16.16s%s %s\n",ANSI_YELLOW, "Sensors", ANSI_WHITE,dmghistory),sizeof(buffer) - 1);
	}}

	/*shield Damage Check*/
	if (sdb[x].shield.exist) {
	if (sdb[x].shield.lasthit != 999 )  {
		dmghistory=beam_name[sdb[x].shield.lasthit];
		strncat(buffer, tprintf("%s%-16.16s%s %sTraces of %s blasts\n",ANSI_YELLOW, "Shields", ANSI_WHITE,ANSI_RED,dmghistory), sizeof(buffer) - 1);
		}
	else {
		dmghistory="No Weapons Traces";	
		strncat(buffer, tprintf("%s%-16.16s%s %s\n",ANSI_YELLOW, "Shields", ANSI_WHITE,dmghistory),sizeof(buffer) - 1);
	}}

	/*cloak Damage Check*/
	if (sdb[x].cloak.exist) {
	if (sdb[x].cloak.lasthit != 999)  {
		dmghistory=beam_name[sdb[x].cloak.lasthit];
		strncat(buffer, tprintf("%s%-16.16s%s %sTraces of %s blasts\n",ANSI_YELLOW, "Cloak", ANSI_WHITE,ANSI_RED,dmghistory), sizeof(buffer) - 1);
		}
	else {
		dmghistory="No Weapons Traces";	
		strncat(buffer, tprintf("%s%-16.16s%s %s\n",ANSI_YELLOW, "Cloak", ANSI_WHITE,dmghistory),sizeof(buffer) - 1);
	}}
	
	/*trans Damage Check*/
	if (sdb[x].trans.exist){
	if (sdb[x].trans.lasthit != 999)  {
		dmghistory=beam_name[sdb[x].trans.lasthit];
		strncat(buffer, tprintf("%s%-16.16s%s %sTraces of %s blasts\n",ANSI_YELLOW, "Transporter", ANSI_WHITE,ANSI_RED,dmghistory), sizeof(buffer) - 1);
		}
	else {
		dmghistory="No Weapons Traces";	
		strncat(buffer, tprintf("%s%-16.16s%s %s\n",ANSI_YELLOW, "Transporter", ANSI_WHITE,dmghistory),sizeof(buffer) - 1);
	}}
	/*lifesupport Damage Check*/
	if (sdb[x].lifesupport.lasthit != 999)  {
		dmghistory=beam_name[sdb[x].lifesupport.lasthit];
		strncat(buffer, tprintf("%s%-16.16s%s %sTraces of %s blasts\n",ANSI_YELLOW, "Life Support", ANSI_WHITE,ANSI_RED,dmghistory), sizeof(buffer) - 1);
		}
	else {
		dmghistory="No Weapons Traces";	
		strncat(buffer, tprintf("%s%-16.16s%s %s\n",ANSI_YELLOW, "Life Support", ANSI_WHITE,dmghistory),sizeof(buffer) - 1);
	}
	/*tract Damage Check*/
	if (sdb[x].tract.exist) {
	if (sdb[x].tract.lasthit != 999 )  {
		dmghistory=beam_name[sdb[x].tract.lasthit];
		strncat(buffer, tprintf("%s%-16.16s%s %sTraces of %s blasts\n",ANSI_YELLOW, "Tractor Beam", ANSI_WHITE,ANSI_RED,dmghistory), sizeof(buffer) - 1);
		}
	else {
		dmghistory="No Weapons Traces";	
		strncat(buffer, tprintf("%s%-16.16s%s %s\n",ANSI_YELLOW, "Tractor Beam", ANSI_WHITE,dmghistory),sizeof(buffer) - 1);
	}}
	/*main Damage Check*/
	if (sdb[x].main.exist) {
	if (sdb[x].main.lasthit != 999)  {
		dmghistory=beam_name[sdb[x].main.lasthit];
		strncat(buffer, tprintf("%s%-16.16s%s %sTraces of %s blasts\n",ANSI_YELLOW, "Warp Reactor", ANSI_WHITE,ANSI_RED,dmghistory), sizeof(buffer) - 1);
		}
	else {
		dmghistory="No Weapons Traces";	
		strncat(buffer, tprintf("%s%-16.16s%s %s\n",ANSI_YELLOW, "Warp Reactor", ANSI_WHITE,dmghistory),sizeof(buffer) - 1);
	}}
	/*aux Damage Check*/
	if (sdb[x].aux.exist){
	if (sdb[x].aux.lasthit != 999)  {
		dmghistory=beam_name[sdb[x].aux.lasthit];
		strncat(buffer, tprintf("%s%-16.16s%s %sTraces of %s blasts\n",ANSI_YELLOW, "Fusion Reactor", ANSI_WHITE,ANSI_RED,dmghistory), sizeof(buffer) - 1);
		}
	else {
		dmghistory="No Weapons Traces";	
		strncat(buffer, tprintf("%s%-16.16s%s %s\n",ANSI_YELLOW, "Fusion Reactor", ANSI_WHITE,dmghistory),sizeof(buffer) - 1);
	}}
	/*batt Damage Check*/
	if (sdb[x].batt.exist){
	if (sdb[x].batt.lasthit != 999)  {
		dmghistory=beam_name[sdb[x].batt.lasthit];
		strncat(buffer, tprintf("%s%-16.16s%s %sTraces of %s blasts\n",ANSI_YELLOW, "Batteries", ANSI_WHITE,ANSI_RED,dmghistory), sizeof(buffer) - 1);
		}
	else {
		dmghistory="No Weapons Traces";	
		strncat(buffer, tprintf("%s%-16.16s%s %s\n",ANSI_YELLOW, "Batteries", ANSI_WHITE,dmghistory),sizeof(buffer) - 1);
	}}
	/*beam Damage Check*/
	if (sdb[x].beam.exist){
	if (sdb[x].beam.lasthit != 999)  {
		dmghistory=beam_name[sdb[x].beam.lasthit];
		strncat(buffer, tprintf("%s%-16.16s%s %sTraces of %s blasts\n",ANSI_YELLOW, "Beam Weapons", ANSI_WHITE,ANSI_RED,dmghistory), sizeof(buffer) - 1);
		}
	else {
		dmghistory="No Weapons Traces";	
		strncat(buffer, tprintf("%s%-16.16s%s %s\n",ANSI_YELLOW, "Beam Weapons", ANSI_WHITE,dmghistory),sizeof(buffer) - 1);
	}}
	/*missile Damage Check*/
	if (sdb[x].missile.exist){
	if (sdb[x].missile.lasthit != 999)  {
		dmghistory=beam_name[sdb[x].missile.lasthit];
		strncat(buffer, tprintf("%s%-16.16s%s %sTraces of %s blasts\n",ANSI_YELLOW, "Missile Weapons", ANSI_WHITE,ANSI_RED,dmghistory), sizeof(buffer) - 1);
		}
	else {
		dmghistory="No Weapons Traces";	
		strncat(buffer, tprintf("%s%-16.16s%s %s\n",ANSI_YELLOW, "Missile Weapons", ANSI_WHITE,dmghistory),sizeof(buffer) - 1);
	}}

	
	
/*	  if (sdb[x].lifesupport.active) {
*		strncat(buffer, tprintf("Status: (%s)\n","ONLINE"), sizeof(buffer) - 1);	  	
*		
*	  } else {
*		strncat(buffer, tprintf("Status: (%s%s%s%s%s)\n",ANSI_RED, ANSI_BLINK,"OFFLINE", ANSI_WHITE, ANSI_NORMAL), sizeof(buffer) - 1);	  	
*		 }  	  
*/  
	
	

	return (buffer);
}


/* ------------------------------------------------------------------------ */

char *format_s_Transporters (int x)
{
	static char buffer[120];

	snprintf(buffer, sizeof(buffer), "%s%-16.16s%s %5.5s %-15.15s ",
	  ANSI_YELLOW, system_name[13], ANSI_WHITE,
	  unparse_percent(sdb[x].trans.damage),
	  unparse_damage(sdb[x].trans.damage));

	if (!sdb[x].trans.active) {
		strncat(buffer, tprintf("Rating: %s (%s)\n",
		  unparse_distance(MAX_TRANSPORTER_DISTANCE * sdb[x].tech.sensors),
		  unparse_power(sdb[x].power.total * sdb[x].alloc.transporters)), sizeof(buffer) - 1);
	} else {
		if (!sdb[x].trans.s_lock) {
			strncat(buffer, "Source Unlocked, ", sizeof(buffer) - 1);
		} else
			strncat(buffer, tprintf("S Lock: %.11s, ",
		 	  unparse_identity(x, sdb[x].trans.s_lock)), sizeof(buffer) - 1);
		if (!sdb[x].trans.d_lock) {
			strncat(buffer, "Dest Unlocked\n", sizeof(buffer) - 1);
		} else
			strncat(buffer, tprintf("D Lock: %.11s\n",
		 	  unparse_identity(x, sdb[x].trans.d_lock)), sizeof(buffer) - 1);
	}


	return (buffer);
}

/* ------------------------------------------------------------------------ */

char *format_s_Tractors (int x)
{
	static char buffer[120];

	snprintf(buffer, sizeof(buffer), "%s%-16.16s%s %5.5s %-15.15s ",
	  ANSI_YELLOW, system_name[12], ANSI_WHITE,
	  unparse_percent(sdb[x].tract.damage),
	  unparse_damage(sdb[x].tract.damage));

	if (!sdb[x].tract.active) {
		if (sdb[x].status.tractored) {
			strncat(buffer, tprintf("Rating: Tractored by %.19s\n",
			  unparse_identity(x, sdb[x].status.tractored)), sizeof(buffer) - 1);
 		} else
			strncat(buffer, tprintf("Rating: (%s)\n",
			  unparse_power(sdb[x].power.total * sdb[x].alloc.tractors)), sizeof(buffer) - 1);
	} else
		if (sdb[x].status.tractored) {
			strncat(buffer, tprintf("Tractored by %.27s\n",
			  unparse_identity(x, sdb[x].status.tractored)), sizeof(buffer) - 1);
		} else if (sdb[x].status.tractoring) {
			strncat(buffer, tprintf("Tractoring %.29s\n",
			  unparse_identity(x, sdb[x].status.tractoring)), sizeof(buffer) - 1);
		} else
			strncat(buffer, tprintf("%10.10s: Unlocked\n",
			  unparse_power(sdb[x].power.total * sdb[x].alloc.tractors)), sizeof(buffer) - 1);

	return (buffer);

}

/* ------------------------------------------------------------------------ */

char *format_s_Cloak (int x)
{
	static char buffer[120];

	snprintf(buffer, sizeof(buffer), "%s%-16.16s%s %5.5s %-15.15s ",
	  ANSI_YELLOW, system_name[4], ANSI_WHITE,
	  unparse_percent(sdb[x].cloak.damage),
	  unparse_damage(sdb[x].cloak.damage));

	if (!sdb[x].cloak.active) {
		strncat(buffer, tprintf("Rating: %s",
		  unparse_power(sdb[x].cloak.cost)), sizeof(buffer) - 1);
		strncat(buffer, tprintf(" (%s)\n",
		  unparse_power(sdb[x].alloc.cloak * sdb[x].power.total)), sizeof(buffer) - 1);
	} else
		strncat(buffer, tprintf("%10.10s: %5.5s Cloaked\n",
		  unparse_power(sdb[x].alloc.cloak * sdb[x].power.total),
		  unparse_percent(sdb[x].alloc.cloak * sdb[x].power.total / sdb[x].cloak.cost)), sizeof(buffer) - 1);

	return (buffer);
}

/* ------------------------------------------------------------------------ */

char *format_s_LRS (int x)
{
	static char buffer[120];

	snprintf(buffer, sizeof(buffer), "%s%-16.16s%s %5.5s %-15.15s ",
	  ANSI_YELLOW, system_name[7], ANSI_WHITE,
	  unparse_percent(sdb[x].sensor.lrs_damage),
	  unparse_damage(sdb[x].sensor.lrs_damage));

	if (!sdb[x].sensor.lrs_active) {
		strncat(buffer, tprintf("Rating: %s\n",
		  unparse_distance(sdb[x].tech.sensors * PARSEC)), sizeof(buffer) - 1);
	} else
		strncat(buffer, tprintf("Resolution: %s\n",
		  unparse_percent_3(sdb[x].sensor.lrs_damage * sdb[x].sensor.visibility)), sizeof(buffer) - 1);

	return (buffer);
}

/* ------------------------------------------------------------------------ */

char *format_s_SRS (int x)
{
	static char buffer[120];

	snprintf(buffer, sizeof(buffer), "%s%-16.16s%s %5.5s %-15.15s ",
	  ANSI_YELLOW, system_name[11], ANSI_WHITE,
	  unparse_percent(sdb[x].sensor.srs_damage),
	  unparse_damage(sdb[x].sensor.srs_damage));

	if (!sdb[x].sensor.srs_active) {
		strncat(buffer, tprintf("Rating: %s\n",
		  unparse_distance(sdb[x].tech.sensors * 1000000.0)), sizeof(buffer) - 1);
	} else
		strncat(buffer, tprintf("Resolution: %s\n",
		  unparse_percent_3(sdb[x].sensor.srs_damage * sdb[x].sensor.visibility)), sizeof(buffer) - 1);

	return (buffer);
}

/* ------------------------------------------------------------------------ */

char *format_s_EW (int x)
{
	static char buffer[120];

	snprintf(buffer, sizeof(buffer), "%s%-16.16s%s %5.5s %-15.15s ",
	  ANSI_YELLOW, system_name[5], ANSI_WHITE,
	  unparse_percent(sdb[x].sensor.ew_damage),
	  unparse_damage(sdb[x].sensor.ew_damage));

	if (!sdb[x].sensor.ew_active) {
		strncat(buffer, tprintf("Rating: (%s)\n",
		  unparse_power(sdb[x].power.total * sdb[x].alloc.sensors)), sizeof(buffer) - 1);
	} else
		strncat(buffer, tprintf("LR/SR ECM %.1f/%.1f ECCM %.1f/%.1f\n",
		  sdb2ecm_lrs(x), sdb2ecm_srs(x), sdb2eccm_lrs(x), sdb2eccm_srs(x)), sizeof(buffer) - 1);

	return (buffer);
}

/* ------------------------------------------------------------------------ */

char *format_s_Shield (int x)
{
	static char buffer[100 * MAX_SHIELD_NAME];
	register int i;
	double d;

	strncpy(buffer, "", sizeof(buffer) - 1);

	for (i = 0 ; i < MAX_SHIELD_NAME ; ++i) {

		strncat(buffer, tprintf("%s%-16.16s%s %5.5s %-15.15s ",
		  ANSI_YELLOW, unparse_shield(i), ANSI_WHITE,
		  unparse_percent(sdb[x].shield.damage[i]),
		  unparse_damage(sdb[x].shield.damage[i])), sizeof(buffer) - 1);

		d = sdb2dissipation(x, i);
		if (!sdb[x].shield.active[i] || !d) {
			strncat(buffer, tprintf("Rating: %s:",
			  unparse_power(sdb[x].shield.maximum / sdb[x].shield.ratio)), sizeof(buffer) - 1);
			strncat(buffer, tprintf("%s",
			  unparse_power(sdb[x].shield.maximum)), sizeof(buffer) - 1);
			strncat(buffer, tprintf(" (%s)\n",
			  unparse_power(sdb[x].alloc.shield[i] * sdb[x].power.total)), sizeof(buffer) - 1);
		} else {
			strncat(buffer, tprintf("%10.10s: %5.5s %s\n",
			  unparse_power(d), unparse_percent(1.0 - (1.0 / d)),
			  ansi_red_scale(sdb[x].shield.damage[i], 20)), sizeof(buffer) - 1);
		}
	}

	return (buffer);
}

/* ------------------------------------------------------------------------ */

char *format_s_Beam (int x, int a)
{
	static char buffer[100 * MAX_BEAM_BANKS];
	register int i;
	int c, c_arc;
	double range;
	
	strncpy(buffer, "", sizeof(buffer) - 1);

	for (i = 0 ; i < sdb[x].beam.banks ; ++i) {

		strncat(buffer, tprintf("%s%2d %-13.13s%s %5.5s %-15.15s ",
		  ANSI_YELLOW, i + 1, unparse_beam(sdb[x].blist.name[i]), ANSI_WHITE,
		  unparse_percent(sdb[x].blist.damage[i]),
		  unparse_damage(sdb[x].blist.damage[i])), sizeof(buffer) - 1);
		/* ADDED THE FOLLOWING TO DISPLAY CHARGE TIMER */	
		/* Mordak Aspace v1.0.0p1*/
		if (sdb[x].beam.out < sdb[x].blist.cost[i])
			strncat(buffer, tprintf("NP "), sizeof(buffer) -1 );
		else
		    if(!(sdb[x].blist.load[i] + (beam_refresh / sdb[x].tech.firing) > time(NULL)))
				strncat(buffer, tprintf("RR "), sizeof(buffer) -1);
			else
				strncat(buffer, tprintf("%2.0f ",(beam_refresh / sdb[x].tech.firing)+(sdb[x].blist.load[i] - time(NULL))) , sizeof(buffer) - 1); 
			/* END Aspace v1.0.0p1  */
		if (fabs(sdb[x].move.out) < 1.0) {
			range = sdb[x].blist.range[i] * 10.0;
		} else
			range = sdb[x].blist.range[i] * PARSEC / 1000.0;
		if (!sdb[x].blist.active[i]) {
/* Mordak Aspace v1.0.0p1 - changed from 10->7 to fit charge timer */
			strncat(buffer, tprintf("%5.5s   %7.7s/",
			  unparse_arc(sdb[x].blist.arcs[i]),
	unparse_power(sdb[x].blist.cost[i] / (double) sdb[x].blist.recycle[i] * sdb[x].tech.firing)) , sizeof(buffer) - 1);
			strncat(buffer, tprintf("%-10.10s %10.10s\n",
			  unparse_power(sdb[x].blist.cost[i] + sdb[x].blist.bonus[i]),
			  unparse_distance(range)), sizeof(buffer) - 1);
		} else if (!sdb[x].blist.lock[i]) {
			strncat(buffer, tprintf("%5.5s   Unlocked              %10.10s\n",
			  unparse_arc(sdb[x].blist.arcs[i]),
			  unparse_distance(range)), sizeof(buffer) - 1);
		} else {
			c = sdb2contact(x, sdb[x].blist.lock[i]);
			c_arc = sdb2arc(x, sdb[x].blist.lock[i]);
			if (c == SENSOR_FAIL) {
		 		strncat(buffer, tprintf("%5.5s   ERROR\n",
				  unparse_arc(sdb[x].blist.arcs[i])), sizeof(buffer) - 1);
			} else if (arc_check(c_arc, sdb[x].blist.arcs[i]) == ARC_FAIL) {
				/* Mordak Aspace v1.0.0p1 - Change from 15->12 to fit charge timer */
				strncat(buffer, tprintf("%5.5s - Lock: %-12.12s %10.10s\n",
			 	  unparse_arc(sdb[x].blist.arcs[i]),
			 	  unparse_identity(a, sdb[x].blist.lock[i]),
			 	  unparse_distance(range)), sizeof(buffer) - 1);
             } else if (sdb[x].blist.load[i] + ((double) sdb[x].blist.recycle[i] / sdb[x].tech.firing) > time(NULL)) {
				/* Mordak Aspace v1.0.0p1 - Change from 15->12 to fit charge timer */
				strncat(buffer, tprintf("%5.5s + Lock: %-12.12s %10.10s\n",
			 	  unparse_arc(sdb[x].blist.arcs[i]),
			 	  unparse_identity(a, sdb[x].blist.lock[i]),
			 	  unparse_distance(range)), sizeof(buffer) - 1);
			} else
				/* Mordak Aspace v1.0.0p1 - Change from 15->12 to fit charge timer */
				strncat(buffer, tprintf("%5.5s * Lock: %-12.12s %10.10s\n",
			 	  unparse_arc(sdb[x].blist.arcs[i]),
			 	  unparse_identity(a, sdb[x].blist.lock[i]),
			 	  unparse_distance(range)), sizeof(buffer) - 1);
		}
	}

 	return (buffer);
}

/* ------------------------------------------------------------------------ */

char *format_s_Missile (int x, int a)
{
	static char buffer[100 * MAX_MISSILE_TUBES];
	register int i;
	int c, c_arc;
	double range;

	strncpy(buffer, "", sizeof(buffer) - 1);

	for (i = 0 ; i < sdb[x].missile.tubes ; ++i) {

		strncat(buffer, tprintf("%s%2d %-13.13s%s %5.5s %-15.15s ",
		  ANSI_YELLOW, i + 1, unparse_missile(sdb[x].mlist.name[i]), ANSI_WHITE,
		  unparse_percent(sdb[x].mlist.damage[i]),
		  unparse_damage(sdb[x].mlist.damage[i])), sizeof(buffer) - 1);

		/* ADDED THE FOLLOWING TO DISPLAY CHARGE TIMER */	
		/* Mordak Aspace v1.0.0p1 */
		if (sdb[x].missile.out < sdb[x].mlist.cost[i])
			strncat(buffer, tprintf("NP "), sizeof(buffer) -1 );
		else
			if(!(sdb[x].mlist.load[i] + (missile_refresh / sdb[x].tech.firing) > time(NULL)))
				strncat(buffer, tprintf("RR "), sizeof(buffer) -1);
			else
				strncat(buffer, tprintf("%2.0f ",(missile_refresh / sdb[x].tech.firing)+(sdb[x].mlist.load[i] - time(NULL))) , sizeof(buffer) - 1); 
			/* END Aspace v1.0.0p1  */

		if (fabs(sdb[x].move.out) >= 1.0) {
			range = sdb[x].mlist.range[i] * PARSEC / 1000.0;
		} else
			range = sdb[x].mlist.range[i] * 10.0;
		if (!sdb[x].mlist.active[i]) {
/* Mordak Aspace v1.0.0p1 - changed from 10->7 to fit charge timer */
			strncat(buffer, tprintf("%5.5s   %7.7s/",
			  unparse_arc(sdb[x].mlist.arcs[i]),
		unparse_power(sdb[x].mlist.cost[i] / (double) sdb[x].mlist.recycle[i] * sdb[x].tech.firing)), sizeof(buffer) - 1);
			strncat(buffer, tprintf("%-10.10s %10.10s\n",
			  unparse_power(sdb[x].mlist.warhead[i]),
			  unparse_distance(range)), sizeof(buffer) - 1);
		} else if (!sdb[x].mlist.lock[i]) {
			strncat(buffer, tprintf("%5.5s   Unlocked              %10.10s\n",
			  unparse_arc(sdb[x].mlist.arcs[i]),
			  unparse_distance(range)), sizeof(buffer) - 1);
		} else {
			c = sdb2contact(x, sdb[x].mlist.lock[i]);
			c_arc = sdb2arc(x, sdb[x].mlist.lock[i]);
			if (c == SENSOR_FAIL) {
		 		strncat(buffer, tprintf("%5.5s   ERROR\n",
				  unparse_arc(sdb[x].mlist.arcs[i])), sizeof(buffer) - 1);
			} else {
				if ((sdb[x].structure.type > 1) && (sdb[sdb[x].mlist.lock[i]].move.out >= 1.0))
					range = sdb[x].mlist.range[i] * PARSEC / 1000.0;
				if (arc_check(c_arc, sdb[x].mlist.arcs[i]) == ARC_FAIL) {
					/* Mordak Aspace v1.0.0p1 - Changed 15->12 to fit charge timer */
					strncat(buffer, tprintf("%5.5s - Lock: %-12.12s %10.10s\n",
				 	  unparse_arc(sdb[x].mlist.arcs[i]),
				 	  unparse_identity(a, sdb[x].mlist.lock[i]),
				 	  unparse_distance(range)), sizeof(buffer) - 1);
			} else if (sdb[x].mlist.load[i] + ((double) sdb[x].mlist.recycle[i] / sdb[x].tech.firing) > time(NULL)) {
					/* Mordak Aspace v1.0.0p1 - Changed 15->12 to fit charge timer */
					strncat(buffer, tprintf("%5.5s + Lock: %-12.12s %10.10s\n",
				 	  unparse_arc(sdb[x].mlist.arcs[i]),
				 	  unparse_identity(a, sdb[x].mlist.lock[i]),
				 	  unparse_distance(range)), sizeof(buffer) - 1);
				} else
					/* Mordak Aspace v1.0.0p1 - Changed 15->12 to fit charge timer */
					strncat(buffer, tprintf("%5.5s * Lock: %-12.12s %10.10s\n",
				 	  unparse_arc(sdb[x].mlist.arcs[i]),
				 	  unparse_identity(a, sdb[x].mlist.lock[i]),
				 	  unparse_distance(range)), sizeof(buffer) - 1);
			}
		}
	}

 	return (buffer);
 }

/* ------------------------------------------------------------------------ */
