#pragma once
#include "stdafx.h"
#include "Protocol.h"

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
	int nID;
	char IOBuf[MAX_PACKET_SIZE];
	char packetBuf[MAX_PACKET_SIZE];
	int	 iCurrPacketSize; 
	int	 iStoredPacketSize;

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
	mutex							m_lock;
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
	bool Login(void *buf, int id);
	bool Logout(void *buf, int id);
	bool syncData(void *buf, int id);
	void transmitProcess(void * buf, int id);

	//�������ּҾ��
	void printHostInfo();
};
