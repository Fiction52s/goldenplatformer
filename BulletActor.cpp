#include "Actor.h"
#include <iostream>

using namespace luabridge;
using namespace std;

BulletActor::BulletActor( const std::string &actorType, uint32 p_actorCount, const b2Vec2 &pos, 
	const b2Vec2 &vel, TrueActor *parent, Stage *st )
	:GroupActor( actorType, p_actorCount, pos, vel, true, false, 0, parent, st ), trailSize( 10 )
{
	//vertexArray = new sf::VertexArray[spriteCount];
	//spriteAngle = new float[spriteCount];
	//spriteScale = new sf::Vector2f[spriteCount];

	//for( int i = 0; i < spriteCount; ++i )
	//{
	//	vertexArray[i].resize( actorCount * 4 );
	//	vertexArray[i].setPrimitiveType( sf::Quads );
	//	angle = 0;
	//	spriteScale[i] = sf::Vector2f( 1, 1 );
	//}
	trailArray.resize( p_actorCount * 4 * trailSize );
	trailArray.setPrimitiveType( sf::PrimitiveType::Quads );
	posHistory = new b2Vec2*[p_actorCount];
	velHistory = new b2Vec2*[p_actorCount];
	trailOn = new bool[p_actorCount];
	for( int i = 0; i < p_actorCount; ++i )
	{
		posHistory[i] = new b2Vec2[trailSize];
		velHistory[i] = new b2Vec2[trailSize];
		trailOn[i] = false;
	}
}

void BulletActor::Init( b2World *p_world )
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
	}
	
	lua_getglobal( L, "Init" );
	lua_pcall( L, 0, 0, 0 );
	

	for( int i = 0; i < actorCount; ++i )
	{
		actorParams[i]->UpdateBoundingBoxes();
	}

	for( int i = 0; i < actorCount; ++i )
	{
		SetCurrentActorIndex( i );
		b2Vec2 pos = GetPosition();
		b2Vec2 vel = GetVelocity();
		posHistory[i][0].Set( pos.x, pos.y );
		velHistory[i][0].Set( vel.x, vel.y );
	}
}

BulletActor::~BulletActor()
{
	for( int i = 0; i < actorCount; ++i )
	{
		delete [] posHistory[i];
		delete [] velHistory[i];
	}
	delete [] posHistory;
	delete [] velHistory;
	delete [] trailOn;
}

void BulletActor::Draw( sf::RenderTarget *target )
{	
	target->draw( trailArray );
	for( int i = 0; i < spriteCount; ++i )
	{
		sf::RenderStates states( stage->tileSets[tileSetIndex]->texture );
		//transform[i] = sf::Transform::Identity;
	//	if( parent != NULL )

			//transform[i].rotate( 5, parent->GetPosition().x * BOX2SF, parent->GetPosition().y * BOX2SF );
		//transform[i].rotate( 1 );
		states.transform = transform[i];
		target->draw( vertexArray[i], states );
	}
	//UpdateTrails();
	
}

void BulletActor::UpdatePostPhysics()
{
	//UpdateTrails();

	TrueActor::UpdatePostPhysics();
}

void BulletActor::UpdateTrail()
{
	uint32 i = GetCurrentActorIndex();
	if( IsPaused() )
	{
			
	}
	else
	{
		b2Vec2 cPos = GetPosition();
		b2Vec2 vel = GetVelocity();

		if( trailOn[i] )
		{
			sf::Color col = sf::Color( 141, 197, 23 );
			//float slope = (pos.y - posHistory[i][trailSize-1].y)/(pos.x - posHistory[i][trailSize-1].x );


			for( int trailIt = trailSize-1; trailIt > 0; --trailIt )
			{
				b2Vec2 &prevPos = posHistory[i][trailIt-1];
				b2Vec2 &prevVel = velHistory[i][trailIt-1];
				posHistory[i][trailIt].Set( prevPos.x, prevPos.y );
				velHistory[i][trailIt].Set( prevVel.x, prevVel.y );
			}
			posHistory[i][0].Set( cPos.x, cPos.y );
			velHistory[i][0].Set( vel.x, vel.y );
				

			float originalWidth = 40;
			float currentWidth = originalWidth;
			float width = currentWidth * SF2BOX;

			for( int seg = 0; seg < trailSize - 1; ++seg )
			{
					

				b2Vec2 &oldPos = posHistory[i][seg + 1];
				b2Vec2 &pos = posHistory[i][seg];

				b2Vec2 diff( pos.x - oldPos.x, pos.y - oldPos.y );
				diff.Normalize();

				float temp = diff.x;
				diff.x = diff.y;
				diff.y = temp;

				trailArray[(i * trailSize + seg) * 4 + 0].color = col;
				trailArray[(i * trailSize + seg) * 4 + 0].position = sf::Vector2f( (pos.x + width * diff.x ) * BOX2SF, ( pos.y - width * diff.y ) * BOX2SF );

				trailArray[(i * trailSize + seg) * 4 + 1].color = col;
				trailArray[(i * trailSize + seg) * 4 + 1].position = sf::Vector2f( (pos.x - width * diff.x ) * BOX2SF, ( pos.y + width * diff.y )  * BOX2SF);

					
				if( seg < trailSize - 2 )
				{
					b2Vec2 &olderPos = posHistory[i][seg+2];
					diff = b2Vec2( oldPos.y - olderPos.y, oldPos.x - olderPos.x );
					diff.Normalize();
				}

				currentWidth = ( 1 - ( seg + 1 ) / (float)(trailSize) )  * originalWidth;
				width = currentWidth * SF2BOX;

				col.a = col.a - (255.f/trailSize);

				trailArray[(i * trailSize + seg) * 4 + 2].color = col;
				trailArray[(i * trailSize + seg) * 4 + 2].position 
					= sf::Vector2f( ( oldPos.x - width * diff.x ) * BOX2SF, ( oldPos.y + width * diff.y ) * BOX2SF );

				trailArray[(i * trailSize + seg) * 4 + 3].color = col;
				trailArray[(i * trailSize + seg) * 4 + 3].position 
					= sf::Vector2f( (oldPos.x + width * diff.x ) * BOX2SF, ( oldPos.y - width * diff.y ) * BOX2SF);	
			}
		}
	}
}

/*void BulletActor::UpdateTrails()
{
	uint32 aCount = GetActorCount();
	for( int i = 0; i < aCount; ++i )
	{
		SetCurrentActorIndex( i );
		if( IsPaused() )
		{
			
		}
		else
		{
			b2Vec2 cPos = GetPosition();
			b2Vec2 vel = GetVelocity();

			if( trailOn[i] )
			{
				sf::Color col = sf::Color( 141, 197, 23 );
				//float slope = (pos.y - posHistory[i][trailSize-1].y)/(pos.x - posHistory[i][trailSize-1].x );


				for( int trailIt = trailSize-1; trailIt > 0; --trailIt )
				{
					b2Vec2 &prevPos = posHistory[i][trailIt-1];
					b2Vec2 &prevVel = velHistory[i][trailIt-1];
					posHistory[i][trailIt].Set( prevPos.x, prevPos.y );
					velHistory[i][trailIt].Set( prevVel.x, prevVel.y );
				}
				posHistory[i][0].Set( cPos.x, cPos.y );
				velHistory[i][0].Set( vel.x, vel.y );
				

				float originalWidth = 40;
				float currentWidth = originalWidth;
				float width = currentWidth * SF2BOX;

				for( int seg = 0; seg < trailSize - 1; ++seg )
				{
					

					b2Vec2 &oldPos = posHistory[i][seg + 1];
					b2Vec2 &pos = posHistory[i][seg];

					b2Vec2 diff( pos.x - oldPos.x, pos.y - oldPos.y );
					diff.Normalize();

					float temp = diff.x;
					diff.x = diff.y;
					diff.y = temp;

					trailArray[(i * trailSize + seg) * 4 + 0].color = col;
					trailArray[(i * trailSize + seg) * 4 + 0].position = sf::Vector2f( (pos.x + width * diff.x ) * BOX2SF, ( pos.y - width * diff.y ) * BOX2SF );

					trailArray[(i * trailSize + seg) * 4 + 1].color = col;
					trailArray[(i * trailSize + seg) * 4 + 1].position = sf::Vector2f( (pos.x - width * diff.x ) * BOX2SF, ( pos.y + width * diff.y )  * BOX2SF);

					
					if( seg < trailSize - 2 )
					{
						b2Vec2 &olderPos = posHistory[i][seg+2];
						diff = b2Vec2( oldPos.y - olderPos.y, oldPos.x - olderPos.x );
						diff.Normalize();
					}

					currentWidth = ( 1 - ( seg + 1 ) / (float)(trailSize) )  * originalWidth;
					width = currentWidth * SF2BOX;

					col.a = col.a - (255.f/trailSize);

					trailArray[(i * trailSize + seg) * 4 + 2].color = col;
					trailArray[(i * trailSize + seg) * 4 + 2].position 
						= sf::Vector2f( ( oldPos.x - width * diff.x ) * BOX2SF, ( oldPos.y + width * diff.y ) * BOX2SF );

					trailArray[(i * trailSize + seg) * 4 + 3].color = col;
					trailArray[(i * trailSize + seg) * 4 + 3].position 
						= sf::Vector2f( (oldPos.x + width * diff.x ) * BOX2SF, ( oldPos.y - width * diff.y ) * BOX2SF);	
				}
				


				
				

		//		b2Vec2 diff( pos.x - posHistory[i][0].x, pos.y - posHistory[i][0].y );

		//		trailArray[i * 3 + 0].color = col;
		//		trailArray[i * 3 + 0].position = sf::Vector2f( pos.x * BOX2SF, 
		//			pos.y * BOX2SF);
		//

		//		trailArray[i * 3 + 1].color = col;
		//		trailArray[i * 3 + 1].position = sf::Vector2f( posHistory[i][0].x * BOX2SF - 64,
		//			posHistory[i][0].y * BOX2SF );
		//

		//		trailArray[i * 3 + 2].color = col;
		//		trailArray[i * 3 + 2].position = sf::Vector2f( pos.x * BOX2SF, 
		//			pos.y * BOX2SF + 32 );

		//		trailArray[i * 3 + 2].color = col;
		//		trailArray[i * 3 + 2].position = sf::Vector2f( pos.x * BOX2SF, 
		//			pos.y * BOX2SF + 32 );
		//
				
				
			}
			else
			{
				//trailArray[i * 3+ 0].position = sf::Vector2f( pos.x * BOX2SF, pos.y * BOX2SF);
					
				//trailArray[i * 3 + 1].position = sf::Vector2f( pos.x * BOX2SF, 
				//	pos.y * BOX2SF );
				//trailArray[i * 3 + 2].position = sf::Vector2f( pos.x * BOX2SF, 
				//	pos.y * BOX2SF );

				
			}
		}
	}
}*/

void BulletActor::SetTrailOn( bool on )
{
	uint32 aIndex = GetCurrentActorIndex();

	trailOn[aIndex] = on;

	b2Vec2 pos = GetPosition();
	b2Vec2 vel = GetVelocity();

	
	for( int i = 0; i < trailSize; ++i )
	{
		posHistory[aIndex][i].Set( pos.x, pos.y );
		velHistory[aIndex][i].Set( vel.x, vel.y );
	}
}

void BulletActor::ClearTrail()
{
	int i = GetCurrentActorIndex();

	trailOn[i] = false;

	b2Vec2 pos = GetPosition();
	b2Vec2 vel = GetVelocity();


	
	for( int trailIt = 0; trailIt < trailSize; ++trailIt )
	{
		trailArray[(i * trailSize + trailIt) * 4 + 0].position = sf::Vector2f( pos.x * BOX2SF, 
				pos.y * BOX2SF);
		trailArray[(i * trailSize + trailIt) * 4 + 1].position = sf::Vector2f( pos.x * BOX2SF, 
					pos.y * BOX2SF );
		trailArray[(i * trailSize + trailIt) * 4 + 2].position = sf::Vector2f( pos.x * BOX2SF, 
					pos.y * BOX2SF );
		trailArray[(i * trailSize + trailIt) * 4 + 3].position = sf::Vector2f( pos.x * BOX2SF, 
					pos.y * BOX2SF );
	}
	

	
	

	//uint32 aIndex = GetCurrentActorIndex();
	//posHistory[aIndex][0].Set( pos.x, pos.y );
	//velHistory[aIndex][0].Set( vel.x, vel.y );
}