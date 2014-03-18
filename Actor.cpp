#include "Actor.h"
#include <iostream>
#include "Action.h"
#include <sstream>

using namespace sf;
using namespace std;
using namespace luabridge;
//returns false if dead

//sf::Texture *Zero::s_texture = NULL;


ActorDef::ActorDef( const string &p_type, uint32 p_actorCount, const b2Vec2 &p_pos, const b2Vec2 &p_vel,
	bool p_facingRight, bool p_reverse, float32 p_angle, TrueActor *p_parent)
	:type( p_type ), actorCount( p_actorCount ), pos( p_pos ), vel( p_vel ), facingRight( p_facingRight ),
	 reverse( p_reverse ), angle( p_angle ), parent( p_parent )
{
}

ActorDef::ActorDef()
{
}

//^^possibly eventually need actorDir as a string parameter
PlayerChar::PlayerChar( const b2Vec2 &pos, const b2Vec2 &vel,
		bool facingRight, bool reverse, float32 angle,
		TrueActor *parent, Stage *st )
		:SingleActor( "player", pos, vel, facingRight, reverse, angle, parent, st )
{
	if( !playerShader.loadFromFile( "Resources/Actors/player/firstvertex.vert", 
		"Resources/Actors/player/firstfrag.frag" ) )
	{
		cout << "FAILED SHADER LOAD" << endl;
	}

	getGlobalNamespace( L )
		.beginNamespace( "A" )
			//.deriveClass<PlayerChar, Actor>( "PlayerChar" )
			//	.adddta

	//		.endClass()
			.beginClass<ControllerState>( "ControllerState" )
				.addData( "leftStickMagnitude", &ControllerState::leftStickMagnitude, false )
				.addData( "leftStickRadians", &ControllerState::leftStickRadians, false )
				.addData( "rightStickMagnitude", &ControllerState::rightStickMagnitude, false )
				.addData( "rightStickRadians", &ControllerState::rightStickRadians, false )
				.addData( "leftTrigger", &ControllerState::leftTrigger, false )
				.addData( "rightTrigger", &ControllerState::rightTrigger, false )
				.addData( "start", &ControllerState::start, false )
				.addData( "back", &ControllerState::back, false )
				.addData( "leftShoulder", &ControllerState::leftShoulder, false )
				.addData( "rightShoulder", &ControllerState::rightShoulder, false )
				.addData( "A", &ControllerState::A, false )
				.addData( "B", &ControllerState::B, false )
				.addData( "X", &ControllerState::X, false )
				.addData( "Y", &ControllerState::Y, false )
				.addFunction( "Up", &ControllerState::Up )
				.addFunction( "Down", &ControllerState::Down )
				.addFunction( "Left", &ControllerState::Left )
				.addFunction( "Right", &ControllerState::Right )
			.endClass()
		.endNamespace();

	push( L, &currentInput );
	lua_setglobal( L, "currentInput" );
	push( L, &prevInput );
	lua_setglobal( L, "prevInput" );
}

PlayerChar::~PlayerChar()
{
}

void PlayerChar::Draw( sf::RenderTarget *target )
{
	for( int i = spriteCount-1; i >= 0; --i )
	{
		if( actorParams->spriteIsEnabled[i] )
		{
			target->draw( *(sprite[i]), &playerShader );
		}
	}
}

b2Vec2 & PlayerChar::GetCarryVelocity()
{
	return carryVel;
}

void PlayerChar::SetCarryVelocity( float x, float y )
{
	carryVel.Set( x, y );
}

TrueActor::TrueActor( const std::string &actorType, const b2Vec2 &pos, const b2Vec2 &vel, 
	bool facingRight, bool reverse, float32 angle, TrueActor *parent, Stage*st )
	:stage( st ), parent( parent ), facingRight( facingRight ), isReversed( reverse ),
	actorType( actorType ), hitsReceivedCap( 4 ), bodyCollisionsCap( 4 ), actorsAttackedCap( 16 ), 
	isAlive( false ), isGroup(false) //yes there are 2 isAlives on single actors. oh well
{

	L = luaL_newstate();
	luaL_openlibs( L );

	string luaF =  "Resources/Actors/" + actorType + "/" + actorType + ".lua";
	int s = luaL_loadfile( L, luaF.c_str() );
	
	//cout << luaF << endl;
	if( s != 0 )
	{
		cout << "error getting file: " << luaF << endl;
	}
	//cout << "before initializing" << endl;
	getGlobalNamespace( L )
		.beginNamespace( "ACTOR" )
			.beginClass<TrueActor>( "TrueActor" )
				.addFunction( "TileSetIndex", &TrueActor::TileSetIndex )
				.addFunction( "SetSprite", &TrueActor::SetSprite )
				.addFunction( "SetVelocity", &TrueActor::SetVelocity )			
				.addFunction( "GetVelocity", &TrueActor::GetVelocity )
				//.addFunction( "SetAngularVelocity", &TrueActor::SetAngularVelocity )
				//.addFunction( "ApplyImpulse", &TrueActor::ApplyImpulse )
				.addFunction( "SetPosition", &TrueActor::SetPosition )
				.addFunction( "GetPosition", &TrueActor::GetPosition )
				.addFunction( "CreateBox", &TrueActor::CreateBox )
				.addFunction( "CreateCircle", &TrueActor::CreateCircle )
				.addFunction( "GetWorldLeft", &TrueActor::GetWorldLeft )
				.addFunction( "GetWorldRight", &TrueActor::GetWorldRight )
				.addFunction( "GetWorldTop", &TrueActor::GetWorldTop )
				.addFunction( "GetWorldBottom", &TrueActor::GetWorldBottom )
				.addFunction( "GetFriction", &TrueActor::GetFriction )
				.addFunction( "SetFriction", &TrueActor::SetFriction )
				.addFunction( "SetRestitution", &TrueActor::SetRestitution )
				.addFunction( "GetRestitution", &TrueActor::GetRestitution )
				.addFunction( "ClearHitboxes", &TrueActor::ClearHitboxes )
				.addFunction( "ClearHurtboxes", &TrueActor::ClearHurtboxes )
				.addFunction( "FaceRight", &TrueActor::FaceRight )
				.addFunction( "FaceLeft", &TrueActor::FaceLeft )
				.addFunction( "SetSpriteAngle", &TrueActor::SetSpriteAngle )
				.addFunction( "GetSpriteAngle", &TrueActor::GetSpriteAngle )
				.addFunction( "RayCast", &TrueActor::RayCast )
				.addFunction( "IsFacingRight", &TrueActor::IsFacingRight )
				.addFunction( "SetSpriteOffset", &TrueActor::SetSpriteOffset )
				.addFunction( "GetSpriteOffset", &TrueActor::GetSpriteOffset )
				.addFunction( "ClearPhysicsboxes", &TrueActor::ClearPhysicsboxes )
				.addFunction( "Message", &TrueActor::Message )
				.addFunction( "Kill", &TrueActor::Kill )
				.addFunction( "ClearDetectionboxes", &TrueActor::ClearDetectionboxes )
				.addFunction( "ClearActorsAttacked", &TrueActor::ClearActorsAttacked )
				.addFunction( "Reverse", &TrueActor::Reverse )
				.addFunction( "IsReversed", &TrueActor::IsReversed )
				.addFunction( "GetUniqueID", &TrueActor::GetUniqueID )
				.addFunction( "SetSpriteScale", &TrueActor::SetSpriteScale )
				//.addFunction( "GetSpriteScale", &TrueActor::GetSpriteScale )
				.addFunction( "SetBodyAngle", &TrueActor::SetBodyAngle )
				.addFunction( "GetBodyAngle", &TrueActor::GetBodyAngle )
				.addFunction( "SetPause", &TrueActor::SetPause )
				.addFunction( "IsPaused", &TrueActor::IsPaused )
				.addFunction( "SetSpriteEnabled", &TrueActor::SetSpriteEnabled )
				.addFunction( "SetColor", &TrueActor::SetColor )
				//.addFunction( "GetSpriteEnabled", &TrueActor::GetSpriteEnabled )
				.addData( "type", &TrueActor::actorType, false )
				.addData( "health", &TrueActor::health )
				.addData( "parent", &TrueActor::parent, false )
			.endClass()
			.deriveClass<GroupActor,TrueActor>("GroupActor")
				.addFunction( "SetIndex", &GroupActor::SetCurrentActorIndex )
				.addFunction( "GetIndex", &GroupActor::GetCurrentActorIndex )
				.addFunction( "GetActorCount", &GroupActor::GetActorCount )
				.addFunction( "SetOrigin", &GroupActor::SetOrigin )
			//	.addFunction( "IsGroup", &TrueActor::IsGroup )
				
			.endClass()
			.deriveClass<BulletActor, GroupActor>( "BulletActor" )
				.addFunction( "UpdateTrail", &BulletActor::UpdateTrail )
				.addFunction( "ClearTrail", &BulletActor::ClearTrail )
				.addFunction( "SetTrailOn", &BulletActor::SetTrailOn )
				
			.endClass()
			.deriveClass<SingleActor,TrueActor>("SingleActor")
			.endClass()
			.deriveClass<PlayerChar, SingleActor>("PlayerChar")
				.addFunction( "SetCarryVelocity", &PlayerChar::SetCarryVelocity )
				.addFunction( "GetCarryVelocity", &PlayerChar::GetCarryVelocity )
			.endClass()
			.beginClass<b2Vec2>( "b2Vec2" )
				.addConstructor<void(*)(void)>()
				.addFunction( "Normalize", &b2Vec2::Normalize )
				.addData( "x", &b2Vec2::x )
				.addData( "y", &b2Vec2::y )
			.endClass()
			.beginClass<Stage>( "Stage" )
				.addFunction( "SetCameraPosition", &Stage::SetCameraPosition )
				.addFunction( "SetCameraFollowX", &Stage::SetCameraFollowX )
				.addFunction( "SetCameraFollowY", &Stage::SetCameraFollowY )
				.addFunction( "SetCameraMaxVelocityX", &Stage::SetCameraMaxVelocityX )
				.addFunction( "SetCameraMaxVelocityY", &Stage::SetCameraMaxVelocityY )
				.addFunction( "CreateActor", &Stage::CreateActor )
				.addFunction( "CreateActorGroup", &Stage::CreateActorGroup )
				.addFunction( "CreateBulletGroup", &Stage::CreateBulletGroup )
				.addFunction( "GetCameraPosition", &Stage::GetCameraPosition )
				.addFunction( "SetCameraZoom", &Stage::SetCameraZoom )
				.addData( "player", &Stage::player )
			.endClass()
		.endNamespace();

	push( L, angle );
	lua_setglobal( L, "angle" );

	

	push<Stage*>( L, st );
	lua_setglobal( L, "stage" );

	push( L, (uint32)CollisionLayers::Event );
	lua_setglobal( L, "Layer_Event" );

	push( L, (uint32)CollisionLayers::Environment );
	lua_setglobal( L, "Layer_Environment" );

	push( L, (uint32)CollisionLayers::PlayerHitbox );
	lua_setglobal( L, "Layer_PlayerHitbox" );

	push( L, (uint32)CollisionLayers::PlayerHurtbox );
	lua_setglobal( L, "Layer_PlayerHurtbox" );

	push( L, (uint32)CollisionLayers::PlayerPhysicsbox );
	lua_setglobal( L, "Layer_PlayerPhysicsbox" );

	push( L, (uint32)CollisionLayers::EnemyHitbox );
	lua_setglobal( L, "Layer_EnemyHitbox" );

	push( L, (uint32)CollisionLayers::EnemyHurtbox );
	lua_setglobal( L, "Layer_EnemyHurtbox" );

	push( L, (uint32)CollisionLayers::EnemyPhysicsbox );
	lua_setglobal( L, "Layer_EnemyPhysicsbox" );

	push( L, (uint32)CollisionLayers::EnemyPhysicsboxWithPlayer );
	lua_setglobal( L, "Layer_EnemyPhysicsboxWithPlayer" );

	push( L, (uint32)CollisionLayers::ActorDetectionbox );
	lua_setglobal( L, "Layer_ActorDetection" );

	push( L, (uint32)CollisionLayers::PlayerDetectionbox );
	lua_setglobal( L, "Layer_PlayerDetection" );

	push( L, (uint32)CollisionLayers::PlayerEventCollisionbox );
	lua_setglobal( L, "Layer_PlayerEventCollision" );

	if( parent != NULL )
	{
		push( L, parent->GetPosition() );
		lua_setglobal( L, "parentPosOriginal" );
		push( L, parent->GetVelocity() );
		lua_setglobal( L, "parentVelOriginal" );
		push( L, parent->IsFacingRight() );
		lua_setglobal( L, "parentFacingRightOriginal" );
		//push( L, parent->upsideDown );
		//lua_setglobal( L, "parentUpsideDownOriginal" );
	}

	lua_pcall( L, 0, 0, 0 );

	lua_getglobal( L, "SpriteCount" );
	lua_pcall( L, 0, 1, 0 );
	if( !lua_isnumber( L, -1 ) )
	{ 
			assert( 0 && "must be number" );
	}
	spriteCount = lua_tonumber( L, -1 );
	lua_pop( L, 1 );

	lua_getglobal( L, "BodyType" );
	lua_pcall( L, 0, 2, 0 );
	if( !lua_isnumber( L, -1 ) )
	{
		assert( 0 && "must be number" );
	}
	bodyType = lua_tonumber( L, -1 );
	lua_pop( L, 1 );

	if( !lua_isboolean( L, -1 ) )
	{
		assert( 0 && "must be number" );
	}
	fixedAngle = lua_toboolean( L, -1 );
	lua_pop( L, 1 );

	//lua_gc( L, LUA_GCSTOP, 0 );
}

TrueActor::~TrueActor()
{
	
}

std::string & TrueActor::GetType()
{
	return actorType;
}

int TrueActor::TileSetIndex( const std::string &tileSetName )
{
	for( int i = 0; i < stage->tileSetCount; ++i )
	{
		if( tileSetName == stage->tileSets[i]->imageSource )
		{
			return i;
		}
	}
	cout << "didn't find anything " << tileSetName << endl;
}

void TrueActor::RayCast( float32 startx, float32 starty, float32 endx, float32 endy, uint32 layer )
{
	b2Vec2 start( startx, starty );
	b2Vec2 end( endx, endy );
	if( isReversed ) 
	{
		b2Vec2 diff( endx - startx, endy - starty );
		end.x = startx - diff.x;
		end.y = starty - diff.y;
	}

	closestCast.fraction = 2; //just to be greater than 1
	world->RayCast( this, start, end );
	rayCastLayer = layer;

	if( closestCast.fraction < 2 )
	{
		lua_getglobal( L, "RayCastCallback" );
		push( L, &closestCast.normal );
		push( L, &closestCast.point );
		push( L, closestCast.fraction );
		lua_pcall( L, 3, 0, 0 );
	}
}

float32 TrueActor::ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float32 fraction)
{
	//^^temporary. need to filter out anything not environment most of the time. how do i do this from params?
	if( fixture->GetFilterData().categoryBits != 1 << rayCastLayer
		&&fixture->GetFilterData().categoryBits != 1 << CollisionLayers::EnemyPhysicsboxWithPlayer )
	{
		return -1;
	}

	bool enable = true;
	if( fixture->GetFilterData().categoryBits == 1 << CollisionLayers::Environment )
	{
		b2Vec2 collision = point;
		b2Vec2 norm = normal;
		enable = stage->TileHandleRayCast( this, fixture, collision, norm );
	}

	if( enable )
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
	}
	return 1;
}

bool TrueActor::IsFacingRight()
{
	return facingRight;
}

bool TrueActor::HandleStageCollision( uint32 pointCount,b2WorldManifold &worldManifold, bool contactEnabled )
{
	lua_getglobal( L, "HandleStageCollision" );
	luabridge::push( L, pointCount );
	luabridge::push( L, &worldManifold.points[0] );
	luabridge::push( L, &worldManifold.points[1] );
	luabridge::push( L, &worldManifold.normal );
	luabridge::push( L, contactEnabled );
	
	lua_pcall( L, 5 , 1, 0 );

	if( !lua_isboolean( L, -1 ) )
	{
		assert( 0 && "needs to be boolean" );
	}
	bool enable = lua_toboolean( L, -1 );
	lua_pop( L, 1 );
	return enable;
}

void TrueActor::UpdatePostPhysics()
{
	//this is the exact same as the one in the normal actor
	lua_getglobal( L, "UpdatePostPhysics" );
	lua_pcall( L, 0, 0, 0 );
}

void TrueActor::HitActor( TrueActor *otherActor, uint32 hitboxTag,
	float32 &hitboxCenterX, uint32 &hitstun, uint32 &hitlag, int &dmg,
	std::string &hitboxType )
{

	lua_getglobal( L, "HitActor" );
	push( L, otherActor );
	push( L, hitboxTag );
	
	lua_pcall( L, 2 , 5, 0 );

	if( !lua_isnumber( L, -1 ) )
	{
		assert( 0 && "centerX needs to be a number" );
	}
	hitboxCenterX = lua_tonumber( L, -1 );
	lua_pop( L, 1 );
	if( !lua_isnumber( L, -1 ) )
	{
		assert( 0 && "hitstun needs to be a int" );
	}
	hitstun = lua_tonumber( L, -1 );
	lua_pop( L, 1 );

	if( !lua_isnumber( L, -1 ) )
	{
		assert( 0 && "hitlag needs to be a int" );
	}
	hitlag = lua_tonumber( L, -1 );
	lua_pop( L, 1 );

	if( !lua_isnumber( L, -1 ) )
	{
		assert( 0 && "damage needs to be an int" );
	}
	dmg = lua_tonumber( L, -1 );
	lua_pop( L, 1 );

	if( !lua_isstring ( L, -1 ) )
	{
		assert( 0 && "hitbox type needs to be an string" );
	}
	hitboxType = lua_tostring( L, -1 );
	lua_pop( L, 1 );
}

bool TrueActor::CollideWithActor( TrueActor *otherActor, uint32 tag, bool &isActive )
{
	lua_getglobal( L, "CollideWithActor" );
	push( L, otherActor );
	push( L, tag );
	lua_pcall( L, 2, 2, 0 );

	if( !lua_isboolean ( L, -1 ) )
	{
		assert( 0 && "needs to be a boolean" );
	}
	isActive = lua_toboolean( L, -1 );
	lua_pop( L, 1 );
	if( !lua_isboolean ( L, -1 ) )
	{
		assert( 0 && "needs to be a boolean" );
	}
	bool enable = lua_toboolean( L, -1 );
	lua_pop( L, 1 );
	return enable;
}

int TrueActor::Message( TrueActor *sender, const std::string &msg, float tag )
{
	lua_getglobal( L, "Message" );
	push( L, sender );
	lua_pushstring( L, msg.c_str() );
	lua_pushnumber( L, tag );
	lua_pcall( L, 3, 1, 0 );
	if( !lua_isnumber( L, -1 ) )
	{
		assert( 0 && "needs to be a number" );
	}
	int result = lua_tonumber( L, -1 );
	lua_pop( L, 1 );
	return result;
}

bool TrueActor::IsReversed()
{
	return isReversed;
}

