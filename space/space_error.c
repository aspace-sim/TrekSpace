/* space_error.c */

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
#include "dbdefs.h"
#include "flags.h"

/* ------------------------------------------------------------------------ */
//#define SpaceObj(x) (IS(x, TYPE_THING, THING_SPACE_OBJECT)) THIS IS THE OLD FLAG SYSTEM
#define SpaceObj(x)       (IS(x, TYPE_THING, "SPACE_OBJECT"))




int error_on_console (dbref enactor)
{
	if (!GoodSDB(n)) {
		notify(enactor, ansi_red("SDB number invalid."));
/*	} else if (!SpaceObj(sdb[n].object) || !GoodObject(sdb[n].object)) {
		notify(enactor, ansi_red("Space object invalid."));
	} else if (sdb[n].structure.type == 0) {
		notify(enactor, ansi_red("Space object not loaded."));
*/	} else if (sdb[n].status.crippled == 2) {
		notify(enactor, ansi_red("Space object destroyed."));
	} else if (!sdb[n].status.active) {
		notify(enactor, ansi_red(tprintf("%s systems are inactive.",
		 Name(sdb[n].object))));
	} else if (sdb[n].status.crippled) {
		notify(enactor, ansi_red("Controls are inoperative."));
	} else {
		sdb[n].status.time = sdb[n].move.time;
		return 0;
	}
	return 1;
}

/* ------------------------------------------------------------------------ */

int error_on_contact (int n1, int n2, dbref enactor)
{
	if (n2 == SENSOR_FAIL) {
		notify(enactor, ansi_red("That is not a valid sensor contact."));
	} else if (!GoodSDB(n2)) {
		notify(enactor, ansi_red("That is not a valid sensor contact."));
	} else if (!SpaceObj(sdb[n2].object) || !GoodObject(sdb[n2].object)) {
		notify(enactor, ansi_red("That is not a valid sensor contact."));
		do_log (LT_SPACE, enactor, n2, "BUG:Sensor contact has bad flags");
	} else if (sdb[n2].structure.type == 0) {
		notify(enactor, ansi_red("That is not a valid sensor contact."));
		do_log (LT_SPACE, enactor, n2, "BUG:Sensor contact has bad TYPE");
	} else if (sdb[n2].space != sdb[n1].space) {
		notify(enactor, ansi_red("That is not a valid sensor contact."));
		do_log (LT_SPACE, enactor, n2, "BUG:Sensor contact has bad SPACE");
	} else {
		return 0;
	}
	return 1;
}

/* ------------------------------------------------------------------------ */
