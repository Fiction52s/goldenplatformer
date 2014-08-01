#ifndef __ACTOR_H__
#define __ACTOR_H__
#include <SFML/Graphics.hpp>
#include "Input.h"
#include "Box2D.h"
#include "Stage.h"
#include "Globals.h"
#include <list>
#include "Collision.h"

struct ActorHit
{
	TrueActor *actor;
	b2Body *otherBody;
	b2Body *ownBody;// for group actors
	std::string hitboxName;
	int damage;
	uint32 hitlag;
	uint32 hitstun;
	uint32 hurtboxTag; //the tag on your hurtbox
	float32 centerX; //local to actor
};

struct ActorHitOffense
{
	TrueActor *actor;
	uint32 hitboxTag;
};

struct ActorCollision
{
	TrueActor *actor;
	uint32 hurtboxTag;
	b2Vec2 normal;
	b2Vec2 point1;
	b2Vec2 point2;
	uint32 pointCount;
	bool enabled;
};

struct RayCastInfo
{
	b2Vec2 normal;
	b2Vec2 point;
	float32 fraction;
};



/*struct Actor : public b2RayCastCallback
{
	RayCastInfo closestCast;
	Actor( const std::string &actorType,
		const b2Vec2 &pos, const b2Vec2 &vel,
		bool facingRight, bool reverse, float32 angle,
		Actor *parent, Stage *st );
	virtual ~Actor();
	void Init( b2World *p_world );
	bool UpdatePrePhysics();
	void UpdatePostPhysics();
	bool ProcessCollisions();
	void Draw( sf::RenderTarget *target );
	void UpdateSprites();
	void SetSpriteScale( uint32 index, float x, float y);
	void SetBodyAngle( float angle );
	float GetBodyAngle();

	std::string &GetType();

	void SetSpriteOffset( uint32 index, float x, float y );
	b2Vec2 & GetSpriteOffset( uint32 index );

	//private
	void SetSprite( uint32 index, uint32 tileSetIndex,
		uint32 localID );
	int TileSetIndex( 
		const std::string &tileSetName );
	void SetVelocity( float x, float y );

	const b2Vec2 &GetVelocity();
	bool HandleStageCollision( uint32 pointCount,
		b2WorldManifold &worldManifold, 
		bool contactEnabled );
	void HitActor( b2Body *body, 
		Actor *otherActor, uint32 hitboxTag,
		float32 &hitboxCenterX,
		uint32 &hitstun,
		uint32 &hitlag,
		int &dmg,
		std::string &hitboxType );
	bool CollideWithActor( b2Body *myBody, 
		Actor *otherActor, uint32 tag, bool &isActive );

	
	void SetPosition( float x, float y );
	b2Vec2 GetPosition();
	void ApplyImpulse( float x, float y );
	void ClearHitboxes();
	void ClearHurtboxes();
	void ClearPhysicsboxes();
	void ClearDetectionboxes();
	void CreateBox( uint32 tag,
		int layer, float32 offsetX, 
		float32 offsetY, float32 width, 
		float32 height, float32 angle );
	void CreateCircle( uint32 tag, int layer, 
		float32 offsetX, float32 offsetY, 
		float32 radius );
	void FaceRight();
	void FaceLeft();
	void SetSpriteAngle( uint32 index, float32 angle );
	float32 GetSpriteAngle( uint32 index );
	
	void SetSpriteEnabled( uint32 index, bool enabled );
	bool GetSpriteEnabled( uint32 index );
	
	float GetWorldLeft();
	uint32 GetUniqueID();
	float GetWorldRight();
	float GetWorldTop();
	float GetWorldBottom();
	void SetFriction( float friction );
	float32 GetFriction();
	void Kill();
	void Reverse();
	bool IsReversed();
	

	float32 ReportFixture(b2Fixture* fixture, 
		const b2Vec2& point, const b2Vec2& normal, 
		float32 fraction);

	void RayCast( float32 startx, float32 starty, 
		float32 endx, float32 endy, uint32 layer );

	bool IsFacingRight();

	int Message( Actor *sender,
		std::string msg, float tag );
	void SetPause( bool pause );
	bool IsPaused();
	void ClearActorsAttacked();

	sf::Sprite **sprite;
	uint32 spriteCount;
	bool facingRight;
	std::string actorType;
	int health;
	b2World *world;
	b2Body *m_body;
	b2Vec2 m_startPosition;
	Stage *stage;
	Actor *parent;
	b2BodyDef def;
	b2Vec2 *spriteOffset;

	b2Vec2 adjustedVelocity;
	float32 m_friction;
	//int currentHitboxDamage;
	float32 *m_spriteAngle;
	bool m_paused;
	bool isAlive;
	bool isReversed;
	bool *spriteIsEnabled;
	//void SetSpriteOrigin( float x, float y );
	//sf::Vector2f GetSpriteOrigin();
	//float GetSpriteWidth();
	//float GetSpriteHeight();
	lua_State *L;
	//std::string &GetAttackType( uint32 i );
	b2AABB aabb;
	//std::string currentHitboxType;
	//uint32 currentHitlag;
	//void SetHitboxDamage( int dmg );
	//int GetHitboxDamage();

	//void SetHitboxType( const std::string &type );
	//const std::string & GetHitboxType();
	//void SetHitlag( uint32 lag );
	//uint32 GetHitlag();
	ActorHit *GetHitsReceived();
	uint32 hitsReceivedCap;
	uint32 GetHitsReceivedSize();
	ActorCollision *GetBodyCollisions();
	uint32 bodyCollisionsCap;
	uint32 GetBodyCollisionsSize();
	uint32 rayCastLayer;
	ActorHitOffense *GetActorsAttacked();
	uint32 actorsAttackedCap;
	uint32 GetActorsAttackedSize();
	uint32 GetActorsAttackedIndex(); 
	
	//in case you need more than your cap, start
	//back at the beginning
};*/

struct Squad;
struct ActorDef
{
	ActorDef( const std::string &p_type, uint32 p_actorCount, 
		const b2Vec2 &p_pos, const b2Vec2 &p_vel,
		bool p_facingRight, bool p_reverse, float32 p_angle, 
		TrueActor *p_parent);
	ActorDef();
	std::string type;
	uint32 actorCount;
	b2Vec2 pos;
	b2Vec2 vel;
	bool facingRight;
	bool reverse;
	float32 angle;
	TrueActor *parent;
	std::list<std::pair<std::string, 
		float32>> preInitMessages;
	Squad *squad;
	Room *room;
};

struct TrueActor;
struct ActorParams
{
	ActorParams( int health, bool facingRight, 
		bool reversed, const b2Vec2 &spawnPoint,
		const b2Vec2 &vel,
		float32 angle, uint32 spriteCount, 
		uint32 hitsReceivedCap, 
		uint32 bodyCollisionsCap,
		uint32 actorsAttackedCap, TrueActor *owner );
	~ActorParams();
	int health;
	TrueActor *owner;
	void Init();
	void UpdateBoundingBoxes();
	void CreateBox( uint32 tag, int layer, 
		float32 offsetX, float32 offsetY, float32 width, 
		float32 height, float32 angle );
	void CreateCircle( uint32 tag, int layer, 
		float32 offsetX, float32 offsetY, 
		float32 radius );
	void SetVelocity( float x, float y );
	const b2Vec2 &GetVelocity();	
	void SetPosition( float x, float y );
	b2Vec2 GetPosition();
	void ApplyImpulse( float x, float y );
	void SetBodyAngle( float angle );
	float GetBodyAngle();
	uint32 GetUniqueID();
	void SetFriction( float friction );
	float32 GetFriction();
	void SetRestitution( float restitution );
	float32 GetRestitution();
	void SetPause( bool pause );
	bool IsPaused();
	void ClearActorsAttacked();
	void Kill();
	void SetSpriteOffset( uint32 spriteIndex,
		float x, float y );
	void ClearHitboxes();
	void ClearHurtboxes();
	void ClearPhysicsboxes();
	void ClearDetectionboxes();

	b2Vec2 adjustedVelocity; //just a temp in the cycle
	b2Body *body;
	b2Vec2 spawnPoint;
	
	b2BodyDef def; //do i need this?
	b2Vec2 *spriteOffset;

	

	b2AABB aabb;
	bool isAlive;
	
	float32 m_friction;
	float32 m_restitution;
	//int currentHitboxDamage;
	bool m_paused;

	ActorHit *hitsReceived;
	uint32 hitsReceivedSize;
	ActorCollision *bodyCollisions;
	uint32 bodyCollisionsSize;
	ActorHitOffense *actorsAttacked;
	uint32 actorsAttackedSize;
	uint32 actorsAttackedIndex; 
	bool *spriteIsEnabled;

	
	bool save_isAlive;
	float32 save_friction;
	float32 save_restitution;
	bool save_paused;
	ActorHit *save_hitsReceived;
	uint32 save_hitsReceivedSize;
	ActorCollision *save_bodyCollisions;
	uint32 save_bodyCollisionsSize;
	ActorHitOffense *save_actorsAttacked;
	uint32 save_actorsAttackedSize;
	uint32 save_actorsAttackedIndex;
	bool *save_spriteIsEnabled;
	b2Vec2 *save_spriteOffset;
	b2Vec2 save_position;
	b2Vec2 save_velocity;
	float32 save_angle;
	std::list<b2FixtureDef> save_fixtureDefs;


	void SaveState();
	void LoadState();
	//std::string currentHitboxType;
	//uint32 currentHitlag;
};

struct TrueActor : public b2RayCastCallback //change this later
{
	TrueActor( const std::string &actorType,
		const b2Vec2 &pos, const b2Vec2 &vel,
		bool facingRight, bool reverse, float32 angle,
		TrueActor *parent, Stage*st );

	virtual void Init( b2World *world ) = 0;

	virtual ~TrueActor();
	
	virtual b2Body * GetBody() = 0;

	virtual void FaceLeft() = 0;

	virtual void FaceRight() = 0;

	virtual void SetColor( uint32 spriteIndex, uint8 r, uint8 g, uint8 b, uint8 a ) = 0;

	int Message( TrueActor *sender, const std::string &msg, 
		float tag );

	virtual void Draw( sf::RenderTarget *target,
		uint32 spriteIndex ) = 0;

	virtual void CloneDraw( sf::RenderTarget *target ) = 0;

	virtual void ClearHitboxes() = 0;

	virtual void ClearHurtboxes() = 0;

	virtual void ClearPhysicsboxes() = 0;

	virtual void ClearDetectionboxes() = 0;

	virtual void CreateBox( uint32 tag, int layer, 
		float32 offsetX, float32 offsetY, 
		float32 width, float32 height, 
		float32 angle ) = 0;

	virtual void CreateCircle( uint32 tag, int layer, 
	float32 offsetX, float32 offsetY, 
	float32 radius ) = 0;

	virtual void SetSprite( uint32 spriteIndex,
		uint32 tsIndex, uint32 localID ) = 0;

	virtual bool UpdatePrePhysics() = 0;

	virtual void UpdatePostPhysics();

	virtual bool ProcessCollisions() = 0;

	std::string &GetType();

	int TileSetIndex( const std::string &tileSetName );

	void RayCast( float32 startx, float32 starty, 
		float32 endx, float32 endy, uint32 layer );

	float32 ReportFixture( b2Fixture* fixture, 
		const b2Vec2& point, const b2Vec2& normal, 
		float32 fraction);

	bool IsFacingRight();

	bool IsReversed();

	bool HandleStageCollision( uint32 pointCount,
		b2WorldManifold &worldManifold, 
		bool contactEnabled );

	virtual void UpdateSprites() = 0;

	virtual void SetVelocity( float x, float y ) = 0;

	virtual const b2Vec2 &GetVelocity() = 0;

	virtual void SetPosition( float x, float y ) = 0;

	virtual b2Vec2 GetPosition() = 0;

	void HitActor( TrueActor *otherActor, 
		uint32 hitboxTag, float32 &hitboxCenterX, 
		uint32 &hitstun, uint32 &hitlag, int &dmg,
		std::string &hitboxType );

	bool CollideWithActor( TrueActor *otherActor, 
		uint32 tag, bool &isActive );

	virtual void SetSpriteScale( uint32 index, 
		float x, float y ) = 0;

	virtual void SetBodyAngle( float angle ) = 0;

	virtual float GetBodyAngle() = 0;

	virtual void SetSpriteOffset( uint32 spriteIndex,
		float x, float y ) = 0;

	virtual b2Vec2 & GetSpriteOffset( uint32 spriteIndex ) = 0;

	virtual void SetSpriteAngle( uint32 index, 
		float32 angle ) = 0;

	virtual float32 GetSpriteAngle( uint32 index ) = 0;

	virtual void SetSpriteEnabled( uint32 spriteIndex,
		bool enabled ) = 0;

	virtual uint32 GetUniqueID() = 0;

	virtual float GetWorldLeft() = 0;

	virtual float GetWorldRight() = 0;

	virtual float GetWorldTop() = 0;

	virtual float GetWorldBottom() = 0;

	virtual void SetFriction( float friction ) = 0;

	virtual float32 GetFriction() = 0;

	virtual void SetRestitution( float restitution ) = 0;

	virtual float32 GetRestitution() = 0;

	virtual void Kill() = 0;

	virtual void Reverse() = 0;

	virtual void SetPause( bool pause ) = 0;

	virtual bool IsPaused() = 0;

	virtual void ClearActorsAttacked() = 0;

	virtual ActorHit * & GetHitsReceived() = 0;

	virtual uint32 & GetHitsReceivedSize() = 0;

	virtual ActorCollision * & GetBodyCollisions() = 0;

	virtual uint32 & GetBodyCollisionsSize() = 0;

	virtual ActorHitOffense * & GetActorsAttacked() = 0;

	virtual uint32 & GetActorsAttackedSize() = 0;

	virtual uint32 & GetActorsAttackedIndex() = 0;

	void SetSpritePriority( uint32 spriteIndex, int32 priority );

	virtual sf::FloatRect GetSpriteAABB() = 0;

	int bodyType;
	bool fixedAngle;
	float restitution;
	float density;
	std::string actorType;
	b2World *world;
	Stage *stage;
	TrueActor *parent;
	lua_State *L;
	uint32 spriteCount;
	int32 *spritePriority;


	RayCastInfo closestCast; //just a temp in the cycle
	
	uint32 hitsReceivedCap;
	uint32 bodyCollisionsCap;
	uint32 rayCastLayer;
	uint32 actorsAttackedCap;
	bool facingRight;
	bool isAlive;
	bool isReversed;
	bool isGroup;
	uint32 health;

	float save_restitution;
	float save_density;
	bool save_facingRight;
	bool save_isAlive;
	bool save_isReversed;
	uint32 save_health;
	
	Squad *squad;
	Room *room;

	virtual void SaveState();
	virtual void LoadState();
};

/*struct ActorBody
{
	void CreateBox( uint32 tag, int layer, 
		float32 offsetX, float32 offsetY, 
		float32 width, float32 height, 
		float32 angle );
	void CreateCircle( uint32 tag, int layer, 
		float32 offsetX, float32 offsetY, 
		float32 radius );
	void SetVelocity( float x, float y );
	const b2Vec2 &GetVelocity();	
	void SetPosition( float x, float y );
	const b2Vec2 &GetPosition();
	void SetAngle();
	float GetAngle();
	void SetAngularVelocity();
	void ClearFixtures( int layer );
	void ClearAllFixtures();
	void SaveState();
	void LoadState();
	void SetFriction();
	float GetFriction();
	void SetRestitution();
	float GetResitution();

	float m_friction;
	float m_restitution;


};*/

struct SingleActor : public TrueActor
{
	sf::Sprite **sprite;
	float32 *m_spriteAngle;
	ActorParams *actorParams;
	bool *spriteIsEnabled;

	//clone stuff
	sf::Sprite **save_sprite;
	sf::Color *save_color;
	float32 *save_spriteAngle;
	//end clone stuff


	SingleActor( const std::string &actorType,
		const b2Vec2 &pos, const b2Vec2 &vel,
		bool facingRight, bool reverse, float32 angle,
		TrueActor *parent, Stage *st );

	virtual ~SingleActor();

	virtual void Init( b2World *world );

	b2Body * GetBody();

	void SetSpriteScale( uint32 index, float x, float y);

	void SetColor( uint32 spriteIndex, uint8 r, uint8 g, uint8 b, uint8 a );

	void FaceLeft();

	void FaceRight();

	float GetBodyAngle();

	virtual void Draw( sf::RenderTarget *target, 
		uint32 spriteIndex );

	virtual void CloneDraw( sf::RenderTarget *target );

	void ClearHitboxes();

	void ClearHurtboxes();

	void ClearPhysicsboxes();

	void ClearDetectionboxes();

	virtual void CreateBox( uint32 tag, int layer, 
		float32 offsetX, float32 offsetY, 
		float32 width, float32 height, 
		float32 angle );

	virtual void CreateCircle( uint32 tag, int layer, 
		float32 offsetX, float32 offsetY, 
		float32 radius );

	void SetSprite( uint32 spriteIndex,
		uint32 tsIndex, uint32 localID );

	virtual bool UpdatePrePhysics();

	bool ProcessCollisions();

	void UpdateSprites();

	void SetVelocity( float x, float y );

	const b2Vec2 &GetVelocity();

	void SetPosition( float x, float y );

	b2Vec2 GetPosition();

	void SetBodyAngle( float angle );

	void SetSpriteOffset( uint32 spriteIndex,
		float x, float y );

	b2Vec2 & GetSpriteOffset( uint32 spriteIndex );

	void SetSpriteAngle( uint32 index, 
		float32 angle );

	float32 GetSpriteAngle( uint32 index );

	void SetSpriteEnabled( uint32 spriteIndex, 
		bool enabled );

	uint32 GetUniqueID();

	float GetWorldLeft();

	float GetWorldRight();

	float GetWorldTop();

	float GetWorldBottom();

	void SetFriction( float friction );

	float32 GetFriction();

	void SetRestitution( float restitution );

	float32 GetRestitution();

	void Kill();

	void Reverse();

	void SetPause( bool pause );

	bool IsPaused();

	void ClearActorsAttacked();

	ActorHit * & GetHitsReceived();

	uint32 & GetHitsReceivedSize();

	ActorCollision * & GetBodyCollisions();

	uint32 & GetBodyCollisionsSize();

	ActorHitOffense * & GetActorsAttacked();

	uint32 & GetActorsAttackedSize();

	uint32 & GetActorsAttackedIndex();

	sf::FloatRect GetSpriteAABB();

	virtual void SaveState();
	virtual void LoadState();


};

struct HitboxInfo
{
	HitboxInfo(bool circle, uint32 tag, 
		float32 offsetX, float32 offsetY, 
		float32 width, float32 height,
		float32 angle);
	
	bool circle; //otherwise its a box
	uint32 tag;
	float32 offsetX;
	float32 offsetY;
	float32 width;
	float32 height;
	float32 angle;
};

struct PlayerGhost
{
	PlayerGhost( Stage *stage, 
		PlayerChar *player);
	void CreateBox( uint32 tag, float32 offsetX, 
		float32 offsetY, float32 width, 
		float32 height, float32 angle );
	void CreateCircle( uint32 tag, 
		float32 offsetX, float32 offsetY, 
		float32 radius );
	std::list< std::pair< uint32, 
		std::list<HitboxInfo>> > hitboxes;
	std::list<sf::Sprite> sprites;
	std::list<b2Vec2> position;
	uint32 recordFrame;
	uint32 playFrame;
	b2Body* body;
	PlayerChar *player;
	uint32 hitlagFrames;
};

/*struct NewActor : public TrueActor
{
	NewActor( const b2Vec2 &pos, const b2Vec2 &vel,
		bool facingRight, bool reverse, float32 angle,
		TrueActor *parent, Stage *st );

}*/

struct Tether;
struct PlayerChar: public SingleActor
{
	PlayerChar( const b2Vec2 &pos, const b2Vec2 &vel,
		bool facingRight, bool reverse, float32 angle,
		TrueActor *parent, Stage *st );
	b2Vec2 & GetCarryVelocity();
	void SetCarryVelocity( float x, float y);
	virtual ~PlayerChar();
	void Draw( sf::RenderTarget *target,
		uint32 spriteIndex );
	virtual void CreateBox( uint32 tag, int layer, 
		float32 offsetX, float32 offsetY, 
		float32 width, float32 height, 
		float32 angle );

	bool UpdatePrePhysics();
	void Init( b2World *world );
	virtual void CreateCircle( uint32 tag, int layer, 
		float32 offsetX, float32 offsetY, 
		float32 radius );
	void SetGhostHitlag( uint32 index,
		uint32 hitlagFrames );
	void CreateTether( float posX, float posY, float maxLength, bool left );
	void ReleaseTether( bool left);
	void MaxTetherLength( bool left );
	void TetherShot(float shotSpeed, bool left);

	void GrowTether( float amount, bool left );
	void ShrinkTether( float amount, bool left );
	//virtual void CloneDraw( sf::RenderTarget *target );
	ControllerState currentInput;
	ControllerState prevInput;
	sf::Shader playerShader;
	sf::Shader cloneShader;
	b2Vec2 carryVel;

	void LockTether( bool left );
	std::string GetTetherState( bool left );

	bool tetherAim;

	void SetTetherAim( bool on );
	void SetStoredRadians( double rads );
	double storedRadians;

	Tether * leftTether;
	b2Body *leftTetherShotBody;
	b2Vec2 leftTetherPoint;
	//bool LeftTetherActive();

	Tether * rightTether;
	b2Body *rightTetherShotBody;
	bool rightTetherHit;
	b2Vec2 rightTetherPoint;
	//bool RightTetherActive();

	bool leftTetherHit;
	
	b2Vec2 tetherGoal;

	b2Vec2 save_carryVel;

	PlayerGhost **ghosts;
	uint32 ghostCount;
	uint32 maxGhostCount;
	uint8 ghostVisibility;

	uint32 hitlagFrames;

	bool dropThroughFlag;
	bool cancelDropFlag;

	b2Body * speedBallBody;
	void SetSpeedBallEnabled( bool on );
	bool IsSpeedBallEnabled();
	b2Vec2 GetSpeedBallPos();
	b2Vec2 GetSpeedBallVel();

	//auto drops if it isn't stopped. this is to prevent
	//dropping animation when on both a dropping and non dropping tile

	virtual void SaveState();
	virtual void LoadState();
	
};



struct GroupActor : public TrueActor
{
	sf::VertexArray *vertexArray;
	sf::Transform *transform;
	uint32 actorCount;
	uint32 tileSetIndex;
	ActorParams **actorParams;
	uint32 actorIndex;
	sf::Vector2f origin;
	float *spriteAngle;
	sf::Vector2f *spriteScale;
	GroupActor( const std::string &actorType,
		uint32 p_actorCount,
		const b2Vec2 &pos, const b2Vec2 &vel,
		bool facingRight, bool reverse, float32 angle,
		TrueActor *parent, Stage *st );

	virtual ~GroupActor();
	void SetColor( uint32 spriteIndex, uint8 r, uint8 g, uint8 b, uint8 a );
	bool CollideWithActor( TrueActor *otherActor, 
		uint32 tag, bool &isActive );
	
	b2Body * GetBody();

	int GetBodyIndex( b2Body *myBody );

	uint32 GetActorCount();

	bool HandleStageCollision( uint32 pointCount,
		b2WorldManifold &worldManifold, 
		bool contactEnabled );

	void HitActor( TrueActor *otherActor, 
		uint32 hitboxTag, float32 &hitboxCenterX, 
		uint32 &hitstun, uint32 &hitlag, int &dmg,
		std::string &hitboxType );

	//void SetTileSet( uint32 tsIndex );

	void SetSpriteEnabled( int spriteIndex, 
		bool enabled );

	void SetSprite( uint32 spriteIndex, 
		uint32 tileSetIndex, uint32 localID );
	
	void SetSpriteScale( uint32 index, float x, float y);

	virtual void Init( b2World *p_world );

	bool UpdatePrePhysics();

	float GetBodyAngle();

	bool ProcessCollisions();

	void SetOrigin( float x, float y );

	virtual void Draw( sf::RenderTarget *target,
		uint32 spriteIndex );

	virtual void CloneDraw( sf::RenderTarget *target );

	void UpdateSprites();

	void SetBodyAngle( float angle );

	float GetBodyAngle( uint32 actorIndex );

	void SetSpriteOffset( uint32 spriteIndex, float x, float y );

	b2Vec2 & GetSpriteOffset( uint32 spriteIndex );

	void SetVelocity( float x, 
		float y );

	const b2Vec2 &GetVelocity();

	void SetPosition( float x, float y );

	b2Vec2 GetPosition();

//	void ApplyImpulse( uint32 actorIndex, float x, float y );

	void ClearHitboxes();

	void ClearHurtboxes();

	void ClearPhysicsboxes();

	void ClearDetectionboxes();

	void CreateBox( uint32 tag,
		int layer, float32 offsetX, 
		float32 offsetY, float32 width, 
		float32 height, float32 angle );

	void CreateCircle( uint32 tag, 
		int layer, float32 offsetX, float32 offsetY, 
		float32 radius );

	void FaceRight();

	void FaceLeft();

	void SetSpriteAngle( uint32 index, float32 angle );

	float32 GetSpriteAngle( uint32 index );

	void SetSpriteEnabled( uint32 spriteIndex,
		bool enabled );

	uint32 GetUniqueID();

	float GetWorldLeft();
	
	float GetWorldRight();

	float GetWorldTop();

	float GetWorldBottom();

	void SetFriction( float friction );

	float32 GetFriction();

	void SetRestitution( float restitution );

	float32 GetRestitution();

	void Kill();

	void Reverse();

	bool IsReversed();

	void RayCast( float32 startx, float32 starty, 
		float32 endx, float32 endy, uint32 layer );

	void SetPause( bool pause );

	bool IsPaused();

	void ClearActorsAttacked();

	void SetCurrentActorIndex( uint32 actorIndex );
	
	uint32 GetCurrentActorIndex();

	void SetCurrentBody( b2Body *body );

	ActorHit * & GetHitsReceived();

	uint32 & GetHitsReceivedSize();

	ActorCollision * & GetBodyCollisions();

	uint32 & GetBodyCollisionsSize();

	ActorHitOffense * & GetActorsAttacked();

	uint32 & GetActorsAttackedSize();

	uint32 & GetActorsAttackedIndex();

	sf::FloatRect GroupActor::GetSpriteAABB();

	virtual void SaveState();
	virtual void LoadState();
};

struct BulletActor: public GroupActor
{
	BulletActor( const std::string &actorType,
		uint32 p_actorCount,
		const b2Vec2 &pos, const b2Vec2 &vel,
		TrueActor *parent, Stage *st );
	virtual void Init( b2World *p_world );
	virtual ~BulletActor();
	virtual void Draw( sf::RenderTarget *target,
		uint32 spriteIndex );
	virtual void CloneDraw( sf::RenderTarget *target );
	virtual void UpdatePostPhysics();
	void ClearTrail();
	void SetTrailOn( bool on );
	void UpdateTrail();
	sf::VertexArray trailArray;
	bool *trailOn;
	uint32 trailSize;
	b2Vec2 **posHistory;
	b2Vec2 **velHistory;
	sf::Shader bulletShader;

	virtual void SaveState();
	virtual void LoadState();
};

struct TentacleActor : public GroupActor
{
	TentacleActor( const std::string &actorType,
		uint32 p_actorCount,
		const b2Vec2 &pos, const b2Vec2 &vel,
		TrueActor *parent, Stage *st );
};

#endif