#include "PrecompiledHeader.h"
#include "Physics/PhysicsSystem.h"

void dd::Systems::PhysicsSystem::ContactListener::BeginContact(b2Contact* contact)
{
    b2WorldManifold worldManifold;

    contact->GetWorldManifold(&worldManifold);

    Events::Contact e;
    e.Entity1 = m_PhysicsSystem->m_BodiesToEntities[contact->GetFixtureA()->GetBody()];
    e.Entity2 = m_PhysicsSystem->m_BodiesToEntities[contact->GetFixtureB()->GetBody()];
    e.Normal = glm::normalize(glm::vec2(contact->GetManifold()->localNormal.x, contact->GetManifold()->localNormal.y));
    e.SignificantNormal = glm::normalize((glm::abs(e.Normal.x) > glm::abs(e.Normal.y)) ? glm::vec2(e.Normal.x, 0) : glm::vec2(0, e.Normal.y));

    m_PhysicsSystem->EventBroker->Publish(e);
}

void dd::Systems::PhysicsSystem::ContactListener::EndContact(b2Contact* contact)
{

}

void dd::Systems::PhysicsSystem::ContactListener::PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
{
    EntityID entityA = m_PhysicsSystem->m_BodiesToEntities[contact->GetFixtureA()->GetBody()];
    EntityID entityB = m_PhysicsSystem->m_BodiesToEntities[contact->GetFixtureB()->GetBody()];

    auto physicsComponentA = m_PhysicsSystem->m_World->GetComponent<Components::Physics>(entityA);
    auto physicsComponentB = m_PhysicsSystem->m_World->GetComponent<Components::Physics>(entityB);

    if (physicsComponentA != nullptr && physicsComponentB != nullptr) {
        if (physicsComponentA->Calculate || physicsComponentB->Calculate) {
            // Turn of collisions
            contact->SetEnabled(false);
        }
    }


}

void dd::Systems::PhysicsSystem::ContactListener::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse)
{

}

