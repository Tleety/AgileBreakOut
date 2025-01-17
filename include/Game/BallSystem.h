#ifndef DAYDREAM_BALLSYSTEM_H
#define DAYDREAM_BALLSYSTEM_H

#include <random>

#include "Core/System.h"
#include "Core/World.h"
#include "Core/EventBroker.h"
#include "Core/CTransform.h"
#include "Core/CTemplate.h"
#include "Core/ResourceManager.h"
#include "Core/ConfigFile.h"

#include "Physics/CPhysics.h"
#include "Physics/CCircleShape.h"
#include "Physics/CRectangleShape.h"
#include "Physics/EContact.h"

#include "Rendering/CModel.h"
#include "Rendering/CSprite.h"
#include "Rendering/CPointLight.h"
#include "Rendering/CAnimation.h"

#include "Game/CBall.h"
#include "Game/CPowerUp.h"
#include "Game/CLife.h"
#include "Game/CBrick.h"
#include "Game/CLifebuoy.h"
#include "Game/ELifeLost.h"
#include "Game/EComboEvent.h"
#include "Game/EResetBall.h"
#include "Game/EMultiBall.h"
#include "Game/EMultiBallLost.h"
#include "Game/EStickyPad.h"
#include "Game/EStickyAttachedToPad.h"
#include "Game/EInkBlaster.h"
#include "Game/EInkBlasterOver.h"
#include "Game/EKrakenAttack.h"
#include "Game/EKrakenAttackEnd.h"
#include "Game/EStageCleared.h"
#include "Game/EArrivedAtNewStage.h"
#include "Game/EGameOver.h"
#include "Game/EPause.h"
#include "Game/EResume.h"
#include "Game/EHitPad.h"
#include "Game/EHitLag.h"
#include "Game/EActionButton.h"
#include "Game/ELifebuoyHit.h"

#include "Physics/ECreateParticleSequence.h"
#include "Sound/CCollisionSound.h"
#include "Sound/EPlaySound.h"
#include "Rendering/EAnimationComplete.h"

namespace dd
{

namespace Systems
{

class BallSystem : public System
{
public:
    BallSystem(World *world, std::shared_ptr<dd::EventBroker> eventBroker)
            : System(world, eventBroker) { }

    ~BallSystem();



    void RegisterComponents(ComponentFactory *cf) override;

    void Initialize() override;

    // Called once per system every tick
    void Update(double dt) override;

    // Called once for every entity in the world every tick
    void UpdateEntity(double dt, EntityID entity, EntityID parent) override;

    // Called when components are committed to an entity
    void OnEntityCommit(EntityID entity) override;

    // Called when an entity is removed
    void OnEntityRemoved(EntityID entity) override;

    EntityID CreateBall();
    EntityID Ball() { return m_Ball; };
    void SetBall(const EntityID& ball) { m_Ball = ball; }

    float& XMovementMultiplier() { return m_XMovementMultiplier; }
    void SetXMovementMultiplier(const float& xMovementMultiplier) { m_XMovementMultiplier = xMovementMultiplier; }
    float& EdgeX() { return m_EdgeX; }
    void SetEdgeX(const float& edgeX) { m_EdgeX = edgeX; }
    float& EdgeY() { return m_EdgeY; }
    void SetEdgeY(const float& edgeY) { m_EdgeY = edgeY; }
    int& MultiBalls() { return m_MultiBalls; }
    void SetMultiBalls(const int& multiBalls) { m_MultiBalls = multiBalls; }
    bool ReplaceBall() const { return m_ReplaceBall; }
    void SetReplaceBall(const bool& replaceBall) { m_ReplaceBall = replaceBall; }
    bool IsPaused() const { return m_Pause; }
    void SetPause(const bool& pause) { m_Pause = pause; }

private:
	std::mt19937 m_RandomGenerator;

    float m_XMovementMultiplier = 2.f;
    float m_EdgeX = 3.2f; 
    float m_EdgeY = 5.2f;
    int m_MultiBalls = 0;
    bool m_ReplaceBall = false;
    bool m_Pause = false;
    bool m_Waiting = true;
	bool m_Sticky = false;
	bool m_InkBlaster = false;
	bool m_InkAttached = false;
	bool m_InkBlockedWaiting = false;
	bool m_KrakenAttack = false;
	double m_KrakenCharge = 0;
	bool m_Restarting = false;
	int m_StickyCounter = 3;

	bool m_First = true;

	bool m_StageBlockedWaiting = false;

    glm::vec3 m_SavedSpeed;
    bool m_InitializePause = false;

    EntityID m_Ball;

    std::unordered_map<EntityID, std::list<glm::vec2>> m_Contacts;
    void ResolveContacts();

    dd::EventRelay<BallSystem, dd::Events::MultiBallLost> m_EMultiBallLost;
    dd::EventRelay<BallSystem, dd::Events::ResetBall> m_EResetBall;
    dd::EventRelay<BallSystem, dd::Events::MultiBall> m_EMultiBall;
	dd::EventRelay<BallSystem, dd::Events::StickyPad> m_EStickyPad;
	dd::EventRelay<BallSystem, dd::Events::InkBlaster> m_EInkBlaster;
	dd::EventRelay<BallSystem, dd::Events::InkBlasterOver> m_EInkBlasterOver;
	dd::EventRelay<BallSystem, dd::Events::KrakenAttack> m_EKrakenAttack;
	dd::EventRelay<BallSystem, dd::Events::KrakenAttackEnd> m_EKrakenAttackEnd;
    dd::EventRelay<BallSystem, dd::Events::Pause> m_EPause;
	dd::EventRelay<BallSystem, dd::Events::Resume> m_EResume;
    dd::EventRelay<BallSystem, dd::Events::Contact> m_Contact;
    dd::EventRelay<BallSystem, dd::Events::ActionButton> m_EActionButton;
	dd::EventRelay<BallSystem, dd::Events::StageCleared> m_EStageCleared;
	dd::EventRelay<BallSystem, dd::Events::ArrivedAtNewStage> m_EArrivedAtNewStage;
	dd::EventRelay<BallSystem, dd::Events::AnimationComplete> m_EAnimationComplete;

    bool Contact(const Events::Contact &event);
    bool OnMultiBallLost(const dd::Events::MultiBallLost &event);
    bool OnResetBall(const dd::Events::ResetBall &event);
    bool OnMultiBall(const dd::Events::MultiBall &event);
	bool OnStickyPad(const dd::Events::StickyPad &event);
	bool OnInkBlaster(const dd::Events::InkBlaster &event);
	bool OnInkBlasterOver(const dd::Events::InkBlasterOver &event);
	bool OnKrakenAttack(const dd::Events::KrakenAttack &event);
	bool OnKrakenAttackEnd(const dd::Events::KrakenAttackEnd &event);
    bool OnPause(const dd::Events::Pause &event);
	bool OnResume(const dd::Events::Resume &event);
    bool OnActionButton(const dd::Events::ActionButton &event);
	bool OnStageCleared(const dd::Events::StageCleared &event);
	bool OnArrivedToNewStage(const dd::Events::ArrivedAtNewStage &event);
	bool OnAnimationComplete(const dd::Events::AnimationComplete &event);
};

}

}

#endif
