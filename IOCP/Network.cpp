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

}

SOCKETINFO::~SOCKETINFO()
{
}


CNetwork::CNetwork()
{
	m_listenSock = NULL;
	m_hIOCP = NULL;
	m_nID = 0;
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
		if (m_nID >= MAX_ID_CNT) {
			closesocket(clientSock);
			cout << "[�ý���] ���� " << MAX_ID_CNT << "ĭ�� �ʰ��Ͽ� ���ο� ��������" << endl;
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
		pSocketInfo->nID = m_nID;

		m_vpClientInfo[m_nID++] = pSocketInfo;
		cout << "[�ý���] " << pSocketInfo->nID << "�� Ŭ�� ����! IP�ּ�:" << inet_ntoa(clientAddr.sin_addr) << ", ��Ʈ��ȣ:" << ntohs(clientAddr.sin_port) << endl;

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
			Logout(nullptr, key);
			continue;
		}
		
		if (sockInfo->optype == OP_TYPE::OP_RECV){

			// ��Ŷ���� �� ����
			unsigned recvSize = IOsize;
			char *recvBuf = sockInfo->IOBuf;

			while (0 < recvSize){

				//���� ó���ϴ� ��Ŷ�� ���� ��� recvBuf�� ù��° ����Ʈ�� ������� ����
				if (0 == sockInfo->iCurrPacketSize){
					sockInfo->iCurrPacketSize = recvBuf[0];
				}

				// ��Ŷ�� ����� ���� �ʿ��� ���� ������ = ���� �޾ƾ��� ��Ŷ������ - ������� ������ ��Ŷ������
				UINT restSize = sockInfo->iCurrPacketSize - sockInfo->iStoredPacketSize;

				// io�� ���� �������� ũ�Ⱑ ��Ŷ�� ����� ���� �ʿ��� ������� ũ�ų� ���� ��� ��Ŷ�� �����Ѵ�.
				if (restSize <= recvSize){

					// ��Ŷ���ۿ� ��Ŷ ����� ä�� �� ��ŭ 
					memcpy(sockInfo->packetBuf + sockInfo->iStoredPacketSize, recvBuf, restSize);
					
					// ��Ŷó��
					if (!packetProcess(sockInfo->packetBuf, key)){
						Logout(nullptr, key);
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
	//cout << "��Ŷ ó��"<< (int)buf[1] << endl;
	bool issuccess = true;

	switch (buf[1])
	{
	case PAK_SYNC:
		issuccess = syncData(buf, id);
		break;
	case PAK_ID:
		issuccess = Login(buf, id);
		break;
	}
	return issuccess;
}

bool CNetwork::Login(void * buf, int id)
{
	// ����ۼ�
	UCHAR sendData[MAX_PACKET_SIZE] = { 0 };
	HEADER *phead = (HEADER*)sendData;
	phead->byPacketID = PAK_ID;
	phead->ucSize = sizeof(HEADER) + 1; // ��� + ���̵��ȣ
	// �����ۼ�
	STOC_SYNC *pData = (STOC_SYNC*)(sendData + sizeof(HEADER));
	pData->ID = id;

	// ���̵��ȣ �ο�
	transmitProcess(sendData, id);

	phead->byPacketID = PAK_REG;
	phead->ucSize = sizeof(HEADER) + 1; // ��� + ���̵�
	// �ٸ� �÷��̾�� ���� ����� �˸�
	// �����ڵ忡���� ��ġ���� �� �ΰ������� �Ѱܾ� ��
	for (auto client : m_vpClientInfo) {
		if (client) {
			transmitProcess(sendData, client->nID);
		}
	}

	return true;
}

bool CNetwork::Logout(void * buf, int id)
{
	// ������ �ݴ´�.
	closesocket(m_vpClientInfo[id]->sock);
	delete  m_vpClientInfo[id];
	m_vpClientInfo[id] = nullptr;

	// �÷��̾�鿡�� �������� ����� �˸���.
	UCHAR sendData[MAX_PACKET_SIZE] = { 0 };
	HEADER *phead = (HEADER*)sendData;
	phead->byPacketID = PAK_RMV;
	phead->ucSize = sizeof(HEADER) + 1;
	STOC_SYNC *pdata1 = (STOC_SYNC*)(sendData + sizeof(HEADER));
	pdata1->ID = id;

	for (auto &data : m_vpClientInfo){
		if (data){
			transmitProcess(sendData, data->nID);
		}
	}

	cout << "[�ý���] " << id << "�� Ŭ���̾�Ʈ ���� ����!" << endl;

	return true;
}

bool CNetwork::syncData(void * buf, int id)
{
	CTOS_SYNC *recvData = (CTOS_SYNC*)((UCHAR*)buf + sizeof(HEADER));

	printf("[%d]�� Ŭ��κ��� ���� ������: %s \n", id, recvData->data);
	
	// �� �κп� ���� �����͸� ������ ����� �ؼ� �� ����� Ŭ��鿡�� �־�� �Ѵ�.
	// ����� ���� �����͸� �״�� ��� Ŭ�󿡰� �����Ѵ�.
	
	// ����ۼ�
	UCHAR sendData[MAX_PACKET_SIZE] = { 0 };
	HEADER *phead = (HEADER*)sendData;
	phead->byPacketID = PAK_SYNC;
	phead->ucSize = (UCHAR)(sizeof(HEADER) + strlen(recvData->data) + 2); // ��� + ���۳��� + 2(���̵� + ��)
	// �����ۼ�
	STOC_SYNC *pData = (STOC_SYNC*)(sendData + sizeof(HEADER));
	pData->ID = id;
	memcpy(pData->data, recvData, strlen(recvData->data)+1);

	for (auto client : m_vpClientInfo) {
		if (client) {
			transmitProcess(sendData, client->nID);
		}
	}

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
