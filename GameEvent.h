#ifndef __GAMEEVENT_H__
#define __GAMEEVENT_H__
#include <list>
#include <map>
#include <string>
#include <boost/shared_ptr.hpp>



//^^eventually fill this enum, for now just use a string
//so that event types can be easily added and removed?
namespace GameEventTypes
{
	enum Type
	{

	};
}

struct GameEvent
{
	GameEvent( std::string &type, /*GameEventTypes::Type type*/ 
		void * params );
	//GameEventTypes::Type m_type;
	std::string m_type;
	void * m_params;
};

typedef boost::shared_ptr<GameEvent> SharedEventPtr;

class GameEventHandler
{
public:
	virtual void HandleGameEvent( 
		SharedEventPtr gameEvent ) = 0;
};

/*	
	^^Note: depending on the amount of GameEventTypes and
	how they are structured, an std::map might not be ideal, and a 
	structure that uses indexing, like an array (because number of
	types are known) could be more efficient.
*/

class GameEventManager
{
public:
	static GameEventManager& GetSingleton();
	void Attach( /*GameEventTypes::Type type*/ std::string, 
		GameEventHandler * handler );
	void Detach( /*GameEventTypes::Type type*/ std::string,
		GameEventHandler * handler );
	void Detach( GameEventHandler * handler );
	void Broadcast( std::string type, void *params = NULL );
private:
	GameEventManager() {};
	GameEventManager( GameEventManager const& ) {};
	GameEventManager & operator=( GameEventManager const& ) {};
	static GameEventManager *m_instance;
	std::map< /*GameEventTypes::Type*/ std::string, 
		std::list< GameEventHandler * > > m_handlers;
};


#endif