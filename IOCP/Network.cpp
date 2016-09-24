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
	isReady = false;

}

SOCKETINFO::~SOCKETINFO()
{
}


CNetwork::CNetwork()
{
	m_listenSock = NULL;
	m_hIOCP = NULL;
	m_nID = 0;
	m_nReadyCount = 0;
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
		if (m_nID >= MAX_ID_CNT) {
			closesocket(clientSock);
			cout << "[시스템] 벡터 " << MAX_ID_CNT << "칸을 초과하여 새로운 접속차단" << endl;
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
		pSocketInfo->nID = m_nID;

		m_vpClientInfo[m_nID++] = pSocketInfo;
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

			while (0 < recvSize){

				//현재 처리하는 패킷이 없을 경우 recvBuf의 첫번째 바이트를 사이즈로 설정
				if (0 == sockInfo->iCurrPacketSize){
					sockInfo->iCurrPacketSize = recvBuf[0];
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
	//cout << "패킷 처리"<< (int)buf[1] << endl;
	bool issuccess = true;

	switch (buf[1])
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

	}
	return issuccess;
}

bool CNetwork::Login(int id)
{
	UCHAR sendData[MAX_PACKET_SIZE] = { 0 };
	SC_LOG_INOUT *pData = (SC_LOG_INOUT*)(sendData);
	pData->header.ucSize = sizeof(SC_LOG_INOUT);
	pData->header.byPacketID = PAK_LOGIN;
	pData->ID = id;
	pData->clientNum = m_nID;
	pData->readyCount = m_nReadyCount;
	
	// 아이디번호 부여
	transmitProcess(sendData, id);

	// 다른 플레이어에게 접속 사실을 알림
	pData->header.byPacketID = PAK_REG;
	for (auto client : m_vpClientInfo) {
		if (client && client->nID != id) {
			transmitProcess(sendData, client->nID);
		}
	}

	cout << "[시스템] " << id << "번 클라이언트 접속!" << endl;
	printf("레디 / 총 접속: ( %d / %d ) \n", m_nReadyCount, m_nID);

	return true;
}

bool CNetwork::Logout(int id)
{
	// 레디상태 해제
	if (m_vpClientInfo[id]->isReady) {
		m_vpClientInfo[id]->isReady = false;
		--m_nReadyCount;
	}

	// 소켓을 닫는다.
	closesocket(m_vpClientInfo[id]->sock);
	delete  m_vpClientInfo[id];
	m_vpClientInfo[id] = nullptr;

	// 플레이어들에게 접속종료 사실을 알린다.
	UCHAR sendData[MAX_PACKET_SIZE] = { 0 };
	SC_LOG_INOUT *pData = (SC_LOG_INOUT*)sendData;
	pData->header.ucSize = sizeof(SC_LOG_INOUT);
	pData->header.byPacketID = PAK_RMV;
	pData->ID = id;
	pData->clientNum = m_nID;
	pData->readyCount = m_nReadyCount;

	for (auto &data : m_vpClientInfo){
		if (data){
			transmitProcess(sendData, data->nID);
		}
	}

	cout << "[시스템] " << id << "번 클라이언트 접속 종료!" << endl;
	printf("레디 / 총 접속: ( %d / %d ) \n", m_nReadyCount, m_nID);

	return true;
}

bool CNetwork::Ready(int id)
{
	m_vpClientInfo[id]->isReady = true;
	++m_nReadyCount;

	UCHAR sendData[MAX_PACKET_SIZE] = { 0 };

	SC_LOG_INOUT *pData = (SC_LOG_INOUT*)sendData;
	pData->header.ucSize = sizeof(SC_LOG_INOUT);
	pData->header.byPacketID = PAK_READY;
	pData->clientNum = m_nID;
	pData->readyCount = m_nReadyCount;

	pData->header.byPacketID = (m_nReadyCount < MAX_READY_CNT) ? PAK_READY : PAK_START;
	for (auto &data : m_vpClientInfo) {
		if (data) {
			transmitProcess(sendData, data->nID);
		}
	}

	printf("레디 / 총 접속: ( %d / %d ) \n", m_nReadyCount, m_nID);

	return true;
}

bool CNetwork::Key(int id, void *buf) {

	CS_KEY *pKey = (CS_KEY*)((CHAR*)buf);

	UCHAR sendData[MAX_PACKET_SIZE] = { 0 };
	SC_KEY *pData = (SC_KEY*)sendData;
	pData->header.ucSize = sizeof(SC_KEY);
	pData->header.byPacketID = pKey->header.byPacketID;
	pData->ID = id;
	pData->key = pKey->key;

	// 다른 플레이어에게 키 타입과 값 전송
	for (auto client : m_vpClientInfo) {
		if (client && client->nID != id) {
			transmitProcess(sendData, client->nID);
		}
	}

	if (pKey->header.byPacketID == PAK_KEY_DOWN) {
		printf("%d번 클라가 %d 키를 Down! \n", id, pKey->key);
	}
	else if (pKey->header.byPacketID == PAK_KEY_UP) {
		printf("%d번 클라가 %d 키를 Up! \n", id, pKey->key);
	}

	return true;
}

bool CNetwork::syncData(void * buf, int id)
{
	

	return true;
}

void CNetwork::transmitProcess(void *buf, int id)
{
	SOCKETINFO *psock = new SOCKETINFO;
	UCHAR paksize = ((UCHAR*)buf)[0];

	memcpy(psock->IOBuf, buf, paksize);
	psock->optype = OP_TYPE::OP_SEND;

	ZeroMemory(&psock->overlapped, sizeof(WSAOVERLAPPED));
	psock->wsabuf.buf = psock->IOBuf;
	psock->wsabuf.len = paksize;

	unsigned long IOsize;
	SOCKETINFO* sock = m_vpClientInfo[id];
	if (!sock)
		err_display("Wrong Sock access!!");

	int retval = WSASend(sock->sock, &psock->wsabuf, 1, &IOsize, NULL, &psock->overlapped, NULL);

	if (retval == SOCKET_ERROR){
		int err_code = WSAGetLastError();
		if (WSA_IO_PENDING != err_code){
			err_display("[CNetworkManager::sendPacket()] WSASend");
			printf("Error Code: %d \n", WSAGetLastError());
			return;
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
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCWSTR)msg, MB_ICONERROR);
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
	printf("[%s] %s", msg, (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}
