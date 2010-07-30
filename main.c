/*
 * =====================================================================================
 *
 *       Filename:  main.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2009年09月23日 18时44分21秒 CST
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  lerosua (), lerosua@gmail.com
 *        Company:  Cyclone
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM 1024
int main(int argc,char *argv[])
{
	int foo = 437;
	printf("hello world\n");

	char buf[NUM];
	memset(buf, '1',NUM);
	char var[NUM]="test for linux";

	char dbuf[NUM];
	strncpy(dbuf, var, strlen(var));
	strncpy(buf, var, strlen(var));
	buf[strlen(var)] = 0;

	printf("buf = %s, var= %s, dbuf = %s\n",buf, var,dbuf);

	    return 0;
}
