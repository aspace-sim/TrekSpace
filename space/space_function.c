/* ------------------------------------------------------------------------ */

 extern char *crypt_code (char *code, char *text, int type); 

/* ------------------------------------------------------------------------ */

/* ARGSUSED */
FUNCTION(fun_sdb) /* sdb (<function>[,<field>[,<field>[,<field>[,<field>[,...]]]]]) */
{
	dbref ship, console;
	int s, x, sh;
	double a;
	static char blank[] = "   \0";

	if (!Wizard(executor)) {
		/* Mordak Aspace v1.0.0p1 Removed Cursing from a major function call*/
		safe_str("#-1 Permission Denied", buff, bp);
		/* End Aspace v1.0.0p1 */
		return;
	}

	for (x = 0; x < 7; ++x)
		if (args[x] == NULL)
			args[x] = blank;
	switch (args[0][0]) {
		case 'd': /* do */
			n = parse_integer(args[1]);
			if (!GoodSDB(n)) {
				safe_str("#-1 SDB OUT OF RANGE", buff, bp); return; }
			switch (parse_integer(args[2])) {
				case  0: safe_str(unparse_integer(debug_space(n)), buff, bp); break;
				case  1: safe_str(unparse_integer(do_set_main_reactor(parse_number(args[3]), enactor)), buff, bp); break;
				case  2: safe_str(unparse_integer(do_set_aux_reactor(parse_number(args[3]), enactor)), buff, bp); break;
				case  3: safe_str(unparse_integer(do_set_battery(parse_number(args[3]), enactor)), buff, bp); break;
				case  4: safe_str(unparse_integer(do_set_course(parse_number(args[3]), parse_number(args[4]), parse_number(args[5]), enactor)), buff, bp); break;
				case  5: safe_str(unparse_integer(do_set_speed(parse_number(args[3]), enactor)), buff, bp); break;
				case  6: safe_str(unparse_integer(do_set_eng_alloc(parse_number(args[3]), parse_number(args[4]), parse_number(args[5]), enactor)), buff, bp); break;
				case  7: safe_str(unparse_integer(do_set_helm_alloc(parse_number(args[3]), parse_number(args[4]), parse_number(args[5]), enactor)), buff, bp); break;
				case  8: safe_str(unparse_integer(do_set_shield_alloc(parse_number(args[3]), parse_number(args[4]), parse_number(args[5]), parse_number(args[6]), enactor)), buff, bp); break;
				case  9: safe_str(unparse_integer(do_set_tactical_alloc(parse_number(args[3]), parse_number(args[4]), parse_number(args[5]), enactor)), buff, bp); break;
				case 10: safe_str(unparse_integer(do_set_sensor_alloc(parse_number(args[3]), parse_number(args[4]), enactor)), buff, bp); break;
				case 11: safe_str(unparse_integer(do_set_operations_alloc(parse_number(args[3]), parse_number(args[4]), parse_number(args[5]), enactor)), buff, bp); break;
				case 12: safe_str(unparse_integer(do_set_active(enactor)), buff, bp); break;
				case 13: safe_str(unparse_integer(do_set_inactive(enactor)), buff, bp); break;
				case 14: safe_str(unparse_integer(do_set_dock(parse_integer(args[3]), enactor)), buff, bp); break;
				case 15: safe_str(unparse_integer(do_set_undock(enactor)), buff, bp); break;
				case 16: safe_str(unparse_integer(do_set_shield(parse_integer(args[3]), parse_integer(args[4]), enactor)), buff, bp); break;
				case 17: safe_str(unparse_integer(do_set_trans(parse_integer(args[3]), enactor)), buff, bp); break;
				case 18: safe_str(unparse_integer(do_set_weapon(parse_integer(args[3]), parse_integer(args[4]), parse_integer(args[5]), parse_integer(args[6]), enactor)), buff, bp); break;
				case 19: safe_str(unparse_integer(do_set_trans_locked(parse_integer(args[3]), parse_integer(args[4]), enactor)), buff, bp); break;
				case 20: safe_str(unparse_integer(do_set_autopilot(parse_integer(args[3]), enactor)), buff, bp); break;
				case 21: safe_str(unparse_integer(do_set_trans_unlocked(enactor)), buff, bp); break;
				case 22: safe_str(unparse_integer(do_lock_weapon(parse_integer(args[3]), parse_integer(args[4]), parse_integer(args[5]), parse_integer(args[6]), enactor)), buff, bp); break;
				case 23: safe_str(unparse_integer(do_unlock_weapon(parse_integer(args[3]), parse_integer(args[4]), parse_integer(args[5]), enactor)), buff, bp); break;


				case 26: safe_str(unparse_integer(do_set_lrs(parse_integer(args[3]), enactor)), buff, bp); break;
				case 27: safe_str(unparse_integer(do_set_ftr_alloc(parse_number(args[3]), parse_number(args[4]), parse_number(args[5]), enactor)), buff, bp); break;
				case 28: safe_str(unparse_integer(do_set_evade(parse_integer(args[3]), enactor)), buff, bp); break;
				case 29: safe_str(unparse_integer(do_set_srs(parse_integer(args[3]), enactor)), buff, bp); break;
				case 30: safe_str(unparse_integer(do_set_ew(parse_integer(args[3]), enactor)), buff, bp); break;
				case 31: safe_str(unparse_integer(do_set_tract(parse_integer(args[3]), enactor)), buff, bp); break;
				case 32: safe_str(unparse_integer(do_set_cloak(parse_integer(args[3]), enactor)), buff, bp); break;
				case 33: safe_str(unparse_integer(do_set_eta(parse_number(args[3]), enactor)), buff, bp); break;
				case 34: safe_str(unparse_integer(do_set_land(parse_integer(args[3]), enactor)), buff, bp); break;
				case 35: safe_str(unparse_integer(do_set_launch(enactor)), buff, bp); break;
				case 36: safe_str(unparse_integer(do_set_enter_wormhole(parse_integer(args[3]), enactor)), buff, bp); break;
				case 37: safe_str(unparse_integer(do_set_coords_manual(parse_number(args[3]), parse_number(args[4]), parse_number(args[5]), enactor)), buff, bp); break;
				case 38: safe_str(unparse_integer(do_set_coords_reset(enactor)), buff, bp); break;
				case 39: safe_str(unparse_integer(do_set_fix_damage(args[3], args[4], parse_integer(args[5]), args[6], enactor)), buff, bp); break;
				case 40: safe_str(unparse_integer(do_set_intercept(parse_integer(args[3]), enactor)), buff, bp); break;
				case 41: safe_str(unparse_integer(do_set_coords_layin(parse_number(args[3]), parse_number(args[4]), parse_number(args[5]), enactor)), buff, bp); break;
				case 42: safe_str(unparse_integer(do_set_shield_freq(parse_number(args[3]), enactor)), buff, bp); break;
				case 43: safe_str(unparse_integer(do_set_cloak_freq(parse_number(args[3]), enactor)), buff, bp); break;
				case 44: safe_str(unparse_integer(do_set_beam_freq(parse_number(args[3]), enactor)), buff, bp); break;
				case 45: safe_str(unparse_integer(do_set_missile_freq(parse_number(args[3]), enactor)), buff, bp); break;
				case 46: safe_str(unparse_integer(do_set_trans_freq(parse_number(args[3]), enactor)), buff, bp); break;
				case 47: safe_str(unparse_integer(do_set_tract_freq(parse_number(args[3]), enactor)), buff, bp); break;
				case 48: safe_str(unparse_integer(do_set_tract_locked(parse_integer(args[3]), enactor)), buff, bp); break;
				case 49: safe_str(unparse_integer(do_set_tract_unlocked(enactor)), buff, bp); break;
				case 50: safe_str(unparse_integer(do_set_refuel(args[3], args[4], parse_number(args[5]), enactor)), buff, bp); break;
				case 51: safe_str(unparse_integer(do_set_defuel(args[3], parse_number(args[4]), enactor)), buff, bp); break;
				case 52: safe_str(unparse_integer(do_set_fire(parse_integer(args[3]), parse_integer(args[4]), parse_integer(args[5]), parse_integer(args[6]), enactor)), buff, bp); break;
				case 53: safe_str(unparse_integer(do_set_yaw(parse_number(args[3]), enactor)), buff, bp); break;
				case 54: safe_str(unparse_integer(do_set_pitch(parse_number(args[3]), enactor)), buff, bp); break;
				case 55: safe_str(unparse_integer(do_set_roll(parse_number(args[3]), enactor)), buff, bp); break;
				case 56: safe_str(unparse_integer(do_set_coords_engage(enactor)), buff, bp); break;

				case 58: safe_str(unparse_integer(do_set_parallel(parse_integer(args[3]), enactor)), buff, bp); break;
				default: safe_str("#-1 NO SUCH FIELD SELECTION", buff, bp); break;
			} break;
		case 'e': /* empty */
			x = get_empty_sdb();
			if (x == VACANCY_FAIL) {
				safe_str("#-1 NO EMPTY SDB NUMBER", buff, bp); return; }
			safe_str(unparse_integer(x), buff, bp); break;
		case 'f': /* function */
			switch (args[2][0]) {
				case 'a': /* angular velocity */
					n = parse_integer(args[1]);
					if (!GoodSDB(n)) {
						safe_str("#-1 SDB 1 OUT OF RANGE", buff, bp); return; }
					x = parse_integer(args[3]);
					if (!GoodSDB(x)) {
						safe_str("#-1 SDB 2 OUT OF RANGE", buff, bp); return; }
					safe_str(unparse_number(sdb2angular(n, x)), buff, bp); break;
				case 'b': /* bearing */
					n = parse_integer(args[1]);
					if (!GoodSDB(n)) {
						safe_str("#-1 SDB 1 OUT OF RANGE", buff, bp); return; }
					x = parse_integer(args[3]);
					if (!GoodSDB(x)) {
						safe_str("#-1 SDB 2 OUT OF RANGE", buff, bp); return; }
					safe_str(unparse_number(sdb2bearing(n, x)), buff, bp);
					safe_chr(' ', buff, bp);
					safe_str(unparse_number(sdb2elevation(n, x)), buff, bp); break;
				case 'd': /* dump */
					(void) dump_space(executor);
					safe_chr('1', buff, bp); break;
				case 'f': /* fix */
					n = parse_integer(args[1]);
					if (!GoodSDB(n)) {
						safe_str("#-1 SDB OUT OF RANGE", buff, bp); return; }
					safe_str(unparse_integer(repair_everything()), buff, bp); break;
				case 'r': /* range */
					n = parse_integer(args[1]);
					if (!GoodSDB(n)) {
						safe_str("#-1 SDB 1 OUT OF RANGE", buff, bp); return; }
					x = parse_integer(args[3]);
					if (!GoodSDB(x)) {
						safe_str("#-1 SDB 2 OUT OF RANGE", buff, bp); return; }
					safe_str(unparse_range(sdb2range(n, x)), buff, bp); break;
				case 's': /* shield */
					n = parse_integer(args[1]);
					if (!GoodSDB(n)) {
						safe_str("#-1 SDB 1 OUT OF RANGE", buff, bp); return; }
					x = parse_integer(args[3]);
					if (!GoodSDB(x)) {
						safe_str("#-1 SDB 2 OUT OF RANGE", buff, bp); return; }
					s = sdb2shield(n, x);
					if (sdb[n].shield.exist && sdb2dissipation(n, s)) {
						safe_str(tprintf("%s Up", unparse_shield(s)), buff, bp); return; }
					safe_str(tprintf("%s Dn", unparse_shield(s)), buff, bp); break;
				case 't': /* transporter */
					n = parse_integer(args[1]);
					if (!GoodSDB(n)) {
						safe_str("#-2 SDB 1 OUT OF RANGE", buff, bp); return; }
					if (!sdb[n].trans.exist) {
						safe_str("#-3 TRANSPORTER NONEXISTANT", buff, bp); return; }
					if (!sdb[n].trans.damage) {
						safe_str("#-4 TRANSPORTER DAMAGED", buff, bp); return; }
					if (!sdb[n].trans.active) {
						safe_str("#-5 TRANSPORTER OFFLINE", buff, bp); return; }
					s = sdb[n].trans.s_lock;
					if (!GoodSDB(s)) {
						safe_str("#-6 TRANSPORTER SOURCE UNLOCKED", buff, bp); return; }
					x = sdb[n].trans.d_lock;
					if (!GoodSDB(x)) {
						safe_str("#-7 TRANSPORTER DEST UNLOCKED", buff, bp); return; }
					if (n != s)
						if (sdb[n].slist.lev[sdb2slist(n, s)] < 0.5) {
							safe_str("#-8 INSUFFICIENT SOURCE RESOLUTION", buff, bp); return; }
					if (n != x)
						if (sdb[n].slist.lev[sdb2slist(n, x)] < 0.5) {
							safe_str("#-9 INSUFFICIENT DEST RESOLUTION", buff, bp); return; }
					if (n != s)
						if (sdb2range(n, s) > MAX_TRANSPORTER_DISTANCE * sdb[n].tech.sensors) {
							safe_str("#-10 SOURCE OUT OF RANGE", buff, bp); return; }
					if (n != x)
						if (sdb2range(n, x) > MAX_TRANSPORTER_DISTANCE * sdb[n].tech.sensors) {
							safe_str("#-11 DEST OUT OF RANGE", buff, bp); return; }
					if (sdb[n].power.total * sdb[n].alloc.transporters < sdb[n].trans.cost) {
						safe_str("#-12 INSUFFICIENT POWER", buff, bp); return; }
					if (s != n) {
						if (sdb[n].shield.exist)
							if (fabs(sdb[n].shield.freq - sdb[n].trans.freq) > 0.005)
								if (sdb2dissipation(n, sdb2shield(n, s)) != 0.0) {
									safe_str("#-13 SOURCE FACING SHIELD UP", buff, bp); return; }
						if (sdb[s].shield.exist)
							if (fabs(sdb[s].shield.freq - sdb[n].trans.freq) > 0.005)
								if (sdb2dissipation(s, sdb2shield(s, n)) != 0.0) {
									safe_str("#-14 SOURCE CONTACT SHIELD UP", buff, bp); return; }
					}
					if (x != n) {
						if (sdb[n].shield.exist)
							if (fabs(sdb[n].shield.freq - sdb[n].trans.freq) > 0.005)
								if (sdb2dissipation(n, sdb2shield(n, x)) != 0.0) {
									safe_str("#-15 DEST FACING SHIELD UP", buff, bp); return; }
						if (sdb[x].shield.exist)
							if (fabs(sdb[n].shield.freq - sdb[x].trans.freq) > 0.005)
								if (sdb2dissipation(x, sdb2shield(x, n)) != 0.0) {
									safe_str("#-16 DEST CONTACT SHIELD UP", buff, bp); return; }
					}
					safe_str(tprintf("%s %s %s", unparse_dbref(sdb[n].object),
					  unparse_dbref(sdb[s].object), unparse_dbref(sdb[x].object)), buff, bp); break;
				default: safe_str("#-1 NO SUCH SDB FUNCTION", buff, bp); break;
			} break;
		case 'g': /* get */
			x = parse_integer(args[1]);
			if (!GoodSDB(x)) {
				safe_str("#-1 SDB OUT OF RANGE", buff, bp); return; }
			do_space_db_get(x, args[2], args[3], args[4], args[5], buff, bp); break;
		case 'i': /* iterate */
			safe_str(unparse_integer(do_space_db_iterate()), buff, bp); break;
		case 'p': /* put */
			x = parse_integer(args[1]);
			if (!GoodSDB(x)) {
				safe_str("#-1 SDB OUT OF RANGE", buff, bp); return; }
			do_space_db_put(x, args[2], args[3], args[4], args[5], args[6], buff, bp); break;
		case 'r': /* read */
			ship = parse_dbref(args[1]);
			if (!SpaceObj(ship) || !GoodObject(ship)) {
				safe_str("#-1 INVALID SPACE OBJECT", buff, bp); return; }
			safe_str(unparse_integer(do_space_db_read(ship, executor)), buff, bp); break;
		case 's': /* status */
			n = parse_integer(args[1]);
			if (!GoodSDB(n)) {
				safe_str("#-1 SDB OUT OF RANGE", buff, bp); return; }
			switch (parse_integer(args[2])) {
				case 1: safe_str(unparse_integer(do_sensor_contacts(args[3], enactor)), buff, bp); break;
				case 2: safe_str(unparse_integer(do_eng_status(enactor)), buff, bp); break;
				case 3: safe_str(unparse_integer(do_helm_status(enactor)), buff, bp); break;
				case 4: safe_str(unparse_integer(do_tactical_status(enactor)), buff, bp); break;
				case 5: safe_str(unparse_integer(do_beam_status(enactor)), buff, bp); break;
				case 6: safe_str(unparse_integer(do_missile_status(enactor)), buff, bp); break;
				case 7: safe_str(unparse_integer(do_operations_status(enactor)), buff, bp); break;
				case 8: safe_str(unparse_integer(do_sensor_report(parse_integer(args[3]), enactor)), buff, bp); break;
				case 9: safe_str(unparse_integer(do_allocation_status(enactor)), buff, bp); break;
				case 10: safe_str(unparse_integer(do_damage_status(parse_integer(args[3]), args[4], enactor)), buff, bp); break;
				case 11: safe_str(unparse_integer(do_science_status(enactor)), buff, bp); break;
				case 12: safe_str(unparse_integer(do_fighter_status(enactor)), buff, bp); break;
				case 13: safe_str(unparse_integer(do_scanner_report(parse_integer(args[3]), args[4], enactor)), buff, bp); break;
				case 14: safe_str(unparse_integer(do_nebula_report(enactor)), buff, bp); break;
				case 15: safe_str(unparse_integer(do_border_report(enactor)), buff, bp); break;
				case 16: safe_str(unparse_integer(do_planet_report(args[3], 1, enactor)), buff, bp); break;
				case 17: safe_str(unparse_integer(do_planet_report(args[3], 0, enactor)), buff, bp); break;
				case 18: safe_str(do_sensor_contacts_bot(args[3], enactor), buff, bp); break;
				default: safe_str("#-1 NO SUCH FIELD SELECTION", buff, bp); break;
			} break;
		case 'v': /* variable */
			switch (args[1][2]) {
				case 'c': /* cochrane */
					a = parse_number(args[2]);
					if (a < 1.0) {
						cochrane = COCHRANE;
					} else if (a > COCHRANE * 1000.0) {
						cochrane = COCHRANE * 1000.0;
					} else
						cochrane = a;
					notify(executor, "Spacetime Cochrane constant set.");
					safe_str(unparse_number(cochrane), buff, bp); break;
				case 'n': /* console */
					console = parse_dbref(args[3]);
					if (!GoodObject(console)) {
						safe_str("#-1 NOT A VALID DBREF", buff, bp); return; }
					switch (args[2][0]) {
						case 'c': /* communication */
							notify(executor, "Communications console parent dbref set.");
							console_communication = console; break;
						case 'd': /* damage */
							notify(executor, "Damage Control console parent dbref set.");
							console_damage = console; break;
						case 'e': /* engineering */
							notify(executor, "Engineering console parent dbref set.");
							console_engineering = console; break;
						case 'f': /* fighter */
							notify(executor, "Fighter/Shuttle console parent dbref set.");
							console_fighter = console; break;
						case 'h': /* helm */
							notify(executor, "Helm/Navigation console parent dbref set.");
							console_helm = console; break;
						case 'm': /* monitor */
							notify(executor, "Monitor console parent dbref set.");
							console_monitor = console; break;
						case 'n': /* helm */
							notify(executor, "Helm/Navigation console parent dbref set.");
							console_helm = console; break;
						case 'o': /* operations */
							notify(executor, "Operations console parent dbref set.");
							console_operation = console; break;
						case 's':
							switch (args[2][1]) {
								case 'c': /* science */
									notify(executor, "Science console parent dbref set.");
									console_science = console; break;
								case 'e': /* security */
									notify(executor, "Security console parent dbref set.");
									console_security = console; break;
								case 'h': /* fighter */
									notify(executor, "Fighter/Shuttle console parent dbref set.");
									console_fighter = console; break;
								default: safe_str("#-1 NO SUCH FIELD SELECTION", buff, bp); break;
							} break;
						case 't':
							switch (args[2][1]) {
								case 'a': /* tactical */
									notify(executor, "Tactical/Weapons console parent dbref set.");
									console_tactical = console; break;
								case 'r': /* transporter */
									notify(executor, "Transporter console parent dbref set.");
									console_transporter = console; break;
								default : safe_str("#-1 NO SUCH FIELD SELECTION", buff, bp); break;
							} break;
						case 'w': /* tactical */
							notify(executor, "Tactical/Weapons console parent dbref set.");
							console_tactical = console; break;
						default: safe_str("#-1 NO SUCH FIELD SELECTION", buff, bp); break;
					}
					safe_str(unparse_dbref(console), buff, bp); break;
				default: safe_str("#-1 NO SUCH FIELD SELECTION", buff, bp); break;
			} break;
		case 'w': /* write */
			ship = parse_dbref(args[1]);
			if (!SpaceObj(ship) || !GoodObject(ship)) {
				safe_str("#-1 INVALID SPACE OBJECT", buff, bp); return; }
			safe_str(unparse_integer(do_space_db_write(ship, executor)), buff, bp); break;
		default: safe_str("#-1 NO SUCH FIELD SELECTION", buff, bp); break;
	}
	return;
}

/* ------------------------------------------------------------------------ */

FUNCTION(fun_cdb) /* cdb (<function>[,<field>[,<field>[,<field>[,<field>[,...]]]]]) */
{
	register int i, x, y, e;
	double range, max_range, freq;
	dbref zone;
	char *tptr[10];
	char const *tp, *tbuf;
	static char ibuf1[20], ibuf2[20];
	static char dbuf1[20], dbuf2[10], dbuf3[10];
	static char nbuf1[20], nbuf2[20], nbuf3[20];
	static char msg[BUFFER_LEN];
	char *mp = msg;
	ATTR *a;

	if (!Wizard(Owner(executor))) {
		safe_str("#-1 PERMISSION DENIED", buff, bp);
	} else {
		switch (args[0][0]) {
			case 'a': /* add to CDB <void> */
			 	if (Typeof(executor) != TYPE_THING) {
			 		safe_str("#-1 INVALID TYPE", buff, bp);
					return;
				}
				for (i = MIN_COMMS_OBJECTS; i <= MAX_COMMS_OBJECTS; ++i)
					if (cdb[i].object == executor) {
						safe_str("#-1 OBJECT ALREADY IN CDB", buff, bp);
						return;
					} else if (cdb[i].object == 0) {
						if (i > max_comms_objects)
							max_comms_objects = i;
						cdb[i].object = executor;
						cdb[i].lower = MIN_COMMS_FREQUENCY;
						cdb[i].upper = MAX_COMMS_FREQUENCY;
						safe_chr('1', buff, bp);
						notify(executor, tprintf("#%d/%d: object added.", cdb[i].object, i));
						return;
					}
				safe_str("#-1 OUT OF CDB SPACE", buff, bp);
				break;
			case 'b': /* bug fixing <void> */
				for (i = MIN_COMMS_OBJECTS; i <= MAX_COMMS_OBJECTS; ++i)
					if (cdb[i].object) {
						if (!GoodObject(cdb[i].object) || !Hasprivs(Owner(cdb[i].object))) {
							notify(executor, tprintf("#%d/%d: bad object deleted.", cdb[i].object, i));
							cdb[i].object = 0;
							cdb[i].lower = 0.0;
							cdb[i].upper = 0.0;
						} else {
							max_comms_objects = i;
							if (cdb[i].lower < MIN_COMMS_FREQUENCY) {
								notify(executor, tprintf("#%d/%d: bad lower bandpass fixed.", cdb[i].object, i));
								cdb[i].lower = MIN_COMMS_FREQUENCY;
							}
							if (cdb[i].upper > MAX_COMMS_FREQUENCY) {
								notify(executor, tprintf("#%d/%d: bad upper bandpass fixed.", cdb[i].object, i));
								cdb[i].upper = MAX_COMMS_FREQUENCY;
							}
							if (cdb[i].upper < cdb[i].lower) {
								notify(executor, tprintf("#%d/%d: bad bandpass fixed.", cdb[i].object, i));
								cdb[i].upper = cdb[i].lower;
							}
						}
					}
				safe_str(unparse_integer(max_comms_objects), buff, bp);
				break;
			case 'c': /* check the CDB <void> */
				for (i = MIN_COMMS_OBJECTS; i <= max_comms_objects; ++i)
					if (cdb[i].object == executor) {
						safe_str(unparse_integer(i), buff, bp);
						return;
					}
				safe_str("#-1 OBJECT NOT IN CDB", buff, bp);
				break;
			case 'd': /* delete from CDB <void> */
				for (i = MIN_COMMS_OBJECTS; i <= max_comms_objects; ++i)
					if (cdb[i].object == executor) {
						notify(executor, tprintf("#%d/%d: object deleted.", cdb[i].object, i));
						cdb[i].object = 0;
						cdb[i].lower = 0.0;
						cdb[i].upper = 0.0;
						safe_chr('1', buff, bp);
						return;
					}
				safe_str("#-1 OBJECT NOT IN CDB", buff, bp);
				break;
			case 'l': /* list <void> */
				for (i = MIN_COMMS_OBJECTS; i <= max_comms_objects; ++i)
					if (cdb[i].object == executor) {
						safe_str(tprintf("#%d %d %f %f", cdb[i].object, i, cdb[i].lower, cdb[i].upper), buff, bp);
						return;
					}
				safe_str("#-1 OBJECT NOT IN CDB", buff, bp);
				break;
			case 's': /* set bandpass <lower> <upper> */
				for (i = MIN_COMMS_OBJECTS; i <= max_comms_objects; ++i)
					if (cdb[i].object == executor) {
						cdb[i].lower = parse_number(args[1]);
						cdb[i].upper = parse_number(args[2]);
						if (cdb[i].lower < MIN_COMMS_FREQUENCY)
							cdb[i].lower = MIN_COMMS_FREQUENCY;
						if (cdb[i].upper > MAX_COMMS_FREQUENCY)
							cdb[i].upper = MAX_COMMS_FREQUENCY;
						if (cdb[i].upper < cdb[i].lower)
							cdb[i].upper = cdb[i].lower;
						safe_str(tprintf("%f %f", cdb[i].lower, cdb[i].upper), buff, bp);
						notify(executor, tprintf("#%d/%d: bandpass set.", cdb[i].object, i, cdb[i].lower, cdb[i].upper));
						return;
					}
				safe_str("#-1 OBJECT NOT IN CDB", buff, bp);
				break;
			case 't': /* transmit <freq> <max range> <code> <message> */
				freq = parse_number(args[1]);
				if (freq < MIN_COMMS_FREQUENCY || freq > MAX_COMMS_FREQUENCY) {
					safe_str("#-1 BAD FREQUENCY VALUE", buff, bp);
					return;
				}
				max_range = parse_number(args[2]);
				if (max_range <= 0) {
					safe_str("#-1 BAD RANGE VALUE", buff, bp);
					return;
				}
				zone = Location(executor);
				if (Typeof(zone) != TYPE_ROOM) {
					zone = Location(zone);
					if (Typeof(zone) != TYPE_ROOM) {
						zone = Location(zone);
						if (Typeof(zone) != TYPE_ROOM) {
							safe_str("#-1 BAD LOCATION", buff, bp);
							return;
						} else
							zone = Zone(zone);
					} else
						zone = Zone(zone);
				} else
					zone = Zone(zone);
				if (!SpaceObj(zone)) {
					safe_str("#-1 BAD SPACE OBJECT", buff, bp);
					return;
				}
				a = atr_get(zone, SDB_ATTR_NAME);
				if (a == NULL) {
					safe_str("#-1 NO SDB NUMBER", buff, bp);
					return;
				}
				x = parse_integer(uncompress(a->value));
				if (!GoodSDB(x)) {
					safe_str("#-1 BAD SDB NUMBER", buff, bp);
					return;
				}
				/* save the stack */
				for (i = 0; i < 10; i++)
    				tptr[i] = wenv[i];
				snprintf(ibuf1, sizeof(ibuf1), "%d", x);
				snprintf(dbuf1, sizeof(dbuf1), "#%d", enactor);
				snprintf(dbuf2, sizeof(dbuf2), "#%d", executor);
				snprintf(nbuf1, sizeof(nbuf1), "%g", freq);
				snprintf(nbuf2, sizeof(nbuf2), "%.0f", max_range);
    			wenv[0] = nbuf1;
    			wenv[1] = dbuf1;
				wenv[2] = dbuf2;
				wenv[4] = ibuf1;
				wenv[6] = nbuf2;
				wenv[8] = args[3];
				a = atr_get(executor, ENCRYPTION_ATTR_NAME);
				if (a == NULL) {
					safe_str(args[4], msg, &mp);
					e = 0;
				} else {
					safe_str(crypt_code(uncompress(a->value), args[4], 1), msg, &mp);
					e = 1;
				}
				*mp = '\0';
				for (i = MIN_COMMS_OBJECTS; i <= max_comms_objects; ++i)
					if (cdb[i].object) {
						if (freq < cdb[i].lower || freq > cdb[i].upper)
							continue;
						zone = Location(cdb[i].object);
						if (GoodObject(zone)) {
							if (Typeof(zone) != TYPE_ROOM) {
								zone = Location(zone);
								if (GoodObject(zone)) {
									if (Typeof(zone) != TYPE_ROOM) {
										zone = Location(zone);
										if (GoodObject(zone)) {
											if (Typeof(zone) != TYPE_ROOM) {
												continue;
											} else
												zone = Zone(zone);
										} else
											continue;
									} else
										zone = Zone(zone);
								} else
									continue;
							} else
								zone = Zone(zone);
						} else
							continue;
						if (!SpaceObj(zone))
							continue;
						a = atr_get(zone, SDB_ATTR_NAME);
						if (a == NULL)
							continue;
						y = parse_integer(uncompress(a->value));
						if (!GoodSDB(y))
							continue;
						range = sdb2range(x, y);
						if (range > max_range)
							continue;
						a = atr_get(cdb[i].object, EXECUTE_ATTR_NAME);
						if (a == NULL)
							continue;
						tp = tbuf = safe_uncompress(a->value);
						snprintf(dbuf3, sizeof(dbuf3), "#%d", cdb[i].object);
						snprintf(ibuf2, sizeof(ibuf2), "%d", y);
						snprintf(nbuf3, sizeof(nbuf3), "%.0f", range);
						wenv[3] = dbuf3;
						wenv[5] = ibuf2;
						wenv[7] = nbuf3;
						if (e) {
							a = atr_get(cdb[i].object, ENCRYPTION_ATTR_NAME);
							if (a == NULL) {
								wenv[9] = msg;
							} else
								wenv[9] = crypt_code(uncompress(a->value), msg, 0);
						} else
							wenv[9] = msg;
						process_expression(buff, bp, &tp, cdb[i].object, executor, enactor,
						  PE_DEFAULT, PT_DEFAULT, pe_info);
						free((Malloc_t) tbuf);
					}
				/* restore the stack */
				for (i = 0; i < 10; i++)
    				wenv[i] = tptr[i];
				safe_chr('1', buff, bp);
				notify(executor, "transmission sent.");
				break;
			default:
				safe_str("#-1 NO SUCH FIELD SELECTION", buff, bp);
				break;
		}
	}

	return;
}

/* ------------------------------------------------------------------------ */
