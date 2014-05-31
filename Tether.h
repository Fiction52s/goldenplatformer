#ifndef __TETHER_H__
#define __TETHER_H__

#include "Actor.h"

struct Tether : public b2RayCastCallback
{
	Tether( Stage *st, b2Body *playerBody, b2Vec2 pos, b2World *world );
	b2Body *bodies;
	void Init();
	void Split( PlayerChar *player, b2Vec2 splitPoint );
	b2Vec2 anchor;
	void Update( PlayerChar *player );
	b2Vec2 oldPlayerPos;
	b2Body * swingBody;
	b2Body * anchorBody;
	b2Body * testingBody;
	std::list<b2Vec2> anchorPoints;
	Stage *st;
	float32 ReportFixture( b2Fixture* fixture, 
		const b2Vec2& point, const b2Vec2& normal, 
		float32 fraction);
	b2Vec2 closest;
	float32 closestFrac;
	std::list<std::pair<b2Vec2, float>> closestList;

	bool splitting;
	bool something;
};

#endif
