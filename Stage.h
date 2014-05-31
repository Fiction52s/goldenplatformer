#ifndef __STAGE_H__
#define __STAGE_H__
#include <Box2D.h>
#include <string>
#include <SFML\Graphics.hpp>
#include <list>
#include <map>
#include "Globals.h"
#include "Collision.h"
#include "Input.h"


struct TileSet
{
	TileSet();
	~TileSet();
	sf::Texture *texture;
	uint32 tileWidth;
	uint32 tileHeight;
	uint32 firstGID;
	uint32 TileCount();
	uint32 GetGID( uint32 localID );
	sf::IntRect GetSubRect( uint32 localID );
	std::string imageSource;
	std::string collisionHandler;
	std::list<std::list<b2Vec2>>* tileChains;
};



struct ImageLayer
{
	ImageLayer();
	~ImageLayer();
	//^^use a list for now. optimize later if needed
	//should be an animated image instead of a sprite
	//that I will update every frame
	//std::list< sf::Sprite* > objects; 
	std::map<TileSet*, sf::VertexArray*> objectMap;
	float scrollRatio;
	float scrollx;
	float scrolly;

	sf::View view;
};

struct ObjectParams
{
	ObjectParams( const sf::Vector2i &pos, 
		uint32 localid, TileSet* tileSet );
	sf::Vector2i pos;
	uint32 localid;
	TileSet *tileSet;
};

//items in the array   
struct PlayerChar;
struct TrueActor;
struct SingleActor;
struct GroupActor;
struct BulletActor;

struct Room;
struct Door;

class Stage;



struct Camera
{
	
	enum CameraMode
	{
		normal,
		transition,
		frozen,
		special,
		hybrid
	};

	Camera( Stage *stage );
	
	b2Vec2 pos;
	b2Vec2 offset;
	b2Vec2 maxOffset;
	float32 offsetSpeed;
	
	sf::Vector2f GetViewPos();
	void UpdatePosition( Room * currentRoom );
	void UpdateZoom();
	void Reset();
	CameraMode mode;
	Stage *stage;
	float maxZoom;
	float32 zoom;
	uint32 slowCounter;
};

class Stage
{
public:
	//enum CameraMode
	//{
	//	NORMAL,
	//	RUMBLE,
	
	

	bool exitRoom;
	uint32 playerPowers;
	uint32 lives;
	Camera c;
	Camera cloneCamera;
	//};
	void ResetCamera();
	void UpdateCamera();
	Stage( GameController &controller,
		sf::RenderWindow *window, 
		const std::string &dir, 
		const std::string &name );
	~Stage();
	void EventAreaEnter( 
		b2Fixture *eventAreaFixture );
	void EventAreaLeave( 
		b2Fixture *eventAreaFixture );
	bool UpdatePrePhysics();
	void UpdatePostPhysics();
	void UpdateTileModifications();
	void SetRoomByName( const std::string &roomName );
	Room *GetRoomByName( const std::string &roomName );
	void SetRoom( Room *room );
	bool HasPlayerPower( const std::string & powerName );
	void UpdateEventArea( b2Fixture *fixture,
		bool entered );
	bool HandleCollision( b2Contact *contact );
	uint32 **staticLocalID;
	TileSet ***staticTileSets;
	uint32 **staticAirTiles;
	uint32 tileSize;
	uint32 stageWidth;
	uint32 stageHeight;
	//the layer is NULL to represent placement of
	//the game layers
	std::list<ImageLayer* > layers; 
	TileSet **tileSets;
	uint32 tileSetCount;
	std::string *events;
	uint32 eventCount;
	std::list<b2Fixture *> eventAreasEntered;
	std::list<b2Fixture*> eventAreasExited;
	std::string *airEffects;
	uint32 airEffectCount;


	b2Vec2 camera;
	b2Vec2 offset;
	float32 offsetSpeed;




	b2Vec2 follow;
	b2Vec2 camMaxVel;
	PlayerChar *player;
	sf::View view;
	sf::View mapView;
	sf::Vector2f testOldView;
	sf::Shader testShader;
	void SetCameraPosition( float x, float y );
	b2Vec2 & GetCameraPosition();
	void SetCameraFollowX( float rate );
	void SetCameraFollowY( float rate );
	void SetCamVelocityX( float speed );
	void SetCameraZoom( float zoom );
	float GetCameraZoom();
	float cameraZoom;
	void SetCameraMaxVelocityX( float speed );
	void SetCameraMaxVelocityY( float speed );
	//void CreateActor( const std::string &type,
	//	float32 posX, float32 posy, float32 velX,
	//	float32 velY, bool facingRight, bool reverse,
	//	float32 angle, Actor *parent );
	SingleActor * CreateActor( const std::string &type,
		b2Vec2 &pos, b2Vec2 &vel, bool facingRight, 
		bool reverse, float32 angle, TrueActor *parent );
	GroupActor * CreateActorGroup( const std::string &type,
		uint32 actorCount,
		b2Vec2 &pos, b2Vec2 &vel, bool facingRight, 
		bool reverse, float32 angle, TrueActor *parent );
	BulletActor * CreateBulletGroup( uint32 actorCount, b2Vec2 &pos, b2Vec2 &vel, TrueActor *parent );
	TrueActor *GetActor( const std::string &type );
	void DebugDraw( sf::RenderTarget *rt );
	bool Run();
	sf::RenderWindow *window;
	ControllerState currentInput;
	ControllerState prevInput;
	sf::Sprite *background;
	bool TileHandleRayCast( TrueActor *a, 
		b2Fixture *fixture, b2Vec2 &collision, 
		b2Vec2 &normal );

	b2World *world;

	std::list<Room*> rooms;
	Room *startRoom;
	Room *currentRoom;
	
	Room * newRoom; //for switching rooms
	//std::string enterDoor;
	Door * currentDoor;

	Room *oldRoom;

	std::list<TrueActor*> cloneAddedActors;
	std::list<TrueActor*> cloneActiveActors;
	std::list<TrueActor*> cloneKilledActors;

	std::list<TrueActor*> activeActors;

	std::list<b2Vec2> tetherCollisions;
	//std::list<std::list<b2Vec2>> tetherCollisions;

private:
	TileSet * LoadTileSet( const std::string &dir,
		const std::string &name, 
		int firstgid, std::list< 
		std::list< b2Vec2 > >* &chains, 
		std::map< uint32, std::string > &actorTiles);
	TileSet * GlobalToLocal( uint32 &id );
	bool TileHandleCollision( 
		const std::string &handler,
		TrueActor *a, b2Vec2 &tileVec, 
		b2Vec2 &collisionVec,
		b2Vec2 &normalVec );
	
	void TileModifyWorld( StageCollision &sc );
	void GhostVertexCheck( std::list<std::list<b2Vec2>> &chainList, b2ChainShape &chainShape, 
		int x, int y, std::list<b2Vec2> &boxChain, b2Vec2 &bestPrev, b2Vec2 &bestNext);
	lua_State *L;
	std::map<std::string, lua_State*> 
		collisionHandlers;
	std::list<StageCollision> collisions;
	std::map<uint32, std::string> airHandlers;  
	std::list<std::string> consumed;
	
	std::list<TrueActor*> allActors;
	std::list<TrueActor*> addedActors;

	
	
	std::list<std::string> save_consumed;
	std::list<StageCollision> save_collisions;
	ControllerState save_prevInput;
	


	int testPlayerDeathCount;
	
	ControllerState storedInput; //for pauses,etc
	GameController controller;
	bool debugDrawOn;
	bool debugDrawEnv;
	void DebugDrawOn( bool on, bool environment );
	b2Body *mapBody;
	uint32 actorTimeTest;
	TrueActor * actorTimeTestActor;
	void QuitStage();
	
	void RoomRestart();
	void LevelRestart();


	
	

	void EnterCloneWorld();
	void CollapseCloneWorld();
	void RevertCloneWorld();
	void ExtraCloneWorld();
	
	//void ExtraCloneWorld();

	//void SaveState();
	//void LoadState();
	void DrawActorBackLayers(sf::RenderTarget *target);
	void DrawActorMidLayers(sf::RenderTarget *target);
	void DrawActorFrontLayers(sf::RenderTarget *target);
	

	std::list<std::pair<TrueActor*, uint32>> backLayers[4];
	std::list<std::pair<TrueActor*, uint32>> midLayers[4];
	std::list<std::pair<TrueActor*, uint32>> frontLayers[4];
	std::list<std::pair<TrueActor*, uint32>> actorLayer;

	
public:
	void SetSpritePriority( TrueActor* actor, uint32 spriteIndex, int32 priority );
	bool cloneWorld;
	bool cloneWorldStart;
	bool cloneWorldRevert;
	bool cloneWorldCollapse;
	bool cloneWorldExtra;
	static void InitStaticVars( sf::RenderWindow * win );
	static sf::Texture *pauseBGTexture;
	static sf::Sprite *pauseBGSprite;
	static sf::Text pauseMenuTexts[5]; 
	//sf::Text fpsText;

	static sf::Font debugFont;
	static int selectedPauseMenu;

	uint32 freezeFrames;
	void Freeze( uint32 frames );
};

#endif