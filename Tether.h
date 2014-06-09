#ifndef __TETHER_H__
#define __TETHER_H__

#include "Actor.h"

struct Tether : public b2RayCastCallback
{
	Tether( Stage *st, b2Body *playerBody, b2World *world, float maxLength );
	~Tether();
	b2Body *bodies;
	void Anchor();
	void Split( PlayerChar *player, b2Vec2 splitPoint );
	void Join( PlayerChar *player );
	b2Vec2 anchor;
	void Update( PlayerChar *player );
	void SetMaxLength( float max );
	b2Vec2 oldPlayerPos;
	b2Body * swingBody;
	b2Body * anchorBody;
	b2Body * testingBody;
	b2Body *playerBody;
	std::list<b2Vec2> anchorPoints;
	Stage *st;
	b2World *world;

	float lockedRopeLength;

	float32 ReportFixture( b2Fixture* fixture, 
		const b2Vec2& point, const b2Vec2& normal, 
		float32 fraction);
	void Lock( PlayerChar *player );
	void Reset();
	bool CheckTetherShot();
	void Shot( PlayerChar *player,const b2Vec2 &vel );
	void Grow( float amount );
	void Shrink( float amount );
	b2Vec2 closest;
	float32 closestFrac;
	std::list<std::pair<b2Vec2, float>> closestList;
	bool anchored;

//	b2Vec2 shotDelta;
//	b2Vec2 shotVelocity;
//	bool shotActive;

	bool shotHit;
	b2Vec2 shotHitPoint;

	std::string GetState();
	
	bool splitting;
	bool hitSomething;
	bool checking;
	bool hitSomethingChecking;

	std::string raySetting;
	bool shotCheckFlag;

	bool locked;

	b2Vec2 shotPos;
	b2Vec2 shotVelocity;
	bool shotActive;

	float maxLen;

	b2Body *shotBody;

	bool shortReset;
};

#endif
