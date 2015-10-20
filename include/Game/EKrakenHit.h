//
// Created by Adniklastrator on 2015-10-19.
//

#ifndef DAYDREAM_EKRAKENHIT_H
#define DAYDREAM_EKRAKENHIT_H

#include "Core/EventBroker.h"

namespace dd
{

namespace Events
{

struct KrakenHit : Event
{
	EntityID Kraken;
	EntityID Hitter;
};

}

}

#endif //DAYDREAM_EKRAKENHIT_H
