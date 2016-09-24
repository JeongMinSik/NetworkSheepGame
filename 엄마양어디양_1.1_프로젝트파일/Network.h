#include "stdafx.h"
#include "Protocol.h"
using namespace std;

class CNetwork
{
	SOCKET m_socket;
	thread *m_pRecvThread;
	int m_nID;
	int m_nLeft;
	char m_saveBuf[MAX_PACKET_SIZE];
public:
	CNetwork();
	~CNetwork();
	//오류 출력 함수
	void err_quit(char *);
	void err_display(char *);

	void connectServer();
	void endServer();
	void recvThreadFunc();
	void packetUnpacker();
	void keyDown(char);
	void keyUp(char);
	void getReady();
};

