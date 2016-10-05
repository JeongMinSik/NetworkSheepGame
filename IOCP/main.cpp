#include "stdafx.h"
#include "NetWork.h"

CNetwork gNetwork;

void CALLBACK Loop(UINT, UINT, DWORD_PTR, DWORD_PTR, DWORD_PTR)
{
	gNetwork.Timer();
}

int main()
{
	//��Ʈ��ũ�� �ʱ�ȭ �Ѵ�
	gNetwork.initNetwork();
	//������ �����Ѵ�
	gNetwork.startServer();

	TIMECAPS caps;
	timeGetDevCaps(&caps, sizeof(caps));

	timeSetEvent(FIXED_FRAME_TIME, caps.wPeriodMin, Loop, NULL, TIME_PERIODIC);

	//������ �����Ѵ�
	gNetwork.endServer();
}
