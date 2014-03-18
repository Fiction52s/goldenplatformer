#ifndef __COLLISION_H__
#define __COLLISION_H__

#include <Box2D.h>
#include <string>
#include "Globals.h"

class Stage;
namespace CollisionLayers
{
	enum Layer
	{
		Event = 0,
		Environment,
		PlayerHitbox,
		PlayerHurtbox,
		PlayerPhysicsbox,
		EnemyHitbox,
		EnemyHurtbox,
		EnemyPhysicsboxWithPlayer, //collides with the player
		EnemyPhysicsbox, //collides with only the env
		//EnemyPhysicsboxWithEnemy, //collides with other enemy physics boxes
		ActorDetectionbox,
		PlayerDetectionbox,
		PlayerEventCollisionbox,
		ActivateBox,
		Door
	};
	void SetupFixture(CollisionLayers::Layer layer ,
		uint16 &categoryBits, uint16 &maskbits );
}

struct TrueActor;
struct StageCollision
{
	StageCollision( const std::string &t, 
		b2Vec2 &tile, TrueActor *a, b2Vec2 &fraction, 
		b2Vec2 &normal, bool en );
	TrueActor *m_actor;
	b2Vec2 m_fraction;
	b2Vec2 m_normal;
	b2Vec2 m_tile;
	std::string m_type;
	bool m_enabled;
};


class ContactListener : public b2ContactListener
{
public:
	void BeginContact(b2Contact* contact);
	
	void EndContact(b2Contact* contact);
	
	void PreSolve(b2Contact* contact, 
		const b2Manifold* oldManifold);
	
	void PostSolve(b2Contact* contact, 
		const b2ContactImpulse* impulse);
	Stage *stage;
	b2Vec2 doorEnterPos;
};

#endif