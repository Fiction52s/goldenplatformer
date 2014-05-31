#include "Tether.h"
#include <iostream>

using namespace std;

Tether::Tether( Stage *st, b2Body *playerBody, b2World *world, float maxLength )
	:st( st ), maxLen( maxLength ), checking( false ), playerBody( playerBody ), world( world ), 
	initialized( false )
{
	maxLen = 10;
	//maxLen = 10;
	
	//b2RopeJoint *rj = (b2RopeJoint*)world->CreateJoint( &rjd );

	/*anchor = pos;
	anchor.y -= 5;

	float dx = playerBody->GetPosition().x - anchor.x;
	float dy = playerBody->GetPosition().y - anchor.y;

	float t0 = atan2( dy, dx );

	float vx = playerBody->GetLinearVelocity().x;
	float vy = playerBody->GetLinearVelocity().y;
	
	b2Vec2 a = b2Vec2( 0,2 * cos( t0 ) );

	b2PolygonShape shape;
	shape.SetAsBox( .1, 3, b2Vec2(0,3), 0 );*/

	

/*	b2BodyDef bd;
	bd.type = b2_dynamicBody;
	//pos.x = newX;
	//pos.y = newY;
	bd.position = groundDef.position;
	bd.allowSleep = false;

	swingBody = world->CreateBody( &bd );
	swingBody->CreateFixture( &d );

	b2RevoluteJointDef jd;
	jd.bodyA = anchorBody;
	jd.bodyB = swingBody;
	b2RevoluteJoint *r = (b2RevoluteJoint*)world->CreateJoint( &jd );
	r->SetMaxMotorTorque( 20 );
	r->EnableMotor( true );


	//b2RevoluteJointDef jd;
	jd.bodyA = playerBody;
	jd.bodyB = swingBody;
	jd.localAnchorB = b2Vec2( 0, 6 );
	b2RevoluteJoint *r2 = (b2RevoluteJoint*)world->CreateJoint( &jd );
	r2->SetMaxMotorTorque( 20 );
	r2->EnableMotor( true );
	//jd.localAnchorA.Set( 0, radius );
	//jd.localAnchorB.Set( 0, -radius );*/


	//playerBody->SetLinearVelocity( playerBody->GetLinearVelocity().x, playerBody->GetLinearVelocity().y );
	//playerBody->setan
		//-= gravity
	/*cout << "creating the tether now" << endl;
	float radius = .4;

	b2CircleShape cs;
	cs.m_radius = radius;

	b2PolygonShape shape;
	shape.SetAsBox( .1, radius / 2, b2Vec2(0,0), 0 );

	b2FixtureDef d;
	d.density = 50;
	d.friction = .3;
	d.restitution = 0;
	d.shape = &shape;
	CollisionLayers::SetupFixture( CollisionLayers::PlayerHitbox, d.filter.categoryBits,
		d.filter.maskBits );

	int maxUnits = 8;

	b2BodyDef groundDef;
	groundDef.position = pos;
	groundDef.position.x -= radius * maxUnits;
	

	b2Body *groundBody = world->CreateBody( &groundDef );
	groundBody->CreateFixture( &d );

	//b2Body * ground = world->CreateBody( &groundDef );

	b2Body *prev = groundBody;
	
	for( int i = 0; i < maxUnits; ++i )
	{
		float newX = pos.x;
		float newY = pos.y - i * radius;

		b2BodyDef bd;
		bd.type = b2_dynamicBody;
		pos.x = newX;
		pos.y = newY;
		bd.position = pos;
		bd.allowSleep = false;

		b2Body *body = world->CreateBody( &bd );
		body->ApplyForceToCenter( b2Vec2( 0, 5 ) );
		b2Fixture *fix = body->CreateFixture( &d );
		

		b2RevoluteJointDef jd;
		jd.bodyA = prev;
		jd.bodyB = body;
		jd.localAnchorA.Set( 0, radius );
		jd.localAnchorB.Set( 0, -radius );

		b2RopeJointDef rjd;
		rjd.maxLength = .5;
		rjd.bodyA = prev;
		rjd.bodyB = body;
		//b2RopeJoint *rj = (b2RopeJoint*)world->CreateJoint( &rjd );
		
		b2RevoluteJoint *r = (b2RevoluteJoint*)world->CreateJoint( &jd );
		r->SetMaxMotorTorque( 20 );
	
		r->EnableMotor( true );

		b2DistanceJointDef djd;
		djd.localAnchorA.Set( 0, radius );
		djd.localAnchorB.Set( 0, -radius );
		djd.bodyA = prev;
		djd.bodyB = body;
		//b2DistanceJoint *dj = (b2DistanceJoint*)world->CreateJoint( &djd );

		prev = body;
		
	}

	

	b2RevoluteJointDef jd;
	jd.bodyA = prev;
	jd.bodyB = playerBody;
	jd.localAnchorA.Set( 0, radius );
	jd.localAnchorB.Set( 0, 0 );
	b2RevoluteJoint *rr = (b2RevoluteJoint*)world->CreateJoint( &jd );
	rr->SetMaxMotorTorque( 20 );
	rr->EnableMotor( true );

	b2RopeJointDef rjd;
	rjd.maxLength = .5;
	rjd.bodyA = prev;
	rjd.bodyB = groundBody;
	//b2RopeJoint *rj = (b2RopeJoint*)world->CreateJoint( &rjd );
		
	//b2RevoluteJoint *r = (b2RevoluteJoint*)world->CreateJoint( &jd );
	//r->SetMaxMotorTorque( 1 );
	//r->EnableMotor( true );

	//b2RopeJointDef rjd;
	rjd.maxLength = maxUnits * radius * 2 + radius * 2;

	rjd.bodyA = playerBody;
	rjd.bodyB = groundBody;
	rjd.localAnchorA.Set( 1, 0 );
	//b2RopeJoint *rj2 = (b2RopeJoint*)world->CreateJoint( &rjd );*/

	//b2BodyDef testingDef;
	//testingBody = world->CreateBody( &testingDef );

}

Tether::~Tether()
{
	
}

void Tether::Init( b2Vec2 pos )
{
	initialized = true;
	anchorPoints.push_back( pos );

	b2FixtureDef d;
	d.density = 5;
	d.friction = .3;
	d.restitution = 0;
	//d.shape = &shape;
	CollisionLayers::SetupFixture( CollisionLayers::Tether, d.filter.categoryBits,
		d.filter.maskBits );

	b2BodyDef groundDef;
	groundDef.position = pos;
	//groundDef.position.y -= 6;
	anchorBody = world->CreateBody( &groundDef );

	b2RopeJointDef rjd;
	rjd.maxLength = maxLen;
	rjd.bodyA = playerBody;
	rjd.bodyB = anchorBody;
	rjd.localAnchorA.Set( 1, 0 );
	b2RopeJoint *rj = (b2RopeJoint*)world->CreateJoint( &rjd );
}

bool Tether::CheckTetherShot( b2Body *shotBody )
{
	something2 = false;
	checking = true;
	st->world->RayCast( this, shotBody->GetPosition(), st->player->GetPosition() );
	checking = false;
	return something2;
}

void Tether::Update( PlayerChar *player )
{
	//make a triangle shape

	/*b2Vec2 vertices[3];
	vertices[0] = anchor;
	vertices[1] = oldPlayerPos;
	vertices[2] = player->GetPosition();
	b2PolygonShape ps;
	ps.Set( vertices, 3 );

	b2Fixture *f = testingBody->GetFixtureList();
	while( f != NULL )
	{
		b2Fixture *temp = f->GetNext();
		testingBody->DestroyFixture( f );
		f = temp;
	}

	b2FixtureDef testFix;
	testFix.shape = &ps;
	CollisionLayers::SetupFixture( CollisionLayers::TetherTest, testFix.filter.categoryBits, testFix.filter.maskBits );
	testingBody->CreateFixture( &testFix );*/



	//cout << "before ray cast" << endl;
	b2World *world = anchorBody->GetWorld();

	closestFrac = 1.1;
	closest.Set( -1, -1 );
	splitting = true;

	world->RayCast( this, anchorPoints.back(), player->GetPosition() );

	float totalDist = 0;

	closestFrac = 1.1;
	
	while( !closestList.empty() )
	{
		float min = 1.1;
		b2Vec2 minVec;
		list<pair<b2Vec2,float>>::iterator minIt;
		for( list<pair<b2Vec2, float>>::iterator it = closestList.begin(); it != closestList.end(); ++it )
		{
			if( (*it).second < min )
			{
				minVec = (*it).first;
				min = (*it).second;
				minIt = it;
			}
		}
		closestList.erase( minIt );

		if( minVec.x == anchorPoints.back().x && minVec.y == anchorPoints.back().y )
		{
			continue;
		}
		anchorPoints.push_back( minVec );
		anchorBody->SetTransform( minVec, 0 );

		float totalDist = 0;
		for( list<b2Vec2>::iterator anchorIt = anchorPoints.begin(); anchorIt != anchorPoints.end(); )
		{
			list<b2Vec2>::iterator past = anchorIt;
			anchorIt++;
			if( anchorIt == anchorPoints.end() )
				break;
			float diffX = (*past).x - (*anchorIt).x;
			float diffY = (*past).y - (*anchorIt).y;
			//cout << "diffx: " << diffX << ", diffY: " << diffY << endl;
			//cout << "past: " << (*past).x << ", " << (*past).y << endl;
			//cout << "anchor: " << (*anchorIt).x << ", " << (*anchorIt).y << endl;
			totalDist += sqrt( diffX * diffX + diffY * diffY );
		}

		b2RopeJoint *rj = (b2RopeJoint*)anchorBody->GetJointList()->joint;
		float xx = anchorPoints.back().x - closest.x;
		float yy = anchorPoints.back().y - closest.y;

		if( totalDist < maxLen )
		{
			rj->SetMaxLength( maxLen - totalDist );
		}
		else
		{
			rj->SetMaxLength( 1 );
			cout << "rope is too long" << endl;	
		}
		
		cout << "total dist: " << totalDist << endl;
		cout << "closestPoint: " << minVec.x << ", " << minVec.y << endl;//<< " .. fraction: " << closestFrac << endl;
		cout << "anchorPoints size: " << anchorPoints.size() << endl;
		
	}

	
	if( closestFrac <= 1 )
	{
	for( list<b2Vec2>::iterator anchorIt = anchorPoints.begin(); anchorIt != anchorPoints.end(); )
	{
		list<b2Vec2>::iterator past = anchorIt;
		anchorIt++;
		if( anchorIt == anchorPoints.end() )
			break;
		float diffX = (*past).x - (*anchorIt).x;
		float diffY = (*past).y - (*anchorIt).y;
		//cout << "diffx: " << diffX << ", diffY: " << diffY << endl;
		//cout << "past: " << (*past).x << ", " << (*past).y << endl;
		//cout << "anchor: " << (*anchorIt).x << ", " << (*anchorIt).y << endl;
		totalDist += sqrt( diffX * diffX + diffY * diffY );
	}

		b2RopeJoint *rj = (b2RopeJoint*)anchorBody->GetJointList()->joint;
		float xx = anchorPoints.back().x - closest.x;
		float yy = anchorPoints.back().y - closest.y;

		if( totalDist < 6 )
		{
			rj->SetMaxLength( 6  - totalDist );
		}
		else
		{
			rj->SetMaxLength( 1 );
			cout << "rope is too long" << endl;	
		}


		
		//else
		{
		//	rj->SetMaxLength( 6 );
		}
		
		cout << "total dist: " << totalDist << endl;
		cout << "closestPoint: " << closest.x << ", " << closest.y << " .. fraction: " << closestFrac << endl;
		anchorPoints.push_back( closest );
		anchorBody->SetTransform( closest, 0 );
		

	}


	if( anchorPoints.size() > 1 )
	{
		
		splitting = false;
		list<b2Vec2>::reverse_iterator rIt = anchorPoints.rbegin(); 
		rIt++;

		//second from the last
		something = false;
		world->RayCast( this, (*rIt), player->GetPosition() );	
		b2Vec2 blah( player->GetPosition().x - anchorPoints.back().x, player->GetPosition().y - anchorPoints.back().y );
		for( int b = 1; b < 5; ++b )
		{
			float b2 = (float)b / 5;
			b2Vec2 bbb( blah );
			bbb.x *= b2;
			bbb.y *= b2;
			world->RayCast( this, (*rIt), anchorPoints.back() + bbb );
			//cout << "start: " << (*rIt).x << ", " << (*rIt).y << endl;
			b2Vec2 xx(anchorPoints.back() +bbb);
			//cout << "end: " << xx.x << ", " << xx.y << endl;
		}



		//cout << "checking raycast starting at: " << (*rIt).x << ", " << (*rIt).y << " and going to " 
		//	<< player->GetPosition().x << ", " << player->GetPosition().y << endl;

		/*float lineAvx = player->GetPosition().x - anchorPoints.back().x;
		float lineAvy = player->GetPosition().y - anchorPoints.back().y;
		b2Vec2 line1( lineAvx, lineAvy );
		line1.Normalize();
		

		float lineBvx = anchorPoints.back().x - (*rIt).x;
		float lineBvy = anchorPoints.back().y - (*rIt).y; 
		b2Vec2 line2( lineBvx, lineBvy );
		line2.Normalize();

		//float dot = lineAvx * lineBvx + lineAvy * lineBvy;
		float dot = line1.x * line2.x + line1.y * line2.y;
		cout << "dot: " << dot << endl;*/
		if( !something )
		{
			cout << "joining" << endl;
			anchorPoints.pop_back();

			anchorBody->SetTransform( anchorPoints.back(), 0 );
		

			for( list<b2Vec2>::iterator anchorIt = anchorPoints.begin(); anchorIt != anchorPoints.end(); )
			{
				list<b2Vec2>::iterator past = anchorIt;
				anchorIt++;
				if( anchorIt == anchorPoints.end() )
					break;
				float diffX = (*past).x - (*anchorIt).x;
				float diffY = (*past).y - (*anchorIt).y;
				//cout << "diffx: " << diffX << ", diffY: " << diffY << endl;
				//cout << "past: " << (*past).x << ", " << (*past).y << endl;
				//cout << "anchor: " << (*anchorIt).x << ", " << (*anchorIt).y << endl;
				totalDist += sqrt( diffX * diffX + diffY * diffY );
			}

			b2RopeJoint *rj = (b2RopeJoint*)anchorBody->GetJointList()->joint;
			float xx = anchorPoints.back().x - closest.x;
			float yy = anchorPoints.back().y - closest.y;

			if( totalDist < maxLen )
			{
				rj->SetMaxLength( maxLen - totalDist );
			}
			else
			{
				rj->SetMaxLength( 1 );
				cout << "rope is too long" << endl;	
			}
		}
	}


	//cout << "after ray cast" << endl;

	

	
	


	oldPlayerPos = player->GetPosition();


}

void Tether::Split( PlayerChar* player, b2Vec2 splitPoint )
{
	/*cout << "split point: " << splitPoint.x << ", " << splitPoint.y << endl;
	//splitPoint.y += 1;
	anchor = splitPoint;


	b2PolygonShape shape;
	float xd = splitPoint.x - player->GetPosition().x;
	float yd = splitPoint.y - player->GetPosition().y;
	float newHeight = sqrt( xd * xd + yd * yd ) / 2; 
	shape.SetAsBox( .1, newHeight, b2Vec2(0,newHeight), 0 );

	b2FixtureDef d;
	d.density = 5;
	d.friction = .3;
	d.restitution = 0;
	d.shape = &shape;
	CollisionLayers::SetupFixture( CollisionLayers::Tether, d.filter.categoryBits,
		d.filter.maskBits );

	swingBody->SetTransform( splitPoint, 0 );

	b2BodyDef bd;
	bd.type = b2_dynamicBody;
	//pos.x = newX;
	//pos.y = newY;
	bd.position = splitPoint;
	bd.allowSleep = false;

	b2JointEdge *next = anchorBody->GetJointList();
	while( next != NULL )
	{
		b2JointEdge *temp = next->next;
		player->world->DestroyJoint( next->joint );
		next = temp;
	}

	next = swingBody->GetJointList();
	while( next != NULL )
	{
		b2JointEdge *temp = next->next;
		player->world->DestroyJoint( next->joint );
		next = temp;
	}

	swingBody->DestroyFixture( swingBody->GetFixtureList() );
	swingBody->CreateFixture( &d );
	//swingBody->destr

	b2RevoluteJointDef jd;
	jd.bodyA = anchorBody;
	jd.bodyB = swingBody;
	b2RevoluteJoint *r = (b2RevoluteJoint*)player->world->CreateJoint( &jd );
	r->SetMaxMotorTorque( 20 );
	r->EnableMotor( true );


	//b2RevoluteJointDef jd;
	jd.bodyA = player->actorParams->body;
	jd.bodyB = swingBody;
	jd.localAnchorB = b2Vec2( 0, newHeight * 2 );
	b2RevoluteJoint *r2 = (b2RevoluteJoint*)player->world->CreateJoint( &jd );
	r2->SetMaxMotorTorque( 20 );
	r2->EnableMotor( true );*/
}

void Tether::Reset()
{
	if( initialized )
	{
		st->world->DestroyJoint( anchorBody->GetJointList()->joint );
		st->world->DestroyBody( anchorBody );
		initialized = false;
	}
}


void Tether::SetMaxLength( float max )
{
	maxLen = max;
}

float32 Tether::ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float32 fraction)
{
	//^^temporary. need to filter out anything not environment most of the time. how do i do this from params?
	/*if( fixture->GetFilterData().categoryBits != 1 << rayCastLayer
		&&fixture->GetFilterData().categoryBits != 1 << CollisionLayers::EnemyPhysicsboxWithPlayer )
	{
		return -1;
	}*/

	
	if( fraction == 0 )
	{
		return 1;
	}

	if( checking )
	{
		something2 = true;
		return 0;	
	}


	if( splitting )
	{

	if( fixture->GetFilterData().categoryBits == 1 << CollisionLayers::Environment )
	{
		//closestFrac = fraction;
		b2Vec2 collision = point;
			
		b2Vec2 *tempVec = (b2Vec2*)(fixture->GetUserData());
		int tileX = tempVec->x;
		int tileY = tempVec->y;
		int localTile = st->staticLocalID[tileX][tileY];

		
		list<list<b2Vec2>> & stuff = st->staticTileSets[tileX][tileY]->tileChains[localTile];

		b2Vec2 closestPoint(-1,-1);
		for( list<list<b2Vec2>>::iterator it = stuff.begin(); it != stuff.end(); ++it )
		{
			for( list<b2Vec2>::iterator it2 = (*it).begin(); it2 != (*it).end(); ++it2 )
			{
				if( closestPoint.x < 0 )
				{
					closestPoint.Set( (*it2).x * SF2BOX + tileX, (*it2).y * SF2BOX + tileY );
				}
				else
				{
					float diffX = closestPoint.x - point.x;
					float diffY = closestPoint.y - point.y;
					float distSqr = diffX * diffX + diffY * diffY;
					
					float diffX2 = ((*it2).x * SF2BOX + tileX) - point.x;
					float diffY2 = ((*it2).y * SF2BOX + tileY) - point.y;
					float distSqr2 = diffX2 * diffX2 + diffY2 * diffY2;

					if( distSqr2 < distSqr && ( closestPoint.x != anchorPoints.back().x || closestPoint.y != anchorPoints.back()
						.y ) )
					{
						closestPoint.Set( (*it2).x * SF2BOX + tileX, (*it2).y * SF2BOX + tileY );
					}
				}
			}
		}

		//closest = closestPoint;

		closestList.push_back( pair<b2Vec2,float>( closestPoint, fraction ) );


		

		


		b2Vec2 tile = *tempVec;
		b2Vec2 norm = normal;
		//enable = stage->TileHandleRayCast( this, fixture, collision, norm );
	}

	}
	else if( fixture->GetFilterData().categoryBits == 1 << CollisionLayers::Environment )
	{
	//	cout << "ran into something" << endl;
		something = true;
	}
	

	return 1;

	/*if( enable )
	{
		b2Vec2 trueNormal = normal;
		if( isReversed ) trueNormal.y = -trueNormal.y;

		if( fraction < closestCast.fraction )
		{
			closestCast.fraction = fraction;
			closestCast.normal = trueNormal;
			closestCast.point = point;
		}
	}
	else
	{
		return -1;
	}*/
	//return 1;
}