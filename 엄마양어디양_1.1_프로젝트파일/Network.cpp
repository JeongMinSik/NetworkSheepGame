#include "Network.h"
#include "stdafx.h"

CNetwork::CNetwork()
{
	m_socket = NULL;
	m_pRecvThread = nullptr;
	m_nPlayerCount = 1;
	m_piGameMode = nullptr;
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
	exit(0);
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
		m_Players[0].m_nID = login.ID;
		//printf("접속했습니다. 부여받은 아이디는 %d입니다. \n",m_nID);
		printf("접속했습니다. \n");
		printf("레디 / 총 접속: ( %d / %d ) \n", login.readyCount, login.clientNum);
		break;
	}
	case PAK_REG:
	{
		SC_LOG_INOUT login;
		memcpy(&login, m_saveBuf, sizeof(SC_LOG_INOUT));
		printf("%d번 클라가 접속했습니다. \n",login.ID);
		printf("레디 / 총 접속: ( %d / %d ) \n", login.readyCount, login.clientNum);
		break;
	}
	case PAK_RMV:
	{
		SC_LOG_INOUT logout;
		memcpy(&logout, m_saveBuf, sizeof(SC_LOG_INOUT));
		printf("%d번 클라가 로그아웃했습니다. \n", logout.ID);
		if (*m_piGameMode == READY_MODE) {
			printf("레디 / 총 접속: ( %d / %d ) \n", logout.readyCount, logout.clientNum);
		}
		for (int i = 0; i < m_nPlayerCount; ++i) {
			if (m_Players[i].m_nID == logout.ID) {
				m_Players[i].m_nID = -1;
			}
		}
		--m_nPlayerCount;
		break;
	}
	case PAK_READY:
	{
		SC_LOG_INOUT ready;
		memcpy(&ready, m_saveBuf, sizeof(SC_LOG_INOUT));
		printf("%d번 클라 준비완료! \n", ready.ID);
		printf("레디 / 총 접속: ( %d / %d ) \n", ready.readyCount, ready.clientNum);
		break;
	}
	case PAK_START:
	{
		SC_START start;
		memcpy(&start, m_saveBuf, sizeof(SC_START));
		printf("게임을 시작합니다. \n");
		*m_piGameMode = PLAY_MODE; 
		m_nPlayerCount = MAX_PLAYER_CNT;

		// 나 자신(인덱스0)을 제외한 타 플레이어들 아이디번호 저장
		for (int i = 0, j = 1; i < m_nPlayerCount; ++i) {
			if (start.ID_LIST[i] != m_Players[0].m_nID) {
				m_Players[j++].m_nID = start.ID_LIST[i];
			}
			m_Players[i].m_pSheep->iGameMode = PLAY_MODE;
		}
		SoundPackage *pSound = m_Players[0].m_pSheep->pSound;
		FMOD_System_PlaySound(pSound->System, FMOD_CHANNEL_FREE, pSound->Sound[GAME_BGM], 0, &pSound->Channel[GAME_BGM]);
		FMOD_Channel_SetVolume(pSound->Channel[GAME_BGM], GAME_BGM_VOLUME);
		FMOD_System_PlaySound(pSound->System, FMOD_CHANNEL_FREE, pSound->Sound[CRY_E], 0, &pSound->Channel[CRY_E]);

		break;
	}
	case PAK_KEY_DOWN:
	{
		SC_KEY keyDown;
		memcpy(&keyDown, m_saveBuf, sizeof(SC_KEY));
		for (int i = 0; i < m_nPlayerCount; ++i) {
			if (m_Players[i].m_nID == keyDown.ID) {
				m_Players[i].m_pSheep->special_key(keyDown.key,m_ppObstacles);
				m_Players[i].m_pSheep->pCamera->keyboard(keyDown.key);
				break;
			}
		}
		break;
	}
	case PAK_KEY_UP: 
	{
		SC_KEY keyUp;
		memcpy(&keyUp, m_saveBuf, sizeof(SC_KEY));
		for (int i = 0; i < m_nPlayerCount; ++i) {
			if (m_Players[i].m_nID == keyUp.ID) {
				m_Players[i].m_pSheep->special_key_up(keyUp.key);
				break;
			}
		}
		break;
	}
	default:
		cout << "패킷 ID오류:" <<m_saveBuf[1]<< endl;
		break;
	}
	m_nLeft -= m_saveBuf[0];
	memmove(m_saveBuf, m_saveBuf + m_saveBuf[0], m_nLeft);

}

void CNetwork::keyDown(int key)
{
	char sendData[MAX_PACKET_SIZE] = { 0 };
	CS_KEY *pData = (CS_KEY*)sendData;
	pData->header.ucSize = sizeof(CS_KEY);
	pData->header.byPacketID = PAK_KEY_DOWN;
	pData->key = key;

	int retval = send(m_socket, sendData, sizeof(CS_KEY), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
	}
}

void CNetwork::keyUp(int key)
{
	char sendData[MAX_PACKET_SIZE] = { 0 };
	CS_KEY *pData = (CS_KEY*)sendData;
	pData->header.ucSize = sizeof(CS_KEY);
	pData->header.byPacketID = PAK_KEY_UP;
	pData->key = key;

	int retval = send(m_socket, sendData, sizeof(CS_KEY), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
	}
}

void CNetwork::getReady()
{
	*m_piGameMode = READY_MODE;

	char sendData[MAX_PACKET_SIZE] = { 0 };
	HEADER *pData = (HEADER*)sendData;
	pData->ucSize = sizeof(HEADER);
	pData->byPacketID = PAK_READY;

	int retval = send(m_socket, sendData, sizeof(HEADER), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
	}
}

void CNetwork::keySync()
{
}

void CNetwork::finishEnding()
{
	char sendData[MAX_PACKET_SIZE] = { 0 };
	HEADER *pData = (HEADER*)sendData;
	pData->ucSize = sizeof(HEADER);
	pData->byPacketID = PAK_ENDING;

	int retval = send(m_socket, sendData, sizeof(HEADER), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
	}
}
