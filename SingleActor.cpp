#include "Actor.h"
#include <iostream>
#include "Action.h"
#include <sstream>

using namespace sf;
using namespace std;
using namespace luabridge;

SingleActor::SingleActor( const std::string &actorType, const b2Vec2 &pos, const b2Vec2 &vel,
		bool facingRight, bool reverse, float32 angle, TrueActor *parent, Stage *st )
		:TrueActor( actorType, pos, vel, facingRight, reverse, angle, parent, st )
{
	//set up lua stuff here


	isGroup = false;
	sprite = new sf::Sprite*[ spriteCount ];
	m_spriteAngle = new float32[ spriteCount ];
	//spriteIsEnabled = new bool[ spriteCount ];

	save_sprite = new sf::Sprite*[spriteCount];	
	save_spriteAngle = new float32[ spriteCount ];
	save_color = new sf::Color[ spriteCount ];

	for( int i = 0; i < spriteCount; ++i )
	{
		//spriteIsEnabled[i] = false;
		m_spriteAngle[i] = 0;
		sprite[i] = new sf::Sprite();
		

		//save_spriteAngle[i] = 0;
		save_sprite[i] = new sf::Sprite();
		
	}



	actorParams = new ActorParams( 0, facingRight, reverse, pos, vel, angle, spriteCount, 
		hitsReceivedCap, bodyCollisionsCap, actorsAttackedCap, this );

	push( L, this );
	lua_setglobal( L, "actor" );
}

SingleActor::~SingleActor()
{
	lua_getglobal( L, "Die" );
	lua_pcall( L, 0, 0, 0 );
	lua_close( L );	

	for( int i = 0; i < spriteCount; ++i )
	{
		delete sprite[i];
		delete save_sprite[i];
	}
	delete [] sprite;
	delete [] save_sprite;
	delete [] m_spriteAngle;
	delete [] save_spriteAngle;
	//delete [] spriteIsEnabled;

	

	delete actorParams;
}

ActorHit * & SingleActor::GetHitsReceived()
{
	return actorParams->hitsReceived;
}

uint32 & SingleActor::GetHitsReceivedSize()
{
	return actorParams->hitsReceivedSize;
}

ActorCollision * & SingleActor::GetBodyCollisions()
{
	return actorParams->bodyCollisions;
}

uint32 & SingleActor::GetBodyCollisionsSize()
{
	return actorParams->bodyCollisionsSize;
}

ActorHitOffense * & SingleActor::GetActorsAttacked()
{
	return actorParams->actorsAttacked;
}

uint32 & SingleActor::GetActorsAttackedSize()
{
	return actorParams->actorsAttackedSize;
}

uint32 & SingleActor::GetActorsAttackedIndex()
{
	return actorParams->actorsAttackedIndex;
}

void SingleActor::Init( b2World *p_world )
{
	isAlive = true;
	
	world = p_world;
	actorParams->Init( );
	
	if( !facingRight ) 
	{
		facingRight = true; //so that it can be turned to the left again
		FaceLeft();
	}

	//this will set up the actions, set an initial action, 
	lua_getglobal( L, "Init" );
	lua_pcall( L, 0, 0, 0 );

	actorParams->UpdateBoundingBoxes();
}

void SingleActor::FaceLeft()
{
	if( facingRight )
	{
		for( int i = 0; i < spriteCount; ++i )
		{
			sprite[i]->scale( -1, 1 );
		}
		facingRight = false;
	}
}

void SingleActor::FaceRight()
{
	if( !facingRight )
	{
		for( int i = 0; i < spriteCount; ++i )
		{
			sprite[i]->scale( -1, 1 );
		}
		
		facingRight = true;
	}
}

void SingleActor::Draw( sf::RenderTarget *target )
{
	//for( int i = spriteCount-1; i >=0 ; --i )
	for( int i = 0; i < spriteCount; ++i )
	{
		if( actorParams->spriteIsEnabled[i] )
		{
			target->draw( *(sprite[i]) );
		}
	}
}

void SingleActor::CloneDraw( sf::RenderTarget *target )
{
	for( int i = 0; i < spriteCount; ++i )
	{
		if( actorParams->save_spriteIsEnabled[i] )
		{
			target->draw( *(save_sprite[i]) );
		}
	}
}



void SingleActor::ClearHitboxes()
{
	actorParams->ClearHitboxes();
}

void SingleActor::ClearHurtboxes()
{
	actorParams->ClearHurtboxes();
}

void SingleActor::ClearPhysicsboxes()
{
	actorParams->ClearPhysicsboxes();
}

void SingleActor::ClearDetectionboxes()
{
	actorParams->ClearDetectionboxes();
}

HitboxInfo::HitboxInfo( bool circle, uint32 tag, float32 offsetX, float32 offsetY, float32 width, float32 height,
	float32 angle )
	:circle( circle ), tag( tag ), offsetX( offsetX ), offsetY( offsetY ), width( width ), height( height ), 
	angle( angle )
{

}

PlayerGhost::PlayerGhost( Stage *stage )
	:recordFrame( 0 ), playFrame( 0 ), body( NULL )
{
	b2BodyDef d;
	d.type = b2BodyType::b2_staticBody;
	d.active = false;
	//d.bullet = true;
	d.angle = 0;
	d.fixedRotation = true;
	d.position = stage->player->GetPosition();
	body = stage->world->CreateBody( &d );
}

void SingleActor::CreateBox( uint32 tag, int layer, 
	float32 offsetX, float32 offsetY, 
	float32 width, float32 height, 
	float32 angle )
{
	
	actorParams->CreateBox( tag, layer, offsetX, offsetY, width, height, angle );
}

void SingleActor::CreateCircle( uint32 tag, int layer, 
	float32 offsetX, float32 offsetY, 
	float32 radius )
{
	actorParams->CreateCircle( tag, layer, offsetX, offsetY, radius );
}

void SingleActor::SetSprite( uint32 spriteIndex,
	uint32 tsIndex, uint32 localID )
{
	sprite[spriteIndex]->setTexture( *stage->tileSets[tsIndex]->texture );
	sprite[spriteIndex]->setTextureRect( stage->tileSets[tsIndex]->GetSubRect( localID ) );
	sprite[spriteIndex]->setOrigin( sprite[spriteIndex]->getLocalBounds().width / 2.f, 
		sprite[spriteIndex]->getLocalBounds().height / 2.f );
}

bool SingleActor::UpdatePrePhysics()
{
	lua_getglobal( L, "UpdatePrePhysics" );
	lua_pcall( L, 0, 0, 0 );

	actorParams->UpdateBoundingBoxes();

	return true;
}

bool SingleActor::ProcessCollisions()
{
	for( int i = 0; i < GetHitsReceivedSize(); ++i )
	{	
		ActorHit &h = GetHitsReceived()[i];
		lua_getglobal( L, "HitByActor" );
		push( L, h.actor );
		push( L, h.hitboxName );
		push( L, h.damage );
		push( L, h.hitlag );
		push( L, h.hitstun );
		push( L, h.hurtboxTag );
		push( L, h.centerX );
		
		
		lua_pcall( L, 7, 1, 0 );
		if( !lua_isboolean ( L, -1 ) )
		{
			assert( 0 && "needs to be a boolean" );
		}
		bool confirmed = lua_toboolean( L, -1 );
		lua_pop( L, 1 );
		if( confirmed )
		{
			if( h.actor->isGroup )
			{
				GroupActor *ga = (GroupActor*)h.actor;
				ga->SetCurrentBody( h.otherBody );
			}

			lua_getglobal(h.actor->L, "ConfirmHit" );
			push( h.actor->L, this );
			push( h.actor->L, h.hitboxName );
			push( h.actor->L, h.damage );
			push( h.actor->L, h.hitlag );
			push( h.actor->L, h.hitstun );
			lua_pcall( h.actor->L, 5, 0, 0 );
		}

		//delete (*it);
	}
	GetHitsReceivedSize() = 0;
	//hitsReceived.clear();
	//if( bodyCollisionsSize > 0 ) cout << "bodycolsize: " << bodyCollisionsSize << endl;
	for( int i = 0; i < GetBodyCollisionsSize(); ++i )
	{
		ActorCollision &c = GetBodyCollisions()[i];
		lua_getglobal( L, "HandleActorCollision" );
		push( L, c.actor );
		push( L, c.hurtboxTag );
		push( L, c.pointCount );
		push( L, &c.point1 );
		push( L, &c.point2 );
		push( L, &c.normal );
		lua_pushboolean( L, c.enabled );
		
		lua_pcall( L, 7, 0, 0 );
	}
	GetBodyCollisionsSize() = 0;

	return false;
}

void SingleActor::UpdateSprites()
{
	//cout << "update sprites" << endl;
	for( int i = 0; i < spriteCount; ++i )
	{
		float a = m_spriteAngle[i] / PI;
		if( (this->IsFacingRight()) )
		{
			
		}
		a = -a;
		sprite[i]->setRotation( m_spriteAngle[i] * 180 );
		sprite[i]->setPosition( ( actorParams->body->GetPosition().x 
			+ actorParams->spriteOffset[i].x * cos( a )
		+ actorParams->spriteOffset[i].y * sin( a ) ) * BOX2SF, 
			( actorParams->GetPosition().y + actorParams->spriteOffset[i].y * cos( a )
			+ actorParams->spriteOffset[i].x * sin( a ) ) * BOX2SF ); 
		//sprite[i]->setColor( sf::Color( 0,255,255 ) );
		if( actorType == "player" )
		{
			//cout << "offset: " << sprite[i]->getPosition().x - actorParams->body->GetPosition().x * BOX2SF
			//	<< ", " << sprite[i]->getPosition().y - actorParams->body->GetPosition().y * BOX2SF << endl;
		}
		//sprite[i]->setPosition( ( actorParams->body->GetPosition().x 
		//	+ actorParams->spriteOffset[i].x ) * BOX2SF, 
		//	( actorParams->GetPosition().y + actorParams->spriteOffset[i].y ) * BOX2SF ); 
	}		
}

void SingleActor::SetVelocity( float x, float y )
{
	actorParams->SetVelocity( x, y );
}

const b2Vec2 &SingleActor::GetVelocity()
{
	return actorParams->GetVelocity();
}

void SingleActor::SetPosition( float x, float y )
{
	actorParams->SetPosition( x, y );
}

b2Vec2 SingleActor::GetPosition()
{
	return actorParams->GetPosition();
}

void SingleActor::SetBodyAngle( float angle )
{
	actorParams->SetBodyAngle( angle );
}

void SingleActor::SetSpriteOffset( uint32 spriteIndex,
	float x, float y )
{
	actorParams->SetSpriteOffset( spriteIndex, x, y );
}

void SingleActor::SetSpriteAngle( uint32 index, 
	float32 angle )
{
	assert( index >= 0 && index < spriteCount );
	if( isReversed )
	{
		angle = -angle;
	}
	m_spriteAngle[index] = angle;
}

float32 SingleActor::GetSpriteAngle( uint32 index )
{
	return m_spriteAngle[index];
}

void SingleActor::SetSpriteEnabled( uint32 spriteIndex, bool enabled )
{
	actorParams->spriteIsEnabled[spriteIndex] = enabled;
}


b2Vec2 & SingleActor::GetSpriteOffset( uint32 spriteIndex )
{
//	cout << "getting sprite offset" << endl;
	return actorParams->spriteOffset[spriteIndex];
}

uint32 SingleActor::GetUniqueID()
{
	return actorParams->GetUniqueID();
}

float SingleActor::GetWorldLeft()
{
	return actorParams->aabb.lowerBound.x;
}

float SingleActor::GetWorldRight()
{
	return actorParams->aabb.upperBound.x;
}

float SingleActor::GetWorldTop()
{
	return actorParams->aabb.lowerBound.y;
}

float SingleActor::GetWorldBottom()
{
	return actorParams->aabb.upperBound.y;
}

void SingleActor::SetFriction( float friction )
{
	actorParams->SetFriction( friction );
}

float32 SingleActor::GetFriction()
{
	return actorParams->GetFriction();
}

void SingleActor::SetRestitution( float restitution )
{
	actorParams->SetRestitution( restitution );
}

float32 SingleActor::GetRestitution()
{
	return actorParams->GetRestitution();
}

void SingleActor::Kill()
{
	isAlive = false;
	actorParams->Kill();
}

void SingleActor::Reverse()
{
	isReversed = !isReversed;
	for( int i = 0; i < spriteCount; ++i )
	{
		sprite[i]->scale( 1, -1 );
	}
}

void SingleActor::SetPause( bool pause )
{
	actorParams->SetPause( pause );
}

bool SingleActor::IsPaused()
{
	return actorParams->IsPaused();
}

void SingleActor::ClearActorsAttacked()
{
	actorParams->ClearActorsAttacked();
}

void SingleActor::SetColor( uint32 spriteIndex, uint8 r, uint8 g, uint8 b, uint8 a )
{
	//for( int i = 0; i < spriteCount; ++i )
	//{
	sprite[spriteIndex]->setColor( sf::Color(r,g,b,a) );
	//}
}

void SingleActor::SetSpriteScale( uint32 spriteIndex, float x, float y )
{
	if( !IsFacingRight() )
	{
		x = -x;
	}
	if( IsReversed() )
	{
		y = -y;
	}
	
	sprite[spriteIndex]->setScale( x, y );
}

float SingleActor::GetBodyAngle()
{
	return actorParams->GetBodyAngle();
}

b2Body * SingleActor::GetBody()
{
	return actorParams->body;
}

void SingleActor::SaveState()
{
	for( int i = 0; i < spriteCount; ++i )
	{
		save_spriteAngle[i] = m_spriteAngle[i];
		save_sprite[i]->setScale( sprite[i]->getScale() );
		save_sprite[i]->setOrigin( sprite[i]->getOrigin() );
		save_sprite[i]->setPosition( sprite[i]->getPosition() );
		save_sprite[i]->setRotation( sprite[i]->getRotation() );
		
		save_sprite[i]->setTexture( *(sprite[i]->getTexture()) );
		save_sprite[i]->setTextureRect( sprite[i]->getTextureRect() );
		sf::Color color = sprite[i]->getColor();
		save_color[i] = color;
		color.a = 80;
		save_sprite[i]->setColor( color );

		
		
		//(*(save_sprite[i])) *sprite;
	//	(*(save_sprite[i])) = *sprite;
	//	save_sprite[i]->
	}

	actorParams->SaveState();

	TrueActor::SaveState();
}

void SingleActor::LoadState()
{
	for( int i = 0; i < spriteCount; ++i )
	{
		m_spriteAngle[i] = save_spriteAngle[i];

		sprite[i]->setScale( save_sprite[i]->getScale() );
		sprite[i]->setOrigin( save_sprite[i]->getOrigin() );
		sprite[i]->setPosition( save_sprite[i]->getPosition() );
		sprite[i]->setRotation( save_sprite[i]->getRotation() );
		sprite[i]->setTexture( *(save_sprite[i]->getTexture()) );
		sprite[i]->setTextureRect( save_sprite[i]->getTextureRect() );
		sprite[i]->setColor( save_color[i] );

	}

	actorParams->LoadState();

	TrueActor::LoadState();

}