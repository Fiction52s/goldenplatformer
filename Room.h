#ifndef __ROOM_H__
#define __ROOM_H__

#include <list>
#include <map>
#include "Box2d.h"
#include "Actor.h"


//temp points disappear when you leave the room



class Stage;
struct Room
{
	Room( Stage *stage );
	~Room();
	std::map<std::string, b2Vec2> referencePoints;
	std::map<std::string, b2Vec2> tempReferencePoints;
	void SetTempPoint( const std::string &name, const b2Vec2 &point );
	b2Vec2 GetPoint( const std::string &name ); //not a reference because I'm not sure how it works in lua sometimes.
	b2Vec2 GetTempPoint( const std::string &name );
	void Enter( const std::string &doorName );
	void Exit();
	Room *GetOwner();
	b2Vec2 &GetSpawnPoint();
	std::list<b2FixtureDef*> eventParams;
	b2Body *eventBody;
	std::list<b2Vec2> border;
	float32 top;
	float32 bottom;
	float32 left;
	float32 right;
	std::list<ActorDef*> actorDefs;
	std::list<TrueActor*> actors;
	std::string name;
	Stage *stage;
	b2Vec2 spawn;
	Room *owner; 
	std::string ownerName;
	//if it has a parent then if
	//you die here you spawn 
	//in the parent room
	
	//void Update();
};

struct Door
{
	//std::string roomA;
	//std::string roomB;
	b2Body *body;
	bool currentA; //currently in rooma or roomb?
	bool open;
	bool collide;
	Room *roomA;
	Room *roomB;
	std::string name;
	float32 angle;
	//Room *roomB;
	//b2Body *body;
	//b2Fixture *fix;

};

#endif