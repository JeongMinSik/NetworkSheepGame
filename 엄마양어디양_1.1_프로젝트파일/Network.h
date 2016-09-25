#include "stdafx.h"
#include "Player.h"

class CNetwork
{
	SOCKET m_socket;
	thread *m_pRecvThread;
	int m_nLeft;
	char m_saveBuf[MAX_PACKET_SIZE];

	// 게임관련정보
	int *m_piGameMode;
public:
	int m_nPlayerCount;
	CPlayer m_Players[MAX_PLAYER_CNT];
	Object **m_ppObstacles;

	CNetwork();
	~CNetwork();
	//오류 출력 함수
	void err_quit(char *);
	void err_display(char *);

	void connectServer();
	void endServer();
	void recvThreadFunc();
	void packetUnpacker();
	void keyDown(int);
	void keyUp(int);
	void getReady();
	void keySync();

	void initGameMode(int *piGameMode) {
		m_piGameMode = piGameMode;
		for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
			m_Players[i].m_pSheep->iGameMode = *piGameMode;
		}
		
	}
};

