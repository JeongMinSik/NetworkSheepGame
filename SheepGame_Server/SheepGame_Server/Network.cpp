#include "Network.h"

SOCKETINFO::SOCKETINFO()
{
	m_bReady = false;
}

SOCKETINFO::~SOCKETINFO()
{
}


CNetwork::CNetwork()
{
	m_nPlayerCount = 0;
	m_nReadyCount = 0;
	m_bPlaying = false;
}

CNetwork::~CNetwork()
{
}

void CNetwork::err_quit(char * msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	exit(1);
}

void CNetwork::err_display(char * msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, lpMsgBuf);
	LocalFree(lpMsgBuf);
}

void CNetwork::initNetwork()
{
	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		err_quit("���� �ʱ�ȭ ����");

	for (int i = 0; i < MAX_ID_CNT; ++i) {
		m_vpClientInfo.push_back(nullptr);
	}

	printf("initNetwork() \n");

}

void CNetwork::printHostInfo() {
	char szLocalHostName[512]; // �� ��ǻ�Ͱ� ��Ʈ��ũ���� �ĺ��Ǵ� �̸��� ����� ���ڿ� �����Դϴ�.
	struct hostent * pLocalHostInformation; // ���� ȣ��Ʈ�� ������ ��� ����ü�� ������

	gethostname(szLocalHostName, sizeof(szLocalHostName));
	printf("���� ȣ��Ʈ�� �̸��� \"%s\"�Դϴ�.\n", szLocalHostName);
	pLocalHostInformation = gethostbyname(szLocalHostName);

	/* �� ��ǻ�Ϳ��� ���� IP�� �Ҵ� ���� �� �ֽ��ϴ�. �̸� ��� ����մϴ�. */
	for (int i = 0; pLocalHostInformation->h_addr_list[i] != NULL; i++) {
		printf("hostent.h_addr_list[%d] = %s\n", i, inet_ntoa(*(struct in_addr*)pLocalHostInformation->h_addr_list[i]));
	}
}


void CNetwork::startServer()
{
	m_vpThreadlist.push_back(new thread{ mem_fun(&CNetwork::acceptThread), this });
	printf("startServer() \n");
	printHostInfo();
}

void CNetwork::acceptThread()
{
	m_listenSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	
	// ���̱۾˰���
	BOOL optval = TRUE;
	setsockopt(m_listenSock, IPPROTO_TCP, TCP_NODELAY, (char*)&optval, sizeof(optval));

	if (m_listenSock == INVALID_SOCKET) CNetwork::err_quit("WSASocket() error!");

	int nSendSize = 0;
	int nIntSize = sizeof(int);
	nSendSize = 1048576;
	setsockopt(m_listenSock, SOL_SOCKET, SO_SNDBUF, (char*)&nSendSize, nIntSize);
	//printf("�۽� ������ ũ��: %d bytes \n", nSendSize);

	//�ּ� bind()
	sockaddr_in	listenSockAddr;
	ZeroMemory(&listenSockAddr, sizeof(listenSockAddr));
	listenSockAddr.sin_family = AF_INET;
	listenSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	listenSockAddr.sin_port = htons(SERVER_PORT);


	int retval = ::bind(m_listenSock, (sockaddr*)&listenSockAddr, sizeof(listenSockAddr));
	if (retval == SOCKET_ERROR) {
		err_quit("Bind() error!");
	}

	//listen()
	retval = listen(m_listenSock, SOMAXCONN);
	if (retval == SOCKET_ERROR) {
		err_quit("Listen() error!");
	}

	// ������ ��ſ� ����� ����
	SOCKET			clientSock;
	SOCKADDR_IN		clientAddr;
	int				flags = 0;
	int				addrLen;

	while (1) {

		addrLen = sizeof(clientAddr);
		clientSock = accept(m_listenSock, (SOCKADDR*)&clientAddr, &addrLen);
		if (clientSock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}

		// ��������
		// !! (�߰��ʿ�) �ش� �÷��̾�� �� ����� �˷��� ��
		if (m_bPlaying || m_nPlayerCount >= MAX_PLAYER_CNT) {
			closesocket(clientSock);
			printf("-> ���ο� Ŭ���̾�Ʈ�� ���� ���� \n\n");
			continue;
		}

		SOCKETINFO *pSocketInfo = new SOCKETINFO;
		if (pSocketInfo == NULL) break;
		pSocketInfo->m_sock = clientSock;

		for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
			if (m_vpClientInfo[i] == nullptr) {
				pSocketInfo->m_nID = i;
				m_vpClientInfo[i] = pSocketInfo;
				++m_nPlayerCount;
				break;
			}
		}

		m_vpThreadlist.push_back(new thread{ mem_fun(&CNetwork::recvThread), this, pSocketInfo->m_nID });

	}

}

void CNetwork::recvThread(int id)
{
	printf("-> %d�� Ŭ�� ���ӿ� ���� ���ú꽺������� \n\n", id);
	int retval;
	int iCurrPacketSize = 0;
	int iStoredPacketSize = 0;
	char recvBuf[MAX_PACKET_SIZE] = { 0 };

	while (1) {

		ZeroMemory(recvBuf, sizeof(recvBuf));
		retval = recv(m_vpClientInfo[id]->m_sock, recvBuf, sizeof(recvBuf), 0);

		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			Logout(id);
			break;
		}

		char *pRecvBuf = recvBuf;

		while (0 < retval) {

			if (0 == iCurrPacketSize) {

				if (retval + iStoredPacketSize >= sizeof(HEADER)) {
					int restHeaderSize = sizeof(HEADER) - iStoredPacketSize;
					memcpy(m_vpClientInfo[id]->m_saveBuf + iStoredPacketSize, pRecvBuf, restHeaderSize);
					pRecvBuf += restHeaderSize;
					iStoredPacketSize += restHeaderSize;
					retval -= restHeaderSize;
					iCurrPacketSize = ((HEADER*)m_vpClientInfo[id]->m_saveBuf)->packetSize;
				}
				else {
					memcpy(m_vpClientInfo[id]->m_saveBuf + iStoredPacketSize, pRecvBuf, retval);
					iStoredPacketSize += retval;
					retval = 0;
					break;
				}

			}

			int restSize = iCurrPacketSize - iStoredPacketSize;

			if (restSize <= retval) {
				memcpy(m_vpClientInfo[id]->m_saveBuf + iStoredPacketSize, pRecvBuf, restSize);
				packetProcess(id);
				iCurrPacketSize = iStoredPacketSize = 0;
				pRecvBuf += restSize;
				retval -= restSize;
			}
			else {
				memcpy(m_vpClientInfo[id]->m_saveBuf + iStoredPacketSize, pRecvBuf, retval);
				iStoredPacketSize += retval;
				retval = 0;
			}

		}
	}
}

void CNetwork::packetProcess(int id)
{
	HEADER *pHeader = (HEADER*)m_vpClientInfo[id]->m_saveBuf;
	switch (pHeader->packetID)
	{
	case PAK_LOGIN:
		Login(id);
		break;
	case PAK_RMV:
		Logout(id);
		break;
	case PAK_READY:
		Ready(id);
		break;
	case PAK_KEY_DOWN: case PAK_KEY_UP:
		Key(id, m_vpClientInfo[id]->m_saveBuf);
		break;
	default:
		printf("�˷����� ���� ��ŶID \n");
		break;
	}
}

bool CNetwork::Login(int id)
{

	UCHAR sendData[MAX_PACKET_SIZE] = { 0 };
	SC_LOG_INOUT *pData = (SC_LOG_INOUT*)(sendData);
	pData->header.packetSize = sizeof(SC_LOG_INOUT);
	pData->header.packetID = PAK_LOGIN;
	pData->ID = id;
	pData->clientNum = m_nPlayerCount;
	pData->readyCount = m_nReadyCount;

	// ���̵��ȣ �ο�
	transmitProcess(sendData, id);

	// �ٸ� �÷��̾�� ���� ����� �˸�
	pData->header.packetID = PAK_REG;
	for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
		if (m_vpClientInfo[i] && m_vpClientInfo[i]->m_nID != id) {
			transmitProcess(sendData, m_vpClientInfo[i]->m_nID);
		}
	}

	printf("-> %d�� Ŭ���̾�Ʈ ���� \n", id);
	printf("-> �غ����( %d / %d ), �� ������: %d \n", m_nReadyCount, MAX_PLAYER_CNT, m_nPlayerCount);

	return true;
}

bool CNetwork::Logout(int id)
{
	if (m_vpClientInfo[id] == NULL || m_vpClientInfo[id]->m_sock == NULL) return true;

	// ������ �ݴ´�.
	shutdown(m_vpClientInfo[id]->m_sock, SD_SEND);
	closesocket(m_vpClientInfo[id]->m_sock);
	m_vpClientInfo[id]->m_sock = NULL;
	m_nPlayerCount--;

	if (m_bPlaying) {
		// ����÷��̾�����
		if (m_nPlayerCount == 0) {
			m_bPlaying = false;
			m_nReadyCount = 0;
			for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
				delete m_vpClientInfo[i];
				m_vpClientInfo[i] = nullptr;
			}
			printf("-> ��� �÷��̾ ������ ����! \n\n");
			return true;
		}
	}
	else {
		// ������� ����
		if (m_vpClientInfo[id]->m_bReady) {
			m_vpClientInfo[id]->m_bReady = false;
			--m_nReadyCount;
		}
		delete m_vpClientInfo[id];
		m_vpClientInfo[id] = nullptr;
	}
	

	// �÷��̾�鿡�� �������� ����� �˸���.
	UCHAR sendData[MAX_PACKET_SIZE] = { 0 };
	SC_LOG_INOUT *pData = (SC_LOG_INOUT*)sendData;
	pData->header.packetSize = sizeof(SC_LOG_INOUT);
	pData->header.packetID = PAK_RMV;
	pData->ID = id;
	pData->clientNum = m_nPlayerCount;
	pData->readyCount = m_nReadyCount;

	for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
		if (m_vpClientInfo[i] && m_vpClientInfo[i]->m_sock) {
			transmitProcess(sendData, m_vpClientInfo[i]->m_nID);
		}
	}


	printf("-> %d�� Ŭ���̾�Ʈ ���� \n", id);
	printf("-> �غ����( %d / %d ), �� ������: %d \n\n", m_nReadyCount, MAX_PLAYER_CNT, m_nPlayerCount);

	return true;
}

bool CNetwork::Ready(int id)
{
	m_vpClientInfo[id]->m_bReady = true;
	++m_nReadyCount;

	UCHAR sendData[MAX_PACKET_SIZE] = { 0 };
	SC_LOG_INOUT *pData = (SC_LOG_INOUT*)sendData;
	pData->header.packetSize = sizeof(SC_LOG_INOUT);
	pData->header.packetID = PAK_READY;
	pData->clientNum = m_nPlayerCount;
	pData->readyCount = m_nReadyCount;
	pData->ID = id;

	for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
		if (m_vpClientInfo[i] && m_vpClientInfo[i]->m_sock) {
			transmitProcess(sendData, m_vpClientInfo[i]->m_nID);
		}
	}

	printf("-> %d�� Ŭ���̾�Ʈ �غ� \n", id);
	printf("-> �غ����( %d / %d ), �� ������: %d \n\n", m_nReadyCount, MAX_PLAYER_CNT, m_nPlayerCount);

	if (m_nReadyCount >= MAX_PLAYER_CNT) {
		return Start();
	}

	return true;
}

bool CNetwork::Start()
{
	UCHAR sendData[MAX_PACKET_SIZE] = { 0 };
	SC_START *pData = (SC_START*)sendData;
	pData->header.packetSize = sizeof(SC_START);
	pData->header.packetID = PAK_START;
	for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
		pData->ID_LIST[i] = m_vpClientInfo[i]->m_nID;
	}

	if (ob_num > 0) {
		DestroyWorld();
	}
	CreateWorld();

	m_bPlaying = true;
	for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
		if (m_vpClientInfo[i] && m_vpClientInfo[i]->m_sock) {
			transmitProcess(sendData, m_vpClientInfo[i]->m_nID);
		}
	}

	printf("-> ���� ����! \n\n");
	return true;
}

bool CNetwork::Key(int id, void *buf) {

	CS_KEY *pKey = (CS_KEY*)((CHAR*)buf);
	UCHAR sendData[MAX_PACKET_SIZE] = { 0 };
	SC_KEY *pData = (SC_KEY*)sendData;
	pData->header.packetSize = sizeof(SC_KEY);
	pData->ID = id;
	pData->key = pKey->key;
	pData->header.packetID = pKey->header.packetID;

	for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
		if (m_vpClientInfo[i] && m_vpClientInfo[i]->m_sock) {
			if (m_vpClientInfo[i]->m_nID == id) continue;
			transmitProcess(sendData, m_vpClientInfo[i]->m_nID);
		}
	}

	if (pData->header.packetID == PAK_KEY_DOWN) {
		//printf("%d��Ŭ�� %dŰ�� Down! \n", id, pData->key);
		for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
			if (m_vpClientInfo[i]->m_nID == id) {
				m_vpClientInfo[i]->m_pSheep->special_key(pKey->key, obstacles);
				m_vpClientInfo[i]->m_pSheep->pCamera->keyboard(pKey->key);
				break;
			}
		}
	}
	else if (pData->header.packetID == PAK_KEY_UP) {
		//printf("%d��Ŭ�� %dŰ�� Up! \n", id, pData->key);
		for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
			if (m_vpClientInfo[i]->m_nID == id) {
				m_vpClientInfo[i]->m_pSheep->special_key_up(pKey->key);
				break;
			}
		}
	}

	return true;
}

bool CNetwork::Hurt(int id)
{
	UCHAR sendData[MAX_PACKET_SIZE] = { 0 };
	SC_EVENT *pData = (SC_EVENT*)sendData;
	pData->header.packetSize = sizeof(SC_EVENT);
	pData->header.packetID = PAK_HURT;
	pData->ID = id;

	for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
		if (m_vpClientInfo[i] && m_vpClientInfo[i]->m_sock) {
			transmitProcess(sendData, m_vpClientInfo[i]->m_nID);
		}
	}

	return true;
}

bool CNetwork::Finish(int id) {

	for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
		if (m_vpClientInfo[i] && m_vpClientInfo[i]->m_sock) {
			m_vpClientInfo[i]->m_bReady = false;
		}
	}
	m_nReadyCount = 0;
	m_bPlaying = false;

	printf("-> ���ڴ� %d�� Ŭ��! \n", id);

	UCHAR sendData[MAX_PACKET_SIZE] = { 0 };
	SC_EVENT *pData = (SC_EVENT*)sendData;
	pData->header.packetSize = sizeof(SC_EVENT);
	pData->header.packetID = PAK_ENDING;
	pData->ID = id;

	for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
		if (m_vpClientInfo[i] && m_vpClientInfo[i]->m_sock) {
			transmitProcess(sendData, m_vpClientInfo[i]->m_nID);
		}
	}

	return true;
}

bool CNetwork::Sync()
{
	UCHAR sendData[MAX_PACKET_SIZE] = { 0 };
	SC_SYNC *pData = (SC_SYNC*)sendData;
	pData->header.packetSize = sizeof(SC_SYNC);
	pData->header.packetID = PAK_SYNC;

	for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
		pData->sheep_ID[i] = m_vpClientInfo[i]->m_nID;
		pData->sheep_pos[i].x = m_vpClientInfo[i]->m_pSheep->x;
		pData->sheep_pos[i].y = m_vpClientInfo[i]->m_pSheep->y;
		pData->sheep_pos[i].z = m_vpClientInfo[i]->m_pSheep->z;
		memcpy(pData->state[i], m_vpClientInfo[i]->m_pSheep->state, sizeof(pData->state[i]));
	}

	for (int i = 0; i < MOVING_OB_CNT; ++i) {
		pData->object_pos[i].x = m_vpMovingObject[i]->x;
		pData->object_pos[i].y = m_vpMovingObject[i]->y;
		pData->object_pos[i].z = m_vpMovingObject[i]->z;
	}

	for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
		if (m_vpClientInfo[i] && m_vpClientInfo[i]->m_sock) {
			transmitProcess(sendData, m_vpClientInfo[i]->m_nID);
		}
	}

	return true;
}

bool CNetwork::GameOver()
{
	for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
		if (m_vpClientInfo[i] && m_vpClientInfo[i]->m_sock) {
			m_vpClientInfo[i]->m_bReady = false;
		}
	}
	m_nReadyCount = 0;
	m_bPlaying = false;

	printf("-> ��� �÷��̾� ���ӿ��� \n");

	UCHAR sendData[MAX_PACKET_SIZE] = { 0 };
	SC_EVENT *pData = (SC_EVENT*)sendData;
	pData->header.packetSize = sizeof(SC_EVENT);
	pData->header.packetID = PAK_GAMEOVER;

	for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
		if (m_vpClientInfo[i] && m_vpClientInfo[i]->m_sock) {
			transmitProcess(sendData, m_vpClientInfo[i]->m_nID);
		}
	}

	return true;
}


void CNetwork::transmitProcess(void* buf, int id)
{
	SOCKET clientSocket = m_vpClientInfo[id]->m_sock;
	if (!clientSocket)
		err_display("Wrong Sock access!!");
	
	int packetSize = ((HEADER *)buf)->packetSize;
	int retval = send(clientSocket, (char*)buf, packetSize, 0);
	
	if (retval == SOCKET_ERROR) {
		err_display("[CNetworkManager::sendPacket()] WSASend");
		Logout(id);
		return;
	}
}

void CNetwork::endServer()
{
	for (auto &data : m_vpThreadlist) {
		data->join();
	}
	WSACleanup();

	printf("endServer()\n");
}


void CNetwork::DestroyWorld() {

	for (auto g : ground)
		delete g;
	delete mother_sheep;
	for (int i = 0; i < ob_num; ++i)
		delete obstacles[i];
	m_vpMovingObject.clear();
	ob_num = 0;
}

void CNetwork::updateServer()
{
	if (!m_bPlaying) return;

	float frameTime = clock() - m_fCurrentTime;
	m_fCurrentTime = clock();
	m_fAccumulator += frameTime;

	while (m_fAccumulator >= FIXED_FRAME_TIME) {

		frameTime = FIXED_FRAME_TIME;

		// ������ �翡 ���� ī�޶�, ���ĵ� ������Ʈ
		for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
			//ī�޶� ������Ʈ
			auto sheep = m_vpClientInfo[i]->m_pSheep;
			m_pSheeps[i] = sheep;

			if (!sheep->killed) {
				sheep->pCamera->update(frameTime);
			}

			//��ü ������Ʈ (+���ĵ� ���� Ȯ��)
			sheep->stading_index = -1;
			for (int i = 0; i < ob_num; ++i) {
				if (sheep->stading_index == -1 && obstacles[i]->is_standing(sheep)) {
					sheep->stading_index = i;
				}
			}
		}

		// ��ֹ� ������Ʈ
		for (int i = 0; i < ob_num; ++i) {
			if (obstacles[i]->type == BLACK_SHEEP) {
				obstacles[i]->update3(m_pSheeps, obstacles, frameTime);
			}
			else
				obstacles[i]->update1(m_pSheeps, frameTime);
		};

		//�� ������Ʈ
		int deathCount = 0;
		for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
			
			if (m_pSheeps[i]->isHurted) {
				m_pSheeps[i]->isHurted = false;
				Hurt(m_vpClientInfo[i]->m_nID);
			}

			switch (m_pSheeps[i]->iGameMode) {

			case PLAY_MODE:
				m_pSheeps[i]->update2(ground[0], obstacles, frameTime);
				break;
			case GAME_OVER:
				if (!m_pSheeps[i]->killed) {
					m_pSheeps[i]->dead_update(frameTime);
				}
				else {
					deathCount++;
				}
				break;
			case ENDING_MODE:
				Finish(m_vpClientInfo[i]->m_nID);
				break;
			}
		}

		if (deathCount >= MAX_PLAYER_CNT) {
			GameOver();
		}

		m_fAccumulator -= FIXED_FRAME_TIME;

		m_fSyncTime += FIXED_FRAME_TIME;
		if (MILISEC_PER_SYNC < m_fSyncTime) {
			m_fSyncTime -= MILISEC_PER_SYNC;
			Sync();
		}
	}
}

void CNetwork::CreateWorld()
{

	// �⺻ ��ü
	ground[0] = new Ground(-200, -100, -100); // z�� -100~300
	for (int i = 1; i < GROUND_NUM; ++i) {
		ground[i] = new Ground(ground[0]->x + ground[0]->width*i, ground[0]->y, ground[0]->z);
	}

	mother_sheep = new MotherSheep();

	// ���� �Է�
	ob_num = 0;
	std::ifstream fin;
	fin.open("DATA.txt");
	if (fin.fail()) {
		err_display("File Error!");
	}
	m_vpMovingObject.reserve(MOVING_OB_CNT);
	while (!fin.eof())
	{
		int object_type;
		float xx, yy, zz, sspeed, max_xx, max_yy, max_zz;
		fin >> object_type >> xx >> yy >> zz >> sspeed >> max_xx >> max_yy >> max_zz;
		switch (object_type)
		{
		case BOX:
			obstacles[ob_num] = new Box(object_type, xx, yy, zz, sspeed, max_xx, max_yy, max_zz);
			break;
		case BRICK:
			obstacles[ob_num] = new Box(object_type, xx, yy, zz, sspeed, max_xx, max_yy, max_zz);
			m_vpMovingObject.push_back(obstacles[ob_num]);
			break;
		case BOXWALL:
			obstacles[ob_num] = new Box(object_type, xx, yy, zz, sspeed, max_xx, max_yy, max_zz);
			break;
		case SCISSORS:
			obstacles[ob_num] = new Scissors(object_type, xx, yy, zz, sspeed, max_xx, max_yy, max_zz);
			m_vpMovingObject.push_back(obstacles[ob_num]);
			break;
		case PUMKIN:
			obstacles[ob_num] = new Pumkin(object_type, xx, yy, zz, sspeed, max_xx, max_yy, max_zz);
			m_vpMovingObject.push_back(obstacles[ob_num]);
			break;
		case HAY:
			obstacles[ob_num] = new Hay(object_type, xx, yy, zz, sspeed, max_xx, max_yy, max_zz);
			break;
		case BLACK_SHEEP:
			obstacles[ob_num] = new Black_Sheep(object_type, xx, yy, zz, sspeed, max_xx, max_yy, max_zz);
			m_vpMovingObject.push_back(obstacles[ob_num]);
			break;
		}
		++ob_num;
	}
	fin.close();

	for (int i = 0; i < MAX_PLAYER_CNT; ++i) {

		Camera *pCamera = new Camera();
		m_vpClientInfo[i]->m_pSheep = new Sheep(SHEEP, pCamera->x, 0, -30, 9);
		m_vpClientInfo[i]->m_pSheep->pCamera = pCamera;
		m_vpClientInfo[i]->m_pSheep->obCnt = ob_num;
	}

}