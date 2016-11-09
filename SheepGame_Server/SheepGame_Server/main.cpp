#include "stdafx.h"
#include "Network.h"

CNetwork g_Network;

void CALLBACK Loop(UINT, UINT, DWORD_PTR, DWORD_PTR, DWORD_PTR)
{
	g_Network.updateServer();
}

int main()
{

	TIMECAPS caps;
	timeGetDevCaps(&caps, sizeof(caps));
	timeSetEvent(FIXED_FRAME_TIME, caps.wPeriodMin, Loop, NULL, TIME_PERIODIC);

	g_Network.initNetwork();
	g_Network.startServer();
	g_Network.endServer();
}