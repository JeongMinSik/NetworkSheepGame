#include "stdafx.h"
#include "Protocol.h"

struct SOCKETINFO
{
	SOCKET		m_sock;
	int			m_nID;
	char		m_saveBuf[MAX_PACKET_SIZE];
	bool		m_bReady;

	SOCKETINFO();
	~SOCKETINFO();
};

class CNetwork
{
	vector<thread*>					m_vpThreadlist;
	vector<SOCKETINFO*>				m_vpClientInfo;
	SOCKET							m_listenSock;
	int								m_nPlayerCount;
	int								m_nReadyCount;
	bool							m_bPlaying;

public:
	CNetwork();
	~CNetwork();

	void err_quit(char *msg);
	void err_display(char *msg);
	void initNetwork();
	void printHostInfo();
	void startServer();
	void acceptThread();
	void recvThread(int id);
	void packetProcess(int id);
	void transmitProcess(void* buf, int id);
	void endServer();

	bool Login(int id);
	bool Logout(int id);
	bool Ready(int id);
};
