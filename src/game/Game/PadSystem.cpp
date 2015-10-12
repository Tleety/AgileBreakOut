//
// Created by Adniklastrator on 2015-09-10.
//

#include "PrecompiledHeader.h"
#include "Game/PadSystem.h"
#include "Core/World.h"
#include <iostream>
#include <Game/CPad.h>
#include <Rendering/CPointLight.h>


void dd::Systems::PadSystem::Initialize()
{
    Events::BindKey r;
    r.KeyCode = GLFW_KEY_RIGHT;
    r.Command = "right";
    EventBroker->Publish(r);

    Events::BindKey l;
    l.KeyCode = GLFW_KEY_LEFT;
    l.Command = "left";
    EventBroker->Publish(l);

    EVENT_SUBSCRIBE_MEMBER(m_EKeyDown, &PadSystem::OnKeyDown);
    EVENT_SUBSCRIBE_MEMBER(m_EKeyUp, &PadSystem::OnKeyUp);
    EVENT_SUBSCRIBE_MEMBER(m_EContact, &PadSystem::OnContact);
    EVENT_SUBSCRIBE_MEMBER(m_EContactPowerUp, &PadSystem::OnContactPowerUp);
    EVENT_SUBSCRIBE_MEMBER(m_EStageCleared, &PadSystem::OnStageCleared);
    EVENT_SUBSCRIBE_MEMBER(m_EPause, &PadSystem::OnPause);
	EVENT_SUBSCRIBE_MEMBER(m_EKrakenAttack, &PadSystem::OnKrakenAttack);
	EVENT_SUBSCRIBE_MEMBER(m_EStickyPad, &PadSystem::OnStickyPad);
	EVENT_SUBSCRIBE_MEMBER(m_EStickyAttachedToPad, &PadSystem::OnStickyAttachedToPad);
	EVENT_SUBSCRIBE_MEMBER(m_EActionButton, &PadSystem::OnActionButton);
    //EVENT_SUBSCRIBE_MEMBER(m_EBindKey, &PadSystem::OnBindKey);

    {
        auto ent = m_World->CreateEntity();
        m_World->SetProperty(ent, "Name", "Pad");
        auto ctransform = m_World->AddComponent<Components::Transform>(ent);
        ctransform->Position = glm::vec3(0.f, -4.8f, -10.f);
        auto rectangleShape = m_World->AddComponent<Components::RectangleShape>(ent);
        rectangleShape->Dimensions = glm::vec2(1.f, 0.1f);
        auto physics = m_World->AddComponent<Components::Physics>(ent);
        physics->CollisionType = CollisionType::Type::Kinematic;
        physics->Category = CollisionLayer::Type::Pad;
		physics->Mask = static_cast<CollisionLayer::Type>(CollisionLayer::Ball | CollisionLayer::PowerUp | CollisionLayer::LifeBuoy);
        physics->Calculate = true;
        ctransform->Sticky = true;
        auto cModel = m_World->AddComponent<Components::Model>(ent);
        cModel->ModelFile = "Models/Ship/Ship.obj";

        auto pad = m_World->AddComponent<Components::Pad>(ent);
        m_World->CommitEntity(ent);

        SetEdge(3.2 - (ctransform->Scale.x / 2));
    }

	//Stick
	{
		auto ent = m_World->CreateEntity();
		m_World->SetProperty(ent, "Name", "Stick");
		std::shared_ptr<Components::Transform> transform = m_World->AddComponent<Components::Transform>(ent);
		transform->Position = glm::vec3(30.f, 0.f, -10.f);
		transform->Scale = glm::vec3(0.1f, 20.f, 0.1f);
		std::shared_ptr<Components::Sprite> sprite = m_World->AddComponent<Components::Sprite>(ent);
		sprite->SpriteFile = "Textures/Core/ErrorTexture.png";
		std::shared_ptr<Components::StickyAim> sticky = m_World->AddComponent<Components::StickyAim>(ent);
		m_World->CommitEntity(ent);

		m_StickTransform = transform;
		m_StickyAim = sticky;
	}
}

void dd::Systems::PadSystem::UpdateEntity(double dt, EntityID entity, EntityID parent)
{
    auto templateCheck = m_World->GetComponent<Components::Template>(entity);
    if (templateCheck != nullptr){ return; }

	auto ball = m_World->GetComponent<Components::Ball>(entity);
	if (ball != nullptr) {
		if (ball->Sticky) {
			auto ballTransform = m_World->GetComponent<Components::Transform>(entity);
			ballTransform->Position = Transform()->Position;
			ballTransform->Position += ball->StickyPlacement;
			glm::vec2 dir = glm::normalize(glm::vec2(ball->SavedSpeed.x, ball->SavedSpeed.y));
			glm::vec2 up = glm::vec2(0.f, 1.f);
			float angle = glm::acos(glm::dot<float>(dir, up)) * glm::sign(dir.x);
			ballTransform->Orientation = glm::rotate(glm::quat(), angle, glm::vec3(0.f, 0.f, -1.f));
			m_StickTransform->Orientation = glm::rotate(glm::quat(), angle, glm::vec3(0.f, 0.f, 1.f));
			m_StickTransform->Position = ballTransform->Position;
		}
		if (!m_StickyAim->Aiming) {
			m_StickTransform->Position = glm::vec3(30.f, 0.f, 0.f);
		}
	}
}

void dd::Systems::PadSystem::Update(double dt)
{
    if (IsPaused()) {
        return;
    }
    if (Entity() == 0) {
        for (auto it = m_World->GetEntities()->begin(); it != m_World->GetEntities()->end(); it++) {
            if (m_World->GetProperty<std::string>(it->first, "Name") == "Pad") {
                SetEntity(it->first);
                SetTransform(m_World->GetComponent<Components::Transform>(Entity()));
                SetPad(m_World->GetComponent<Components::Pad>(Entity()));
                break;
            }
        }
    }

	if (m_KrakenAttack) {
		m_KrakenCharge -= m_KrakenStrength * dt;
		if (m_KrakenCharge < 0) {
			m_KrakenCharge = 0;
		}
		Events::KrakenAttack e;
		e.ChargeUpdate = m_KrakenCharge;
		EventBroker->Publish(e);
		//std::cout << "Charge: " << m_KrakenCharge << std::endl;
		return;
	}

	auto transform = Transform();
	auto pad = Pad();
	auto acceleration = Acceleration();

    if (transform->Velocity.x < -pad->MaxSpeed) {
        transform->Velocity.x = -pad->MaxSpeed;
    }
    else if (transform->Velocity.x > pad->MaxSpeed) {
        transform->Velocity.x = pad->MaxSpeed;
    }
    transform->Position += transform->Velocity * (float)dt;
    if (transform->Position.x > Edge()) {
        transform->Position.x = Edge();
    } else if (transform->Position.x < -Edge()) {
        transform->Position.x = -Edge();
    }
    transform->Velocity += acceleration  * (float)dt;
    if (glm::abs(transform->Velocity.x) > 1 || (!Left() && !Right())) {
        transform->Velocity -= transform->Velocity * pad->SlowdownModifier * (float) dt;
    }

    if (Left()) {
        acceleration.x = -pad->AccelerationSpeed;
    } else if (Right()) {
        acceleration.x = pad->AccelerationSpeed;
    } else {
        acceleration.x = 0.f;
    }

    SetTransform(transform);
    SetPad(pad);
    SetAcceleration(acceleration);

    return;
}

bool dd::Systems::PadSystem::OnPause(const dd::Events::Pause &event)
{
    if (event.Type != "PadSystem" && event.Type != "All") {
        return false;
    }

    if (IsPaused()) {
        SetPause(false);
    } else {
        SetPause(true);
    }
    return true;
}

bool dd::Systems::PadSystem::OnKeyDown(const dd::Events::KeyDown &event) {
    int val = event.KeyCode;
    if (val == GLFW_KEY_UP) {
        //std::cout << "Up!" << std::endl;
    } else if (val == GLFW_KEY_DOWN) {
        //std::cout << "Down!" << std::endl;
    } else if (val == GLFW_KEY_LEFT) {
        //std::cout << "Left!" << std::endl;
        //acceleration.x = -0.01f;
        SetLeft(true);
    } else if (val == GLFW_KEY_RIGHT) {
        //std::cout << "Right!" << std::endl;
        //acceleration.x = 0.01f;
        SetRight(true);
    } else if (val == GLFW_KEY_R) {
        Events::ResetBall e;
        EventBroker->Publish(e);
    } else if (val == GLFW_KEY_M) {
        Events::MultiBall e;
        e.padTransform = Transform();
        EventBroker->Publish(e);
    } else if (val == GLFW_KEY_P) {
        Events::Pause e;
        e.Type = "All";
        EventBroker->Publish(e);
    } else if (val == GLFW_KEY_H) {
        Events::HitLag e;
        e.Time = 0.2;
        e.Type = "All";
        EventBroker->Publish(e);
    } else if (val == GLFW_KEY_S) {
		Events::StageCleared e;
		EventBroker->Publish(e);
	} else if (val == GLFW_KEY_Y) {
		Events::StickyPad e;
		EventBroker->Publish(e);
	}
	else if (val == GLFW_KEY_I) {
		Events::InkBlaster e;
		EventBroker->Publish(e);
	}
	else if (val == GLFW_KEY_K) {
		Events::KrakenAttack e;
		e.ChargeUpdate = 0;
		e.KrakenStrength = 0.1;
		e.PlayerStrength = 0.05;
		EventBroker->Publish(e);
	}
	else if (val == GLFW_KEY_SPACE) {
        Events::ActionButton e;
		e.Position = Transform()->Position;
        EventBroker->Publish(e);
    } else if (val == GLFW_KEY_D) {
        return false;
    }
    return true;
}

bool dd::Systems::PadSystem::OnKeyUp(const dd::Events::KeyUp &event)
{
    int val = event.KeyCode;
    if (val == GLFW_KEY_UP) {

    } else if (val == GLFW_KEY_DOWN) {

    } else if (val == GLFW_KEY_LEFT) {
        SetLeft(false);
    } else if (val == GLFW_KEY_RIGHT) {
        SetRight(false);
    }
    return true;
}

bool dd::Systems::PadSystem::OnContact(const dd::Events::Contact &event)
{
    /*
    EntityID entityBall = event.Entity2;
    auto ball = m_World->GetComponent<Components::Ball>(entityBall);
    if (ball == NULL) {
        return false;
    }
    EntityID entityPad = event.Entity1;
    auto pad = m_World->GetComponent<Components::Pad>(entityPad);
    if (pad == NULL) {
        return false;
    }
    auto transformBall = m_World->GetComponent<Components::Transform>(entityBall);
    auto transformPad = m_World->GetComponent<Components::Transform>(entityPad);

    float movementMultiplier = 0.5f;

    //float movementX = (event.ContactPoint.x - transformPad->Position.x) * movementMultiplier;
    //float movementY = glm::cos((abs(movementX) / (3.2f * movementMultiplier)) * 3.14159265359f / 2) * 2.f;
    if (whatX > 0) {
        movementX = movementMultiplier * whatX;
        //std::cout << "Right!" << std::endl;
    } else {
        movementX = movementMultiplier * whatX;
        //std::cout << "Left!" << std::endl;
    }

   // std::cout << movementX << " " << movementY << std::endl;

    //float movementX = (event.ContactPoint.x - transformPad->Position.x) * movementMultiplier;
    float movementY = glm::cos((abs(movementX) / ((1.6f) * movementMultiplier)) * 3.14159265359f / 2)+ 0.2;
    //std::cout << movementX << " " << movementY << std::endl;
    float len = glm::length<float>(transformBall->Velocity);
    //auto pointlight = m_World->AddComponent<Components::PointLight>(ent);


    transformBall->Velocity += glm::vec3(transformPad->Velocity.x, 0, 0);
    transformBall->Velocity = glm::normalize(transformBall->Velocity) * len;


    //transform->Velocity = glm::vec3(movementX, movementY, 0.f);
     */
	return false;
}

bool dd::Systems::PadSystem::OnContactPowerUp(const dd::Events::Contact &event)
{
    EntityID entityPower = 0;
    EntityID entityPad = 0;
    auto powerUp = m_World->GetComponent<Components::PowerUp>(event.Entity1);
    auto pad = m_World->GetComponent<Components::Pad>(event.Entity2);
    if (powerUp != nullptr) {
        entityPower = event.Entity1;
    } else {
        powerUp = m_World->GetComponent<Components::PowerUp>(event.Entity2);
        if (powerUp != nullptr) {
            entityPower = event.Entity2;
        }
    }
    if (pad != nullptr) {
        entityPad = event.Entity2;
    } else {
        pad = m_World->GetComponent<Components::Pad>(event.Entity1);
        if (pad != nullptr) {
            entityPad = event.Entity1;
        }
    }

    if (entityPower == 0 || entityPad == 0) {
        return false;
    }

    pad = m_World->GetComponent<Components::Pad>(entityPad);
    if (pad == nullptr) {
        return false;
    }

    m_World->RemoveEntity(entityPower);
    Events::PowerUpTaken ep;
    ep.Name = "Something";
    EventBroker->Publish(ep);

    Events::MultiBall e;
    auto transform = m_World->GetComponent<Components::Transform>(entityPad);
    e.padTransform = transform;
    EventBroker->Publish(e);
    return true;
}

bool dd::Systems::PadSystem::OnStageCleared(const dd::Events::StageCleared &event)
{
    //auto entity = CreateBall();
    return true;
}

bool dd::Systems::PadSystem::OnKrakenAttack(const dd::Events::KrakenAttack &event)
{
	if (!m_KrakenAttack) {
		m_KrakenStrength = event.KrakenStrength;
		m_PlayerStrength = event.PlayerStrength;
		m_KrakenCharge = event.ChargeUpdate;
		m_KrakenAttack = true;
		auto transform = Transform();
		auto acceleration = Acceleration();

		transform->Velocity = glm::vec3(0, 0, 0);
		acceleration = glm::vec3(0, 0, 0);

		SetTransform(transform);
		SetAcceleration(acceleration);
	} else if (m_KrakenCharge >= 1) {
		m_KrakenAttack = false;
		m_KrakenCharge = 0;
		m_KrakenStrength = 0;
		m_PlayerStrength = 0;
	}
	return true;
}

bool dd::Systems::PadSystem::OnStickyPad(const dd::Events::StickyPad &event)
{
	return true;
}

bool dd::Systems::PadSystem::OnStickyAttachedToPad(const dd::Events::StickyAttachedToPad &event)
{
	m_StickyAim->Aiming = true;
	return true;
}

bool dd::Systems::PadSystem::OnActionButton(const dd::Events::ActionButton &event)
{
	if (m_KrakenAttack) {
		m_KrakenCharge += m_PlayerStrength;

		Events::KrakenAttack e;
		e.ChargeUpdate = m_KrakenCharge;
		e.KrakenStrength = 0;
		e.PlayerStrength = 0;
		EventBroker->Publish(e);
	}
	m_StickyAim->Aiming = false;
	return true;
}

bool dd::Systems::PadSystem::PadSteeringInputController::OnCommand(const Events::InputCommand &event)
{
    std::string command = event.Command;

    std::cout << "Command!" << std::endl;

    if (command == "right") {
        std::cout << "Right!" << std::endl;
    } else if (command == "left") {
        std::cout << "Left!" << std::endl;
    }

    return true;
}