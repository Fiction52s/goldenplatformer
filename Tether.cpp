#include "Tether.h"
#include <iostream>

using namespace std;

Tether::Tether( Stage *st, b2Body *playerBody, b2World *world, float maxLength )
	:st( st ), maxLen( maxLength ), checking( false ), playerBody( playerBody ), world( world ), 
	anchored( false )
{
	maxLen = 100;
	lockedRopeLength = 100;
	locked = false;
	shotHit = false;
	shortReset = false;

	raySetting = "";

	b2BodyDef groundDef;
	//groundDef.position = pos;
	groundDef.type = b2_kinematicBody;
	//groundDef.position.y -= 6;
	anchorBody = world->CreateBody( &groundDef );

	/*b2BodyDef def;
	def.type = b2_dynamicBody;
	//def.position = playerBody->GetPosition();
	//def.linearVelocity = b2Vec2( velx, vely );
	def.fixedRotation = true;
	def.bullet = true;

	b2CircleShape cs;
	cs.m_radius = .1;

	b2FixtureDef fd;
	fd.shape = &cs;
	CollisionLayers::SetupFixture( CollisionLayers::TetherShot, fd.filter.categoryBits, fd.filter.maskBits );

	shotBody = world->CreateBody( &def );
	shotBody->CreateFixture( &fd );
	shotBody->SetActive( false );*/
}

Tether::~Tether()
{
	
}

void Tether::Anchor()
{
	assert( !anchored );

	anchored = true;
	locked = false;

	anchorPoints.push_back( shotHitPoint );

	anchorBody->SetTransform( shotHitPoint, 0 );

	//shotBody->SetActive( false );
	shotActive = false;
	
	lockedRopeLength = maxLen;

	b2RopeJointDef rjd;
	rjd.maxLength = maxLen;
	rjd.bodyA = playerBody;
	rjd.bodyB = anchorBody;
	rjd.localAnchorA.Set( 1, 0 );
	b2RopeJoint *rj = (b2RopeJoint*)world->CreateJoint( &rjd );

	//Lock( st->player );
}

bool Tether::CheckTetherShot()
{
	hitSomethingChecking = false;

	
	raySetting = "checking";
	world->RayCast( this, shotPos, st->player->GetPosition() );

	

	return hitSomethingChecking;
}

void Tether::Split( PlayerChar *player, b2Vec2 splitPoint )
{
	anchorPoints.push_back( splitPoint );
	anchorBody->SetTransform( splitPoint, 0 );

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

	if( totalDist < lockedRopeLength )
	{
		rj->SetMaxLength( lockedRopeLength - totalDist );
	}
	else
	{
		rj->SetMaxLength( 1 );
		cout << "rope is too long" << endl;	
	}
}

void Tether::Lock( PlayerChar *player )
{
	//assert( !locked );

	float totalDist = 0;

	if( anchorPoints.size() > 1 )
	{
		for( list<b2Vec2>::iterator anchorIt = anchorPoints.begin(); anchorIt != anchorPoints.end(); )
		{
			list<b2Vec2>::iterator past = anchorIt;
			anchorIt++;
			if( anchorIt == anchorPoints.end() )
				break;
			float diffX = (*past).x - (*anchorIt).x;
			float diffY = (*past).y - (*anchorIt).y;
			totalDist += sqrt( diffX * diffX + diffY * diffY );
		}
	}

	float dx = player->GetPosition().x - anchorPoints.back().x;
	float dy = player->GetPosition().y - anchorPoints.back().y;
	float distSqr = dx * dx + dy * dy;
	float extraDist = sqrt( distSqr );
	//totalDist += sqrt( distSqr );

	lockedRopeLength = totalDist + extraDist;

	b2RopeJoint *rj = (b2RopeJoint*)anchorBody->GetJointList()->joint;

	rj->SetMaxLength( extraDist );

	locked = true;
}

void Tether::Update( PlayerChar *player )
{
	//b2World *world = anchorBody->GetWorld();

	if( shortReset )
	{
		shortReset = false;
		Reset();
		return;
	}

	

	if( anchored )
	{
		raySetting = "splitting";
		world->RayCast( this, anchorPoints.back(), player->GetPosition() );
	
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

			cout << "SPLITTING THING" << endl;
			Split( player, minVec );
		}

		if( anchorPoints.size() > 1 )
		{
			list<b2Vec2>::reverse_iterator rIt = anchorPoints.rbegin(); 
			rIt++;	//second from the last

			hitSomething = false;

			raySetting = "joining";
			world->RayCast( this, (*rIt), player->GetPosition() );	

			int rayCastNum = 50;
			b2Vec2 blah( player->GetPosition().x - anchorPoints.back().x, player->GetPosition().y - anchorPoints.back().y );
			for( int b = 1; b < rayCastNum; ++b )
			{
				float b2 = (float)b / rayCastNum;
				b2Vec2 bbb( blah );
				bbb.x *= b2;
				bbb.y *= b2;
				world->RayCast( this, (*rIt), anchorPoints.back() + bbb );
			}

			if( !hitSomething )
			{
				cout << "JOINING THING" << endl;
				Join( player );
			}
		}
	}
	else
	{
		if( shotActive )
		{
			b2Vec2 oldShotPos = shotPos;
			shotPos += b2Vec2( shotVelocity.x / 60.f, shotVelocity.y / 60.f );

			raySetting = "shotCheck";
			world->RayCast( this, player->GetPosition(), oldShotPos );	
			if( shotCheckFlag )
			{
				//shotActive = false;
				shotCheckFlag = false;
				Reset();
				return;
				
				//tether hit an earlier point. cancel it. 

			}

			
			raySetting = "shotAnchor";
			world->RayCast( this, oldShotPos, shotPos );
			int closestListSize = closestList.size();
			b2Vec2 minVec( -1, -1 );

			if( closestListSize > 1 )
			{
				float min = 1.1;
				
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
			}
			else if( closestListSize == 1 )
			{
				minVec = closestList.front().first;
			}
			else
			{
				//assert( 0 && "error: 1" );
				return;
			}

			shotHitPoint = minVec;
			Anchor();
		}
	}
}

void Tether::Join( PlayerChar* player )
{
	anchorPoints.pop_back();

	anchorBody->SetTransform( anchorPoints.back(), 0 );
		
	float totalDist = 0;
	for( list<b2Vec2>::iterator anchorIt = anchorPoints.begin(); anchorIt != anchorPoints.end(); )
	{
		list<b2Vec2>::iterator past = anchorIt;
		anchorIt++;
		if( anchorIt == anchorPoints.end() )
			break;
		float diffX = (*past).x - (*anchorIt).x;
		float diffY = (*past).y - (*anchorIt).y;
		totalDist += sqrt( diffX * diffX + diffY * diffY );
	}

	b2RopeJoint *rj = (b2RopeJoint*)anchorBody->GetJointList()->joint;
	float xx = anchorPoints.back().x - closest.x;
	float yy = anchorPoints.back().y - closest.y;

	if( totalDist < lockedRopeLength )
	{
		rj->SetMaxLength( lockedRopeLength - totalDist );
	}
	else
	{
		rj->SetMaxLength( 1 );
		cout << "rope is too long" << endl;	
	}
}

void Tether::Shot( PlayerChar *player, const b2Vec2 &vel )
{
	//assert( "shot active should not be true here" && shotActive );
	shotActive = true;
	shotPos = player->GetPosition();
	shotVelocity = vel;
	cout << "end of shot" << endl;
	//shotBody->SetActive( true );
	//shotBody->SetTransform( player->GetPosition(), 0 );
	//shotBody->SetLinearVelocity( vel );
}

void Tether::Reset()
{
	if( anchored )
	{
		st->world->DestroyJoint( anchorBody->GetJointList()->joint );
		//st->world->DestroyBody( anchorBody );
		anchorPoints.clear();
		anchored = false;
		locked = false;
	}
	else
	{
		shotActive = false;
		//shotBody->SetActive( false );
	}
}

void Tether::SetMaxLength( float max )
{
	/*float dx = player->GetPosition().x - anchorPoints.back().x;
	float dy = player->GetPosition().y - anchorPoints.back().y;
	float distSqr = dx * dx + dy * dy;
	float extraDist = sqrt( distSqr );

	
	lockedRopeLength = max;

	//maxLen = max;
	b2RopeJoint* rj = (b2RopeJoint*)anchorBody->GetJointList()->joint;
	rj->SetMaxLength( max );*/
}

void Tether::Grow( float amount )
{
	b2RopeJoint* rj = (b2RopeJoint*)anchorBody->GetJointList()->joint;
	rj->SetMaxLength( rj->GetMaxLength() + amount );

	lockedRopeLength += amount;
}

void Tether::Shrink( float amount )
{
	b2RopeJoint* rj = (b2RopeJoint*)anchorBody->GetJointList()->joint;

	if( rj->GetMaxLength() - amount < 1 )
	{
		if( anchorPoints.size() > 1 )
		{
			//cout << "joining because lockedropelength = " << lockedRopeLength << " and amount= " << amount << " and size= " << anchorPoints.size() << endl;
			//Join( st->player );

			//rj->SetMaxLength( rj->GetMaxLength() - amount );
			//lockedRopeLength -= amount;
			float temp = rj->GetMaxLength() - amount;
			if( temp < 1 )
			{
				//shortReset = true;
				//return;
				//Reset();
				//return;
			
				
				rj->SetMaxLength( 1 );
				lockedRopeLength -= temp;	
			}
			
			
		}
		else
		{
			cout << "too short nothing left" << endl;
		}
	}
	else
	{
		rj->SetMaxLength( rj->GetMaxLength() - amount );
		lockedRopeLength -= amount;
	}

	b2Vec2 dir( anchorPoints.back().x - st->player->GetPosition().x, 
		anchorPoints.back().y - st->player->GetPosition().y );
	dir.Normalize();

	st->player->SetVelocity( st->player->GetVelocity().x + dir.x * amount * 1.f / 1, st->player->GetVelocity().y + dir.y * amount * 1.f / 1);
	cout << "adding vel x: " << dir.x * amount << ", y: " << dir.y * amount << endl;
	//st->player->GetBody()->ApplyLinearImpulse( amount * 100 * dir, b2Vec2( 0, 0 ) );
//	st->player->GetBody()->ApplyForceToCenter( amount * 100 * dir );
	//st->player->SetPosition( st->player->GetPosition() + 
}

std::string Tether::GetState()
{
	if( shotActive )
	{
		return "shot";
	}
	
	if( locked )
	{
		return "locked";
	}

	if( anchored )
	{
		return "anchored";
	}

	return "dormant";
}

float32 Tether::ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float32 fraction)
{
	//^^temporary. need to filter out anything not environment most of the time. how do i do this from params?
	/*if( fixture->GetFilterData().categoryBits != 1 << rayCastLayer
		&&fixture->GetFilterData().categoryBits != 1 << CollisionLayers::EnemyPhysicsboxWithPlayer )
	{
		return -1;
	}*/

	
	if( fraction == 0 || fixture->GetFilterData().categoryBits != 1 << CollisionLayers::Environment )
	{
		return 1;
	}

	/*if( checking && fixture->GetFilterData().categoryBits == 1 << CollisionLayers::Environment )
	{
		hitSomethingChecking = true;
		return 0;	
	}
	else if( checking )
	{
		return 1;
	}*/


	if( raySetting == "splitting" || raySetting == "shotAnchor" )
	{
		//closestFrac = fraction;
		b2Vec2 collision = point;
			
		b2Vec2 *tempVec = (b2Vec2*)(fixture->GetUserData());
		int tileX = tempVec->x;
		int tileY = tempVec->y;
		int localTile = st->staticLocalID[tileX][tileY];

		
		list<list<b2Vec2>> & stuff = st->staticTileSets[tileX][tileY]->tileChains[localTile];

		closestList.clear();
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
		closestList.push_back( pair<b2Vec2,float>( closestPoint, fraction ) );

		b2Vec2 tile = *tempVec;
		b2Vec2 norm = normal;
	}
	else if( raySetting == "shotCheck" )
	{
		shotCheckFlag = true;
	}
	else if( raySetting == "joining" )
	{
		//cout << "ran into something" << endl;
		hitSomething = true;
	}
	else if( raySetting == "checking" )
	{
		hitSomethingChecking = true;
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