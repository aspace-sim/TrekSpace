/* econ_utils.c */

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
//FOR MYSQL SUPPORT
#include <mysql.h>
#include <errmsg.h>
//END MYSQL SUPPORT

#include "copyrite.h"
#include "ansi.h"
#include "externs.h"
#include "intrface.h"
#include "parse.h"
#include "confmagic.h"
#include "econ.h"
#include "dbdefs.h"
#include "flags.h"


void econ_create_company _((char *companyname,char *chairman,dbref enactor));
void econ_create_idcard _((char *name,dbref enactor));
int econ_dupe_compnamecheck _((char *name,char *zonedbref,dbref enactor));

/* ------------------------------------------------------------------------ */
void econ_create_company(char *companyname,char *chairman,dbref enactor) {
/*
	if (*companyname ==NULL) {
		notify(enactor, tprintf("#-1 COMPANY NAME NOT ENTERED\n"));
		return;
	}
	if (*chairman == NULL){
		notify(enactor, tprintf("#-1 CHAIRMAN NAME NOT ENTERED. CONTACT ADMIN"));
	}

	//IF the company is not null lets make a record in the database
	query=tprintf("insert into econcompany(name,mainoffice,bankaccount,category,empire,chairman,ceo) 
     values('%s',1,'100',0,1,'%s','%s')",companyname,chairman,chairman);

	//If we're this far we're sure the companyname is not null
	//sql_init(enactor);
	//lets run a query to insert this name
	//UNCOMMENT THIS LATER
	//sql_query_internal(enactor, query, buff, bp);

	//Now lets update the players Player record to show his new company
	query=tprintf("select recordid from econcompany
		where name='%s'",companyname);	
	

	mysql = (MYSQL *) malloc(sizeof(MYSQL));
	mysql_real_connect(mysql,"localhost.localdomain","root","pw","db",0,NULL,0);

   if (!mysql) {
     notify(enactor, "No SQL database connection.");
   }
   if (mysql) {
	notify(enactor, "SQL database connection.");
	//mysql_real_query(mysql, query, strlen(query));
	//qres = mysql_store_result(mysql);
	//field = mysql_fetch_field(qres);
   }
	  
*/	
	return;

}
/* ------------------------------------------------------------------------ */

void econ_create_idcard(char *name,dbref enactor) {
	//Lets give *companyname some spcae
	MYSQL *conn;
	MYSQL_RES *res_set;
	MYSQL_ROW	row;
	MYSQL_FIELD *field;
	unsigned int	i;
	char *componentname;
	
	if (*name ==NULL) {
		notify(enactor, tprintf("#-1 NAME NOT ENTERED\n",name));
		return;
	}

/* Lets	do the component check */
	conn = mysql_init (NULL);
	mysql_real_connect(conn,"localhost.localdomain","root","pw","db",0,NULL,0);

	if (!conn) {  /*notify(executor, "No SQL database connection.");*/	}
   if (conn) {	/*notify(executor, "SQL database connection.");*/
	//Lets run the query
	   if (mysql_query (conn, tprintf("insert into econplayers(name,idcard,playerdbref) values('%s',1,'%s')",name,unparse_dbref(enactor))) != 0){
			//notify(executor, "Query Failed");
	   }
   }
   
	//mysql_free_result(res_set);
	mysql_close(conn);
	/* End Component Check */
				
			return;

}


/* ------------------------------------------------------------------------ */

int econ_dupe_compnamecheck(char *name,char *zonedbref,dbref enactor) {
	//Lets give *companyname some spcae
	MYSQL *conn;
	MYSQL_RES *res_set,*res_set1;
	MYSQL_ROW	row;
	MYSQL_FIELD *field;
	unsigned int	i;

	if (*name ==NULL) {
		notify(enactor, tprintf("#-1 NAME NOT ENTERED\n"));
		return;
	}

/* Lets	do the component check */
	conn = mysql_init (NULL);
	mysql_real_connect(conn,"localhost.localdomain","root","pw","db",0,NULL,0);

	if (!conn) { /*notify(executor, "No SQL database connection.");*/	 return 7; }
   if (conn) {	/*notify(executor, "SQL database connection.");*/
	//Lets run the query
	   if (mysql_query (conn, tprintf("select name from econcompany where name='%s'",name)) != 0){
			//notify(executor, "Query Failed");
	   }
	else {
		res_set = mysql_store_result (conn); //Generate Result Set
		if (res_set==NULL) {
				//notify(executor, "Mysql_store_result() Failed");
		}
		else {
			if ((unsigned long) mysql_num_rows (res_set) > 0) {
				mysql_free_result(res_set);
				mysql_close(conn);
				//notify(enactor, tprintf("That name is already taken. Please choose another\n"));
				return 1;
			}
			else {
				//Lets Create the company
				mysql_query (conn, tprintf("insert into econcompany(name,mainoffice,empire,chairman,ceo) values('%s','%s',0,'%s','%s')",name,zonedbref,unparse_dbref(enactor),unparse_dbref(enactor)));
				
				mysql_query (conn, tprintf("select companyid from econcompany where name='%s'",name));
				res_set1 = mysql_store_result (conn);

				if (res_set1==NULL) {
				notify(enactor, "Mysql_store_result() Failed");
				}
				else {
				//notify(enactor, "Got In!");
				//Lets Get the recordid of the company!
					while((row=mysql_fetch_row (res_set1)) != NULL)
						{

							//do_ship_notify(1, tprintf("%i X%f Y %f Z%f",t,sdb[t].coords.x,sdb[t].coords.y,sdb[t].coords.z));
							mysql_field_seek (res_set1,0);
							for (i = 0; i < mysql_num_fields (res_set1); i++)
							{
								field=mysql_fetch_field (res_set1);
								//Lets get the recordid
								if (strcmp(field->name,"companyid") ==0) {
									//notify(enactor, tprintf("Value %i",atoi(row[i])));
									mysql_query (conn, tprintf("update econplayers set companyid='%i' where playerdbref='%s'",atoi(row[i]),unparse_dbref(enactor)));
									mysql_free_result(res_set1);
								}	

							}
					}

				}
				
				
				
				
				//notify(enactor, tprintf("Your new company name will be %s\n",name));

				mysql_free_result(res_set);
				mysql_close(conn);
				return 0;
			}

			
		}

	}

   }	
		
	

}


/* ------------------------------------------------------------------------ */
