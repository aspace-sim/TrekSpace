/* space_write.c */

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
#include "attrib.h"
#include "log.h"
#include "dbdefs.h"
#include "flags.h"

//FOR MYSQL SUPPORT
#include <mysql.h>
#include <errmsg.h>
//END MYSQL SUPPORT

void write_process_components _((MYSQL *conn, MYSQL_RES *res_set,dbref executor,int x));


/* ------------------------------------------------------------------------ */
//#define SpaceObj(x) (IS(x, TYPE_THING, THING_SPACE_OBJECT)) THIS IS THE OLD FLAG SYSTEM
#define SpaceObj(x)       (IS(x, TYPE_THING, "SPACE_OBJECT"))


int do_space_db_write (dbref ship, dbref executor)
{
	register int i;
	int x, result, lasthit1,weaponhit;
	static char buffer[BUFFER_LEN];
	ATTR *a;
	MYSQL *conn;
		//, *result;
	MYSQL_RES *res_set;

/* SDB */

	x = 0;
	for (i = MIN_SPACE_OBJECTS ; i <= max_space_objects ; ++i)
		if (sdb[i].object == ship) {
			x = i;
			break;
		}
	if (!GoodSDB(x)) {
		a = atr_get(ship, SDB_ATTR_NAME);
		if (a == NULL) {
			do_log(LT_SPACE, executor, ship, "WRITE: unable to read SDB attribute.");
			return 0;
		}
		result = parse_integer(uncompress(a->value));
		if (!GoodSDB(result)) {
			do_log(LT_SPACE, executor, ship, "WRITE: unable to validate SDB.");
			return 0;
		} else if (sdb[result].object != ship) {
			do_log(LT_SPACE, executor, ship, "WRITE: unable to verify SDB.");
			return 0;
		} else if (sdb[result].structure.type == 0) {
			do_log(LT_SPACE, executor, ship, "WRITE: unable to verify STRUCTURE.");
			return 0;
		} else
			x = result;
	}
	snprintf(buffer, sizeof(buffer), "%d", x);
	result = atr_add(ship, SDB_ATTR_NAME, buffer, GOD, (AF_MDARK + AF_WIZARD + AF_NOPROG));
	if (result != 1) {
		do_log(LT_SPACE, executor, ship, "WRITE: unable to write SDB attribute.");
		return 0;
	} else if (max_space_objects < x)
		max_space_objects = x;

/* OBJECT */

	if (!SpaceObj(ship) || !GoodObject(ship)) {
		do_log(LT_SPACE, executor, ship, "WRITE: unable to validate SPACE_OBJECT.");
		return 0;
	}

/* DEBUGGING */

	n = x;
	result = debug_space(x);
	if (result == 0) {
		do_log(LT_SPACE, executor, ship, "WRITE: Bugs found and corrected.");
	}


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
			write_process_components (conn, res_set, executor,x);
			mysql_free_result(res_set);
		}


	}



	
	//qres = mysql_store_result(mysql);
	//field = mysql_fetch_field(qres);
   }	
	

	
	
	mysql_close(conn);
	/* End Component Check */




/* LOCATION */

	strncpy(buffer, unparse_integer(sdb[x].location), sizeof(buffer) - 1);
	result = atr_add(ship, LOCATION_ATTR_NAME, buffer, GOD, (AF_MDARK + AF_WIZARD + AF_NOPROG));
	if (result != 1) {
		do_log(LT_SPACE, executor, ship, "WRITE: unable to write LOCATION attribute.");
		return 0;
	}

/* SPACE */

	strncpy(buffer, unparse_integer(sdb[x].space), sizeof(buffer) - 1);
	result = atr_add(ship, SPACE_ATTR_NAME, buffer, GOD, (AF_MDARK + AF_WIZARD + AF_NOPROG));
	if (result != 1) {
		do_log(LT_SPACE, executor, ship, "WRITE: unable to write SPACE attribute.");
		return 0;
	}

/* ALLOCATE */

	snprintf(buffer, sizeof(buffer), "%d %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f",
		sdb[x].alloc.version,
		sdb[x].alloc.helm,
		sdb[x].alloc.tactical,
		sdb[x].alloc.operations,
		sdb[x].alloc.movement,
		sdb[x].alloc.shields,
		sdb[x].alloc.shield[0],
		sdb[x].alloc.shield[1],
		sdb[x].alloc.shield[2],
		sdb[x].alloc.shield[3],
		sdb[x].alloc.cloak,
		sdb[x].alloc.beams,
		sdb[x].alloc.missiles,
		sdb[x].alloc.sensors,
		sdb[x].alloc.ecm,
		sdb[x].alloc.eccm,
		sdb[x].alloc.transporters,
		sdb[x].alloc.tractors,
		sdb[x].alloc.miscellaneous);
	result = atr_add(ship, ALLOCATE_ATTR_NAME, buffer, GOD, (AF_MDARK + AF_WIZARD + AF_NOPROG));
	if (result != 1) {
		do_log(LT_SPACE, executor, ship, "WRITE: unable to write ALLOCATE attribute.");
		return 0;
	}

/* BEAM */

	if (sdb[x].beam.exist) {
		for (i = 0; i < sdb[x].beam.banks; ++i) {
			
			if (sdb[x].blist.damage[i] <=.99) {
			weaponhit=1;
			break; }

		} }
					
			if (weaponhit ==1 ) {
			lasthit1=999; }
			else {
			lasthit1=sdb[x].beam.lasthit; 	}
			
	snprintf(buffer, sizeof(buffer), "%f %f %f %d %d %i",
		sdb[x].beam.in,
		sdb[x].beam.out,
		sdb[x].beam.freq,
		sdb[x].beam.exist,
		sdb[x].beam.banks,
		lasthit1);
	result = atr_add(ship, BEAM_ATTR_NAME, buffer, GOD, (AF_MDARK + AF_WIZARD + AF_NOPROG));
	if (result != 1) {
		do_log(LT_SPACE, executor, ship, "WRITE: unable to write BEAM attribute.");
		return 0;
	}

/* BEAM_ACTIVE */

	strncpy(buffer, "", sizeof(buffer) - 1);
	if (sdb[x].beam.exist) {
		for (i = 0; i < sdb[x].beam.banks; ++i)
			strncat(buffer, tprintf("%d ", sdb[x].blist.active[i]), sizeof(buffer) - 1);
			result = atr_add(ship, BEAM_ACTIVE_ATTR_NAME, buffer, GOD, (AF_MDARK + AF_WIZARD + AF_NOPROG));
	} else {
		atr_clr(ship, (char *) BEAM_ACTIVE_ATTR_NAME, GOD);
		result = 1;
	}
	if (result != 1) {
		do_log(LT_SPACE, executor, ship, "WRITE: unable to write BEAM_ACTIVE attribute.");
		return 0;
	}

/* BEAM_NAME */

	strncpy(buffer, "", sizeof(buffer) - 1);
	if (sdb[x].beam.exist) {
		for (i = 0; i < sdb[x].beam.banks; ++i)
			strncat(buffer, tprintf("%d ", sdb[x].blist.name[i]), sizeof(buffer) - 1);
			result = atr_add(ship, BEAM_NAME_ATTR_NAME, buffer, GOD, (AF_MDARK + AF_WIZARD + AF_NOPROG));
	} else {
		atr_clr(ship, (char *) BEAM_NAME_ATTR_NAME, GOD);
		result = 1;
	}
	if (result != 1) {
		do_log(LT_SPACE, executor, ship, "WRITE: unable to write BEAM_NAME attribute.");
		return 0;
	}

/* BEAM_DAMAGE */

	strncpy(buffer, "", sizeof(buffer) - 1);
	if (sdb[x].beam.exist) {
		for (i = 0; i < sdb[x].beam.banks; ++i)
			strncat(buffer, tprintf("%f ", sdb[x].blist.damage[i]), sizeof(buffer) - 1);
			result = atr_add(ship, BEAM_DAMAGE_ATTR_NAME, buffer, GOD, (AF_MDARK + AF_WIZARD + AF_NOPROG));
	} else {
		atr_clr(ship, (char *) BEAM_DAMAGE_ATTR_NAME, GOD);
		result = 1;
	}
	if (result != 1) {
		do_log(LT_SPACE, executor, ship, "WRITE: unable to write BEAM_DAMAGE attribute.");
		return 0;
	}

/* BEAM_BONUS */

	strncpy(buffer, "", sizeof(buffer) - 1);
	if (sdb[x].beam.exist) {
		for (i = 0; i < sdb[x].beam.banks; ++i)
			strncat(buffer, tprintf("%d ", sdb[x].blist.bonus[i]), sizeof(buffer) - 1);
			result = atr_add(ship, BEAM_BONUS_ATTR_NAME, buffer, GOD, (AF_MDARK + AF_WIZARD + AF_NOPROG));
	} else {
		atr_clr(ship, (char *) BEAM_BONUS_ATTR_NAME, GOD);
		result = 1;
	}
	if (result != 1) {
		do_log(LT_SPACE, executor, ship, "WRITE: unable to write BEAM_BONUS attribute.");
		return 0;
	}

/* BEAM_COST */

	strncpy(buffer, "", sizeof(buffer) - 1);
	if (sdb[x].beam.exist) {
		for (i = 0; i < sdb[x].beam.banks; ++i)
			strncat(buffer, tprintf("%d ", sdb[x].blist.cost[i]), sizeof(buffer) - 1);
			result = atr_add(ship, BEAM_COST_ATTR_NAME, buffer, GOD, (AF_MDARK + AF_WIZARD + AF_NOPROG));
	} else {
		atr_clr(ship, (char *) BEAM_COST_ATTR_NAME, GOD);
		result = 1;
	}
	if (result != 1) {
		do_log(LT_SPACE, executor, ship, "WRITE: unable to write BEAM_COST attribute.");
		return 0;
	}

/* BEAM_RANGE */

	strncpy(buffer, "", sizeof(buffer) - 1);
	if (sdb[x].beam.exist) {
		for (i = 0; i < sdb[x].beam.banks; ++i)
			strncat(buffer, tprintf("%d ", sdb[x].blist.range[i]), sizeof(buffer) - 1);
			result = atr_add(ship, BEAM_RANGE_ATTR_NAME, buffer, GOD, (AF_MDARK + AF_WIZARD + AF_NOPROG));
	} else {
		atr_clr(ship, (char *) BEAM_RANGE_ATTR_NAME, GOD);
		result = 1;
	}
	if (result != 1) {
		do_log(LT_SPACE, executor, ship, "WRITE: unable to write BEAM_RANGE attribute.");
		return 0;
	}

/* BEAM_ARCS */

	strncpy(buffer, "", sizeof(buffer) - 1);
	if (sdb[x].beam.exist) {
		for (i = 0; i < sdb[x].beam.banks; ++i)
			strncat(buffer, tprintf("%d ", sdb[x].blist.arcs[i]), sizeof(buffer) - 1);
			result = atr_add(ship, BEAM_ARCS_ATTR_NAME, buffer, GOD, (AF_MDARK + AF_WIZARD + AF_NOPROG));
	} else {
		atr_clr(ship, (char *) BEAM_ARCS_ATTR_NAME, GOD);
		result = 1;
	}
	if (result != 1) {
		do_log(LT_SPACE, executor, ship, "WRITE: unable to write BEAM_ARCS attribute.");
		return 0;
	}

/* BEAM_LOCK */

	strncpy(buffer, "", sizeof(buffer) - 1);
	if (sdb[x].beam.exist) {
		for (i = 0; i < sdb[x].beam.banks; ++i)
			strncat(buffer, tprintf("%d ", sdb[x].blist.lock[i]), sizeof(buffer) - 1);
			result = atr_add(ship, BEAM_LOCK_ATTR_NAME, buffer, GOD, (AF_MDARK + AF_WIZARD + AF_NOPROG));
	} else {
		atr_clr(ship, (char *) BEAM_LOCK_ATTR_NAME, GOD);
		result = 1;
	}
	if (result != 1) {
		do_log(LT_SPACE, executor, ship, "WRITE: unable to write BEAM_LOCK attribute.");
		return 0;
	}

/* BEAM_LOAD */

	strncpy(buffer, "", sizeof(buffer) - 1);
	if (sdb[x].beam.exist) {
		for (i = 0; i < sdb[x].beam.banks; ++i)
			strncat(buffer, tprintf("%d ", sdb[x].blist.load[i]), sizeof(buffer) - 1);
			result = atr_add(ship, BEAM_LOAD_ATTR_NAME, buffer, GOD, (AF_MDARK + AF_WIZARD + AF_NOPROG));
	} else {
		atr_clr(ship, (char *) BEAM_LOAD_ATTR_NAME, GOD);
		result = 1;
	}
	if (result != 1) {
		do_log(LT_SPACE, executor, ship, "WRITE: unable to write BEAM_LOAD attribute.");
		return 0;
	}

/* BEAM_RECYCLE */ 

strncpy(buffer, "", sizeof(buffer) - 1); 
if (sdb[x].beam.exist) { 
for (i = 0; i < sdb[x].beam.banks; ++i) 
strncat(buffer, tprintf("%d ", sdb[x].blist.recycle[i]), sizeof(buffer) - 1); 
result = atr_add(ship, BEAM_RECYCLE_ATTR_NAME, buffer, GOD, (AF_MDARK + AF_WIZARD + AF_NOPROG)); 
} else { 
atr_clr(ship, (char *) BEAM_RECYCLE_ATTR_NAME, GOD); 
result = 1; 
} 
if (result != 1) { 
do_log(LT_SPACE, executor, ship, "WRITE: unable to write BEAM_RECYCLE attribute."); 
return 0; 
}


/* MISSILE */

	snprintf(buffer, sizeof(buffer), "%f %f %f %d %d %i",
		sdb[x].missile.in,
		sdb[x].missile.out,
		sdb[x].missile.freq,
		sdb[x].missile.exist,
		sdb[x].missile.tubes,
		sdb[x].missile.lasthit);
	result = atr_add(ship, MISSILE_ATTR_NAME, buffer, GOD, (AF_MDARK + AF_WIZARD + AF_NOPROG));
	if (result != 1) {
		do_log(LT_SPACE, executor, ship, "WRITE: unable to write MISSILE attribute.");
		return 0;
	}

/* MISSILE_ACTIVE */

	strncpy(buffer, "", sizeof(buffer) - 1);
	if (sdb[x].missile.exist) {
		for (i = 0; i < sdb[x].missile.tubes; ++i)
			strncat(buffer, tprintf("%d ", sdb[x].mlist.active[i]), sizeof(buffer) - 1);
			result = atr_add(ship, MISSILE_ACTIVE_ATTR_NAME, buffer, GOD, (AF_MDARK + AF_WIZARD + AF_NOPROG));
	} else {
		atr_clr(ship, (char *) MISSILE_ACTIVE_ATTR_NAME, GOD);
		result = 1;
	}
	if (result != 1) {
		do_log(LT_SPACE, executor, ship, "WRITE: unable to write MISSILE_ACTIVE attribute.");
		return 0;
	}

/* MISSILE_NAME */

	strncpy(buffer, "", sizeof(buffer) - 1);
	if (sdb[x].missile.exist) {
		for (i = 0; i < sdb[x].missile.tubes; ++i)
			strncat(buffer, tprintf("%d ", sdb[x].mlist.name[i]), sizeof(buffer) - 1);
			result = atr_add(ship, MISSILE_NAME_ATTR_NAME, buffer, GOD, (AF_MDARK + AF_WIZARD + AF_NOPROG));
	} else {
		atr_clr(ship, (char *) MISSILE_NAME_ATTR_NAME, GOD);
		result = 1;
	}
	if (result != 1) {
		do_log(LT_SPACE, executor, ship, "WRITE: unable to write MISSILE_NAME attribute.");
		return 0;
	}

/* MISSILE_DAMAGE */

	strncpy(buffer, "", sizeof(buffer) - 1);
	if (sdb[x].missile.exist) {
		for (i = 0; i < sdb[x].missile.tubes; ++i)
			strncat(buffer, tprintf("%f ", sdb[x].mlist.damage[i]), sizeof(buffer) - 1);
			result = atr_add(ship, MISSILE_DAMAGE_ATTR_NAME, buffer, GOD, (AF_MDARK + AF_WIZARD + AF_NOPROG));
	} else {
		atr_clr(ship, (char *) MISSILE_DAMAGE_ATTR_NAME, GOD);
		result = 1;
	}
	if (result != 1) {
		do_log(LT_SPACE, executor, ship, "WRITE: unable to write MISSILE_DAMAGE attribute.");
		return 0;
	}

/* MISSILE_WARHEAD */

	strncpy(buffer, "", sizeof(buffer) - 1);
	if (sdb[x].missile.exist) {
		for (i = 0; i < sdb[x].missile.tubes; ++i)
			strncat(buffer, tprintf("%d ", sdb[x].mlist.warhead[i]), sizeof(buffer) - 1);
			result = atr_add(ship, MISSILE_WARHEAD_ATTR_NAME, buffer, GOD, (AF_MDARK + AF_WIZARD + AF_NOPROG));
	} else {
		atr_clr(ship, (char *) MISSILE_WARHEAD_ATTR_NAME, GOD);
		result = 1;
	}
	if (result != 1) {
		do_log(LT_SPACE, executor, ship, "WRITE: unable to write MISSILE_WARHEAD attribute.");
		return 0;
	}

/* MISSILE_COST */

	strncpy(buffer, "", sizeof(buffer) - 1);
	if (sdb[x].missile.exist) {
		for (i = 0; i < sdb[x].missile.tubes; ++i)
			strncat(buffer, tprintf("%d ", sdb[x].mlist.cost[i]), sizeof(buffer) - 1);
			result = atr_add(ship, MISSILE_COST_ATTR_NAME, buffer, GOD, (AF_MDARK + AF_WIZARD + AF_NOPROG));
	} else {
		atr_clr(ship, (char *) MISSILE_COST_ATTR_NAME, GOD);
		result = 1;
	}
	if (result != 1) {
		do_log(LT_SPACE, executor, ship, "WRITE: unable to write MISSILE_COST attribute.");
		return 0;
	}

/* MISSILE_RANGE */

	strncpy(buffer, "", sizeof(buffer) - 1);
	if (sdb[x].missile.exist) {
		for (i = 0; i < sdb[x].missile.tubes; ++i)
			strncat(buffer, tprintf("%d ", sdb[x].mlist.range[i]), sizeof(buffer) - 1);
			result = atr_add(ship, MISSILE_RANGE_ATTR_NAME, buffer, GOD, (AF_MDARK + AF_WIZARD + AF_NOPROG));
	} else {
		atr_clr(ship, (char *) MISSILE_RANGE_ATTR_NAME, GOD);
		result = 1;
	}
	if (result != 1) {
		do_log(LT_SPACE, executor, ship, "WRITE: unable to write MISSILE_RANGE attribute.");
		return 0;
	}

/* MISSILE_ARCS */

	strncpy(buffer, "", sizeof(buffer) - 1);
	if (sdb[x].missile.exist) {
		for (i = 0; i < sdb[x].missile.tubes; ++i)
			strncat(buffer, tprintf("%d ", sdb[x].mlist.arcs[i]), sizeof(buffer) - 1);
			result = atr_add(ship, MISSILE_ARCS_ATTR_NAME, buffer, GOD, (AF_MDARK + AF_WIZARD + AF_NOPROG));
	} else {
		atr_clr(ship, (char *) MISSILE_ARCS_ATTR_NAME, GOD);
		result = 1;
	}
	if (result != 1) {
		do_log(LT_SPACE, executor, ship, "WRITE: unable to write MISSILE_ARCS attribute.");
		return 0;
	}

/* MISSILE_LOCK */

	strncpy(buffer, "", sizeof(buffer) - 1);
	if (sdb[x].missile.exist) {
		for (i = 0; i < sdb[x].missile.tubes; ++i)
			strncat(buffer, tprintf("%d ", sdb[x].mlist.lock[i]), sizeof(buffer) - 1);
			result = atr_add(ship, MISSILE_LOCK_ATTR_NAME, buffer, GOD, (AF_MDARK + AF_WIZARD + AF_NOPROG));
	} else {
		atr_clr(ship, (char *) MISSILE_LOCK_ATTR_NAME, GOD);
		result = 1;
	}
	if (result != 1) {
		do_log(LT_SPACE, executor, ship, "WRITE: unable to write MISSILE_LOCK attribute.");
		return 0;
	}

/* MISSILE_LOAD */

	strncpy(buffer, "", sizeof(buffer) - 1);
	if (sdb[x].missile.exist) {
		for (i = 0; i < sdb[x].missile.tubes; ++i)
			strncat(buffer, tprintf("%d ", sdb[x].mlist.load[i]), sizeof(buffer) - 1);
			result = atr_add(ship, MISSILE_LOAD_ATTR_NAME, buffer, GOD, (AF_MDARK + AF_WIZARD + AF_NOPROG));
	} else {
		atr_clr(ship, (char *) MISSILE_LOAD_ATTR_NAME, GOD);
		result = 1;
	}
	if (result != 1) {
		do_log(LT_SPACE, executor, ship, "WRITE: unable to write MISSILE_LOAD attribute.");
		return 0;
	}

/* MISSILE_RECYCLE */ 

strncpy(buffer, "", sizeof(buffer) - 1); 
if (sdb[x].missile.exist) { 
for (i = 0; i < sdb[x].missile.tubes; ++i) 
strncat(buffer, tprintf("%d ", sdb[x].mlist.recycle[i]), sizeof(buffer) - 1); 
result = atr_add(ship, MISSILE_RECYCLE_ATTR_NAME, buffer, GOD, (AF_MDARK + AF_WIZARD + AF_NOPROG)); 
} else { 
atr_clr(ship, (char *) MISSILE_RECYCLE_ATTR_NAME, GOD); 
result = 1; 
} 
if (result != 1) { 
do_log(LT_SPACE, executor, ship, "WRITE: unable to write MISSILE_RECYCLE attribute."); 
return 0; 
} 


/* ENGINE */

 	if ((sdb[x].engine.warp_damage ==1 || sdb[x].engine.impulse_damage ==1) &&  sdb[x].engine.lasthit != 999) {
	lasthit1=999;
	}
	else {
	lasthit1=sdb[x].engine.lasthit;
	}

	snprintf(buffer, sizeof(buffer), "%d %f %f %d %f %f %d %f %f %i",
		sdb[x].engine.version,
		sdb[x].engine.warp_damage,
		sdb[x].engine.warp_max,
		sdb[x].engine.warp_exist,
		sdb[x].engine.impulse_damage,
		sdb[x].engine.impulse_max,
		sdb[x].engine.impulse_exist,
		sdb[x].engine.warp_cruise,
		sdb[x].engine.impulse_cruise,
		lasthit1);
	result = atr_add(ship, ENGINE_ATTR_NAME, buffer, GOD, (AF_MDARK + AF_WIZARD + AF_NOPROG));
	if (result != 1) {
		do_log(LT_SPACE, executor, ship, "WRITE: unable to write ENGINE attribute.");
		return 0;
	}

/* STRUCTURE */

	snprintf(buffer, sizeof(buffer), "%d %f %f %f %f %d %d %d %d %d %f %d",
		sdb[x].structure.type,
		sdb[x].structure.displacement,
		sdb[x].structure.cargo_hold,
		sdb[x].structure.cargo_mass,
		sdb[x].structure.superstructure,
		sdb[x].structure.max_structure,
		sdb[x].structure.has_landing_pad,
		sdb[x].structure.has_docking_bay,
		sdb[x].structure.can_land,
		sdb[x].structure.can_dock,
		sdb[x].structure.repair,
		sdb[x].structure.max_repair);
	result = atr_add(ship, STRUCTURE_ATTR_NAME, buffer, GOD, (AF_MDARK + AF_WIZARD + AF_NOPROG));
	if (result != 1) {
		do_log(LT_SPACE, executor, ship, "WRITE: unable to write STRUCTURE attribute.");
		return 0;
	}

/* POWER */

	snprintf(buffer, sizeof(buffer), "%d %f %f %f %f",
		sdb[x].power.version,
		sdb[x].power.main,
		sdb[x].power.aux,
		sdb[x].power.batt,
		sdb[x].power.total);
	result = atr_add(ship, POWER_ATTR_NAME, buffer, GOD, (AF_MDARK + AF_WIZARD + AF_NOPROG));
	if (result != 1) {
		do_log(LT_SPACE, executor, ship, "WRITE: unable to write POWER attribute.");
		return 0;
	}

/* SENSOR */

 	if ((sdb[x].sensor.lrs_damage ==1 || sdb[x].sensor.srs_damage ==1 || sdb[x].sensor.ew_damage ==1) &&  sdb[x].sensor.lasthit != 999) {
	lasthit1=999;
	}
	else {
	lasthit1=sdb[x].sensor.lasthit;
	}

	snprintf(buffer, sizeof(buffer), "%d %f %d %d %f %f %f %d %d %f %f %f %d %d %f %d %d %i",
		sdb[x].sensor.version,
		sdb[x].sensor.lrs_damage,
		sdb[x].sensor.lrs_active,
		sdb[x].sensor.lrs_exist,
		sdb[x].sensor.lrs_resolution,
		sdb[x].sensor.lrs_signature,
		sdb[x].sensor.srs_damage,
		sdb[x].sensor.srs_active,
		sdb[x].sensor.srs_exist,
		sdb[x].sensor.srs_resolution,
		sdb[x].sensor.srs_signature,
		sdb[x].sensor.ew_damage,
		sdb[x].sensor.ew_active,
		sdb[x].sensor.ew_exist,
		sdb[x].sensor.visibility,
		sdb[x].sensor.contacts,
		sdb[x].sensor.counter,
		lasthit1);
	result = atr_add(ship, SENSOR_ATTR_NAME, buffer, GOD, (AF_MDARK + AF_WIZARD + AF_NOPROG));
	if (result != 1) {
		do_log(LT_SPACE, executor, ship, "WRITE: unable to write SENSOR attribute.");
		return 0;
	}

/* SHIELD */

 	if ((sdb[x].shield.damage[0] ==1 || sdb[x].shield.damage[1] ==1 || sdb[x].shield.damage[2] ==1 || sdb[x].shield.damage[2] ==1 || sdb[x].shield.damage[3] ==1) &&  sdb[x].shield.lasthit != 999) {
	lasthit1=999;
	}
	else {
	lasthit1=sdb[x].shield.lasthit;
	}

	snprintf(buffer, sizeof(buffer), "%f %d %f %d %d %d %d %d %f %f %f %f %i",
		sdb[x].shield.ratio,
		sdb[x].shield.maximum,
		sdb[x].shield.freq,
		sdb[x].shield.exist,
		sdb[x].shield.active[0],
		sdb[x].shield.active[1],
		sdb[x].shield.active[2],
		sdb[x].shield.active[3],
		sdb[x].shield.damage[0],
		sdb[x].shield.damage[1],
		sdb[x].shield.damage[2],
		sdb[x].shield.damage[3],
		lasthit1);
	result = atr_add(ship, SHIELD_ATTR_NAME, buffer, GOD, (AF_MDARK + AF_WIZARD + AF_NOPROG));
	if (result != 1) {
		do_log(LT_SPACE, executor, ship, "WRITE: unable to write SHIELD attribute.");
		return 0;
	}

/* TECHNOLOGY */

	snprintf(buffer, sizeof(buffer), "%f %f %f %f %f %f %f %f %f",
		sdb[x].tech.firing,
		sdb[x].tech.fuel,
		sdb[x].tech.stealth,
		sdb[x].tech.cloak,
		sdb[x].tech.sensors,
		sdb[x].tech.aux_max,
		sdb[x].tech.main_max,
		sdb[x].tech.armor,
		sdb[x].tech.ly_range);
	result = atr_add(ship, TECHNOLOGY_ATTR_NAME, buffer, GOD, (AF_MDARK + AF_WIZARD + AF_NOPROG));
	if (result != 1) {
		do_log(LT_SPACE, executor, ship, "WRITE: unable to write TECHNOLOGY attribute.");
		return 0;
	}

/* MOVEMENT */

	snprintf(buffer, sizeof(buffer), "%d %d %f %f %f %f %f %d %d",
		sdb[x].move.time,
		sdb[x].move.dt,
		sdb[x].move.in,
		sdb[x].move.out,
		sdb[x].move.ratio,
		sdb[x].move.cochranes,
		sdb[x].move.v,
		sdb[x].move.empire,
		sdb[x].move.quadrant);
	result = atr_add(ship, MOVEMENT_ATTR_NAME, buffer, GOD, (AF_MDARK + AF_WIZARD + AF_NOPROG));
	if (result != 1) {
		do_log(LT_SPACE, executor, ship, "WRITE: unable to write MOVEMENT attribute.");
		return 0;
	}

/* CLOAK */

 	if (sdb[x].cloak.damage ==1 &&  sdb[x].cloak.lasthit != 999) {
	lasthit1=999;
	}
	else {
	lasthit1=sdb[x].cloak.lasthit;
	}

	snprintf(buffer, sizeof(buffer), "%d %d %f %d %d %f %i",
		sdb[x].cloak.version,
		sdb[x].cloak.cost,
		sdb[x].cloak.freq,
		sdb[x].cloak.exist,
		sdb[x].cloak.active,
		sdb[x].cloak.damage,
		lasthit1);
	result = atr_add(ship, CLOAK_ATTR_NAME, buffer, GOD, (AF_MDARK + AF_WIZARD + AF_NOPROG));
	if (result != 1) {
		do_log(LT_SPACE, executor, ship, "WRITE: unable to write CLOAK attribute.");
		return 0;
	}

//***********************START CUSTOM CODE***********************	

/* LIFE SUPPORT - Added by bill 4-9-2002*/
 
 	if (sdb[x].lifesupport.damage ==1 && sdb[x].lifesupport.lasthit != 999) {
	lasthit1=999;
	}
	else {
	lasthit1=sdb[x].lifesupport.lasthit;
	}
 
	snprintf(buffer, sizeof(buffer), "%f %d %i %i",
		sdb[x].lifesupport.damage,
		sdb[x].lifesupport.active,
		sdb[x].lifesupport.time,
		lasthit1);
		
		
	result = atr_add(ship, LIFESUPPORT_ATTR_NAME, buffer, GOD, (AF_MDARK + AF_WIZARD + AF_NOPROG));
	if (result != 1) {
		do_log(LT_SPACE, executor, ship, "WRITE: unable to write TRANS attribute.");
		return 0;
	}
	
// Confidence Factor

	snprintf(buffer, sizeof(buffer), "%i",
		sdb[x].cf.factor);
		
		
	result = atr_add(ship, CF_ATTR_NAME, buffer, GOD, (AF_MDARK + AF_WIZARD + AF_NOPROG));
	if (result != 1) {
		do_log(LT_SPACE, executor, ship, "WRITE: unable to write CF attribute.");
		return 0;
	}

// Breakdown Factor

	snprintf(buffer, sizeof(buffer), "%f",
		sdb[x].breakdown.factor);
		
		
	result = atr_add(ship, BREAKDOWN_ATTR_NAME, buffer, GOD, (AF_MDARK + AF_WIZARD + AF_NOPROG));
	if (result != 1) {
		do_log(LT_SPACE, executor, ship, "WRITE: unable to write Breakdown attribute.");
		return 0;
	}

// IFF Factor

	snprintf(buffer, sizeof(buffer), "%f",
		sdb[x].iff.freq);
		
		
	result = atr_add(ship, IFF_ATTR_NAME, buffer, GOD, (AF_MDARK + AF_WIZARD + AF_NOPROG));
	if (result != 1) {
		do_log(LT_SPACE, executor, ship, "WRITE: unable to write IFF attribute.");
		return 0;
	}

//***********************END CUSTOM CODE***********************

/* TRANS */

 	if (sdb[x].trans.damage ==1 &&  sdb[x].trans.lasthit != 999) {
	lasthit1=999;
	}
	else {
	lasthit1=sdb[x].trans.lasthit;
	}

	snprintf(buffer, sizeof(buffer), "%d %f %d %d %f %d %d %i",
		sdb[x].trans.cost,
		sdb[x].trans.freq,
		sdb[x].trans.exist,
		sdb[x].trans.active,
		sdb[x].trans.damage,
		sdb[x].trans.d_lock,
		sdb[x].trans.s_lock,
		lasthit1);
	result = atr_add(ship, TRANS_ATTR_NAME, buffer, GOD, (AF_MDARK + AF_WIZARD + AF_NOPROG));
	if (result != 1) {
		do_log(LT_SPACE, executor, ship, "WRITE: unable to write TRANS attribute.");
		return 0;
	}

/* TRACT */

 	if (sdb[x].tract.damage ==1 &&  sdb[x].tract.lasthit != 999) {
	lasthit1=999;
	}
	else {
	lasthit1=sdb[x].tract.lasthit;
	}

	snprintf(buffer, sizeof(buffer), "%d %f %d %d %f %d %i",
		sdb[x].tract.cost,
		sdb[x].tract.freq,
		sdb[x].tract.exist,
		sdb[x].tract.active,
		sdb[x].tract.damage,
		sdb[x].tract.lock,
		lasthit1);
	result = atr_add(ship, TRACT_ATTR_NAME, buffer, GOD, (AF_MDARK + AF_WIZARD + AF_NOPROG));
	if (result != 1) {
		do_log(LT_SPACE, executor, ship, "WRITE: unable to write TRACT attribute.");
		return 0;
	}

/* COORDS */

	snprintf(buffer, sizeof(buffer), "%f %f %f %f %f %f %f %f %f",
		sdb[x].coords.x,
		sdb[x].coords.y,
		sdb[x].coords.z,
		sdb[x].coords.xo,
		sdb[x].coords.yo,
		sdb[x].coords.zo,
		sdb[x].coords.xd,
		sdb[x].coords.yd,
		sdb[x].coords.zd);
	result = atr_add(ship, COORDS_ATTR_NAME, buffer, GOD, (AF_MDARK + AF_WIZARD + AF_NOPROG));
	if (result != 1) {
		do_log(LT_SPACE, executor, ship, "WRITE: unable to write COORDS attribute.");
		return 0;
	}

/* COURSE */

	snprintf(buffer, sizeof(buffer), "%d %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f",
		sdb[x].course.version,
		sdb[x].course.yaw_in,
		sdb[x].course.yaw_out,
		sdb[x].course.pitch_in,
		sdb[x].course.pitch_out,
		sdb[x].course.roll_in,
		sdb[x].course.roll_out,
		sdb[x].course.d[0][0],
		sdb[x].course.d[0][1],
		sdb[x].course.d[0][2],
		sdb[x].course.d[1][0],
		sdb[x].course.d[1][1],
		sdb[x].course.d[1][2],
		sdb[x].course.d[2][0],
		sdb[x].course.d[2][1],
		sdb[x].course.d[2][2],
		sdb[x].course.rate);
	result = atr_add(ship, COURSE_ATTR_NAME, buffer, GOD, (AF_MDARK + AF_WIZARD + AF_NOPROG));
	if (result != 1) {
		do_log(LT_SPACE, executor, ship, "WRITE: unable to write COURSE attribute.");
		return 0;
	}

/* MAIN */

 	if (sdb[x].main.damage ==1 &&  sdb[x].main.lasthit != 999) {
	lasthit1=999;
	}
	else {
	lasthit1=sdb[x].main.lasthit;
	}

	snprintf(buffer, sizeof(buffer), "%f %f %f %f %d %i",
		sdb[x].main.in,
		sdb[x].main.out,
		sdb[x].main.damage,
		sdb[x].main.gw,
		sdb[x].main.exist,
		lasthit1);
	result = atr_add(ship, MAIN_ATTR_NAME, buffer, GOD, (AF_MDARK + AF_WIZARD + AF_NOPROG));
	if (result != 1) {
		do_log(LT_SPACE, executor, ship, "WRITE: unable to write MAIN attribute.");
		return 0;
	}

/* AUX */

 	if (sdb[x].aux.damage ==1 &&  sdb[x].aux.lasthit != 999) {
	lasthit1=999;
	}
	else {
	lasthit1=sdb[x].aux.lasthit;
	}

	snprintf(buffer, sizeof(buffer), "%f %f %f %f %d %i",
		sdb[x].aux.in,
		sdb[x].aux.out,
		sdb[x].aux.damage,
		sdb[x].aux.gw,
		sdb[x].aux.exist,
		lasthit1);
	result = atr_add(ship, AUX_ATTR_NAME, buffer, GOD, (AF_MDARK + AF_WIZARD + AF_NOPROG));
	if (result != 1) {
		do_log(LT_SPACE, executor, ship, "WRITE: unable to write AUX attribute.");
		return 0;
	}

/* BATT */

 	if (sdb[x].batt.damage ==1 &&  sdb[x].batt.lasthit != 999) {
	lasthit1=999;
	}
	else {
	lasthit1=sdb[x].batt.lasthit;
	}

	snprintf(buffer, sizeof(buffer), "%f %f %f %f %d %i",
		sdb[x].batt.in,
		sdb[x].batt.out,
		sdb[x].batt.damage,
		sdb[x].batt.gw,
		sdb[x].batt.exist,
		lasthit1);
	result = atr_add(ship, BATT_ATTR_NAME, buffer, GOD, (AF_MDARK + AF_WIZARD + AF_NOPROG));
	if (result != 1) {
		do_log(LT_SPACE, executor, ship, "WRITE: unable to write BATT attribute.");
		return 0;
	}

/* FUEL */

	snprintf(buffer, sizeof(buffer), "%f %f %f",
			sdb[x].fuel.antimatter,
			sdb[x].fuel.deuterium,
			sdb[x].fuel.reserves);
	result = atr_add(ship, FUEL_ATTR_NAME, buffer, GOD, (AF_MDARK + AF_WIZARD + AF_NOPROG));
	if (result != 1) {
		do_log(LT_SPACE, executor, ship, "WRITE: unable to write FUEL attribute.");
		return 0;
	}

/* STATUS */

	snprintf(buffer, sizeof(buffer), "%d %d %d %d %d %d %d %d %d %d",
		sdb[x].status.active,
		sdb[x].status.docked,
		sdb[x].status.landed,
		sdb[x].status.connected,
		sdb[x].status.crippled,
 		sdb[x].status.tractoring,
		sdb[x].status.tractored,
		sdb[x].status.open_landing,
 		sdb[x].status.open_docking,
		sdb[x].status.link);
	result = atr_add(ship, STATUS_ATTR_NAME, buffer, GOD, (AF_MDARK + AF_WIZARD + AF_NOPROG));
	if (result != 1) {
		do_log(LT_SPACE, executor, ship, "WRITE: unable to write STATUS attribute.");
		return 0;
	}

 //Now that we've written the var, lets reread the stuff

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














	return 1;
}


/* -------------------------------------------------------------------- */
//Process Components
void write_process_components (MYSQL *conn, MYSQL_RES *res_set,dbref executor,int x) {
MYSQL_ROW	row;
MYSQL_FIELD *field;
unsigned int	i;
char *system1;//,*value,*newsystem;
int newsystem, active,recordid,remove1;
float value;
	
	while((row=mysql_fetch_row (res_set)) != NULL)
	{


		//Lets reset the variables
		value=atof("0");
		newsystem="0";
		active="0";
		remove1="0";
		system1=NULL;

			mysql_field_seek (res_set,0);
		for (i = 0; i < mysql_num_fields (res_set); i++)
		{
			field=mysql_fetch_field (res_set);
				//notify(executor, tprintf("Field: %i  -- %d",strcmp(field->name,"system"),strlen(field->name)));

			//Lets get the recordid
			if (strcmp(field->name,"recordid") ==0) {
				recordid=atoi(row[i]);
			}			

			//Lets see if they need to be removed
			if (strcmp(field->name,"remove") ==0) {
				remove1=atoi(row[i]);
			}

						
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
				//notify(executor, tprintf("Before %f",sdb[x].main.gw));				
				//notify(executor, tprintf("system1 %s",system1));
				//notify(executor, tprintf("AFter %f, Value: %f - %s",sdb[x].main.gw,atof(row[i]),row[i]));
				active=atoi(row[i]);
			}

			//If they want to be removed lets remove the fuckers and update the records before we process shit on our own :)
			if (remove1 == 1) {
				mysql_query (conn, tprintf("update components set active=0, remove=0 where recordid='%i'",recordid));

			}			



			//notify(executor, tprintf("system1 %s - NewSystem %i",system1,newsystem));
			//Lets not go through here if the system is inactive
			if (active == 1) {
				//notify(executor, tprintf("system1 %s - NewSystem %i",system1,newsystem));
				if (system1 == "main") {
					//notify(executor, tprintf("Before value %f, main.gw %f",value,sdb[x].main.gw));
					sdb[x].main.gw=sdb[x].main.gw - value;
					//notify(executor, tprintf("After value %f, main.gw %f",value,sdb[x].main.gw));
					sdb[x].main.in=0.999;
				}
				if (system1 == "cloak" && newsystem==1) {
				//sdb[x].main.gw=sdb[x].main.gw + atof(row[i]);
					sdb[x].cloak.exist=0;
					sdb[x].cloak.cost=0;
					sdb[x].cloak.damage = 0;
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
					sdb[x].trans.exist=0;
					sdb[x].trans.damage=0;
				}
				if (system1 == "tractor" && newsystem==1) {
				//sdb[x].main.gw=sdb[x].main.gw + atof(row[i]);
					sdb[x].tract.exist=0;
					sdb[x].tract.damage=0;
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
/* -------------- ------------------------------------------------------ */
