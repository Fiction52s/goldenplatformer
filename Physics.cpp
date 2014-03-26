#include "Physics.h"
#include <assert.h>
#include <iostream>
#include "Actor.h"

using namespace sf;


AABB::AABB( float x, float y, float w, float h, Object *p_owner )
	:pos( x, y ), size( w, h ),owner(p_owner)
{
	directionOn = 0xF;
	active = true;
}

AABB::AABB( const Vector2f &pos, const Vector2f &size, Object *p_owner )
	:pos( pos ), size( size ), owner( p_owner )
{
	directionOn = 0xF;
	active = true;
}

//current system only has one collision per frame
Collision * AABB::Collide( AABB *other )
{
	bool totRight =  WorldLeft() >= other->WorldRight();
	bool totLeft = WorldRight() <= other->WorldLeft();
	bool totBelow = WorldTop() >= other->WorldBottom();
	bool totAbove = WorldBottom() <= other->WorldTop();

	if( totRight || totLeft || totBelow || totAbove )
		return NULL;

	float toleranceX = 20;//other->size.x / 2;
	float toleranceY = 20;//other->size.y / 2;
	float overlap = 0;
	//caller is the one with variable position
	Vector2f normal;
	Collision *col = NULL;

	float topOverlap = WorldBottom() - other->WorldTop();
	float leftOverlap = WorldRight() - other->WorldLeft();
	float rightOverlap = other->WorldRight() - WorldLeft();
	float bottomOverlap = other->WorldBottom() - WorldTop();

	bool topCollide = other->TopActive() && topOverlap > 0 && topOverlap < toleranceY;
	bool leftCollide = other->LeftActive() && leftOverlap > 0 && leftOverlap < toleranceX;
	bool rightCollide = other->RightActive() && rightOverlap > 0 && rightOverlap < toleranceX;
	bool bottomCollide = other->BottomActive() && bottomOverlap > 0 && bottomOverlap < toleranceY;

	int collisionCount = topCollide + leftCollide + rightCollide + bottomCollide;

	//if( collisionCount > 1 )
	//std::cout << "collisionCount: " << collisionCount << std::endl;

	DynamicObject *dynOwner = (DynamicObject*)owner;
	if( dynOwner->vel.x >= 0 && dynOwner->vel.x > abs(dynOwner->vel.y) && leftCollide )
	{
		//left collision stuff
		std::cout << "left" << std::endl;
		col = new Collision();
		col->normal = Vector2f( -1, 0 );
		col->objectA = owner;
		col->objectB = other->owner;
	//	col->overlap = leftOverlap + pos.x;
	}
	else if( dynOwner->vel.x <= 0 && abs(dynOwner->vel.x) > abs( dynOwner->vel.y ) && rightCollide )
	{
		//right stuff
		std::cout << "right" << std::endl;
		col = new Collision();
		col->normal = Vector2f( 1, 0 );
		col->objectA = owner;
		col->objectB = other->owner;
	//	col->overlap = rightOverlap - pos.x;
	}
	//vel.y has greater absolute value
	else if( dynOwner->vel.y >= 0 && topCollide )
	{
		//top
		std::cout << "top" << std::endl;
		col = new Collision();
		col->normal = Vector2f( 0, -1 );
		col->objectA = owner;
		col->objectB = other->owner;
		//col->overlap = topOverlap + pos.y;
	}
	else if( dynOwner->vel.y <= 0 && bottomCollide )
	{
		assert( bottomCollide );

		std::cout << "bottom" << std::endl;
		col = new Collision();
		col->normal = Vector2f( 0, 1 );
		col->objectA = owner;
		col->objectB = other->owner;
		//col->overlap = bottomOverlap + pos.y;
		//bottom
	}

	//else if( dynOwner->vel.y > 0 && dynOwner->vel.y 
	//if( topCollide && ( topOverlap < bottomOverlap && bottomCollide && topOverlap < leftOverlap * leftCollide 
	//	&& topOverlap < rightOverlap * rightCollide ) )
	//{
	//
	//}
	//else if( leftCollide )
	//{
	//	
	//}
	//else if( rightCollide )
	//{
	//	
	//}
	//else if( bottomCollide )
	//{
	//	
	//}
	//else
	//{
	//	std::cout << "Wat" << std::endl;
	//}
	//std::cout << "this: " << pos.x << ", " << pos.y << std::endl;
	//std::cout << "other: " << other->pos.x << ", " << other->pos.y << std::endl;
	//std::cout << "leftOverlap: " << leftOverlap << std::endl;

	return col;
}

Vector2f AABB::GetWorldPos()
{
	return Vector2f( owner->pos.x + pos.x, owner->pos.y + pos.y );
}

float AABB::WorldRight()
{
	return owner->pos.x + pos.x + size.x / 2;
}

float AABB::WorldTop()
{
	return owner->pos.y + pos.y - size.y / 2;
}

float AABB::WorldBottom()
{
	return owner->pos.y + pos.y + size.y / 2;
}

float AABB::WorldLeft()
{
	return owner->pos.x + pos.x - size.x / 2;
}

float AABB::LocalRight()
{
	return pos.x + size.x / 2;
}

float AABB::LocalTop()
{
	return pos.y - size.y / 2;
}

float AABB::LocalBottom()
{
	return pos.y + size.y / 2;
}

float AABB::LocalLeft()
{
	return pos.x - size.x / 2;
}

sf::Vector2f AABB::WorldPos()
{
	return owner->pos + pos;
}

bool AABB::TopActive()
{
	return directionOn & 1;
}

bool AABB::LeftActive()
{
	return directionOn & 4;
}

bool AABB::RightActive()
{
	return directionOn & 8;
}

bool AABB::BottomActive()
{
	return directionOn & 2;
}

Object::Object()
	:pos( 0, 0 ), dynamic( false ), hitbox( false )
{
	
}

Object::Object( float x, float y, bool dyn, float mass)
	:pos( x, y ),dynamic(dyn), mass( mass )
{
}

void Object::AddToWorld()
{
	for( std::list<AABB*>::iterator it = boxes.begin(); it != boxes.end(); ++it )
	{
		//removed from the world on the worlds next iteration
		(*it)->active = true;
	}
}

DynamicObject::DynamicObject( float posx, float posy, bool hitbox,
	float velx, float vely, float accx, float accy, float maxVelx, float maxVely, float mass )
	:Object( posx, posy, true, mass ), vel( velx, vely ), acc( accx, accy ), maxVel( maxVelx, maxVely )
{
}

DynamicObject::DynamicObject()
	:Object( 0, 0, true, 0 )
{

}

void World::Step()
{
	//construct large AABB for each dynamic object 
	if( actors.empty() )
		return;

	//std::list<AABB*> largeDynamics;
	//AABB that encompasses all mini
	//for( std::list<Actor*>::iterator actorIt = actors.begin(); actorIt != actors.end(); ++actorIt )
	//{
	//	DynamicObject *object = (*actorIt)->objectBody;
	//	if( !object || object->boxes.size() == 0 )
	//		continue;

	//	
	//	assert( object->boxes.size() > 0 );
	//	//^^if its not active, remove it
	//	float top,left,right,bottom;
	//	Vector2f pos;

	//	std::list<AABB*>::iterator boxIt = object->boxes.begin();
	//	top = (*boxIt)->WorldTop();
	//	left = (*boxIt)->WorldLeft();
	//	right = (*boxIt)->WorldRight();
	//	bottom = (*boxIt)->WorldBottom();

	//	for( ;boxIt != object->boxes.end(); ++boxIt )
	//	{
	//		float tempTop = (*boxIt)->WorldTop();
	//		float tempLeft = (*boxIt)->WorldLeft();
	//		float tempRight = (*boxIt)->WorldRight();
	//		float tempBottom = (*boxIt)->WorldBottom();

	//		if( tempTop < top )
	//			top = tempTop;
	//		if ( tempLeft < left )
	//			left = tempLeft;
	//		if( tempRight > right )
	//			right = tempRight;
	//		if( tempBottom > bottom )
	//			bottom = tempBottom;
	//	}

	//	pos.x = ( right + left ) / 2;
	//	pos.y = ( bottom + top ) / 2;

	//	Vector2f size( right - left, bottom - top );
	//	largeDynamics.push_back( new AABB( ( pos - object->pos ), size, object ) );
	//}

		//get possible future
		/*Vector2f nextPos = object->pos;
			Vector2f nextVel = object->vel;
		Vector2f tempAcc = object->acc;

			if( nextVel.x > object->maxVel.x || nextVel.x < -object->maxVel.x )
			{
				if( nextVel.x < -object->maxVel.x )
					nextVel.x = -object->maxVel.x;
				else
					nextVel.x = object->maxVel.x ;

				tempAcc.x = 0;
			}
			else if( nextVel.y > object->maxVel.y || nextVel.y < -object->maxVel.y )
			{
				if( nextVel.y < -object->maxVel.y )
					nextVel.y = -object->maxVel.y;
				else 
					nextVel.y = object->maxVel.y;

				tempAcc.y = 0;
			}
			nextPos += nextVel * TIMESTEP;
			nextVel += tempAcc * TIMESTEP;

			Vector2f posDelta = nextPos - object->pos;
			if( posDelta.x < 0 ) 
				left += posDelta.x;
			else
				right += posDelta.x;
			if( posDelta.y < 0 )
				top += posDelta.y;
			else
				bottom += posDelta.y;*/

	/*std::list<Collision*> collisions;
	for( std::list<AABB*>::iterator it = largeDynamics.begin(); it != largeDynamics.end(); ++it )
	{	
		for( std::list<AABB*>::iterator it2 = largeDynamics.begin(); it2 != largeDynamics.end(); 
			++it2 )
		{
			if( (*it) != (*it2) )
			{
				Collision *c = (*it)->Collide( (*it2) );
				if( c != NULL )
				{
					collisions.push_back( c );
				}
			}
		}
		for( std::list< Object*>::iterator tileIt = tiles.begin(); tileIt != tiles.end(); ++tileIt )
		{
			for( std::list<AABB*>::iterator tileAABBIt = (*tileIt)->boxes.begin(); 
				tileAABBIt != (*tileIt)->boxes.end(); ++tileAABBIt )
			{
				Collision *c = (*it)->Collide( (*tileAABBIt) );
				if( c != NULL )
				{
					collisions.push_back( c );
				}
			}
		}
	}*/

	for( std::list<Actor*>::iterator it = actors.begin(); it != actors.end(); ++it )
	{
		DynamicObject *body = (*it)->objectBody;

		if( body->vel.x > body->maxVel.x || body->vel.x < -body->maxVel.x )
		{
			if( body->vel.x < -body->maxVel.x )
				body->vel.x = -body->maxVel.x;
			else
				body->vel.x = body->maxVel.x ;

			body->acc.x = 0;
		}
		if( body->vel.y > body->maxVel.y || body->vel.y < -body->maxVel.y )
		{
			if( body->vel.y < -body->maxVel.y )
				body->vel.y = -body->maxVel.y;
			else 
				body->vel.y = body->maxVel.y;

			body->acc.y = 0;
		}

		Object *closestBodyX = NULL;
		Object *closestBodyY = NULL;

		
		body->vel += body->acc * TIMESTEP;
		Vector2f vel = body->vel;

		//if( body->vel.x = 0 )
			//body->vel.x += body->acc.x * TIMESTEP;
		//if( body->vel.y != 0 )
			//body->vel.y += body->acc.y * TIMESTEP;

		for( std::list<Actor*>::iterator it2 = actors.begin(); it2 != actors.end(); ++it2 )
		{
			if( it != it2 )
			{
				DynamicObject *otherBody = (*it2)->objectBody;

				if( (body->vel.x > 0 && body->boxes.front()->WorldRight() <= otherBody->boxes.front()->WorldLeft()
					&& ( closestBodyX == NULL || closestBodyX->boxes.front()->WorldLeft() 
					> otherBody->boxes.front()->WorldLeft() ) ) 
					|| 
					(body->vel.x < 0 && body->boxes.front()->WorldLeft() >= otherBody->boxes.front()->WorldRight()
					&& ( closestBodyX == NULL || closestBodyX->boxes.front()->WorldRight() 
					< otherBody->boxes.front()->WorldRight() ) ) )
				{
					if( body->boxes.front()->WorldTop() + body->vel.y * TIMESTEP <= otherBody->boxes.front()->WorldBottom()
					&& body->boxes.front()->WorldBottom() + body->vel.y * TIMESTEP >= otherBody->boxes.front()->WorldTop() )
						closestBodyX = otherBody;
				}

				if( (body->vel.y > 0 && body->boxes.front()->WorldBottom() <= otherBody->boxes.front()->WorldTop()
					&& ( closestBodyY == NULL || closestBodyY->boxes.front()->WorldTop()
					> otherBody->boxes.front()->WorldTop() ) ) 
					|| 
					(body->vel.y < 0 && body->boxes.front()->WorldTop() >= otherBody->boxes.front()->WorldBottom()
					&& ( closestBodyY == NULL || closestBodyY->boxes.front()->WorldBottom() 
					< otherBody->boxes.front()->WorldBottom() ) ) )
				{
					if( body->boxes.front()->WorldLeft() + body->vel.x * TIMESTEP <= otherBody->boxes.front()->WorldRight()
					&& body->boxes.front()->WorldRight() + body->vel.x  * TIMESTEP >= otherBody->boxes.front()->WorldLeft() )
						closestBodyY = otherBody;
				}
			}
		}

		

		for( std::list<Object*>::iterator itTiles = tiles.begin(); itTiles != tiles.end(); ++itTiles )
		{
			Object *otherBody = (*itTiles);

			if( (body->vel.x > 0 && body->boxes.front()->WorldRight() <= otherBody->boxes.front()->WorldLeft()
					&& ( closestBodyX == NULL || closestBodyX->boxes.front()->WorldLeft() 
					> otherBody->boxes.front()->WorldLeft() ) ) 
					|| 
					(body->vel.x < 0 && body->boxes.front()->WorldLeft() >= otherBody->boxes.front()->WorldRight()
					&& ( closestBodyX == NULL || closestBodyX->boxes.front()->WorldRight() 
					< otherBody->boxes.front()->WorldRight() ) ) )
				{
					if( body->boxes.front()->WorldTop() + body->vel.y * TIMESTEP <= otherBody->boxes.front()->WorldBottom()
					&& body->boxes.front()->WorldBottom() + body->vel.y * TIMESTEP >= otherBody->boxes.front()->WorldTop() )
						closestBodyX = otherBody;
				}

				if( (body->vel.y > 0 && body->boxes.front()->WorldBottom() <= otherBody->boxes.front()->WorldTop()
					&& ( closestBodyY == NULL || closestBodyY->boxes.front()->WorldTop()
					> otherBody->boxes.front()->WorldTop() ) ) 
					|| 
					(body->vel.y < 0 && body->boxes.front()->WorldTop() >= otherBody->boxes.front()->WorldBottom()
					&& ( closestBodyY == NULL || closestBodyY->boxes.front()->WorldBottom() 
					< otherBody->boxes.front()->WorldBottom() ) ) )
				{
					if( body->boxes.front()->WorldLeft() + body->vel.x * TIMESTEP <= otherBody->boxes.front()->WorldRight()
					&& body->boxes.front()->WorldRight() + body->vel.x  * TIMESTEP >= otherBody->boxes.front()->WorldLeft() )
						closestBodyY = otherBody;
				}
		}

		
		
		//if( body->vel.x == 0 )
		//	body->vel.x += body->acc.x * TIMESTEP;		
		//if( body->vel.y == 0 )
		//	body->vel.y += body->acc.y * TIMESTEP;		

		if( closestBodyX != NULL )
		{
			float collisionDistanceX = -1.5;
			if( vel.x > 0 )
				collisionDistanceX = (closestBodyX->boxes.front()->WorldLeft() 
				- body->boxes.front()->WorldRight()) - 1;
			else if( vel.x < 0 )
				collisionDistanceX = (closestBodyX->boxes.front()->WorldRight()
				- body->boxes.front()->WorldLeft()) + 1;

			//assert( vel.x != 0 );
			
			if( abs( vel.x * TIMESTEP )  > abs( collisionDistanceX ) )
			{
				Actor *a = (Actor*)(body->userData);
				
				Collision col;
				col.objectA = body;
				col.objectB = closestBodyX;
				if( vel.x > 0 )
					col.normal = Vector2f( -1, 0 );
				else
					col.normal = Vector2f( 1, 0 );

				a->collisions.push_back( col );

				body->pos.x += collisionDistanceX;
				body->vel.x = 0;
				body->acc.x = 0;
			}
			else
				body->pos.x += vel.x * TIMESTEP;
		}
		else
			body->pos.x += vel.x * TIMESTEP;

		if( closestBodyY != NULL )
		{
			float collisionDistanceY = 0;
			if( vel.y > 0 )
				collisionDistanceY = (closestBodyY->boxes.front()->WorldTop() 
					- body->boxes.front()->WorldBottom()) - 1;
			else if( vel.y < 0 )
				collisionDistanceY = (closestBodyY->boxes.front()->WorldBottom() 
					- body->boxes.front()->WorldTop()) + 1;
			
			//assert( vel.y != 0 );


			if( abs( vel.y * TIMESTEP ) > abs( collisionDistanceY ) )
			{
				Actor *a = (Actor*)(body->userData);
				
				Collision col;
				col.objectA = body;
				col.objectB = closestBodyX;
				if( vel.y > 0 )
					col.normal = Vector2f( 0, -1 );
				else
					col.normal = Vector2f( 0, 1 );

				bool copied = false;

				for( std::list<Collision>::iterator cIt = a->collisions.begin();
					cIt != a->collisions.end(); ++cIt )
				{
					if( (*cIt).objectB == col.objectB )
					{
						//(*cIt).hitType = col.hitType;
						//(*cIt).normal = col.normal;
						copied = true;
						break;
					}
				}

				if( !copied )
					a->collisions.push_back( col );

				body->pos.y += collisionDistanceY;
				body->vel.y = 0;
				body->acc.y = 0;
			}
			else
				body->pos.y += vel.y * TIMESTEP;
		}
		else
			body->pos.y += vel.y * TIMESTEP;

		
	}
	
	/*for( std::list<Actor*>::iterator actorIt = actors.begin(); actorIt != actors.end(); ++actorIt )
	{
		if( (*actorIt)->objectBody )
		(*actorIt)->objectBody->UpdatePosition();
	}*/
	//for( std::list<Collision*>::iterator it = collisions.begin(); it != collisions.end(); ++it )
	//{
	//	if( !(*it)->objectA->dynamic )
	//	{
	//		assert( 0 && "objects searching for collisions should be dynamic only" );
	//	}

	//	DynamicObject * dynObject = (DynamicObject*)((*it)->objectA);
	//	if( (*it)->normal.x != 0 )
	//		dynObject->vel.x = 0;
	//	if( (*it)->normal.y != 0 )
	//		dynObject->vel.y = 0;
	//		
	//	//set the velocity to zero?
	//	if( !(*it)->objectB->dynamic || (*it)->objectA->mass == 0 || (*it)->objectB->mass == 0  )
	//	{
	//		(*it)->objectA->pos += (*it)->normal * ((*it)->overlap );
	//	}
	//	else
	//	{
	//		float total = (*it)->objectA->mass + (*it)->objectB->mass;
	//		float ratio = (*it)->objectA->mass / total;
	//		(*it)->objectA->pos += (*it)->normal * ((*it)->overlap * ratio );
	//	}

	//	
	//	//(*it)->normal.x *= -1;
	//	//(*it)->normal.y *= -1;
	//	//(*it)->objectB->pos -= (*it)->normal * ((*it)->overlap / 2 );
	//}
	float tileSize = 30;
	//levels need to be of even tile height and width
}
