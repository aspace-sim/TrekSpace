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
#include "econ.h"
//END MYSQL SUPPORT

int do_econ_iterate _((void));
void do_econ_daily_iterate _((void));
void do_econ_pay_employees _((void));

int e;

/*-----------LETS DO ALL THE ECON CHECKS----------*/
int do_econ_iterate (void)
{

	



	return 1;
}
/*------------------------------------------------*/
void do_econ_daily_iterate (void) {

//Lets Pay employees
do_econ_pay_employees();



}
/*------------------------------------------------*/

void do_econ_pay_employees (void) {
MYSQL *conn;
MYSQL_RES *res_set,*res_set1;
MYSQL_ROW	row;
MYSQL_FIELD *field,*field1;
unsigned int	i,i1;
int companyid ="0",primarybankid="0",balance="0",playerid="0";
int fieldcount,fieldcount1,companyid1,salary="0",bankid="0";
char *playername;
char *from,*subject,*to,*bcc,*body,*priority;


//Lets loop through and find all companies that need to pay people
	conn = mysql_init (NULL);
	mysql_real_connect(conn,"localhost.localdomain","root","yourpw","yourdb",0,NULL,0);
	if (!conn) { /*do_ship_notify(1, tprintf("No SQL database connection."));	*/ }
	if (conn) {  /*do_ship_notify(1, tprintf("SQL database connection."));*/
	//Lets run the query
	if (mysql_query (conn, tprintf("select companyid,primarybankid from econcompany")) != 0){	/*do_ship_notify(1, tprintf("Query Failed"));*/   }
	else {
		res_set = mysql_store_result (conn); //Generate Result Set
		if (res_set==NULL) { /*notify(executor, "Mysql_store_result() Failed");*/ 		}
		else {
				//Start the actual processing of the errors
					while((row=mysql_fetch_row (res_set)) != NULL)
						{
							mysql_field_seek (res_set,0);
							for (i = 0; i < mysql_num_fields (res_set); i++)
							{
								field=mysql_fetch_field (res_set);						
								//Lets get the recordid
								if (strcmp(field->name,"companyid") ==0) {
									companyid=atoi(row[i]);
									fieldcount=1;
								}		
								if (strcmp(field->name,"primarybankid") ==0) {
									primarybankid=atoi(row[i]);
									fieldcount=2;
								}		
								if (fieldcount ==2) {
								//LETS LOOP OVER THE STUFF NOW	
								//do_ship_notify(1, tprintf("companyid %i - Primarybankid %i",companyid,primarybankid));				

									/*do_ship_notify(1, tprintf("companyid %i - Primarybankid %i",companyid,primarybankid));*/
									if (mysql_query (conn, tprintf("select econcompany.companyid as mycompanyid,balance,playerid,salary,bankid,econplayers.name as myplayer from econcompany inner join econcorpbankaccounts on econcompany.companyid=econcorpbankaccounts.companyid  inner join econplayers on econcompany.companyid=econplayers.companyid where econcorpbankaccounts.bankid='%i' and econcompany.companyid='%i'",primarybankid,companyid)) != 0) {
										/*notify(executor, "Query Failed");*/   
									}
									
								/*	mysql_query (conn, tprintf("select econcompany.companyid,balance,playerid from econcompany inner join 
									econcorpbankaccounts on econcompany.companyid=econcorpbankaccounts.companyid  inner join econplayers on 
									econcompany.companyid=econplayers.companyid where econcorpbankaccounts.bankid='%i' and econcompany.companyid='%i'",primarybankid,companyid))
								mysql_query (conn, tprintf("select companyid,balance from econcorpbankaccounts where bankid='%i' and companyid='%i'",primarybankid,companyid))
									*/																			
									res_set1 = mysql_store_result (conn);

									if (res_set1==NULL) {
									//notify(enactor, "Mysql_store_result() Failed");
									//do_ship_notify(1, tprintf("Mysql Store reseult failed"));
									}
									else {
									//do_ship_notify(1, tprintf("Got In"));
									//Lets Get the recordid of the company!
										while((row=mysql_fetch_row (res_set1)) != NULL)
											{
											//do_ship_notify(1, tprintf("%i X%f Y %f Z%f",t,sdb[t].coords.x,sdb[t].coords.y,sdb[t].coords.z));
												mysql_field_seek (res_set1,0);
												for (i1 = 0; i1 < mysql_num_fields (res_set1); i1++)
												{
													field1=mysql_fetch_field (res_set1);
													//Lets get the companyid
													if (strcmp(field1->name,"mycompanyid") ==0) {
														companyid1=atoi(row[i1]);
														fieldcount1=1;
													}	
													//Lets get the balance
													if (strcmp(field1->name,"balance") ==0) {
														balance=atoi(row[i1]);
														fieldcount1=2;
													}	
													//Lets get the balance
													if (strcmp(field1->name,"playerid") ==0) {
														playerid=atoi(row[i1]);
														fieldcount1=3;
													}	
													//Lets get the balance
													if (strcmp(field1->name,"salary") ==0) {
														salary=atoi(row[i1]);
														fieldcount1=4;
													}	
													//Lets get the balance
													if (strcmp(field1->name,"bankid") ==0) {
														bankid=atoi(row[i1]);
														fieldcount1=5;
													}	
													if (strcmp(field1->name,"myplayer") ==0) {
														playername=row[i1];
														fieldcount1=6;
													}	
													//do_ship_notify(1, tprintf("companyid %i - balance %i - playerid %i",companyid1,balance,playerid));									
													if (fieldcount1==6) {
														fieldcount1=0;
														//Lets reset the balance varaible just in casebankid
														 
														if(balance > 10000) {  //Lets only pay them if they have the money
															//do_ship_notify(1, tprintf("companyid %i - balance %i - playerid %i - salary %i - playername %s",companyid1,balance,playerid,salary,playername));
															//Lets give the players their cas
															body="Your daily payroll has been deposited";
															from="Payroll Dept";
															subject="Paycheck";
															to=playername;
															bcc="";
															priority="0";
	
															/*do_ship_notify(1, tprintf("From %s, Subject %s, to %s, bcc %s, body %s, priority %s\n",from,subject,to,bcc,body,priority));*/
															/*do_ship_notify(1, tprintf("companyid %i - balance %i - playerid %i - salary %i - playername %s\n",companyid1,balance,playerid,salary,playername));*/
															 
															mysql_query (conn, tprintf("update econplayers set cash=cash+%i where playerid='%i'",salary,playerid));
															//Lets Take the companies money
															mysql_query (conn, tprintf("update econcorpbankaccounts set balance=balance-%i where companyid='%i' and bankid='%i'",salary,companyid1,bankid));
															
															
															send_icmail(from,subject,to,bcc,body,priority);
														}
													}

												}
										//We would reset vars here
										companyid1="0";
										playername="";
										bankid="0";
										salary="0";
										playerid="0";
										balance="0";

										}


									}
									

								mysql_free_result(res_set1);
								//End loop


								}
				



							}									
						}
							
							primarybankid="0";
							companyid="0";
							fieldcount="0";
							//Put any vars to blank out here

			}
					/*	if (mysql_errno (conn) !=0)
								notify(executor, "mysql_Fetch_row() Failed");
						else
								notify(executor, tprintf("%lu rows returned\n",(unsigned long) mysql_num_rows (res_set)));
					*/	
				//End the actual processing of the errors
			mysql_free_result(res_set);
		}
	}
	
		
	
	mysql_close(conn);
	/* End Component Check */


}
/*------------------------------------------------*/



