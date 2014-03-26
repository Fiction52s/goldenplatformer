#ifndef __PHSYSICS_H__
#define __PHSYSICS_H__
#include <list>
#include <SFML/System.hpp>


#define TIMESTEP 1.f/60.f

struct Object;
struct Collision;
struct Actor;

struct AABB
{
	AABB( float p_x, float p_y, float p_w, float p_h, 
		Object *p_owner );
	AABB( const sf::Vector2f &pos, 
		const sf::Vector2f &size, Object *p_owner );
	AABB(){};
	Collision * Collide( AABB *other );
	sf::Vector2f GetWorldPos();
	float WorldRight();
	float WorldTop();
	float WorldBottom();
	float WorldLeft();
	float LocalRight();
	float LocalTop();
	float LocalBottom();
	float LocalLeft();
	sf::Vector2f WorldPos();
	bool TopActive();
	bool LeftActive();
	bool RightActive();
	bool BottomActive();

	sf::Vector2f pos;
	sf::Vector2u size;
	bool active;
	//0x1 = up, 0x2 = down, 0x4 = left, 0x8 = right
	sf::Uint8 directionOn;
	Object *owner;
};

//any physical object
struct Object
{
	Object();
	Object( float x, float y, bool dynamic,
		float mass = 0 );
	void AddToWorld();
	void RemoveFromWorld();
	sf::Vector2f pos;
	std::list<AABB*> boxes;
	bool dynamic;
	bool hitbox;
	bool vulnerable;
	std::string name;
	sf::Uint32 filterInfo;
	sf::Uint32 userData;
	float mass;
};

struct DynamicObject : public Object
{
	DynamicObject( float posx, float posy, 
		bool hitbox,float velx, float vely, 
		float accx, float accy, float maxVelx, 
		float maxVely, float mass );
	DynamicObject();
	sf::Vector2f vel;
	sf::Vector2f acc;
	sf::Vector2f maxVel;
};

struct Hitbox : public Object
{
	Hitbox( float x, float y, bool dynamic );
	//list of processes
};

struct Collision
{
	//objectA is the sender
	Object *objectA;
	Object *objectB;
	//sf::Vector2f pos;
	sf::Vector2f normal;
	//"ice", "heal" etc for sound effects and visuals
	std::string hitType;
};

struct Actor;
struct World
{
	std::list<Object*> tiles;
	std::list<Actor*> actors;
	sf::Vector2f center;
	sf::Vector2f size;
	void Step();
};




#endif