//
// Created by Adniklastrator on 2015-10-08.
//


#include "PrecompiledHeader.h"
#include "Game/LifebuoySystem.h"


void dd::Systems::LifebuoySystem::Initialize()
{
    EVENT_SUBSCRIBE_MEMBER(m_EContact, &LifebuoySystem::OnContact);
	EVENT_SUBSCRIBE_MEMBER(m_EPause, &LifebuoySystem::OnPause);
	EVENT_SUBSCRIBE_MEMBER(m_EResume, &LifebuoySystem::OnResume);
	EVENT_SUBSCRIBE_MEMBER(m_ELifebuoy, &LifebuoySystem::OnLifebuoy);
	EVENT_SUBSCRIBE_MEMBER(m_ELifebuoyHit, &LifebuoySystem::OnLifebuoyHit);

	//Lifebuoy
	{
		auto ent = m_World->CreateEntity();
		m_World->SetProperty(ent, "Name", "Lifebuoy");
		std::shared_ptr<Components::Transform> ctransform = m_World->AddComponent<Components::Transform>(ent);
		ctransform->Position = glm::vec3(50.f, -4.8f, -10.f);
		auto rectangleShape = m_World->AddComponent<Components::RectangleShape>(ent);
		auto lifebuoyTemplate = m_World->AddComponent<Components::Template>(ent);
		rectangleShape->Dimensions = glm::vec2(0.9f, 0.19f);
		auto physics = m_World->AddComponent<Components::Physics>(ent);
		physics->CollisionType = CollisionType::Type::Static;
		physics->Category = CollisionLayer::LifeBuoy;
		physics->Mask = static_cast<CollisionLayer::Type>(CollisionLayer::Ball | CollisionLayer::Brick | CollisionLayer::LifeBuoy | CollisionLayer::Pad | CollisionLayer::Water);
		physics->Density = 1.0f;
		physics->GravityScale = 1;
		auto cModel = m_World->AddComponent<Components::Model>(ent);
		cModel->ModelFile = "Models/Lifebuoy/Lifebuoy1.obj";
		auto lifebuoy = m_World->AddComponent<Components::Lifebuoy>(ent);

		m_World->CommitEntity(ent);

		m_Template = ent;
	}

    return;
}

void dd::Systems::LifebuoySystem::Update(double dt)
{
	/*for (auto it = m_Lifebuoys.begin(); it != m_Lifebuoys.end();) { //Niklas note: Jag fortstatte f� fel efter ni gick hem, och jag f�rstod inte det h�r s� jag gjorde om som jag visste hur i UpdateEntity.
		
		auto transformComponent = m_World->GetComponent<Components::Transform>(it->Entity);
		auto lifebuoyComponent = m_World->GetComponent<Components::Lifebuoy>(it->Entity);

		if (!m_World->ValidEntity(it->Entity)){
			m_Lifebuoys.erase(it++);
		}


		if (transformComponent->Position.y < m_DownEdge) {
			m_World->RemoveEntity(it->Entity);
			m_Lifebuoys.erase(it++);
		}
		else {
			
			if (transformComponent->Position.x > m_RightEdge) {
				transformComponent->Position = glm::vec3(m_LeftEdge + 0.1f, transformComponent->Position.y, transformComponent->Position.z);
			} else if (transformComponent->Position.x < m_LeftEdge) {
				transformComponent->Position = glm::vec3(m_RightEdge - 0.1f, transformComponent->Position.y, transformComponent->Position.z);
			}
			
			if (lifebuoyComponent->Hits <= 0) {
				auto physicsComponent = m_World->GetComponent<Components::Physics>(it->Entity);
				physicsComponent->Mask = CollisionLayer::Type::LifeBuoy;
				physicsComponent->GravityScale = 10.f;
				auto modelComponent = m_World->GetComponent<Components::Model>(it->Entity);
				modelComponent->Color = glm::vec4(0.5f, 0.5f, 0.5f, 0.f);
			}

			if (transformComponent->Position.y < m_DownLimit && lifebuoyComponent->Hits > 0) {
				transformComponent->Position.y = m_DownLimit;
			}

			++it;
		}
	}*/

	if (IsPaused()) {
		return;
	}
	
	if (m_Lifebuoy) {
		m_Timer += dt;
		if (m_Timer > m_WaitingTime) {
			m_Timer = 0;
			m_Lifebuoy = false;
			Lifebuoy(m_SavedEvent);
		}
	}
}

void dd::Systems::LifebuoySystem::UpdateEntity(double dt, EntityID entity, EntityID parent)
{
    if (IsPaused()) {
        return;
    }
    auto templateCheck = m_World->GetComponent<Components::Template>(entity);
    if (templateCheck != nullptr){ return; }

	auto lifebuoyComponent = m_World->GetComponent<Components::Lifebuoy>(entity);

	if (lifebuoyComponent != nullptr) {
		auto transformComponent = m_World->GetComponent<Components::Transform>(entity);

		if (transformComponent->Position.y < m_DownEdge) {
			m_World->RemoveEntity(entity);
			//m_Lifebuoys.erase(it++);
		}
		else {

			if (transformComponent->Position.x > m_RightEdge) {
				transformComponent->Position = glm::vec3(m_LeftEdge + 0.1f, transformComponent->Position.y, transformComponent->Position.z);
			}
			else if (transformComponent->Position.x < m_LeftEdge) {
				transformComponent->Position = glm::vec3(m_RightEdge - 0.1f, transformComponent->Position.y, transformComponent->Position.z);
			}

			if (lifebuoyComponent->Hits <= 0) {
				auto physicsComponent = m_World->GetComponent<Components::Physics>(entity);
				physicsComponent->Mask = CollisionLayer::Type::LifeBuoy;
				physicsComponent->GravityScale = 10.f;
				auto modelComponent = m_World->GetComponent<Components::Model>(entity);
				modelComponent->Color = glm::vec4(0.5f, 0.5f, 0.5f, 0.f);
			}

			if (transformComponent->Position.y < m_DownLimit && lifebuoyComponent->Hits > 0) {
				transformComponent->Position.y = m_DownLimit;
			}
		}
	}
}

bool dd::Systems::LifebuoySystem::OnPause(const dd::Events::Pause &event)
{
    /*if (event.Type != "LifebuoySystem" && event.Type != "All") {
        return false;
    }*/

    m_Pause = true;

    return true;
}

bool dd::Systems::LifebuoySystem::OnResume(const dd::Events::Resume &event)
{
	/*if (event.Type != "LifebuoySystem" && event.Type != "All") {
	return false;
	}*/
	m_Pause = false;
	return true;
}

bool dd::Systems::LifebuoySystem::OnContact(const dd::Events::Contact &event)
{
    
    return true;
}

bool dd::Systems::LifebuoySystem::OnLifebuoy(const dd::Events::Lifebuoy &event)
{
	m_Lifebuoy = true;
	m_SavedEvent = event;
	return true;
}

bool dd::Systems::LifebuoySystem::Lifebuoy(const dd::Events::Lifebuoy &event) 
{
	auto ent = m_World->CloneEntity(m_Template);
	m_World->SetProperty(ent, "Name", "Lifebuoy");
	m_World->RemoveComponent<Components::Template>(ent);
	auto transform = m_World->GetComponent<Components::Transform>(ent);
	auto physics = m_World->GetComponent<Components::Physics>(ent);
	physics->CollisionType = CollisionType::Type::Dynamic;
	//transform->Position = glm::vec3(0.f, 0.f, -10.f);
	transform->Position = event.Transform->Position;
	transform->Velocity = glm::vec3(20.f, 3.f, 0.f);
	LifebuoyInfo info;
	info.Entity = ent;
	m_Lifebuoys.push_back(info);
	
	return true;
}

bool dd::Systems::LifebuoySystem::OnLifebuoyHit(const dd::Events::LifebuoyHit &event)
{
	auto transformComponent = m_World->GetComponent<Components::Transform>(event.Lifebuoy);
	auto lifebuoyComponent = m_World->GetComponent<Components::Lifebuoy>(event.Lifebuoy);
	if (lifebuoyComponent != nullptr) {
		lifebuoyComponent->Hits -= 1;
		auto modelComponent = m_World->GetComponent<Components::Model>(event.Lifebuoy);
		modelComponent->ModelFile = "Models/Lifebuoy/Lifebuoy" + std::to_string(5 - lifebuoyComponent->Hits) + ".obj";

		Events::CreateParticleSequence e;
		e.EmitterLifeTime = 4;
		e.EmittingAngle = glm::half_pi<float>();
		e.Spread = 0.5f;
		e.NumberOfTicks = 1;
		e.ParticleLifeTime = 2.f;
		e.ParticlesPerTick = 1;
		e.Position = transformComponent->Position;
		e.ScaleValues.clear();
		e.ScaleValues.push_back(glm::vec3(0.5f));
		e.ScaleValues.push_back(glm::vec3(2.f, 2.f, 0.2f));
		e.SpriteFile = "Textures/Particles/Cloud_Particle.png";
		e.Color = glm::vec4(1, 0, 0, 1);
		e.AlphaValues.clear();
		e.AlphaValues.push_back(1.f);
		e.AlphaValues.push_back(0.f);
		e.Speed = 10.f;
		EventBroker->Publish(e);
	}

	return true;
}
