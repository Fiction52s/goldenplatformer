#include "Actor.h"
#include <iostream>
#include "Action.h"
#include <sstream>

using namespace sf;
using namespace std;
using namespace luabridge;

ActorParams::ActorParams( int health, bool facingRight, 
		bool reversed, const b2Vec2 &spawnPoint, const b2Vec2 &vel, 
		float32 angle, uint32 spriteCount, uint32 hitsReceivedCap, uint32 bodyCollisionsCap, 
		uint32 actorsAttackedCap, TrueActor *owner )
		:health( health ), adjustedVelocity( 0, 0 ), spawnPoint( spawnPoint ), body( NULL ), 
		hitsReceivedSize( 0 ), bodyCollisionsSize( 0 ), actorsAttackedSize( 0 ), actorsAttackedIndex( 0 ), 
		isAlive( false ), owner( owner ), m_paused( false )
{
	hitsReceived = new ActorHit[hitsReceivedCap];
	bodyCollisions = new ActorCollision[bodyCollisionsCap];
	actorsAttacked = new ActorHitOffense[actorsAttackedCap];
	spriteOffset = new b2Vec2[spriteCount];
	
	spriteIsEnabled = new bool[spriteCount];

	for( int i = 0; i < spriteCount; ++i )
	{
		spriteOffset[i].SetZero();
		spriteIsEnabled[i] = false;
	}
	//the index is for when you run out of space and need to start again. This information 
	//can be lost at high values, and is just so that no attack does impossible amounts of damage
	//GetActorsAttackedIndex() = 0;
	
	def.type = (b2BodyType)(owner->bodyType);
	def.position.Set( spawnPoint.x, spawnPoint.y );
	def.linearVelocity.Set( vel.x, vel.y );
	
	def.fixedRotation = owner->fixedAngle;
	def.bullet = true;

	m_restitution = 0;
	m_friction = 0;
}

ActorParams::~ActorParams()
{
	if( body != NULL )
		owner->world->DestroyBody( body );
	body = NULL;

	delete [] hitsReceived;
	delete [] bodyCollisions;
	delete [] actorsAttacked;

	delete [] spriteOffset;
	delete [] spriteIsEnabled;
}

void ActorParams::Init( )
{
	isAlive = true;
	body = owner->world->CreateBody( &def );
	body->SetUserData( owner );
}


void ActorParams::UpdateBoundingBoxes()
{
	b2Fixture *f = body->GetFixtureList();
	aabb.lowerBound = body->GetPosition();
	aabb.upperBound = body->GetPosition();
	while( f != NULL )
	{
		if( f->GetFilterData().categoryBits == 1 << CollisionLayers::PlayerPhysicsbox 
			|| f->GetFilterData().categoryBits == 1 << CollisionLayers::EnemyPhysicsbox )
		{
			b2AABB box = f->GetAABB( 0 );

			if( box.GetCenter().x - box.GetExtents().x < aabb.GetCenter().x - aabb.GetExtents().x )
			{
				aabb.Combine( box );
			}
		}

		f = f->GetNext();
	}
}

void ActorParams::CreateBox( uint32 tag, int layer, 
		float32 offsetX, float32 offsetY, float32 width, 
		float32 height, float32 angle )
{
	b2FixtureDef fdef;
	b2PolygonShape shape;
	b2Vec2 offset( offsetX, offsetY );
	if( owner->IsReversed() )
	{
		offset.y = -offset.y;
		angle = -angle;
	}
	shape.SetAsBox( width / 2, height / 2, offset, angle );
	fdef.shape = &shape;

	CollisionLayers::SetupFixture( (CollisionLayers::Layer)layer, fdef.filter.categoryBits, 
		fdef.filter.maskBits );
	
	fdef.userData = (void*)tag;
	fdef.friction = 0;
	fdef.density = 1;
	body->CreateFixture( &fdef );	
}

void ActorParams::CreateCircle( uint32 tag, int layer, 
		float32 offsetX, float32 offsetY, 
		float32 radius )
{
	b2FixtureDef fdef;
	b2CircleShape shape;
	b2Vec2 offset( offsetX, offsetY );
	if( owner->IsReversed() ) offset.y = -offset.y;
	shape.m_p = offset;
	shape.m_radius = radius;
	fdef.shape = &shape;
	
	CollisionLayers::SetupFixture( (CollisionLayers::Layer)layer, fdef.filter.categoryBits, 
		fdef.filter.maskBits );
	
	fdef.userData = (void*)tag;
	fdef.friction = 0;
	fdef.restitution = 0;
	fdef.density = 1;
	body->CreateFixture( &fdef );
}

void ActorParams::SetVelocity( float x, float y )
{
	b2Vec2 v( x, y );
	if( owner->IsReversed() ) v.y = -v.y;
	body->SetLinearVelocity( v );
}

void ActorParams::ApplyImpulse( float x, float y )
{
	b2Vec2 impulse( x, y );
	body->ApplyLinearImpulse( impulse , body->GetWorldCenter() );
}

const b2Vec2 &ActorParams::GetVelocity()
{
	adjustedVelocity.Set( body->GetLinearVelocity().x, body->GetLinearVelocity().y );
	if( owner->IsReversed() ) adjustedVelocity.y = -adjustedVelocity.y;
	return adjustedVelocity;
}

void ActorParams::SetPosition( float x, float y )
{
	b2Vec2 v( x, y );
	body->SetTransform( v, body->GetAngle() );
}

b2Vec2 ActorParams::GetPosition()
{
	return body->GetPosition();
}

void ActorParams::SetBodyAngle( float angle )
{
	body->SetTransform( body->GetPosition(), angle );
}

float ActorParams::GetBodyAngle()
{
	return body->GetAngle();
}

uint32 ActorParams::GetUniqueID()
{
	return (uint32)body;
}

void ActorParams::SetFriction( float friction )
{
	m_friction = friction;
}

float32 ActorParams::GetFriction()
{
	return m_friction;
}

void ActorParams::SetRestitution( float restitution )
{
	m_restitution = restitution;
}

float32 ActorParams::GetRestitution()
{
	return m_restitution;
}

void ActorParams::SetPause( bool pause )
{
	m_paused = pause;
}

bool ActorParams::IsPaused()
{
	return m_paused;
}

void ActorParams::Kill()
{
	isAlive = false;
}

void ActorParams::SetSpriteOffset( uint32 spriteIndex, float x, float y )
{
	spriteOffset[spriteIndex].Set( x, y );
}

void ActorParams::ClearHitboxes()
{
	b2Fixture *flist = body->GetFixtureList();
	while( flist != NULL )
	{
		b2Fixture *temp = flist->GetNext();
		if( flist->GetFilterData().categoryBits == 1 << CollisionLayers::PlayerHitbox
			|| flist->GetFilterData().categoryBits == 1 << CollisionLayers::EnemyHitbox )
		{
			body->DestroyFixture( flist );
		}
		flist = temp;
	}
}

void ActorParams::ClearHurtboxes()
{
	b2Fixture *flist = body->GetFixtureList();
	while( flist != NULL )
	{
		b2Fixture *temp = flist->GetNext();
		if( flist->GetFilterData().categoryBits == 1 << CollisionLayers::PlayerHurtbox
			|| flist->GetFilterData().categoryBits == 1 << CollisionLayers::EnemyHurtbox )
		{
			body->DestroyFixture( flist );
		}
		flist = temp;
	}
}

void ActorParams::ClearPhysicsboxes()
{
	b2Fixture *flist = body->GetFixtureList();
	while( flist != NULL )
	{
		b2Fixture *temp = flist->GetNext();
		if( flist->GetFilterData().categoryBits == 1 << CollisionLayers::PlayerPhysicsbox
			|| flist->GetFilterData().categoryBits == 1 << CollisionLayers::EnemyPhysicsbox )
		{
			body->DestroyFixture( flist );
		}
		flist = temp;
	}
}

void ActorParams::ClearDetectionboxes()
{
	b2Fixture *flist = body->GetFixtureList();
	while( flist != NULL )
	{
		b2Fixture *temp = flist->GetNext();
		if( flist->GetFilterData().categoryBits == 1 << CollisionLayers::PlayerDetectionbox
			|| flist->GetFilterData().categoryBits == 1 << CollisionLayers::ActorDetectionbox )
		{
			body->DestroyFixture( flist );
		}
		flist = temp;
	}
}

void ActorParams::ClearActorsAttacked()
{
	actorsAttackedSize = 0;
	actorsAttackedIndex = 0;
}