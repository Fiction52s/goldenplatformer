#include "GameEvent.h"
#include <iostream>
using namespace std;




GameEvent::GameEvent( std::string &type, /*GameEventTypes::Type type*/ void * params )
	:m_type( type ), m_params( params )
{}

GameEventManager * GameEventManager::m_instance = NULL;

GameEventManager& GameEventManager::GetSingleton()
{
	static GameEventManager instance;

	return instance;
}

void GameEventManager::Attach( /*GameEventTypes::Type type*/ std::string type, 
	GameEventHandler * handler )
{
	m_handlers[ type ].push_back( handler );
}



void GameEventManager::Detach( /*GameEventTypes::Type*/ std::string type, GameEventHandler * handler )
{
	list< GameEventHandler * > & handlerList = m_handlers[ type ];
	list< GameEventHandler * >::iterator it = handlerList.begin();
	for( ; it != handlerList.end(); ++it )
	{
		if( (*it) == handler )
			break;
	}
	handlerList.erase( it );
}

//^^Note: Needs testing
void GameEventManager::Detach( GameEventHandler * handler )
{
	for( std::map< /*GameEventTypes::Type*/std::string, 
		std::list< GameEventHandler * > >::iterator typeIt = 
		m_handlers.begin(); typeIt != m_handlers.end(); ++typeIt )
	{
		list< GameEventHandler * >::iterator it = (*typeIt).second.begin();
		for( ; it != (*typeIt).second.end(); ++it )
		{
			if( (*it) == handler )
			{
				(*typeIt).second.erase( it );
				break;
			}
		}
		
	}
}

void GameEventManager::Broadcast( std::string type, void *params )
{
	list< GameEventHandler * > & handlers = m_handlers[ type ];
	SharedEventPtr gEvent( new GameEvent( type, params ) );
	for( list< GameEventHandler* >::iterator it = handlers.begin(); it != handlers.end(); ++it )
	{
		(*it)->HandleGameEvent( gEvent );
	}
}