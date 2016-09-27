#pragma once
#include "Objects.h"

struct CPlayer
{
	Sheep *m_pSheep;
	int m_nID;
	CPlayer();
	~CPlayer();
	void init();
};

