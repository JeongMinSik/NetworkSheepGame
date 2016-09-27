#pragma once
#include "stdafx.h"
#include "Protocol.h"
#include "Objects.h"

enum class OP_TYPE
{
	OP_SEND = 0,
	OP_RECV
};

// ���� ���� ������ ���� ����ü
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

	//����
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
	bool							isPlaying;

	//mutex							m_lock;
	// ���Ӱ���
	Ground* ground[GROUND_NUM];
	Object* obstacles[400];
	MotherSheep* mother_sheep;
	int ob_num = 0;
	int Game_Mode = MAIN_MODE;

public:

	CNetwork();
	~CNetwork();

	//���� ��� �Լ�
	void err_quit(char *msg);
	void err_display(char *msg);

	void initNetwork();
	void startServer();
	void endServer();

	//������ �Լ�
	bool acceptThread();
	void workerThread();

	//��Ŷ ó�� �Լ�
	bool packetProcess(CHAR*, int);
	bool Login(int id);
	bool Logout(int id);
	bool Ready(int id); 
	bool Finish(int id);
	bool Start();
	bool Key(int id, void *buf);
	bool syncData(void *buf, int id);
	void transmitProcess(void * buf, int id);
	void CreateWorld();
	void Timer();

	//�������ּҾ��
	void printHostInfo();
};
