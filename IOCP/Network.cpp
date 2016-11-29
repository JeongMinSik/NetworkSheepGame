#include "Network.h"

SOCKETINFO::SOCKETINFO()
{
	ZeroMemory(&overlapped, sizeof(overlapped));
	ZeroMemory(&wsabuf, sizeof(wsabuf));
	ZeroMemory(IOBuf, sizeof(MAX_PACKET_SIZE));
	ZeroMemory(packetBuf, sizeof(MAX_PACKET_SIZE));
	sock = NULL;
	optype = OP_TYPE::OP_RECV;
	iCurrPacketSize = iStoredPacketSize = 0;
	bReady = false;

}
SOCKETINFO::~SOCKETINFO()
{
}

CNetwork::CNetwork()
{
	m_listenSock = NULL;
	m_hIOCP = NULL;
	m_nPlayerCount = 0;
	m_nReadyCount = 0;
	m_bPlaying = false;
	m_fCurrentTime = clock();
	m_fAccumulator = 0.0f;
	m_fSyncTime = 0.0f;
	m_fSyncObjectTime = 0.0f;
}
CNetwork::~CNetwork()
{
}

void CNetwork::initNetwork()
{
	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) 
		err_quit("���� �ʱ�ȭ ����");
	// ����� �Ϸ� ��Ʈ ����
	m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (m_hIOCP == NULL)
		err_quit("IOCP ���� ����");

	for (int i = 0; i < MAX_ID_CNT; ++i){
		m_vpClientInfo.push_back(nullptr);
	}

	cout << "initNetwork()" << endl;

}
void CNetwork::startServer()
{
	// CPU ���� Ȯ��
	SYSTEM_INFO si;
	GetSystemInfo(&si);

	m_vpThreadlist.push_back(new thread{ mem_fun(&CNetwork::acceptThread), this });
	for (DWORD i = 0; i < si.dwNumberOfProcessors * 2; ++i) {
		m_vpThreadlist.push_back(new thread{ mem_fun(&CNetwork::workerThread), this });
	}

	cout << "acceptThread�� " << si.dwNumberOfProcessors * 2 << "���� workerThread ����" << endl;
	cout << "startServer()" << endl;

	printHostInfo();
}
void CNetwork::endServer()
{
	for (auto &data : m_vpThreadlist){
		data->join();
	}
	WSACleanup();

	cout << "endServer()" << endl;
}
void CNetwork::printHostInfo() {
	char szLocalHostName[512]; // �� ��ǻ�Ͱ� ��Ʈ��ũ���� �ĺ��Ǵ� �̸��� ����� ���ڿ� �����Դϴ�.
	struct hostent * pLocalHostInformation; // ���� ȣ��Ʈ�� ������ ��� ����ü�� ������

	gethostname(szLocalHostName, sizeof(szLocalHostName));
	printf("���� ȣ��Ʈ�� �̸��� \"%s\"�Դϴ�.\n", szLocalHostName);
	pLocalHostInformation = gethostbyname(szLocalHostName);

	/* �� ��ǻ�Ϳ��� ���� IP�� �Ҵ� ���� �� �ֽ��ϴ�. �̸� ��� ����մϴ�. */
	for (int i = 0; pLocalHostInformation->h_addr_list[i] != NULL; i++){
		printf("hostent.h_addr_list[%d] = %s\n", i, inet_ntoa(*(struct in_addr*)pLocalHostInformation->h_addr_list[i]));
	}
}

bool CNetwork::acceptThread()
{
	m_listenSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	
	if (m_listenSock == INVALID_SOCKET) CNetwork::err_quit("WSASocket() error!");

	
	// ���̱۾˰���
	BOOL optval = TRUE;
	setsockopt(m_listenSock, IPPROTO_TCP, TCP_NODELAY, (char*)&optval, sizeof(optval));

	int nSendSize = 0;
	int nIntSize = sizeof(int);
	//getsockopt(m_listenSock, SOL_SOCKET, SO_SNDBUF, (char*)&nSendSize, &nIntSize);
	//printf("�۽� ������ ũ��: bytes %d \n", nSendSize);
	nSendSize = 1048576;
	setsockopt(m_listenSock, SOL_SOCKET, SO_SNDBUF, (char*)&nSendSize, nIntSize);
	printf("�۽� ������ ũ��: %d bytes \n", nSendSize);

	//�ּ� bind()
	sockaddr_in	listenSockAddr;
	ZeroMemory(&listenSockAddr, sizeof(listenSockAddr));
	listenSockAddr.sin_family = AF_INET;
	listenSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	listenSockAddr.sin_port = htons(SERVER_PORT);


	int retval = ::bind(m_listenSock, (sockaddr*)&listenSockAddr, sizeof(listenSockAddr));
	if (retval == SOCKET_ERROR){
		err_quit("Bind() error!");
		return false;
	}

	//listen()
	retval = listen(m_listenSock, SOMAXCONN);
	if (retval == SOCKET_ERROR){
		err_quit("Listen() error!");
		return false;
	}

	// ������ ��ſ� ����� ����
	SOCKET			clientSock;
	SOCKADDR_IN		clientAddr;
	DWORD			flags = 0;
	int				addrLen;

	while (1) {

		// accept
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
			printf("���ο� Ŭ���� ������ �����߽��ϴ�.\n\n");
			continue;
		}

		// ���� ���� ����ü �Ҵ�
		SOCKETINFO *pSocketInfo = new SOCKETINFO;
		if (pSocketInfo == NULL) break;
		ZeroMemory(&pSocketInfo->overlapped, sizeof(pSocketInfo->overlapped));
		pSocketInfo->sock = clientSock;
		pSocketInfo->optype = OP_TYPE::OP_RECV;
		pSocketInfo->wsabuf.buf = pSocketInfo->IOBuf;
		pSocketInfo->wsabuf.len = MAX_PACKET_SIZE;

		for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
			if (m_vpClientInfo[i] == nullptr) {
				pSocketInfo->nID = i;
				m_vpClientInfo[i] = pSocketInfo;
				++m_nPlayerCount;
				break;
			}
		}

		//cout << "[�ý���] " << pSocketInfo->nID << "�� Ŭ�� ����! IP�ּ�:" << inet_ntoa(clientAddr.sin_addr) << ", ��Ʈ��ȣ:" << ntohs(clientAddr.sin_port) << endl;

		// ���ϰ� ����� �Ϸ� ��Ʈ ����
		CreateIoCompletionPort((HANDLE)clientSock, m_hIOCP, pSocketInfo->nID, 0);	//�ڵ�, ��Ʈ, Ű��, �ִ뽺����(�ǹ�x)


		// �񵿱� ����� ����
		flags = 0;
		retval = WSARecv(clientSock, &pSocketInfo->wsabuf, 1, NULL, &flags, &pSocketInfo->overlapped, NULL);
		// (����, ����, ���۰���, ��������Ʈ��, �÷���, ������, ���ø��Ƿ�ƾ)
		if (retval == SOCKET_ERROR) {
			if (WSAGetLastError() != ERROR_IO_PENDING) {
				err_display("WSARecv()1");
			}
			continue;
		}
	}

	return false;
}

void CNetwork::workerThread()
{
	DWORD IOsize;
	ULONG key;
	SOCKETINFO *sockInfo;

	while (1){

		GetQueuedCompletionStatus(m_hIOCP, &IOsize, &key, (LPOVERLAPPED *)&sockInfo, INFINITE);

		// ���� ���� ó��
		if (0 == IOsize){
			Logout(key);
			continue;
		}
		
		if (sockInfo->optype == OP_TYPE::OP_RECV){

			// ��Ŷ���� �� ����
			unsigned recvSize = IOsize;
			char *recvBuf = sockInfo->IOBuf;
			HEADER* pHeader = (HEADER*)recvBuf;
			while (0 < recvSize){

				//���� ó���ϴ� ��Ŷ�� ���� ��� recvBuf�� ù��° ����Ʈ�� ������� ����
				if (0 == sockInfo->iCurrPacketSize){
					sockInfo->iCurrPacketSize = pHeader->packetSize;
				}

				// ��Ŷ�� ����� ���� �ʿ��� ���� ������ = ���� �޾ƾ��� ��Ŷ������ - ������� ������ ��Ŷ������
				UINT restSize = sockInfo->iCurrPacketSize - sockInfo->iStoredPacketSize;

				// io�� ���� �������� ũ�Ⱑ ��Ŷ�� ����� ���� �ʿ��� ������� ũ�ų� ���� ��� ��Ŷ�� �����Ѵ�.
				if (restSize <= recvSize){

					// ��Ŷ���ۿ� ��Ŷ ����� ä�� �� ��ŭ 
					memcpy(sockInfo->packetBuf + sockInfo->iStoredPacketSize, recvBuf, restSize);
					
					// ��Ŷó��
					if (!packetProcess(sockInfo->packetBuf, key)){
						Logout(key);
						continue;
					}

					sockInfo->iCurrPacketSize = sockInfo->iStoredPacketSize = 0;

					recvBuf += restSize;
					recvSize -= restSize;
				}
				else{
					// ó���� ��ŭ�� ����� �ƴ� ��� ��Ŷ���ۿ� ���� �� ����
					memcpy(sockInfo->packetBuf + sockInfo->iStoredPacketSize, recvBuf, recvSize);

					sockInfo->iStoredPacketSize += recvSize;
					recvSize = 0;
					//recvBuf += recvSize;
				}
			}

			// �ٽ� Recv()ȣ��
			DWORD flags = 0;
	
			int retval = WSARecv(sockInfo->sock, &sockInfo->wsabuf, 1, NULL, &flags, (LPOVERLAPPED)sockInfo, NULL);
			if (retval == SOCKET_ERROR) {
				if (WSAGetLastError() != ERROR_IO_PENDING) {
					err_display("WSARecv()2");
				}
			}
		}
		else if (sockInfo->optype == OP_TYPE::OP_SEND){
			// Overlapped ����ü�� ������� �Ϸ�Ǳ� ������ �޸𸮿��� ������� �ȵǹǷ� �����Ҵ��� �߾���.
			// �׷��Ƿ� Send()�� �Ϸ�Ǿ��ٸ� �ݵ�� �޸� ������ ���־�� �Ѵ�.
			delete sockInfo;
		}
		else{
			exit(-1);
		}
	}
}

bool CNetwork::packetProcess(CHAR* buf, int id)
{
	
	bool issuccess = true;

	HEADER *pHeader = (HEADER*)buf;
	switch (pHeader->packetID)
	{
	case PAK_LOGIN:
		issuccess = Login(id);
		break;
	case PAK_RMV:
		issuccess = Logout(id);
		break;
	case PAK_READY:
		issuccess = Ready(id);
		break;
	case PAK_KEY_DOWN: case PAK_KEY_UP:
		issuccess = Key(id, buf);
		break;
	default:
		issuccess = false;
		printf("�˷����� ���� ��ŶID \n");
		break;
	}

	return issuccess;
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
		if (m_vpClientInfo[i] && m_vpClientInfo[i]->nID != id) {
			transmitProcess(sendData, m_vpClientInfo[i]->nID);
		}
	}

	printf("-> %d�� Ŭ���̾�Ʈ ���� \n", id);
	printf("-> �غ����( %d / %d ), �� ������: %d \n\n", m_nReadyCount, MAX_PLAYER_CNT, m_nPlayerCount);

	return true;
}

bool CNetwork::Logout(int id)
{
	m_vpClientInfo[id]->socketLock.lock();
	if (m_vpClientInfo[id] == NULL || m_vpClientInfo[id]->sock == NULL) {
		m_vpClientInfo[id]->socketLock.unlock();
		return true;
	}

	// ������ �ݴ´�.
	shutdown(m_vpClientInfo[id]->sock, SD_SEND);
	closesocket(m_vpClientInfo[id]->sock);
	m_vpClientInfo[id]->sock = NULL;
	m_nPlayerCount--;

	if (m_bPlaying) {
		// ����÷��̾�����
		if (m_nPlayerCount == 0) {
			m_bPlaying = false;
			m_nReadyCount = 0;
			m_vpClientInfo[id]->socketLock.unlock();
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
		if (m_vpClientInfo[id]->bReady) {
			m_vpClientInfo[id]->bReady = false;
			--m_nReadyCount;
		}
		m_vpClientInfo[id]->socketLock.unlock();
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
		if (m_vpClientInfo[i] && m_vpClientInfo[i]->sock) {
			transmitProcess(sendData, m_vpClientInfo[i]->nID);
		}
	}


	printf("-> %d�� Ŭ���̾�Ʈ ���� \n", id);
	printf("-> �غ����( %d / %d ), �� ������: %d \n\n", m_nReadyCount, MAX_PLAYER_CNT, m_nPlayerCount);

	return true;
}

bool CNetwork::Ready(int id)
{
	printf("�����Լ�����\n");
//	m_vpClientInfo[id]->socketLock.lock();

	m_vpClientInfo[id]->bReady = true;
	++m_nReadyCount;

//	m_vpClientInfo[id]->socketLock.unlock();

	UCHAR sendData[MAX_PACKET_SIZE] = { 0 };
	SC_LOG_INOUT *pData = (SC_LOG_INOUT*)sendData;
	pData->header.packetSize = sizeof(SC_LOG_INOUT);
	pData->header.packetID = PAK_READY;
	pData->clientNum = m_nPlayerCount;
	pData->readyCount = m_nReadyCount;
	pData->ID = id;

	for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
		if (m_vpClientInfo[i] && m_vpClientInfo[i]->sock) {
			transmitProcess(sendData, m_vpClientInfo[i]->nID);
		}
	}

	printf("-> %d�� Ŭ���̾�Ʈ �غ� \n", id);
	printf("-> �غ����( %d / %d ), �� ������: %d \n\n", m_nReadyCount, MAX_PLAYER_CNT, m_nPlayerCount);

	if (m_nReadyCount >= MAX_PLAYER_CNT) {
		return CNetwork::Start();
	}

	return true;
}

bool CNetwork::Start()
{
	UCHAR sendData[MAX_PACKET_SIZE] = { 0 };
	SC_START *pData = (SC_START*)sendData;
	pData->header.packetSize = sizeof(SC_START);
	pData->header.packetID = PAK_START;
	for (int i = 0; i < MAX_PLAYER_CNT; ++i){
		pData->ID_LIST[i] = m_vpClientInfo[i]->nID;
	}

	if (ob_num > 0) {
		DestroyWorld();
	}
	CreateWorld();

	m_bPlaying = true;
	for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
		if (m_vpClientInfo[i] && m_vpClientInfo[i]->sock) {
			transmitProcess(sendData, m_vpClientInfo[i]->nID);
		}
	}

	printf("-> ���� ����! \n\n");
	return true;
}

bool CNetwork::Finish(int id) {

	for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
		if (m_vpClientInfo[i] && m_vpClientInfo[i]->sock) {
			m_vpClientInfo[i]->bReady = false;
		}
	}
	m_nReadyCount = 0;
	m_bPlaying = false;

	printf("-> ���ڴ� %d�� Ŭ��! \n",id);

	UCHAR sendData[MAX_PACKET_SIZE] = { 0 };
	SC_EVENT *pData = (SC_EVENT*)sendData;
	pData->header.packetSize = sizeof(SC_EVENT);
	pData->header.packetID = PAK_ENDING;
	pData->ID = id;

	for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
		if (m_vpClientInfo[i] && m_vpClientInfo[i]->sock) {
			transmitProcess(sendData, m_vpClientInfo[i]->nID);
		}
	}

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
		if (m_vpClientInfo[i] && m_vpClientInfo[i]->sock) {
			if (m_vpClientInfo[i]->nID == id) continue;
			transmitProcess(sendData, m_vpClientInfo[i]->nID);
		}
	}

	if (pData->header.packetID == PAK_KEY_DOWN) {
		//printf("%d��Ŭ�� %dŰ�� Down! \n", id, pData->key);
		for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
			if (m_vpClientInfo[i]->nID == id) {
				m_vpClientInfo[i]->pSheep->special_key(pKey->key,obstacles);
				m_vpClientInfo[i]->pSheep->pCamera->keyboard(pKey->key);
				break;
			}
		}
	}
	else if (pData->header.packetID == PAK_KEY_UP) {
		//printf("%d��Ŭ�� %dŰ�� Up! \n", id, pData->key);
		for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
			if (m_vpClientInfo[i]->nID == id) {
				m_vpClientInfo[i]->pSheep->special_key_up(pKey->key);
				break;
			}
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
		pData->sheep_ID[i] = m_vpClientInfo[i]->nID;
		pData->sheep_pos[i].x = m_vpClientInfo[i]->pSheep->x;
		pData->sheep_pos[i].y = m_vpClientInfo[i]->pSheep->y;
		pData->sheep_pos[i].z = m_vpClientInfo[i]->pSheep->z;
		memcpy(pData->state[i], m_vpClientInfo[i]->pSheep->state, sizeof(pData->state[i]));
	}

	for (int i = 0; i < MOVING_OB_CNT; ++i) {
		pData->object_pos[i].x = m_vpMovingObject[i]->x;
		pData->object_pos[i].y = m_vpMovingObject[i]->y;
		pData->object_pos[i].z = m_vpMovingObject[i]->z;
	}

	for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
		if (m_vpClientInfo[i] && m_vpClientInfo[i]->sock) {
			transmitProcess(sendData, m_vpClientInfo[i]->nID);
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
		if (m_vpClientInfo[i] && m_vpClientInfo[i]->sock) {
			transmitProcess(sendData, m_vpClientInfo[i]->nID);
		}
	}

	return true;
}

bool CNetwork::GameOver()
{
	for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
		if (m_vpClientInfo[i] && m_vpClientInfo[i]->sock) {
			m_vpClientInfo[i]->bReady = false;
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
		if (m_vpClientInfo[i] && m_vpClientInfo[i]->sock) {
			transmitProcess(sendData, m_vpClientInfo[i]->nID);
		}
	}

	return true;
}

void CNetwork::transmitProcess(void *buf, int id)
{
	SOCKETINFO *psock = new SOCKETINFO;
	auto paksize = ((HEADER*)buf)->packetSize;

	memcpy(psock->IOBuf, buf, paksize);
	psock->optype = OP_TYPE::OP_SEND;

	ZeroMemory(&psock->overlapped, sizeof(WSAOVERLAPPED));
	psock->wsabuf.buf = psock->IOBuf;
	psock->wsabuf.len = paksize;

	unsigned long IOsize;
	m_vpClientInfo[id]->socketLock.lock();
	SOCKETINFO* sock = m_vpClientInfo[id];

	if (!sock)
		err_display("Wrong Sock access!!");

	int retval = WSASend(sock->sock, &psock->wsabuf, 1, &IOsize, NULL, &psock->overlapped, NULL);

	m_vpClientInfo[id]->socketLock.unlock();

	if (retval == SOCKET_ERROR){
		int err_code = WSAGetLastError();
		if (WSA_IO_PENDING != err_code){
			err_display("[CNetworkManager::sendPacket()] WSASend");
			printf("Error Code: %d \n", WSAGetLastError());
			return;
		}
	}
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

void CNetwork::CreateWorld()
{

	// �⺻ ��ü
	ground[0] = new Ground(-200, -100, -100); // z�� -100~300
	for (int i = 1; i < GROUND_NUM; ++i){
		ground[i] = new Ground(ground[0]->x + ground[0]->width*i, ground[0]->y, ground[0]->z);
	}

	mother_sheep = new MotherSheep();

	// ���� �Է�
	ob_num = 0;
	std::ifstream fin;
	fin.open("DATA.txt");
	m_vpMovingObject.reserve(MOVING_OB_CNT);
	if (fin.fail()) {
		err_display("File Error!");
	}
	while (!fin.eof())
	{
		int object_type, xx, yy, zz, sspeed, max_xx, max_yy, max_zz;
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
		m_vpClientInfo[i]->pSheep = new Sheep(SHEEP, pCamera->x, 0, -30, 9);
		m_vpClientInfo[i]->pSheep->pCamera = pCamera;
		m_vpClientInfo[i]->pSheep->obCnt = ob_num;
	}

}

void CNetwork::updateServer()
{
	if (!m_bPlaying) return;

	float frameTime = clock() - m_fCurrentTime;
	m_fCurrentTime = clock();
	m_fAccumulator += frameTime;
	//printf("FPS:%f \n", 1000.0 / frameTime);

	while (m_fAccumulator >= FIXED_FRAME_TIME) {

		frameTime = FIXED_FRAME_TIME;

		// ������ �翡 ���� ī�޶�, ���ĵ� ������Ʈ
		for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
			//ī�޶� ������Ʈ
			auto sheep = m_vpClientInfo[i]->pSheep;
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
				Hurt(m_vpClientInfo[i]->nID);
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
				Finish(m_vpClientInfo[i]->nID);
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
	//MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCWSTR)msg, MB_ICONERROR);
	printf("[%s] %s", msg, lpMsgBuf);
	LocalFree(lpMsgBuf);
}
