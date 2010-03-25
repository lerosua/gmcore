/*
 * =====================================================================================
 *
 *       Filename:  main.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2010年03月19日 10时19分32秒 CST
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  lerosua (), lerosua@gmail.com
 *        Company:  Cyclone
 *
 * =====================================================================================
 */

#include <stdio.h>
#include "excgloox.h"
void recv_msg(const char* id,const char* msg)
{
	printf("%s say : %s \n",id,msg);

}
void on_connect()
{
	printf("qx client has connected!!!! in c\n");

}

void handle_roster(const char* jid, const char* nick, const char* group,const char* resource,int sub)
{
	printf("jid= %s,nick= %s, group = %s, resource= %s,subscript = %d\n",jid,nick,group,resource,sub);
}

void handle_roster_presence(const char* jid, const char* resource, const char* msg, int presence)
{
	printf("jid = %s,resource = %s, msg= %s,presence = %d\n",jid,resource,msg,presence);
}

int main(int argc,char *argv[])
{

	qx_init();
	qx_register_handle_on_connect(on_connect);
	qx_register_handle_messgae(recv_msg);
	qx_register_handle_roster(handle_roster);
	qx_register_handle_roster_presence(handle_roster_presence);
	qx_login("botcalk@gmail.com/gloox","botcalk2038");

	qx_run(1);
	

	printf("\n");
	return 0;
}

