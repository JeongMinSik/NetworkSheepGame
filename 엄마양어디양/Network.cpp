#include "stdafx.h"
#include "Network.h"
#include "Objects.h"
#include "SoundPackage.h"
#include "Camera.h"
#include "Sheep.h"

CNetwork::CNetwork()
{
	m_socket = NULL;
	m_pRecvThread = nullptr;
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
	printf("[%s] %s", msg, (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

void CNetwork::connectServer()
{
	char strServerAddr[256];
	cout << "아이피주소입력(xxx.xxx.xxx.xxx):";
	cin >> strServerAddr;

	if (strServerAddr[0] == '0') {
		strcpy_s( strServerAddr, sizeof( strServerAddr ), "127.0.0.1");
	}

	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		err_quit("윈속 초기화 실패");

	m_socket = socket(AF_INET, SOCK_STREAM, 0);

	// 네이글알고리즘
	BOOL optval = TRUE;
	setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, (char*)&optval, sizeof(optval));

	if (m_socket == INVALID_SOCKET) err_quit("socket()");

	// connect()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(strServerAddr);
	serveraddr.sin_port = htons(SERVER_PORT);
	retval = connect(m_socket, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

	m_pRecvThread = new thread{ mem_fun(&CNetwork::recvThreadFunc),this };

	cout << "connectServer() Clear!" << endl;

	char sendData[MAX_PACKET_SIZE] = { 0 };
	HEADER *pData = (HEADER*)sendData;
	pData->packetSize = sizeof(HEADER);
	pData->packetID = PAK_LOGIN;

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

// 사용자 정의 데이터 수신 함수
int recvn(SOCKET s, char *buf, int len, int flags)
{
	int received;
	char *ptr = buf;
	int left = len;

	while (left > 0) {
		received = recv(s, ptr, left, flags);
		if (received == SOCKET_ERROR)
			return SOCKET_ERROR;
		else if (received == 0)
			break;
		left -= received;
		ptr += received;
	}

	return (len - left);
}

void CNetwork::recvThreadFunc()
{
	char recvBuf[MAX_PACKET_SIZE] = { 0 };
	int retval;
	int iCurrPacketSize = 0;
	int iStoredPacketSize = 0;
	while( 1 ) {

		// 데이터 받기
		ZeroMemory( recvBuf, sizeof( recvBuf ) );
		retval = recv( m_socket, recvBuf, sizeof( recvBuf ), 0 );

		if( retval == SOCKET_ERROR ) {
			err_quit( "recv()" );
			break;
		}

		char *pRecvBuf = recvBuf;

		while( 0 < retval ) {

			if( 0 == iCurrPacketSize ) {
				if( retval + iStoredPacketSize >= sizeof( HEADER ) ) {
					int restHeaderSize = sizeof( HEADER ) - iStoredPacketSize;
					memcpy( m_saveBuf + iStoredPacketSize, pRecvBuf, restHeaderSize );
					pRecvBuf += restHeaderSize;
					iStoredPacketSize += restHeaderSize;
					retval -= restHeaderSize;
					iCurrPacketSize = ( ( HEADER* )m_saveBuf )->packetSize;
				}
				else {
					memcpy( m_saveBuf + iStoredPacketSize, pRecvBuf, retval );
					iStoredPacketSize += retval;
					retval = 0;
					break;
				}
			}

			int restSize = iCurrPacketSize - iStoredPacketSize;

			if( restSize <= retval ) {

				memcpy( m_saveBuf + iStoredPacketSize, pRecvBuf, restSize );

				packetProcess();

				iCurrPacketSize = iStoredPacketSize = 0;

				pRecvBuf += restSize;
				retval -= restSize;
			}
			else {
				memcpy( m_saveBuf + iStoredPacketSize, pRecvBuf, retval );

				iStoredPacketSize += retval;
				retval = 0;
				//recvBuf += recvSize;
			}
		}
	}
}

void CNetwork::packetProcess()
{
	HEADER *pHeader = (HEADER*)m_saveBuf;
	switch (pHeader->packetID)
	{
	case PAK_LOGIN:
	{
		SC_LOG_INOUT login;
		memcpy(&login, m_saveBuf, sizeof(SC_LOG_INOUT));
		m_Players[0].m_nID = login.ID;
		printf("-> 접속했습니다. 부여받은 아이디는 %d번 입니다. \n", login.ID);
		printf("-> 준비상태( %d / %d ), 총 접속자: %d명 \n\n", login.readyCount, MAX_PLAYER_CNT, login.clientNum);

		break;
	}
	case PAK_REG:
	{
		SC_LOG_INOUT login;
		memcpy(&login, m_saveBuf, sizeof(SC_LOG_INOUT));
		printf("-> %d번 클라가 접속했습니다. \n", login.ID);
		printf("-> 준비상태( %d / %d ), 총 접속자: %d명 \n\n", login.readyCount, MAX_PLAYER_CNT, login.clientNum);
		break;
	}
	case PAK_RMV:
	{
		SC_LOG_INOUT logout;
		memcpy(&logout, m_saveBuf, sizeof(SC_LOG_INOUT));
		printf("-> %d번 클라가 로그아웃했습니다. \n", logout.ID);
		if (*m_piGameMode == READY_MODE) {
			printf("-> 준비상태( %d / %d ), 총 접속자: %d명 \n\n", logout.readyCount, MAX_PLAYER_CNT, logout.clientNum);
		}
		for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
			if (m_Players[i].m_nID == logout.ID) {
				m_Players[i].m_nID = -1;
			}
		}
		break;
	}
	case PAK_READY:
	{
		SC_LOG_INOUT ready;
		memcpy(&ready, m_saveBuf, sizeof(SC_LOG_INOUT));
		printf("-> %d번 클라가 준비했습니다. \n", ready.ID);
		printf("-> 준비상태( %d / %d ), 총 접속자: %d명 \n\n", ready.readyCount, MAX_PLAYER_CNT, ready.clientNum);
		break;
	}
	case PAK_START:
	{
		SC_START start;
		memcpy(&start, m_saveBuf, sizeof(SC_START));
		printf("-> 게임을 시작합니다. \n\n");
		*m_piGameMode = PLAY_MODE; 

		// 나 자신(인덱스0)을 제외한 타 플레이어들 아이디번호 저장
		for (int i = 0, j = 1; i < MAX_PLAYER_CNT; ++i) {
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
		for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
			if (m_Players[i].m_nID == keyDown.ID) {
				m_Players[i].m_pSheep->special_key(keyDown.key);
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
		for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
			if (m_Players[i].m_nID == keyUp.ID) {
				m_Players[i].m_pSheep->special_key_up(keyUp.key);
				break;
			}
		}
		break;
	}
	case PAK_SYNC:
	{
		SC_SYNC sync;
		memcpy(&sync, m_saveBuf, sizeof(SC_SYNC));
	
		// 양들 동기화
		for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
			for (int j = 0; j < MAX_PLAYER_CNT; ++j) {
				if (m_Players[i].m_nID == sync.sheep_ID[j]) {
					m_Players[i].m_pSheep->x = sync.sheep_pos[j].x;
					m_Players[i].m_pSheep->y = sync.sheep_pos[j].y;
					m_Players[i].m_pSheep->z = sync.sheep_pos[j].z;
					m_Players[i].m_pSheep->pCamera->x = sync.sheep_pos[j].x;
					memcpy(m_Players[i].m_pSheep->state, sync.state[j], sizeof(sync.state[j]));
					break;
				}
			}
		}

		// 동적객체 동기화
		for (int i = 0; i < MOVING_OB_CNT; ++i) {
			m_vpMovingObject[i]->x = sync.object_pos[i].x;
			m_vpMovingObject[i]->y = sync.object_pos[i].y;
			m_vpMovingObject[i]->z = sync.object_pos[i].z;
		}
		break;

	}
	case PAK_HURT:
	{
		SC_EVENT hurt;
		memcpy(&hurt, m_saveBuf, sizeof(SC_EVENT));
		for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
			if (m_Players[i].m_nID == hurt.ID) {
				m_Players[i].m_pSheep->get_hurt();
				break;
			}
		}
		break;
	}
	case PAK_ENDING:
	{
		SC_EVENT ending;
		memcpy(&ending, m_saveBuf, sizeof(SC_EVENT));
		for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
			if (m_Players[i].m_nID == ending.ID) {
				m_Players[i].m_pSheep->iGameMode = ENDING_MODE;
				break;
			}
		}
		if (m_Players[0].m_nID == ending.ID) {
			printf("당신이 승리했습니다! \n");
		}
		else {
			printf("당신은 패배했습니다! \n");
			printf("%d번 아이디가 승리했습니다. \n",ending.ID);
		}

		break;
	}
	case PAK_GAMEOVER:
	{
		*m_piGameMode = GAME_OVER;
		for (int i = 0; i < MAX_PLAYER_CNT; ++i) {
			m_Players[i].m_pSheep->ending_finished = true;
		}
		printf("모든 플레이어가 게임오버! \n");
		break;
	}
	default:
		cout << "패킷 ID오류:" << pHeader->packetID << endl;
		break;
	}
}

void CNetwork::keyDown(int key)
{
	char sendData[MAX_PACKET_SIZE] = { 0 };
	CS_KEY *pData = (CS_KEY*)sendData;
	pData->header.packetSize = sizeof(CS_KEY);
	pData->header.packetID = PAK_KEY_DOWN;
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
	pData->header.packetSize = sizeof(CS_KEY);
	pData->header.packetID = PAK_KEY_UP;
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
	pData->packetSize = sizeof(HEADER);
	pData->packetID = PAK_READY;

	int retval = send(m_socket, sendData, sizeof(HEADER), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
	}
}

