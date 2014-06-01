#include "Collision.h"
#include <iostream>
#include "Stage.h"
#include "Actor.h"
#include "Globals.h"
#include <list>
#include "Room.h"
#include "Tether.h"

using namespace std;
using namespace luabridge;


void CollisionLayers::SetupFixture(CollisionLayers::Layer layer, uint16 &categoryBits, uint16 &maskBits )
{
	categoryBits = 1 << layer;
	maskBits = 0;

	switch( layer )
	{
		case Event:
			maskBits |= 1 << PlayerEventCollisionbox;
			break;
		case Environment:
			maskBits |= 1 << PlayerPhysicsbox;
			maskBits |= 1 << EnemyPhysicsbox;
			maskBits |= 1 << TetherShot;
			//maskBits |= 1 << Tether;
			break;
		case PlayerHitbox:
			maskBits |= 1 << EnemyHurtbox;
			break;
		case PlayerHurtbox:
			maskBits |= 1 << EnemyHitbox;
			maskBits |= 1 << ActorDetectionbox;
			maskBits |= 1 << PlayerDetectionbox;
			//maskBits |= 1 << ActivateBox;
			break;
		case PlayerPhysicsbox:
			maskBits |= 1 << Environment;
			//maskBits |= 1 << PlayerPhysicsbox;
			maskBits |= 1 << EnemyPhysicsboxWithPlayer;			
		case EnemyHitbox:
			maskBits |= 1 << PlayerHurtbox;
			break;
		case EnemyHurtbox:
			maskBits |= 1 << PlayerHitbox;
			maskBits |= 1 << ActorDetectionbox;
			break;
		case EnemyPhysicsbox:
			maskBits |= 1 << Environment;
			maskBits |= 1 << Door;
			//maskBits |= 1 << PlayerPhysicsbox;
			//maskBits |= 1 << EnemyPhysicsbox;
			break;
		case EnemyPhysicsboxWithPlayer:
			//maskBits |= 1 << Environment;
			maskBits |= 1 << PlayerPhysicsbox;
			//maskBits |= 1 << EnemyPhysicsbox;
			break;
		//case EnemyPhysicsboxWithEnemy:
		//	maskBits |= 1 << EnemyPhysicsboxWithEnemy;
		//	break;
		case ActorDetectionbox:
			maskBits |= 1 << PlayerHurtbox;
			maskBits |= 1 << EnemyHurtbox;
			break;
		case PlayerDetectionbox:
			maskBits |= 1 << PlayerHurtbox;
			break;
		case PlayerEventCollisionbox:
			maskBits |= 1 << Event;
			maskBits |= 1 << Door;
			break;
		//case ActivateBox:
		//	maskBits |= 1 << PlayerHurtbox;
		//	break;
		case Door:
			maskBits |= 1 << PlayerEventCollisionbox;
			maskBits |= 1 << EnemyPhysicsbox;
			break;
		case Tether:
			//maskBits |= 1 << Environment;
			cout << "tether" << endl;
			break;
		case TetherShot:
			maskBits |= 1 << Environment;
			break;
		default:
			cout << "error" << endl;
			assert( 0 );
	}
}

StageCollision::StageCollision( const std::string &t, b2Vec2 &tile, TrueActor *a, b2Vec2 &fraction, b2Vec2 &normal,
	bool en )
		:m_type( t ), m_actor( a ), m_fraction( fraction ), m_normal( normal ), m_tile( tile ), m_enabled( en )
{

}

//^^just use the velocity
void ContactListener::BeginContact(b2Contact* contact)
{ 
	if( contact->GetFixtureA()->GetFilterData().categoryBits == 1 << CollisionLayers::Door 
		|| contact->GetFixtureB()->GetFilterData().categoryBits == 1 << CollisionLayers::Door )
	{
		if( contact->GetFixtureA()->GetFilterData().categoryBits != 1 << CollisionLayers::EnemyPhysicsbox 
		&& contact->GetFixtureB()->GetFilterData().categoryBits != 1 << CollisionLayers::EnemyPhysicsbox )
		{
			doorEnterPos.x = stage->player->GetPosition().x;
			doorEnterPos.y = stage->player->GetPosition().y;
		}
	}


	if ( contact->GetFixtureA()->GetFilterData().categoryBits 
		== 1 << CollisionLayers::Event )
	{
		stage->EventAreaEnter( contact->GetFixtureA() );
	}
	else if ( contact->GetFixtureB()->GetFilterData().categoryBits 
		== 1 << CollisionLayers::Event )
	{
		stage->EventAreaEnter( contact->GetFixtureB() );
	}
}

void ContactListener::EndContact( b2Contact* contact )
{
	uint16 aLayer = contact->GetFixtureA()->GetFilterData().categoryBits;
	uint16 bLayer = contact->GetFixtureB()->GetFilterData().categoryBits;

	if( aLayer == 1 << CollisionLayers::Door || bLayer == 1 << CollisionLayers::Door )
	{
		if( aLayer != 1 << CollisionLayers::EnemyPhysicsbox && bLayer != 1 << CollisionLayers::EnemyPhysicsbox )
		{
			cout << "player pos: " << stage->player->GetPosition().x << ", " << stage->player->GetPosition().y << endl;	

			b2Fixture *a = contact->GetFixtureA();
			b2Fixture *b = contact->GetFixtureB();

			Door * door = NULL;
			b2Fixture *edgeFix = NULL;
			if( aLayer == 1 << CollisionLayers::Environment )
			{
				door = (Door*)b->GetBody()->GetUserData();
				edgeFix = b;
			}
			else
			{
				door = (Door*)a->GetBody()->GetUserData();
				edgeFix = a;
			}

			assert( door != NULL );


			b2Vec2 enterDist;
			b2Vec2 exitDist;

			b2Vec2 currPos = stage->player->GetPosition();

			b2EdgeShape *es = (b2EdgeShape*)(edgeFix->GetShape());
			b2Vec2 v1 = es->m_vertex1;
			b2Vec2 v2 = es->m_vertex2;
	
			cout << "v1: " << v1.x << ", " << v1.y << endl;
			cout << "v2: " << v2.x << ", " << v2.y << endl;
			b2Vec2 line = v2 - v1;
			//no need to add body position because the door body is at (0,0)
			float lsq = line.LengthSquared();

			float t = ((doorEnterPos.x - v1.x) * (v2.x - v1.x) + ( doorEnterPos.y - v1.y ) * ( v2.y - v1.y )) / lsq;

			if( t < 0 )
			{
				enterDist = doorEnterPos - v1;
			}
			else if( t > 1.f )
			{
				enterDist = doorEnterPos - v2;
			}
			else
			{
				const b2Vec2 proj = v1 + t * ( v2 - v1 );
				enterDist = doorEnterPos - proj;
				//cout << "proj: " << proj.x << ", " << proj.y << endl;
			}

			t = ((currPos.x - v1.x) * (v2.x - v1.x) + ( currPos.y - v1.y ) * ( v2.y - v1.y )) / lsq;
			if( t < 0 )
			{
				exitDist = currPos - v1;
			}
			else if( t > 1.f )
			{
				exitDist = currPos - v2;
			}
			else
			{
				const b2Vec2 proj = v1 + t * ( v2 - v1 );
				exitDist = currPos - proj;
			}

			if( enterDist.x > 0 && exitDist.x < 0
				|| enterDist.x < 0 && exitDist.x > 0 
				|| enterDist.y > 0 && exitDist.y < 0
				|| enterDist.y < 0 && exitDist.y > 0 )
			{
				if( stage->currentRoom == door->roomA )
				{
					stage->newRoom = door->roomB;
				}
				else
				{
					stage->newRoom = door->roomA;
				}
			}

			

			
			//doorEnterPos -= 

			/*float enterX = 
			b2Vec2 enter( doorEnterPos.x * cos( door->angle ) + 
				doorEnterPos.y * sin( door->angle ), doorEnterPos.x * sin( door->angle ) + 
				doorEnterPos.y * cos( door->angle ) );

			b2Vec2 currPos = stage->player->GetPosition();
			b2Vec2 exit( currPos.x * cos( door->angle ) + 
				currPos.y * sin( door->angle ), currPos.x * sin( door->angle ) + 
				currPos.y * cos( door->angle ) );

			

			if( enter.x > 0 && exit.x < 0
				|| enter.x < 0 && exit.x > 0 
				|| enter.y > 0 && exit.y < 0
				|| enter.y < 0 && exit.y > 0 )
			{
				if( stage->currentRoom == door->roomA )
				{
					stage->newRoom = door->roomB;
				}
				else
				{
					stage->newRoom = door->roomA;
				}
			}*/
		}
			
			//if( stage->currentRoom == roomA )
			//{
			//	stage->newRoom = roomB;
			//	stage->currentDoor = door;//need to update it to take the actual name of the door

				//roomA->Exit();
				//stage->currentRoom = roomB;
				//roomB->Enter( door->roomA );
			//}
			//else 
			//{
				
			//	stage->newRoom = roomA;
			//	stage->currentDoor = door;
				//roomB->Exit();
				//stage->currentRoom = roomA;
				//roomA->Enter( door->roomB );
			//}
		//}
	}
	if ( contact->GetFixtureA()->GetFilterData().categoryBits == 1 << CollisionLayers::Event )
	{
		stage->EventAreaLeave( contact->GetFixtureA() );
	}
	else if ( contact->GetFixtureB()->GetFilterData().categoryBits == 1 << CollisionLayers::Event )
	{
		stage->EventAreaLeave( contact->GetFixtureB() );
	}
}
	
void ContactListener::PreSolve( b2Contact* contact, const b2Manifold* oldManifold )
{ 
	b2Fixture *a = contact->GetFixtureA();
	b2Fixture *b = contact->GetFixtureB();
	uint16 aLayer = a->GetFilterData().categoryBits;
	uint16 bLayer = b->GetFilterData().categoryBits;
	
	if( aLayer == ( 1 << CollisionLayers::TetherShot ) || bLayer == ( 1 << CollisionLayers::TetherShot ) )
	{
		int numPoints = contact->GetManifold()->pointCount;

		b2WorldManifold worldManifold;
		contact->GetWorldManifold( &worldManifold );
		
		PlayerChar *player = stage->player;

		b2Vec2 furthestPoint( worldManifold.points[0].x, worldManifold.points[0].y );
		if( numPoints > 1 )
		{
			float x = furthestPoint.x - player->GetPosition().x;
			float y = furthestPoint.y - player->GetPosition().y;
			float distSqr = x * x + y * y;

			float x2 = worldManifold.points[1].x - player->GetPosition().x;
			float y2 = worldManifold.points[1].y - player->GetPosition().y;
			float distSqr2 = x2 * x2 + y2 * y2;

			if( distSqr2 > distSqr )
			{
				furthestPoint = worldManifold.points[1];
			}
		}

		b2Body * body = NULL;
		if( aLayer == ( 1 << CollisionLayers::TetherShot ) )
		{
			body = contact->GetFixtureA()->GetBody();
		}
		else
		{
			body = contact->GetFixtureB()->GetBody();
		}

		assert( body != NULL );

		Tether *t = NULL;
		if( body == player->leftTether->shotBody )
		{
			t = player->leftTether;
		}
		else
		{
			t = player->rightTether;
		}


		t->shotHit = true;
		t->shotHitPoint = furthestPoint;
	}
	else if( aLayer == ( 1 << CollisionLayers::Tether ) || bLayer == ( 1 << CollisionLayers::Tether ) )
	{
		contact->SetEnabled( true );
		b2Vec2 tilePos;
		if( aLayer == ( 1 << CollisionLayers::Tether ) )
		{
			//b->GetShape()->
			b2Vec2 *tempVec= (b2Vec2*)(b->GetUserData());
			tilePos.x = tempVec->x;
			tilePos.y = tempVec->y;

			b2ChainShape *cs = (b2ChainShape*)b->GetShape();
			cout << "count: " << cs->m_count << endl;
			cout << "first: " << cs->m_vertices[0].x << ", " << cs->m_vertices[0].y << endl;
			cout << "second: " << cs->m_vertices[1].x << ", " << cs->m_vertices[1].y << endl;
			cout << "prev vert: " << cs->m_prevVertex.x << ", " << cs->m_prevVertex.y << endl;
			cout << "next vert: " << cs->m_nextVertex.x << ", " << cs->m_nextVertex.y << endl;
			
			//stage->tetherCollisions.push_back( list<b2Vec2>() );
			for( int i = 0; i < cs->m_count; ++i )
			{
				//stage->tetherCollisions.back().push_back( cs->m_vertices[i] );
				stage->tetherCollisions.push_back( cs->m_vertices[i] );
			}
		}
		else
		{
			b2Vec2 *tempVec= (b2Vec2*)(a->GetUserData());
			tilePos.x = tempVec->x;
			tilePos.y = tempVec->y;

			b2ChainShape *cs = (b2ChainShape*)a->GetShape();
			cout << "count: " << cs->m_count << endl;
			cout << "first: " << cs->m_vertices[0].x << ", " << cs->m_vertices[0].y << endl;
			cout << "second: " << cs->m_vertices[1].x << ", " << cs->m_vertices[1].y << endl;
			cout << "prev vert: " << cs->m_prevVertex.x << ", " << cs->m_prevVertex.y << endl;
			cout << "next vert: " << cs->m_nextVertex.x << ", " << cs->m_nextVertex.y << endl;
			//blayer is tether

			//stage->tetherCollisions.push_back( list<b2Vec2>() );
			for( int i = 0; i < cs->m_count; ++i )
			{
				stage->tetherCollisions.push_back( cs->m_vertices[i] );
			}

		}

		//cout << "tile pos: " << tilePos.x << ", " << tilePos.y << endl;
		//stage->staticTileSets[tilePos.x][tilePos.y]
	}
	else if( aLayer == ( 1 << CollisionLayers::Environment ) || bLayer == ( 1 << CollisionLayers::Environment ) )
	{
		bool contactEnabled =  stage->HandleCollision( contact );
		contact->SetEnabled( contactEnabled );
		TrueActor *actor;
		b2Body *actorBody = NULL;
		if( aLayer == 1 << CollisionLayers::Environment )
		{
			actor = (TrueActor*)b->GetBody()->GetUserData();
			actorBody = b->GetBody();
		}
		else
		{
			actor = (TrueActor*)a->GetBody()->GetUserData();
			actorBody = a->GetBody();
			//bLayer is environment
		}
		

		b2WorldManifold worldManifold;
		contact->GetWorldManifold( &worldManifold );
		if( actor->IsReversed() ) worldManifold.normal.y = - worldManifold.normal.y;
		//cout << "pointCount: " << contact->GetManifold()->pointCount << endl;
		
		if( actor->isGroup )
		{
			GroupActor *ga = (GroupActor*)actor;
			ga->SetCurrentBody( actorBody );
		}
		//cout << "point1: " << worldManifold.points[0].x << ", " << worldManifold.points[0].y << endl;
		//cout << "point2: " << worldManifold.points[1].x << ", " << worldManifold.points[1].y << endl;
		bool enable = actor->HandleStageCollision( contact->GetManifold()->pointCount, 
			worldManifold, contactEnabled );
		contact->SetEnabled( enable );
		/*if( !lua_isnumber( l, -1 ) )
		{
			assert( 0 && "needs to be a float" );
		}
		float tangentSpeed = lua_tonumber( l, -1 );
		lua_pop( l, 1 );*/
		
		//cout << "tan: " << tangentSpeed << endl;
		//if( aLayer == CollisionLayers::Environment )
		//	contact->SetTangentSpeed( tangentSpeed );
		//else
		//	contact->SetTangentSpeed( -tangentSpeed );

		contact->SetFriction( actor->GetFriction() );
		contact->SetRestitution( actor->GetRestitution() );
	}
	else if( aLayer == ( 1 << CollisionLayers::Door ) || bLayer == ( 1 << CollisionLayers::Door ) )
	{
		//contact->SetEnabled( true );

		if( aLayer != ( 1 << CollisionLayers::EnemyPhysicsbox ) && bLayer != ( 1 << CollisionLayers::EnemyPhysicsbox ) )
		{
			contact->SetEnabled( false );
		}

		//if( aLayer != ( 1 << CollisionLayers::EnemyPhysicsbox ) && bLayer != ( 1 << CollisionLayers::EnemyPhysicsbox ) )
		//{
		//	Room *roomA = NULL;
		//	Room *roomB = NULL;

		//	Door * door = NULL;
		//	if( aLayer == 1 << CollisionLayers::Environment )
		//	{
		//		door = (Door*)b->GetBody()->GetUserData();
		//	}
		//	else
		//	{
		//		door = (Door*)a->GetBody()->GetUserData();
		//	}

		//	assert( door != NULL );

		//	b2WorldManifold worldManifold;
		//	contact->GetWorldManifold( &worldManifold );

		//	
		//	door->collide = true;
		//	if( door->open )
		//	{
		//		//cout << "normal: " << worldManifold.normal.x << ", " << worldManifold.normal.y << endl;
		//		door->open = false;
		//		for( list<Room*>::iterator rIt = stage->rooms.begin(); rIt != stage->rooms.end(); ++rIt )
		//		{
		//			if( (*rIt)->name == door->roomA )
		//				roomA = (*rIt);
		//			else if( (*rIt)->name == door->roomB )
		//				roomB = (*rIt);
		//		}

		//		if( stage->currentRoom == roomA )
		//		{
		//			stage->newRoom = roomB;
		//			stage->currentDoor = door;//need to update it to take the actual name of the door

		//			//roomA->Exit();
		//			//stage->currentRoom = roomB;
		//			//roomB->Enter( door->roomA );
		//		}
		//		else 
		//		{
		//		
		//			stage->newRoom = roomA;
		//			stage->currentDoor = door;
		//			//roomB->Exit();
		//			//stage->currentRoom = roomA;
		//			//roomA->Enter( door->roomB );
		//		}
		//	//assert( roomA != NULL && roomB != NULL );
		//	}

		//	//cout << "room transition!: ." << door->roomA << ", " << door->roomB << ". roomssize: " << stage->rooms.size() << endl;
		//	contact->SetEnabled( false );
		//}
	}
	else
	{
		//not a stage collision
		
		TrueActor *actorA = (TrueActor*)a->GetBody()->GetUserData();
		TrueActor *actorB = (TrueActor*)b->GetBody()->GetUserData();
		
		if( actorA->isGroup )
		{
			GroupActor *ga = (GroupActor*)actorA;
			ga->SetCurrentBody( a->GetBody() );
		}

		if( actorB->isGroup )
		{
			GroupActor *ga = (GroupActor*)actorB;
			ga->SetCurrentBody( b->GetBody() );
		}

		if( actorA->IsPaused() || actorB->IsPaused() )
		{
			contact->SetEnabled( false );
			return;
		}

		

		if( aLayer == 1 << CollisionLayers::PlayerHitbox || aLayer == 1 << CollisionLayers::EnemyHitbox
			|| bLayer == 1 << CollisionLayers::PlayerHitbox || bLayer == 1 << CollisionLayers::EnemyHitbox )
		{
			contact->SetEnabled( false );
			TrueActor *attacker = NULL, *defender = NULL;
			if( aLayer == 1 << CollisionLayers::PlayerHitbox || aLayer == 1 << CollisionLayers::EnemyHitbox )
			{
				attacker = actorA;
				defender = actorB;
			}
			else
			{
				attacker = actorB;
				defender = actorA;
			}

			bool valid = defender->GetHitsReceivedSize() < defender->hitsReceivedCap;
			

			//start at the most recent hit
			for( int i = defender->GetHitsReceivedSize() - 1; i >= 0 && valid; --i )
			{
				if( defender->GetHitsReceived()[i].actor == attacker )
				{
					//already been hit by this actor this frame
					valid = false;
				}
			}



			uint32 hitboxTag;
			if( attacker == actorA )
				hitboxTag = (uint32)a->GetUserData();
			else
				hitboxTag = (uint32)b->GetUserData();

			//cout << "size: " << attacker->GetActorsAttackedSize() << endl;
			for( int i = attacker->GetActorsAttackedSize() - 1; i >= 0 && valid; --i )
			{
				if( attacker->GetActorsAttacked()[i].actor == defender 
					&& attacker->GetActorsAttacked()[i].hitboxTag == hitboxTag )
				{
					//this hitbox already hit the actor
					valid = false;
				}	
			}

			if( valid )
			{
				if( attacker->GetActorsAttackedIndex() == attacker->actorsAttackedCap ) 
				{
					//loop back around
					attacker->GetActorsAttackedIndex() = 0;
					attacker->GetActorsAttacked()[0].actor = defender;
					attacker->GetActorsAttacked()[0].hitboxTag = hitboxTag;
				}
				else 
				{
					//increment size if its not full
					if( attacker->GetActorsAttackedSize() < attacker->actorsAttackedCap )
						attacker->GetActorsAttackedSize()++;
				//	cout << "size2: " << attacker->GetActorsAttackedSize() << endl;
					//
					attacker->GetActorsAttacked()[attacker->GetActorsAttackedIndex()].actor = defender;
					attacker->GetActorsAttacked()[attacker->GetActorsAttackedIndex()].hitboxTag = hitboxTag;
					attacker->GetActorsAttackedIndex()++;	
				}
			}
			

			
			if( valid )
			{

				uint32 hitboxTag = 0;
				b2Body *actorBody = NULL;
				if( attacker == actorA )
				{
					hitboxTag = (uint32)a->GetUserData();
					actorBody = a->GetBody();
				}	
				else
				{
					hitboxTag = (uint32)b->GetUserData();
					actorBody = b->GetBody();
				}

				

				float32 hitboxCenterX = 0;
				uint32 hitstun = 0;
				uint32 hitlag = 0;
				int dmg = 0;
				std::string hitboxType;

				attacker->HitActor( defender, hitboxTag, hitboxCenterX, hitstun, hitlag, dmg, 
					hitboxType );


				ActorHit &h = defender->GetHitsReceived()[defender->GetHitsReceivedSize()++];
				h.actor = attacker;
				h.damage = dmg;
				h.hitboxName = hitboxType;
				h.hitlag = hitlag;
				h.centerX = hitboxCenterX;
				h.otherBody = attacker->GetBody();
				h.ownBody = defender->GetBody();

				if( attacker == actorA )
					h.hurtboxTag = (uint32)b->GetUserData();
				else
					h.hurtboxTag = (uint32)a->GetUserData();
			}
		}
		else 
		{
			//contact->SetEnabled( true );
			bool valid = true;
			if( actorA->GetBodyCollisionsSize() == actorA->bodyCollisionsCap 
				|| actorB->GetBodyCollisionsSize() == actorB->bodyCollisionsCap )
			{
				valid = false;
			}
			
			//can use either actor here, because if one collided with another they will both show evidence of the collision
			for( int i = 0; i < actorA->GetBodyCollisionsSize() && valid; ++i )
			{
				if( actorA->GetBodyCollisions()[i].actor == actorB )
					valid = false;
			}
			//for( list<ActorCollision*>::iterator it = actorA->GetBodyCollisions().begin(); it != actorA->GetBodyCollisions().end() 
		//		&& !found; ++it )
		//	{
		//		if( (*it)->actor == actorB )
		//			found = true;
		//	}	
			
			if( ( aLayer == 1 << CollisionLayers::PlayerPhysicsbox 
				|| bLayer == 1 << CollisionLayers::PlayerPhysicsbox )
				&& ( aLayer == 1 << CollisionLayers::EnemyPhysicsboxWithPlayer 
				|| bLayer == 1 << CollisionLayers::EnemyPhysicsboxWithPlayer ) )
			{
			//	contact->SetEnabled( true );
			}
			else
			{
				contact->SetEnabled( false );
			}

			if( valid )
			{
				//contact->SetFriction( 0 );
				//cout << "layerA " << aLayer << ", layerB " << bLayer << endl;
				
				uint32 tagA = (uint32)a->GetUserData();
				bool activeA = false;

				bool enableA = actorA->CollideWithActor( actorB, tagA, activeA );
				

				uint32 tagB = (uint32)b->GetUserData();
				bool activeB = false;


				bool enableB = actorB->CollideWithActor( actorA, tagB, activeB );

				b2WorldManifold worldManifold;
				contact->GetWorldManifold( &worldManifold );

				//if( active )
				//{
				if( ( aLayer == 1 << CollisionLayers::PlayerPhysicsbox 
				|| bLayer == 1 << CollisionLayers::PlayerPhysicsbox )
				&& ( aLayer == 1 << CollisionLayers::EnemyPhysicsboxWithPlayer 
				|| bLayer == 1 << CollisionLayers::EnemyPhysicsboxWithPlayer ) )
				{
					if( activeA )
					{
					ActorCollision &c = actorB->GetBodyCollisions()[actorB->GetBodyCollisionsSize()++];
					c.actor = actorA;
					c.hurtboxTag = (uint32)b->GetUserData();
					c.normal = worldManifold.normal;
					if( actorB->IsReversed() )
					{
						c.normal.y = -c.normal.y;
					}
					c.pointCount = contact->GetManifold()->pointCount;
					c.point1 = worldManifold.points[0];
					c.point2 = worldManifold.points[1];
					c.enabled = enableA && enableB;
					}
				}


				if( ( aLayer == 1 << CollisionLayers::PlayerPhysicsbox 
				|| bLayer == 1 << CollisionLayers::PlayerPhysicsbox )
				&& ( aLayer == 1 << CollisionLayers::EnemyPhysicsboxWithPlayer 
				|| bLayer == 1 << CollisionLayers::EnemyPhysicsboxWithPlayer ) )
				{
					if( activeB )
					{
					ActorCollision &c2 = actorA->GetBodyCollisions()[actorA->GetBodyCollisionsSize()++];
					c2.actor = actorB;
					c2.hurtboxTag = (uint32)a->GetUserData();
					//c2.normal = worldManifold.normal;
					c2.normal.x = -worldManifold.normal.x;
					c2.normal.y = -worldManifold.normal.y;
					if( actorA->IsReversed() )
					{
						c2.normal.y = -c2.normal.y;
					}
					c2.pointCount = contact->GetManifold()->pointCount;
					c2.point1 = worldManifold.points[0];
					c2.point2 = worldManifold.points[1];
					c2.enabled = enableA && enableB;
					}
				}

				if( enableA && enableB ) 
				{
					//contact->SetEnabled( true );
					//cout << "not enabled" << endl;	
					//contact->SetEnabled( false);
				}
				else
					contact->SetEnabled( false );
					//+figure out a way to make sure there isn't any dynamic allocatio here?/delete the memory
				//	ActorCollision *ac = new ActorCollision;
				//	ac->actor = actorB;
			//		ac->hurtboxTag = (uint32)a->GetUserData();
					//actorA->GetBodyCollisions().push_back( ac );
				//}	

				
			}
			else
				contact->SetEnabled( false );

		
			//if( aLayer == 1 << CollisionLayers::EnemyPhysicalHurtbox
			//	|| bLayer == 1 << CollisionLayers::EnemyPhysicalHurtbox )
			//	contact->SetEnabled( true );
			//else
			//	contact->SetEnabled( false );
		}	
	}
}
	
void ContactListener::PostSolve( b2Contact* contact, const b2ContactImpulse* impulse )
{ 
	//b2WorldManifold worldManifold;
	//contact->GetWorldManifold( &worldManifold );
	//cout << "post: " << worldManifold.points[0].x << ", " << worldManifold.points[0].y << endl;
}

