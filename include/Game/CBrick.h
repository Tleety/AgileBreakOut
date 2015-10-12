//
// Created by Adniklastrator on 2015-09-09.
//

#ifndef DAYDREAM_CBRICK_H
#define DAYDREAM_CBRICK_H

#include "Core/Component.h"

namespace dd
{

namespace Components
{

struct Brick : Component
{
    int Score = 50;
    int Type = 0;
    bool Removed = false;

	//What number of types each is.
	const int EmptyBrickSpace = 0;
	const int StandardBrick = 1;
	const int MultiBallBrick = 2;
	const int LifebuoyBrick = 3;
	const int StickyBrick = 4;
	const int InkBlasterBrick = 5;
	const int KrakenAttackBrick = 6;
};

}

}

#endif //DAYDREAM_CBRICK_H
