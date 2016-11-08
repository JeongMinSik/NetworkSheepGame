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
	//MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCWSTR)msg, MB_ICONERROR);
	printf("[%s] %s", msg, lpMsgBuf);
	LocalFree(lpMsgBuf);
}

void CNetwork::initNetwork()
{
	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		err_quit("윈속 초기화 실패");

	for (int i = 0; i < MAX_ID_CNT; ++i) {
		m_vpClientInfo.push_back(nullptr);
	}

	printf("initNetwork() \n");

}

void CNetwork::printHostInfo() {
	char szLocalHostName[512]; // 이 컴퓨터가 네트워크에서 식별되는 이름이 저장될 문자열 버퍼입니다.
	struct hostent * pLocalHostInformation; // 로컬 호스트의 정보가 담길 구조체의 포인터

	gethostname(szLocalHostName, sizeof(szLocalHostName));
	printf("로컬 호스트의 이름은 \"%s\"입니다.\n", szLocalHostName);
	pLocalHostInformation = gethostbyname(szLocalHostName);

	/* 한 컴퓨터에서 여러 IP를 할당 받을 수 있습니다. 이를 모두 출력합니다. */
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
	if (m_listenSock == INVALID_SOCKET) CNetwork::err_quit("WSASocket() error!");

	int nSendSize = 0;
	int nIntSize = sizeof(int);
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
	if (retval == SOCKET_ERROR) {
		err_quit("Bind() error!");
	}

	//listen()
	retval = listen(m_listenSock, SOMAXCONN);
	if (retval == SOCKET_ERROR) {
		err_quit("Listen() error!");
	}

	// 데이터 통신에 사용할 변수
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

		// 접속차단
		// !! (추가필요) 해당 플레이어에게 그 사실을 알려야 함
		if (m_bPlaying || m_nPlayerCount >= MAX_PLAYER_CNT) {
			closesocket(clientSock);
			printf("새로운 클라이언트의 접속 차단 \n");
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
	printf("%d번 클라 접속에 따른 리시브스레드생성 \n", id);
	int retval;
	int iCurrPacketSize = 0;
	int iStoredPacketSize = 0;
	char recvBuf[MAX_PACKET_SIZE] = { 0 };

	while (1) {

		ZeroMemory(recvBuf, sizeof(recvBuf));
		retval = recv(m_vpClientInfo[id]->m_sock, recvBuf, sizeof(recvBuf), 0);

		if (retval == SOCKET_ERROR) {
			err_display("recv()");
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
		printf("id %d에게 패킷을 받았습니다. 타입:%d \n", id, pHeader->packetID);
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
		//Key(id, buf);
		break;
	case PAK_ENDING:
		//Finish(id);
		break;
	default:
		printf("알려지지 않은 패킷ID \n");
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

	// 아이디번호 부여
	transmitProcess(sendData, id);

	// 다른 플레이어에게 접속 사실을 알림
	pData->header.packetID = PAK_REG;
	for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
		if (m_vpClientInfo[i] && m_vpClientInfo[i]->m_nID != id) {
			transmitProcess(sendData, m_vpClientInfo[i]->m_nID);
		}
	}

	printf("-> %d번 클라이언트 접속 \n", id);
	printf("-> 준비상태( %d / %d ), 총 접속자: %d \n", m_nReadyCount, MAX_PLAYER_CNT, m_nPlayerCount);

	return true;
}

bool CNetwork::Logout(int id)
{
	if (m_vpClientInfo[id] == NULL || m_vpClientInfo[id]->m_sock == NULL) return true;

	// 소켓을 닫는다.
	shutdown(m_vpClientInfo[id]->m_sock, SD_SEND);
	closesocket(m_vpClientInfo[id]->m_sock);
	m_vpClientInfo[id]->m_sock = NULL;
	m_nPlayerCount--;

	// 레디상태 해제
	if (m_vpClientInfo[id]->m_bReady) {
		m_vpClientInfo[id]->m_bReady = false;
		--m_nReadyCount;
	}


	// 플레이종료
	if (m_nPlayerCount == 0) {
		m_bPlaying = false;
		for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
			delete m_vpClientInfo[i];
			m_vpClientInfo[i] = nullptr;
		}
		printf("모든 플레이어가 접속을 종료! \n");
		return true;
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
		if (m_vpClientInfo[i] && m_vpClientInfo[i]->m_sock) {
			transmitProcess(sendData, m_vpClientInfo[i]->m_nID);
		}
	}


	printf("-> %d번 클라이언트 종료 \n", id);
	printf("-> 준비상태( %d / %d ), 총 접속자: %d \n", m_nReadyCount, MAX_PLAYER_CNT, m_nPlayerCount);

	return true;
}

bool CNetwork::Ready(int id)
{
	printf("레디함수시작");

	m_vpClientInfo[id]->m_bReady = true;
	++m_nReadyCount;

	UCHAR sendData[MAX_PACKET_SIZE] = { 0 };
	SC_LOG_INOUT *pData = (SC_LOG_INOUT*)sendData;
	pData->header.packetSize = sizeof(SC_LOG_INOUT);
	pData->header.packetID = PAK_READY;
	pData->clientNum = m_nPlayerCount;
	pData->readyCount = m_nReadyCount;

	for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
		if (m_vpClientInfo[i] && m_vpClientInfo[i]->m_sock) {
			transmitProcess(sendData, m_vpClientInfo[i]->m_nID);
		}
	}

	printf("-> %d번 클라이언트 준비 \n", id);
	printf("-> 준비상태( %d / %d ), 총 접속자: %d \n", m_nReadyCount, MAX_PLAYER_CNT, m_nPlayerCount);

	if (m_nReadyCount >= MAX_PLAYER_CNT) {
		printf("게임을 시작합니다(미구현)\n");
		return true;
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
		int err_code = WSAGetLastError();
		if (WSA_IO_PENDING != err_code) {
			err_display("[CNetworkManager::sendPacket()] WSASend");
			printf("Error Code: %d \n", WSAGetLastError());
			return;
		}
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


