#include "PrecompiledHeader.h"
#include "Game/BallSystem.h"

dd::Systems::BallSystem::~BallSystem() {

}

void dd::Systems::BallSystem::RegisterComponents(ComponentFactory *cf)
{

}

void dd::Systems::BallSystem::Initialize()
{
	std::random_device rd;
	m_RandomGenerator = std::mt19937(rd());
    EVENT_SUBSCRIBE_MEMBER(m_Contact, &BallSystem::Contact);
    EVENT_SUBSCRIBE_MEMBER(m_EMultiBallLost, &BallSystem::OnMultiBallLost);
    EVENT_SUBSCRIBE_MEMBER(m_EResetBall, &BallSystem::OnResetBall);
    EVENT_SUBSCRIBE_MEMBER(m_EMultiBall, &BallSystem::OnMultiBall);
	EVENT_SUBSCRIBE_MEMBER(m_EStickyPad, &BallSystem::OnStickyPad);
	EVENT_SUBSCRIBE_MEMBER(m_EInkBlaster, &BallSystem::OnInkBlaster);
	EVENT_SUBSCRIBE_MEMBER(m_EInkBlasterOver, &BallSystem::OnInkBlasterOver);
	EVENT_SUBSCRIBE_MEMBER(m_EKrakenAttack, &BallSystem::OnKrakenAttack);
	EVENT_SUBSCRIBE_MEMBER(m_EKrakenAttackEnd, &BallSystem::OnKrakenAttackEnd);
	EVENT_SUBSCRIBE_MEMBER(m_EPause, &BallSystem::OnPause); 
	EVENT_SUBSCRIBE_MEMBER(m_EResume, &BallSystem::OnResume);
    EVENT_SUBSCRIBE_MEMBER(m_EActionButton, &BallSystem::OnActionButton);
	EVENT_SUBSCRIBE_MEMBER(m_EStageCleared, &BallSystem::OnStageCleared);
	EVENT_SUBSCRIBE_MEMBER(m_EArrivedAtNewStage, &BallSystem::OnArrivedToNewStage);
	EVENT_SUBSCRIBE_MEMBER(m_EAnimationComplete, &BallSystem::OnAnimationComplete);


    //OctoBall
    {
        auto ent = m_World->CreateEntity();
        std::shared_ptr<Components::Transform> transform = m_World->AddComponent<Components::Transform>(ent);
        transform->Position = glm::vec3(-20.f, 0.26f, -10.f);
        transform->Scale = glm::vec3(.3f, 0.3f, 0.3f);
        transform->Velocity = glm::vec3(0.f, 0.f, 0.f);
		transform->Orientation = glm::quat();
        auto model = m_World->AddComponent<Components::Model>(ent);
        model->ModelFile = "Models/Sid/Sid_Flying.dae";
		auto animation = m_World->AddComponent<Components::Animation>(ent);
		animation->Speed = 1.0;
        std::shared_ptr<Components::CircleShape> circleShape = m_World->AddComponent<Components::CircleShape>(ent);
        circleShape->Radius = 0.4f;
        std::shared_ptr<Components::Ball> ball = m_World->AddComponent<Components::Ball>(ent);
        ball->Speed = 5.f;
        ball->Waiting = true;
        std::shared_ptr<Components::Physics> physics = m_World->AddComponent<Components::Physics>(ent);
        std::shared_ptr<Components::Template> ballTemplate = m_World->AddComponent<Components::Template>(ent);
        physics->CollisionType = CollisionType::Type::Dynamic;
        physics->Category = CollisionLayer::Type::Ball;
		physics->Mask = static_cast<CollisionLayer::Type>(CollisionLayer::Type::Pad | CollisionLayer::Type::Brick | CollisionLayer::Type::Wall | CollisionLayer::LifeBuoy);
        physics->Calculate = true;
		auto collisionSound = m_World->AddComponent<Components::CollisionSound>(ent);
		collisionSound->FilePath = "Sounds/Jelly/squid-bounce-n.wav";

        m_World->CommitEntity(ent);

        SetBall(ent);

        auto ent2 = CreateBall();
        auto transform2 = m_World->GetComponent<Components::Transform>(ent2);
        transform2->Position = glm::vec3(-0.f, 0.26f, -10.f);
        transform2->Velocity = glm::vec3(0.0f, -10.f, 0.f);

		/*auto PosterBoy = CreateBall();
		auto PosterBall = m_World->GetComponent<Components::Ball>(PosterBoy);
		PosterBall->PosterBoy = 1;
		auto Postertran = m_World->GetComponent<Components::Transform>(PosterBoy);
		Postertran->Position = glm::vec3(-3.f, -3.f, -10.f);
		Postertran->Velocity = glm::vec3(0.0f, 0.f, 0.f);*/
    }

	SetReplaceBall(true);
}

void dd::Systems::BallSystem::Update(double dt)
{
    ResolveContacts();
}

void dd::Systems::BallSystem::UpdateEntity(double dt, EntityID entity, EntityID parent)
{
    //TODO: When the ball collides with 2 bricks on the same frame it is reflected and then reflected again, make it reflect only once like a list or something?

	if (IsPaused()) {
		return;
	}

    auto ballComponent = m_World->GetComponent<Components::Ball>(entity);

    auto templateCheck = m_World->GetComponent<Components::Template>(entity);
    if (templateCheck != nullptr){ return; }

    if (ballComponent != nullptr) {
		auto transformBall = m_World->GetComponent<Components::Transform>(entity);
		if (ballComponent->PosterBoy == 1) {
			transformBall->Position = glm::vec3(-3, 3, -10);
		}

		if (ReplaceBall()) {
			m_First = true;
			SetReplaceBall(false);
			m_Waiting = true;
			std::uniform_real_distribution<float> dist(-0.5f, 0.5f);
			float random = dist(m_RandomGenerator);
			ballComponent->SavedSpeed = glm::vec3(random, 1, 0.f);
			//ballComponent->SavedSpeed = glm::vec3(0.f, 1, 0.f);
			ballComponent->Waiting = true;
		}

        if (ballComponent->Waiting) {
            if (!m_Waiting) {
				m_Restarting = false;
                ballComponent->Waiting = false;
				/*std::uniform_real_distribution<float> dist(-0.5f, 0.5f);
				float random = dist(m_RandomGenerator);*/
                transformBall->Velocity = glm::normalize(ballComponent->SavedSpeed) * ballComponent->Speed;

				auto cAnim = m_World->GetComponent<Components::Animation>(entity);
				cAnim->Loop = false;
				cAnim->Speed = 2.f;
				cAnim->Time = 0.f; 
				//transform->Velocity = glm::normalize(glm::vec3(0, 1, 0)) * ballComponent->Speed;
			} else {
                transformBall->Velocity = glm::vec3(0.f, 0.f, 0.f);

				auto cModel = m_World->GetComponent<Components::Model>(entity);
				cModel->ModelFile = "Models/Sid/Sid_Jump.dae";

				auto cAnim = m_World->GetComponent<Components::Animation>(entity);
				cAnim->Speed = 0.f;
				cAnim->Time = 0.f;
				cAnim->Loop = false;

				if (!ballComponent->Sticky) {
					auto transform = m_World->GetComponent<Components::Transform>(entity);
					transform->Velocity = glm::vec3(0.f, 0.f, 0.f);
					//transform->Position = glm::vec3(0.0f, -3.f, -10.f);

					if (m_First) {
						transform->Orientation = glm::quat();
						m_First = false;
					}
				}
                return;
            }
        }
		if (ballComponent->Sticky) {
			//TODO - Ball can stick to same pad just after detaching.
			if (!m_Waiting)
			{
				transformBall->Velocity = ballComponent->SavedSpeed;
				m_StickyCounter--;
				if (m_StickyCounter > 0) {
					m_Sticky = true;
				}

				auto cAnim = m_World->GetComponent<Components::Animation>(entity);
				cAnim->Speed = 2.f;
				cAnim->Time = 0.f;
				cAnim->Loop = false;

				ballComponent->Sticky = false;
			}
		}
        
        if (glm::abs(transformBall->Velocity.y) < 2) {
            if (transformBall->Velocity.y > 0) {
                transformBall->Velocity.y = 2;
            } else {
                transformBall->Velocity.y = -2;
            }
        }
		if (transformBall->Position.y < -EdgeY() - 4 || transformBall->Position.y > EdgeY() + 4) {
            if (MultiBalls() != 0) {
                m_World->RemoveEntity(entity);
                Events::MultiBallLost e;
                EventBroker->Publish(e);
            } else if (!m_Restarting) { //If we lose lives when we shouldn't, look here.
                Events::ResetBall be;
                EventBroker->Publish(be);
                Events::LifeLost e;
                e.Entity = entity;
                EventBroker->Publish(e);
				Events::PlaySound se;
				se.FilePath = "Sounds/jap-lose-life.wav";
				EventBroker->Publish(se);
                return;
            }
        } //Removing this made the wall collisions work again
        /* else if (transformBall->Position.x > EdgeX()) {
            if (transformBall->Velocity.x > 0) {
                glm::vec2 reflectedVelocity = glm::reflect(glm::vec2(transformBall->Velocity.x, transformBall->Velocity.y), glm::vec2(1, 0));
                transformBall->Velocity = glm::vec3(reflectedVelocity, 0.f);
            }
        } else if (transformBall->Position.x < -EdgeX()) {
            if (transformBall->Velocity.x < 0) {
                glm::vec2 reflectedVelocity = glm::reflect(glm::vec2(transformBall->Velocity.x, transformBall->Velocity.y), glm::vec2(-1, 0));
                transformBall->Velocity = glm::vec3(reflectedVelocity, 0.f);
            }
        } /*else if (transformBall->Position.y > EdgeY()) {
            if (transformBall->Velocity.y > 0) {
                glm::vec2 reflectedVelocity = glm::reflect(glm::vec2(transformBall->Velocity.x, transformBall->Velocity.y), glm::vec2(0, 1));
                transformBall->Velocity = glm::vec3(reflectedVelocity, 0.f);
            }
        }*/
    }

    if (ballComponent != nullptr) {
		if (!ballComponent->Sticky)
		{
			auto transform = m_World->GetComponent<Components::Transform>(entity);
			glm::vec2 dir = glm::normalize(glm::vec2(transform->Velocity.x, transform->Velocity.y));
			glm::vec2 up = glm::vec2(0.f, 1.f);
			
			float angle = atan2(dir.x, dir.y);
			transform->Orientation = glm::rotate(glm::quat(), angle, glm::vec3(0.f, 0.f, -1.f));
		}
    }
}

bool dd::Systems::BallSystem::OnPause(const dd::Events::Pause &event)
{
    /*if (event.Type != "BallSystem" && event.Type != "All") {
        return false;
    }*/

    m_Pause = true;

    return true;
}

bool dd::Systems::BallSystem::OnResume(const dd::Events::Resume &event)
{
	/*if (event.Type != "BallSystem" && event.Type != "All") {
		return false;
	}*/

	m_Pause = false;

	return true;
}

void dd::Systems::BallSystem::OnEntityCommit(EntityID entity)
{

}

void dd::Systems::BallSystem::OnEntityRemoved(EntityID entity)
{

}

bool dd::Systems::BallSystem::Contact(const Events::Contact &event)
{
    Components::PowerUp* power1 = m_World->GetComponent<Components::PowerUp>(event.Entity1);
    Components::PowerUp* power2 = m_World->GetComponent<Components::PowerUp>(event.Entity2);

    if (power1 != nullptr || power2 != nullptr) {
        return false;
    }

    Components::Ball* ballComponent1 = m_World->GetComponent<Components::Ball>(event.Entity1);
    Components::Ball* ballComponent2 = m_World->GetComponent<Components::Ball>(event.Entity2);
    Components::Ball* ballComponent;

    if (ballComponent1 != nullptr && ballComponent2 != nullptr) {
        return false;
    }


    EntityID ballEntity, otherEntitiy;

    //Which is the ball?
    if (ballComponent1 != nullptr) {
        ballEntity = event.Entity1;
        otherEntitiy = event.Entity2;
        ballComponent = ballComponent1;
    }
    else if (ballComponent2 != nullptr) {
        ballEntity = event.Entity2;
        otherEntitiy = event.Entity1;
        ballComponent = ballComponent2;
    }
    else {
        return false;
    }

	if (ballComponent->Waiting || ballComponent->Sticky) {
		return false;
	}

	
    auto ballTransform = m_World->GetComponent<Components::Transform>(ballEntity);
    glm::vec2 ballVelocity = glm::vec2(ballTransform->Velocity.x, ballTransform->Velocity.y);


    if (m_World->GetProperty<std::string>(otherEntitiy, "Name") == "Pad") {
		auto padTransform = m_World->GetComponent<Components::Transform>(otherEntitiy);

        Events::HitPad e;
		e.Ball = ballEntity;
		e.Pad = otherEntitiy;
        EventBroker->Publish(e);
       // Events::HitLag el;
       // el.Time = 0.1f;
        //el.Type = "All";
       // EventBroker->Publish(el);

        
		float x = (ballTransform->Position.x - padTransform->Position.x) * XMovementMultiplier();
        float y = glm::cos((abs(x) / (1.6f)) * glm::pi<float>() / 2.f) + 1.f;

		//When a combo is more than 2 create a particle showing it.
		if (ballComponent->Combo >= 2){

			Events::CreateParticleSequence particleEvent;

			particleEvent.EmitterLifeTime = 3;
			particleEvent.EmittingAngle = glm::half_pi<float>();
			particleEvent.Spread = 0.f;
			particleEvent.NumberOfTicks = 1;
			particleEvent.ParticleLifeTime = 1.3f;
			particleEvent.ParticlesPerTick = 1;
			particleEvent.Position = glm::vec3(ballTransform->Position.x, -3.5f, -3.f);
			if (ballTransform->Position.x >= 2.5f) {
				particleEvent.Position = glm::vec3(2.5f, -3.5f, -3.f);
			} else if (ballTransform->Position.x <= -2.5f) {
				particleEvent.Position = glm::vec3(-2.5f, -3.5f, -3.f);
			}
			particleEvent.ScaleValues.push_back(glm::vec3(1.5f));
			particleEvent.ScaleValues.push_back(glm::vec3(2.f));
			particleEvent.Color = glm::vec4(1.f);
			particleEvent.Speed = 1.f;
			particleEvent.AlphaValues.push_back(1.f);
			particleEvent.AlphaValues.push_back(0.7f);

			if (ballComponent->Combo <= 9) {
				particleEvent.SpriteFile = "Textures/Combo/Combo00" + std::to_string(ballComponent->Combo) + ".png";
			} else if (ballComponent->Combo <= 42) {
				particleEvent.SpriteFile = "Textures/Combo/Combo0" + std::to_string(ballComponent->Combo) + ".png";
			} else {
				particleEvent.SpriteFile = "Textures/Combo/Combo043.png";
			}
			EventBroker->Publish(particleEvent);
		}
		
		ballComponent->Combo = 0;
		if (!ballComponent->Waiting) {
			if (m_InkBlaster) {
				if (!m_InkAttached) {
					m_InkAttached = true;
					m_Waiting = true;
					m_InkBlockedWaiting = true;
					ballComponent->Sticky = true;
					ballComponent->StickyPlacement = glm::vec3(0, 0.25f, 0);
					ballComponent->SavedSpeed = glm::normalize(glm::vec3(x, y, 0.f)) * ballComponent->Speed;
					ballTransform->Velocity = glm::vec3(0.f, 0.f, 0.f);
				}
			}
			else if (m_Sticky) {
				m_Sticky = false;
				m_Waiting = true;
				ballComponent->Sticky = true;
				//ballComponent->StickyPlacement = ballTransform->Position - padTransform->Position;
				ballComponent->StickyPlacement = glm::vec3(0, 0.25f, 0);
				ballComponent->SavedSpeed = glm::normalize(glm::vec3(x, y, 0.f)) * ballComponent->Speed;
				ballTransform->Velocity *= -1;
				Events::StickyAttachedToPad e;
				EventBroker->Publish(e);


				auto cModel = m_World->GetComponent<Components::Model>(ballEntity);
				cModel->ModelFile = "Models/Sid/Sid_Jump.dae";
				auto cAnim = m_World->GetComponent<Components::Animation>(ballEntity);
				cAnim->Loop = false;
				cAnim->Speed = 0.f;
				cAnim->Time = 0.f;
				return true;
			}
		}

        ballTransform->Velocity = glm::normalize(glm::vec3(x, y ,0.f)) * ballComponent->Speed;
        Events::ComboEvent ec;
        ec.Combo = ballComponent->Combo;
        ec.Ball = ballEntity;
        EventBroker->Publish(ec);
        //std::cout << "Combo: " << ballComponent->Combo << std::endl;
	}	// Lifebuoy should always reflect the ball upwards
	else if (m_World->GetProperty<std::string>(otherEntitiy, "Name") == "Lifebuoy"){
		ballTransform->Velocity = glm::vec3(ballTransform->Velocity.x,  abs(ballTransform->Velocity.y), ballTransform->Velocity.z);
		Events::LifebuoyHit e;
		e.Lifebuoy = otherEntitiy;
		EventBroker->Publish(e);
	} else {
        auto it = m_Contacts.find(ballEntity);
        if (it == m_Contacts.end()) {
            std::list<glm::vec2> normalList;
            normalList.push_back(event.Normal);
            m_Contacts[ballEntity] = normalList;
        } else {
            std::list<glm::vec2> normalList = it->second;
            normalList.push_back(event.Normal);
            m_Contacts[ballEntity] = normalList;
        }
    }

    return true;
}


void dd::Systems::BallSystem::ResolveContacts()
{

    for (auto c : m_Contacts) {
        EntityID entity = c.first;
        std::list<glm::vec2> contactNormals = c.second;
        glm::vec2 finalNormal = glm::vec2(0.f);
        for (auto it = contactNormals.begin(); it != contactNormals.end(); it++) {
            finalNormal = finalNormal + (*it);
        }
        finalNormal = glm::normalize(finalNormal);
        auto transform = m_World->GetComponent<Components::Transform>(entity);
        glm::vec2 velocity = glm::vec2(transform->Velocity.x, transform->Velocity.y);
        glm::vec2 reflectedVelocity = glm::reflect(velocity, finalNormal);
		auto ballComponent = m_World->GetComponent<Components::Ball>(entity);
		reflectedVelocity = glm::normalize(reflectedVelocity) * ballComponent->Speed;
        transform->Velocity = glm::vec3(reflectedVelocity, 0.f);
    }
    m_Contacts.clear();
}

EntityID dd::Systems::BallSystem::CreateBall()
{
    auto ent = m_World->CloneEntity(Ball());
    m_World->RemoveComponent<Components::Template>(ent);

    return ent;
}

bool dd::Systems::BallSystem::OnMultiBallLost(const dd::Events::MultiBallLost &event)
{
    SetMultiBalls(MultiBalls() - 1);
    //std::cout << MultiBalls() << std::endl;
    if (MultiBalls() < 0) {
        auto ent = CreateBall();
        SetMultiBalls(0);
    }
    return true;
}

bool dd::Systems::BallSystem::OnResetBall(const dd::Events::ResetBall &event)
{
    SetReplaceBall(true);
    return true;
}

bool dd::Systems::BallSystem::OnMultiBall(const dd::Events::MultiBall &event)
{
    auto ent1 = CreateBall();
    auto ent2 = CreateBall();
    auto transform1 = m_World->GetComponent<Components::Transform>(ent1);
    auto transform2 = m_World->GetComponent<Components::Transform>(ent2);
    auto ball1 = m_World->GetComponent<Components::Ball>(ent1);
    auto ball2 = m_World->GetComponent<Components::Ball>(ent2);
    ball1->Waiting = false;
    ball2->Waiting = false;
    auto padTransform = event.padTransform;
    float x1 = padTransform->Position.x/* - 2*/, x2 = padTransform->Position.x/* + 2*/;
    if (x1 < -m_EdgeX + 0.4) {
        x1 = m_EdgeX - 0.7;
    }
    if (x2 > m_EdgeX - 0.4) {
        x2 = -m_EdgeX + 0.7;
    }
    transform1->Position = glm::vec3(x1, -5.5, -10);
    transform2->Position = glm::vec3(x2, -5.5, -10);

    transform1->Velocity = glm::normalize(glm::vec3(2.5, 5 ,0.f)) * ball1->Speed;
    transform2->Velocity = glm::normalize(glm::vec3(-2.5, 5 ,0.f)) * ball2->Speed;

    SetMultiBalls(MultiBalls() + 2);
    //std::cout << MultiBalls() << std::endl;
    return true;
}

bool dd::Systems::BallSystem::OnStickyPad(const dd::Events::StickyPad &event) 
{
	m_Sticky = true;
	m_StickyCounter = event.Times;
	return true;
}

bool dd::Systems::BallSystem::OnInkBlaster(const dd::Events::InkBlaster &event)
{
	m_InkBlaster = true;
	return true;
}

bool dd::Systems::BallSystem::OnInkBlasterOver(const dd::Events::InkBlasterOver &event)
{
	auto ballComponent = m_World->GetComponent<Components::Ball>(event.Ball);
	auto transform = m_World->GetComponent<Components::Transform>(event.Ball);


	auto cModel = m_World->GetComponent<Components::Model>(event.Ball);
	cModel->ModelFile = "Models/Sid/Sid_Jump.dae";

	auto cAnim = m_World->GetComponent<Components::Animation>(event.Ball);
	cAnim->Speed = 2.f;
	cAnim->Loop = false;

	m_InkBlaster = false;
	m_InkAttached = false;
	m_InkBlockedWaiting = false;
	ballComponent->SavedSpeed = glm::normalize(glm::vec3(0.f, 1.f, 0.f)) * ballComponent->Speed;
	m_Waiting = false;
	transform->Velocity = glm::normalize(glm::vec3(0.f, 1, 0.f)) * ballComponent->Speed;
	return true;
}

bool dd::Systems::BallSystem::OnKrakenAttack(const dd::Events::KrakenAttack &event)
{
	m_KrakenAttack = true;
	return true;
}

bool dd::Systems::BallSystem::OnKrakenAttackEnd(const dd::Events::KrakenAttackEnd &event)
{
	m_KrakenAttack = false;
	return true;
}

bool dd::Systems::BallSystem::OnActionButton(const dd::Events::ActionButton &event)
{
	if (!m_KrakenAttack) {
		if (!m_StageBlockedWaiting) {
			if (!m_InkBlockedWaiting) {
				m_Waiting = false;
			}
		}
	}
    return true;
}

bool dd::Systems::BallSystem::OnStageCleared(const dd::Events::StageCleared &event)
{
	m_Restarting = true;
	m_StageBlockedWaiting = true;
	return true;
}

bool dd::Systems::BallSystem::OnArrivedToNewStage(const dd::Events::ArrivedAtNewStage &event)
{
	m_StageBlockedWaiting = false;
	return true;
}

bool dd::Systems::BallSystem::OnAnimationComplete(const dd::Events::AnimationComplete &event)
{
	auto model = m_World->GetComponent<Components::Model>(event.Entity);
	if (model) {
		if (model->ModelFile == "Models/Sid/Sid_Jump.dae") {
			model->ModelFile = "Models/Sid/Sid_Flying.dae";

			auto anim = m_World->GetComponent<Components::Animation>(event.Entity);
			anim->Speed = 1.f;
			anim->Loop = true;
			anim->Time = 0.f;
		}
	}

	return true;
}

