//
// Created by Adniklastrator on 2015-10-13.
//

#ifndef DAYDREAM_TRAVELLINGSYSTEM_H
#define DAYDREAM_TRAVELLINGSYSTEM_H


#include "Core/System.h"
#include "Core/CTransform.h"
#include "Core/CTemplate.h"
#include "Core/EventBroker.h"
#include "Core/World.h"
#include "Transform/EMove.h"
#include "Game/EPause.h"
#include "Game/EResume.h"
#include "Game/EArrivedAtNewStage.h"
#include "Game/EStageCleared.h"
#include "Game/CTravels.h"
#include "Game/CBackground.h"
#include "Game/CNewWaterVolume.h"


namespace dd
{

namespace Systems
{

class TravellingSystem : public System
{
public:
    TravellingSystem(World* world, std::shared_ptr<dd::EventBroker> eventBroker)
    : System(world, eventBroker)
    { }

    
	void Initialize() override;
	void Update(double dt) override;
	void UpdateEntity(double dt, EntityID entity, EntityID parent) override;

	bool IsPaused() const { return m_Pause; }
	void SetPause(const bool& pause) { m_Pause = pause; }

private:
	bool m_Pause = false;

	bool m_Travelling = false;
	float m_DistanceTravelled = 0;

    dd::EventRelay<TravellingSystem, dd::Events::Pause> m_EPause;
	dd::EventRelay<TravellingSystem, dd::Events::Resume> m_EResume;
	dd::EventRelay<TravellingSystem, dd::Events::StageCleared> m_EStageCleared;
	dd::EventRelay<TravellingSystem, dd::Events::ArrivedAtNewStage> m_EArrivedAtNewStage;

    bool OnPause(const dd::Events::Pause &event);
	bool OnResume(const dd::Events::Resume &event);
	bool OnStageCleared(const dd::Events::StageCleared &event);
	bool OnArrivedAtNewStage(const dd::Events::ArrivedAtNewStage &event);
};

}

}

#endif //DAYDREAM_TRAVELLINGSYSTEM_H
