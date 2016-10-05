#include "stdafx.h"
#include "NetWork.h"

CNetwork gNetwork;

void CALLBACK Loop(UINT, UINT, DWORD_PTR, DWORD_PTR, DWORD_PTR)
{
	gNetwork.Timer();
}

int main()
{
	//네트워크를 초기화 한다
	gNetwork.initNetwork();
	//서버를 시작한다
	gNetwork.startServer();

	TIMECAPS caps;
	timeGetDevCaps(&caps, sizeof(caps));

	timeSetEvent(FIXED_FRAME_TIME, caps.wPeriodMin, Loop, NULL, TIME_PERIODIC);

	//서버를 종료한다
	gNetwork.endServer();
}
