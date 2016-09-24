#include "Network.h"
#include "Protocol.h"

CNetwork::CNetwork()
{
	m_socket = NULL;
	m_pRecvThread = nullptr;
}

CNetwork::~CNetwork()
{
}

void CNetwork::err_quit(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	//MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	printf("[%s] %s", msg, (char *)lpMsgBuf);
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


void CNetwork::connectServer()
{
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		err_quit("윈속 초기화 실패");

	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_socket == INVALID_SOCKET) err_quit("socket()");

	// connect()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(SERVERADDR);
	serveraddr.sin_port = htons(SERVER_PORT);
	retval = connect(m_socket, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

	m_pRecvThread = new thread{ mem_fun(&CNetwork::recvThreadFunc),this };

	cout << "connectServer() Clear!" << endl;

	char sendData[MAX_PACKET_SIZE] = { 0 };
	HEADER *pData = (HEADER*)sendData;
	pData->ucSize = sizeof(HEADER);
	pData->byPacketID = PAK_LOGIN;

	retval = send(m_socket, sendData, sizeof(HEADER), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
	}
}

void CNetwork::endServer()
{
	m_pRecvThread->join();
	WSACleanup();

	cout << "endServer() Clear!" << endl;
}

void CNetwork::recvThreadFunc()
{
	char recvBuf[MAX_PACKET_SIZE] = { 0 };
	int retval;

	while (1) {

		// 데이터 받기
		ZeroMemory(recvBuf, sizeof(recvBuf));
		retval = recv(m_socket, recvBuf, sizeof(recvBuf), 0);

		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}

		// 패킷조립
		while (1) {

			if (m_nLeft == 0 && retval == 0) {
				break;
			}
			else if (sizeof(HEADER) <= m_nLeft + retval) {
				if (m_nLeft == 0) {
					memcpy(m_saveBuf, recvBuf, retval);
					m_nLeft = retval;
					retval = 0;
				}
				else {

					if (m_saveBuf[0] <= m_nLeft + retval) {

						if (m_saveBuf[0] <= m_nLeft) {
							// 패킷처리
							packetUnpacker();
						}
						else {
							int lack = m_saveBuf[0] - m_nLeft;
							memcpy(m_saveBuf + m_nLeft, recvBuf, lack);
							m_nLeft += lack;
							memmove(recvBuf, recvBuf + lack, retval - lack);
							retval -= lack;
						}
					}
					else {
						memcpy(m_saveBuf + m_nLeft, recvBuf, retval);
						m_nLeft += retval;
						return;
					}
				}
			}
			else {
				memcpy(m_saveBuf + m_nLeft, recvBuf, sizeof(retval));
				m_nLeft += retval;
				retval = 0;
			}
		}
	}
}

void CNetwork::packetUnpacker()
{

	//cout << "패킷아이디:" << (int)header.byPacketID << endl;
	//cout << "size:" << (int)m_saveBuf[0] << " ID: " << (int)m_saveBuf[1] << endl;
	switch (m_saveBuf[1])
	{
	case PAK_LOGIN:
	{
		SC_LOG_INOUT login;
		memcpy(&login, m_saveBuf, sizeof(SC_LOG_INOUT));
		m_nID = login.ID;
		cout << "부여받은 아이디는 " << m_nID << "번 \n";
		break;
	}
	case PAK_REG:
	{
		SC_LOG_INOUT login;
		memcpy(&login, m_saveBuf, sizeof(SC_LOG_INOUT));
		m_nID = login.ID;
		cout << "신규 접속 아이디는 " << m_nID << "번 \n";
		break;
	}
	case PAK_RMV:
	{
		SC_LOG_INOUT login;
		memcpy(&login, m_saveBuf, sizeof(SC_LOG_INOUT));
		m_nID = login.ID;
		cout << "로그아웃 아이디는 " << m_nID << "번 \n";
		break;
	}
	default:
		cout << "패킷 ID오류:" <<m_saveBuf[1]<< endl;
		break;
	}
	m_nLeft -= m_saveBuf[0];
	memmove(m_saveBuf, m_saveBuf + m_saveBuf[0], m_nLeft);

}
