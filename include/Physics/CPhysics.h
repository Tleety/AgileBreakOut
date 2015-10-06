#ifndef DAYDREAM_CPHYSICS_H
#define DAYDREAM_CPHYSICS_H
#include "Core/Component.h"

namespace dd
{

namespace CollisionLayer
{
enum Type
{
    Pad = 1 << 0,
    Ball = 1 << 1,
    Brick = 1 << 2,
    Water = 1 << 3,
    PowerUp = 1 << 4,
    Other = 1 << 5,
    Wall = 1 << 6
};
}
namespace Components
{

struct Physics: public Component
{
    bool Static = true;
    bool Calculate = false;
    float GravityScale = 0.f;
    CollisionLayer::Type Category = CollisionLayer::Type::Other;
	CollisionLayer::Type Mask =	static_cast<CollisionLayer::Type>(
		CollisionLayer::Pad
		| CollisionLayer::Ball
		| CollisionLayer::Brick
		| CollisionLayer::Water
		| CollisionLayer::PowerUp
		| CollisionLayer::Wall
		| CollisionLayer::Other
	);
};

}
}

#endif //DAYDREAM_CPHYSICS_H
