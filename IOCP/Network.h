#pragma once
#include "stdafx.h"
#include "Protocol.h"
#include "Objects.h"

enum class OP_TYPE
{
	OP_SEND = 0,
	OP_RECV
};

// 소켓 정보 저장을 위한 구조체
struct SOCKETINFO
{
	OVERLAPPED overlapped;
	WSABUF wsabuf;
	SOCKET sock;
	OP_TYPE optype = OP_TYPE::OP_RECV;
	char IOBuf[MAX_PACKET_SIZE];
	char packetBuf[MAX_PACKET_SIZE];
	int	 iCurrPacketSize; 
	int	 iStoredPacketSize;
	Sheep *pSheep;

	//정보
	int nID;
	bool isReady;

	SOCKETINFO();
	~SOCKETINFO();
};

class CNetwork
{
	vector<thread*>					m_vpThreadlist;
	vector<SOCKETINFO*>				m_vpClientInfo;
	SOCKET							m_listenSock;
	HANDLE							m_hIOCP;
	UINT							m_nID;
	UINT							m_nReadyCount;
	bool							m_isPlaying;

	mutex							m_lock;
	// 게임관련
	Ground* ground[GROUND_NUM];
	Object* obstacles[OB_CNT];
	MotherSheep* mother_sheep;
	int ob_num = 0;
	int Game_Mode = MAIN_MODE;
	vector<Object*> m_vpMovingObject;
	Sheep* m_pSheeps[MAX_PLAYER_CNT];
	float m_fCurrentTime;
	float m_fAccumulator;
	float m_fSyncTime;

public:

	CNetwork();
	~CNetwork();

	//오류 출력 함수
	void err_quit(char *msg);
	void err_display(char *msg);

	void initNetwork();
	void startServer(); 
	void updateServer();
	void endServer();

	//스레드 함수
	bool acceptThread();
	void workerThread();

	void CreateWorld();
	void DestroyWorld();
	
	//패킷 처리 함수
	bool packetProcess(CHAR*, int);
	void transmitProcess(void * buf, int id);
	bool Login(int id);
	bool Logout(int id);
	
	//보내는 함수
	bool Ready(int id); 
	bool Finish(int id);
	bool Start();
	bool Key(int id, void *buf);
	bool Sync();
	bool Hurt(int id);

	//아이피주소얻기
	void printHostInfo();
};
