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

#include "copyrite.h"
#include "ansi.h"
#include "externs.h"
#include "intrface.h"
#include "parse.h"
#include "confmagic.h"
#include "econ.h"
#include "dbdefs.h"
#include "flags.h"
//FOR MYSQL SUPPORT
#include <mysql.h>
#include <errmsg.h>
//END MYSQL SUPPORT

void send_icmail _((char *from,char *subject,char *to, char *bcc,char *body,char *priority));
char *str_deliminate _((char *string, char deliminator));

/* ------------------------------------------------------------------------ */
//This function will send ic mail to another user 
void send_icmail(char *from,char *subject,char *to, char *bcc,char *body,char *priority) {
MYSQL *conn;
MYSQL_RES *res_set;
MYSQL_ROW	row;
MYSQL_FIELD *field;
unsigned int	i;
int myid,fieldcount="0";
char *str_end;

//Lets break the to list up into parts
//do_ship_notify(1, tprintf(""));


//str_end = str_deliminate( to, ' ');

//end breakup



/* Lets	do the component check */
	conn = mysql_init (NULL);
	mysql_real_connect(conn,"localhost.localdomain","root","pw","db",0,NULL,0);

	if (!conn) { /*notify(executor, "No SQL database connection.");*/	}
   if (conn) {	/*notify(executor, "SQL database connection.");*/
	//Lets run the query
	   //Lets insert the RECORD TO START	
	   mysql_query (conn, tprintf("insert into mailsystem(mailfrom,mailsent,mailbody,mailsubject,mailpriority) values('%s',now(),'%s','%s','%s')",from,body,subject,priority));

	   if (mysql_query (conn, tprintf("select max(messageid) as myid from mailsystem")) != 0){
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
								if (strcmp(field->name,"myid") ==0) {
									myid=atoi(row[i]);
									fieldcount=1;
								}		
								if (fieldcount ==1){
									//Lets insert the CCs first
									mysql_query (conn, tprintf("insert into mailto(messageid,mailto,bcc) values('%i','%s','0')",myid,to));
									//Lets insert the BCC's now
									mysql_query (conn, tprintf("insert into mailto(messageid,mailto,bcc) values('%i','%s','1')",myid,bcc));
									fieldcount=0;
								}
									

							}																	
						}							

							//Reset all vars here
							//End reset all vars here
		}
				//End the actual processing of the errors
			mysql_free_result(res_set);
		}
	}
   			
	
	mysql_close(conn);
	/* End Component Check */






return;
}
/* ------------------------------------------------------------------------ */
char *str_deliminate(char *string, char deliminator)
{
        char *string_end;
        string_end = strchr(string, (char)NULL);
        while(string < string_end) {
                string = strchr(string, deliminator);
                *string = (char)NULL;
                string++;
        }
        return (string_end);
}
/* ------------------------------------------------------------------------ */

