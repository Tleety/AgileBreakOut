//
// Created by Adniklastrator on 2015-09-10.
//

#ifndef DAYDREAM_CBALL_H
#define DAYDREAM_CBALL_H

#include "Core/Component.h"

namespace dd
{

namespace Components
{

struct Ball : Component
{
    int Number = 0;
    float Speed = 5.f;
    int Combo = 0;
    bool Paused = false;
    glm::vec3 SavedSpeed = glm::vec3(0, 0, 0);
};

}

}

#endif //DAYDREAM_CBALL_H
