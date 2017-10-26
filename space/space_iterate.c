/* space_iterate.c */

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
#include "ansi.h"cd
#include "externs.h"
#include "intrface.h"
#include "parse.h"
#include "confmagic.h"
#include "space.h"
#include "function.h"
#include "attrib.h"
#include "log.h"
#include "dbdefs.h"
#include "flags.h"
/* Commands that create new objects */
#include "conf.h"
#include "mushdb.h"
#include "attrib.h"
#include "extchat.h"
#include "lock.h"
#include "game.h"
//FOR MYSQL SUPPORT
#include <mysql.h>
#include <errmsg.h>
//END MYSQL SUPPORT

/* ------------------------------------------------------------------------ */

//Define the Functions
int check_inside _((double aax, double aay, double aaz, double myrad,int secondship));
void self_destruct_count _((void));
void check_space_collision _((void));
void check_warpeject _((void));
void up_nebula _((int mynebula));
void component_odometer _((MYSQL *conn, MYSQL_RES *res_set,int x));
void check_component_breakdown _((void));


// END FUNCTION DEFINE

int    temp_sdb[MAX_SENSOR_CONTACTS];
int    temp_num[MAX_SENSOR_CONTACTS];
double temp_lev[MAX_SENSOR_CONTACTS];

extern time_t mudtime;

/* ------------------------------------------------------------------------ */

void up_alloc_balance (void)
{
	balance_eng_power();
	balance_helm_power();
	balance_shield_power();
	balance_tact_power();
	balance_sensor_power();
	balance_ops_power();
	report_eng_power();
	report_helm_power();
	report_tact_power();
	report_ops_power();
	sdb[n].alloc.version = 0;
	sdb[n].engine.version = 1;
	sdb[n].sensor.version = 1;
	sdb[n].cloak.version = 1;
	return;
}

/* -------------------------------------------------------------------- */

void up_main_io (void)
{
	if (sdb[n].main.gw)
		if (sdb[n].main.out > sdb[n].main.in) {
			sdb[n].main.out -= sdb[n].move.dt / 30.0;
			if (sdb[n].main.out <= sdb[n].main.in) {
				sdb[n].main.out = sdb[n].main.in;
				alert_main_balance(n);
			}
		} else if (sdb[n].main.out < sdb[n].main.in) {
			sdb[n].main.out += sdb[n].move.dt / 60.0;
			if (sdb[n].main.out >= sdb[n].main.in) {
				sdb[n].main.out = sdb[n].main.in;
				alert_main_balance(n);
			}
		}
	sdb[n].power.main = sdb[n].main.gw * sdb[n].main.out;
	sdb[n].power.version = 1;
	return;
}

/* ------------------------------------------------------------------------ */

void up_aux_io (void)
{
	if (sdb[n].aux.gw)
		if (sdb[n].aux.out > sdb[n].aux.in) {
			sdb[n].aux.out -= sdb[n].move.dt / 5.0;
			if (sdb[n].aux.out <= sdb[n].aux.in) {
				sdb[n].aux.out = sdb[n].aux.in;
				alert_aux_balance(n);
			}
		} else if (sdb[n].aux.out < sdb[n].aux.in) {
			sdb[n].aux.out += sdb[n].move.dt / 10.0;
			if (sdb[n].aux.out >= sdb[n].aux.in) {
				sdb[n].aux.out = sdb[n].aux.in;
				alert_aux_balance(n);
			}
		}
	sdb[n].power.aux = sdb[n].aux.gw * sdb[n].aux.out;
	sdb[n].power.version = 1;
	return;
}

/* ------------------------------------------------------------------------ */

void up_batt_io (void)
{
	sdb[n].batt.out = sdb[n].batt.in;
	alert_batt_balance(n);
	sdb[n].power.batt = sdb[n].batt.gw * sdb[n].batt.out;
	sdb[n].power.version = 1;
	return;
}

/* ------------------------------------------------------------------------ */

void up_main_damage (void)
{
	if (sdb[n].main.exist) {
		double dmg = (sdb[n].main.out - sdb[n].main.damage) *
		  sdb[n].move.dt / sdb[n].tech.main_max / 1000.0;

		if (sdb[n].main.damage > 0.0 && (sdb[n].main.damage - dmg) <= 0.0) {
			alert_main_overload(n);
		}
		sdb[n].main.damage -= dmg;
		if (sdb[n].main.damage <= -1.0) {
			sdb[n].main.damage = -1.0;
			do_all_console_notify(n, tprintf(ansi_warn("%s core breach."), system_name[8]));
			damage_structure(n, sdb[n].power.main * (getrandom(100) + 1.0));
			sdb[n].main.in = 0.0;
			sdb[n].main.out = 0.0;
			sdb[n].power.main = 0.0;
			sdb[n].power.version = 1;
		}
	}
	return;
}

/* ------------------------------------------------------------------------ */

void up_aux_damage (void)
{
	if (sdb[n].aux.exist) {
		double dmg = (sdb[n].aux.out - sdb[n].aux.damage) *
		  sdb[n].move.dt / sdb[n].tech.aux_max / 1000.0;

		if (sdb[n].aux.damage > 0.0 && (sdb[n].aux.damage - dmg) <= 0.0) {
			alert_aux_overload(n);
		}
		sdb[n].aux.damage -= dmg;
		if (sdb[n].aux.damage <= -1.0) {
			sdb[n].aux.damage = -1.0;
			do_all_console_notify(n, tprintf(ansi_warn("%s core breach."), system_name[1]));
			damage_structure(n, sdb[n].power.aux * (getrandom(10) + 1.0));
			sdb[n].aux.in = 0.0;
			sdb[n].aux.out = 0.0;
			sdb[n].power.aux = 0.0;
			sdb[n].power.version = 1;
		}
	}
	return;
}

/* -------------------------------------------------------------------- */

void up_fuel (void)
{
	double mloss = sdb[n].main.out * sdb[n].main.out * sdb[n].main.gw *
	  100.0 / sdb[n].tech.fuel * sdb[n].move.dt;
	double aloss = sdb[n].aux.out * sdb[n].aux.out * sdb[n].aux.gw *
	  100.0 / sdb[n].tech.fuel * sdb[n].move.dt;

	sdb[n].fuel.antimatter -= mloss;
	sdb[n].fuel.deuterium -= mloss + aloss;
	if (sdb[n].fuel.antimatter < 0.0) {
		if (sdb[n].main.out > 0.0)
			alert_anti_runout(n);
		sdb[n].fuel.antimatter = sdb[n].main.in = sdb[n].main.out = sdb[n].power.main = 0.0;
		sdb[n].power.version = 1;
	}
	if (sdb[n].fuel.deuterium < 0.0) {
		if (sdb[n].aux.out > 0.0 || sdb[n].main.out > 0.0)
			alert_deut_runout(n);
		sdb[n].fuel.deuterium = sdb[n].main.in = sdb[n].main.out = sdb[n].power.main = sdb[n].aux.in = sdb[n].aux.out = sdb[n].power.aux = 0.0;
		sdb[n].power.version = 1;
	}
	return;
}

/* ------------------------------------------------------------------------ */

void up_reserve (void)
{
	sdb[n].fuel.reserves += (((sdb[n].power.main + sdb[n].power.aux + sdb[n].power.batt) * sdb[n].alloc.miscellaneous) - sdb[n].power.batt) * sdb[n].move.dt;
	if (sdb[n].fuel.reserves < 0.0) {
		sdb[n].fuel.reserves = sdb[n].batt.in = sdb[n].batt.out = sdb[n].power.batt = 0.0;
		alert_batt_runout(n);
		sdb[n].power.version = 1;
	} else if (sdb[n].fuel.reserves > sdb2max_reserve(n))
		sdb[n].fuel.reserves = sdb2max_reserve(n);
	return;
}

/* ------------------------------------------------------------------------ */

void up_total_power (void)
{
	sdb[n].power.total = sdb[n].power.main + sdb[n].power.aux + sdb[n].power.batt;
	sdb[n].power.version = 0;
	sdb[n].engine.version = 1;
	sdb[n].sensor.version = 1;
	sdb[n].cloak.version = 1;
	up_turn_rate();
	return;
}

/* ------------------------------------------------------------------------ */

void up_warp_damage (void)
{
	if (sdb[n].engine.warp_exist)
		if (fabs(sdb[n].move.out) >= 1.0)
			if (fabs(sdb[n].move.out) > sdb[n].engine.warp_cruise) {
				sdb[n].engine.warp_damage -= (fabs(sdb[n].move.out) - sdb[n].engine.warp_cruise) * sdb[n].move.dt / sdb[n].tech.main_max / 10000.0;
				if (sdb[n].engine.warp_damage < 0.0) {
					sdb[n].move.in = sdb[n].move.out = sdb[n].move.v = 0.0;
					alert_warp_overload(n);
					alert_speed_stop(n);
					alert_ship_exit_warp(n);
				}
				up_warp_max();
			}
	return;
}

/* ------------------------------------------------------------------------ */

void up_impulse_damage (void)
{
	if (sdb[n].engine.impulse_exist)
		if (fabs(sdb[n].move.out) < 1.0)
			if (fabs(sdb[n].move.out) > sdb[n].engine.impulse_cruise && fabs(sdb[n].move.in) < 1.0) {
				sdb[n].engine.impulse_damage -= (fabs(sdb[n].move.out) - sdb[n].engine.impulse_cruise) * sdb[n].move.dt / sdb[n].tech.aux_max / 10000.0;
				if (sdb[n].engine.impulse_damage < 0.0) {
					sdb[n].move.in = sdb[n].move.out = sdb[n].move.v = 0.0;
					alert_impulse_overload(n);
					alert_speed_stop(n);
				}
				up_impulse_max();
			}
	return;
}

/* ------------------------------------------------------------------------ */

void up_warp_max (void)
{
	sdb[n].engine.warp_max = sdb2max_warp(n);
	sdb[n].engine.warp_cruise = sdb2cruise_warp(n);

	if ((sdb[n].move.in >= 1.0) && (sdb[n].move.in > sdb[n].engine.warp_max)) {
		sdb[n].move.in = sdb[n].engine.warp_max;
	} else if ((sdb[n].move.in <= -1.0) && (sdb[n].move.in < (-sdb[n].engine.warp_max / 2.0)))
		sdb[n].move.in = -sdb[n].engine.warp_max / 2.0;

	return;
}

/* ------------------------------------------------------------------------ */

void up_impulse_max (void)
{
	sdb[n].engine.impulse_max = sdb2max_impulse(n);
	sdb[n].engine.impulse_cruise = sdb2cruise_impulse(n);

	if ((sdb[n].move.in > 0.0) && (sdb[n].move.in < 1.0) && (sdb[n].move.in > sdb[n].engine.impulse_max)) {
		sdb[n].move.in = sdb[n].engine.impulse_max;
	} else if ((sdb[n].move.in > -1.0) && (sdb[n].move.in < 0.0) && (sdb[n].move.in < (-sdb[n].engine.impulse_max / 2.0)))
		sdb[n].move.in = -sdb[n].engine.impulse_max / 2.0;

	return;
}

/* ------------------------------------------------------------------------ */

void up_tract_status (void)
{
	register int x;
	double p;

	if (sdb[n].status.tractoring) {
		x = sdb[n].status.tractoring;
		p = sdb[n].tract.damage * sdb[n].power.total * sdb[n].alloc.tractors / (sdb2range(n, x) + 1.0);
		if ((sdb[x].tract.active && p < sdb[x].tract.damage * sdb[x].power.total * sdb[x].alloc.tractors) || p < 1.0) {
			alert_tract_lost(n, x);
			sdb[n].tract.lock = 0;
			sdb[n].status.tractoring = 0;
			sdb[x].status.tractored = 0;
			sdb[n].power.version = 1;
			sdb[x].power.version = 1;
		}
	} else if (sdb[n].status.tractored && sdb[n].tract.active) {
		x = sdb[n].status.tractored;
		p = sdb[x].tract.damage * sdb[x].power.total * sdb[x].alloc.tractors / (sdb2range(x, n) + 1.0);
		if (p < (sdb[n].tract.damage * sdb[n].power.total * sdb[n].alloc.tractors)) {
			alert_tract_lost(x, n);
			sdb[x].tract.lock = 0;
			sdb[x].status.tractoring = 0;
			sdb[n].status.tractored = 0;
			sdb[n].power.version = 1;
			sdb[x].power.version = 1;
		}
	}
	return;
}

/* ------------------------------------------------------------------------ */

void up_cloak_status (void)
{
	if (sdb[n].cloak.active)
		if (sdb[n].alloc.cloak * sdb[n].power.total < sdb[n].cloak.cost) {
			alert_cloak_failure(n);
			alert_ship_cloak_offline(n);
			sdb[n].cloak.active = 0;
			sdb[n].sensor.version = 1;
			sdb[n].engine.version = 1;
		}
	sdb[n].cloak.version = 0;
	return;
}

/* ------------------------------------------------------------------------ */

void up_beam_io (void)
{
	if (sdb[n].beam.out > sdb[n].beam.in) {
		sdb[n].beam.out = sdb[n].beam.in;
		alert_beam_balance(n);
	} else if (sdb[n].alloc.beams * sdb[n].power.total > 0.0) {
		sdb[n].beam.out += sdb[n].alloc.beams * sdb[n].power.total * sdb[n].move.dt;
		if (sdb[n].beam.out >= sdb[n].beam.in) {
			sdb[n].beam.out = sdb[n].beam.in;
			alert_beam_charged(n);
		}
	}
	if (sdb[n].beam.out < 0.0)
		sdb[n].beam.out = 0.0;
	sdb[n].sensor.version = 1;
	return;
}

/* ------------------------------------------------------------------------ */

void up_missile_io (void)
{
	if (sdb[n].missile.out > sdb[n].missile.in) {
		sdb[n].missile.out = sdb[n].missile.in;
		alert_missile_balance(n);
	} else if (sdb[n].alloc.missiles * sdb[n].power.total > 0.0) {
		sdb[n].missile.out += sdb[n].alloc.missiles * sdb[n].power.total * sdb[n].move.dt;
		if (sdb[n].missile.out >= sdb[n].missile.in) {
			sdb[n].missile.out = sdb[n].missile.in;
			alert_missile_charged(n);
		}
	}
	if (sdb[n].missile.out < 0.0)
		sdb[n].missile.out = 0.0;
	sdb[n].sensor.version = 1;
	return;
}

/* ------------------------------------------------------------------------ */

void up_autopilot (void)
{
	double r = xyz2range(sdb[n].coords.x, sdb[n].coords.y, sdb[n].coords.z,
	  sdb[n].coords.xd, sdb[n].coords.yd, sdb[n].coords.zd);
	double s = 99;
	int a = sdb[n].status.autopilot;

	if (r < 5.0) { s = 0; a = 0;
		do_console_notify(n, console_helm, 0, 0,
		  ansi_cmd(sdb[n].object, "Autopilot destination reached"));
	} else if (r < 10) { s = 0.01; a = 1;
	} else if (r < 12) { s = 0.02; a = 2;
	} else if (r < 14) { s = 0.04; a = 3;
	} else if (r < 16) { s = 0.08; a = 4;
	} else if (r < 32) { s = 0.16; a = 5;
	} else if (r < 64) { s = 0.32; a = 6;
	} else if (r < 128) { s = 0.64; a = 7;
	} else {
		r /= sdb[n].move.cochranes * LIGHTSPEED;
		if (r < 5.0) { s = 0.999; a = 8;
		} else if (r < 10.0) { s = pow(r / (int) (r), 0.3); a = 9;
		} else if (r < 20.0) { s = 1.2; a = 10;
		} else if (r < 40.0) { s = 1.5; a = 11;
		} else if (r < 80.0) { s = 1.9; a = 12;
		} else if (r < 160.0) { s = 2.3; a = 13;
		} else if (r < 320.0) { s = 2.8; a = 14;
		} else if (r < 640.0) { s = 3.5; a = 15;
		} else if (r < 1280.0) { s = 4.3; a = 16;
		} else if (r < 2560.0) { s = 5.2; a = 17;
		} else if (r < 5120.0) { s = 6.5; a = 18;
		} else if (r < 10240.0) { s = 8.0; a = 19;
		} else if (r < 20480.0) { s = 9.8; a = 20;
		} else if (r < 40960.0) { s = 12.1; a = 21;
		} else if (r < 81920.0) { s = 14.9; a = 22;
		} else if (r < 163840.0) { s = 18.4; a = 23;
		} else if (r < 327680.0) { s = 22.6; a = 24;
		} else if (r < 655360.0) { s = 27.9; a = 25;
		} else if (r < 1310720.0) { s = 34.3; a = 26;
		} else if (r < 2621440.0) { s = 42.2; a = 27;
		} else if (r < 5242880.0) { s = 52.0; a = 28;
		} else if (r < 10485760.0) { s = 64.0; a = 29;
		}
	}

	if (sdb[n].status.autopilot != a) {
		sdb[n].status.autopilot = a;
		sdb[n].course.yaw_in = xy2bearing(sdb[n].coords.xd -
		  sdb[n].coords.x, sdb[n].coords.yd - sdb[n].coords.y);
		sdb[n].course.pitch_in = xyz2elevation(sdb[n].coords.xd -
		  sdb[n].coords.x, sdb[n].coords.yd - sdb[n].coords.y,
		  sdb[n].coords.zd -sdb[n].coords.z);
		if (sdb[n].move.in > s) {
			if (s >= 1.0 && s > sdb[n].engine.warp_cruise)
				s = sdb[n].engine.warp_cruise;
			if (s < 1.0 && s > sdb[n].engine.impulse_cruise)
				s = sdb[n].engine.impulse_cruise;
			sdb[n].move.in = s;
		}
	}

	return;
}

/* ------------------------------------------------------------------------ */

void up_speed_io (void)
{
	double a;

	if (sdb[n].move.ratio <= 0.0)
		return;

	if (fabs(sdb[n].move.out) < 1.0) {
		if (fabs(sdb[n].move.in) >= 1.0) {
			a = sdb[n].power.main * 0.99 + sdb[n].power.total *
			  sdb[n].alloc.movement * 0.01;
		} else
			a = sdb[n].power.aux * 0.9 + sdb[n].power.total *
			  sdb[n].alloc.movement * 0.1;
		a *= (1.0 - fabs(sdb[n].move.out)) / sdb[n].move.ratio / 50.0;
	} else
		a = (sdb[n].power.main * 0.99 + sdb[n].power.total *
		  sdb[n].alloc.movement * 0.0) / sdb[n].move.ratio /
		  fabs(sdb[n].move.out) / 5.0;

	a *= (sdb[n].move.ratio + 1.0) / sdb[n].move.ratio * sdb[n].move.dt;

	if (sdb[n].move.out < 0.0)
		a /= 2.0;

	if (sdb[n].status.tractoring) {
		a *= (sdb[n].structure.displacement + 0.1) /
		  (sdb[sdb[n].status.tractoring].structure.displacement +
		  sdb[n].structure.displacement + 0.1);
	} else if (sdb[n].status.tractored)
		a *= (sdb[n].structure.displacement + 0.1) /
		  (sdb[sdb[n].status.tractored].structure.displacement +
		  sdb[n].structure.displacement + 0.1);

	if (a < 0.01)
		a = 0.01;

	if ((sdb[n].move.in >= 1.0) && (sdb[n].move.in > sdb[n].engine.warp_max)) {
		sdb[n].move.in = sdb[n].engine.warp_max;
	} else if ((sdb[n].move.in <= -1.0) && (sdb[n].move.in < (-sdb[n].engine.warp_max / 2.0))) {
		sdb[n].move.in = -sdb[n].engine.warp_max / 2.0;
	} else if ((sdb[n].move.in >= 0.0) && (sdb[n].move.in < 1.0) && (sdb[n].move.in > sdb[n].engine.impulse_max)) {
		sdb[n].move.in = sdb[n].engine.impulse_max;
	} else if ((sdb[n].move.in <= 0.0) && (sdb[n].move.in > -1.0) && (sdb[n].move.in < (-sdb[n].engine.impulse_max / 2.0))) {
		sdb[n].move.in = -sdb[n].engine.impulse_max / 2.0;
	}

	if (sdb[n].move.out > sdb[n].move.in) {
		if (sdb[n].move.out >= 1.0) {
			if (sdb[n].move.in >= 1.0) {
				sdb[n].move.out = sdb[n].move.in;
				alert_speed_warp(n);
			} else if (sdb[n].move.in > 0.0 && sdb[n].move.in < 1.0) {
				sdb[n].move.out = sdb[n].move.in;
				alert_speed_impulse(n);
				alert_ship_exit_warp(n);
			} else if (sdb[n].move.in <= 0.0) {
				sdb[n].move.out = 0.0;
				alert_speed_stop(n);
				alert_ship_exit_warp(n);
			}
		} else if (sdb[n].move.out > 0.0 && sdb[n].move.out < 1.0) {
			if (sdb[n].move.in > 0.0) {
				sdb[n].move.out = sdb[n].move.in;
				alert_speed_impulse(n);
			} else if (sdb[n].move.in <= 0.0) {
				sdb[n].move.out = 0.0;
				alert_speed_stop(n);
			}
		} else if (sdb[n].move.out <= 0.0) {
			if (sdb[n].move.out > -1.0) {
				sdb[n].move.out -= a;
				if (sdb[n].move.out <= sdb[n].move.in) {
					sdb[n].move.out = sdb[n].move.in;
					if (sdb[n].move.out > -1.0) {
						alert_speed_impulse(n);
					} else {
						alert_speed_warp(n);
						alert_ship_enter_warp(n);
					}
				} else if (sdb[n].move.out <= -1.0) {
					alert_ship_enter_warp(n);
				}
			} else {
				sdb[n].move.out -= a;
				if (sdb[n].move.out <= sdb[n].move.in) {
					sdb[n].move.out = sdb[n].move.in;
					alert_speed_warp(n);
				}
			}
		}
	} else if (sdb[n].move.out < sdb[n].move.in) {
		if (sdb[n].move.out <= -1.0) {
			if (sdb[n].move.in <= -1.0) {
				sdb[n].move.out = sdb[n].move.in;
				alert_speed_warp(n);
			} else if (sdb[n].move.in < 0.0 && sdb[n].move.in > -1.0) {
				sdb[n].move.out = sdb[n].move.in;
				alert_speed_impulse(n);
				alert_ship_exit_warp(n);
			} else if (sdb[n].move.in >= 0.0) {
				sdb[n].move.out = 0.0;
				alert_speed_stop(n);
				alert_ship_exit_warp(n);
			}
		} else if (sdb[n].move.out < 0.0 && sdb[n].move.out > -1.0) {
			if (sdb[n].move.in < 0.0) {
				sdb[n].move.out = sdb[n].move.in;
				alert_speed_impulse(n);
			} else if (sdb[n].move.in >= 0.0) {
				sdb[n].move.out = 0.0;
				alert_speed_stop(n);
			}
		} else if (sdb[n].move.out >= 0.0) {
			if (sdb[n].move.out < 1.0) {
				sdb[n].move.out += a;
				if (sdb[n].move.out >= sdb[n].move.in) {
					sdb[n].move.out = sdb[n].move.in;
					if (sdb[n].move.out < 1.0) {
						alert_speed_impulse(n);
					} else {
						alert_speed_warp(n);
						alert_ship_enter_warp(n);
					}
				} else if (sdb[n].move.out >= 1.0) {
					alert_ship_enter_warp(n);
				}
			} else {
				sdb[n].move.out += a;
				if (sdb[n].move.out >= sdb[n].move.in) {
					sdb[n].move.out = sdb[n].move.in;
					alert_speed_warp(n);
				}
			}
		}
	}
	sdb[n].sensor.version = 1;
	return;
}

/* ------------------------------------------------------------------------ */

void up_turn_rate (void)
{
	double a;

	if (sdb[n].move.ratio <= 0.0)
		return;

	if (fabs(sdb[n].move.out) < 1.0) {
		if (fabs(sdb[n].move.in) >= 1.0) {
			a = sdb[n].power.main * 0.99 + sdb[n].power.total *
			  sdb[n].alloc.movement * 0.01;
		} else
			a = sdb[n].power.aux * 0.9 + sdb[n].power.total *
			  sdb[n].alloc.movement * 0.1;
		a *= 3.6 * (1.0 - fabs(sdb[n].move.out)) / sdb[n].move.ratio;
	} else
		a = 3.6 * (sdb[n].power.main * 0.99 + sdb[n].power.total *
		  sdb[n].alloc.movement * 0.0) / sdb[n].move.ratio /
		  fabs(sdb[n].move.out);

	a *= (sdb[n].move.ratio + 1.0) / sdb[n].move.ratio * sdb[n].move.dt;

	if (sdb[n].move.out < 0.0)
		a /= 2.0;

	if (sdb[n].status.tractoring) {
		a *= (sdb[n].structure.displacement + 0.1) /
		  (sdb[sdb[n].status.tractoring].structure.displacement +
		  sdb[n].structure.displacement + 0.1);
	} else if (sdb[n].status.tractored)
		a *= (sdb[n].structure.displacement + 0.1) /
		  (sdb[sdb[n].status.tractored].structure.displacement +
		  sdb[n].structure.displacement + 0.1);

	if (a < 1.0)
		a = 1.0;

	sdb[n].course.rate = a;

	return;
}

/* ------------------------------------------------------------------------ */

void up_cochranes (void)
{
	sdb[n].move.cochranes = xyz2cochranes(sdb[n].coords.x, sdb[n].coords.y, sdb[n].coords.z);

	return;
}

/* ------------------Calculate Velocity----------------------------------- */

void up_velocity (void)
{
	if (sdb[n].engine.warp_exist || sdb[n].engine.impulse_exist) {
		if (sdb[n].move.out >= 1.0) {
			sdb[n].move.v = LIGHTSPEED * pow(sdb[n].move.out, 3.333333);
		} else if (sdb[n].move.out <= -1.0) {
			sdb[n].move.v = LIGHTSPEED * -pow(fabs(sdb[n].move.out), 3.333333);
		} else
			sdb[n].move.v = LIGHTSPEED * sdb[n].move.out;
	}

	return;
}

/* ------------------------------------------------------------------------ */

void up_quadrant (void)
{
	register int q;

	if (sdb[n].coords.x > 0.0) {
		if (sdb[n].coords.y > 0.0) {
			q = 0;
		} else
			q = 1;
	} else
		if (sdb[n].coords.y > 0.0) {
			q = 2;
		} else
			q = 3;
	if (sdb[n].move.quadrant != q) {
		sdb[n].move.quadrant = q;
		alert_enter_quadrant(n);
	}

	return;
}

/* ------------------------------------------------------------------------ */
// This is where we calculate all teh guts of movement

void up_empire (void)
{
	double dx, dy, dz;
	register int empire;

	for (empire = 0 ; empire < MAX_EMPIRE_NAME ; ++empire) {
		// This takes the X Y Z values of the current empire and
		// divides them by the parsec to find the centers
		dx = (empire_center[empire][1] - sdb[n].coords.x) / PARSEC;
		dy = (empire_center[empire][2] - sdb[n].coords.y) / PARSEC;
		dz = (empire_center[empire][3] - sdb[n].coords.z) / PARSEC;
		if ((dx * dx + dy * dy + dz * dz) < (empire_center[empire][0] * empire_center[empire][0])) {
			if (sdb[n].move.empire != empire) {
				if (getrandom(100) < ((int) (sdb[n].sensor.lrs_signature * sdb[n].sensor.visibility * 100.0))) {
					alert_exit_empire(n);
					alert_border_cross (n, sdb[n].move.empire, 0);
				}
				sdb[n].move.empire = empire;
				if (getrandom(100) < ((int) (sdb[n].sensor.lrs_signature * sdb[n].sensor.visibility * 100.0))) {
					alert_enter_empire(n);
					alert_border_cross (n, sdb[n].move.empire, 1);
				}
			}
			break;
		}
	}

	return;
}

/* ------------------------------------------------------------------------ */
// This is where we calculate all the guts of movement through nebulas
void up_nebula(int mynebula)
{
	float dx, dy, dz;
	ATTR *anebula,*bnebula,*cnebula;
	int py,pb,pc;
	char *qnebula,*xnebula,*znebula;
	int nexthit;
	//Modified to add custom hits
	register int j;					
	double  pdmg, d_system[16], d_shield[MAX_SHIELD_NAME],ss;
	double d_beam[MAX_BEAM_BANKS], d_missile[MAX_MISSILE_TUBES];
	int dmg_weap;

		//Lets 
		// This takes the X Y Z values of the current nebula and
		// divides them by the parsec to find the centers
		dx = (sdb[mynebula].coords.x - sdb[n].coords.x) / PARSEC;
		dy = (sdb[mynebula].coords.y - sdb[n].coords.y) / PARSEC;
		dz = (sdb[mynebula].coords.z - sdb[n].coords.z) / PARSEC;
	
		//Testing Code - Uncomment to test
		//do_ship_notify(n, tprintf("X %f Y %f Z %f",dx,dy,dz));	
		//do_ship_notify(n, tprintf("COORDS %f",(dx*dx)+(dy*dy)+(dz*dz)));	
		//do_ship_notify(n, tprintf("DISPLACEMENT %f",su2pc(sdb[mynebula].structure.displacement) * su2pc(sdb[mynebula].structure.displacement)));

		if ((dx * dx + dy * dy + dz * dz) < (su2pc(sdb[mynebula].structure.displacement) *  su2pc(sdb[mynebula].structure.displacement))) {
			//If they just entered the nebula then lets Muff them up
				  	anebula = atr_get(sdb[n].object, "NEBULA");
					if (anebula != NULL) {
					qnebula = safe_uncompress(anebula->value);
					py=atoi(qnebula);
						if (py == 0) {
							/* Entering the Nebula Lets give them the enter nebula alert*/
							atr_add(sdb[n].object, "NEBULA", tprintf("1"), GOD,(AF_MDARK + AF_WIZARD + AF_NOPROG));
							alert_enter_nebula(n,mynebula);
						}
						//This is where the bot iterate will go that checks 
						if (py == 1) {

							//Lets get the ShotCounter
							bnebula = atr_get(sdb[mynebula].object, "SHOTCOUNTER");
							// We cant have either of these things being NULL :)
							if (bnebula == NULL) {
								atr_add(sdb[mynebula].object, "SHOTCOUNTER", tprintf("0"), GOD,(AF_MDARK + AF_WIZARD + AF_NOPROG));	
							}
							else if (bnebula != NULL){
								xnebula = safe_uncompress(bnebula->value);
								pb=atoi(xnebula);
							}
							//Lets get the NEXTSHOT
							cnebula = atr_get(sdb[mynebula].object, "NEXTSHOT");
							if (cnebula == NULL) {
								nexthit=getrandom(60)+10;
								atr_add(sdb[mynebula].object, "NEXTSHOT", tprintf("%i",nexthit), GOD,(AF_MDARK + AF_WIZARD + AF_NOPROG));	
							}
							else if (cnebula !=NULL ){
							znebula = safe_uncompress(cnebula->value);
							pc=atoi(znebula);
							}

							//Lets check to see if its time to hurt the ship
							if (pb == pc && cnebula != NULL && bnebula != NULL) {
								//FIRE CODE HERE

				pdmg=0;
				d_system[1]=0;
				d_system[2]=0;
				d_system[3]=0;
				d_system[4]=0;
				d_system[5]=0;
				d_system[6]=0;
				d_system[7]=0;
				d_system[8]=0;
				d_system[9]=0;
				d_system[10]=0;
				d_system[11]=0;
				d_system[12]=0;
				d_system[13]=0;
				d_system[14]=0;
				d_system[15]=0;

								
							pdmg = getrandom(500) / 100.0;
							
							switch (getrandom(10) + getrandom(10) + 2) {
							case 2: d_shield[getrandom(MAX_SHIELD_NAME)] += pdmg; break;
							case 3: d_system[11] += pdmg; break;
							case 4: d_system[7] += pdmg; break;
							case 5: d_system[5] += pdmg; break;
							case 6: d_system[1] += pdmg; break;
							case 7: d_system[8] += pdmg; break;
							case 13:  d_system[15] += pdmg; break;
							case 15: d_system[14] += pdmg; break;
							case 16: d_system[6] += pdmg; break;
							case 17: d_system[13] += pdmg; break;
							case 18: d_system[12] += pdmg; break;
							case 19: d_system[4] += pdmg; break;
							case 20: d_system[2] += pdmg; break;
							default: break;
						}


				do_ship_notify(n, ansi_alert(tprintf("The vessel rocks as energy from the nebula buffets it.")));

				if (sdb[n].shield.exist && sdb[n].shield.damage[0] > 0 && sdb[n].shield.damage[1] > 0 && sdb[n].shield.damage[2] > 0 && sdb[n].shield.damage[3] > 0) {
					for (j = 0; j < MAX_SHIELD_NAME; ++j)
						if (d_shield[j] > 0.0)
							damage_shield(n, j, d_shield[j], "999");
				}
				else {
				if (d_system[0] > 0.0) {
				pdmg = sdb[n].structure.superstructure;
				damage_structure(n, d_system[0]);
				if (d_system[1] > 0.0 && sdb[n].aux.exist) {
					damage_aux(n, d_system[1], "999");
				}
				if (d_system[8] > 0.0 && sdb[n].main.exist) {
					damage_main(n, d_system[8], "999");
				}
				if (d_system[2] > 0.0 && sdb[n].batt.exist)
					damage_batt(n, d_system[2], "999");
				if (d_system[4] > 0.0 && sdb[n].cloak.exist)
					damage_cloak(n, d_system[4], "999");
				if (d_system[5] > 0.0 && sdb[n].sensor.ew_exist)
					damage_ew(n, d_system[5], "999");
				if (d_system[6] > 0.0 && sdb[n].engine.impulse_exist)
					damage_impulse(n, d_system[6], dmg_weap);
				if (d_system[7] > 0.0 && sdb[n].sensor.lrs_exist)
					damage_lrs(n, d_system[7], "999");
				if (d_system[11] > 0.0 && sdb[n].sensor.srs_exist)
					damage_srs(n, d_system[11], "999");
				if (d_system[12] > 0.0 && sdb[n].tract.exist)
					damage_tract(n, d_system[12], "999");
				if (d_system[13] > 0.0 && sdb[n].trans.exist)
					damage_trans(n, d_system[13], "999");
				if (d_system[14] > 0.0 && sdb[n].engine.warp_exist)
					damage_warp(n, d_system[14], "999");
				if (d_system[15] > 0.0 && sdb[n].lifesupport.active)
					damage_lifesupport(n, d_system[15], "999");
				
				//do_fed_shield_bug_check(n);
				pdmg=0;
				d_system[1]=0;
				d_system[2]=0;
				d_system[3]=0;
				d_system[4]=0;
				d_system[5]=0;
				d_system[6]=0;
				d_system[7]=0;
				d_system[8]=0;
				d_system[9]=0;
				d_system[10]=0;
				d_system[11]=0;
				d_system[12]=0;
				d_system[13]=0;
				d_system[14]=0;
				d_system[15]=0;
				}
	
				
				
			}


								//RESET NEXTSHOT
								atr_add(sdb[mynebula].object, "SHOTCOUNTER", tprintf("0"), GOD,(AF_MDARK + AF_WIZARD + AF_NOPROG));	
								nexthit=getrandom(60)+10;
								atr_add(sdb[mynebula].object, "NEXTSHOT", tprintf("%i",nexthit), GOD,(AF_MDARK + AF_WIZARD + AF_NOPROG));	
							}
							else {
								atr_add(sdb[mynebula].object, "SHOTCOUNTER", tprintf("%i",pb + 1), GOD,(AF_MDARK + AF_WIZARD + AF_NOPROG));	
							}
						}

					}
					else {
						/* Default the value to 0 */
						atr_add(sdb[n].object, "NEBULA", tprintf("0"), GOD,(AF_MDARK + AF_WIZARD + AF_NOPROG));	
					}
		}
		//We only want them to EXIT THE NEBULA when they are out
		else {
				  	anebula = atr_get(sdb[n].object, "NEBULA");
					if (anebula != NULL) {
					qnebula = safe_uncompress(anebula->value);
					py=atoi(qnebula);
						if (py == 1) {
							// Exiting the Nebula lets give them the exit nebula alert
							atr_add(sdb[n].object, "NEBULA", tprintf("0"), GOD,(AF_MDARK + AF_WIZARD + AF_NOPROG));
							alert_exit_nebula(n,mynebula);
						}
					}
					else {
						/* Default the Value to 0 */
						atr_add(sdb[n].object, "NEBULA", tprintf("0"), GOD,(AF_MDARK + AF_WIZARD + AF_NOPROG));	
					}

		}
		
		//Lets free up some ram
		free((Malloc_t) qnebula);
		free((Malloc_t) znebula);

	return;
}

/* ------------------------------------------------------------------------ */

void up_visibility (void)
{
	double x, y, z;
	ATTR *avisibility;
	int py;
	char *qvisibility;

	//Lets see if they are in a nebula
    avisibility = atr_get(sdb[n].object, "NEBULA");

	if (sdb[n].status.docked || sdb[n].status.landed) {
		sdb[n].sensor.visibility = 1.0;
	} 
	
	else {
		sdb[n].sensor.visibility = xyz2vis(sdb[n].coords.x, sdb[n].coords.y, sdb[n].coords.z);
	}

	if (avisibility != NULL) {
	qvisibility = safe_uncompress(avisibility->value);
	py=atoi(qvisibility);
	if (py == 1) {
		//If they are in a nebula they are going to be fubared
		sdb[n].sensor.visibility = 0.10;
		//Lets free up some ram
		free((Malloc_t) qvisibility);
	}
	}




	return;
}

/* ------------------------------------------------------------------------ */

void up_yaw_io (void)
{
	if (sdb[n].course.yaw_out < sdb[n].course.yaw_in) {
		if ((sdb[n].course.yaw_in - sdb[n].course.yaw_out) <= 180.0) {
			sdb[n].course.yaw_out += sdb[n].course.rate * sdb[n].move.dt;
			if (sdb[n].course.yaw_out >= sdb[n].course.yaw_in) {
				sdb[n].course.yaw_out = sdb[n].course.yaw_in;
				alert_yaw(n);
			}
		} else {
			sdb[n].course.yaw_out -= sdb[n].course.rate * sdb[n].move.dt;
			if (sdb[n].course.yaw_out < 0.0) {
				sdb[n].course.yaw_out += 360.0;
				if (sdb[n].course.yaw_out <= sdb[n].course.yaw_in) {
					sdb[n].course.yaw_out = sdb[n].course.yaw_in;
					alert_yaw(n);
				}
			}
		}
	} else {
		if ((sdb[n].course.yaw_out - sdb[n].course.yaw_in) <= 180.0) {
			sdb[n].course.yaw_out -= sdb[n].course.rate * sdb[n].move.dt;
			if (sdb[n].course.yaw_out <= sdb[n].course.yaw_in) {
				sdb[n].course.yaw_out = sdb[n].course.yaw_in;
				alert_yaw(n);
			}
		} else {
			sdb[n].course.yaw_out += sdb[n].course.rate * sdb[n].move.dt;
			if (sdb[n].course.yaw_out >= 360.0) {
				sdb[n].course.yaw_out -= 360.0;
				if (sdb[n].course.yaw_out >= sdb[n].course.yaw_in) {
					sdb[n].course.yaw_out = sdb[n].course.yaw_in;
					alert_yaw(n);
				}
			}
		}
	}
	sdb[n].course.version = 1;

	return;
}

/* ------------------------------------------------------------------------ */

void up_pitch_io (void)
{
	if (sdb[n].course.pitch_out < sdb[n].course.pitch_in) {
		if ((sdb[n].course.pitch_in - sdb[n].course.pitch_out) <= 180.0) {
			sdb[n].course.pitch_out += sdb[n].course.rate * sdb[n].move.dt;
			if (sdb[n].course.pitch_out >= sdb[n].course.pitch_in) {
				sdb[n].course.pitch_out = sdb[n].course.pitch_in;
				alert_pitch(n);
			}
		} else {
			sdb[n].course.pitch_out -= sdb[n].course.rate * sdb[n].move.dt;
			if (sdb[n].course.pitch_out < 0.0) {
				sdb[n].course.pitch_out += 360.0;
				if (sdb[n].course.pitch_out <= sdb[n].course.pitch_in) {
					sdb[n].course.pitch_out = sdb[n].course.pitch_in;
					alert_pitch(n);
				}
			}
		}
	} else {
		if ((sdb[n].course.pitch_out - sdb[n].course.pitch_in) <= 180.0) {
			sdb[n].course.pitch_out -= sdb[n].course.rate * sdb[n].move.dt;
			if (sdb[n].course.pitch_out <= sdb[n].course.pitch_in) {
				sdb[n].course.pitch_out = sdb[n].course.pitch_in;
				alert_pitch(n);
			}
		} else {
			sdb[n].course.pitch_out += sdb[n].course.rate * sdb[n].move.dt;
			if (sdb[n].course.pitch_out >= 360.0) {
				sdb[n].course.pitch_out -= 360.0;
				if (sdb[n].course.pitch_out >= sdb[n].course.pitch_in) {
					sdb[n].course.pitch_out = sdb[n].course.pitch_in;
					alert_pitch(n);
				}
			}
		}
	}
	sdb[n].course.version = 1;

	return;
}

/* ------------------------------------------------------------------------ */

void up_roll_io (void)
{
	if (sdb[n].course.roll_out < sdb[n].course.roll_in) {
		if ((sdb[n].course.roll_in - sdb[n].course.roll_out) <= 180.0) {
			sdb[n].course.roll_out += sdb[n].course.rate * sdb[n].move.dt;
			if (sdb[n].course.roll_out >= sdb[n].course.roll_in) {
				sdb[n].course.roll_out = sdb[n].course.roll_in;
				alert_roll(n);
			}
		} else {
			sdb[n].course.roll_out -= sdb[n].course.rate * sdb[n].move.dt;
			if (sdb[n].course.roll_out < 0.0) {
				sdb[n].course.roll_out += 360.0;
				if (sdb[n].course.roll_out <= sdb[n].course.roll_in) {
					sdb[n].course.roll_out = sdb[n].course.roll_in;
					alert_roll(n);
				}
			}
		}
	} else {
		if ((sdb[n].course.roll_out - sdb[n].course.roll_in) <= 180.0) {
			sdb[n].course.roll_out -= sdb[n].course.rate * sdb[n].move.dt;
			if (sdb[n].course.roll_out <= sdb[n].course.roll_in) {
				sdb[n].course.roll_out = sdb[n].course.roll_in;
				alert_roll(n);
			}
		} else {
			sdb[n].course.roll_out += sdb[n].course.rate * sdb[n].move.dt;
			if (sdb[n].course.roll_out >= 360.0) {
				sdb[n].course.roll_out -= 360.0;
				if (sdb[n].course.roll_out >= sdb[n].course.roll_in) {
					sdb[n].course.roll_out = sdb[n].course.roll_in;
					alert_roll(n);
				}
			}
		}
	}
	sdb[n].course.version = 1;

	return;
}

/* ------------------------------------------------------------------------ */

void up_vectors (void)
{
	double d2r = PI / 180.0;
	double sy = sin(sdb[n].course.yaw_out * d2r);
	double cy = cos(sdb[n].course.yaw_out * d2r);
	double sp = sin(sdb[n].course.pitch_out * d2r);
	double cp = cos(sdb[n].course.pitch_out * d2r);
	double sr = sin(sdb[n].course.roll_out * d2r);
	double cr = cos(sdb[n].course.roll_out * d2r);

	sdb[n].course.d[0][0] = cy * cp;
	sdb[n].course.d[0][1] = sy * cp;
	sdb[n].course.d[0][2] = sp;
	sdb[n].course.d[1][0] = -(sy * cr) + (cy * sp * sr);
	sdb[n].course.d[1][1] = (cy * cr) + (sy * sp * sr);
	sdb[n].course.d[1][2] = -(cp * sr);
	sdb[n].course.d[2][0] = -(sy * sr) - (cy * sp * cr);
	sdb[n].course.d[2][1] = (cy * sr) - (sy * sp * cr);
	sdb[n].course.d[2][2] = (cp * cr);
	sdb[n].course.version = 0;

	return;
}

/* ------------------------------------------------------------------------ */

void up_position (void)
{
	double dv = sdb[n].move.v * sdb[n].move.dt;

	if (fabs(sdb[n].move.out) >= 1.0)
		dv *= sdb[n].move.cochranes;
	if (sdb[n].status.tractoring) {
		sdb[sdb[n].status.tractoring].coords.x += dv * sdb[n].course.d[0][0];
		sdb[sdb[n].status.tractoring].coords.y += dv * sdb[n].course.d[0][1];
		sdb[sdb[n].status.tractoring].coords.z += dv * sdb[n].course.d[0][2];
	} else if (sdb[n].status.tractored) {
		sdb[sdb[n].status.tractored].coords.x += dv * sdb[n].course.d[0][0];
		sdb[sdb[n].status.tractored].coords.y += dv * sdb[n].course.d[0][1];
		sdb[sdb[n].status.tractored].coords.z += dv * sdb[n].course.d[0][2];
	}
	sdb[n].coords.x += dv * sdb[n].course.d[0][0];
	sdb[n].coords.y += dv * sdb[n].course.d[0][1];
	sdb[n].coords.z += dv * sdb[n].course.d[0][2];

	return;
}

/* ------------------------------------------------------------------------ */

void up_resolution (void)
{
	int i;

	if (sdb[n].sensor.lrs_active) {
		sdb[n].sensor.lrs_resolution = sdb[n].tech.sensors * sdb[n].sensor.lrs_damage;
		if (sdb[n].sensor.ew_active)
			 sdb[n].sensor.lrs_resolution *= sdb2eccm_lrs(n);
		if (sdb[n].cloak.active)
			sdb[n].sensor.lrs_resolution /= 10.0;
	} else
		sdb[n].sensor.lrs_resolution = 0.0;
	if (sdb[n].sensor.srs_active) {
		sdb[n].sensor.srs_resolution = sdb[n].tech.sensors * sdb[n].sensor.srs_damage;
		if (sdb[n].sensor.ew_active)
			sdb[n].sensor.srs_resolution *= sdb2eccm_srs(n);
		if (sdb[n].cloak.active)
			sdb[n].sensor.srs_resolution /= 10.0;
	} else
		sdb[n].sensor.srs_resolution = 0.0;
	return;
}

/* ------------------------------------------------------------------------ */

void up_signature (int x)
{
	double base = pow(sdb[x].structure.displacement, 0.333333) / sdb[x].tech.stealth / 100.0;
	double sig = base, cloak;
	ATTR *asignature;
	int py;
	char *qsignature;

	if (sdb[x].cloak.active) {
		sdb[x].cloak.level = 0.001 / sdb[x].power.total / sdb[x].alloc.cloak
		  / sdb[x].tech.cloak * sdb[x].cloak.cost;
		if (sdb[x].status.tractored)
			sdb[x].cloak.level *= 100.0;
		if (sdb[x].status.tractoring)
			sdb[x].cloak.level *= 100.0;
		if (sdb[x].beam.out > 1.0)
			sdb[x].cloak.level *= sdb[x].beam.out;
		if (sdb[x].missile.out > 1.0)
			sdb[x].cloak.level *= sdb[x].missile.out;
		if (sdb[x].sensor.visibility < 1.0)
			sdb[x].cloak.level *= (1.0 - sdb[x].sensor.visibility) * 10000.0;
		if (sdb[x].cloak.level > 1.0)
			sdb[x].cloak.level = 1.0;
	} else
		sdb[x].cloak.level = 1.0;

	sdb[x].sensor.lrs_signature = sig;
	sdb[x].sensor.srs_signature = sig * 10.0;
	sdb[x].sensor.lrs_signature *= sdb[x].move.out * sdb[n].move.out + 1.0;
	sdb[x].sensor.srs_signature *= 1.0 + sdb[x].power.main +
	  (sdb[x].power.aux / 10.0) + (sdb[x].power.batt / 100.0);
	if (sdb[x].sensor.ew_active) {
		sdb[x].sensor.lrs_signature /= sdb2ecm_lrs(x);
		sdb[x].sensor.srs_signature /= sdb2ecm_srs(x);
	}
	sdb[x].sensor.version = 0;

	//Lets see if they are in a nebula
    asignature = atr_get(sdb[n].object, "NEBULA");

	if (asignature != NULL) {
	qsignature = safe_uncompress(asignature->value);
	py=atoi(qsignature);
	if (py == 1) {
		//If they are in a nebula they are going to be fubared
		sdb[x].sensor.lrs_signature = .10;
		sdb[x].sensor.srs_signature = .10;
		sdb[x].sensor.lrs_resolution = .10;
		sdb[x].sensor.srs_resolution = .10;
	}
		//Lets free up some ram
		free((Malloc_t) qsignature);
	}	
	
	
	return;
}

/* ------------------------------------------------------------------------ */

void up_sensor_message (int contacts)
{
	register int i, j, lose, gain, flag;

	for (i = 0 ; i < contacts ; ++i) {
		gain = 0;
		for (j = 0 ; j < sdb[n].sensor.contacts ; ++j) {
			if (temp_sdb[i] == sdb[n].slist.sdb[j]) {
				gain = 1;
				temp_num[i] = sdb[n].slist.num[j];
				break;
			}
		}
		if (!gain) {
			++sdb[n].sensor.counter;
			if (sdb[n].sensor.counter > 999) {
				sdb[n].sensor.counter = 1;
			}
			temp_num[i] = sdb[n].sensor.counter;
			do_console_notify(n, console_helm, console_science, console_tactical,
			  ansi_warn(tprintf("New sensor contact (%d): %s",
			  temp_num[i], unparse_type(temp_sdb[i]))));
		}
	}
	for (i = 0 ; i < sdb[n].sensor.contacts ; ++i) {
		lose = 0;
		for (j = 0 ; j < contacts ; ++j) {
			if (temp_sdb[j] == sdb[n].slist.sdb[i]) {
				lose = 1;
				break;
			}
		}
		if (!lose) {
			do_console_notify(n, console_helm, console_science, console_tactical,
			  ansi_warn(tprintf("%s contact lost: %s",
			  unparse_type(sdb[n].slist.sdb[i]),
			  unparse_identity(n, sdb[n].slist.sdb[i]))));
			if (sdb[n].trans.s_lock == sdb[n].slist.sdb[i]) {
				do_console_notify(n, console_operation, console_transporter, 0,
				  ansi_warn(tprintf("%s source lock on %s lost",
				  system_name[13], unparse_identity(n, sdb[n].slist.sdb[i]))));
				sdb[n].trans.s_lock = 0;
			}
			if (sdb[n].trans.d_lock == sdb[n].slist.sdb[i]) {
				do_console_notify(n, console_operation, console_transporter, 0,
				  ansi_warn(tprintf("%s lock on %s lost",
				  system_name[13], unparse_identity(n, sdb[n].slist.sdb[i]))));
				sdb[n].trans.d_lock = 0;
			}
			if (sdb[n].tract.lock == sdb[n].slist.sdb[i]) {
				do_console_notify(n, console_operation, console_helm, 0,
				  ansi_warn(tprintf("%s lock on %s lost",
				  system_name[12], unparse_identity(n, sdb[n].slist.sdb[i]))));
				sdb[n].tract.lock = 0;
				sdb[n].status.tractoring = 0;
				sdb[sdb[n].slist.sdb[i]].status.tractored = 0;
				sdb[n].engine.version = 1;
				sdb[sdb[n].slist.sdb[i]].engine.version = 1;
			}
			flag = 0;
			for ( j = 0 ; j < sdb[n].beam.banks ; ++j )
				if (sdb[n].blist.lock[j] == sdb[n].slist.sdb[i]) {
					flag = 1;
					sdb[n].blist.lock[j] = 0;
				}
			if (flag)
				do_console_notify(n, console_tactical, 0, 0,
				  ansi_warn(tprintf("%s lock on %s lost",
				  system_name[3], unparse_identity(n, sdb[n].slist.sdb[i]))));
			flag = 0;
			for ( j = 0 ; j < sdb[n].missile.tubes ; ++j )
				if (sdb[n].mlist.lock[j] == sdb[n].slist.sdb[i]) {
					flag = 1;
					sdb[n].mlist.lock[j] = 0;
				}
			if (flag)
				do_console_notify(n, console_tactical, 0, 0,
				  ansi_warn(tprintf("%s lock on %s lost",
				  system_name[9], unparse_identity(n, sdb[n].slist.sdb[i]))));
		}
	}
	sdb[n].sensor.contacts = contacts;
	if (contacts == 0) {
		sdb[n].sensor.counter = 0;
	} else {
		for (i = 0 ; i < contacts ; ++i) {
			sdb[n].slist.sdb[i] = temp_sdb[i];
			sdb[n].slist.num[i] = temp_num[i];
			sdb[n].slist.lev[i] = temp_lev[i];
		}
	}
	return;
}

/* --------------Edited by bill--------------------------- */

void up_sensor_list (void)
{
	register int object, i;
	register int contacts = 0;
	double x, y, z, level, limit = PARSEC * 100.0;

	for (object = MIN_SPACE_OBJECTS ; object <= max_space_objects ; ++object)
		if (sdb[n].location == sdb[object].location &&
		  sdb[n].space == sdb[object].space &&
		  sdb[object].structure.type &&
		  n != object) {
			x = fabs(sdb[n].coords.x - sdb[object].coords.x);
			if (x > limit)
				continue;
			y = fabs(sdb[n].coords.y - sdb[object].coords.y);
			if (y > limit)
				continue;
			z = fabs(sdb[n].coords.z - sdb[object].coords.z);
			if (z > limit)
				continue;
			level = (sdb[n].sensor.srs_resolution + 0.01) * sdb[object].sensor.srs_signature / (0.1 + (x * x + y * y + z * z) / 10101.010101);
			x /= PARSEC;
			y /= PARSEC;
			z /= PARSEC;
			level += sdb[n].sensor.lrs_resolution * sdb[object].sensor.lrs_signature / (1.0 + (x * x + y * y + z * z) * 99.0);
			level *= sdb[n].sensor.visibility * sdb[object].sensor.visibility;
			if (level < 0.01)
				continue;
			if (sdb[object].cloak.active)
				if (sdb[n].tech.sensors < 2.0)
					level *= sdb[object].cloak.level;
			if (level < 0.01)
				continue;
			temp_sdb[contacts] = object;
			temp_lev[contacts] = level;
			++contacts;
			if (contacts == MAX_SENSOR_CONTACTS) {
				break;
			}
		}

	if (contacts != sdb[n].sensor.contacts) {
		up_sensor_message(contacts);
	} else
		for (i = 0 ; i < sdb[n].sensor.contacts ; ++i) {
			sdb[n].slist.sdb[i] = temp_sdb[i];
			sdb[n].slist.lev[i] = temp_lev[i];
		}

	return;
}

/* ------------------------------------------------------------------------ */

void up_repair (void)
{
	sdb[n].structure.repair += sdb[n].move.dt * sdb[n].structure.max_repair / 1000.0 *
	   (1.0 + sqrt(sdb[n].alloc.miscellaneous * sdb[n].power.total));
	if (sdb[n].structure.repair >= sdb[n].structure.max_repair) {
		sdb[n].structure.repair = sdb[n].structure.max_repair;
		alert_max_repair(n);
	}
	return;
}

/*---------------------------------------------------------------------------*/

void check_lifesupport(void){
		
	char *shiplifesupport;
	sprintf(shiplifesupport,"#%i",sdb[n].object);				
	
					if (sdb[n].lifesupport.active != 1  && sdb[n].lifesupport.time < 301) {
						if (sdb[n].lifesupport.time == 30) {
							do_zemit(5,shiplifesupport,ANSI_HILITE ANSI_RED ANSI_BLINK "Computer Says: \"Warning! Life Support Offline Oyxgen Levels at 95%%. Ambient Room Temperature at 70 degress and falling.\"" ANSI_WHITE ANSI_NORMAL);
						}
						if (sdb[n].lifesupport.time == 60) {
							do_zemit(5,shiplifesupport,ANSI_HILITE ANSI_RED ANSI_BLINK "Computer Says: \"Warning! Life Support Offline Oyxgen Levels at 90%%. Ambient Room Temperature at 60 degress and falling.\"" ANSI_WHITE ANSI_NORMAL);
						}
						if (sdb[n].lifesupport.time == 90) {
							do_zemit(5,shiplifesupport,ANSI_HILITE ANSI_RED ANSI_BLINK "Computer Says: \"Warning! Life Support Offline Oyxgen Levels at 85%%. Ambient Room Temperature at 50 degress and falling.\"" ANSI_WHITE ANSI_NORMAL);
						}						
						if (sdb[n].lifesupport.time == 120) {
							do_zemit(5,shiplifesupport,ANSI_HILITE ANSI_RED ANSI_BLINK "Computer Says: \"Warning! Life Support Offline Oyxgen Levels at 80%%. Ambient Room Temperature at 40 degress and falling.\"" ANSI_WHITE ANSI_NORMAL);
						}						
						if (sdb[n].lifesupport.time == 150) {
							do_zemit(5,shiplifesupport,ANSI_HILITE ANSI_RED ANSI_BLINK "Computer Says: \"Warning! Life Support Offline Oyxgen Levels at 75%%. Ambient Room Temperature at 30 degress and falling.\"" ANSI_WHITE ANSI_NORMAL);
						}						
						if (sdb[n].lifesupport.time == 180) {
							do_zemit(5,shiplifesupport,ANSI_HILITE ANSI_RED ANSI_BLINK "Computer Says: \"Warning! Life Support Offline Oyxgen Levels at 55%%. Ambient Room Temperature at 20 degress and falling.\"" ANSI_WHITE ANSI_NORMAL);
						}						
						if (sdb[n].lifesupport.time == 210) {
							do_zemit(5,shiplifesupport,ANSI_HILITE ANSI_RED ANSI_BLINK "Computer Says: \"Warning! Life Support Offline Oyxgen Levels at 45%%. Ambient Room Temperature at 10 degress and falling.\"" ANSI_WHITE ANSI_NORMAL);
						}						
						if (sdb[n].lifesupport.time == 240) {
							do_zemit(5,shiplifesupport,ANSI_HILITE ANSI_RED ANSI_BLINK "Computer Says: \"Warning! Life Support Offline Oyxgen Levels at 30%%. Ambient Room Temperature at 0 degress and falling.\"" ANSI_WHITE ANSI_NORMAL);
						}						
						if (sdb[n].lifesupport.time == 270) {
							do_zemit(5,shiplifesupport,ANSI_HILITE ANSI_RED ANSI_BLINK "Computer Says: \"Warning! Life Support Offline Oyxgen Levels at 15%%. Ambient Room Temperature at -25 degress and falling.\"" ANSI_WHITE ANSI_NORMAL);
						}						
						if (sdb[n].lifesupport.time == 300) {
							do_zemit(5,shiplifesupport,ANSI_HILITE ANSI_RED ANSI_BLINK "Computer Says: \"Warning! Life Support Offline Oyxgen Levels at 0%%. Ambient Room Temperature at -50 degress and falling.\"" ANSI_WHITE ANSI_NORMAL);
	
						/* Death code would go in here - bill */
						}													
					++sdb[n].lifesupport.time ;
					}
					/* Lets see if life support came back*/
					if (sdb[n].lifesupport.active != 1  && sdb[n].lifesupport.damage > 0.0) {
						sdb[n].lifesupport.active=1;
						sdb[n].lifesupport.time=0;					
							do_zemit(5,shiplifesupport,ANSI_HILITE ANSI_RED ANSI_BLINK "Computer Says: \"Life Support Restored. Have a nice day.\"" ANSI_WHITE ANSI_NORMAL);

							/*do_ship_notify(n, tprintf("%s%s%sComputer Says: \"Life Support Restored. Have a nice day.\"%s%s",
				            *ANSI_HILITE, ANSI_RED, ANSI_BLINK,ANSI_WHITE, ANSI_NORMAL));								*/
					}

					return;
}
/* ------------------------------------------------------------------------ */

void set_breakdown (void)
{
	double cf;
	ATTR *bbreakdown,*cbreakdown;
	int py;
	float pz;
	char *qbreakdown,*zbreakdown;


	
					   /* Increment the BREAKDOWN */
					   bbreakdown = atr_get(sdb[n].object, "ODOMETER");
					   cbreakdown = atr_get(sdb[n].object, "CF");			
	
					   if (cbreakdown !=NULL)	{
					   qbreakdown = safe_uncompress(cbreakdown->value);
						py=atoi(qbreakdown);
					   }
					   if (bbreakdown !=NULL){
						zbreakdown = safe_uncompress(bbreakdown->value);
						pz=atof(zbreakdown);
					   }
	
						/* WHAT DO I DO? 
						* 	I COMPARE THE 'CF' VALUE OF A SHIP AND THEN DETERMINE USING THAT NUMBER
						*	WHEN THE NEXT RANDOM BREAKDOWN IS GOING TO OCCUR
						*/	
						/* If cf is null then we have to set it to 100 for default */					   
				   		if (py ==100 || cbreakdown == NULL) {
							cf=pz + (.007000 + getrandom(.007000));
						    atr_add(sdb[n].object, "CF", tprintf("100"), GOD,(AF_MDARK + AF_WIZARD + AF_NOPROG));        				  
 						    atr_add(sdb[n].object, "BREAKDOWN", tprintf("%f",cf), GOD,(AF_MDARK + AF_WIZARD + AF_NOPROG)); }
				   		else if (py >=91 && py <=99) {
							cf=pz + (.006000 + getrandom(.001000));
 						    atr_add(sdb[n].object, "BREAKDOWN", tprintf("%f",cf), GOD,(AF_MDARK + AF_WIZARD + AF_NOPROG)); }
				   		else if (py >=81 && py <=89) {
							cf=pz + (.005500 + getrandom(.000500));
 						    atr_add(sdb[n].object, "BREAKDOWN", tprintf("%f",cf), GOD,(AF_MDARK + AF_WIZARD + AF_NOPROG));		 }					
				   		else if (py >=71 && py <=79)  {
							cf=pz + (.005000 + getrandom(.000500));
 						    atr_add(sdb[n].object, "BREAKDOWN", tprintf("%f",cf), GOD,(AF_MDARK + AF_WIZARD + AF_NOPROG));		 }					
				   		else if (py >=61 && py <=69) {
							cf=pz + (.004000 + getrandom(.000500));
 						    atr_add(sdb[n].object, "BREAKDOWN", tprintf("%f",cf), GOD,(AF_MDARK + AF_WIZARD + AF_NOPROG));	 }						
				   		else if (py >=51 && py <=59) {
							cf=pz + (.003500 + getrandom(.000500));
 						    atr_add(sdb[n].object, "BREAKDOWN", tprintf("%f",cf), GOD,(AF_MDARK + AF_WIZARD + AF_NOPROG));		 }					
				   		else if (py >=41 && py <=49) {
							cf=pz + (.003000 + getrandom(.000600));
 						    atr_add(sdb[n].object, "BREAKDOWN", tprintf("%f",cf), GOD,(AF_MDARK + AF_WIZARD + AF_NOPROG));		 }					
				   		else if (py >=31 && py <=39) {
							cf=pz + (.002500 + getrandom(.001100));
 						    atr_add(sdb[n].object, "BREAKDOWN", tprintf("%f",cf), GOD,(AF_MDARK + AF_WIZARD + AF_NOPROG));	 }						
				   		else if (py >=21 && py <=29) {
							cf=pz + (.002000 + getrandom(.001600));
 						    atr_add(sdb[n].object, "BREAKDOWN", tprintf("%f",cf), GOD,(AF_MDARK + AF_WIZARD + AF_NOPROG));		 }					
				   		else if (py >=11 && py <=19) {
							cf=pz + (.001800 + getrandom(.001800));
 						    atr_add(sdb[n].object, "BREAKDOWN", tprintf("%f",cf), GOD,(AF_MDARK + AF_WIZARD + AF_NOPROG));	 }						
				   		else if (py >0 && py <=9) {
							cf=pz + (.000900 + getrandom(.002700));
 						    atr_add(sdb[n].object, "BREAKDOWN", tprintf("%f",cf), GOD,(AF_MDARK + AF_WIZARD + AF_NOPROG));		 }	
				   		else {
							cf=pz + (.000900 + getrandom(.002700));
 						    atr_add(sdb[n].object, "BREAKDOWN", tprintf("%f",cf), GOD,(AF_MDARK + AF_WIZARD + AF_NOPROG));		 }	

   		//Lets free up some ram
		free((Malloc_t) qbreakdown);
		free((Malloc_t) zbreakdown);

					   
	return;
}

/*----------------------------------------------------------------------------------*/
void remove_blastdamage(void){
	int weaponhit,i;

				 	if ((sdb[n].engine.warp_damage ==1 && sdb[n].engine.impulse_damage ==1) &&  sdb[n].engine.lasthit != 999) {
					sdb[n].engine.lasthit=999;	}
					if ((sdb[n].sensor.lrs_damage ==1 && sdb[n].sensor.srs_damage ==1 && sdb[n].sensor.ew_damage ==1) &&  sdb[n].sensor.lasthit != 999) {
					sdb[n].sensor.lasthit=999;	}
				 	if ((sdb[n].shield.damage[0] ==1 && sdb[n].shield.damage[1] ==1 && sdb[n].shield.damage[2] ==1 && sdb[n].shield.damage[2] ==1 && sdb[n].shield.damage[3] ==1) &&  sdb[n].shield.lasthit != 999){	sdb[n].shield.lasthit=999;	}		
					if (sdb[n].cloak.damage ==1 &&  sdb[n].cloak.lasthit != 999) {
					sdb[n].cloak.lasthit=999;	}
				 	if (sdb[n].lifesupport.damage ==1 && sdb[n].lifesupport.lasthit != 999) {
					sdb[n].lifesupport.lasthit=999;	}	
				 	if (sdb[n].trans.damage ==1 &&  sdb[n].trans.lasthit != 999) {
					sdb[n].trans.lasthit=999;	}
				 	if (sdb[n].tract.damage ==1 &&  sdb[n].tract.lasthit != 999) {
					sdb[n].tract.lasthit=999;	}
				 	if (sdb[n].main.damage ==1 &&  sdb[n].main.lasthit != 999) {
					sdb[n].main.lasthit=999;	}
				 	if (sdb[n].aux.damage ==1 &&  sdb[n].aux.lasthit != 999) {
					sdb[n].aux.lasthit=999;	}
				 	if (sdb[n].batt.damage ==1 &&  sdb[n].batt.lasthit != 999) {
					sdb[n].batt.lasthit=999;	}
					if (sdb[n].beam.exist) {
						for (i = 0; i < sdb[n].beam.banks; ++i) {			
							if (sdb[n].blist.damage[i] <=.99) {
							weaponhit=1;
							break; }
						} }					
							if (weaponhit ==1 ) {
							sdb[n].beam.lasthit=sdb[n].beam.lasthit;
							}			
							else { 
								sdb[n].beam.lasthit=999;	}					
return; 

}
/*----------------------------------------------------------------------------------*/
void random_damage(void) {

		int randomvar=getrandom(11);
		int randomshield=getrandom(4),randomweapon,randomweapon1,i;
		char *shieldnamerandom; 
/*		sprintf(ship,"#%i",sdb[n].object);	
*/

//do_console_notify(n, console_engineering, 0, 0,ansi_alert(tprintf("BROKEN!! RANDOM %i\nShield %i",randomvar,randomshield)));

/* Ok we've read the point that something has to break
*   lets figure out what that sompething is 
*	Warp Drive Failure: Subtract 1Dl0 damage points from one warp engine; to
*	determine which engine, roll 1D10, with 1 - 5 being the port engine and
*	6 - 10 being the starboard en-gine. The warp engines are shut down until
*	repaired and/or balanced.
*	Impulse Engine Failure: Maximum warp speed re-duced by 1. No movement in
*	Starship Combat until the engine is repaired.
*	Computer Breakdown: No sensors, weapons, or shields. Warp and impulse
*	engines shut down until repaired.
*	Computer Brain Malfunction: All higher functions of the computer are
*	lost including Library Data, Computing, and Astrogation. All weapon fire
*	has an automatic +1 penalty.
*	Weapons Control Malfunction: All weapons are in-active until controls
*	are repaired.
*	Weapon Breakdown: One weapons subsystem (tor-pedo tube, phaser, or laser
*	bank, determined randomly) breaks down. That system must be repaired
*	before use.
*	Weapon Malfunction: One weapons sub-system (determine randomly, see
*	above) is reduced to half damage until system is repaired.
*	Deflector Control Malfunction: All deflector shields inactive until
*	controls are repaired.
*	Shield Generator Breakdown: One of the shield generators (determine
*	randomly) breaks down. That shield may not be raised until repaired.
*	Shield Generator Malfunction: One of the shield generators (determine
*	randomly) is reduced to half power until repaired.
*/

	if (randomvar ==0 && sdb[n].engine.warp_exist && sdb[n].engine.warp_damage != -1.0) { 
	  	do_console_notify(n, console_engineering, 0, 0,ansi_alert(tprintf("Warp Drive Failure")));  //Warp Drive Failure
		sdb[n].engine.warp_damage = -1.0; 		}
	if (randomvar ==1 && sdb[n].engine.impulse_exist && sdb[n].engine.impulse_damage != -1.0) {
	 	do_console_notify(n, console_engineering, 0, 0,ansi_alert(tprintf("Impulse Engine Failure"))); //Impulse Engine Failure
		sdb[n].engine.impulse_damage = -1.0; 		}
	if (randomvar ==2)	{
	do_console_notify(n, console_engineering, 0, 0,ansi_alert(tprintf("Computer Breakdown - All Systems disabled"))); //Computer Breakdown 
		//Disable the weapons
				if (sdb[n].beam.exist) {
				for (i = 0; i < sdb[n].beam.banks; ++i) {
					if (sdb[n].blist.damage[i] !=-1.0) {
					sdb[n].blist.damage[i]=-1.0;
					break; }

				} }
			if (sdb[n].missile.exist) {
				for (i = 0; i < sdb[n].missile.tubes; ++i) {
					if (sdb[n].mlist.damage[i] !=-1.0) {
					sdb[n].mlist.damage[i]=-1.0;
					break; }

				} }	
		//Disable the shields
			if (sdb[n].shield.damage[0] != -1.0){
				sdb[n].shield.damage[0] = -1.0;
								sdb[n].shield.active[0] = 0; }
			if (sdb[n].shield.damage[1] != -1.0){
				sdb[n].shield.damage[1] = -1.0;
								sdb[n].shield.active[1] = 0; }
			if (sdb[n].shield.damage[2] != -1.0){
				sdb[n].shield.damage[2] = -1.0; 
								sdb[n].shield.active[2] = 0; }
			if (sdb[n].shield.damage[3] != -1.0) {
				sdb[n].shield.damage[3] = -1.0;		
						 sdb[n].shield.active[3] = 0; }
		//Disable the engines
			sdb[n].engine.impulse_damage = -1.0; 
			sdb[n].engine.warp_damage = -1.0;
		//Disable the sensors
			sdb[n].sensor.ew_damage=-1.0;
			sdb[n].sensor.lrs_damage=-1.0;
			sdb[n].sensor.srs_damage=-1.0;
	}
	if (randomvar==3) {
		do_console_notify(n, console_engineering, 0, 0,ansi_alert(tprintf("Computer Brain Malfunction")));  //Computer Brain Malfunction

	}
	if (randomvar==4) {
	 	do_console_notify(n, console_engineering, 0, 0,ansi_alert(tprintf("Weapons Control Malfunction - All Weapons Disabled"))); //Weapons Control Malfunction
			if (sdb[n].beam.exist) {
				for (i = 0; i < sdb[n].beam.banks; ++i) {
					if (sdb[n].blist.damage[i] !=-1.0) {
					sdb[n].blist.damage[i]=-1.0;
					break; }

				} }
			if (sdb[n].missile.exist) {
				for (i = 0; i < sdb[n].missile.tubes; ++i) {
					if (sdb[n].mlist.damage[i] !=-1.0) {
					sdb[n].mlist.damage[i]=-1.0;
					break; }
				} }	
	}
	if (randomvar==5) {
		do_console_notify(n, console_engineering, 0, 0,ansi_alert(tprintf("Weapon Breakdown - 1 Weapon Disabled")));  //Weapon Breakdown
			randomweapon = sdb[n].beam.banks + 1;
			randomweapon1=getrandom(randomweapon);
					sdb[n].blist.damage[randomweapon1] =-1.0;
	}
	if (randomvar==6) {
		do_console_notify(n, console_engineering, 0, 0,ansi_alert(tprintf("Weapon Malfunction - 1 Weapon damaged 50%%")));  //Weapon Malfunction
			randomweapon = sdb[n].beam.banks + 1;
			randomweapon1=getrandom(randomweapon);
					sdb[n].blist.damage[randomweapon1] =.50;	
	}
	if (randomvar==7 && sdb[n].shield.exist && (sdb[n].shield.damage[0] != -1.0 || sdb[n].shield.damage[1] != -1.0 ||  sdb[n].shield.damage[2] != -1.0 || sdb[n].shield.damage[3] != -1.0)) {
		do_console_notify(n, console_engineering, 0, 0,ansi_alert(tprintf("Deflector Control Malfunction. All Shields Offline"))); //Deflector Control Malfunction
	if (sdb[n].shield.damage[0] != -1.0){
		sdb[n].shield.damage[0] = -1.0;
                        sdb[n].shield.active[0] = 0; }
	if (sdb[n].shield.damage[1] != -1.0){
		sdb[n].shield.damage[1] = -1.0;
                        sdb[n].shield.active[1] = 0; }
	if (sdb[n].shield.damage[2] != -1.0){
		sdb[n].shield.damage[2] = -1.0; 
                        sdb[n].shield.active[2] = 0; }
	if (sdb[n].shield.damage[3] != -1.0) {
		sdb[n].shield.damage[3] = -1.0;		
                 sdb[n].shield.active[3] = 0; }
	}
	if (randomvar==8 && sdb[n].shield.exist && sdb[n].shield.damage[randomshield] != -1.0) {
		if (randomshield== 0) 
			shieldnamerandom="Forward Shield";
		if (randomshield== 1) 
			shieldnamerandom="Aft Shield"; 
		if (randomshield== 2) 
			shieldnamerandom="Port Shield"; 
		if (randomshield== 3) {
			shieldnamerandom="Starboard Shield"; }
		do_console_notify(n, console_engineering, 0, 0,ansi_alert(tprintf("Shield Generator Breakdown. %s Offline.",shieldnamerandom))); //Shield Generator Breakdown
		sdb[n].shield.damage[randomshield] = -1.0;
                        sdb[n].shield.active[randomshield] = 0;
	}
	if (randomvar==9 && sdb[n].shield.exist && sdb[n].shield.damage[randomshield] >.51) {
		if (randomshield == 0)
			shieldnamerandom="Forward Shield";
		if (randomshield ==1)
			shieldnamerandom="Aft Shield";
		if (randomshield ==2)
			shieldnamerandom="Port Shield";
		if (randomshield ==3) {
			shieldnamerandom="Starboard Shield"; }
		do_console_notify(n, console_engineering, 0, 0,ansi_alert(tprintf("Shield Generator Malfunction. %s at 50%%",shieldnamerandom)));  //Shield Generator Malfunction
		sdb[n].shield.damage[randomshield] = .50;
	}
	if (randomvar ==10) {
		do_console_notify(n, console_engineering, 0, 0,ansi_alert(tprintf("Random Breakdown...Damage Averted...Saving Roll Made"))); //Saving Roll
	}	


	
	return;
}
/* ---------------------------IDLE CODE editid by bill------------------------ */
int do_space_db_iterate (void)
{
	register int count = 0;
	int px,nn;
	double odometer;
	ATTR *aiterate,*biterate,*citerate;
	float py,pz;
	char *qiterate,*ziterate,*xiterate;
	//start mysql vars
	MYSQL *conn;
	MYSQL_RES *res_set;
	//end mysql vars
	dbref ship;
	
	for (n = MIN_SPACE_OBJECTS; n <= max_space_objects; ++n) {


		if ((sdb[n].status.active && sdb[n].structure.type)) {
				++count;
				sdb[n].move.dt = mudtime - sdb[n].move.time;
				sdb[n].move.time = mudtime;
				if (sdb[n].move.dt > 0.0) {
					if (sdb[n].structure.type == 1)
						if (sdb[n].move.time - sdb[n].status.time > 86000) {
							if (sdb[n].main.in > 0.0) {
								do_set_main_reactor(0.0, sdb[n].object);
								sdb[n].main.in = 0.0;
							}
							if (sdb[n].aux.in > 0.0) {
								do_set_aux_reactor(0.0, sdb[n].object);
								sdb[n].aux.in = 0.0;
							}
							if (sdb[n].batt.in > 0.0) {
								do_set_battery(0.0, sdb[n].object);
								sdb[n].batt.in = 0.0;
							}
							if (sdb[n].power.total == 0.0) {
								do_set_inactive(sdb[n].object);
							}
						}
					if (sdb[n].move.dt > 60.0)
						sdb[n].move.dt = 60.0;
					if(sdb[n].alloc.version)
						up_alloc_balance();
					if (sdb[n].main.out != sdb[n].main.in)
						up_main_io();
					if (sdb[n].aux.out != sdb[n].aux.in)
						up_aux_io();
					if (sdb[n].batt.out != sdb[n].batt.in)
						up_batt_io();
					if (sdb[n].main.out > 0.0)
						if (sdb[n].main.out > sdb[n].main.damage)
							up_main_damage();
					if (sdb[n].aux.out > 0.0)
						if (sdb[n].aux.out > sdb[n].aux.damage)
							up_aux_damage();
					if (sdb[n].power.main > 0.0 || sdb[n].power.aux > 0.0)
						up_fuel();
					if (sdb[n].power.batt > 0.0 || sdb[n].alloc.miscellaneous > 0.0)
						up_reserve();
					if (sdb[n].power.version)
						up_total_power();
						up_tract_status();
					if (sdb[n].beam.in != sdb[n].beam.out)
						up_beam_io();
					if (sdb[n].missile.in != sdb[n].missile.out)
						up_missile_io();
					if (sdb[n].engine.version) {
						up_warp_max();
						up_impulse_max();
						up_turn_rate();
						sdb[n].engine.version = 0;
					}
					if (sdb[n].status.autopilot)
						up_autopilot();
					if (sdb[n].move.in != sdb[n].move.out) {
						up_speed_io();
						up_velocity();
						up_turn_rate();
					}							
								
					/* Lets check the to see if life support went offline */
						check_lifesupport();
					/* Lets check for the Damage to be repaired to remove the blast traces */
						remove_blastdamage();
					/*End Damage repair check */
					//Check for warp core ejection
						check_warpeject();
					//end warp core ejection check
					//Lets check the Components Breakdown status
						check_component_breakdown();
					//END check the components breakdown status
				

			/************************ Check for random breakdown *******************************************/
		   aiterate = atr_get(sdb[n].object, "BREAKDOWN");
		   biterate =atr_get(sdb[n].object, "ODOMETER");
		   citerate =atr_get(sdb[n].object, "CF");

		   if (biterate == NULL) {
			atr_add(sdb[n].object, "ODOMETER", tprintf("0.000000"), GOD,(AF_MDARK + AF_WIZARD + AF_NOPROG));																   
		   }
		   if (aiterate == NULL){				   
					//* Lets call the breakdown routine 
		   			set_breakdown();
				}								
		   if (aiterate !=NULL){
						qiterate = safe_uncompress(aiterate->value);
						py=atof(qiterate);
		   }
		   if (biterate !=NULL){
						ziterate = safe_uncompress(biterate->value);
						pz=atof(ziterate);
		   }
		   if (citerate !=NULL){
   						xiterate = safe_uncompress(citerate->value);
						px=atoi(xiterate) - 1;
		   }
				
						//*do_ship_notify(n, tprintf("BREAKDOWN - %f\nODOMETER -%f",py,pz));		
	  if (pz==py) {  			
		 	// Run Random Damage Routines & Decrease CF Factor
		 	random_damage();
			atr_add(sdb[n].object, "CF", tprintf("%i",px), GOD,(AF_MDARK + AF_WIZARD + AF_NOPROG));			
		 	// Reset the Breakdowntime --Resets the breakdown meter
			set_breakdown();			
		   }
	
  		//Lets free up some ram
		free((Malloc_t) qiterate);
		free((Malloc_t) ziterate);
		free((Malloc_t) xiterate);



			/************************* End Random Breakdowncheck ********************************************/

		//****************************CHECK FOR NEBULAS*****************************************************
	   // THIS ROUTINE WILL ONLY RUN IF THIS IS A SHIP
	  if (sdb[n].structure.type == 1){
			//do_ship_notify(n, tprintf("IN THE FIRST CHECK\n"));	
			for (nn = MIN_SPACE_OBJECTS; nn <= max_space_objects; ++nn)
			//do_ship_notify(n, tprintf("THIS IS THE %i",nn));	
				// We only want this to run on nebulas
				if (sdb[nn].structure.type == 6 && sdb[nn].space==sdb[n].space){
			up_nebula(nn);
			//do_ship_notify(n, tprintf("THIS IS THE %i",billtest));
			}
	  }
		//*****************************END NEBULA CHECK *****************************************************

	
		//*****************************START SELF DESTRUCT CHECK*****************************************************
			self_destruct_count();
  		//*****************************END SELF DESTRUCT CHECK*****************************************************


						if (sdb[n].move.in > 0.0) {				
							/*Add Odometer */		
						  	aiterate = atr_get(sdb[n].object, "ODOMETER");
							if (aiterate != NULL) {
								qiterate = safe_uncompress(aiterate->value);
								py=atof(qiterate) + .000001;
								/*printf("%d SHIP - ATTR %f\n", sdb[n].object,py); COMMENTED OUT, UNCOMMENT TO DEBUG */			
					/* Increment the Odometer */
					atr_add(sdb[n].object, "ODOMETER", tprintf("%f",py), GOD,(AF_MDARK + AF_WIZARD + AF_NOPROG));

					
					/* Lets	do the component check */

						//do_ship_notify(n, tprintf("shipdbref %s",unparse_dbref(sdb[n].object)));

						conn = mysql_init (NULL);
						mysql_real_connect(conn,"localhost.localdomain","root","pw","db",0,NULL,0);

					   if (!conn) {
						 //do_ship_notify(n, tprintf("No Sql Connection "));
					   }
					   if (conn) {
						//do_ship_notify(n, tprintf("Sql database Connection"));
						//mysql_real_query(mysql, query, strlen(query));
						
						//Lets run the query
						   ship=sdb[n].object;
						   if (mysql_query (conn, tprintf("select active,odometer,recordid from components where shipdbref='%s'",unparse_dbref(ship))) != 0) {
							do_ship_notify(n, tprintf("Query Failed"));
						   }
						else {

							//do_ship_notify(n, tprintf("I'm in nigga!"));

							res_set = mysql_store_result (conn); //Generate Result Set
							if (res_set==NULL) {
								//do_ship_notify(n, tprintf("Mysql_store_result()"));	
							}
							else {
								//Process result set, then deallocate it 
								component_odometer (conn, res_set,n);
								mysql_free_result(res_set);
							}

						}

						//qres = mysql_store_result(mysql);
						//field = mysql_fetch_field(qres);
					   }	
						
						mysql_close(conn);
						/* End Component Check */



				//Lets free up some ram
				free((Malloc_t) qiterate);

							}
							else 
					/* Increment the Odometer */
					atr_add(sdb[n].object, "ODOMETER", tprintf("0.000000"), GOD,(AF_MDARK + AF_WIZARD + AF_NOPROG));														
					}
				
				//************************************START COLLISION CODE********************************************
	   				 if (sdb[n].structure.type == 1 && sdb[n].move.out != 0.0){
						//do_ship_notify(1, tprintf("SHIP %i",n));
						 check_space_collision();
					  }					
				//************************************END COLLISION CODE********************************************

					if (sdb[n].move.out != 0.0) {				
						up_warp_damage();
						up_impulse_damage();
					}
					if (sdb[n].course.yaw_in != sdb[n].course.yaw_out)
						up_yaw_io();
					if (sdb[n].course.pitch_in != sdb[n].course.pitch_out)
						up_pitch_io();
					if (sdb[n].course.roll_in != sdb[n].course.roll_out)
						up_roll_io();
					if (sdb[n].course.version)
						up_vectors();
					if (sdb[n].move.v != 0.0) {
						up_position();
						up_cochranes();
						up_empire();
						up_quadrant();
						up_visibility();
					}
					if (sdb[n].cloak.version)
						up_cloak_status();
					if (sdb[n].sensor.version) {
						up_resolution();
						up_signature(n);
					}
					up_sensor_list();
					if (sdb[n].structure.repair != sdb[n].structure.max_repair)
						up_repair();
				}
		}
		//Lets Do the self destruct of the warp core
		if(sdb[n].structure.type==7) {
			self_destruct_count();
		}
}

	return count;
}

/* ------------------------------------------------------------------------ */
// This is the Check Inside Routine!
int check_inside (double aax, double aay, double aaz, double myrad,int secondship)
{
	
	double dx, dy, dz;

		//Lets  
		// This takes the X Y Z values of the PASSED VALUES and
		// divides them by the parsec to find the centers
		dx = (aax - sdb[secondship].coords.x) / PARSEC;
		dy = (aay - sdb[secondship].coords.y) / PARSEC;
		dz = (aaz - sdb[secondship].coords.z) / PARSEC;
	
		//Testing Code - Uncomment to test
/*		do_ship_notify(1, tprintf("SHIP %i",secondship));	
		do_ship_notify(1, tprintf("X %d Y %d Z %d",dx,dy,dz));	
		do_ship_notify(1, tprintf("COORDS %d",dx*dx+dy*dy+dz*dz));	
		do_ship_notify(1, tprintf("DISPLACEMENT3 %d\n-------------------------------\n",su2pc(myrad) * su2pc(myrad)));
		do_ship_notify(1, tprintf("MYRAD %d INPC %d\n-------------------------------\n",myrad, su2pc(myrad)));
		*/




		if ((dx * dx + dy * dy + dz * dz) < (su2pc(myrad) *  su2pc(myrad))) {
			//Return 1 if they are inside 
			return 1;				
		}
		else {
			//Return 0 if they are outside
			return 0;
		}

}
/* ------------------------------------------------------------------------ */

void self_destruct_count (void) {		
	int px,nn,py,t,inside;
	double odometer;
	ATTR *aselfdestruct,*bselfdestruct,*cselfdestruct;
	char *qselfdestruct,*zselfdestruct,*yselfdestruct;


	
					aselfdestruct = atr_get(sdb[n].object, "SELFDESTRUCTTIMER");
					cselfdestruct = atr_get(sdb[n].object, "SELFDESTRUCTSILENT");
				if (aselfdestruct != NULL) {
					//selfdestructsilent
					zselfdestruct = safe_uncompress(cselfdestruct->value);
					px=atoi(zselfdestruct);
					//Current Click on the Timer
					yselfdestruct = safe_uncompress(aselfdestruct->value);
					nn=atoi(yselfdestruct);
					//do_ship_notify(n, ansi_alert(tprintf("TIME TO DEATH %i",nn)));	
					//We Only want to start checking if the timer has moved
					if (nn > 0) {
						//we only want to send an audible message if they ask
						if (px == 1) {
							if (nn == 600){
							do_ship_notify(n, ansi_alert(tprintf("Woman voice says \"Self Destruct in 10 minutes\"")));	
							} 
							if (nn == 540){
							do_ship_notify(n, ansi_alert(tprintf("Woman voice says \"Self Destruct in 9 minutes\"")));	
							} 
							if (nn == 480){
							do_ship_notify(n, ansi_alert(tprintf("Woman voice says \"Self Destruct in 8 minutes\"")));	
							} 
							if (nn == 420){
							do_ship_notify(n, ansi_alert(tprintf("Woman voice says \"Self Destruct in 7 minutes\"")));	
							} 
							if (nn == 360){
							do_ship_notify(n, ansi_alert(tprintf("Woman voice says \"Self Destruct in 6 minutes\"")));	
							} 
							if (nn == 300){
							do_ship_notify(n, ansi_alert(tprintf("Woman voice says \"Self Destruct in 5 minutes\"")));	
							} 
							if (nn == 240){
							do_ship_notify(n, ansi_alert(tprintf("Woman voice says \"Self Destruct in 4 minutes\"")));	
							} 
							if (nn == 180){
							do_ship_notify(n, ansi_alert(tprintf("Woman voice says \"Self Destruct in 3 minutes\"")));	
							} 
							if (nn == 120){
							do_ship_notify(n, ansi_alert(tprintf("Woman voice says \"Self Destruct in 2 minutes\"")));	
							} 
							if (nn == 60){
							do_ship_notify(n, ansi_alert(tprintf("Woman voice says \"Self Destruct in 60 Seconds\"")));	
							} 
							if (nn == 30){
							do_ship_notify(n, ansi_alert(tprintf("Woman voice says \"Self Destruct in 30 Seconds. Abort No Longer Possible\"")));	
							} 
						}
							//LETS KILL THEM NIGGA!
							if (nn == 1) {
							do_ship_notify(n, ansi_alert(tprintf("Woman voice says \"Self Destruct Initiated.\"")));	

							do_ship_notify(n, tprintf("%s%s%s%s rumbles loudly, you hear the hiss of space then it all explodes into white hot vapor.%s%s",
							  ANSI_HILITE, ANSI_INVERSE, ANSI_RED, Name(sdb[n].object), ANSI_WHITE, ANSI_NORMAL));
							do_log(LT_SPACE, sdb[n].object, sdb[n].object, tprintf("LOG: Self Destructed! , Shields %.6f GHz",
							  sdb[n].shield.freq));
							do_space_notify_one(n, console_helm, console_tactical, console_science,
									"has self destructed");

							//When we die everyone else dies! :)
							//that are close to the dead ship let me fix 
							for (t = MIN_SPACE_OBJECTS ; t <= max_space_objects ; ++t) {
										//do_ship_notify(1, tprintf("%i X%f Y %f Z%f",t,sdb[t].coords.x,sdb[t].coords.y,sdb[t].coords.z));
										//do_ship_notify(1, tprintf("ID %i\nStructure Type:%i\nSpace %i\n",t,sdb[t].structure.type,sdb[t].space));
								
										//We only want to kill ships or bases!
								if (((sdb[t].structure.type == 1 || sdb[t].structure.type ==2)) && sdb[t].space == sdb[n].space && t != n) {
											inside= check_inside(sdb[n].coords.x,sdb[n].coords.y,sdb[n].coords.z,SELF_DESTRUCT_RANGE,t);
											
											if (inside == 1) {
													//do_ship_notify(1, tprintf("YUP %i",i));
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
							//Lets kill the ship off
							sdb[n].space = -1;
							sdb[n].status.active = 0;
							sdb[n].status.crippled = 2;	

							
							} 	

							//INCREMENT THE DEATH TIMER DOWN 1
							atr_add(sdb[n].object, "SELFDESTRUCTTIMER", tprintf("%i",nn - 1), GOD,(AF_MDARK + AF_WIZARD + AF_NOPROG));					

							
		//Lets free up some ram
		free((Malloc_t) zselfdestruct);
		free((Malloc_t) yselfdestruct);



					}	
				}


}

//*************************CHECK FOR WARP CORE BREECHS******************************************//
void check_warpeject(void) {
	ATTR *awarpcore;
	char *qwarpcore;
	char *objectwarpcore,*flagwarpcore;
	dbref warpcore;
	int nn,objectid;
	double corehead;

	awarpcore = atr_get(sdb[n].object, "WARPCOREEJECT");
		if (awarpcore != NULL) {
			qwarpcore = safe_uncompress(awarpcore->value);
			nn=atoi(qwarpcore);

			//do_ship_notify(n, tprintf("NN %i",nn));

				if (nn == 1){
				do_console_notify(n, console_tactical, console_science, console_helm,
				  ansi_alert(tprintf("Woman voice says \"Warp Core seperation in 10 secs.\"")));
				} 
				else if (nn == 2){
				do_console_notify(n, console_tactical, console_science, console_helm,
				  ansi_alert(tprintf("Woman voice says \"Warp Core seperation in 9 secs.\"")));
				} 
				else if (nn == 3){
				do_console_notify(n, console_tactical, console_science, console_helm,
				  ansi_alert(tprintf("Woman voice says \"Warp Core seperation in 8 secs.\"")));
				} 
				else if (nn == 4){
				do_console_notify(n, console_tactical, console_science, console_helm,
				  ansi_alert(tprintf("Woman voice says \"Warp Core seperation in 7 secs.\"")));
				} 
				else if (nn == 5){
				do_console_notify(n, console_tactical, console_science, console_helm,
				  ansi_alert(tprintf("Woman voice says \"Warp Core seperation in 6 secs.\"")));
				} 
				else if (nn == 6){
				do_console_notify(n, console_tactical, console_science, console_helm,
				  ansi_alert(tprintf("Woman voice says \"Warp Core seperation in 5 secs.\"")));
				} 
				else if (nn == 7) {
				do_console_notify(n, console_tactical, console_science, console_helm,
				  ansi_alert(tprintf("Woman voice says \"Warp Core seperation in 4 secs.\"")));
				} 
				else if (nn == 8) {
				do_console_notify(n, console_tactical, console_science, console_helm,
				  ansi_alert(tprintf("Woman voice says \"Warp Core seperation in 3 secs.\"")));
				} 
				else if (nn == 9) {
				do_console_notify(n, console_tactical, console_science, console_helm,
				  ansi_alert(tprintf("Woman voice says \"Warp Core seperation in 2 secs.\"")));
				} 
				else if (nn == 10 ){
				do_console_notify(n, console_tactical, console_science, console_helm,
				  ansi_alert(tprintf("Woman voice says \"Warp Core seperation in 1 secs.\"")));
				} 
				else if (nn == 11) {
				do_ship_notify(n, tprintf("You feel the surface beneath shake beneath your feet and then smoothness.\n
				%s%sComputer Says: \"Warp Core Ejection Complete\"%s%s",
				ANSI_HILITE, ANSI_RED, ANSI_WHITE, ANSI_NORMAL));		
				
				do_console_notify(n, console_tactical, console_science, console_helm,
				  ansi_alert(tprintf("Woman voice says \"Warp Core ejected, Now on Impulse power only.\"")));
				//Lets reset the warp engine to be off
				sdb[n].move.in=0.0;
				sdb[n].move.out=0.0;
				do_set_main_reactor(0.0, sdb[n].object);
				sdb[n].engine.warp_exist=0;
				sdb[n].main.exist=0;
				atr_add(sdb[n].object, "WARPCOREEJECT", tprintf("0"), GOD,(AF_MDARK + AF_WIZARD + AF_NOPROG));
				
				//Lets create the OBJECT
				objectwarpcore="Warp Core";
				warpcore=do_create_internal(5,objectwarpcore,0);
				// Lets parent the warp core to the Warpcore parent
				Parent(warpcore) = WARP_CORE_PARENT;
				// Lets set the SPACE_OBJECT flag on the obj now
				flagwarpcore="SPACE_OBJECT";
				set_flag(5,warpcore,flagwarpcore,0,0,0);
				//Now lets space this baby
				do_space_db_read(warpcore,5);
				objectid=db2sdb(warpcore);
				//lets set the space and coords right
		sdb[objectid].coords.x=sdb[n].coords.x - 5;
		sdb[objectid].coords.y=sdb[n].coords.y - 5;
		sdb[objectid].coords.z=sdb[n].coords.z - 5;
		//Lets make sure we put the core in the same space as the shipt
		sdb[objectid].space=sdb[n].space;
		//sdb[objectid].space=1;
		sdb[objectid].move.in=.90;
		sdb[objectid].move.v=.90;
		sdb[objectid].move.out=.90;
		corehead=sdb2angular(n,objectid);

				//do_ship_notify(n, tprintf("ANGLE %d",corehead));

				//Lets let all the other ships in the area know
				do_space_notify_one(n, console_helm, console_tactical, console_science,
				"has ejected its warpcore. The warp core begins to become unstable");
				//Start the Self Destruct
				do_selfdestruct(objectid,1,180);

				
				}
			/*	else if (nn == 20 ){
				do_console_notify(n, console_tactical, console_science, console_helm,
				  ansi_alert(tprintf("The Warp Core pulses and grows closer to exploding.")));
				}*/ 
				else if (nn == 30 ){
				//Lets let all the other ships in the area know
				} 
				/*else if (nn == 45 ){
				do_console_notify(n, console_tactical, console_science, console_helm,
				  ansi_alert(tprintf("BANG!")));
				atr_add(sdb[n].object, "WARPCOREEJECT", tprintf("0"), GOD,(AF_MDARK + AF_WIZARD + AF_NOPROG));
				} 
			*/

				atr_add(sdb[n].object, "WARPCOREEJECT", tprintf("%i",nn + 1), GOD,(AF_MDARK + AF_WIZARD + AF_NOPROG));					
		
				//Lets free up some ram
				free((Malloc_t) qwarpcore);
		}
}
/************************END WARP CORE BREACH CHECKING *******************************************/							

//*************************CHECK FOR SPACE COLLISIONS******************************************//
void check_space_collision(void) {
	int t,inside;

for (t = MIN_SPACE_OBJECTS ; t <= max_space_objects ; ++t) {
				//do_ship_notify(1, tprintf("%i X%f Y %f Z%f",t,sdb[t].coords.x,sdb[t].coords.y,sdb[t].coords.z));
								
				//We only wwant to blow up objects that should die
				if ((sdb[t].structure.type == 1 || sdb[t].structure.type == 2 || sdb[t].structure.type == 3 || sdb[t].structure.type == 5) && sdb[t].space == sdb[n].space && t != n) {
							inside= check_inside(sdb[n].coords.x,sdb[n].coords.y,sdb[n].coords.z,COLLISION_RANGE,t);
							
							if (inside == 1) { 
								//do_ship_notify(1, tprintf("YUP %i",t));
								do_ship_notify(n, tprintf("%s%s%s%s has collided with %s and died..%s%s",
								  ANSI_HILITE, ANSI_INVERSE, ANSI_RED, Name(sdb[t].object), Name(sdb[n].object), ANSI_WHITE, ANSI_NORMAL));

								do_ship_notify(t, tprintf("%s%s%s%s has collided with %s and died..%s%s",
								  ANSI_HILITE, ANSI_INVERSE, ANSI_RED, Name(sdb[t].object), Name(sdb[n].object), ANSI_WHITE, ANSI_NORMAL));

								do_log(LT_SPACE, sdb[t].object, sdb[n].object, tprintf("LOG: %s has collided with %s",
									  Name(sdb[t].object),Name(sdb[n].object)));
									//Lets just cripple them for now
									//sdb[t].space = -1;
									//sdb[t].status.active = 0;
									//sdb[t].structure.superstructure = -50;
									//sdb[t].status.crippled = 1;	
									
									sdb[n].space = -1;
									sdb[n].status.active = 0;
									sdb[n].status.crippled = 2;

									sdb[t].space = -1;
									sdb[t].status.active = 0;
									sdb[t].status.crippled = 2;

									
							}
				}
	}
}
/************************END COLLISION CODE *******************************************/							


/************************COMPONENT ODOMETER CODE***************************************/
void component_odometer (MYSQL *conn, MYSQL_RES *res_set,int x) {
MYSQL_ROW	row;
MYSQL_FIELD *field;
unsigned int	i;
int recordid,active;

	while((row=mysql_fetch_row (res_set)) != NULL)
	{

		mysql_field_seek (res_set,0);
		for (i = 0; i < mysql_num_fields (res_set); i++)
		{
			field=mysql_fetch_field (res_set);

			//Lets get the recordid
			if (strcmp(field->name,"recordid") ==0) {
				recordid=atoi(row[i]);
			}			
			//Lets get the recordid
			if (strcmp(field->name,"active") ==0) {
				active=atoi(row[i]);
			}			
							

			//Lets increament the odometer only if the ship is active
			if (active ==1 ) {
				mysql_query (conn, tprintf("update components set odometer=odometer+1 where recordid='%i'",recordid));
			}
				
		}
			recordid="0";
			active="0";
		//notify(executor, tprintf("\n"));	
	}
/*	if (mysql_errno (conn) !=0)
			notify(executor, "mysql_Fetch_row() Failed");
	else
			notify(executor, tprintf("%lu rows returned\n",(unsigned long) mysql_num_rows (res_set)));
*/	
 }
/********************END COMPONENT ODOMETER CODE***************************************/


/************************COMPONENT ODOMETER CODE***************************************/
void check_component_breakdown (void) {
MYSQL *conn;
MYSQL_RES *res_set;
MYSQL_ROW	row;
MYSQL_FIELD *field;
unsigned int	i;
int recordid,odometer,breakdown,quality,cf;
char *componentname;


/* Lets	do the component check */
	conn = mysql_init (NULL);
	mysql_real_connect(conn,"localhost.localdomain","root","pw","db",0,NULL,0);

	if (!conn) { /*notify(executor, "No SQL database connection.");*/	}
   if (conn) {	/*notify(executor, "SQL database connection.");*/
	//Lets run the query
	   if (mysql_query (conn, tprintf("select * from components where shipdbref='%s'",unparse_dbref(sdb[n].object))) != 0){
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

							//do_ship_notify(1, tprintf("%i X%f Y %f Z%f",t,sdb[t].coords.x,sdb[t].coords.y,sdb[t].coords.z));
							mysql_field_seek (res_set,0);
							for (i = 0; i < mysql_num_fields (res_set); i++)
							{
								field=mysql_fetch_field (res_set);
									
								//Lets get the recordid
								if (strcmp(field->name,"recordid") ==0) {
									recordid=atoi(row[i]);
								}		
								//Lets get the recordid
								if (strcmp(field->name,"name") ==0) {
									componentname=row[i];
								}								
								//Lets get the Quality
								if (strcmp(field->name,"quality") ==0) {
									quality=atoi(row[i]);
								}								
								//Lets get the cf
								if (strcmp(field->name,"cf") ==0) {
									cf=atoi(row[i]);
								}								
								//Lets get the odometer reading for the component
								if (strcmp(field->name,"odometer") ==0) {
									odometer=atoi(row[i]);
								}			
								//Lets get the breakdown reading for the component
								if (strcmp(field->name,"breakdown") ==0) {
									breakdown=atoi(row[i]);
								}			
								//Lets check to see if the component should break
								if (breakdown==odometer) {
									//Lets reset the odometer and increment the cf
	  								do_console_notify(n, console_engineering, 0, 0,ansi_alert(tprintf("Component: %s has failed",componentname)));  //Let everyone know something broke				
									
									if (quality == 10) {   //3600 * 24
										breakdown=(86400 + getrandom(518400)); 
									}
									else if (quality == 9) { //3600 * 22
										breakdown=(79200 + getrandom(475200));
									}
									else if (quality == 8) { //3600 * 20 
										breakdown=(72000 + getrandom(432000));
									}
									else if (quality == 7) { //3600 * 18 
										breakdown=(64800 + getrandom(388800));
									}
									else if (quality == 6) { //3600 * 16 
										breakdown=(57600 + getrandom(315600));
									}
									else if (quality == 5) { //3600 * 14 
										breakdown=(50400 + getrandom(302400));
									}
									else if (quality == 4) { //3600 * 12 
										breakdown=(43200 + getrandom(259200));
									}
									else if (quality == 3) { //3600 * 10 
										breakdown=(36000 + getrandom(216000));
									}
									else if (quality == 2) { //3600 * 4 
										breakdown=(14400 + getrandom(86400));
									}
									else if (quality == 1) { //3600 * 1
										breakdown=(3600 + getrandom(21600));
									}
									else {
										breakdown=(3600 + getrandom(21600));
									}									
									mysql_query (conn, tprintf("update components set odometer=0,damaged=1,cf=cf-1,remove=1,breakdown=%i where recordid='%i'",breakdown,recordid));
									//Now lets make sure the component is removed
									do_space_db_write(sdb[n].object,GOD);
								}																	
							}							
							componentname="";
							odometer="0";
							breakdown="1";
							recordid="0";
							quality="";							
							//notify(executor, tprintf("\n"));	
						}
				//End the actual processing of the errors
			mysql_free_result(res_set);
		}
	}
   }			
	
	mysql_close(conn);
	/* End Component Check */
	
 }
/********************END COMPONENT ODOMETER CODE***************************************/
