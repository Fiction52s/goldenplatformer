#include "Room.h"
#include <string>
#include "Stage.h"

using namespace std;

Room::Room( Stage *stage )
	:top( 0 ), left( 0 ), right( 0 ), bottom( 0 ), name( "" ), eventBody( NULL ), stage( stage ), spawn( -1,-1 ),owner( NULL )
{
	
}

Room::~Room()
{
	if( eventBody != NULL )
	{
		stage->world->DestroyBody( eventBody );
		eventBody = NULL;
	}

	for( list<b2FixtureDef*>::iterator it = eventParams.begin(); it != eventParams.end(); ++it )
	{
		delete (*it);
	}

	for( list<ActorDef*>::iterator it = actorDefs.begin(); it != actorDefs.end(); ++it )
	{
		delete (*it);
	}

	for( list<TrueActor*>::iterator it = actors.begin(); it != actors.end(); ++it )
	{
		delete (*it);
	}
}

void Room::Enter( const string &doorName )
{
	b2BodyDef eventDef;
	eventDef.type = b2_staticBody;
	eventBody = stage->world->CreateBody( &eventDef );
	for( list<b2FixtureDef*>::iterator it = eventParams.begin(); it != eventParams.end(); ++it )
	{
		eventBody->CreateFixture( (*it) );
	}

	for( list<ActorDef*>::iterator it = actorDefs.begin(); it != actorDefs.end(); ++it )
	{
		ActorDef &ad = *(*it);

		//^^create group if i have an actorCount greater than 1


		//create all the actors now, but eventually have different spawning types in place
		TrueActor * a = stage->CreateActor( ad.type, ad.pos, ad.vel, ad.facingRight, ad.reverse, ad.angle, ad.parent );

		for( list<pair<string,float>>::iterator preInitMessagesIt = ad.preInitMessages.begin();
			preInitMessagesIt != ad.preInitMessages.end(); ++preInitMessagesIt )
		{
			a->Message( NULL, (*preInitMessagesIt).first, (*preInitMessagesIt).second );
		}

		actors.push_back( a );
	}
//	stage->c.mode = Camera::CameraMode::transition;
	
}

void Room::Exit()
{
	stage->world->DestroyBody( eventBody );
	eventBody = NULL;

	for( list<TrueActor*>::iterator it = actors.begin(); it != actors.end(); ++it )
	{
		//delete (*it);
		(*it)->isAlive = false;
	}
	actors.clear();
	//cleanup 
}

b2Vec2 & Room::GetSpawnPoint()
{
	if( owner == NULL )
	{
		//assert( spawn.x >=0 && spawn.y >=0 );
		return spawn;
	}
	else
	{
		return owner->GetSpawnPoint();
	}
}

void Room::SetTempPoint( const string &name, const b2Vec2 &point )
{
	tempReferencePoints[name] = point; //duplicates just update the point value
}

Room * Room::GetOwner()
{
	if( owner == NULL )
	{
		return this;
	}
	else
	{
		return owner->GetOwner();
	}
}

b2Vec2 Room::GetPoint( const string &name )
{
	try 
	{
		return referencePoints[name];
	}
	catch( int e )
	{
		string errorMsg = string( "There is no point by the name: " ) + name;
		assert( 0 && errorMsg.c_str() );
		//b2Vec2
	}
}

b2Vec2 Room::GetTempPoint( const string &name )
{
	try 
	{
		return tempReferencePoints[name];
	}
	catch( int e )
	{
		string errorMsg = string( "There is no point by the name: " ) + name;
		assert( 0 && errorMsg.c_str() );
	}
}