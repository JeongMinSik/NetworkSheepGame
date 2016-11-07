#pragma once
#include "stdafx.h"

struct SoundPackage {
	FMOD_SYSTEM *System;
	FMOD_SOUND **Sound;
	FMOD_CHANNEL **Channel;
};