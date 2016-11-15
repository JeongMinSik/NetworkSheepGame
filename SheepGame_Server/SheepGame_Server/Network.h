#include "stdafx.h"
#include "Objects.h"
#include "Protocol.h"

struct SOCKETINFO
{
	SOCKET		m_sock;
	int			m_nID;
	char		m_saveBuf[MAX_PACKET_SIZE];
	bool		m_bReady;
	Sheep*		m_pSheep;

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
	float m_fSyncObjectTime;

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
	bool Start();
	bool Key(int id, void *buf);

	bool Hurt(int id);

	bool Sync();

	void updateServer();
	void CreateWorld();
	void DestroyWorld();
};
