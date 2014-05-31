#ifndef __TETHER_H__
#define __TETHER_H__

#include "Actor.h"

struct Tether : public b2RayCastCallback
{
	Tether( Stage *st, b2Body *playerBody, b2World *world, float maxLength );
	~Tether();
	b2Body *bodies;
	void Init( b2Vec2 pos );
	void Split( PlayerChar *player, b2Vec2 splitPoint );
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

	float32 ReportFixture( b2Fixture* fixture, 
		const b2Vec2& point, const b2Vec2& normal, 
		float32 fraction);

	void Reset();
	bool CheckTetherShot( b2Body *shotBody );
	b2Vec2 closest;
	float32 closestFrac;
	std::list<std::pair<b2Vec2, float>> closestList;
	bool initialized;
	
	bool splitting;
	bool something;
	bool checking;
	bool something2;

	float maxLen;
};

#endif
