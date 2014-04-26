#include "Actor.h"
#include <iostream>
#include "Action.h"
#include <sstream>

using namespace sf;
using namespace std;
using namespace luabridge;

GroupActor::GroupActor( const std::string &actorType,
		uint32 p_actorCount,
		const b2Vec2 &pos, const b2Vec2 &vel,
		bool facingRight, bool reverse, float32 angle,
		TrueActor *parent, Stage *st )
		:TrueActor( actorType, pos, vel, facingRight, reverse, angle, parent, st ),
		vertexArray( NULL ), actorCount( p_actorCount )
{
	
	isGroup = true;
	//not necessarily like this. i could call a lua function to determine quads and spritecount
	vertexArray = new sf::VertexArray[spriteCount];
	spriteAngle = new float[spriteCount];
	spriteScale = new sf::Vector2f[spriteCount];

	for( int i = 0; i < spriteCount; ++i )
	{
		vertexArray[i].resize( actorCount * 4 );
		vertexArray[i].setPrimitiveType( sf::Quads );
		angle = 0;
		spriteScale[i] = sf::Vector2f( 1, 1 );
	}
	

	actorParams = new ActorParams*[ actorCount ];
	for( int i = 0; i < actorCount; ++i )
	{
		actorParams[i] = new ActorParams( /*health*/0, facingRight, reverse, pos, vel, angle, spriteCount,
			hitsReceivedCap, bodyCollisionsCap, actorsAttackedCap, this );
	}
	
	//cout << "something: " << actorParams[0]->isAlive << endl;

	transform = new sf::Transform[spriteCount];

	
}

GroupActor::~GroupActor()
{
	lua_getglobal( L, "Die" );
	lua_pcall( L, 0, 0, 0 );
	lua_close( L );	

	delete [] vertexArray;
	delete [] transform;
	for( int i = 0; i < actorCount; ++i )
	{
		delete actorParams[i];
	}
	delete [] actorParams;
	delete [] spriteAngle;
	delete [] spriteScale;
}

void GroupActor::Init( b2World *p_world )
{
	isAlive = true;
	//init first?
	push( L, this );
	lua_setglobal( L, "actor" );
	//lua init
	world = p_world;
	for( int i = 0; i < actorCount; ++i )
	{
		actorParams[i]->Init();
	}

	if( !facingRight ) 
	{
		for( int i = 0; i < spriteCount; ++i )
		{
			transform[i].scale( -1, 1 );
		}
		
		//facingRight = true; //so that it can be turned to the left again
		//FaceLeft();
	}

	
	
	lua_getglobal( L, "Init" );
	lua_pcall( L, 0, 0, 0 );
	
	for( uint32 i = 0; i < spriteCount; ++i )
	{
		SetSpritePriority( i, spritePriority[i] );
	}

	for( int i = 0; i < actorCount; ++i )
	{
		actorParams[i]->UpdateBoundingBoxes();
	}
	
}

void GroupActor::FaceLeft()
{
	if( facingRight )
	{
		for( int i = 0; i < spriteCount; ++i )
		{
			transform[i].scale( -1, 1 );
		}
		facingRight = false;
	}
}

void GroupActor::FaceRight()
{
	if( !facingRight )
	{
		for( int i = 0; i < spriteCount; ++i )
		{
			transform[i].scale( -1, 1 );
		}
		facingRight = true;
	}
}

void GroupActor::Draw( sf::RenderTarget *target, uint32 spriteIndex )
{
	//for( int i = 0; i < spriteCount; ++i )
	//{
		sf::RenderStates states( stage->tileSets[tileSetIndex]->texture );
		//transform[i] = sf::Transform::Identity;
	//	if( parent != NULL )

			//transform[i].rotate( 5, parent->GetPosition().x * BOX2SF, parent->GetPosition().y * BOX2SF );
		//transform[i].rotate( 1 );
		states.transform = transform[spriteIndex];
		target->draw( vertexArray[spriteIndex], states );
	//}
}

void GroupActor::CloneDraw( sf::RenderTarget *target )
{
	/*for( int i = 0; i < spriteCount; ++i )
	{
		sf::RenderStates states( stage->tileSets[tileSetIndex]->texture );
		//transform[i] = sf::Transform::Identity;
	//	if( parent != NULL )

			//transform[i].rotate( 5, parent->GetPosition().x * BOX2SF, parent->GetPosition().y * BOX2SF );
		//transform[i].rotate( 1 );
		states.transform = transform[i];
		target->draw( vertexArray[i], states );
	}*/
}



void GroupActor::ClearHitboxes()
{
	actorParams[actorIndex]->ClearHitboxes();
}

void GroupActor::ClearHurtboxes()
{
	actorParams[actorIndex]->ClearHurtboxes();
}

void GroupActor::ClearPhysicsboxes()
{
	actorParams[actorIndex]->ClearPhysicsboxes();
}

void GroupActor::ClearDetectionboxes()
{
	actorParams[actorIndex]->ClearDetectionboxes();
}

void GroupActor::CreateBox( uint32 tag,
	int layer, float32 offsetX, 
	float32 offsetY, float32 width, 
	float32 height, float32 angle )
{
	actorParams[actorIndex]->CreateBox( tag, layer, offsetX, offsetY, width, height, angle );
}

void GroupActor::CreateCircle( uint32 tag, int layer, 
	float32 offsetX, float32 offsetY, 
	float32 radius )
{
	actorParams[actorIndex]->CreateCircle( tag, layer, offsetX, offsetY, radius );
}

/*void GroupActor::SetTileSet( uint32 tsIndex )
{
	tileSetIndex = tsIndex;
}*/

void GroupActor::SetSprite( uint32 spriteIndex, uint32 tsIndex, uint32 localID )
{
	tileSetIndex = tsIndex;
	TileSet *ts = stage->tileSets[tileSetIndex];
	int i = actorIndex * 4;
	
	float blend = 0.f;
	sf::IntRect r = ts->GetSubRect( localID );
	TileSet *f; 

	for( int ai = 0; ai < actorCount; ++ai )
	{
		//actorParams[ai]->spriteIsEnabled[spriteIndex] = true;
	}

	vertexArray[spriteIndex][i].texCoords = sf::Vector2f( r.left - blend, r.top - blend );
	vertexArray[spriteIndex][i+1].texCoords = sf::Vector2f( r.left - blend, r.top + r.height + blend );
	vertexArray[spriteIndex][i+2].texCoords = sf::Vector2f( r.left + r.width + blend, r.top + r.height + blend);
	vertexArray[spriteIndex][i+3].texCoords = sf::Vector2f( r.left + r.width + blend, r.top - blend );
}

bool GroupActor::UpdatePrePhysics()
{
	lua_getglobal( L, "UpdatePrePhysics" );	
	lua_pcall( L, 0, 0, 0 );

	for( int i = 0; i < actorCount; ++i )
	{
		actorParams[i]->UpdateBoundingBoxes();
	}
	return true;
}

bool GroupActor::ProcessCollisions()
{
	for( int aIndex = 0; aIndex < actorCount; ++aIndex )
	{
		ActorParams *ap = actorParams[aIndex];
		for( int i = 0; i < ap->hitsReceivedSize; ++i )
		{	
			ActorHit &h = ap->hitsReceived[i];
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

				SetCurrentBody( h.ownBody );

				lua_getglobal( h.actor->L, "ConfirmHit" );
				push( h.actor->L, this );
				push( h.actor->L, h.hitboxName );
				push( h.actor->L, h.damage );
				push( h.actor->L, h.hitlag );
				push( h.actor->L, h.hitstun );
				lua_pcall( h.actor->L, 5, 0, 0 );
			}

			//delete (*it);
		}
		ap->hitsReceivedSize = 0;
			//GetHitsReceived().clear();
	//if( GetBodyCollisionsSize() > 0 ) cout << "bodycolsize: " << GetBodyCollisionsSize() << endl;
		for( int i = 0; i < ap->bodyCollisionsSize; ++i )
		{
			ActorCollision &c = ap->bodyCollisions[i];
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
		ap->bodyCollisionsSize = 0;

	}
	return false;
}

void GroupActor::UpdateSprites()
{
	for( int spriteIndex = 0; spriteIndex < spriteCount; ++spriteIndex )
	{
		transform[spriteIndex] = sf::Transform::Identity;
		transform[spriteIndex].rotate( spriteAngle[spriteIndex], origin );
	}

	for( int aIndex = 0; aIndex < actorCount; ++aIndex )
	{
		//cout << "tile set index: " << tileSetIndex << endl;
		//if( actorParams[aIndex]->IsPaused() ) continue;
		for( int spriteIndex = 0; spriteIndex < spriteCount; ++spriteIndex )
		{
			TileSet *ts = stage->tileSets[tileSetIndex];
			const b2Vec2 &bodyPos = actorParams[aIndex]->body->GetPosition();
			const b2Vec2 &offset = actorParams[aIndex]->spriteOffset[spriteIndex];
			sf::Vector2f pos( ( bodyPos.x + offset.x ) * BOX2SF, ( bodyPos.y + offset.y ) * BOX2SF );
			pos = transform[spriteIndex].getInverse().transformPoint( pos );

			float halfWidth = ts->tileWidth / 2.f;//floor( ts->tileWidth / 2.f; + .5f );
			float halfHeight = ts->tileHeight / 2.f;//floor( ts->tileHeight / 2.f + .5f );

			if( !(actorParams[aIndex]->spriteIsEnabled[spriteIndex]) )
			{
				halfWidth = 0;
				halfHeight = 0;
			}

			
			//sf::Transform bodyAngleTransform;
			//bodyAngleTransform = sf::Transform::Identity;
			//float angle = 90;

			//bodyAngleTransform.translate( pos.x, pos.y );
			//bodyAngleTransform.rotate( actorParams[aIndex]->body->GetAngle() * 180 / PI );
			//bodyAngleTransform.rotate( angle );
			//pos = bodyAngleTransform.transformPoint( pos );
			//pos = transform[spriteIndex].getInverse().transformPoint( pos );
			

			float left = pos.x - halfWidth;
			float right = pos.x + halfWidth;
			float top = pos.y - halfWidth;
			float bottom = pos.y + halfWidth;

			

			int i = aIndex * 4;
			vertexArray[spriteIndex][i].position = 
				sf::Vector2f( left , top );
			vertexArray[spriteIndex][i+1].position = 
				sf::Vector2f( left, bottom);
			vertexArray[spriteIndex][i+2].position = 
				sf::Vector2f( right, bottom );
			vertexArray[spriteIndex][i+3].position = 
				sf::Vector2f( right, top );


			//vertexArray[spriteIndex][i].position = bodyAngleTransform.transformPoint( vertexArray[spriteIndex][i].position );
		}
	}
	
	//this may only be needed for the single actor



	//for( int i = 0; i < spriteCount; ++i )
	//{
		//sprite[i]->setRotation( m_spriteAngle[i] * 180 );
		//sprite[i]->setPosition( ( m_body->GetPosition().x + spriteOffset[i].x ) * BOX2SF, 
		//	( m_body->GetPosition().y + spriteOffset[i].y ) * BOX2SF ); 
	//}	
}

void GroupActor::SetColor( uint32 spriteIndex, uint8 r, uint8 g, uint8 b, uint8 a )
{
	//I'll fill this in later
}

void GroupActor::SetVelocity( float x, float y )
{
	actorParams[actorIndex]->SetVelocity( x, y );
}

const b2Vec2 &GroupActor::GetVelocity()
{
	return actorParams[actorIndex]->GetVelocity();
}

void GroupActor::SetPosition( float x, float y )
{
	actorParams[actorIndex]->SetPosition( x, y );
}

b2Vec2 GroupActor::GetPosition()
{
	return actorParams[actorIndex]->GetPosition();
}

int GroupActor::GetBodyIndex( b2Body *body )
{
	int actorIndex = -1;
	for( int i = 0; i < actorCount; ++i )
	{
		if( body == actorParams[i]->body )
		{
			actorIndex = i;
			break;
		}
	}
	assert( actorIndex >= 0 );
	return actorIndex;
}

void GroupActor::SetBodyAngle( float angle )
{
	actorParams[actorIndex]->SetBodyAngle( angle );
}

void GroupActor::SetSpriteOffset( uint32 spriteIndex, float x, float y )
{
	if( isReversed ) y = -y;
	actorParams[actorIndex]->spriteOffset[spriteIndex].Set( x, y );
}

b2Vec2 & GroupActor::GetSpriteOffset( uint32 spriteIndex )
{
	return actorParams[actorIndex]->spriteOffset[spriteIndex];
}

void GroupActor::SetSpriteAngle( uint32 index, float32 p_angle )
{
	assert( index >= 0 && index < spriteCount );
	if( isReversed )
	{
		p_angle = -p_angle;
	}
	spriteAngle[index] = p_angle;
	//transform[index] = sf::Transform::Identity;
	//TileSet *ts = stage->tileSets[tileSetIndex];
	//transform[index].translate( 20, 20 );
	//transform[index].rotate( angle );
}

float32 GroupActor::GetSpriteAngle( uint32 index )
{
//	assert( index >= 0 && index < spriteCount );
//	return transform[index].getrom_spriteAngle[index];
//	//return m_body->GetAngle();
	return 0;
}

void GroupActor::SetSpriteEnabled( uint32 spriteIndex, bool enabled )
{
	actorParams[actorIndex]->spriteIsEnabled[spriteIndex] = enabled;
	if( enabled )
	{
		
		//doesn't matter here, just need to add the sprite back.
	}
	else
	{
		//int i = actorIndex * 4;
		//vertexArray[spriteIndex][i].texCoords = sf::Vector2f( 0, 0 );
		//vertexArray[spriteIndex][i+1].texCoords = sf::Vector2f( 0, 0 );
		//vertexArray[spriteIndex][i+2].texCoords = sf::Vector2f( 0, 0 );
		//vertexArray[spriteIndex][i+3].texCoords = sf::Vector2f( 0, 0 );
	}
	
	//vertexArray[spriteIndex][actorIndex].color = sf::Color::Transparent;
}

uint32 GroupActor::GetUniqueID()
{
	return actorParams[actorIndex]->GetUniqueID();
}

float GroupActor::GetWorldLeft()
{
	return actorParams[actorIndex]->aabb.lowerBound.x;
}



float GroupActor::GetWorldRight()
{
	return actorParams[actorIndex]->aabb.upperBound.x;
}

float GroupActor::GetWorldTop()
{
	return actorParams[actorIndex]->aabb.lowerBound.y;
}

float GroupActor::GetWorldBottom()
{
	return actorParams[actorIndex]->aabb.upperBound.y;
}

void GroupActor::SetFriction( float friction )
{
	actorParams[actorIndex]->SetFriction( friction );
}

float32 GroupActor::GetFriction()
{
	return actorParams[actorIndex]->m_friction;
}

void GroupActor::SetRestitution( float restitution )
{
	actorParams[actorIndex]->SetRestitution( restitution );
}

float32 GroupActor::GetRestitution()
{
	return actorParams[actorIndex]->m_restitution;
}

//this only kills individual actors. the group has another function bool IsDead 
//which calls lua to decide how to do it.
void GroupActor::Kill()
{
	actorParams[actorIndex]->Kill();
}

void GroupActor::Reverse()
{	
	isReversed = !isReversed;
	for( int spriteIndex = 0; spriteIndex < spriteCount; ++spriteIndex )
	{
		transform[spriteIndex].scale( -1, 1 );
	}
}

bool GroupActor::IsReversed()
{
	return isReversed;
}

void GroupActor::SetPause( bool pause )
{
	actorParams[actorIndex]->SetPause( pause );
}

bool GroupActor::IsPaused()
{
	return actorParams[actorIndex]->IsPaused();
}

void GroupActor::ClearActorsAttacked()
{
	actorParams[actorIndex]->ClearActorsAttacked();
}

void GroupActor::SetCurrentActorIndex( uint32 index )
{
	assert( index >= 0 && index < actorCount );
	actorIndex = index;
}

uint32 GroupActor::GetCurrentActorIndex()
{
	return actorIndex;
}

void GroupActor::SetCurrentBody( b2Body *body )
{
	uint32 test = GetBodyIndex( body );
	actorIndex = GetBodyIndex( body );
}

ActorHit * & GroupActor::GetHitsReceived()
{
	return actorParams[actorIndex]->hitsReceived;
}

uint32 & GroupActor::GetHitsReceivedSize()
{
	return actorParams[actorIndex]->hitsReceivedSize;
}

ActorCollision * & GroupActor::GetBodyCollisions()
{
	return actorParams[actorIndex]->bodyCollisions;
}

uint32 & GroupActor::GetBodyCollisionsSize()
{
	return actorParams[actorIndex]->bodyCollisionsSize;
}

ActorHitOffense * & GroupActor::GetActorsAttacked()
{
	return actorParams[actorIndex]->actorsAttacked;
}

uint32 & GroupActor::GetActorsAttackedSize()
{
	return actorParams[actorIndex]->actorsAttackedSize;
}

uint32 & GroupActor::GetActorsAttackedIndex()
{
	return actorParams[actorIndex]->actorsAttackedIndex;
}

void GroupActor::SetSpriteScale( uint32 spriteIndex, float x, float y )
{
	transform[spriteIndex].scale( x, y, 0, 0 );
}

float GroupActor::GetBodyAngle()
{
	return actorParams[actorIndex]->GetBodyAngle();
}

b2Body * GroupActor::GetBody()
{
	return actorParams[actorIndex]->body;
}

uint32 GroupActor::GetActorCount()
{
	return actorCount;
}

void GroupActor::SetOrigin( float x, float y )
{
	origin.x = x * BOX2SF;
	origin.y = y * BOX2SF;
}

void GroupActor::SaveState()
{
}

void GroupActor::LoadState()
{
}

sf::FloatRect GroupActor::GetSpriteAABB()
{
	return transform->transformRect( vertexArray->getBounds() );
	/*sf::FloatRect aabb = vertexArray[0].getBounds();
	for( uint32 i = 1; i < spriteCount; ++i )
	{
		sf::FloatRect r = sprite[i]->getGlobalBounds();
		if( r.left < aabb.left )
		{ 
			float oldLeft = aabb.left;
			aabb.left = r.left;
			aabb.width = ( oldLeft + aabb.width ) - aabb.left;
		}
		if( r.left + r.width > aabb.left + aabb.width )
		{
			aabb.width = ( r.left + r.width ) - aabb.left;
		}

		if( r.top < aabb.top )
		{ 
			float oldTop = aabb.top;
			aabb.top = r.top;
			aabb.height = ( oldTop + aabb.height ) - aabb.top;
		}
		if( r.top + r.height > aabb.top + aabb.height )
		{
			aabb.height = ( r.top + r.height ) - aabb.top;
		}
	}*/
	//return aabb;
}