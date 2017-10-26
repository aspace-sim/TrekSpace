/* space_variables.c */

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

/* ------------------------------------------------------------------------ */

struct space_database_t sdb[MAX_SPACE_OBJECTS + 1];
struct comms_database_t cdb[MAX_COMMS_OBJECTS + 1];
double cochrane = COCHRANE;
int n;
int max_space_objects = MIN_SPACE_OBJECTS;
int max_comms_objects = MIN_COMMS_OBJECTS;
int beam_refresh = DEF_BEAM_REFRESH; 
int missile_refresh = DEF_MISSILE_REFRESH;
dbref console_security = CONSOLE_SECURITY;
dbref console_helm = CONSOLE_HELM;
dbref console_engineering = CONSOLE_ENGINEERING;
dbref console_operation = CONSOLE_OPERATION;
dbref console_science = CONSOLE_SCIENCE;
dbref console_damage = CONSOLE_DAMAGE;
dbref console_communication = CONSOLE_COMMUNICATION;
dbref console_tactical = CONSOLE_TACTICAL;
dbref console_transporter = CONSOLE_TRANSPORTER;
dbref console_monitor = CONSOLE_MONITOR;
dbref console_fighter = CONSOLE_FIGHTER;

/* ------------------------------------------------------------------------ */

const char *shield_name[] = {
    "Forward shield",
    "Starboard shield",
    "Aft shield",
    "Port shield"
};

const char *cloak_name[] = {
    "Other",
    "Cloak"
};

const char *type_name[] = {
    "None",
    "Ship",
    "Base",
    "Planet",
    "Anomaly",
    "Star",
    "Nebula",
	"Warp Core",
	"Moon",
	"Probe",
	"Asteriod",
};

const char *beam_name[] = {
"FL-1",
"FL-2",
"FL-3",
"FL-4",
"FL-5",
"FL-6",
"FH-1",
"FH-2",
"FH-3",
"FH-4",
"FH-5",
"FH-6",
"FH-7",
"FH-8",
"FH-9",
"FH-10",
"FH-11",
"FH-12",
"FH-13",
"KB-1",
"KB-2",
"KB-3",
"KB-4",
"KB-5",
"KB-6",
"KB-7",
"KB-8",
"KB-9",
"KB-10",
"KB-11",
"KB-12",
"KB-13",
"KB-14",
"RB-1",
"RB-2",
"RB-2a",
"RB-3",
"RB-3a",
"RB-4",
"RB-5",
"RB-6",
"RB-7",
"RB-7a",
"RB-8",
"RB-9",
"RB-10",
"RB-11",
"OD-1",
"OD-2",
"OD-3",
"OD-4",
"OD-5",
"GBL-1",
"GBL-2",
"GBL-3",
"GBL-4",
"GBL-5",
"GBL-6",
"GBL-7",
"GBL-8",
"UnKnown",
};

const char *missile_name[] = { 
"FAC-1",
"FAC-2",
"FAC-3",
"FP-1",
"FP-2",
"FP-3",
"FP-4",
"FP-5",
"FP-6",
"FP-7",
"KP-1",
"KP-2",
"KP-3",
"KP-4",
"KP-5",
"KP-6",
"RPL-1",
"RPL-2",
"RPL-3",
"RP-1",
"RP-2",
"RP-3",
"GP-1",
"GP-2",
"GP-3",
"GP-4",
"UnKnown",
};

const char *quadrant_name[] = {
    "Alpha",
    "Beta",
    "Delta",
    "Gamma" };

/* Edit by bill 4/9/02 - Added Life Support */
const char *system_name[] = {
	"Superstructure",
	"Fusion Reactor",
	"Batteries",
	"Beam Weapon",
	"Cloaking Device",
	"EW Systems",
	"Impulse Drive",
	"LR Sensors",
	"M/A Reactor",
	"Missile Weapon",
	"Shield",
	"SR Sensors",
	"Tractor Beams",
	"Transporters",
	"Warp Drive",
	"Life Support"
};

double repair_mult[] = {
    32.0, 4.0, 2.0, 1.0, 8.0, 8.0, 4.0, 8.0, 8.0, 2.0, 1.0, 8.0, 1.0, 8.0, 4.0,4.0
};

const char *damage_name[] = {
	"No Damage",
	"Patchable Damage",
	"Minor Damage",
	"Light Damage",
	"Moderate Damage",
	"Heavy Damage",
	"Severe Damage",
	"Inoperative",
	"Destroyed"
};

const char *empire_name[] = {
    "Federation",
    "(Unclaimed)"
};

double empire_center[MAX_EMPIRE_NAME][11] = {

		85, 292829098129.153, 24559459667346.7, 8639846941.6399,
		30000.0000000000,               0.0,               0.0,             0.0
};

/* ------------------------------------------------------------------------ */
