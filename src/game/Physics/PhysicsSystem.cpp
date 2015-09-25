#include "PrecompiledHeader.h"
#include "Physics/PhysicsSystem.h"



void dd::Systems::PhysicsSystem::RegisterComponents(ComponentFactory* cf)
{
    cf->Register<Components::CircleShape>();
}

void dd::Systems::PhysicsSystem::Initialize()
{
    m_ContactListener = new ContactListener(this);

    m_Gravity = b2Vec2(0.f, -9.82f);
    m_PhysicsWorld = new b2World(m_Gravity);

    m_TimeStep = 1.f/60.f;
    m_VelocityIterations = 6;
    m_PositionIterations = 2;
    m_Accumulator = 0.f;

    m_PhysicsWorld->SetContactListener(m_ContactListener);

    EVENT_SUBSCRIBE_MEMBER(m_SetImpulse, PhysicsSystem::SetImpulse);
}

bool dd::Systems::PhysicsSystem::SetImpulse(const Events::SetImpulse &event)
{
    b2Body* body = m_EntitiesToBodies[event.Entity];

    b2Vec2 impulse;
    impulse.x = event.Impulse.x;
    impulse.y = event.Impulse.y;

    b2Vec2 point;
    point.x = event.Point.x;
    point.y = event.Point.y;


    Impulse i;
    i.Body = body;
    i.Impulse = impulse;
    i.Point = point;

    m_Impulses.push_back(i);

    return true;
}

void dd::Systems::PhysicsSystem::Update(double dt)
{

    m_Accumulator += dt;
    while(m_Accumulator >= m_TimeStep)
    {

        for (auto i : m_EntitiesToBodies) {
            EntityID entity = i.first;
            b2Body* body = i.second;

            auto transformComponent = m_World->GetComponent<Components::Transform>(entity);
            if (! transformComponent) {
                continue;
                LOG_ERROR("RigidBody with no TransformComponent");
            }
            auto physicsComponent = m_World->GetComponent<Components::Physics>(entity);
            if (! physicsComponent) {
                continue;
                LOG_ERROR("RigidBody with no PhysicsComponent");
            }


            if (body == nullptr) {
                LOG_ERROR("This body should not exist");
                continue;
            }

            if (m_World->GetEntityParent(entity) == 0) { //TODO: Make this work with childs too
                b2Vec2 position;
                position.x = transformComponent->Position.x;
                position.y = transformComponent->Position.y;
                float angle = -glm::eulerAngles(transformComponent->Orientation).z;
                body->SetTransform(position, angle);
                body->SetLinearVelocity(b2Vec2(transformComponent->Velocity.x, transformComponent->Velocity.y));

                body->SetGravityScale(physicsComponent->GravityScale);


                b2Filter filter;
                filter.categoryBits = physicsComponent->Category;
                filter.maskBits = physicsComponent->Mask;
                body->GetFixtureList()->SetFilterData(filter);
            }
        }

        for (auto i : m_Impulses) {
            i.Body->ApplyLinearImpulse(i.Impulse, i.Point, true);
        }
        m_Impulses.clear();




        m_PhysicsWorld->Step(m_TimeStep, m_VelocityIterations, m_PositionIterations);
        m_Accumulator -= dt;



        for (auto i : m_EntitiesToBodies) {
            EntityID entity = i.first;
            b2Body* body = i.second;

            if (body == nullptr) {
                LOG_ERROR("This body should not exist");
                continue;
            }

            auto transformComponent = m_World->GetComponent<Components::Transform>(entity);
            if (! transformComponent)
                continue;
            if (m_World->GetEntityParent(entity) == 0) {
                b2Vec2 position = body->GetPosition();
                transformComponent->Position.x = position.x;
                transformComponent->Position.y = position.y;

                float angle = body->GetAngle();


                transformComponent->Orientation =  glm::quat(glm::vec3(0, 0, -angle));

                b2Vec2 velocity = body->GetLinearVelocity();
                transformComponent->Velocity.x = velocity.x;
                transformComponent->Velocity.y = velocity.y;

            }
        }
    }
}

void dd::Systems::PhysicsSystem::UpdateEntity(double dt, EntityID entity, EntityID parent)
{

}


void dd::Systems::PhysicsSystem::OnEntityCommit(EntityID entity)
{
    auto physicsComponent = m_World->GetComponent<Components::Physics>(entity);
    if(physicsComponent) {
        CreateBody(entity);
    }
}


void dd::Systems::PhysicsSystem::OnEntityRemoved(EntityID entity)
{
    auto physics = m_World->GetComponent<Components::Physics>(entity);
    if (physics == nullptr) {
        return;
    }

    b2Body* body = m_EntitiesToBodies[entity];

    if (body != nullptr) {
        m_EntitiesToBodies.erase(entity);
        m_BodiesToEntities.erase(body);

        m_PhysicsWorld->DestroyBody(body);
    }

}


void dd::Systems::PhysicsSystem::CreateBody(EntityID entity)
{
    auto physicsComponent = m_World->GetComponent<Components::Physics>(entity);
    if(!physicsComponent){
        LOG_ERROR("No PhysicsComponent in CreateBody");
        return;
    }


    auto transformComponent = m_World->GetComponent<Components::Transform>(entity);
    if(!transformComponent) {
        LOG_ERROR("No TransformComponent in CreateBody");
        return;
    }

    auto absoluteTransform = m_World->GetSystem<Systems::TransformSystem>()->AbsoluteTransform(entity);

    b2BodyDef bodyDef;
    bodyDef.position.Set(absoluteTransform.Position.x, absoluteTransform.Position.y);
    bodyDef.angle = -glm::eulerAngles(absoluteTransform.Orientation).z;

    if (physicsComponent->Static) {
        bodyDef.type = b2_staticBody;
    } else {
        bodyDef.type = b2_dynamicBody;

    }

    b2Body* body = m_PhysicsWorld->CreateBody(&bodyDef);

    b2Shape* pShape;

    auto boxComponent = m_World->GetComponent<Components::RectangleShape>(entity);
    if (boxComponent) {
        b2PolygonShape* bShape = new b2PolygonShape();
        bShape->SetAsBox(absoluteTransform.Scale.x/2, absoluteTransform.Scale.y/2); //TODO: THIS SUCKS DUDE 4?!?!?!?
        pShape = bShape;
    } else {
        auto circleComponent = m_World->GetComponent<Components::CircleShape>(entity);
        if (circleComponent) {
            pShape = new b2CircleShape();
            pShape->m_radius = absoluteTransform.Scale.x;


            if (absoluteTransform.Scale.x != absoluteTransform.Scale.y &&  absoluteTransform.Scale.y != absoluteTransform.Scale.z) {
                LOG_WARNING("Circles has to be of uniform scale.");
            }
            pShape->m_radius = absoluteTransform.Scale.x/2; //TODO: THIS ALSO SUCKS 4 WTH

        }
    }

    b2FixtureDef fixtureDef;
    fixtureDef.shape = pShape;
    fixtureDef.filter.categoryBits = physicsComponent->Category;
    fixtureDef.filter.maskBits = physicsComponent->Mask;


    if(physicsComponent->Static) {
        body->CreateFixture(&fixtureDef); //Density kanske ska vara 0 p� statiska kroppar
    }
    else {
        fixtureDef.shape = pShape;
        fixtureDef.density = 1.f;
        fixtureDef.restitution = 1.0f;
        fixtureDef.friction = 0.0f;
        body->CreateFixture(&fixtureDef);

    }

    delete pShape;


    body->SetGravityScale(physicsComponent->GravityScale);
    m_EntitiesToBodies.insert(std::make_pair(entity, body));
    m_BodiesToEntities.insert(std::make_pair(body, entity));
}

dd::Systems::PhysicsSystem::~PhysicsSystem()
{
    if (m_ContactListener != nullptr) {
        delete m_ContactListener;
        m_ContactListener = nullptr;
    }
}
