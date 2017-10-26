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
#include "ansi.h"
#include "externs.h"
#include "intrface.h"
#include "parse.h"
#include "confmagic.h"
#include "econ.h"
#include "function.h"
#include "attrib.h"
#include "log.h"
#include "dbdefs.h"
#include "flags.h"


//FOR MYSQL SUPPORT
#include <mysql.h>
#include <errmsg.h>
#include "space.h"
#include "combat.h"
//END MYSQL SUPPORT

int do_combat_iterate _((void));
void do_combat_addturns _((void));

/*-----------LETS DO ALL THE combat CHECKS----------*/
int do_combat_iterate (void)
{
//Lets do all the combat checks
do_combat_addturns();
	



	return 1;
}

/*------------------------------------------------*/

void do_combat_addturns (void) {
MYSQL *conn;
MYSQL_RES *res_set,*res_set1;
MYSQL_ROW	row;
MYSQL_FIELD *field,*field1;
unsigned int	i,turn,turnmax,playerid;
int fieldcount,fieldcount1;


//Lets loop through and find all companies that need to pay people
	conn = mysql_init (NULL);
	mysql_real_connect(conn,"localhost.localdomain","root","yourpw","yourdb",0,NULL,0);
	if (!conn) { /*do_ship_notify(1, tprintf("No SQL database connection."));	*/ }
	if (conn) {  /*do_ship_notify(1, tprintf("SQL database connection."));*/
	//Lets run the query
	if (mysql_query (conn, tprintf("update combatplayers set turn=turn+1 where turn<turnmax")) != 0){	/*do_ship_notify(1, tprintf("Query Failed"));*/   }

	}

	mysql_close(conn);
	/* End Component Check */


}
/*------------------------------------------------*/

