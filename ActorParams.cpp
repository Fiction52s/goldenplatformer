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
	
	save_hitsReceived = new ActorHit[hitsReceivedCap];
	save_bodyCollisions = new ActorCollision[bodyCollisionsCap];
	save_actorsAttacked = new ActorHitOffense[actorsAttackedCap];
	save_spriteOffset = new b2Vec2[spriteCount];

	spriteIsEnabled = new bool[spriteCount];

	save_spriteIsEnabled = new bool[spriteCount];

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
	body->SetActive( !pause );
	
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
	cout << "setting offset: " << x << ", " << y << endl;
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

void ActorParams::SaveState()
{
	
	save_isAlive = isAlive;
	save_friction = m_friction;
	save_restitution = m_restitution;
	save_paused = m_paused;
	save_hitsReceivedSize = hitsReceivedSize;
	for( int i = 0; i < save_hitsReceivedSize; ++i )
	{
		save_hitsReceived[i] = hitsReceived[i]; 
	}
	save_bodyCollisionsSize = bodyCollisionsSize;
	for( int i = 0; i < save_bodyCollisionsSize; ++i )
	{
		save_bodyCollisions[i] = bodyCollisions[i];
	}
	save_actorsAttackedSize = actorsAttackedSize;
	save_actorsAttackedIndex = actorsAttackedIndex;
	for( int i = 0; i < owner->spriteCount; ++i )
	{
		save_spriteIsEnabled[i] = spriteIsEnabled[i];
		save_spriteOffset[i] = spriteOffset[i];
	}
	save_position = body->GetPosition();
	save_velocity = GetVelocity();
	save_angle = body->GetAngle();
	

	b2PolygonShape *s = NULL;
	b2Vec2 *vecs = NULL;
	b2PolygonShape *newPolygonShape = NULL;
	b2CircleShape *cs = NULL;
	b2CircleShape *newCircleShape = NULL;

	b2Fixture *f = body->GetFixtureList();

	save_fixtureDefs.clear();

	while( f != NULL )
	{
		
		b2Shape *shape = f->GetShape();
		//b2Shape *newShape = NULL;
		switch( shape->GetType() )
		{
			case b2Shape::e_circle:
				cs = (b2CircleShape*)shape;
				newCircleShape = new b2CircleShape;
				newCircleShape->m_radius = cs->m_radius;
				newCircleShape->m_p = cs->m_p;

				save_fixtureDefs.push_back( b2FixtureDef() );
				save_fixtureDefs.back().shape = newCircleShape;
				break;
			case b2Shape::e_polygon:
				s = (b2PolygonShape*)shape;
				vecs = new b2Vec2[s->GetVertexCount()];
				for( int i = 0; i < 4; ++i )
				{
					vecs[i] = s->m_vertices[i];
				}

				newPolygonShape = new b2PolygonShape;
				newPolygonShape->Set( vecs, s->m_vertexCount );

				save_fixtureDefs.push_back( b2FixtureDef() );
				save_fixtureDefs.back().shape = newPolygonShape;
				break;
			default:
				assert( 0 );
				break;
		}

		b2FixtureDef &fd = save_fixtureDefs.back();
		fd.density = f->GetDensity();
		fd.filter = f->GetFilterData();
		fd.friction = f->GetFriction();
		fd.isSensor = f->IsSensor();
		fd.restitution = f->GetRestitution();
		fd.userData = f->GetUserData();

		f = f->GetNext();
	}
	
}

void ActorParams::LoadState()
{
	//cout << "load state: " << owner->actorType << endl;
	isAlive = save_isAlive;
	m_friction = save_friction;
	m_restitution = save_restitution;
	SetPause( save_paused );
	//m_paused = save_paused;
	hitsReceivedSize = save_hitsReceivedSize;
	for( int i = 0; i < hitsReceivedSize; ++i )
	{
		hitsReceived[i] = save_hitsReceived[i]; 
	}

	bodyCollisionsSize = save_bodyCollisionsSize;
	for( int i = 0; i < bodyCollisionsSize; ++i )
	{
		bodyCollisions[i] = save_bodyCollisions[i];
	}

	actorsAttackedSize = save_actorsAttackedSize;
	actorsAttackedIndex = save_actorsAttackedIndex;
	for( int i = 0; i < owner->spriteCount; ++i )
	{
		spriteIsEnabled[i] = save_spriteIsEnabled[i];
		spriteOffset[i] = save_spriteOffset[i];
	}
	//cout << "position: " << GetPosition().x << ", " << GetPosition().y << endl;
	SetPosition( save_position.x, save_position.y );
	//cout << "new position: " << GetPosition().x << ", " << GetPosition().y << endl;
	SetBodyAngle( save_angle );
	SetVelocity( save_velocity.x, save_velocity.y );

	
	b2Fixture *f = body->GetFixtureList();
	while( f != NULL )
	{
		b2Fixture *toDestroy = f;
		f = f->GetNext();
		body->DestroyFixture( toDestroy );
	}

	for( list<b2FixtureDef>::iterator it = save_fixtureDefs.begin(); it != save_fixtureDefs.end(); ++it )
	{
		body->CreateFixture( &(*it) );
	}
}