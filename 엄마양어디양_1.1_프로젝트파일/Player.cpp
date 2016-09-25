#include "Player.h"



CPlayer::CPlayer()
{
	Camera *pCamera = new Camera();
	m_pSheep = new Sheep(SHEEP, pCamera->x, 0, -30, 9);
	m_pSheep->pCamera = pCamera;
	m_pSheep->iGameMode = MAIN_MODE;
	m_nID = 0;
}


CPlayer::~CPlayer()
{
	delete m_pSheep;
}
