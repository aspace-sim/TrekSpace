/* space_read.c */

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
//#include <string>

#include "copyrite.h"
#include "ansi.h"
#include "externs.h"
#include "intrface.h"
#include "parse.h"
#include "match.h"
#include "confmagic.h"
#include "space.h"
#include "attrib.h"
#include "log.h"
#include "dbdefs.h"
#include "flags.h"
//FOR MYSQL SUPPORT
#include <mysql.h>
#include <errmsg.h>
//END MYSQL SUPPORT
#include "function.h"
#include "command.h"


//std::string mystring = "Hello";
//std::string strName = field->name;

void process_components _((MYSQL *conn, MYSQL_RES *res_set,dbref executor,int x));

/* ------------------------------------------------------------------------ */
//#define SpaceObj(x) (IS(x, TYPE_THING, THING_SPACE_OBJECT)) THIS IS THE OLD FLAG SYSTEM
#define SpaceObj(x)       (IS(x, TYPE_THING, "SPACE_OBJECT"))


int do_space_db_read (dbref ship, dbref executor)
{
	ATTR *a;
	array_t array;
	int arg_count;
	register int i, x;
	int result;
	static char buffer[10];
	dbref object;
	//Start Mysql Component support
	//static MYSQL *mysql_struct= NULL;		
	MYSQL *conn;
		//, *result;
	MYSQL_RES *res_set;
	//MYSQL_FIELD *field;
	//char *query;
	//End Mysql Component 




/* SDB */

	x = 0;
	for (i = MIN_SPACE_OBJECTS ; i <= max_space_objects ; ++i)
		if (sdb[i].object == ship) {
			x = i;
			break;
		}
	if (!GoodSDB(x)) {
		result = get_empty_sdb();
		if (result == VACANCY_FAIL) {
			do_log(LT_SPACE, executor, ship, "READ: unable find empty SDB slot.");
			return 0;
		} else
			x = result;
	}
	snprintf(buffer, sizeof(buffer), "%d", x);
	result = atr_add(ship, SDB_ATTR_NAME, buffer, GOD, AF_MDARK|AF_WIZARD|AF_NOPROG);
	if (result == -1) {
		do_log(LT_SPACE, executor, ship, "READ: unable to write SDB attribute.");
		return 0;
	} else if (max_space_objects < x)
		max_space_objects = x;

/* OBJECT */

	if (!SpaceObj(ship) || !GoodObject(ship)) {
		do_log(LT_SPACE, executor, ship, "READ: unable to validate SPACE_OBJECT.");
		return 0;
	} else
		sdb[x].object = ship;

/* SPACE */

	a = atr_get(ship, SPACE_ATTR_NAME); 
	if (a == NULL) {
		do_log(LT_SPACE, executor, ship, "READ: unable to read SPACE attribute.");
		return 0;
	} else {
		sdb[x].space = parse_integer(uncompress(a->value));
	}

/* ALLOCATE */

	a = atr_get(ship, ALLOCATE_ATTR_NAME);
	if (a == NULL) {
		do_log(LT_SPACE, executor, ship, "READ: unable to read ALLOCATE attribute.");
		return 0;
	}
	result = crack_list(a->value, ALLOCATE_DATA_NUMBER, ALLOCATE_DATA_NUMBER, &arg_count, array);
	if (result != SUCCESS) {
		do_log(LT_SPACE, executor, ship, "READ: unable to crack ALLOCATE attribute.");
		return 0;
	}
	result += convert_long(array[0], 0, MAX_LONG, 0, &sdb[x].alloc.version);
	result += convert_double(array[1], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].alloc.helm);
	result += convert_double(array[2], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].alloc.tactical);
	result += convert_double(array[3], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].alloc.operations);
	result += convert_double(array[4], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].alloc.movement);
	result += convert_double(array[5], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].alloc.shields);
	result += convert_double(array[6], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].alloc.shield[0]);
	result += convert_double(array[7], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].alloc.shield[1]);
	result += convert_double(array[8], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].alloc.shield[2]);
	result += convert_double(array[9], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].alloc.shield[3]);
	result += convert_double(array[10], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].alloc.cloak);
	result += convert_double(array[11], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].alloc.beams);
	result += convert_double(array[12], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].alloc.missiles);
	result += convert_double(array[13], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].alloc.sensors);
	result += convert_double(array[14], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].alloc.ecm);
	result += convert_double(array[15], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].alloc.eccm);
	result += convert_double(array[16], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].alloc.transporters);
	result += convert_double(array[17], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].alloc.tractors);
	result += convert_double(array[18], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].alloc.miscellaneous);
	if (result != SUCCESS) {
		do_log(LT_SPACE, executor, ship, "READ: unable to convert ALLOCATE attribute.");
		return 0;
	}

/* BEAM */

	a = atr_get(ship, BEAM_ATTR_NAME);
	if (a == NULL) {
		do_log(LT_SPACE, executor, ship, "READ: unable to read BEAM attribute.");
		return 0;
	}
	result = crack_list(a->value, BEAM_DATA_NUMBER, BEAM_DATA_NUMBER, &arg_count, array);
	if (result != SUCCESS) {
		do_log(LT_SPACE, executor, ship, "READ: unable to crack BEAM attribute.");
		return 0;
	}
	result += convert_double(array[0], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].beam.in);
	result += convert_double(array[1], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].beam.out);
	result += convert_double(array[2], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].beam.freq);
	result += convert_long(array[3], 0, MAX_LONG, 0, &sdb[x].beam.exist);
	result += convert_long(array[4], 0, MAX_LONG, 0, &sdb[x].beam.banks);
	result += convert_long(array[5], 0, MAX_LONG, 0, &sdb[x].beam.lasthit);
	if (result != SUCCESS) {
		do_log(LT_SPACE, executor, ship, "READ: unable to convert BEAM attribute.");
		return 0;
	} else {
		if (sdb[x].beam.banks > MAX_BEAM_BANKS)
			sdb[x].beam.banks = MAX_BEAM_BANKS;
		if (sdb[x].beam.banks <= 0) {
			sdb[x].beam.banks = 0;
			sdb[x].beam.exist = 0;
		}
	}

/* BEAM_ACTIVE */

	if (sdb[x].beam.exist) {
		a = atr_get(ship, BEAM_ACTIVE_ATTR_NAME);
		if (a == NULL) {
			do_log(LT_SPACE, executor, ship, "READ: unable to read BEAM_ACTIVE attribute.");
			return 0;
		}
		result = crack_list(a->value, sdb[x].beam.banks, sdb[x].beam.banks, &arg_count, array);
		if (result != SUCCESS) {
			do_log(LT_SPACE, executor, ship, "READ: unable to crack BEAM_ACTIVE attribute.");
			return 0;
		}
		for (i = 0 ; i < sdb[x].beam.banks ; ++i) {
			result += convert_long(array[i], 0, MAX_LONG, 0, &sdb[x].blist.active[i]);
		}
		if (result != SUCCESS) {
			do_log(LT_SPACE, executor, ship, "READ: unable to convert BEAM_ACTIVE attribute.");
			return 0;
		}
	}

/* BEAM_NAME */

	if (sdb[x].beam.exist) {
		a = atr_get(ship, BEAM_NAME_ATTR_NAME);
		if (a == NULL) {
			do_log(LT_SPACE, executor, ship, "READ: unable to read BEAM_NAME attribute.");
			return 0;
		}
		result = crack_list(a->value, sdb[x].beam.banks, sdb[x].beam.banks, &arg_count, array);
		if (result != SUCCESS) {
			do_log(LT_SPACE, executor, ship, "READ: unable to crack BEAM_NAME attribute.");
			return 0;
		}
		for (i = 0 ; i < sdb[x].beam.banks ; ++i) {
			result += convert_long(array[i], 0, MAX_LONG, 0, &sdb[x].blist.name[i]);
		}
		if (result != SUCCESS) {
			do_log(LT_SPACE, executor, ship, "READ: unable to convert BEAM_NAME attribute.");
			return 0;
		}
	}

/* BEAM_DAMAGE */

	if (sdb[x].beam.exist) {
		a = atr_get(ship, BEAM_DAMAGE_ATTR_NAME);
		if (a == NULL) {
			do_log(LT_SPACE, executor, ship, "READ: unable to read BEAM_DAMAGE attribute.");
			return 0;
		}
		result = crack_list(a->value, sdb[x].beam.banks, sdb[x].beam.banks, &arg_count, array);
		if (result != SUCCESS) {
			do_log(LT_SPACE, executor, ship, "READ: unable to crack BEAM_DAMAGE attribute.");
			return 0;
		}
		for (i = 0 ; i < sdb[x].beam.banks ; ++i) {
			result += convert_double(array[i], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].blist.damage[i]);
		}
		if (result != SUCCESS) {
			do_log(LT_SPACE, executor, ship, "READ: unable to convert BEAM_DAMAGE attribute.");
			return 0;
		}
	}

/* BEAM_BONUS */

	if (sdb[x].beam.exist) {
		a = atr_get(ship, BEAM_BONUS_ATTR_NAME);
		if (a == NULL) {
			do_log(LT_SPACE, executor, ship, "READ: unable to read BEAM_BONUS attribute.");
			return 0;
		}
		result = crack_list(a->value, sdb[x].beam.banks, sdb[x].beam.banks, &arg_count, array);
		if (result != SUCCESS) {
			do_log(LT_SPACE, executor, ship, "READ: unable to crack BEAM_BONUS attribute.");
			return 0;
		}
		for (i = 0 ; i < sdb[x].beam.banks ; ++i) {
			result += convert_long(array[i], 0, MAX_LONG, 0, &sdb[x].blist.bonus[i]);
		}
		if (result != SUCCESS) {
			do_log(LT_SPACE, executor, ship, "READ: unable to convert BEAM_BONUS attribute.");
			return 0;
		}
	}

/* BEAM_COST */

	if (sdb[x].beam.exist) {
		a = atr_get(ship, BEAM_COST_ATTR_NAME);
		if (a == NULL) {
			do_log(LT_SPACE, executor, ship, "READ: unable to read BEAM_COST attribute.");
			return 0;
		}
		result = crack_list(a->value, sdb[x].beam.banks, sdb[x].beam.banks, &arg_count, array);
		if (result != SUCCESS) {
			do_log(LT_SPACE, executor, ship, "READ: unable to crack BEAM_COST attribute.");
			return 0;
		}
		for (i = 0 ; i < sdb[x].beam.banks ; ++i) {
			result += convert_long(array[i], 0, MAX_LONG, 0, &sdb[x].blist.cost[i]);
		}
		if (result != SUCCESS) {
			do_log(LT_SPACE, executor, ship, "READ: unable to convert BEAM_COST attribute.");
			return 0;
		}
	}

/* BEAM_RANGE */

	if (sdb[x].beam.exist) {
		a = atr_get(ship, BEAM_RANGE_ATTR_NAME);
		if (a == NULL) {
			do_log(LT_SPACE, executor, ship, "READ: unable to read BEAM_RANGE attribute.");
			return 0;
		}
		result = crack_list(a->value, sdb[x].beam.banks, sdb[x].beam.banks, &arg_count, array);
		if (result != SUCCESS) {
			do_log(LT_SPACE, executor, ship, "READ: unable to crack BEAM_RANGE attribute.");
			return 0;
		}
		for (i = 0 ; i < sdb[x].beam.banks ; ++i) {
			result += convert_long(array[i], 0, MAX_LONG, 0, &sdb[x].blist.range[i]);
		}
		if (result != SUCCESS) {
			do_log(LT_SPACE, executor, ship, "READ: unable to convert BEAM_RANGE attribute.");
			return 0;
		}
	}

/* BEAM_ARCS */

	if (sdb[x].beam.exist) {
		a = atr_get(ship, BEAM_ARCS_ATTR_NAME);
		if (a == NULL) {
			do_log(LT_SPACE, executor, ship, "READ: unable to read BEAM_ARCS attribute.");
			return 0;
		}
		result = crack_list(a->value, sdb[x].beam.banks, sdb[x].beam.banks, &arg_count, array);
		if (result != SUCCESS) {
			do_log(LT_SPACE, executor, ship, "READ: unable to crack BEAM_ARCS attribute.");
			return 0;
		}
		for (i = 0 ; i < sdb[x].beam.banks ; ++i) {
			result += convert_long(array[i], 0, MAX_LONG, 0, &sdb[x].blist.arcs[i]);
		}
		if (result != SUCCESS) {
			do_log(LT_SPACE, executor, ship, "READ: unable to convert BEAM_ARCS attribute.");
			return 0;
		}
	}

/* BEAM_LOCK */

	if (sdb[x].beam.exist) {
		a = atr_get(ship, BEAM_LOCK_ATTR_NAME);
		if (a == NULL) {
			do_log(LT_SPACE, executor, ship, "READ: unable to read BEAM_LOCK attribute.");
			return 0;
		}
		result = crack_list(a->value, sdb[x].beam.banks, sdb[x].beam.banks, &arg_count, array);
		if (result != SUCCESS) {
			do_log(LT_SPACE, executor, ship, "READ: unable to crack BEAM_LOCK attribute.");
			return 0;
		}
		for (i = 0 ; i < sdb[x].beam.banks ; ++i) {
			result += convert_long(array[i], 0, MAX_LONG, 0, &sdb[x].blist.lock[i]);
		}
		if (result != SUCCESS) {
			do_log(LT_SPACE, executor, ship, "READ: unable to convert BEAM_LOCK attribute.");
			return 0;
		}
	}

/* BEAM_LOAD */

	if (sdb[x].beam.exist) {
		a = atr_get(ship, BEAM_LOAD_ATTR_NAME);
		if (a == NULL) {
			do_log(LT_SPACE, executor, ship, "READ: unable to read BEAM_LOAD attribute.");
			return 0;
		}
		result = crack_list(a->value, sdb[x].beam.banks, sdb[x].beam.banks, &arg_count, array);
		if (result != SUCCESS) {
			do_log(LT_SPACE, executor, ship, "READ: unable to crack BEAM_LOAD attribute.");
			return 0;
		}
		for (i = 0 ; i < sdb[x].beam.banks ; ++i) {
			result += convert_long(array[i], 0, MAX_LONG, 0, &sdb[x].blist.load[i]);
		}
		if (result != SUCCESS) {
			do_log(LT_SPACE, executor, ship, "READ: unable to convert BEAM_LOAD attribute.");
			return 0;
		}
	}

/* BEAM_RECYCLE */ 

if (sdb[x].beam.exist) { 
a = atr_get(ship, BEAM_RECYCLE_ATTR_NAME); 
if (a == NULL) { 
do_log(LT_SPACE, executor, ship, "READ: unable to read BEAM_RECYCLE attribute."); 
return 0; 
} 
result = crack_list(a->value, sdb[x].beam.banks, sdb[x].beam.banks, &arg_count, array); 
if (result != SUCCESS) { 
do_log(LT_SPACE, executor, ship, "READ: unable to crack BEAM_RECYCLE attribute."); 
return 0; 
} 
for (i = 0 ; i < sdb[x].beam.banks ; ++i) { 
result += convert_long(array[i], 0, MAX_LONG, 0, &sdb[x].blist.recycle[i]); 
} 
if (result != SUCCESS) { 
do_log(LT_SPACE, executor, ship, "READ: unable to convert BEAM_RECYCLE attribute."); 
return 0; 
} 
} 



/* MISSILE */

	a = atr_get(ship, MISSILE_ATTR_NAME);
	if (a == NULL) {
		do_log(LT_SPACE, executor, ship, "READ: unable to read MISSILE attribute.");
		return 0;
	}
	result = crack_list(a->value, MISSILE_DATA_NUMBER, MISSILE_DATA_NUMBER, &arg_count, array);
	if (result != SUCCESS) {
		do_log(LT_SPACE, executor, ship, "READ: unable to crack MISSILE attribute.");
		return 0;
	}
	result += convert_double(array[0], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].missile.in);
	result += convert_double(array[1], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].missile.out);
	result += convert_double(array[2], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].missile.freq);
	result += convert_long(array[3], 0, MAX_LONG, 0, &sdb[x].missile.exist);
	result += convert_long(array[4], 0, MAX_LONG, 0, &sdb[x].missile.tubes);
	result += convert_long(array[5], 0, MAX_LONG, 0, &sdb[x].missile.lasthit);	
	if (result != SUCCESS) {
		do_log(LT_SPACE, executor, ship, "READ: unable to convert MISSILE attribute.");
		return 0;
	} else {
		if (sdb[x].missile.tubes > MAX_MISSILE_TUBES)
			sdb[x].missile.tubes = MAX_MISSILE_TUBES;
		if (sdb[x].missile.tubes <= 0) {
			sdb[x].missile.tubes = 0;
			sdb[x].missile.exist = 0;
		}
	}


/* MISSILE_ACTIVE */

	if (sdb[x].missile.exist) {
		a = atr_get(ship, MISSILE_ACTIVE_ATTR_NAME);
		if (a == NULL) {
			do_log(LT_SPACE, executor, ship, "READ: unable to read MISSILE_ACTIVE attribute.");
			return 0;
		}
		result = crack_list(a->value, sdb[x].missile.tubes, sdb[x].missile.tubes, &arg_count, array);
		if (result != SUCCESS) {
			do_log(LT_SPACE, executor, ship, "READ: unable to crack MISSILE_ACTIVE attribute.");
			return 0;
		}
		for (i = 0 ; i < sdb[x].missile.tubes ; ++i) {
			result += convert_long(array[i], 0, MAX_LONG, 0, &sdb[x].mlist.active[i]);
		}
		if (result != SUCCESS) {
			do_log(LT_SPACE, executor, ship, "READ: unable to convert MISSILE_ACTIVE attribute.");
			return 0;
		}
	}

/* MISSILE_NAME */

	if (sdb[x].missile.exist) {
		a = atr_get(ship, MISSILE_NAME_ATTR_NAME);
		if (a == NULL) {
			do_log(LT_SPACE, executor, ship, "READ: unable to read MISSILE_NAME attribute.");
			return 0;
		}
		result = crack_list(a->value, sdb[x].missile.tubes, sdb[x].missile.tubes, &arg_count, array);
		if (result != SUCCESS) {
			do_log(LT_SPACE, executor, ship, "READ: unable to crack MISSILE_NAME attribute.");
			return 0;
		}
		for (i = 0 ; i < sdb[x].missile.tubes ; ++i) {
			result += convert_long(array[i], 0, MAX_LONG, 0, &sdb[x].mlist.name[i]);
		}
		if (result != SUCCESS) {
			do_log(LT_SPACE, executor, ship, "READ: unable to convert MISSILE_NAME attribute.");
			return 0;
		}
	}

/* MISSILE_DAMAGE */

	if (sdb[x].missile.exist) {
		a = atr_get(ship, MISSILE_DAMAGE_ATTR_NAME);
		if (a == NULL) {
			do_log(LT_SPACE, executor, ship, "READ: unable to read MISSILE_DAMAGE attribute.");
			return 0;
		}
		result = crack_list(a->value, sdb[x].missile.tubes, sdb[x].missile.tubes, &arg_count, array);
		if (result != SUCCESS) {
			do_log(LT_SPACE, executor, ship, "READ: unable to crack MISSILE_DAMAGE attribute.");
			return 0;
		}
		for (i = 0 ; i < sdb[x].missile.tubes ; ++i) {
			result += convert_double(array[i], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].mlist.damage[i]);
		}
		if (result != SUCCESS) {
			do_log(LT_SPACE, executor, ship, "READ: unable to convert MISSILE_DAMAGE attribute.");
			return 0;
		}
	}

/* MISSILE_WARHEAD */

	if (sdb[x].missile.exist) {
		a = atr_get(ship, MISSILE_WARHEAD_ATTR_NAME);
		if (a == NULL) {
			do_log(LT_SPACE, executor, ship, "READ: unable to read MISSILE_WARHEAD attribute.");
			return 0;
		}
		result = crack_list(a->value, sdb[x].missile.tubes, sdb[x].missile.tubes, &arg_count, array);
		if (result != SUCCESS) {
			do_log(LT_SPACE, executor, ship, "READ: unable to crack MISSILE_WARHEAD attribute.");
			return 0;
		}
		for (i = 0 ; i < sdb[x].missile.tubes ; ++i) {
			result += convert_long(array[i], 0, MAX_LONG, 0, &sdb[x].mlist.warhead[i]);
		}
		if (result != SUCCESS) {
			do_log(LT_SPACE, executor, ship, "READ: unable to convert MISSILE_WARHEAD attribute.");
			return 0;
		}
	}

/* MISSILE_COST */

	if (sdb[x].missile.exist) {
		a = atr_get(ship, MISSILE_COST_ATTR_NAME);
		if (a == NULL) {
			do_log(LT_SPACE, executor, ship, "READ: unable to read MISSILE_COST attribute.");
			return 0;
		}
		result = crack_list(a->value, sdb[x].missile.tubes, sdb[x].missile.tubes, &arg_count, array);
		if (result != SUCCESS) {
			do_log(LT_SPACE, executor, ship, "READ: unable to crack MISSILE_COST attribute.");
			return 0;
		}
		for (i = 0 ; i < sdb[x].missile.tubes ; ++i) {
			result += convert_long(array[i], 0, MAX_LONG, 0, &sdb[x].mlist.cost[i]);
		}
		if (result != SUCCESS) {
			do_log(LT_SPACE, executor, ship, "READ: unable to convert MISSILE_COST attribute.");
			return 0;
		}
	}

/* MISSILE_RANGE */

	if (sdb[x].missile.exist) {
		a = atr_get(ship, MISSILE_RANGE_ATTR_NAME);
		if (a == NULL) {
			do_log(LT_SPACE, executor, ship, "READ: unable to read MISSILE_RANGE attribute.");
			return 0;
		}
		result = crack_list(a->value, sdb[x].missile.tubes, sdb[x].missile.tubes, &arg_count, array);
		if (result != SUCCESS) {
			do_log(LT_SPACE, executor, ship, "READ: unable to crack MISSILE_RANGE attribute.");
			return 0;
		}
		for (i = 0 ; i < sdb[x].missile.tubes ; ++i) {
			result += convert_long(array[i], 0, MAX_LONG, 0, &sdb[x].mlist.range[i]);
		}
		if (result != SUCCESS) {
			do_log(LT_SPACE, executor, ship, "READ: unable to convert MISSILE_RANGE attribute.");
			return 0;
		}
	}

/* MISSILE_ARCS */

	if (sdb[x].missile.exist) {
		a = atr_get(ship, MISSILE_ARCS_ATTR_NAME);
		if (a == NULL) {
			do_log(LT_SPACE, executor, ship, "READ: unable to read MISSILE_ARCS attribute.");
			return 0;
		}
		result = crack_list(a->value, sdb[x].missile.tubes, sdb[x].missile.tubes, &arg_count, array);
		if (result != SUCCESS) {
			do_log(LT_SPACE, executor, ship, "READ: unable to crack MISSILE_ARCS attribute.");
			return 0;
		}
		for (i = 0 ; i < sdb[x].missile.tubes ; ++i) {
			result += convert_long(array[i], 0, MAX_LONG, 0, &sdb[x].mlist.arcs[i]);
		}
		if (result != SUCCESS) {
			do_log(LT_SPACE, executor, ship, "READ: unable to convert MISSILE_ARCS attribute.");
			return 0;
		}
	}

/* MISSILE_LOCK */

	if (sdb[x].missile.exist) {
		a = atr_get(ship, MISSILE_LOCK_ATTR_NAME);
		if (a == NULL) {
			do_log(LT_SPACE, executor, ship, "READ: unable to read MISSILE_LOCK attribute.");
			return 0;
		}
		result = crack_list(a->value, sdb[x].missile.tubes, sdb[x].missile.tubes, &arg_count, array);
		if (result != SUCCESS) {
			do_log(LT_SPACE, executor, ship, "READ: unable to crack MISSILE_LOCK attribute.");
			return 0;
		}
		for (i = 0 ; i < sdb[x].missile.tubes ; ++i) {
			result += convert_long(array[i], 0, MAX_LONG, 0, &sdb[x].mlist.lock[i]);
		}
		if (result != SUCCESS) {
			do_log(LT_SPACE, executor, ship, "READ: unable to convert MISSILE_LOCK attribute.");
			return 0;
		}
	}

/* MISSILE_LOAD */

	if (sdb[x].missile.exist) {
		a = atr_get(ship, MISSILE_LOAD_ATTR_NAME);
		if (a == NULL) {
			do_log(LT_SPACE, executor, ship, "READ: unable to read MISSILE_LOAD attribute.");
			return 0;
		}
		result = crack_list(a->value, sdb[x].missile.tubes, sdb[x].missile.tubes, &arg_count, array);
		if (result != SUCCESS) {
			do_log(LT_SPACE, executor, ship, "READ: unable to crack MISSILE_LOAD attribute.");
			return 0;
		}
		for (i = 0 ; i < sdb[x].missile.tubes ; ++i) {
			result += convert_long(array[i], 0, MAX_LONG, 0, &sdb[x].mlist.load[i]);
		}
		if (result != SUCCESS) {
			do_log(LT_SPACE, executor, ship, "READ: unable to convert MISSILE_LOAD attribute.");
			return 0;
		}
	}

/* MISSILE_RECYCLE */ 

if (sdb[x].missile.exist) { 
a = atr_get(ship, MISSILE_RECYCLE_ATTR_NAME); 
if (a == NULL) { 

do_log(LT_SPACE, executor, ship, "READ: unable to read MISSILE_RECYCLE attribute."); 
return 0; 
} 
result = crack_list(a->value, sdb[x].missile.tubes, sdb[x].missile.tubes, &arg_count, array); 
if (result != SUCCESS) { 
do_log(LT_SPACE, executor, ship, "READ: unable to crack MISSILE_RECYCLE attribute."); 
return 0; 
} 
for (i = 0 ; i < sdb[x].missile.tubes ; ++i) { 
result += convert_long(array[i], 0, MAX_LONG, 0, &sdb[x].mlist.recycle[i]); 
} 
if (result != SUCCESS) { 
do_log(LT_SPACE, executor, ship, "READ: unable to convert MISSILE_RECYCLE attribute."); 
return 0; 
} 
} 




/* ENGINE */

 	a = atr_get(ship, ENGINE_ATTR_NAME);
	if (a == NULL) {
		do_log(LT_SPACE, executor, ship, "READ: unable to read ENGINE attribute.");
		return 0;
	}
	result = crack_list(a->value, ENGINE_DATA_NUMBER, ENGINE_DATA_NUMBER, &arg_count, array);
	if (result != SUCCESS) {
		do_log(LT_SPACE, executor, ship, "READ: unable to crack ENGINE attribute.");
		return 0;
	}
	result += convert_long(array[0], 0, MAX_LONG, 0, &sdb[x].engine.version);
	result += convert_double(array[1], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].engine.warp_damage);
	result += convert_double(array[2], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].engine.warp_max);
	result += convert_long(array[3], 0, MAX_LONG, 0, &sdb[x].engine.warp_exist);
	result += convert_double(array[4], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].engine.impulse_damage);
	result += convert_double(array[5], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].engine.impulse_max);
	result += convert_long(array[6], 0, MAX_LONG, 0, &sdb[x].engine.impulse_exist);
	result += convert_double(array[7], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].engine.warp_cruise);
	result += convert_double(array[8], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].engine.impulse_cruise);
	result += convert_long(array[9], 0, MAX_LONG, 0, &sdb[x].engine.lasthit);	
	if (result != SUCCESS) {
		do_log(LT_SPACE, executor, ship, "READ: unable to convert ENGINE attribute.");
		return 0;
	}
	sdb[x].engine.version = 1;

/* STRUCTURE */

 	a = atr_get(ship, STRUCTURE_ATTR_NAME);
	if (a == NULL) {
		do_log(LT_SPACE, executor, ship, "READ: unable to read STRUCTURE attribute.");
		return 0;
	}
	result = crack_list(a->value, STRUCTURE_DATA_NUMBER, STRUCTURE_DATA_NUMBER, &arg_count, array);
	if (result != SUCCESS) {
		do_log(LT_SPACE, executor, ship, "READ: unable to crack STRUCTURE attribute.");
		return 0;
	}
	result += convert_long(array[0], 0, MAX_LONG, 0, &sdb[x].structure.type);
	result += convert_double(array[1], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].structure.displacement);
	result += convert_double(array[2], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].structure.cargo_hold);
	result += convert_double(array[3], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].structure.cargo_mass);
	result += convert_double(array[4], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].structure.superstructure);
	result += convert_long(array[5], 0, MAX_LONG, 0, &sdb[x].structure.max_structure);
	result += convert_long(array[6], 0, MAX_LONG, 0, &sdb[x].structure.has_landing_pad);
	result += convert_long(array[7], 0, MAX_LONG, 0, &sdb[x].structure.has_docking_bay);
	result += convert_long(array[8], 0, MAX_LONG, 0, &sdb[x].structure.can_land);
	result += convert_long(array[9], 0, MAX_LONG, 0, &sdb[x].structure.can_dock);
	result += convert_double(array[10], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].structure.repair);
	result += convert_long(array[11], 0, MAX_LONG, 0, &sdb[x].structure.max_repair);
	if (result != SUCCESS) {
		do_log(LT_SPACE, executor, ship, "READ: unable to convert STRUCTURE attribute.");
		return 0;
	}

/* POWER */

 	a = atr_get(ship, POWER_ATTR_NAME);
	if (a == NULL) {
		do_log(LT_SPACE, executor, ship, "READ: unable to read POWER attribute.");
		return 0;
	}
	result = crack_list(a->value, POWER_DATA_NUMBER, POWER_DATA_NUMBER, &arg_count, array);
	if (result != SUCCESS) {
		do_log(LT_SPACE, executor, ship, "READ: unable to crack POWER attribute.");
		return 0;
	}
	result += convert_long(array[0], 0, MAX_LONG, 0, &sdb[x].power.version);
	result += convert_double(array[1], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].power.main);
	result += convert_double(array[2], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].power.aux);
	result += convert_double(array[3], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].power.batt);
	result += convert_double(array[4], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].power.total);
	if (result != SUCCESS) {
		do_log(LT_SPACE, executor, ship, "READ: unable to convert POWER attribute.");
		return 0;
	}

/* SENSOR */

 	a = atr_get(ship, SENSOR_ATTR_NAME);
	if (a == NULL) {
		do_log(LT_SPACE, executor, ship, "READ: unable to read SENSOR attribute.");
		return 0;
	}
	result = crack_list(a->value, SENSOR_DATA_NUMBER, SENSOR_DATA_NUMBER, &arg_count, array);
	if (result != SUCCESS) {
		do_log(LT_SPACE, executor, ship, "READ: unable to crack SENSOR attribute.");
		return 0;
	}
	result += convert_long(array[0], 0, MAX_LONG, 0, &sdb[x].sensor.version);
	result += convert_double(array[1], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].sensor.lrs_damage);
	result += convert_long(array[2], 0, MAX_LONG, 0, &sdb[x].sensor.lrs_active);
	result += convert_long(array[3], 0, MAX_LONG, 0, &sdb[x].sensor.lrs_exist);
	result += convert_double(array[4], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].sensor.lrs_resolution);
	result += convert_double(array[5], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].sensor.lrs_signature);
	result += convert_double(array[6], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].sensor.srs_damage);
	result += convert_long(array[7], 0, MAX_LONG, 0, &sdb[x].sensor.srs_active);
	result += convert_long(array[8], 0, MAX_LONG, 0, &sdb[x].sensor.srs_exist);
	result += convert_double(array[9], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].sensor.srs_resolution);
	result += convert_double(array[10], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].sensor.srs_signature);
	result += convert_double(array[11], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].sensor.ew_damage);
	result += convert_long(array[12], 0, MAX_LONG, 0, &sdb[x].sensor.ew_active);
	result += convert_long(array[13], 0, MAX_LONG, 0, &sdb[x].sensor.ew_exist);
	result += convert_double(array[14], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].sensor.visibility);
	result += convert_long(array[15], 0, MAX_LONG, 0, &sdb[x].sensor.contacts);
	result += convert_long(array[16], 0, MAX_LONG, 0, &sdb[x].sensor.counter);
	result += convert_long(array[17], 0, MAX_LONG, 0, &sdb[x].sensor.lasthit);	
	if (result != SUCCESS) {
		do_log(LT_SPACE, executor, ship, "READ: unable to convert SENSOR attribute.");
		return 0;
	}
	sdb[x].sensor.contacts = 0;
	sdb[x].sensor.counter = 0;

/* SHIELD */

 	a = atr_get(ship, SHIELD_ATTR_NAME);
	if (a == NULL) {
		do_log(LT_SPACE, executor, ship, "READ: unable to read SHIELD attribute.");
		return 0;
	}
	result = crack_list(a->value, SHIELD_DATA_NUMBER, SHIELD_DATA_NUMBER, &arg_count, array);
	if (result != SUCCESS) {
		do_log(LT_SPACE, executor, ship, "READ: unable to crack SHIELD attribute.");
		return 0;
	}
	result += convert_double(array[0], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].shield.ratio);
	result += convert_long(array[1], 0, MAX_LONG, 0, &sdb[x].shield.maximum);
	result += convert_double(array[2], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].shield.freq);
	result += convert_long(array[3], 0, MAX_LONG, 0, &sdb[x].shield.exist);
	result += convert_long(array[4], 0, MAX_LONG, 0, &sdb[x].shield.active[0]);
	result += convert_long(array[5], 0, MAX_LONG, 0, &sdb[x].shield.active[1]);
	result += convert_long(array[6], 0, MAX_LONG, 0, &sdb[x].shield.active[2]);
	result += convert_long(array[7], 0, MAX_LONG, 0, &sdb[x].shield.active[3]);
	result += convert_double(array[8], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].shield.damage[0]);
	result += convert_double(array[9], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].shield.damage[1]);
	result += convert_double(array[10], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].shield.damage[2]);
	result += convert_double(array[11], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].shield.damage[3]);
	result += convert_long(array[12], 0, MAX_LONG, 0, &sdb[x].shield.lasthit);
	if (result != SUCCESS) {
		do_log(LT_SPACE, executor, ship, "READ: unable to convert SHIELD attribute.");
		return 0;
	}

/* TECHNOLOGY */

 	a = atr_get(ship, TECHNOLOGY_ATTR_NAME);
	if (a == NULL) {
		do_log(LT_SPACE, executor, ship, "READ: unable to read TECHNOLOGY attribute.");
		return 0;
	}
	result = crack_list(a->value, TECHNOLOGY_DATA_NUMBER, TECHNOLOGY_DATA_NUMBER, &arg_count, array);
	if (result != SUCCESS) {
		do_log(LT_SPACE, executor, ship, "READ: unable to crack TECHNOLOGY attribute.");
		return 0;
	}
	result += convert_double(array[0], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].tech.firing);
	result += convert_double(array[1], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].tech.fuel);
	result += convert_double(array[2], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].tech.stealth);
	result += convert_double(array[3], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].tech.cloak);
	result += convert_double(array[4], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].tech.sensors);
	result += convert_double(array[5], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].tech.aux_max);
	result += convert_double(array[6], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].tech.main_max);
	result += convert_double(array[7], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].tech.armor);
	result += convert_double(array[8], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].tech.ly_range);
	if (result != SUCCESS) {
		do_log(LT_SPACE, executor, ship, "READ: unable to convert TECHNOLOGY attribute.");
		return 0;
	}

/* MOVEMENT */

 	a = atr_get(ship, MOVEMENT_ATTR_NAME);
	if (a == NULL) {
		do_log(LT_SPACE, executor, ship, "READ: unable to read MOVEMENT attribute.");
		return 0;
	}
	result = crack_list(a->value, MOVEMENT_DATA_NUMBER, MOVEMENT_DATA_NUMBER, &arg_count, array);
	if (result != SUCCESS) {
		do_log(LT_SPACE, executor, ship, "READ: unable to crack MOVEMENT attribute.");
		return 0;
	}
	result += convert_long(array[0], 0, MAX_LONG, 0, &sdb[x].move.time);
	result += convert_long(array[1], 0, MAX_LONG, 0, &sdb[x].move.dt);
	result += convert_double(array[2], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].move.in);
	result += convert_double(array[3], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].move.out);
	result += convert_double(array[4], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].move.ratio);
	result += convert_double(array[5], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].move.cochranes);
	result += convert_double(array[6], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].move.v);
	result += convert_long(array[7], 0, MAX_LONG, 0, &sdb[x].move.empire);
	result += convert_long(array[8], 0, MAX_LONG, 0, &sdb[x].move.quadrant);
	if (result != SUCCESS) {
		do_log(LT_SPACE, executor, ship, "READ: unable to convert MOVEMENT attribute.");
		return 0;
	}

/* CLOAK */

 	a = atr_get(ship, CLOAK_ATTR_NAME);
	if (a == NULL) {
		do_log(LT_SPACE, executor, ship, "READ: unable to read CLOAK attribute.");
		return 0;
	}
	result = crack_list(a->value, CLOAK_DATA_NUMBER, CLOAK_DATA_NUMBER, &arg_count, array);
	if (result != SUCCESS) {
		do_log(LT_SPACE, executor, ship, "READ: unable to crack CLOAK attribute.");
		return 0;
	}
	result += convert_long(array[0], 0, MAX_LONG, 0, &sdb[x].cloak.version);
	result += convert_long(array[1], 0, MAX_LONG, 0, &sdb[x].cloak.cost);
	result += convert_double(array[2], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].cloak.freq);
	result += convert_long(array[3], 0, MAX_LONG, 0, &sdb[x].cloak.exist);
	result += convert_long(array[4], 0, MAX_LONG, 0, &sdb[x].cloak.active);
	result += convert_double(array[5], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].cloak.damage);
	result += convert_long(array[6], 0, MAX_LONG, 0, &sdb[x].cloak.lasthit);
	if (result != SUCCESS) {
		do_log(LT_SPACE, executor, ship, "READ: unable to convert CLOAK attribute.");
		return 0;
	}

/* TRANS */

 	a = atr_get(ship, TRANS_ATTR_NAME);
	if (a == NULL) {
		do_log(LT_SPACE, executor, ship, "READ: unable to read TRANS attribute.");
		return 0;
	}
	result = crack_list(a->value, TRANS_DATA_NUMBER, TRANS_DATA_NUMBER, &arg_count, array);
	if (result != SUCCESS) {
		do_log(LT_SPACE, executor, ship, "READ: unable to crack TRANS attribute.");
		return 0;
	}
	result += convert_long(array[0], 0, MAX_LONG, 0, &sdb[x].trans.cost);
	result += convert_double(array[1], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].trans.freq);
	result += convert_long(array[2], 0, MAX_LONG, 0, &sdb[x].trans.exist);
	result += convert_long(array[3], 0, MAX_LONG, 0, &sdb[x].trans.active);
	result += convert_double(array[4], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].trans.damage);
	result += convert_long(array[5], 0, MAX_LONG, 0, &sdb[x].trans.d_lock);
	result += convert_long(array[6], 0, MAX_LONG, 0, &sdb[x].trans.s_lock);
	result += convert_long(array[7], 0, MAX_LONG, 0, &sdb[x].trans.lasthit);
	if (result != SUCCESS) {
		do_log(LT_SPACE, executor, ship, "READ: unable to convert TRANS attribute.");
		return 0;
	}

//************************START CUSTOM CODE ******************************

/* LIFE SUPPORT - ADDED BY BILL 4-9-2002 */
 
 	a = atr_get(ship, LIFESUPPORT_ATTR_NAME);
	if (a == NULL) {
		do_log(LT_SPACE, executor, ship, "READ: unable to read LIFESUPPORT attribute.");
		return 0;
	}
	result = crack_list(a->value, LIFESUPPORT_DATA_NUMBER, LIFESUPPORT_DATA_NUMBER, &arg_count, array);
	if (result != SUCCESS) {
		do_log(LT_SPACE, executor, ship, "READ: unable to crack LIFESUPPORT attribute.");
		return 0;
	}
	result += convert_double(array[0], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].lifesupport.damage);
	result += convert_long(array[1], 0, MAX_LONG, 0, &sdb[x].lifesupport.active);
	result += convert_long(array[2], 0, MAX_LONG, 0, &sdb[x].lifesupport.time);
	result += convert_long(array[3], 0, MAX_LONG, 0, &sdb[x].lifesupport.lasthit);
	if (result != SUCCESS) {
		do_log(LT_SPACE, executor, ship, "READ: unable to convert LifeSupport attribute.");
		return 0;
	}	


/* Confidence Factor */
 
 	a = atr_get(ship, CF_ATTR_NAME);
	if (a == NULL) {
		do_log(LT_SPACE, executor, ship, "READ: unable to read CF attribute.");
		return 0;
	}
	result = crack_list(a->value, CF_DATA_NUMBER, CF_DATA_NUMBER, &arg_count, array);
	if (result != SUCCESS) {
		do_log(LT_SPACE, executor, ship, "READ: unable to crack CF attribute.");
		return 0;
	}
	result += convert_long(array[0], 0, MAX_LONG, 0, &sdb[x].cf.factor);
	if (result != SUCCESS) {
		do_log(LT_SPACE, executor, ship, "READ: unable to convert CF attribute.");
		return 0;
	}	

/* Breakdown Factor */
 
 	a = atr_get(ship, BREAKDOWN_ATTR_NAME);
	if (a == NULL) {
		do_log(LT_SPACE, executor, ship, "READ: unable to read BREAKDOWN attribute.");
		return 0;
	}
	result = crack_list(a->value, BREAKDOWN_DATA_NUMBER, BREAKDOWN_DATA_NUMBER, &arg_count, array);
	if (result != SUCCESS) {
		do_log(LT_SPACE, executor, ship, "READ: unable to crack CF attribute.");
		return 0;
	}
	result += convert_double(array[0], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].breakdown.factor);
	if (result != SUCCESS) {
		do_log(LT_SPACE, executor, ship, "READ: unable to convert BREAKDOWN attribute.");
		return 0;
	}	

/* IFF Factor */
 
 	a = atr_get(ship, IFF_ATTR_NAME);
	if (a == NULL) {
		do_log(LT_SPACE, executor, ship, "READ: unable to read IFF attribute.");
		return 0;
	}
	result = crack_list(a->value, IFF_DATA_NUMBER, IFF_DATA_NUMBER, &arg_count, array);
	if (result != SUCCESS) {
		do_log(LT_SPACE, executor, ship, "READ: unable to crack CF attribute.");
		return 0;
	}
	result += convert_double(array[0], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].iff.freq);
	if (result != SUCCESS) {
		do_log(LT_SPACE, executor, ship, "READ: unable to convert IFF attribute.");
		return 0;
	}	


	
//************************END CUSTOM CODE ******************************
	
/* TRACT */

 	a = atr_get(ship, TRACT_ATTR_NAME);
	if (a == NULL) {
		do_log(LT_SPACE, executor, ship, "READ: unable to read TRACT attribute.");
		return 0;
	}
	result = crack_list(a->value, TRACT_DATA_NUMBER, TRACT_DATA_NUMBER, &arg_count, array);
	if (result != SUCCESS) {
		do_log(LT_SPACE, executor, ship, "READ: unable to crack TRACT attribute.");
		return 0;
	}
	result += convert_long(array[0], 0, MAX_LONG, 0, &sdb[x].tract.cost);
	result += convert_double(array[1], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].tract.freq);
	result += convert_long(array[2], 0, MAX_LONG, 0, &sdb[x].tract.exist);
	result += convert_long(array[3], 0, MAX_LONG, 0, &sdb[x].tract.active);
	result += convert_double(array[4], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].tract.damage);
	result += convert_long(array[5], 0, MAX_LONG, 0, &sdb[x].tract.lock);
	result += convert_long(array[6], 0, MAX_LONG, 0, &sdb[x].tract.lasthit);
	if (result != SUCCESS) {
		do_log(LT_SPACE, executor, ship, "READ: unable to convert TRACT attribute.");
		return 0;
	}

/* COORDS */

 	a = atr_get(ship, COORDS_ATTR_NAME);
	if (a == NULL) {
		do_log(LT_SPACE, executor, ship, "READ: unable to read COORDS attribute.");
		return 0;
	}
	result = crack_list(a->value, COORDS_DATA_NUMBER, COORDS_DATA_NUMBER, &arg_count, array);
	if (result != SUCCESS) {
		do_log(LT_SPACE, executor, ship, "READ: unable to crack COORDS attribute.");
		return 0;
	}
	result += convert_double(array[0], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].coords.x);
	result += convert_double(array[1], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].coords.y);
	result += convert_double(array[2], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].coords.z);
	result += convert_double(array[3], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].coords.xo);
	result += convert_double(array[4], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].coords.yo);
	result += convert_double(array[5], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].coords.zo);
	result += convert_double(array[6], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].coords.xd);
	result += convert_double(array[7], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].coords.yd);
	result += convert_double(array[8], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].coords.zd);
	if (result != SUCCESS) {
		do_log(LT_SPACE, executor, ship, "READ: unable to convert COORDS attribute.");
		return 0;
	}

/* COURSE */

 	a = atr_get(ship, COURSE_ATTR_NAME);
	if (a == NULL) {
		do_log(LT_SPACE, executor, ship, "READ: unable to read COURSE attribute.");
		return 0;
	}
	result = crack_list(a->value, COURSE_DATA_NUMBER, COURSE_DATA_NUMBER, &arg_count, array);
	if (result != SUCCESS) {
		do_log(LT_SPACE, executor, ship, "READ: unable to crack COURSE attribute.");
		return 0;
	}
	result += convert_long(array[0], 0, MAX_LONG, 0, &sdb[x].course.version);
	result += convert_double(array[1], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].course.yaw_in);
	result += convert_double(array[2], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].course.yaw_out);
	result += convert_double(array[3], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].course.pitch_in);
	result += convert_double(array[4], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].course.pitch_out);
	result += convert_double(array[5], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].course.roll_in);
	result += convert_double(array[6], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].course.roll_out);
	result += convert_double(array[7], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].course.d[0][0]);
	result += convert_double(array[8], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].course.d[0][1]);
	result += convert_double(array[9], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].course.d[0][2]);
	result += convert_double(array[10], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].course.d[1][0]);
	result += convert_double(array[11], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].course.d[1][1]);
	result += convert_double(array[12], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].course.d[1][2]);
	result += convert_double(array[13], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].course.d[2][0]);
	result += convert_double(array[14], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].course.d[2][1]);
	result += convert_double(array[15], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].course.d[2][2]);
	result += convert_double(array[16], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].course.rate);
	if (result != SUCCESS) {
		do_log(LT_SPACE, executor, ship, "READ: unable to convert COURSE attribute.");
		return 0;
	}

/* MAIN */

 	a = atr_get(ship, MAIN_ATTR_NAME);
	if (a == NULL) {
		do_log(LT_SPACE, executor, ship, "READ: unable to read MAIN attribute.");
		return 0;
	}
	result = crack_list(a->value, MAIN_DATA_NUMBER, MAIN_DATA_NUMBER, &arg_count, array);
	if (result != SUCCESS) {
		do_log(LT_SPACE, executor, ship, "READ: unable to crack MAIN attribute.");
		return 0;
	}
	result += convert_double(array[0], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].main.in);
	result += convert_double(array[1], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].main.out);
	result += convert_double(array[2], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].main.damage);
	result += convert_double(array[3], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].main.gw);
	result += convert_long(array[4], 0, MAX_LONG, 0, &sdb[x].main.exist);
	result += convert_long(array[5], 0, MAX_LONG, 0, &sdb[x].main.lasthit);
	if (result != SUCCESS) {
		do_log(LT_SPACE, executor, ship, "READ: unable to convert MAIN attribute.");
		return 0;
	}

/* AUX */

 	a = atr_get(ship, AUX_ATTR_NAME);
	if (a == NULL) {
		do_log(LT_SPACE, executor, ship, "READ: unable to read AUX attribute.");
		return 0;
	}
	result = crack_list(a->value, AUX_DATA_NUMBER, AUX_DATA_NUMBER, &arg_count, array);
	if (result != SUCCESS) {
		do_log(LT_SPACE, executor, ship, "READ: unable to crack AUX attribute.");
		return 0;
	}
	result += convert_double(array[0], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].aux.in);
	result += convert_double(array[1], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].aux.out);
	result += convert_double(array[2], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].aux.damage);
	result += convert_double(array[3], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].aux.gw);
	result += convert_long(array[4], 0, MAX_LONG, 0, &sdb[x].aux.exist);
	result += convert_long(array[5], 0, MAX_LONG, 0, &sdb[x].aux.lasthit);
	if (result != SUCCESS) {
		do_log(LT_SPACE, executor, ship, "READ: unable to convert AUX attribute.");
		return 0;
	}

/* BATT */

 	a = atr_get(ship, BATT_ATTR_NAME);
	if (a == NULL) {
		do_log(LT_SPACE, executor, ship, "READ: unable to read BATT attribute.");
		return 0;
	}
	result = crack_list(a->value, BATT_DATA_NUMBER, BATT_DATA_NUMBER, &arg_count, array);
	if (result != SUCCESS) {
		do_log(LT_SPACE, executor, ship, "READ: unable to crack BATT attribute.");
		return 0;
	}
	result += convert_double(array[0], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].batt.in);
	result += convert_double(array[1], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].batt.out);
	result += convert_double(array[2], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].batt.damage);
	result += convert_double(array[3], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].batt.gw);
	result += convert_long(array[4], 0, MAX_LONG, 0, &sdb[x].batt.exist);
	result += convert_long(array[5], 0, MAX_LONG, 0, &sdb[x].batt.lasthit);
	if (result != SUCCESS) {
		do_log(LT_SPACE, executor, ship, "READ: unable to convert BATT attribute.");
		return 0;
	}

/* FUEL */

 	a = atr_get(ship, FUEL_ATTR_NAME);
	if (a == NULL) {
		do_log(LT_SPACE, executor, ship, "READ: unable to read FUEL attribute.");
		return 0;
	}
	result = crack_list(a->value, FUEL_DATA_NUMBER, FUEL_DATA_NUMBER, &arg_count, array);
	if (result != SUCCESS) {
		do_log(LT_SPACE, executor, ship, "READ: unable to crack FUEL attribute.");
		return 0;
	}
	result += convert_double(array[0], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].fuel.antimatter);
	result += convert_double(array[1], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].fuel.deuterium);
	result += convert_double(array[2], -MAX_DOUBLE, MAX_DOUBLE, 0.0, &sdb[x].fuel.reserves);
	if (result != SUCCESS) {
		do_log(LT_SPACE, executor, ship, "READ: unable to convert FUEL attribute.");
		return 0;
	}

/* STATUS */

 	a = atr_get(ship, STATUS_ATTR_NAME);
	if (a == NULL) {
		do_log(LT_SPACE, executor, ship, "READ: unable to read STATUS attribute.");
		return 0;
	}
	result = crack_list(a->value, STATUS_DATA_NUMBER, STATUS_DATA_NUMBER, &arg_count, array);
	if (result != SUCCESS) {
		do_log(LT_SPACE, executor, ship, "READ: unable to crack STATUS attribute.");
		return 0;
	}
	result += convert_long(array[0], 0, MAX_LONG, 0, &sdb[x].status.active);
	result += convert_long(array[1], 0, MAX_LONG, 0, &sdb[x].status.docked);
	result += convert_long(array[2], 0, MAX_LONG, 0, &sdb[x].status.landed);
	result += convert_long(array[3], 0, MAX_LONG, 0, &sdb[x].status.connected);
	result += convert_long(array[4], 0, MAX_LONG, 0, &sdb[x].status.crippled);
	result += convert_long(array[5], 0, MAX_LONG, 0, &sdb[x].status.tractoring);
	result += convert_long(array[6], 0, MAX_LONG, 0, &sdb[x].status.tractored);
	result += convert_long(array[7], 0, MAX_LONG, 0, &sdb[x].status.open_landing);
	result += convert_long(array[8], 0, MAX_LONG, 0, &sdb[x].status.open_docking);
	result += convert_long(array[9], 0, MAX_LONG, 0, &sdb[x].status.link);
	if (result != SUCCESS) {
		do_log(LT_SPACE, executor, ship, "READ: unable to convert STATUS attribute.");
		return 0;
	}
	sdb[x].status.time = sdb[x].move.time;

/* Lets	do the component check */

	//IF the company is not null lets make a record in the database
	//query=tprintf("select * from components where shipdbref='%s'",unparse_dbref(ship));
	
	//notify(executor, tprintf("Query %s",query));


	//If we're this far we're sure the companyname is not null

	conn = mysql_init (NULL);
	mysql_real_connect(conn,"localhost.localdomain","root","pw","db",0,NULL,0);

   if (!conn) {
     notify(executor, "No SQL database connection.");
   }
   if (conn) {
	notify(executor, "SQL database connection.");
	//mysql_real_query(mysql, query, strlen(query));
	
	//Lets run the query
	if (mysql_query (conn, tprintf("select * from components where shipdbref='%s'",unparse_dbref(ship))) != 0)
			notify(executor, "Query Failed");
	else {
		res_set = mysql_store_result (conn); //Generate Result Set
		if (res_set==NULL) 
				notify(executor, "Mysql_store_result() Failed");
		else {
			//Process result set, then deallocate it 
			process_components (conn, res_set, executor,x);
			mysql_free_result(res_set);
		}


	}



	
	//qres = mysql_store_result(mysql);
	//field = mysql_fetch_field(qres);
   }	
	

	
	
	mysql_close(conn);
	/* End Component Check */



/* DEBUGGING */

	result = debug_space(x);
	if (result == 0) {
		do_log(LT_SPACE, executor, ship, "READ: Bugs found and corrected.");
	}

	return 1;
}

/* -------------------------------------------------------------------- */
//Process Components
void process_components (MYSQL *conn, MYSQL_RES *res_set,dbref executor,int x) {
MYSQL_ROW	row;
MYSQL_FIELD *field;
unsigned int	i;
char *system1;//,*value,*newsystem;
int newsystem, active;
float value;
	
	while((row=mysql_fetch_row (res_set)) != NULL)
	{

		//Lets reset the variables
		value=atof("0");
		newsystem="0";
		active="0";
		system1=NULL;


		//This loops us through all available Records
		mysql_field_seek (res_set,0);

		for (i = 0; i < mysql_num_fields (res_set); i++)
		{
			
			field=mysql_fetch_field (res_set);
			//notify(executor, tprintf("Field: %i  -- %d",strcmp(field->name,"system"),strlen(field->name)));

			//What system is it that is being added
			if (strcmp(field->name,"system") ==0) 
			{
				//MAINS
				if (strcmp(row[i],"main") ==0) {
					system1="main";
				}
				if (strcmp(row[i],"cloak") ==0) {
					system1="cloak";
				}
				if (strcmp(row[i],"sensors") ==0) {
					system1="sensors";
				}
				if (strcmp(row[i],"ecm") ==0) {
					system1="ecm";
				}
				if (strcmp(row[i],"eccm") ==0) {
					system1="eccm";
				}
				if (strcmp(row[i],"transporter") ==0) {
					system1="transporter";
				}
				if (strcmp(row[i],"tractor") ==0) {
					system1="tractor";
				}
				if (strcmp(row[i],"technology") ==0) {
					system1="technology";
				}
				if (strcmp(row[i],"structure") ==0) {
					system1="structure";
				}
				if (strcmp(row[i],"lifesupport") ==0) {
					system1="lifesupport";
				}
				if (strcmp(row[i],"transponder") ==0) {
					system1="transponder";
				}	
				/*else {
					system1="EMPTY";
				}*/
			}

			
			//Lets get the value of the Mod now
			if (strcmp(field->name,"modvalue") ==0) {
				value=atof(row[i]);
			}
			//Lets see if this is a new system
			if (strcmp(field->name,"newsystem") ==0) {
				newsystem=atoi(row[i]);
			}
			//Lets see if this is a active system
			if (strcmp(field->name,"active") ==0) {
				active=atoi(row[i]);
			}
			
			//notify(executor, tprintf("Field: %i  -- %d",strcmp(field->name,"system"),strlen(field->name)));
			//notify(executor, tprintf("system1 %s - NewSystem %i",system1,newsystem));
			//Lets not go through here if the system is inactive
			if (active == 1) {
					//notify(executor, tprintf("system1 %s - NewSystem %i",system1,newsystem));
				if (system1 == "main") {
					//notify(executor, tprintf("Before value %f, main.gw %f",value,sdb[x].main.gw));
					sdb[x].main.in=0.999;
					sdb[x].main.in=1.00;
					sdb[x].main.gw=sdb[x].main.gw + value;
					//do_console_notify(x, console_engineering, 0, 0,ansi_alert(tprintf("Main Power Reset to 0.00")));  //Let everyone know something broke	
					//notify(executor, tprintf("After value %f, main.gw %f",value,sdb[x].main.gw));
				}
				if (system1 == "cloak" && newsystem==1) {
				//sdb[x].main.gw=sdb[x].main.gw + atof(row[i]);
					sdb[x].cloak.exist=1;
					sdb[x].cloak.cost=value;
					sdb[x].cloak.damage = 1;
				} 
				if (system1 == "sensors") {
				//sdb[x].main.gw=sdb[x].main.gw + atof(row[i]);
					sdb[x].tech.sensors=value;
				}
				if (system1 == "ecm") {
				//sdb[x].main.gw=sdb[x].main.gw + atof(row[i]);
				}
				if (system1 == "eccm") {
				//sdb[x].main.gw=sdb[x].main.gw + atof(row[i]);
				}
				if (system1 == "transporter" && newsystem==1) {
				//sdb[x].main.gw=sdb[x].main.gw + atof(row[i]);
					sdb[x].trans.exist=1;
					sdb[x].trans.damage=1;
				}
				if (system1 == "tractor" && newsystem==1) {
				//sdb[x].main.gw=sdb[x].main.gw + atof(row[i]);
					sdb[x].tract.exist=1;
					sdb[x].tract.damage=1;
				}
				if (system1 == "technology") {
				//sdb[x].main.gw=sdb[x].main.gw + atof(row[i]);
				}
				if (system1 == "structure") {
				//sdb[x].main.gw=sdb[x].main.gw + atof(row[i]);
					sdb[x].structure.superstructure=sdb[x].structure.superstructure + value;
				}
				if (system1 == "lifesupport") {
				//sdb[x].main.gw=sdb[x].main.gw + atof(row[i]);
					sdb[x].lifesupport.damage=sdb[x].lifesupport.damage + value;
				}
				if (system1 == "transponder") {
				//sdb[x].main.gw=sdb[x].main.gw + atof(row[i]);
					sdb[x].iff.freq=value;
				}
			}
				//notify(executor, tprintf("%s",row[i]!=NULL ? row[i]: "NULL"));				
		}
				notify(executor, tprintf("\n"));
	}
	if (mysql_errno (conn) !=0)
			notify(executor, "mysql_Fetch_row() Failed");
	else
			notify(executor, tprintf("%lu rows returned\n",(unsigned long) mysql_num_rows (res_set)));
	
 }
/* -------------------------------------------------------------------- */
