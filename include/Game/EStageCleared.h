//
// Created by Adniklastrator on 2015-09-09.
//

#ifndef DAYDREAM_ESTAGECLEARED_H
#define DAYDREAM_ESTAGECLEARED_H

#include "Core/EventBroker.h"

namespace dd
{

namespace Events
{

struct StageCleared : Event
{
    int ClearedStage = 0;
    int StageCluster = 0;
	int StagesInCluster = 6;
};

}

}

#endif //DAYDREAM_ESTAGECLEARED_H
