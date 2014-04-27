#include "Room.h"
#include <string>
#include "Stage.h"
#include <iostream> 

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
	
	for( list<Squad*>::iterator it = squads.begin(); it != squads.end(); ++it )
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


	/*for( list<ActorDef*>::iterator it = actorDefs.begin(); it != actorDefs.end(); ++it )
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
	}*/
	stage->c.mode = Camera::CameraMode::transition;
	
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

	for( list<Squad*>::iterator it = squads.begin(); it != squads.end(); ++it )
	{
		(*it)->Deactivate();
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

void Room::UpdateSquads( sf::Vector2f pos, sf::Vector2f size )
{
	for( list<Squad*>::iterator it = squads.begin(); it != squads.end(); ++it )
	{
		/*cout << "checking camera for: " << (*it)->name;
		if( (*it)->activated )
			cout << ", its activated";
		else
			cout << ", its not activated";

		if( (*it)->initialized )
			cout << " and its initialized" <<endl;
		else
			cout << " and its not initialized" << endl;*/
		(*it)->CheckCamera( pos, size );
	}
}

Squad::Squad( Stage *st, const string &name )
	:name( name ), st( st ), initialized( false ), activated( false )
{
}

Squad::~Squad()
{
	for( list<ActorDef*>::iterator it = actorDefs.begin(); it != actorDefs.end(); ++it )
	{
		delete (*it);
	}
}

void Squad::CheckCamera( sf::Vector2f pos, sf::Vector2f size )
{
	if( name == "room" )
	{
		if( initialized && activated )
		{
			return;
		}
		else
		{
			initialized = true;
			activated = true;
		}
	}

	//if( activeActors.empty() )
	//{
	//	activated = false;
	//	initialized = false;
	//}

	b2Vec2 camPos( pos.x * SF2BOX, pos.y * SF2BOX );
	
	b2Vec2 camSize( size.x * SF2BOX, size.y * SF2BOX );
	//cout << "camSize: " << camSize.x << ", " << camSize.y << endl;
	b2Vec2 camHalfSize( camSize.x / 2, camSize.y / 2 );
	//cout << "camhalfSize: " << camHalfSize.x << ", " << camHalfSize.y << endl;
	
	float32 camLeft = camPos.x - camHalfSize.x;
	float32 camRight = camPos.x + camHalfSize.x;
	float32 camTop = camPos.y - camHalfSize.y;
	float32 camBottom = camPos.y + camHalfSize.y;

	//cout << "camleft: " << camLeft << ", camRight: " << camRight << ", camTop: " 
	//	<< camTop << ", camBottom: " << camBottom << endl;
	sf::FloatRect camActorRect( camLeft, camTop, camSize.x, camSize.y );
	sf::FloatRect camRect( pos.x - size.x /2, pos.y - size.y / 2, size.x, size.y );
	sf::FloatRect medRect( camPos.x - camSize.x, camPos.y - camSize.y, 
		camSize.x * 2, camSize.y * 2 );

	if( !initialized )
	{	
		for( std::list<ActorDef*>::iterator it = actorDefs.begin(); it != actorDefs.end(); ++it )
		{
			ActorDef &ad = *(*it);
			b2Vec2 actorPos( ad.pos );
			if( actorPos.x >= medRect.left && actorPos.x <= ( medRect.left + medRect.width ) 
				&& actorPos.y >= medRect.top && actorPos.y <= ( medRect.top + medRect.height ) )
			{
				initialized = true;
				break;
			}
		}

		if( initialized )
		{
			for( std::list<ActorDef*>::iterator it = actorDefs.begin(); it != actorDefs.end(); ++it )
			{
				ActorDef &ad = *(*it);
				TrueActor *a = new SingleActor( ad.type, ad.pos, ad.vel, ad.facingRight, 
					ad.reverse, ad.angle, ad.parent, st );
				a->squad = ad.squad;
				//^^not sure if i should keep this here but it makes things a lot easier.
				if( st->cloneWorld )
				{
					st->cloneActiveActors.push_back( a );
				}
				else
				{
					st->activeActors.push_back( a );
				}

				for( list<pair<string,float>>::iterator preInitMessagesIt = ad.preInitMessages.begin();
					preInitMessagesIt != ad.preInitMessages.end(); ++preInitMessagesIt )
				{
					a->Message( NULL, (*preInitMessagesIt).first, (*preInitMessagesIt).second );
				}

				a->Init( st->world );
				a->UpdateSprites();

				a->SetPause( true );
				activeActors.push_back( a );	
			}
		}
	}
	if( initialized && !activated )
	{
		for( std::list<TrueActor*>::iterator it = activeActors.begin(); it != activeActors.end(); ++it )
		{
			TrueActor *a = (*it);
			sf::FloatRect aabb = a->GetSpriteAABB();
			
			if( aabb.intersects( camRect ) )
			{
				activated = true;
				break;
			}
		}

		if( activated )
		{
			for( std::list<TrueActor*>::iterator it = activeActors.begin(); it != activeActors.end(); ++it )
			{
				(*it)->SetPause( false );
			}
		}
	}
	
	if( initialized && activated )
	{
		//sf::FloatRect largeRect( camLeft - camSize.x, camTop - camSize.y, camSize.x * 2, camSize.y * 2 );

		if( !activeActors.empty() )
		{
			sf::FloatRect largeRect( camRect.left - size.x, camRect.top - size.y, size.x * 3, size.y * 3 );

			for( std::list<TrueActor*>::iterator it = activeActors.begin(); it != activeActors.end(); )
			{
				TrueActor *a = (*it);
				sf::FloatRect aabb = a->GetSpriteAABB();
				if( !aabb.intersects( largeRect ) )
				{
					it = activeActors.erase( it );
					a->Kill();
				}
				else
				{
					++it;
				}
			}
		}
		else
		{
			bool reset = true;
			for( std::list<ActorDef*>::iterator it = actorDefs.begin(); it != actorDefs.end(); ++it )
			{
				ActorDef &ad = *(*it);
				b2Vec2 actorPos( ad.pos );

				if( actorPos.x >= medRect.left && actorPos.x <= ( medRect.left + medRect.width ) 
				&& actorPos.y >= medRect.top && actorPos.y <= ( medRect.top + medRect.height ) )
				//if( actorPos.x >= camLeft && actorPos.x <= camRight
				//	&& actorPos.y >= camTop && actorPos.y <= camBottom )
				{
					reset = false;
					break;
				}
			}

			if( reset )
			{
				initialized = false;
				activated = false;
			}
		}

		
	}
	
}

void Squad::DeactivateActor( TrueActor *actor )
{
	activeActors.remove( actor );
}

void Squad::Deactivate()
{
	activeActors.clear();
	activated = false;
	initialized = false;
}