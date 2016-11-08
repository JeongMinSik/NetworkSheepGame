#include "stdafx.h"
#include "Network.h"

CNetwork g_Network;

int main()
{
	g_Network.initNetwork();
	g_Network.startServer();
	g_Network.endServer();
}