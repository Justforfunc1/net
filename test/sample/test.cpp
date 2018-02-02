/**********************************************************
 * \file test.cpp
 * \brief
 * \note	注意事项：
 *
 *\version 
 ** \author Allen.L
**********************************************************/

#include <stdio.h>
#include "epoll_connection.h"

int main(int args, char **argv)
{
	epoll_connection epc;
	epc.ep_startupServer();
	return 0;
}
