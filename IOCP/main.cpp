#include "stdafx.h"
#include "NetWork.h"

CNetwork gNetwork;

int main()
{
	//��Ʈ��ũ�� �ʱ�ȭ �Ѵ�
	gNetwork.initNetwork();
	//������ �����Ѵ�
	gNetwork.startServer();
	//������ �����Ѵ�
	gNetwork.endServer();
}
