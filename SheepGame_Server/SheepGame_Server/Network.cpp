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
	if (m_listenSock == INVALID_SOCKET) CNetwork::err_quit("WSASocket() error!");

	int nSendSize = 0;
	int nIntSize = sizeof(int);
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
			printf("���ο� Ŭ���̾�Ʈ�� ���� ���� \n");
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
	printf("%d�� Ŭ�� ���ӿ� ���� ���ú꽺������� \n", id);
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
		printf("id %d���� ��Ŷ�� �޾ҽ��ϴ�. Ÿ��:%d \n", id, pHeader->packetID);
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

	// ������� ����
	if (m_vpClientInfo[id]->m_bReady) {
		m_vpClientInfo[id]->m_bReady = false;
		--m_nReadyCount;
	}


	// �÷�������
	if (m_nPlayerCount == 0) {
		m_bPlaying = false;
		for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
			delete m_vpClientInfo[i];
			m_vpClientInfo[i] = nullptr;
		}
		printf("��� �÷��̾ ������ ����! \n");
		return true;
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
	printf("-> �غ����( %d / %d ), �� ������: %d \n", m_nReadyCount, MAX_PLAYER_CNT, m_nPlayerCount);

	return true;
}

bool CNetwork::Ready(int id)
{
	printf("�����Լ�����");

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

	printf("-> %d�� Ŭ���̾�Ʈ �غ� \n", id);
	printf("-> �غ����( %d / %d ), �� ������: %d \n", m_nReadyCount, MAX_PLAYER_CNT, m_nPlayerCount);

	if (m_nReadyCount >= MAX_PLAYER_CNT) {
		printf("������ �����մϴ�(�̱���)\n");
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

