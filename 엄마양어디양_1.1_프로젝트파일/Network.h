#include "stdafx.h"
#include "Player.h"

class CNetwork
{
	SOCKET m_socket;
	thread *m_pRecvThread;
	int m_nLeft;
	char m_saveBuf[MAX_PACKET_SIZE];

	// ���Ӱ�������
	int *m_piGameMode;
public:
	CPlayer m_Players[MAX_PLAYER_CNT];
	vector<Object*> m_vpMovingObject;

	CNetwork();
	~CNetwork();
	//���� ��� �Լ�
	void err_quit(char *);
	void err_display(char *);

	void connectServer();
	void endServer();
	void recvThreadFunc();
	void packetUnpacker();
	void keyDown(int);
	void keyUp(int);
	void getReady();

	void initGameMode(int *piGameMode) {
		m_piGameMode = piGameMode;
		for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
			m_Players[i].m_pSheep->iGameMode = *piGameMode;
		}
		
	}
};

