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
	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) 
		err_quit("윈속 초기화 실패");
	// 입출력 완료 포트 생성
	m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (m_hIOCP == NULL)
		err_quit("IOCP 생성 실패");

	for (int i = 0; i < MAX_ID_CNT; ++i){
		m_vpClientInfo.push_back(nullptr);
	}

	cout << "initNetwork()" << endl;

}
void CNetwork::startServer()
{
	// CPU 개수 확인
	SYSTEM_INFO si;
	GetSystemInfo(&si);

	m_vpThreadlist.push_back(new thread{ mem_fun(&CNetwork::acceptThread), this });
	for (DWORD i = 0; i < si.dwNumberOfProcessors * 2; ++i) {
		m_vpThreadlist.push_back(new thread{ mem_fun(&CNetwork::workerThread), this });
	}

	cout << "acceptThread와 " << si.dwNumberOfProcessors * 2 << "개의 workerThread 생성" << endl;
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
	char szLocalHostName[512]; // 이 컴퓨터가 네트워크에서 식별되는 이름이 저장될 문자열 버퍼입니다.
	struct hostent * pLocalHostInformation; // 로컬 호스트의 정보가 담길 구조체의 포인터

	gethostname(szLocalHostName, sizeof(szLocalHostName));
	printf("로컬 호스트의 이름은 \"%s\"입니다.\n", szLocalHostName);
	pLocalHostInformation = gethostbyname(szLocalHostName);

	/* 한 컴퓨터에서 여러 IP를 할당 받을 수 있습니다. 이를 모두 출력합니다. */
	for (int i = 0; pLocalHostInformation->h_addr_list[i] != NULL; i++){
		printf("hostent.h_addr_list[%d] = %s\n", i, inet_ntoa(*(struct in_addr*)pLocalHostInformation->h_addr_list[i]));
	}
}

bool CNetwork::acceptThread()
{
	m_listenSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	
	if (m_listenSock == INVALID_SOCKET) CNetwork::err_quit("WSASocket() error!");

	
	// 네이글알고리즘
	BOOL optval = TRUE;
	setsockopt(m_listenSock, IPPROTO_TCP, TCP_NODELAY, (char*)&optval, sizeof(optval));

	int nSendSize = 0;
	int nIntSize = sizeof(int);
	//getsockopt(m_listenSock, SOL_SOCKET, SO_SNDBUF, (char*)&nSendSize, &nIntSize);
	//printf("송신 버퍼의 크기: bytes %d \n", nSendSize);
	nSendSize = 1048576;
	setsockopt(m_listenSock, SOL_SOCKET, SO_SNDBUF, (char*)&nSendSize, nIntSize);
	printf("송신 버퍼의 크기: %d bytes \n", nSendSize);

	//주소 bind()
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

	// 데이터 통신에 사용할 변수
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

		// 접속차단
		// !! (추가필요) 해당 플레이어에게 그 사실을 알려야 함
		if (m_bPlaying || m_nPlayerCount >= MAX_PLAYER_CNT) {
			closesocket(clientSock);
			printf("새로운 클라의 접속을 차단했습니다.\n\n");
			continue;
		}

		// 소켓 정보 구조체 할당
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

		//cout << "[시스템] " << pSocketInfo->nID << "번 클라 접속! IP주소:" << inet_ntoa(clientAddr.sin_addr) << ", 포트번호:" << ntohs(clientAddr.sin_port) << endl;

		// 소켓과 입출력 완료 포트 연결
		CreateIoCompletionPort((HANDLE)clientSock, m_hIOCP, pSocketInfo->nID, 0);	//핸들, 포트, 키값, 최대스레드(의미x)


		// 비동기 입출력 시작
		flags = 0;
		retval = WSARecv(clientSock, &pSocketInfo->wsabuf, 1, NULL, &flags, &pSocketInfo->overlapped, NULL);
		// (소켓, 버퍼, 버퍼개수, 읽은바이트수, 플래그, 오버랩, 컴플리션루틴)
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

		// 접속 종료 처리
		if (0 == IOsize){
			Logout(key);
			continue;
		}
		
		if (sockInfo->optype == OP_TYPE::OP_RECV){

			// 패킷조립 및 실행
			unsigned recvSize = IOsize;
			char *recvBuf = sockInfo->IOBuf;
			HEADER* pHeader = (HEADER*)recvBuf;
			while (0 < recvSize){

				//현재 처리하는 패킷이 없을 경우 recvBuf의 첫번째 바이트를 사이즈로 설정
				if (0 == sockInfo->iCurrPacketSize){
					sockInfo->iCurrPacketSize = pHeader->packetSize;
				}

				// 패킷을 만들기 위해 필요한 남은 사이즈 = 현재 받아야할 패킷사이즈 - 현재까지 저장한 패킷사이즈
				UINT restSize = sockInfo->iCurrPacketSize - sockInfo->iStoredPacketSize;

				// io로 받은 데이터의 크기가 패킷을 만들기 위해 필요한 사이즈보다 크거나 같은 경우 패킷을 조립한다.
				if (restSize <= recvSize){

					// 패킷버퍼에 패킷 사이즈를 채워 줄 만큼 
					memcpy(sockInfo->packetBuf + sockInfo->iStoredPacketSize, recvBuf, restSize);
					
					// 패킷처리
					if (!packetProcess(sockInfo->packetBuf, key)){
						Logout(key);
						continue;
					}

					sockInfo->iCurrPacketSize = sockInfo->iStoredPacketSize = 0;

					recvBuf += restSize;
					recvSize -= restSize;
				}
				else{
					// 처리할 만큼의 사이즈가 아닌 경우 패킷버퍼에 저장 해 놓음
					memcpy(sockInfo->packetBuf + sockInfo->iStoredPacketSize, recvBuf, recvSize);

					sockInfo->iStoredPacketSize += recvSize;
					recvSize = 0;
					//recvBuf += recvSize;
				}
			}

			// 다시 Recv()호출
			DWORD flags = 0;
	
			int retval = WSARecv(sockInfo->sock, &sockInfo->wsabuf, 1, NULL, &flags, (LPOVERLAPPED)sockInfo, NULL);
			if (retval == SOCKET_ERROR) {
				if (WSAGetLastError() != ERROR_IO_PENDING) {
					err_display("WSARecv()2");
				}
			}
		}
		else if (sockInfo->optype == OP_TYPE::OP_SEND){
			// Overlapped 구조체는 입출력이 완료되기 전까지 메모리에서 사라지면 안되므로 동적할당을 했었다.
			// 그러므로 Send()가 완료되었다면 반드시 메모리 해제를 해주어야 한다.
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
		printf("알려지지 않은 패킷ID \n");
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
	
	// 아이디번호 부여
	transmitProcess(sendData, id);

	// 다른 플레이어에게 접속 사실을 알림
	pData->header.packetID = PAK_REG;
	for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
		if (m_vpClientInfo[i] && m_vpClientInfo[i]->nID != id) {
			transmitProcess(sendData, m_vpClientInfo[i]->nID);
		}
	}

	printf("-> %d번 클라이언트 접속 \n", id);
	printf("-> 준비상태( %d / %d ), 총 접속자: %d \n\n", m_nReadyCount, MAX_PLAYER_CNT, m_nPlayerCount);

	return true;
}

bool CNetwork::Logout(int id)
{
	m_vpClientInfo[id]->socketLock.lock();
	if (m_vpClientInfo[id] == NULL || m_vpClientInfo[id]->sock == NULL) {
		m_vpClientInfo[id]->socketLock.unlock();
		return true;
	}

	// 소켓을 닫는다.
	shutdown(m_vpClientInfo[id]->sock, SD_SEND);
	closesocket(m_vpClientInfo[id]->sock);
	m_vpClientInfo[id]->sock = NULL;
	m_nPlayerCount--;

	if (m_bPlaying) {
		// 모든플레이어종료
		if (m_nPlayerCount == 0) {
			m_bPlaying = false;
			m_nReadyCount = 0;
			m_vpClientInfo[id]->socketLock.unlock();
			for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
				delete m_vpClientInfo[i];
				m_vpClientInfo[i] = nullptr;
			}
			printf("-> 모든 플레이어가 접속을 종료! \n\n");
			return true;
		}
	}
	else {
		// 레디상태 해제
		if (m_vpClientInfo[id]->bReady) {
			m_vpClientInfo[id]->bReady = false;
			--m_nReadyCount;
		}
		m_vpClientInfo[id]->socketLock.unlock();
		delete m_vpClientInfo[id];
		m_vpClientInfo[id] = nullptr;
	}


	// 플레이어들에게 접속종료 사실을 알린다.
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


	printf("-> %d번 클라이언트 종료 \n", id);
	printf("-> 준비상태( %d / %d ), 총 접속자: %d \n\n", m_nReadyCount, MAX_PLAYER_CNT, m_nPlayerCount);

	return true;
}

bool CNetwork::Ready(int id)
{
	printf("레디함수시작\n");
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

	printf("-> %d번 클라이언트 준비 \n", id);
	printf("-> 준비상태( %d / %d ), 총 접속자: %d \n\n", m_nReadyCount, MAX_PLAYER_CNT, m_nPlayerCount);

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

	printf("-> 게임 시작! \n\n");
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

	printf("-> 승자는 %d번 클라! \n",id);

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
		//printf("%d번클라가 %d키를 Down! \n", id, pData->key);
		for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
			if (m_vpClientInfo[i]->nID == id) {
				m_vpClientInfo[i]->pSheep->special_key(pKey->key,obstacles);
				m_vpClientInfo[i]->pSheep->pCamera->keyboard(pKey->key);
				break;
			}
		}
	}
	else if (pData->header.packetID == PAK_KEY_UP) {
		//printf("%d번클라가 %d키를 Up! \n", id, pData->key);
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

	printf("-> 모든 플레이어 게임오버 \n");

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

	// 기본 객체
	ground[0] = new Ground(-200, -100, -100); // z축 -100~300
	for (int i = 1; i < GROUND_NUM; ++i){
		ground[i] = new Ground(ground[0]->x + ground[0]->width*i, ground[0]->y, ground[0]->z);
	}

	mother_sheep = new MotherSheep();

	// 파일 입력
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

		// 각각의 양에 대한 카메라, 스탠딩 업데이트
		for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
			//카메라 업데이트
			auto sheep = m_vpClientInfo[i]->pSheep;
			m_pSheeps[i] = sheep;

			if (!sheep->killed) {
				sheep->pCamera->update(frameTime);
			}

			//객체 업데이트 (+스탠딩 상태 확인)
			sheep->stading_index = -1;
			for (int i = 0; i < ob_num; ++i) {
				if (sheep->stading_index == -1 && obstacles[i]->is_standing(sheep)) {
					sheep->stading_index = i;
				}
			}
		}

		// 장애물 업데이트
		for (int i = 0; i < ob_num; ++i) {
			if (obstacles[i]->type == BLACK_SHEEP) {
				obstacles[i]->update3(m_pSheeps, obstacles, frameTime);
			}
			else
				obstacles[i]->update1(m_pSheeps, frameTime);
		};

		//양 업데이트
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
