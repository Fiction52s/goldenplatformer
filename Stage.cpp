#include <rapidxml.hpp>
#include <fstream>
#include <vector>
#include <string>
#include <iostream>
#include <rapidxml_print.hpp>
#include <boost/lexical_cast.hpp>
#include <sstream>
#include "Collision.h"
#include "Actor.h"
#include "Input.h"
#include <SFML/Window/Keyboard.hpp>
#include <boost/algorithm/string.hpp>
#include "Room.h"


using namespace std;
using namespace boost;
using namespace luabridge;
using namespace sf;



sf::Texture * Stage::pauseBGTexture = NULL;
sf::Sprite * Stage::pauseBGSprite = NULL;
int Stage::selectedPauseMenu = 0;
sf::Text Stage::pauseMenuTexts[5];
sf::Font Stage::debugFont;

ObjectParams::ObjectParams( const sf::Vector2i &pos, uint32 localid, TileSet* tileSet )
	:pos( pos ), localid( localid ), tileSet( tileSet )
{

}

uint32 TileSet::TileCount()
{
	return texture->getSize().x / tileWidth * texture->getSize().y / tileHeight;
}

uint32 TileSet::GetGID( uint32 localID )
{
	return firstGID + localID;
}

ImageLayer::ImageLayer()
{
	view.setSize( 1920, 1080 );
}

ImageLayer::~ImageLayer()
{
	for( std::map<TileSet*, sf::VertexArray*>::iterator objectMapIt = objectMap.begin(); 
		objectMapIt != objectMap.end(); ++objectMapIt )
	{
		delete (*objectMapIt).second;
	}
}

sf::IntRect TileSet::GetSubRect( uint32 localID )
{
	int xi,yi;
	yi = localID / (texture->getSize().x / tileWidth );
	xi = localID % (texture->getSize().x / tileWidth );

	return sf::IntRect( xi * tileWidth, yi * tileHeight, tileWidth, tileHeight ); 
}

Camera::Camera( Stage *stage )
	:offsetSpeed( .1 ), mode( normal ), stage( stage )
{
	//UpdatePosition();
	offset.x = 0;
	offset.y = 0;
}

sf::Vector2f Camera::GetViewPos()
{
	sf::Vector2f viewPos( pos.x * BOX2SF, pos.y * BOX2SF );
	viewPos.x = floor( viewPos.x + .5f );
	viewPos.y = floor( viewPos.y + .5f );

	return viewPos;
}

void Camera::Reset()
{
	offset.SetZero();

}

void Camera::UpdatePosition()
{
	b2Vec2 playerPos = stage->player->GetPosition();
	b2Vec2 playerVel = stage->player->GetVelocity() - stage->player->GetCarryVelocity();
	switch( mode )
	{
		case normal:
			maxOffset.x = 8 * stage->GetCameraZoom();
			maxOffset.y = 5 * stage->GetCameraZoom();
			if( playerVel.x > 17 )
			{
				offset.x += offsetSpeed;
			}
			else if( playerVel.x < -17 )
			{
				offset.x -= offsetSpeed;
			}
			else
			{
				if( offset.x > 0 )
				{
					offset.x -= offsetSpeed;
					if( offset.x < 0 ) offset.x = 0;
				}
				else if( offset.x < 0 )
				{
					offset.x += offsetSpeed;
					if( offset.x > 0 ) offset.x = 0;
				}

				
				
				//if( offset.y > maxOffset.y ) offset.y = maxOffset.y;
				//else if( offset.y < maxOffset.y ) offset.y = -maxOffset.y;
			}

			if( offset.x > maxOffset.x ) 
			{
				offset.x = maxOffset.x;
			}
			else if( offset.x < -maxOffset.x ) 
			{
				offset.x = -maxOffset.x;
			}

			pos = playerPos + offset;
			//cout << "pos: " << pos.x << ", " << pos.y << endl;
			break;
		//	break;
		//case special:
		//	break;
		//case hybrid:
		//	break;
		default:
			assert( 0 && "not working" );
			break;
	}
}




Stage::Stage( GameController &controller, sf::RenderWindow *window, const std::string &dir, 
	const std::string &name )
	:window( window ), controller( controller ), debugDrawOn( false ), debugDrawEnv( false ),c( this )
{
	playerPowers = 0x00; //load powers here
				//vertical farming is bit 0

	playerPowers = 0;

	lives = 3; //this might not be universal but it works for now

	currentRoom = NULL;
	newRoom = NULL;
	currentDoor = NULL;
	startRoom = NULL;
	player = NULL;
	//Resources/Worlds/Vertical Farming/
	//"Resources/Worlds/Vertical Farming/Maps


	//if( !testShader.loadFromFile( "firstfrag.frag", sf::Shader::Fragment) )
	//if( !testShader.loadFromFile( "firstvertex.vert", sf::Shader::Vertex ) )
	if( !testShader.loadFromFile( "firstvertex.vert", "firstfrag.frag" ) )
	{
		cout << "FAILED SHADER LOAD" << endl;
	}

	testPlayerDeathCount = 0;
	b2Vec2 gravity( 0, 0 );
	world = new b2World( gravity );
	background = new sf::Sprite();

	camera.SetZero();
	follow.SetZero();
	camMaxVel.Set( 20, 20 );
	cameraZoom = 1;

	L = luaL_newstate();
	luaL_openlibs( L );
	
	
	
	//luaL_dostring( L, "print( 'testing123123' )" );
	
	string luaF = dir + "Scripts/" + name + ".lua";
	int s = luaL_loadfile( L, luaF.c_str() );

	getGlobalNamespace( L )
		.beginNamespace( "STAGE" )
			.beginClass<Room>( "Room" )
				.addData( "left", &Room::left )
				.addData( "right", &Room::right )
				.addData( "top", &Room::top )
				.addData( "bottom", &Room::bottom )
				.addData( "spawn", &Room::spawn )
			.endClass()
			.beginClass<Stage>( "Stage" )
				.addFunction( "SetCameraPosition", &Stage::SetCameraPosition )
				.addFunction( "GetCameraPosition", &Stage::GetCameraPosition )
				.addFunction( "SetCameraFollowX", &Stage::SetCameraFollowX )
				.addFunction( "SetCameraFollowY", &Stage::SetCameraFollowY )
				.addFunction( "SetCameraMaxVelocityX", &Stage::SetCameraMaxVelocityX )
				.addFunction( "SetCameraMaxVelocityY", &Stage::SetCameraMaxVelocityY )
				.addFunction( "SetCameraZoom", &Stage::SetCameraZoom )
				.addFunction( "GetCameraZoom", &Stage::GetCameraZoom )
				.addFunction( "CreateActor", &Stage::CreateActor )
				.addFunction( "DebugDrawOn", &Stage::DebugDrawOn )
				.addData( "player", &Stage::player )
				.addData( "currentInput", &Stage::currentInput )
				.addData( "prevInput", &Stage::prevInput )
				.addData( "room", &Stage::currentRoom )
			.endClass()
			.beginClass<TrueActor>( "TrueActor" )
				.addFunction( "TileSetIndex", &TrueActor::TileSetIndex )
				.addFunction( "SetSprite", &TrueActor::SetSprite )
				.addFunction( "SetVelocity", &TrueActor::SetVelocity )
				.addFunction( "GetUniqueID", &TrueActor::GetUniqueID )
				.addFunction( "GetVelocity", &TrueActor::GetVelocity )
				.addFunction( "Kill", &TrueActor::Kill )
				.addFunction( "SetPosition", &TrueActor::SetPosition )
				.addFunction( "GetPosition", &TrueActor::GetPosition )
				.addFunction( "CreateBox", &TrueActor::CreateBox )
				.addFunction( "GetWorldLeft", &TrueActor::GetWorldLeft )
				.addFunction( "GetWorldRight", &TrueActor::GetWorldRight )
				.addFunction( "GetWorldTop", &TrueActor::GetWorldTop )
				.addFunction( "GetWorldBottom", &TrueActor::GetWorldBottom )
				.addFunction( "Message", &TrueActor::Message )
				.addData( "health", &TrueActor::health )
			.endClass()
			.deriveClass<GroupActor,TrueActor>("GroupActor")
			.endClass()
			.deriveClass<SingleActor,TrueActor>("SingleActor")
			.endClass()
			.deriveClass<PlayerChar, SingleActor>("PlayerChar")
				.addFunction( "SetCarryVelocity", &PlayerChar::SetCarryVelocity )
				.addFunction( "GetCarryVelocity", &PlayerChar::GetCarryVelocity )
			.endClass()
			//.deriveClass<PlayerChar, Actor>( "PlayerChar" )
			//.endClass()
			.beginClass<b2Vec2>( "b2Vec2" )
				.addConstructor<void(*)(void)>()
				.addData( "x", &b2Vec2::x )
				.addData( "y", &b2Vec2::y )
				.addFunction( "Set", &b2Vec2::Set )
			.endClass()
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

	push<Stage*>( L, this );
	lua_setglobal( L, "stage" );
	push( L, &currentInput );
	lua_setglobal( L, "currentInput" );

	/*lua_pushnumber( L, 1 );
	lua_setglobal( L, "Event" );

	lua_pushnumber( L, 2 );
	lua_setglobal( L, "PlayerHitbox" );

	lua_pushnumber( L, 3 );
	lua_setglobal( L, "PlayerHurtbox" );

	lua_pushnumber( L, 4 );
	lua_setglobal( L, "EnemyHitbox" );

	lua_pushnumber( L, 5 );
	lua_setglobal( L, "EnemyHurtbox" );

	lua_pushnumber( L, 6 );
	lua_setglobal( L, "Environment" );*/


	lua_pcall( L, 0, 0, 0 );
	
	camera.SetZero();
	//+
	//view.setSize( window->getSize().x / 2, window->getSize().y / 2 );
	//view.setSize( 800, 800 );//window->getSize().x, window->getSize().y );
	//cout << window->getSize().x << ", " << window->getSize().y << endl;
	//view.setSize( 1920, 1080 );
#define xxxxx 1920
#define yyyyy 1080
	view.setSize( xxxxx, yyyyy );
	//view.setViewport( sf::FloatRect( .25, .25, .5, .5 ) );
	//view.zoom( 1.2 );

	ifstream stageScript( dir + "Maps/" + name + ".tmx" );
	if( !stageScript.is_open() )
	{
		assert( 0 && "invalid stage name" );
	}
	string stageString( istreambuf_iterator<char>( stageScript ), (istreambuf_iterator<char>()) );
	stageScript.close();

	vector<char> stringCopy( stageString.begin(), stageString.end() );
	stringCopy.push_back( '\0' );

	if( stageString.length() > 0 )
	{
		rapidxml::xml_document<> doc;
		doc.parse<0>( &stringCopy[0] );

		rapidxml::xml_node<> *node = doc.first_node();
		//node = node->first_node();
		string length = node->name();
		//cout << "variable: " << length << endl;
		rapidxml::xml_attribute<> *attr = node->first_attribute();
		attr = attr->next_attribute();
		attr = attr->next_attribute();
		
		stageWidth = boost::lexical_cast<uint32>( attr->value() );
		
		//std::cout << "w: " << width << endl;
		attr = attr->next_attribute();

		stageHeight = boost::lexical_cast<uint32>( attr->value() );

		//cout << "stageWidth: " << stageWidth << endl;
		//cout << "stageHeight: " << stageHeight << endl;
		//cout << "h: " << height << endl;
		attr = attr->next_attribute();
		uint32 tileWidth = boost::lexical_cast<uint32>( attr->value() );
		//cout << "tilew: " << tileWidth << endl;
		attr = attr->next_attribute();
		uint32 tileHeight = boost::lexical_cast<uint32>( attr->value() );
		//cout << "tileh: " << tileHeight << endl;
		node = node->first_node();

		assert( tileHeight == tileWidth );
		tileSize = tileWidth;

		//create the arrays which will store all tile based game stuff
		staticLocalID = new uint32*[stageWidth];
		staticTileSets = new TileSet**[stageWidth];
		staticAirTiles = new uint32*[stageWidth];
		for( int i = 0; i < stageWidth; ++i )
		{
			staticLocalID[i] = new uint32[stageHeight];
			staticTileSets[i] = new TileSet*[stageHeight];
			staticAirTiles[i] = new uint32[stageHeight];

			for( int j = 0; j < stageHeight; ++j )
			{
				staticTileSets[i][j] = NULL;
			}
		}

		//tilesets, and their chain shapes which are stored in an array indexed with localID
		map< TileSet*, list< list< b2Vec2 > >* > tileSetMap;
		list<TileSet*> tileSetList;
		map<uint32, string> actorTiles;

		while( string(node->name()) == "tileset" )
		{
			attr = node->first_attribute();
			int firstGID = boost::lexical_cast<int>( attr->value() );
			attr = attr->next_attribute();
			string tileSetFileName = attr->value();
			cout << "blah: " << dir << endl;
			cout << "tile set file name: " << tileSetFileName << endl;
			list<list<b2Vec2>>* tileSetChains = NULL;
			
			TileSet* ts = LoadTileSet( dir, tileSetFileName, firstGID, tileSetChains, actorTiles );
			tileSetMap[ts] = tileSetChains;
			tileSetList.push_back( ts );

			if( string(node->next_sibling()->name()) == "tileset" )
					node = node->next_sibling();
			else
				break;
		}

		//create the arra of tile sets which will be used by actors and tiles
		tileSets = new TileSet*[tileSetMap.size()];
		tileSetCount = tileSetMap.size();
		int index = 0;
		for( list< TileSet* >::iterator tileSetIt = tileSetList.begin();
			tileSetIt != tileSetList.end(); ++tileSetIt, ++index )
		{
			tileSets[index] = (*tileSetIt);
		}

		

		bool actorLayerYet = false;
		
		while( node = node->next_sibling() )
		{
			attr = node->first_attribute();
			string layerType = node->name();
			string layerName = attr->value();
			attr = attr->next_attribute();
			int layerWidth = boost::lexical_cast<int>( attr->value() );
			attr = attr->next_attribute();
			int layerHeight = boost::lexical_cast<int>( attr->value() );

			
			if( layerType == "imagelayer" )
			{
				node = node->first_node();
				attr = node->first_attribute();
				string bgImageName = attr->value();

				sf::Texture *bgTexture = new sf::Texture;
				bgTexture->setSmooth( false );
				bgTexture->setRepeated( false );
				bool success = bgTexture->loadFromFile( dir + "Maps/" + bgImageName );
				assert( success );
				background->setTexture( *bgTexture );
				
				sf::IntRect r( 0, 0, 1920, 1080 );
				background->setTextureRect( r );
				background->setOrigin( background->getLocalBounds().width / 2, 
					background->getLocalBounds().height / 2 );
				node = node->parent();
				continue;
			}
		
			node = node->first_node();
			node = node->first_node();
			//node name shoudl now be property
			attr = node->first_attribute();
			attr = attr->next_attribute();
			string typeProperty = attr->value();
			cout << layerName << ": " << typeProperty << endl;

			float scrollRatio = 1;
			float scrollx = 0;
			float scrolly = 0;
			if ( boost::iequals( typeProperty, "image" ) && layerType == "objectgroup" )
			{
				node = node->next_sibling();
				attr = node->first_attribute();
				attr = attr->next_attribute();
				scrollRatio = boost::lexical_cast<float>( attr->value() );
				node = node->next_sibling();
				attr = node->first_attribute();
				attr = attr->next_attribute();
				scrollx = boost::lexical_cast<float>( attr->value() );
				node = node->next_sibling();
				attr = node->first_attribute();
				attr = attr->next_attribute();
				scrolly = boost::lexical_cast<float>( attr->value() );
			}
			
			node = node->parent();

			if( boost::iequals( typeProperty, "map" ) )
			{
				layers.push_back( NULL );
			}
			else if( boost::iequals( typeProperty, "actor" ) && !actorLayerYet )
			{
				layers.push_back( NULL );
				actorLayerYet = true;
			}

			if( node->next_sibling() == NULL )
			{
				//no other properties?
				node = node->parent();
				continue;
			}
			else
			{
				node = node->next_sibling();
			}

			if( boost::iequals( typeProperty, "image" ) )
			{
				ImageLayer *layer = new ImageLayer;
				layer->scrollRatio = scrollRatio;
				layer->scrollx = scrollx;
				layer->scrolly = scrolly;
				layers.push_back( layer );
				

				std::list<ObjectParams*> objectInfo;

				if( layerType == "layer" ) //tile
				{
					node = node->first_node();

					uint32 x=0,y=0;
					while( node != NULL && string(node->name()) == "tile" )
					{
						attr = node->first_attribute();

						uint32 id = atoi( attr->value() );

						TileSet *set = GlobalToLocal( id ); 

						//row by row
						if( id != 0 )
						{
							sf::Vector2i pos( x * tileSize, y * tileSize );
							objectInfo.push_back( new ObjectParams( pos, id, set ) );
						}
					//	staticLocalID[x][y] = id;
					//	staticTileSets[x][y] = ts;

						++x;
						if( x >= stageWidth )
						{
							x = 0;
							++y;
						}

						if( node->next_sibling() != NULL 
							&& string(node->next_sibling()->name()) == "tile" )
							node = node->next_sibling();
						else
							break;
					}
					node = node->parent();
					node = node->parent();
					//will probably have this but not in the demo
				}
				else if( layerType == "objectgroup" ) //object layer
				{
					
					while( node != NULL && string(node->name()) == "object" )
					{
						
						attr = node->first_attribute();
						uint32 id = boost::lexical_cast<int>( attr->value() );

						TileSet *set = GlobalToLocal( id );
						
						attr = attr->next_attribute();
						int xLoc = boost::lexical_cast<int>( attr->value() );
						attr = attr->next_attribute();
						int yLoc = boost::lexical_cast<int>( attr->value() );

						//sf::Sprite * spr = new sf::Sprite( *set->texture );
						//spr->setTextureRect( set->GetSubRect( id ) );
						//spr->setPosition( xLoc + spr->getLocalBounds().width / 2.f, yLoc - spr->getLocalBounds().height / 2.f );
						sf::Vector2i pos( xLoc, yLoc );
						objectInfo.push_back( new ObjectParams( pos, id, set ) );



						//spr->setPosition( (int)spr->getPosition().x, (int)spr->getPosition().y );
						//spr->setOrigin( (int)spr->getLocalBounds().width / 2.f, (int)spr->getLocalBounds().height / 2.f );
						
						//^^have properties for scaling and rotation
						//layer->objects.push_back( spr );
						
						

						if( node->next_sibling() != NULL 
							&& string(node->next_sibling()->name()) == "object" )
							node = node->next_sibling();
						else
							break;
						
						
					}
					node = node->parent();


				}
				std::map<TileSet*, uint32> tilePerSetCount;
				for( std::list<ObjectParams*>::iterator objectInfoIt = objectInfo.begin(); 
					objectInfoIt != objectInfo.end(); ++objectInfoIt )
				{
					tilePerSetCount[(*objectInfoIt)->tileSet]++;
				}

				for( std::map<TileSet*, uint32>::iterator countIt = tilePerSetCount.begin(); 
					countIt != tilePerSetCount.end(); ++countIt )
				{
					cout << "texture: " << (*countIt).first->firstGID << ", count: " << (*countIt).second << endl;
					layer->objectMap[(*countIt).first] = 
						new sf::VertexArray( sf::Quads, (*countIt).second * 4 );
				}

				TileSet *tileSet = NULL;
				
				std::map<TileSet*, sf::VertexArray*> & objectMap = layer->objectMap;
				
				for( std::map<TileSet*, sf::VertexArray*>::iterator objectMapIt = objectMap.begin(); 
					objectMapIt != objectMap.end(); ++objectMapIt )
				{
					tileSet = (*objectMapIt).first;
					sf::VertexArray &vertexArray = *(*objectMapIt).second;
					//for( int i = 0; i < vertexArray.getVertexCount(); i += 4 )
					//{
					//	vertexArray[i].position = 
					//}
					int i = 0;
					for( std::list<ObjectParams*>::iterator objectInfoIt = objectInfo.begin(); 
					objectInfoIt != objectInfo.end(); ++objectInfoIt )
					{
						ObjectParams *info = (*objectInfoIt);
						if( tileSet == info->tileSet )
						{
							float blend = -0.5f;
							sf::IntRect obRect = tileSet->GetSubRect( info->localid );
							vertexArray[i].texCoords = sf::Vector2f( obRect.left - blend, obRect.top - blend );
							vertexArray[i+1].texCoords = sf::Vector2f( obRect.left - blend, obRect.top + obRect.height + blend );
							vertexArray[i+2].texCoords = sf::Vector2f( obRect.left + obRect.width + blend, 
								obRect.top + obRect.height + blend);
							vertexArray[i+3].texCoords = sf::Vector2f( obRect.left + obRect.width + blend, obRect.top - blend );
							
							info->pos.y -= obRect.height;
							vertexArray[i].position = sf::Vector2f( info->pos.x, info->pos.y );
							vertexArray[i+1].position = sf::Vector2f( info->pos.x, info->pos.y + obRect.height );
							vertexArray[i+2].position = sf::Vector2f( info->pos.x + obRect.width, 
								info->pos.y + obRect.height );
							vertexArray[i+3].position = sf::Vector2f( info->pos.x + obRect.width, info->pos.y );

							//vertexArray[i].color = sf::Color::Red;
							
							i += 4;
						}
					}
				}

				for( std::list<ObjectParams*>::iterator objectInfoIt = objectInfo.begin(); 
					objectInfoIt != objectInfo.end(); ++objectInfoIt )
				{
					delete (*objectInfoIt);
				}
			}
			else if( boost::iequals( typeProperty, "map" ) )
			{
				//sf::Clock statClock;
				if( layerType == "layer" )
				{
					node = node->first_node();

					uint32 x=0,y=0;
					while( node != NULL && string(node->name()) == "tile" )
					{
						attr = node->first_attribute();

						uint32 id = atoi( attr->value() );

						TileSet *ts = GlobalToLocal( id ); 

						//row by row
						
						staticLocalID[x][y] = id;
						staticTileSets[x][y] = ts;

						++x;
						if( x >= stageWidth )
						{
							x = 0;
							++y;
						}

						if( node->next_sibling() != NULL 
							&& string(node->next_sibling()->name()) == "tile" )
							node = node->next_sibling();
						else
							break;
					}
					node = node->parent();
					node = node->parent();

					//cout << "map: " << statClock.getElapsedTime().asSeconds() << endl;
					//tile layer
				}
				else if( layerType == "objectgroup" )
				{
					//image layer
					
				}
				else
				{
					//image layer. not in use at the moment
				}
			}
			else if( boost::iequals( typeProperty, "event" ) )
			{
				//the event layer is the first non-image layer so it will make the space for processing
				//layers.push_back( NULL );
				list< string > eventList;
				
				//always object type
				while( node != NULL && string(node->name()) == "object" )
				{
					attr = node->first_attribute();
					string eventType = attr->value();
					attr = attr->next_attribute();

					uint32 xLoc = boost::lexical_cast<int>( attr->value() );
					attr = attr->next_attribute();

					uint32 yLoc = boost::lexical_cast<int>( attr->value() );
					attr = attr->next_attribute();

					uint32 width = boost::lexical_cast<int>( attr->value() );
					attr = attr->next_attribute();

					uint32 height = boost::lexical_cast<int>( attr->value() );
					attr = attr->next_attribute();

					bool ellipse = false;
					if( node->first_node() != NULL ) ellipse = true;

					b2BodyDef eventAreaDef;
					eventAreaDef.type = b2_staticBody;
					eventAreaDef.position.Set( (xLoc + width / 2.f) * SF2BOX, 
						( yLoc + height / 2.f ) * SF2BOX );
					b2Body *eventArea = world->CreateBody( &eventAreaDef );
					b2FixtureDef fixtureDef;
					b2CircleShape c;
					b2PolygonShape p;
					if( ellipse )
					{	
						c.m_radius = width / 2.f * SF2BOX;
						fixtureDef.shape = &c;
					}
					else
					{
						b2Vec2 center( 0, 0 );
						p.SetAsBox( width / 2.f * SF2BOX, height / 2.f * SF2BOX, center, 0 );
						fixtureDef.shape = &p;
					}

					CollisionLayers::SetupFixture( CollisionLayers::Event, fixtureDef.filter.categoryBits,
						fixtureDef.filter.maskBits );

					//fixtureDef.filter.categoryBits = 1 << CollisionLayers::Event;
					fixtureDef.isSensor = true;
					
					eventArea->CreateFixture( &fixtureDef );
					//future index in array

					bool found = false;
					uint32 foundIndex = 0;
					for( list<string>::iterator eventIt = eventList.begin(); eventIt != eventList.end() 
						&& !found; ++eventIt, ++foundIndex )
					{
						if( (*eventIt) == eventType )
						{
							found = true;
							break;
						} 
					}

					if( found )
					{
						eventArea->SetUserData( (void*)foundIndex );
					}
					else
					{
						eventArea->SetUserData( (void*)eventList.size() );
						eventList.push_back( eventType );
					}
						
					if( node->next_sibling() != NULL 
						&& string(node->next_sibling()->name()) == "object" )
						node = node->next_sibling();
					else
						break;
				}

				events = new string[eventList.size()];
				eventCount = eventList.size();
				int eIndex = 0;
				for( list<string>::iterator eventIt = eventList.begin(); eventIt != eventList.end(); ++eventIt,
					++eIndex )
				{
					events[eIndex] = (*eventIt);
				}	

				node = node->parent();
			}
			else if( boost::iequals( typeProperty,  "air" ) )
			{
				if( layerType == "layer" )
				{
					//sf::Clock statClock;
					//sf::Clock sc2;
					//sf::Clock sc3;
					node = node->first_node();
					//double sc2Full = 0, sc3Full = 0;

					
					uint32 x=0;
					uint32 y=0;

					while( node != NULL && string(node->name()) == "tile" )
					{
						attr = node->first_attribute();
						//sc2.restart();

						//uint32 id = (uint32)boost::lexical_cast<int>( attr->value() );
						uint32 id = atoi( attr->value() );

						//sc2Full += sc2.getElapsedTime().asSeconds();

						//row by row
						
						//global ID to reference map 
						//cout << "x: " << x << ", y: " << yy << ", id: " << id << endl;
						//sc3.restart();
						staticAirTiles[x][y] = id;
						//sc3Full += sc3.getElapsedTime().asSeconds();
						//sdfdsf++;
						//cout << sdfdsf << endl;
						
						++x;
						if( x >= stageWidth )
						{
							x = 0;
							++y;
						}

						if( node->next_sibling() != NULL 
							&& string(node->next_sibling()->name()) == "tile" )
							node = node->next_sibling();
						else
							break;
					}
					node = node->parent();
					node = node->parent();

					//cout << "air: " << statClock.getElapsedTime().asSeconds() << endl;
					//cout << "1: " << sc2Full << endl;
					//cout << "2: " << sc3Full << endl;
				}
				//not in the demo
			}
			else if( boost::iequals( typeProperty, "actor" ) )
			{
				if( layerType == "layer" ) //tile
				{

					//will probably have this but not in the demo
				}
				else if( layerType == "objectgroup" ) //object layer
				{
					//sf::Clock statClock;

					list<TrueActor*> *squad = new list<TrueActor*>;
					b2BodyDef activationDef;
					activationDef.position = b2Vec2( 0, 0 );
					b2Body * activationBody = world->CreateBody( &activationDef );

					while( node != NULL && string(node->name()) == "object" )
					{
						attr = node->first_attribute();
						string name = "";
						if( std::string(attr->name()) == "name" )
						{
							name = attr->value();
							attr = attr->next_attribute();
						}

						if( std::string( attr->name() ) != "gid" )
						{
							
							bool circle = false;
							if( node->first_node() != NULL )
							{
								circle = true;
							}
							cout << "creating" << endl;
							int x = boost::lexical_cast<int>( attr->value() );
							attr = attr->next_attribute();
							int y = boost::lexical_cast<int>( attr->value() );
							attr = attr->next_attribute();
							int w = boost::lexical_cast<int>( attr->value() );
							attr = attr->next_attribute();
							int h = boost::lexical_cast<int>( attr->value() );

							//b2BodyDef def;
							//def.position = b2Vec2( ( x + w / 2.f ) * SF2BOX, ( y + h / 2.f ) * SF2BOX );
							//b2Body * b = world->CreateBody( &def );
							b2FixtureDef fdef;
							fdef.isSensor = true;

							CollisionLayers::SetupFixture( CollisionLayers::ActivateBox, fdef.filter.categoryBits, 
									fdef.filter.maskBits );


							if( circle )
							{
								b2CircleShape cs;
								cs.m_radius = w * SF2BOX;
								cs.m_p = b2Vec2( ( x + w / 2.f ) * SF2BOX, ( y + h / 2.f ) * SF2BOX );
								fdef.shape = &cs;
								
								
								activationBody->CreateFixture( &fdef );
							}
							else
							{
								b2PolygonShape ps;
								ps.SetAsBox( w / 2.f * SF2BOX, h / 2.f * SF2BOX, 
									b2Vec2( ( x + w / 2.f ) * SF2BOX, ( y + h / 2.f ) * SF2BOX ), 0 );
								
								fdef.shape = &ps;

								activationBody->CreateFixture( &fdef );
							
							}

							activationBody->SetUserData( (void*)squad );
							
							if( node->next_sibling() != NULL )
							{
								node = node->next_sibling();
								attr = node->first_attribute();
							}
							else
							{
								break;
							}

							continue;
						}
						uint32 id = boost::lexical_cast<uint32>( attr->value() );

						assert( actorTiles.count( id ) > 0 );
						string &actorType = actorTiles[id];

						attr = attr->next_attribute();
						int x = boost::lexical_cast<int>( attr->value() );
						attr = attr->next_attribute();
						int y = boost::lexical_cast<int>( attr->value() );
						float angle = 0;
						b2Vec2 aVel( 0, 0 );
						bool facingRight = true;
						bool reverse = false;
						list<pair<string, float>> preInitMessages;

						if( node->first_node() != NULL ) //checks if there are any properties
						{
							node = node->first_node();
							node = node->first_node();
							attr = node->first_attribute();

							//note: boost::iequals is for comparing strings without checking case
							while( true ) //manually break this loop
							{
								string attrName( attr->value() );
								attr = attr->next_attribute();
								if( boost::iequals( attrName, "angle" ) )
								{
									angle = boost::lexical_cast<float>( attr->value() );
								}
								else if( boost::iequals( attrName, "vel" ) )
								{
									string velVectorStr( attr->value() );

									//start after opening parens. record till comma. then get the number till the close parens
									int i = 1;
									stringstream xss;
									stringstream yss;
									while( velVectorStr[i] != ',' )
									{
										xss << velVectorStr[i];
										++i;
									}
									++i;
									while( velVectorStr[i] != ')' )
									{
										yss << velVectorStr[i];
										++i;
									}

									aVel.x = boost::lexical_cast<float>( xss.str() );
									aVel.y = boost::lexical_cast<float>( yss.str() );
								}
								else if( boost::iequals( attrName, "left" ) )
								{
									facingRight = false;
								}
								else if( boost::iequals( attrName, "right" ) )
								{
									//^^ I dont think this is necessary
									facingRight = true;
								}
								else if( boost::iequals( attrName, "reverse" ) )
								{
									reverse = true;
								}
								else if( boost::iequals( attrName.substr(0, 4), "msg_" ) )
								{
									string msg = attrName.substr( 4 );
									
									float tag = boost::lexical_cast<float>( attr->value() );
								
									preInitMessages.push_back( pair<string,float>( msg, tag ) );
									//aVel.x = boost::lexical_cast<float>( xss.str() );
									//aVel.y = boost::lexical_cast<float>( yss.str() );
								}

								if( node->next_sibling() != NULL )
								{
									node = node->next_sibling();
									attr = node->first_attribute();
								}
								else
								{
									break;
								}
							}
							
							node = node->parent();
							node = node->parent();
						}


						TileSet *set = GlobalToLocal( id );
						
						//attr = attr->next_attribute();
						
						sf::IntRect ob = set->GetSubRect( id );
						
						//^^read in angle from Tiled
						//default to facing left and not being reversed
						b2Vec2 aPos( ( x + ob.width/2.f ) * SF2BOX, ( y - ob.height/2.f ) * SF2BOX );
						TrueActor * a = CreateActor( actorType, aPos, aVel, facingRight, reverse, angle, NULL );
					//	cout << "creating actor of type: " << actorType << " at " << x << ", " << y << endl;
						
						squad->push_back( a );

						if( name != "" )
						{
							push<TrueActor*>( L, a );
							lua_setglobal( L, ("a_" + name).c_str() );
						}

						for( list<pair<string,float>>::iterator preInitMessagesIt = preInitMessages.begin();
							preInitMessagesIt != preInitMessages.end(); ++preInitMessagesIt )
						{
							a->Message( NULL, (*preInitMessagesIt).first, (*preInitMessagesIt).second );
						}

						if( node->next_sibling() != NULL 
							&& string(node->next_sibling()->name()) == "object" )
							node = node->next_sibling();
						else
							break;
					}

					if( squad->empty() )
					{
						delete squad;
						world->DestroyBody( activationBody );
					}
					node = node->parent();
					//cout << "actor: " << statClock.getElapsedTime().asSeconds() << endl;
				}
			}
			else if( boost::iequals( typeProperty, "room" ) )
			{
				if( layerType == "layer" ) //tile
				{

					//will probably have this but not in the demo
				}
				else if( layerType == "objectgroup" ) //object layer
				{
					//sf::Clock statClock;

					//list<TrueActor*> *squad = new list<TrueActor*>;
					//b2BodyDef activationDef;
					//activationDef.position = b2Vec2( 0, 0 );
					//b2Body * activationBody = world->CreateBody( &activationDef );

					Room *room = new Room( this );

					


					while( node != NULL && string(node->name()) == "object" )
					{
						//


						attr = node->first_attribute();
						string name = "";
						if( std::string(attr->name()) == "name" )
						{
							name = attr->value();
							attr = attr->next_attribute();
						}

						if( std::string( attr->name() ) != "gid" )
						{							
							int x = boost::lexical_cast<int>( attr->value() );
							attr = attr->next_attribute();
							int y = boost::lexical_cast<int>( attr->value() );

							bool circle = false;

							bool properties = false;

							if( node->first_node() != NULL && string(node->first_node()->name()) == "properties" ) //checks if there are any properties
							{
								properties = true;
								node = node->first_node();
								node = node->first_node();
								attr = node->first_attribute();

								//note: boost::iequals is for comparing strings without checking case
								while( true ) //manually break this loop
								{
									string attrName( attr->value() );
									attr = attr->next_attribute();


									
									if( boost::iequals( attrName, "owner" ) )
									{
										string ownerRoomName( attr->value() );
										room->ownerName= ownerRoomName;
										/*for( list<Room*>::iterator rIt = rooms.begin(); rIt != rooms.end(); ++rIt )
										{
											if( (*rIt)->name == parentRoomName )
											{
												room->parent = (*rIt);
											}											
										}*/
									}


									if( node->next_sibling() != NULL )
									{
										node = node->next_sibling();
										attr = node->first_attribute();
									}
									else
									{
										break;
									}
								}
							
								node = node->parent();
								//node = node->parent();
							}

							bool polyline = false;
							if( !properties && node->first_node() != NULL )
							{
								node = node->first_node();
								polyline = true;
								//if( string( node->first_node()->name() ) == "polyline" )
								//{

								//}
							}
							else if( properties && node->next_sibling() != NULL )
							{
								node = node->next_sibling();
								polyline = true;
							}

							if( polyline )
							{
								//if( string( node->first_node()->name() ) == "polyline" )
								//{
								room->name = name;
								//node = node->first_node();
								string points = node->first_attribute()->value();

								stringstream pointStream( points );
								int32 px=0,py=0;
								while( pointStream >> px )
								{
									if( pointStream.peek() == ',' )
										pointStream.ignore();

									pointStream >> py;

									px += x;
									py += y;

									room->border.push_back( b2Vec2( px * SF2BOX, py * SF2BOX ) );
								}
										

								//removing final value because its the same as the first value
								room->border.pop_back();

								float32 &left = room->left;
								float32 &right = room->right;
								float32 &top = room->top;
								float32 &bottom = room->bottom;

								left = stageWidth * BOX2SF;
								right = 0;
								top = stageHeight * BOX2SF;
								bottom = 0;
										
								for( list<b2Vec2>::iterator pIt = room->border.begin(); pIt != room->border.end();
									++pIt )
								{
									b2Vec2 &p = (*pIt );

									if( p.x < left ) left = p.x;
									if( p.x > right ) right = p.x;
									if( p.y < top )	top = p.y;
									if( p.y > bottom ) bottom = p.y;
								}


								node = node->parent();

								if( node->next_sibling() != NULL 
									&& string(node->next_sibling()->name()) == "object" )
								{
									node = node->next_sibling();
									continue;
								}
								else
								{
									break;
								}
										
						
							//	}
								//else
								//{
							//		circle = true;
							//	}
							}


							if( attr->next_attribute() == NULL )
							{
								//single point, rectangle of 0 w and 0 h. Put into the room's point 
								//collection

								b2Vec2 refP( x * SF2BOX, y * SF2BOX );
								room->referencePoints[name] = refP;
								
								if( name == "Spawn" )
								{
									room->spawn = refP;
								}

								if( node->next_sibling() != NULL 
									&& string(node->next_sibling()->name()) == "object" )
								{
									node = node->next_sibling();
									continue;
								}
								else
								{
									break;
								}

								
							}
							
							attr = attr->next_attribute();
							int w = boost::lexical_cast<int>( attr->value() );
							attr = attr->next_attribute();
							int h = boost::lexical_cast<int>( attr->value() );

							//b2BodyDef def;
							//def.position = b2Vec2( ( x + w / 2.f ) * SF2BOX, ( y + h / 2.f ) * SF2BOX );
							//b2Body * b = world->CreateBody( &def );
							b2FixtureDef fdef;
							fdef.isSensor = true;

							CollisionLayers::SetupFixture( CollisionLayers::ActivateBox, fdef.filter.categoryBits, 
									fdef.filter.maskBits );


							if( circle )
							{
								b2CircleShape cs;
								cs.m_radius = w * SF2BOX;
								cs.m_p = b2Vec2( ( x + w / 2.f ) * SF2BOX, ( y + h / 2.f ) * SF2BOX );
								fdef.shape = &cs;
								
								
								//activationBody->CreateFixture( &fdef );
							}
							else
							{
								b2PolygonShape ps;
								ps.SetAsBox( w / 2.f * SF2BOX, h / 2.f * SF2BOX, 
									b2Vec2( ( x + w / 2.f ) * SF2BOX, ( y + h / 2.f ) * SF2BOX ), 0 );
								
								fdef.shape = &ps;

								//activationBody->CreateFixture( &fdef );
							
							}

							//activationBody->SetUserData( (void*)squad );
							
							if( node->next_sibling() != NULL )
							{
								node = node->next_sibling();
								attr = node->first_attribute();
							}
							else
							{
								break;
							}

							continue;
						}
						//otherwise its an actor 

						
						uint32 id = boost::lexical_cast<uint32>( attr->value() );
						cout << "id: " << id << ", count: " << actorTiles.count(id ) << endl;
						assert( actorTiles.count( id ) > 0 );
						string &actorType = actorTiles[id];

						attr = attr->next_attribute();
						int x = boost::lexical_cast<int>( attr->value() );
						attr = attr->next_attribute();
						int y = boost::lexical_cast<int>( attr->value() );

						ActorDef *ad = new ActorDef;
						ad->type = actorType;
						ad->vel.Set( 0, 0 );
						ad->facingRight = true;
						ad->reverse = false;
						ad->angle = 0;
						ad->parent = NULL;
						ad->actorCount = 1;
						//ActorDef *ad = new ActorDef( actorType, actorCount, aPos, aVel, facingRight, reverse, angle, 
						//	NULL );
						//float angle = 0;
						//b2Vec2 aVel( 0, 0 );
						//bool facingRight = true;
						//bool reverse = false;
						//list<pair<string, float>> preInitMessages;

						if( node->first_node() != NULL ) //checks if there are any properties
						{
							node = node->first_node();
							node = node->first_node();
							attr = node->first_attribute();

							//note: boost::iequals is for comparing strings without checking case
							while( true ) //manually break this loop
							{
								string attrName( attr->value() );
								attr = attr->next_attribute();
								if( boost::iequals( attrName, "angle" ) )
								{
									ad->angle = boost::lexical_cast<float>( attr->value() );
								}
								else if( boost::iequals( attrName, "vel" ) )
								{
									string velVectorStr( attr->value() );

									//start after opening parens. record till comma. then get the number till the close parens
									int i = 1;
									stringstream xss;
									stringstream yss;
									while( velVectorStr[i] != ',' )
									{
										xss << velVectorStr[i];
										++i;
									}
									++i;
									while( velVectorStr[i] != ')' )
									{
										yss << velVectorStr[i];
										++i;
									}

									ad->vel.x = boost::lexical_cast<float>( xss.str() );
									ad->vel.y = boost::lexical_cast<float>( yss.str() );
								}
								else if( boost::iequals( attrName, "left" ) )
								{
									ad->facingRight = false;
								}
								else if( boost::iequals( attrName, "right" ) )
								{
									//^^ I dont think this is necessary
									ad->facingRight = true;
								}
								else if( boost::iequals( attrName, "reverse" ) )
								{
									ad->reverse = true;
								}
								else if( boost::iequals( attrName.substr(0, 4), "msg_" ) )
								{
									cout << "message interface----------------------" << endl;
									string msg = attrName.substr( 4 );
									
									float tag = boost::lexical_cast<float>( attr->value() );
								
									ad->preInitMessages.push_back( pair<string,float>( msg, tag ) );
									//aVel.x = boost::lexical_cast<float>( xss.str() );
									//aVel.y = boost::lexical_cast<float>( yss.str() );
								}

								if( node->next_sibling() != NULL )
								{
									node = node->next_sibling();
									attr = node->first_attribute();
								}
								else
								{
									break;
								}
							}
							
							node = node->parent();
							node = node->parent();
						}


						TileSet *set = GlobalToLocal( id );
						
						//attr = attr->next_attribute();
						
						sf::IntRect ob = set->GetSubRect( id );
						
						//^^read in angle from Tiled
						//default to facing left and not being reversed
						b2Vec2 aPos( ( x + ob.width/2.f ) * SF2BOX, ( y - ob.height/2.f ) * SF2BOX );
						ad->pos = aPos;
						//TrueActor * a = CreateActor( actorType, aPos, aVel, facingRight, reverse, angle, NULL );
					//	cout << "creating actor of type: " << actorType << " at " << x << ", " << y << endl;
						
						//uint32 actorCount = 1;
						//ActorDef *ad = new ActorDef( actorType, actorCount, aPos, aVel, facingRight, reverse, angle, 
						//	NULL );
						

						if( actorType == "player" )
						{
							currentRoom = room;
							startRoom = currentRoom;
							TrueActor *a = CreateActor( actorType, ad->pos, ad->vel, ad->facingRight, ad->reverse, 
								ad->angle, NULL );
						}
						else
						{
							room->actorDefs.push_back( ad );
						}

						//squad->push_back( a );

						//if( name != "" )
						//{
						//	push<TrueActor*>( L, a );
						//	lua_setglobal( L, ("a_" + name).c_str() );
						//}

						//for( list<pair<string,float>>::iterator preInitMessagesIt = preInitMessages.begin();
						//	preInitMessagesIt != preInitMessages.end(); ++preInitMessagesIt )
						//{
						//	a->Message( NULL, (*preInitMessagesIt).first, (*preInitMessagesIt).second );
						//}

						if( node->next_sibling() != NULL 
							&& string(node->next_sibling()->name()) == "object" )
						{
							node = node->next_sibling();
						}
						else
						{
							break;
						}
					}

					rooms.push_back( room );

					if( currentRoom == room )
					{
						currentRoom->Enter( "test" );
					}
					//if( squad->empty() )
					//{
					//	delete squad;
					//	world->DestroyBody( activationBody );
					//}
					node = node->parent();
					//cout << "actor: " << statClock.getElapsedTime().asSeconds() << endl;
				}
			}
			else if( boost::iequals( typeProperty, "door" ) )
			{
				//node = node->parent();
				//continue;
				if( layerType == "objectgroup" ) //object layer
				{
					while( node != NULL && string(node->name()) == "object" )
					{
						Door *door = new Door;
						door->open = false;
						door->collide = false;
						door->open = true;
						door->roomA = NULL;
						door->roomB = NULL;

						attr = node->first_attribute();

						string name = attr->value();
						attr = attr->next_attribute();

						string roomAName = name.substr( 0, name.find( '_' ) );
						string roomBName = name.substr( name.find( '_' ) + 1 );

						for( list<Room*>::iterator rIt = rooms.begin(); rIt != rooms.end(); ++rIt )
						{
							if( (*rIt)->name == roomAName )
							{
								door->roomA = (*rIt );
							}
							else if( (*rIt)->name == roomBName )
							{
								door->roomB = (*rIt );
							}
							cout <<"ROOM NAME: " << (*rIt)->name << ", aname: " << roomAName << ", bname: " << roomBName << endl;
						}

						assert( door->roomA != NULL && door->roomB != NULL );
						//door->roomA = 
						//door->roomB = 
												
						int x = boost::lexical_cast<int>( attr->value() );
						attr = attr->next_attribute();
						int y = boost::lexical_cast<int>( attr->value() );

						b2BodyDef doorBodyDef;
						doorBodyDef.userData = (void*)door;
						doorBodyDef.type = b2_staticBody;
						doorBodyDef.position = b2Vec2( 0, 0 );
				
					//	list<b2Vec2> doorPoints;
				
						node = node->first_node();
						string points = node->first_attribute()->value();

						b2Vec2 doorPoints[2];
						uint32 dIndex = 0;
						stringstream pointStream( points );
						int32 px=0,py=0;
						while( pointStream >> px )
						{
							if( pointStream.peek() == ',' )
								pointStream.ignore();

							pointStream >> py;

							px += x;
							py += y;


							doorPoints[dIndex].Set( px * SF2BOX, py * SF2BOX );
							++dIndex;
							//doorPoints.push_back( b2Vec2( px * SF2BOX, py * SF2BOX) );
							//cout << "door points: " << px * SF2BOX << ", " << py * SF2BOX << endl;
						}

						assert( dIndex <= 2 );

						b2Vec2 doorDiff( doorPoints[1].x - doorPoints[0].x, doorPoints[1].y - doorPoints[0].y );
						float doorAngle = atan2( doorDiff.y, doorDiff.x );
						door->angle = doorAngle;
						//b2Vec2 *doorPointArray = new b2Vec2[doorPoints.size()];
						//uint32 di = 0;
						//for( list<b2Vec2>::iterator dIt = doorPoints.begin(); dIt != doorPoints.end(); 
						//	++dIt, ++di )
						//{
							//worldcenter + position
						//	doorPointArray[di] = (*dIt);
						//}
						//b2ChainShape chainShape;
						//chainShape.CreateChain( doorPointArray, doorPoints.size() );
						b2EdgeShape edgeShape;
						edgeShape.Set( doorPoints[0], doorPoints[1] );
					
						b2FixtureDef doorFixtureDef;
						//doorFixtureDef.userData = door;
						doorFixtureDef.shape = &edgeShape;

						CollisionLayers::SetupFixture( CollisionLayers::Door, doorFixtureDef.filter.categoryBits, 
							doorFixtureDef.filter.maskBits );

						door->body = world->CreateBody( &doorBodyDef );

						door->body->CreateFixture( &doorFixtureDef );

								
						node = node->parent();

						if( node->next_sibling() != NULL 
							&& string(node->next_sibling()->name()) == "object" )
						{
							node = node->next_sibling();
							continue;
						}
						else
						{
							break;
						}
					}

					node = node->parent();
				}
			}
		}


		for( list<Room*>::iterator rIt = rooms.begin(); rIt != rooms.end(); ++rIt )
		{	
			if( (*rIt)->ownerName != "" )
			{
				(*rIt)->owner= GetRoomByName( (*rIt)->ownerName );
			}
		}

		
		b2BodyDef mapBodyDef;
		mapBodyDef.type = b2_staticBody;
		mapBodyDef.position = b2Vec2( 0, 0 );
			
		mapBody = world->CreateBody( &mapBodyDef );

		for( int x = 0; x < stageWidth; ++x )
		{
			for( int y = 0; y < stageHeight; ++y )
			{
				//if empty, continue
				if( staticTileSets[x][y] == NULL | tileSetMap[staticTileSets[x][y]] == NULL )
					continue;


				list<list<b2Vec2>> &localChains =  tileSetMap[staticTileSets[x][y]][staticLocalID[x][y]];

				list<list<b2Vec2>> *chainsLeft = NULL, *chainsUpLeft = NULL, *chainsUp = NULL, 
					*chainsUpRight = NULL, *chainsRight = NULL, *chainsDownRight = NULL, *chainsDown = NULL, 
					*chainsDownLeft = NULL;
				if( x > 0 ) chainsLeft = &tileSetMap[staticTileSets[x-1][y]][staticLocalID[x-1][y]];
				if( x > 0 && y > 0 ) chainsUpLeft = &tileSetMap[staticTileSets[x-1][y-1]][staticLocalID[x-1][y-1]];
				if( y > 0 ) chainsUp = &tileSetMap[staticTileSets[x][y-1]][staticLocalID[x][y-1]];
				if( x < stageWidth - 1 && y > 0 ) chainsUpRight = 
					&tileSetMap[staticTileSets[x+1][y-1]][staticLocalID[x+1][y-1]];
				if( x < stageWidth - 1 ) chainsRight = &tileSetMap[staticTileSets[x+1][y]][staticLocalID[x+1][y]];
				if( x < stageWidth - 1 && y < stageHeight - 1 ) chainsDownRight = 
					&tileSetMap[staticTileSets[x+1][y+1]][staticLocalID[x+1][y+1]];
				if( y < stageHeight - 1 ) chainsDown = &tileSetMap[staticTileSets[x][y+1]][staticLocalID[x][y+1]];
				if( x > 0 && y < stageHeight - 1 ) chainsDownLeft = 
					&tileSetMap[staticTileSets[x-1][y+1]][staticLocalID[x-1][y+1]];

				list<list<b2Vec2>> surroundings;
				

				for( list<list<b2Vec2>>::iterator localChainsIt = localChains.begin(); 
					localChainsIt != localChains.end(); ++localChainsIt )
				{
					list<b2Vec2> chain = (*localChainsIt);
					list<b2Vec2> boxChain;
					for( list<b2Vec2>::iterator chainIt = chain.begin(); chainIt != chain.end(); ++chainIt )
					{
						b2Vec2 finalVector = (*chainIt );
						finalVector *= SF2BOX;
						finalVector += b2Vec2( x, y );
						boxChain.push_back( finalVector );
					}



					b2Vec2 *boxChainArray = new b2Vec2[boxChain.size()];
					int i = 0;
					for( list<b2Vec2>::iterator boxChainIt = boxChain.begin(); boxChainIt != boxChain.end(); 
						++boxChainIt, ++i )
					{
						//worldcenter + position
						boxChainArray[i] = (*boxChainIt);
					}

					b2ChainShape chainShape;
					if( boxChain.front() == boxChain.back() )
					{
						chainShape.CreateLoop( boxChainArray, boxChain.size() );
					}
					else
					{
						chainShape.CreateChain( boxChainArray, boxChain.size() );
						bool prevSet = false;
						bool nextSet = false;
						//cout << "before: " << chainShape.m_hasPrevVertex << ", " << chainShape.m_hasNextVertex << endl;
						b2Vec2 bestPrev;
						b2Vec2 bestNext;
						if (chainsUp != NULL) GhostVertexCheck( *chainsUp, chainShape, x, y - 1, boxChain, bestPrev, bestNext );
						if (chainsDown != NULL) GhostVertexCheck( *chainsDown, chainShape, x, y + 1, boxChain, bestPrev, bestNext );
						if (chainsLeft != NULL) GhostVertexCheck( *chainsLeft, chainShape, x - 1, y, boxChain, bestPrev, bestNext );
						if (chainsRight != NULL) GhostVertexCheck( *chainsRight, chainShape, x + 1, y, boxChain, bestPrev, bestNext );
						if (chainsUpLeft != NULL) GhostVertexCheck( *chainsUpLeft, chainShape, x - 1, y - 1, boxChain, bestPrev, bestNext );
						
						if (chainsUpRight != NULL) GhostVertexCheck( *chainsUpRight, chainShape, x + 1, y - 1, boxChain, bestPrev, bestNext );
						
						if (chainsDownRight != NULL) GhostVertexCheck( *chainsDownRight, chainShape, x + 1, y + 1, boxChain, bestPrev, bestNext );
						
						if (chainsDownLeft != NULL) GhostVertexCheck( *chainsDownLeft, chainShape, x - 1, y + 1, boxChain, bestPrev, bestNext );
						//cout << "after: " << chainShape.m_hasPrevVertex << ", " << chainShape.m_hasNextVertex << endl;
						//b2Vec2 next = 
					}
					delete [] boxChainArray;
					
					b2FixtureDef fd;
					fd.shape = &chainShape;
					CollisionLayers::SetupFixture( CollisionLayers::Environment, fd.filter.categoryBits, 
						fd.filter.maskBits );
					fd.userData = new b2Vec2( x, y );
					//fd.userData = (void*)(staticTileSets[x][y]->GetGID( staticLocalID[x][y] ));

					b2Fixture *tileChainFixture = mapBody->CreateFixture( &fd );
				}

			}
		}

		for( map< TileSet*, list< list< b2Vec2 > >* >::iterator tileSetMapIt = tileSetMap.begin(); 
			tileSetMapIt != tileSetMap.end(); ++tileSetMapIt )
		{
			delete [] (*tileSetMapIt).second;

			//new list<list<b2Vec2>>[tileCount]
		}

		//map< TileSet*, list< list< b2Vec2 > >* > tileSetMap;
		/*list<list<b2Vec2>> worldChains;
		bool inserted = false;
		for( int x = 0; x < stageWidth; ++x )
		{
			for( int y = 0; y < stageHeight; ++y )
			{
				if( staticTileSets[x][y] == NULL | tileSetMap[staticTileSets[x][y]] == NULL )
					continue;

				list<list<b2Vec2>> &setChains =  tileSetMap[staticTileSets[x][y]][staticLocalID[x][y]];

				for( list<list<b2Vec2>>::iterator it =setChains.begin(); it != setChains.end(); ++it )
				{
					list<b2Vec2> chainCopyWorld;
					for( list<b2Vec2>::iterator it2 = (*it).begin(); it2 != (*it).end(); ++it2 )
					{
						b2Vec2 temp = (*it2);
						temp *= SF2BOX;
						
						temp.x += x;
						temp.y += y;
						chainCopyWorld.push_back( temp );
					}
					b2Vec2 &start = chainCopyWorld.front();
					b2Vec2 &end = chainCopyWorld.back();
					inserted = false;
					for( list<list<b2Vec2>>::iterator worldChainIt = worldChains.begin(); 
						worldChainIt != worldChains.end() && !inserted; ++worldChainIt )
					{
						b2Vec2 &worldStart = (*worldChainIt).front();
						b2Vec2 &worldEnd = (*worldChainIt).back();

						if( (start - worldStart).Length() <= 1 * SF2BOX )
						{
							inserted = true;
							//start at index 1 because we are getting rid of my start
							list<b2Vec2>::iterator sewIt = chainCopyWorld.begin();
							++sewIt;
							for( ; sewIt != chainCopyWorld.end(); ++sewIt )
							{
								(*worldChainIt).push_front( (*sewIt) );
							}
						}
						else if( (end - worldStart).Length() <= 1 * SF2BOX )
						{
							inserted = true;
							//start at index 1 because we are getting rid of my end
							list<b2Vec2>::reverse_iterator sewIt = chainCopyWorld.rbegin();
							++sewIt;
							for( ; sewIt != chainCopyWorld.rend(); ++sewIt )
							{
								(*worldChainIt).push_front( (*sewIt) );
							}
						}
						else if( (start - worldEnd).Length() <= 1 * SF2BOX )
						{
							inserted = true;
							//start at index 1 because we are getting rid of my end
							list<b2Vec2>::iterator sewIt = chainCopyWorld.begin();
							++sewIt;
							for( ; sewIt != chainCopyWorld.end(); ++sewIt )
							{
								(*worldChainIt).push_back( (*sewIt) );
							}
						}
						else if( (end - worldEnd).Length() <= 1 * SF2BOX )
						{
							inserted = true;
							//start at index 1 because we are getting rid of my end
							list<b2Vec2>::reverse_iterator sewIt = chainCopyWorld.rbegin();
							++sewIt;
							for( ; sewIt != chainCopyWorld.rend(); ++sewIt )
							{
								(*worldChainIt).push_back( (*sewIt) );
							}
						}
					}

					if( !inserted )
					{
						worldChains.push_back( chainCopyWorld );
					}
					//^^currently working on sewing together the various edge shapes 
					//^^into chain shapes which I can add to the level
				}
			}
		}

		for( map< TileSet*, list< list< b2Vec2 > >* >::iterator it = tileSetMap.begin(); 
			it != tileSetMap.end(); ++it )
		{
			delete [] (*it).second;
		}

		for( list<list<b2Vec2>>::iterator it = worldChains.begin(); it != worldChains.end(); ++it )
		{
			list<b2Vec2>::iterator pruneIt = (*it).begin();
			++pruneIt;
			++pruneIt;
			while( pruneIt != (*it).end() )
			{
				b2Vec2 &next = (*pruneIt);
				--pruneIt;
				b2Vec2 &current = (*pruneIt);
				--pruneIt;
				b2Vec2 &prev = (*pruneIt);

				b2Vec2 abvec = next - current;
				abvec.x = abvec.x;
				abvec.y = abvec.y;
				abvec.Normalize();

				b2Vec2 bcvec = current - prev;
				bcvec.x = bcvec.x;
				bcvec.y = bcvec.y;
				bcvec.Normalize();

				++pruneIt;


				if( abvec == bcvec )
				{
					pruneIt = (*it).erase( pruneIt );
				}
				else
				{
					++pruneIt;
				}
				++pruneIt;
			}
		}

		bool loop = false;
		for( list<list<b2Vec2>>::iterator it = worldChains.begin(); it != worldChains.end(); ++it )
		{
			loop = false;
			b2Vec2 &start = (*it).front();
			b2Vec2 &end = (*it).back();
			if( (start - end).Length() <= 1 * SF2BOX )
			{
				(*it).pop_back();
				loop = true;
			}

			//cout << "chain: ";
			for( list<b2Vec2>::iterator vecIt = (*it).begin(); vecIt != (*it).end(); ++vecIt )
			{
				//cout << "(" << (*vecIt).x << ", " << (*vecIt).y << "), ";
			}
			//cout << endl << endl;


			b2Vec2 *chain = new b2Vec2[(*it).size()];
			int i = 0;
			for( list<b2Vec2>::iterator vecIt = (*it).begin(); vecIt != (*it).end(); ++vecIt, ++i )
			{
				//worldcenter + position
				chain[i] = (*vecIt );
			}

			b2ChainShape c;
			if( loop )
			{
				c.CreateLoop( chain, (*it).size() );
			}
			else
			{
				c.CreateChain( chain, (*it).size() );
			}
			delete [] chain;


			b2BodyDef mapBodyDef;
			mapBodyDef.type = b2_staticBody;
			mapBodyDef.position = b2Vec2( 0, 0 );
			
			b2Body *b = world->CreateBody( &mapBodyDef );
			b2FixtureDef fd;
		//	fd.friction = 1;
			fd.shape = &c;
			CollisionLayers::SetupFixture( CollisionLayers::Environment, fd.filter.categoryBits, 
				fd.filter.maskBits );

			b2Fixture *f = b->CreateFixture( &fd );
		}*/




	}
	else
		cout << "no file found: " << dir + name + ".tmx"  << endl;

	if( layers.empty() )
	{
		layers.push_back( NULL );
	}
}

void Stage::GhostVertexCheck( list<list<b2Vec2>> &chainList, b2ChainShape &chainShape, int x, int y, 
	list<b2Vec2> &boxChain, b2Vec2 &bestPrev, b2Vec2 &bestNext )
{
	b2Vec2 tile( x, y );
	for( list<list<b2Vec2>>::iterator chainListIt = chainList.begin(); chainListIt != chainList.end(); ++chainListIt )
	{
		//check the front of each chain to see if it matches my front or back
		b2Vec2 pos = (*chainListIt).front();
		pos *= SF2BOX;
		pos +=  tile;
		b2Vec2 pos2 = (*chainListIt).back();
		pos2 *= SF2BOX;
		pos2 += tile;
		
		list<b2Vec2>::iterator boxIt = boxChain.begin();
		b2Vec2 secondBoxPoint = (*++boxIt);
		b2Vec2 frontDir = boxChain.front() - secondBoxPoint;
		frontDir.Normalize();
		list<b2Vec2>::reverse_iterator boxReverseIt = boxChain.rbegin();
		b2Vec2 secondToLastBoxPoint = (*++boxReverseIt);
		b2Vec2 backDir = boxChain.back() - secondToLastBoxPoint;
		backDir.Normalize();

		if( pos == boxChain.front() || pos == boxChain.back() )
		{
			list<b2Vec2>::iterator secondIt = (*chainListIt).begin();
			b2Vec2 second = (*++secondIt);
			second *= SF2BOX;
			second += b2Vec2( x, y );
			if( pos == boxChain.front() )
			{
				b2Vec2 newGhostDir = second - pos;
				newGhostDir.Normalize();
				b2Vec2 oldGhostDir = bestPrev - pos;
				oldGhostDir.Normalize();

				if( abs( b2Cross( newGhostDir, frontDir ) ) < abs( b2Cross( oldGhostDir, frontDir ) ) )
				{
					bestPrev = newGhostDir;
				}

				
			

				//if( frontDir - 
				chainShape.SetPrevVertex( second );
			}
			else 
			{
				b2Vec2 newGhostDir = second - pos;
				newGhostDir.Normalize();
				b2Vec2 oldGhostDir = bestPrev - pos;
				oldGhostDir.Normalize();

				if( abs( b2Cross( newGhostDir, backDir ) ) < abs( b2Cross( oldGhostDir, backDir ) ) )
				{
					bestPrev = newGhostDir;
				}
				chainShape.SetNextVertex( second );
			}
				
		}
		else if( pos2 == boxChain.front() || pos2 == boxChain.back() )
		{
			list<b2Vec2>::reverse_iterator secondToLastIt = (*chainListIt).rbegin();
			b2Vec2 secondToLast = (*++secondToLastIt);
			secondToLast *= SF2BOX;
			secondToLast += b2Vec2( x, y );

			if( pos2 == boxChain.front() )
			{
				b2Vec2 newGhostDir = secondToLast - pos2;
				newGhostDir.Normalize();
				b2Vec2 oldGhostDir = bestPrev - pos2;
				oldGhostDir.Normalize();

				if( abs( b2Cross( newGhostDir, frontDir ) ) < abs( b2Cross( oldGhostDir, frontDir ) ) )
				{
					bestPrev = newGhostDir;
				}
				chainShape.SetPrevVertex( secondToLast );
			}
			
			else
			{
				b2Vec2 newGhostDir = secondToLast - pos2;
				newGhostDir.Normalize();
				b2Vec2 oldGhostDir = bestPrev - pos2;
				oldGhostDir.Normalize();

				if( abs( b2Cross( newGhostDir, backDir ) ) < abs( b2Cross( oldGhostDir, backDir ) ) )
				{
					bestPrev = newGhostDir;
				}
				//chainShape.SetPrevVertex( secondToLast );
				chainShape.SetNextVertex( secondToLast );
			}
			
		}
							
	}
}

Stage::~Stage()
{
	lua_close( L );

	//+ only have the check for player because its being deleted in Run
	
	//delete L;
	b2Fixture *tileFix = mapBody->GetFixtureList();
	while( tileFix != NULL )
	{
		b2Vec2 * tempVec = (b2Vec2*)(tileFix->GetUserData());
		delete tempVec;
		tileFix = tileFix->GetNext();
	}
	delete world;

	for( list<ImageLayer*>::iterator it = layers.begin(); it != layers.end(); ++it )
	{
		if( (*it) != NULL )
			delete (*it);
	}

	for( int i = 0; i < stageWidth; ++i )
	{
		delete [] staticLocalID[i];
		delete [] staticTileSets[i];
		delete [] staticAirTiles[i];
	}
	delete [] staticLocalID;
	delete [] staticTileSets;
	delete [] staticAirTiles;

	for( int i = 0; i < tileSetCount; ++i )
	{
		delete tileSets[i]->texture;
		delete tileSets[i];
	}
	delete tileSets;
	delete background; //also need to delete background's texture

	for( map<string, lua_State*>::iterator it = collisionHandlers.begin(); it != collisionHandlers.end(); ++it )
	{
		lua_close( (*it).second );
		//delete (*it).second;
	}

	/*staticLocalID = new uint32*[stageWidth];
		staticTileSets = new TileSet**[stageWidth];
		staticAirTiles = new uint32*[stageWidth];
		for( int i = 0; i < stageWidth; ++i )
		{
			staticLocalID[i] = new uint32[stageHeight];
			staticTileSets[i] = new TileSet*[stageHeight];
			staticAirTiles[i] = new uint32[stageHeight];

			for( int j = 0; j < stageHeight; ++j )
			{
				staticTileSets[i][j] = NULL;
			}
		}*/



	//delete [] events;
}

void Stage::SetRoomByName( const std::string &roomName )
{
	for( list<Room*>::iterator it = rooms.begin(); it != rooms.end(); ++it )
	{
		if( (*it)->name == roomName )
		{
			newRoom = (*it);
		}
	}
}

Room * Stage::GetRoomByName( const std::string &roomName )
{
	for( list<Room*>::iterator it = rooms.begin(); it != rooms.end(); ++it )
	{
		if( (*it)->name == roomName )
		{
			return (*it);
		}
	}
}

void Stage::SetRoom( Room *room )
{
	newRoom = room;
}

//returns false when the game is over
bool Stage::UpdatePrePhysics()
{
	if( newRoom != NULL )
	{
		currentRoom->Exit();
		currentRoom = newRoom;
		currentRoom->Enter( currentDoor->name );
		newRoom = NULL;
	}


	//changing from one room to the next
	if( currentDoor != NULL )
	{
		if( !currentDoor->open && !currentDoor->collide )
		{
			currentDoor->open = true;
		}
		currentDoor->collide = false;
	}
	
	consumed.clear();


	std::list<TrueActor*> oldAddedActors;
	for( std::list<TrueActor*>::iterator it = addedActors.begin(); it != addedActors.end(); ++it )
	{
		oldAddedActors.push_back( (*it) );
	}
	addedActors.clear();

	for( std::list<TrueActor*>::iterator it = oldAddedActors.begin(); it != oldAddedActors.end(); ++it )
	{
		(*it)->Init( world );
		activeActors.push_back( (*it) );
	}


	//use AABB of the player to get which air tiles he is touching
	list<uint32> activeAirTiles;
	
	uint32 finalX = player->GetWorldRight();
	uint32 finalY = player->GetWorldBottom();

	bool found;
	for( uint32 currentX = player->GetWorldLeft(); currentX <= finalX; ++currentX )
	{
		for( uint32 currentY = player->GetWorldTop(); currentY <= finalY; ++currentY )
		{
			found = false;
			uint32 test = currentY;
			for( list<uint32>::iterator it = activeAirTiles.begin(); it != activeAirTiles.end() 
				&& !found; ++it )
			{
				if( (*it) == staticAirTiles[currentX][currentY] )
					found = true;
			}

			if( !found ) activeAirTiles.push_back( staticAirTiles[currentX][currentY] );
		}
	}

	for( list<uint32>::iterator it = activeAirTiles.begin(); it != activeAirTiles.end(); ++it )
	{
		string & handler = airHandlers[(*it)];
		lua_getglobal( L, handler.c_str() );
		lua_pcall( L, 0, 0, 0 );
	}


	//sf::Clock colClock;
	for( std::list<TrueActor*>::iterator it = activeActors.begin(); it != activeActors.end(); ++it )
	{
		(*it)->ProcessCollisions();
		//cout << (*it)->GetPosition().x << (*it)->GetPosition().y << endl;
	}
	//cout << "coltime: " << colClock.getElapsedTime().asSeconds() << endl;

	

	

	//cout << "size: " << activeActors.size() << endl;


	
	sf::Clock testClock;
	uint32 maxTime = 0;
	//string maxType;

	
	for( std::list<TrueActor*>::iterator it = activeActors.begin(); it != activeActors.end();)
	{
		//if( (*it) != player )
		//{
			if( !(*it)->isAlive )
			{
				if( (*it) != player )
				{
					TrueActor *a = (*it);
					it = activeActors.erase( it );
					//cleanup and delete actor
					//delete a;

					//could be a way to make this more efficient. I need to have this so the room doesn't double-delete the actors
					//when you leave the room
					for( list<TrueActor*>::iterator rIt = currentRoom->actors.begin(); rIt != currentRoom->actors.end(); )
					{
						if( (*rIt) == a )
						{
							//TrueActor *ra = (*rIt);
							rIt = currentRoom->actors.erase( rIt );
						}
						else
						{
							++rIt;
						}
					}

					delete a;

					//world->DestroyBody( a->m_body );
				}
				else
				{
					if( lives > 0 )
					{
						RoomRestart();

						--lives;
					}
					else
					{
						LevelRestart();
						lives = 3;
					}

					++testPlayerDeathCount;
					(*it)->isAlive = true;
					(*it)->UpdatePrePhysics();
					++it;
				}
				//delete a;
			}
			else
			{
				//^^ this can't remain in the game (the timer)
				if( (*it) != player )
				{
					if( !(*it)->IsPaused() && !((*it)->isGroup) || (*it)->isGroup )
					{
					
					
						//lua_gc( (*it)->L, LUA_GCCOLLECT, 0 );
						uint32 temp;
						//lua_gc( (*it)->L, LUA_GCSTEP, 20 );
						sf::Clock testClock2;

					
						(*it)->UpdatePrePhysics();
						temp = testClock2.getElapsedTime().asMicroseconds();
						//float temp = testClock2.getElapsedTime().asSeconds();
						if( temp > maxTime )
						{
							maxTime = temp;
							actorTimeTestActor = (*it);
						}
						//if( temp > maxTime )
						//{
						//	maxTime = temp;
						//	maxType = (*it)->actorType;
						//}
					
					}
				}
				++it;
			}
	}

	if( !player->IsPaused() )
	{
		player->UpdatePrePhysics();
	}
	


	actorTimeTest = maxTime;
	//actorTimeTest = testClock.getElapsedTime().asMicroseconds();
	//cout << "max " << maxType << ": " << maxTime << endl;
	//cout << "actortime: " << testClock.getElapsedTime().asMicroseconds() << endl;
		//else ++it;
			
		//cout << (*it)->GetPosition().x << (*it)->GetPosition().y << endl;
	

	/*if( !player->isAlive )
	{
		++testPlayerDeathCount;
		player->isAlive = true;
		
	}

	player->UpdatePrePhysics();*/
	

	lua_getglobal( L, "UpdatePrePhysics" );
	lua_pcall( L, 0, 0, 0 );

	

	return true;
}

//return true if you want to exit the application. returns false if you want to return to the level menu
bool Stage::Run()
{
	sf::Text fpsText;
	fpsText.setString( "BLAH BLAH" );
	fpsText.setFont( debugFont );
	fpsText.setColor( sf::Color::Red );
	fpsText.setCharacterSize( 80 );
	//fpsText.setOrigin( fpsText.getLocalBounds().left + fpsText.getLocalBounds().width / 2.f,
	//	fpsText.getLocalBounds().top + fpsText.getLocalBounds().height / 2.f );
	fpsText.setPosition( 10, 10 );
	//fpsText.setPosition( window->getSize().x / 2, window->getSize().y / 2 - 50 * 4 + 100 * 5 );


	sf::Text deathDisplay;
	deathDisplay.setCharacterSize( 40 );
	deathDisplay.setColor( sf::Color::Red );
	deathDisplay.setStyle( sf::Text::Regular );
	sf::Text fpsDisplay;
	fpsDisplay.setCharacterSize( 50 );
	fpsDisplay.setColor( sf::Color::Yellow );
	fpsDisplay.setStyle( sf::Text::Regular );
	
	
	sf::Font debugFont;
	

	fpsDisplay.setFont( debugFont );
	deathDisplay.setFont( debugFont );

	//window->setMouseCursorVisible( false );
	ContactListener cl;
	cl.stage = this;
	world->SetContactListener( &cl );
	bool singleFrameMode = false;

	//for( std::list<Actor*>::iterator it = addedActors.begin(); it != addedActors.end(); ++it )
	//{
	//	(*it)->Init( world ;)
	//}
	//addedActors.clear();

	lua_getglobal( L, "Init" );
	lua_pcall( L, 0, 0, 0 );
	int x = layers.size();
	

	sf::Clock statClock;

	sf::Clock gameClock;
	bool quit = false;
	double currentTime = 0;
	double accumulator = TIMESTEP + .1;

	bool m_singleFrameMode = false;
	bool m_skipFrame = false;

	sf::RenderTexture rt;
	//rt.create( 1920, 1080);
	rt.create( 3840, 2160);
	rt.setSmooth( true );
	//rt.create( window->getSize().x, window->getSize().y );
	mapView.setSize( 3840, 2160 );
	sf::Vector2f sp( -1,-1 );//player->m_startPosition.x, player->m_startPosition.y );
	//sp *= BOX2SF;

	bool renderOnce = false;
	//background->setScale( 2, 2 );

	
	for( std::list<ImageLayer*>::iterator layerIt = layers.begin(); layerIt != layers.end(); ++layerIt )
	{
		if( (*layerIt) != NULL )
		{
			(*layerIt)->view.setSize( mapView.getSize() );
		}
	}

	uint32 oldfps = 0;
	uint32 fps = 0;
	uint32 fpsCounter = 0;

	//sf::VertexArray map( sf::Quads, stageWidth * stageHeight * 4 );

/*	if( staticTileSets[x][y] != NULL )
				{
					//(staticTileSets[x][y]->texture)->setSmooth( false );
					tiles->setTexture( *(staticTileSets[x][y]->texture) );
					tiles->setTextureRect( staticTileSets[x][y]->GetSubRect( staticLocalID[x][y] ) );
					//tiles->setPosition( ( x - leftTile ) * tileSize , ( y - topTile ) * tileSize );
					tiles->setPosition( ( x  ) * tileSize , ( y ) * tileSize );
					//window->draw( *tiles );
					rt.draw( *tiles );
				}*/

	std::list<sf::Texture*> texs;
	{
		for( int x = 0; x < stageWidth; ++x )
		{
			for( int y = 0; y < stageHeight; ++y )
			{
				if( staticTileSets[x][y] != NULL )
				{
					bool found = false;
					for( list<sf::Texture*>::iterator it = texs.begin(); it != texs.end() && !found; ++it )
					{
						if( staticTileSets[x][y]->texture == (*it) )
						{
							found = true;
						}
					}
					if( !found ) texs.push_back( staticTileSets[x][y]->texture );
				}
			}
		}
	}


	int viewHalfWidthTiles = 64;//32;//16;
	int viewHalfHeightTiles = 64;//40;//20;//10 ;
	std::map< sf::Texture*, sf::VertexArray* > texVertexMap;
	
	for( list<sf::Texture*>::iterator it = texs.begin(); it != texs.end(); ++it )
	{
		//+ dealloacate the memory here later
		texVertexMap[(*it)] = new sf::VertexArray( sf::Quads, viewHalfWidthTiles * 2 * viewHalfHeightTiles * 2 * 4 );//viewHalfWidthTiles * viewHalfHeightTiles * 4 );
	}

	

	/*for( int tx = 0; tx < stageWidth; ++tx )
	{
		for( int ty = 0; ty < stageHeight; ++ty )
		{
			if( staticTileSets[tx][ty] != NULL )
			{
			int tc = ( tx + ty * stageWidth ) * 4;
			// define the position of the 4 points of the current tile 
			map[tc + 0].position = sf::Vector2f((tx + 0) * tileSize, (ty + 0) * tileSize); 
			map[tc + 1].position = sf::Vector2f((tx + 0) * tileSize, (ty + 1) * tileSize); 
			map[tc + 2].position = sf::Vector2f((tx + 1) * tileSize, (ty + 1) * tileSize); 
			map[tc + 3].position = sf::Vector2f((tx + 1) * tileSize, (ty + 0) * tileSize); 

			// define the texture coordinates of the 4 points of the current tile 
			int ix = staticTileSets[tx][ty]->GetSubRect( staticLocalID[tx][ty] ).left / tileSize; // X index of the tile in the tileset 
			int iy = staticTileSets[tx][ty]->GetSubRect( staticLocalID[tx][ty] ).top / tileSize; // Y index of the tile in the tileset ; 
			map[tc+ 0].texCoords = sf::Vector2f((ix + 0) * tileSize, (iy + 0) * tileSize); 
			map[tc+ 1].texCoords = sf::Vector2f((ix + 0) * tileSize, (iy + 1) * tileSize); 
			map[tc+ 2].texCoords = sf::Vector2f((ix + 1) * tileSize, (iy + 1) * tileSize); 
			map[tc+ 3].texCoords = sf::Vector2f((ix + 1) * tileSize, (iy + 0) * tileSize); 
			//map[tc+0].
			}
		}
	}*/
	std::list<TrueActor*> oldAddedActors;

	
	for( std::list<TrueActor*>::iterator it = addedActors.begin(); it != addedActors.end(); ++it )
	{
		oldAddedActors.push_back( (*it) );
		//(*it)->Init( world );
		//activeActors.push_back( (*it) );
	}
	addedActors.clear();

	for( std::list<TrueActor*>::iterator it = oldAddedActors.begin(); it != oldAddedActors.end(); ++it )
	{
		//oldAddedActors.push_back( (*it) );
		(*it)->Init( world );
		activeActors.push_back( (*it) );
	}
	
/*	sf::Text testText;
	std::stringstream ss2; 
	ss2 << "HELLO";
	//testText.setString( ss2.str() );
	testText.setString( "HELLO" );
	testText.setCharacterSize( 10000 );
	testText.setFont( debugFont );
	testText.setPosition( pauseMenuTexts[3].getPosition() );
	testText.setColor( sf::Color::Red );
	testText.setOrigin( testText.getLocalBounds().left + testText.getLocalBounds().width/2.f,
	testText.getLocalBounds().top + testText.getLocalBounds().height/2.f );*/

	

	while ( !quit )
    {
		double newTime = gameClock.getElapsedTime().asSeconds();
		double frameTime = newTime - currentTime;

	//	if( frameTime > .0167 )
		//cout << "frametime: "  << frameTime * 100000 << endl;

		if ( frameTime > 0.25 )
			frameTime = 0.25;	
        currentTime = newTime;

		accumulator += frameTime;

		window->clear();
	//	if( !m_singleFrameMode || m_skipFrame )
	//	cout << "accum: " << accumulator << ", frametime: " << frameTime << endl;
		//cout << "times: " << (int)(accumulator / TIMESTEP ) << endl;
        while ( accumulator >= TIMESTEP  )
        {
			//sf::Clock ttClock;
			prevInput = currentInput;

			
			if( !controller.UpdateState() )
			{
				bool up = Keyboard::isKeyPressed( Keyboard::Up ) || Keyboard::isKeyPressed( Keyboard::W );
				bool down = Keyboard::isKeyPressed( Keyboard::Down ) || Keyboard::isKeyPressed( Keyboard::S );
				bool left = Keyboard::isKeyPressed( Keyboard::Left ) || Keyboard::isKeyPressed( Keyboard::A );
				bool right = Keyboard::isKeyPressed( Keyboard::Right ) || Keyboard::isKeyPressed( Keyboard::D );

				ControllerState keyboardInput;    
				keyboardInput.B = Keyboard::isKeyPressed( Keyboard::X ) || Keyboard::isKeyPressed( Keyboard::Period );
				keyboardInput.X = Keyboard::isKeyPressed( Keyboard::C ) || Keyboard::isKeyPressed( Keyboard::Comma );
				keyboardInput.Y = Keyboard::isKeyPressed( Keyboard::V ) || Keyboard::isKeyPressed( Keyboard::M );
				keyboardInput.A = Keyboard::isKeyPressed( Keyboard::Z ) || Keyboard::isKeyPressed( Keyboard::Space ) || Keyboard::isKeyPressed( Keyboard::Slash );
				keyboardInput.start = Keyboard::isKeyPressed( Keyboard::Dash );
				keyboardInput.back = Keyboard::isKeyPressed( Keyboard::Equal );
				
				
				if( up && down )
				{
					if( prevInput.Up() )
						keyboardInput.pad += 1;
					else if( prevInput.Down() )
						keyboardInput.pad += ( 1 && down ) << 1;
				}
				else
				{
					keyboardInput.pad += 1 && up;
					keyboardInput.pad += ( 1 && down ) << 1;
				}

				if( left && right )
				{
					if( prevInput.Left() )
					{
						keyboardInput.pad += ( 1 && left ) << 2;
					}
					else if( prevInput.Right() )
					{
						keyboardInput.pad += ( 1 && right ) << 3;
					}
				}
				else
				{
					keyboardInput.pad += ( 1 && left ) << 2;
					keyboardInput.pad += ( 1 && right ) << 3;
				}

				currentInput = keyboardInput;
			}
			
			else
			{
				currentInput = controller.GetState();
				currentInput.X = currentInput.X || currentInput.rightShoulder;
				//currentInput.rightShoulder |= currentInput.rightTrigger > 10;
				currentInput.Y = currentInput.Y || currentInput.leftShoulder;
				currentInput.leftShoulder = currentInput.leftTrigger > 10;
				currentInput.B |= currentInput.rightTrigger > 10;
				
				//bool temp = currentInput.X;
				//currentInput.X = currentInput.leftShoulder;
				//currentInput.leftShoulder = temp;
				if( currentInput.leftStickMagnitude > .4 )
				{
					//cout << "left stick radians: " << currentInput.leftStickRadians << endl;
					float x = cos( currentInput.leftStickRadians );
					float y = sin( currentInput.leftStickRadians );
					float threshold = .4;
					if( x > threshold )
						currentInput.pad += 1 << 3;
					if( x < -threshold )
						currentInput.pad += 1 << 2;
					if( y > threshold )
						currentInput.pad += 1;
					if( y < -threshold )
						currentInput.pad += 1 << 1;
					//cout << "x: " << x << endl;
					//cout << "y: " << y << endl; 
				}
			}

			

			//eventually i will get rid of this
			if( currentInput.start ) //&& !prevInput.start )
			{
				
				
				if( currentInput.Left() )
				{
					currentRoom->spawn.x -= .5;
				}
				else if( currentInput.Right() )
				{
					currentRoom->spawn.x += .5;
				}
				
				if( currentInput.Up() )
				{
					currentRoom->spawn.y -= .5;
				}
				else if( currentInput.Down() )
				{
					currentRoom->spawn.y += .5;
				}

				RoomRestart();
				
			
			}

			if( currentInput.back && !prevInput.back )
			{
				
				if( !m_singleFrameMode )
				{
					selectedPauseMenu = 0;
				}
				m_singleFrameMode = !m_singleFrameMode;
			}
				
			if( m_singleFrameMode )
			{
				renderOnce = true;

				if( currentInput.leftShoulder && !prevInput.leftShoulder || currentInput.rightShoulder )
				{
					m_skipFrame = true;
					prevInput = storedInput;
				}

				if( !m_skipFrame )
				{
					if( currentInput.Down() && !prevInput.Down() )
					{
						if( selectedPauseMenu == 4 )
						{
							selectedPauseMenu = 0;
						}
						else if( selectedPauseMenu == 0 && lives == 0 )
						{
							selectedPauseMenu = 2;
						}
						else
						{
							++selectedPauseMenu;
						}
					}
					else if( currentInput.Up() && !prevInput.Up() )
					{
						if( selectedPauseMenu == 0 )
						{
							selectedPauseMenu = 4;
						}
						else if( selectedPauseMenu == 2 && lives == 0 )
						{
							selectedPauseMenu = 0;
						}
						else
						{
							--selectedPauseMenu;
						}
					}
					else if( currentInput.A && !prevInput.A )
					{
						if( pauseMenuTexts[selectedPauseMenu].getString() == "Resume" )
						{
							m_singleFrameMode = false;
							continue;
							//currentInput.A = false;
						}
						else if( pauseMenuTexts[selectedPauseMenu].getString() == "Restart Room (Lose 1 life)" && lives > 0 )
						{
							--lives;
							RoomRestart();
							m_singleFrameMode = false;
							continue;
						}
						else if( pauseMenuTexts[selectedPauseMenu].getString() == "Restart Level (Full lives)" )
						{
							lives = 3;
							LevelRestart();
							m_singleFrameMode = false;
							continue;
							//currentInput.A = false;
							//respawn all the actors and rooms but don't reload the static stuff like the
							//level. this needs to be as fast as possible.
						}
						else if( pauseMenuTexts[selectedPauseMenu].getString() == "Quit to Level Menu" )
						{
							delete player;

							//+ only check for player because of the manual delete
							for( list<TrueActor*>::iterator it = activeActors.begin(); it != activeActors.end(); ++it )
							{
								if( (*it) != player )
									delete (*it);
							}

							for( list<TrueActor*>::iterator it = addedActors.begin(); it != addedActors.end(); ++it )
							{
								if( (*it) != player )
									delete(*it);
							}

							for( list<sf::Texture*>::iterator it = texs.begin(); it != texs.end(); ++it )
							{
								delete texVertexMap[(*it)];
								//+ dealloacate the memory here later
								//texVertexMap[(*it)] = new sf::VertexArray( sf::Quads, viewHalfWidthTiles * 2 * viewHalfHeightTiles * 2 * 4 );//viewHalfWidthTiles * viewHalfHeightTiles * 4 );
							}

							return false;
						}
						else if( pauseMenuTexts[selectedPauseMenu].getString() ==  "Quit Application" )
						{
							delete player;

							//+ only check for player because of the manual delete
							for( list<TrueActor*>::iterator it = activeActors.begin(); it != activeActors.end(); ++it )
							{
								if( (*it) != player )
									delete (*it);
							}

							for( list<TrueActor*>::iterator it = addedActors.begin(); it != addedActors.end(); ++it )
							{
								if( (*it) != player )
									delete(*it);
							}

							for( list<sf::Texture*>::iterator it = texs.begin(); it != texs.end(); ++it )
							{
								delete texVertexMap[(*it)];
								//+ dealloacate the memory here later
								//texVertexMap[(*it)] = new sf::VertexArray( sf::Quads, viewHalfWidthTiles * 2 * viewHalfHeightTiles * 2 * 4 );//viewHalfWidthTiles * viewHalfHeightTiles * 4 );
							}
							return true;
						}
					}
				}
			/*	const sf::View & pauseTempView = window->getView();
				window->setView( window->getDefaultView() );

				window->draw( *pauseBGSprite );
				for( int i = 0; i < 4; ++i )
				{
					window->draw( pauseMenuTexts[i] );
				}
				window->setView( pauseTempView );
				
				window->display();*/

				
			}

			if( m_singleFrameMode )
			{
				if( m_skipFrame )
				{
					m_skipFrame = false;
					storedInput = currentInput;
				}
				else
				{
					accumulator -= TIMESTEP;
					continue;
				}
			}
			
			if( player->IsReversed() )
			{
				bool tempUp = currentInput.Up();
				bool tempDown = currentInput.Down();
				if( !tempUp && tempDown )
				{
					currentInput.pad -= 2;
				}
				else if( tempUp && !tempDown )
				{
					currentInput.pad += 2;
				}
				if( !tempDown && tempUp )
				{
					currentInput.pad -= 1;
				}
				else if( tempDown && !tempUp )
				{
					currentInput.pad += 1;
				}
			}
			((PlayerChar*)player)->currentInput = currentInput;
			((PlayerChar*)player)->prevInput = prevInput;

			//controller1.UpdateState();

			//Update();
			
			//cout << "eztime: " << ttClock.getElapsedTime().asSeconds() << endl;
			sf::Clock testClock;


			UpdatePrePhysics();

			uint32 pretimetest = testClock.getElapsedTime().asMicroseconds();
			if( pretimetest > 16700 )
			cout << "pretime: " << pretimetest << endl;
			if( actorTimeTest > 1000 && actorTimeTestActor->actorType == "basicturretbullet" )
				cout << ", actortime: " << actorTimeTest << ", type: " << actorTimeTestActor->actorType << endl;
			
			
			//aa->UpdatePrePhysics();
			
			
			UpdateTileModifications();
			
			//statClock.restart();

			//aa->RefreshVelocity();

			

			/*for( std::list<Actor*>::iterator it = actors.begin(); it != actors.end(); ++it )
			{
				bool idk = (*it)->ProcessCollisions();
			}*/

			

			////update set timestep logic
			//6,2

			//testClock.restart();
			//world->Step( TIMESTEP, 10, 10 );
			
			world->Step( TIMESTEP, 6, 2 );
			//cout << "steptime: " << testClock.getElapsedTime().asSeconds()<< endl;
			world->ClearForces();
			//testClock.restart();

			//sf::Time uppsct = statClock.getElapsedTime();
		//	cout << "upp: " << uppsct.asMicroseconds() << endl;


			UpdatePostPhysics();
			//cout << "posttime: " << testClock.getElapsedTime().asSeconds()<< endl;

		//view
			
			//cout << "lives: " << lives << endl;
			

            accumulator -= TIMESTEP;
			renderOnce = true;
			//if( accumulator >= TIMESTEP ) cout << "skip: " << accumulator - TIMESTEP << endl;
			//else cout << endl;

         }


		if( !renderOnce )
		{

			//continue;
		}
		//sf::Clock drawClock;
		renderOnce = false;

		Event ev;
		if( window->pollEvent( ev ) )
		{
			if (ev.type == sf::Event::Closed)
			{
				delete player;

				//+ only check for player because of the manual delete
				for( list<TrueActor*>::iterator it = activeActors.begin(); it != activeActors.end(); ++it )
				{
					if( (*it) != player )
						delete (*it);
				}

				for( list<TrueActor*>::iterator it = addedActors.begin(); it != addedActors.end(); ++it )
				{
					if( (*it) != player )
						delete(*it);
				}

				for( list<sf::Texture*>::iterator it = texs.begin(); it != texs.end(); ++it )
				{
					delete texVertexMap[(*it)];
					//+ dealloacate the memory here later
					//texVertexMap[(*it)] = new sf::VertexArray( sf::Quads, viewHalfWidthTiles * 2 * viewHalfHeightTiles * 2 * 4 );//viewHalfWidthTiles * viewHalfHeightTiles * 4 );
				}

				return true;
			}
				
		}

		if( sf::Keyboard::isKeyPressed( sf::Keyboard::Escape ) )
		{
			delete player;

			//+ only check for player because of the manual delete
			for( list<TrueActor*>::iterator it = activeActors.begin(); it != activeActors.end(); ++it )
			{
				if( (*it) != player )
					delete (*it);
			}

			for( list<TrueActor*>::iterator it = addedActors.begin(); it != addedActors.end(); ++it )
			{
				if( (*it) != player )
					delete(*it);
			}

			for( list<sf::Texture*>::iterator it = texs.begin(); it != texs.end(); ++it )
			{
				delete texVertexMap[(*it)];
				//+ dealloacate the memory here later
				//texVertexMap[(*it)] = new sf::VertexArray( sf::Quads, viewHalfWidthTiles * 2 * viewHalfHeightTiles * 2 * 4 );//viewHalfWidthTiles * viewHalfHeightTiles * 4 );
			}

			return false;
		}

		
		if( sp.x < 0 )
		{
			sp.x = camera.x;
			sp.y = camera.y;
			sp *= BOX2SF;
			
		}
		//else
		//	cout << "sp: " << sp.x << ", " << sp.y << endl;
		
		//window->setView( view );
		//rt.setView( mapView );

		//background->setPosition( -960, -540 );
		//background->setPosition( view.getCenter() );
		//window->draw( *background, &testShader );
		//window->draw( *background );

		rt.clear( sf::Color::Transparent );
		//rt.clear( sf::Color::Blue );
		//rt.clear( sf::Color( 200, 200, 255 ) );
		
		//rt.draw( *background, &testShader );
		

		

		sf::View bgView;
		bgView.setCenter( 0, 0 );
		sf::Vector2f bgSize( 1920, 1080);
		//bgSize += viewSize - sf::Vector2f(1920, 1080);
		bgView.setSize( bgSize );
		
		//bgView.setSize( sf::Vector2f( 1920 * 2, 1080 * 2 ) + sf::Vector2f( 1920 * 2, 1080 * 2) * ( 1 - cameraZoom ) );
		//rt.setView( bgView );
		window->setView( bgView );
		
		//background->setScale( cameraZoom, cameraZoom );
		//background->setPosition( mapView.getCenter() );

		//rt.setView( bgView );
		//rt.draw( *background );
		
		window->draw( *background );
		window->setView( view );
		
		//background->setPosition( view.getCenter() );
		//rt.draw( *background );
		rt.setView( mapView );
		
		//mapView.setSize( sf::Vector2f( rt.getSize() ) * cameraZoom );
		//cout << "cameraZoom: " << cameraZoom << endl;
		sf::Vector2f viewSize( sf::Vector2f( 1920, 1080 ) * cameraZoom );
		
		//cout << "viewSize: " << viewSize.x << ", " << viewSize.y << endl;
		viewSize.x = floor( viewSize.x + .5f );
		viewSize.y = floor( viewSize.y + .5f );
		//cout << "diff: " << viewSize.x - view.getSize().x  << ", " << viewSize.y - view.getSize().y << endl;
		view.setSize( viewSize );

		window->setView( view );
		//layers behind the map
		//sf::View testView;
		//testView.setCenter( mapView.getCenter() );
		//testView.setSize( sf::Vector2f( rt.getSize() ) );
		//rt.setView( testView );

		//-1 just tests for initialization
		


		std::list<ImageLayer*>::iterator layerIt = layers.begin(); 
		for( ; layerIt != layers.end() && (*layerIt) != NULL; ++layerIt )
		{
			//(*layerIt)->view.setCenter( view.getCenter() );
		//	sf::Vector2f test( view.getCenter().x * (*layerIt)->scrollRatio, view.getCenter().y / (*layerIt)->scrollRatio );
			//(*layerIt)->view.setCenter( rt.getView().getCenter() * (*layerIt)->scrollRatio - sf::Vector2f(player->m_startPosition.x, player->m_startPosition.y ) * BOX2SF );
			
			
			if( (*layerIt)->scrollRatio != 1 )
			{
				float sr = (*layerIt)->scrollRatio;
				(*layerIt)->view.setCenter( sp + ( view.getCenter() - sp ) * sr);
				//(*layerIt)->view.setCenter( mapView.getCenter() );
				//cout << view.getCenter().x << ", " << view.getCenter().y << endl;
				//sf::Vector2f one = sf::Vector2f( 1920 * 2, 1080 * 2) * (( 2 - (1 + ( 2 - cameraZoom ) * sr ) + 1 ) ); //* ( cameraZoom - 1 ) *  ( 1 + ( cameraZoom - 1 ) * sr );
				//sf::Vector2f one = sf::Vector2f( 1920 * 2, 1080 * 2) * ( 1 +( 2 - (1 + ( 2 - cameraZoom ) * sr ) ) * ( 1 + ( 1 - ( 2 - cameraZoom ) ) )); 
				//sf::Vector2f one = sf::Vector2f( 1920 * 2, 1080 * 2) *  ( ( 2 - ( 2 - cameraZoom ) * sr ) );
				//sf::Vector2f two = sf::Vector2f( 1920 * 2, 1080 * 2) * ( 2- cameraZoom * sr );
				sf::Vector2f one = sf::Vector2f( 1920 * 2, 1080 * 2 );
				one += sf::Vector2f( 1920 * 2, 1080 * 2 ) *  ( cameraZoom - 1 ) * ( sr );
				one -= viewSize - sf::Vector2f(1920, 1080);
				
				//one -= sf::Vector2f( 1920, 1080 ) * (cameraZoom - 1 ); //(1 - cameraZoom);
				one.x = floor( one.x + .5 );
				one.y = floor( one.y + .5 );

			//	cout << "zoom: " << cameraZoom << ", viewsize: " << viewSize.x << ", " << viewSize.y << ", one: " << one.x << ", " << one.y << endl;

				//sf::Vector2f one( 1920 * 2, 1080 * 2 );
				//one = two - one;
				
				//cout << one.x << ", " << one.y << endl;
				//one -= sf::Vector2f( 1920 * 2, 1080 * 2 ) * cameraZoom * sr ;
				//one.y *= sr + 1;
				//one += sf::Vector2f( 1920, 1080 ) * cameraZoom ; 
				//one = one - 
				(*layerIt)->view.setSize( one );//- sf::Vector2f( 1920 * 2 , 1080 * 2 ) );
				//cout << "size: " << view.getSize().x << ", " << view.getSize().y << endl;
				//cout << "size: " << (*layerIt)->view.getSize().x << ", " << (*layerIt)->view.getSize().y << "), " << "camerazoom: " << cameraZoom << ", sr: " << sr << endl;
				
				//(*layerIt)->view.setSize( 
					/// ( ( (1 -(*layerIt)->scrollRatio)) * cameraZoom ) );

				rt.setView( (*layerIt)->view );
			//	window->setView( (*layerIt)->view );
				
				for( std::map<TileSet*, sf::VertexArray*>::iterator objectIt = (*layerIt)->objectMap.begin();
					objectIt != (*layerIt)->objectMap.end(); ++objectIt )
				{
					rt.draw( *(*objectIt).second, (*objectIt).first->texture );
				}
				//for( std::list<sf::Sprite*>::iterator it2 = (*layerIt)->objects.begin(); 
				//	it2 != (*layerIt)->objects.end(); ++it2 )
				///{
					//rt.draw( *(*it2) );
				//	window->draw( *(*it2) );
				//}
				//window->setView( view );
				rt.setView( view );
				//rt.display();
				
				/*rt.setView( (*layerIt)->view );
				for( std::list<sf::Sprite*>::iterator it2 = (*layerIt)->objects.begin(); 
					it2 != (*layerIt)->objects.end(); ++it2 )
				{
					rt.draw( *(*it2) );
					//window->draw( *(*it2) );
				}
				//window->setView( view );*
				rt.display();
				sf::Sprite layerSprite;
				layerSprite.setTexture( rt.getTexture() );
				layerSprite.setPosition( mapView.getCenter() );
				layerSprite.setOrigin( floor(layerSprite.getLocalBounds().width / 2), 
					floor(layerSprite.getLocalBounds().height / 2) );
				window->draw( layerSprite );
				rt.clear( sf::Color::Transparent );*/
			}
			else
			{
				(*layerIt)->view.setSize( mapView.getSize() );
				(*layerIt)->view.setCenter( mapView.getCenter() );

				rt.setView( (*layerIt)->view  );
				for( std::map<TileSet*, sf::VertexArray*>::iterator objectIt = (*layerIt)->objectMap.begin();
					objectIt != (*layerIt)->objectMap.end(); ++objectIt )
				{
					rt.draw( *(*objectIt).second, (*objectIt).first->texture );
				}
				rt.setView( mapView );
			}
			//else
			//	(*layerIt)->view.setSize( sf::Vector2f( rt.getSize() ) * ( 1 - cameraZoom ) );
			//}
			//else
			//	(*layerIt)->view.setCenter( view.getCenter() );
			
			//rt.setView( (*layerIt)->view );
			//view.zoom( .5 );
			//(*layerIt)->view.zoom( 1.001 );
			//cout << "zoom: " << rt.getSize().x / view.getSize().x << endl;
			//(*layerIt)->view.setSize( sf::Vector2f( view.getSize() ) * 1.f );
			
			//(*layerIt)->view.setCenter( test );
			//window->setView( (*layerIt)->view );
			
		}

		rt.setView( mapView );

		

		//window->setView( view );
		//rt.setView( mapView );

		
		//level display
		//sf::Sprite *tiles = new sf::Sprite;
		//int viewHalfWidthTiles = 8;
		//int viewHalfHeightTiles = 8;

		int leftTile = 0, rightTile = stageWidth, topTile = 0, bottomTile = stageHeight;
		int cameraX = camera.x;
		int cameraY = camera.y;
		if( cameraX - viewHalfWidthTiles > leftTile )
			leftTile = cameraX - viewHalfWidthTiles;
		if( cameraX + viewHalfWidthTiles < rightTile )
			rightTile = cameraX + viewHalfWidthTiles;

		if( cameraY - viewHalfHeightTiles > topTile )
			topTile = cameraY - viewHalfHeightTiles;
		if( cameraY + viewHalfHeightTiles < bottomTile )
			bottomTile = cameraY + viewHalfHeightTiles;
		
		//int testMaxTc = 0;

		/*for( map<sf::Texture*, sf::VertexArray*>::iterator clearIt = texVertexMap.begin();
			clearIt != texVertexMap.end(); ++clearIt )
		{
			VertexArray &map = *((*clearIt).second);
			for( int tx = 0; tx < viewHalfWidthTiles * 2; ++tx )
			{
				for( int ty = 0; ty < viewHalfHeightTiles * 2; ++ty )
				{
					int tc = (tx + ty * viewHalfWidthTiles * 2) * 4;
					map[tc + 0].
				}
			}
		}*/

		//leftTile = (int)player->GetPosition().x - 10;
		//rightTile = leftTile + 20;
		//topTile = (int)player->GetPosition().y - 10;
		//bottomTile = topTile + 20;
		statClock.restart();
		for( int tx = leftTile; tx < rightTile; ++tx )
		{
			for( int ty = topTile; ty < bottomTile; ++ty )
			{
				if( staticTileSets[tx][ty] != NULL )
				{
					int tc = ((tx - leftTile) + (ty - topTile) * viewHalfWidthTiles * 2) * 4;

					VertexArray &map = *(texVertexMap[staticTileSets[tx][ty]->texture]);
					// define the position of the 4 points of the current tile 
					map[tc + 0].position = sf::Vector2f((tx + 0) * tileSize, (ty + 0) * tileSize); 
					map[tc + 1].position = sf::Vector2f((tx + 0) * tileSize, (ty + 1) * tileSize); 
					map[tc + 2].position = sf::Vector2f((tx + 1) * tileSize, (ty + 1) * tileSize); 
					map[tc + 3].position = sf::Vector2f((tx + 1) * tileSize, (ty + 0) * tileSize); 

					// define the texture coordinates of the 4 points of the current tile 
					float blend = 0.5f;
					int ix = staticTileSets[tx][ty]->GetSubRect( staticLocalID[tx][ty] ).left / tileSize; // X index of the tile in the tileset 
					int iy = staticTileSets[tx][ty]->GetSubRect( staticLocalID[tx][ty] ).top / tileSize; // Y index of the tile in the tileset ; 
					map[tc+ 0].texCoords = sf::Vector2f((ix + 0) * tileSize + blend, (iy + 0) * tileSize + blend); 
					map[tc+ 1].texCoords = sf::Vector2f((ix + 0) * tileSize + blend, (iy + 1) * tileSize - blend); 
					map[tc+ 2].texCoords = sf::Vector2f((ix + 1) * tileSize - blend, (iy + 1) * tileSize - blend); 
					map[tc+ 3].texCoords = sf::Vector2f((ix + 1) * tileSize - blend, (iy + 0) * tileSize + blend); 
				}
				else
				{
				}
			}
		}
		sf::Time sct = statClock.getElapsedTime();
		
		//rt.clear(sf::Color::Red );
		

		for( std::list<TrueActor*>::iterator it = activeActors.begin(); it != activeActors.end(); ++it )
		{
			(*it)->Draw( &rt );
		}

		if( (*layerIt) == NULL )
			++layerIt;
		//layers in front of the map but behind the actor
		for( ; layerIt != layers.end() && (*layerIt) != NULL; ++layerIt )
		{
			//if( (*layerIt)->scrollRatio != 1 )
			//	(*layerIt)->view.setSize( sf::Vector2f( rt.getSize() ) * ( 1 + cameraZoom * (*layerIt)->scrollRatio ) );
			//else
			//	(*layerIt)->view.setSize( mapView.getSize() * cameraZoom * .5f );

			rt.setView( (*layerIt)->view  );
			for( std::map<TileSet*, sf::VertexArray*>::iterator objectIt = (*layerIt)->objectMap.begin();
				objectIt != (*layerIt)->objectMap.end(); ++objectIt )
			{
				rt.draw( *(*objectIt).second, (*objectIt).first->texture );
			}
		}
		rt.setView( mapView );

		

		statClock.restart();
		for( std::map<sf::Texture*, sf::VertexArray*>::iterator mapIt = texVertexMap.begin(); 
			mapIt != texVertexMap.end(); ++mapIt )
		{
			rt.draw( *(*mapIt).second, (*mapIt).first );
			//window->draw( *(*mapIt).second, (*mapIt).first );
		}

		//for( std::list<TrueActor*>::iterator it = activeActors.begin(); it != activeActors.end(); ++it )
		//{
		//	(*it)->Draw( &rt );
		//}

		//view.setSize( 1600, 900 );
		
		
		//mapView.setSize( 1920 * 2, 1080 * 2 );
		//mapView.setCenter( view.getCenter() );
		//mapView.setCenter( view.getCenter().x, view.getCenter().y );
		
		//mapView.setSize( view.getSize().x * 2, view.getSize().y * 2 );
		//mapView.setSize( 1920, 1080 );
		
		//cout << view.getCenter().x << ", " << view.getCenter().y << endl;
		
		
		sf::Time sct2 = statClock.getElapsedTime();
		//cout << "sct: " << sct.asMicroseconds() << ", sct2: " << sct2.asMicroseconds() << endl;
		//rt.setSmooth( true );
		//rt.display();
		//sf::Sprite sprdfs( rt.getTexture() );
		//window->draw( sprdfs );

		//for( int x = 0; x < stageWidth; ++x )
		/*for( int x = leftTile ; x < rightTile; ++x )
		{
			//for( int y = 0; y < stageHeight; ++y )
			for( int y = topTile; y < bottomTile; ++y )
			{
				if( staticTileSets[x][y] != NULL )
				{
					//(staticTileSets[x][y]->texture)->setSmooth( false );
					tiles->setTexture( *(staticTileSets[x][y]->texture) );
					tiles->setTextureRect( staticTileSets[x][y]->GetSubRect( staticLocalID[x][y] ) );
					//tiles->setPosition( ( x - leftTile ) * tileSize , ( y - topTile ) * tileSize );
					tiles->setPosition( ( x  ) * tileSize , ( y ) * tileSize );
					//window->draw( *tiles );
					//rt.draw( *tiles );
				}
			}
		}*/

		//rt.setSmooth( true );
		//rt.display();
		

		
		//sf::Sprite spr( rt.getTexture() );
		//spr.setOrigin( spr.getLocalBounds().width / 2.f, spr.getLocalBounds().height / 2.f );
		//spr.setPosition( camera.x * BOX2SF, camera.y * BOX2SF );

		//window->draw( spr );
		//window->draw(map, staticTileSets[0][0]->texture);
		//rt.clear();
		
		//view.setSize( 1920 * 2, 1080 * 2 );
		

		


		
		

		if( (*layerIt) == NULL )
			++layerIt;
		//layers in front of the actor and the map
		for( ; layerIt != layers.end(); ++layerIt )
		{

			(*layerIt)->view.setCenter( sp + ( view.getCenter() - sp ) * (*layerIt)->scrollRatio);
			//(*layerIt)->view.setSize( sf::Vector2f( rt.getSize() ) * ( 1 + cameraZoom * (*layerIt)->scrollRatio ) );
			rt.setView( (*layerIt)->view  );
			for( std::map<TileSet*, sf::VertexArray*>::iterator objectIt = (*layerIt)->objectMap.begin();
				objectIt != (*layerIt)->objectMap.end(); ++objectIt )
			{
				rt.draw( *(*objectIt).second, (*objectIt).first->texture );
			}
		}
		rt.setView( mapView );
		

		if( debugDrawOn )
		{
			DebugDraw( &rt );
		}

		//rt.setView( mapView );
		rt.display();
		
		//view.setSize( 1920, 1080 );
		sf::Sprite sprf( rt.getTexture() );
		//cout << view.getCenter().x << ", " << view.getCenter().y << endl;
		
		sprf.setPosition( mapView.getCenter() );
		//sprf.setScale( .5, .5 );
		//sprf.setScale( 1920.f / view.getSize().x , 1080.f / view.getSize().y );
		sprf.setOrigin( floor(sprf.getLocalBounds().width / 2), floor(sprf.getLocalBounds().height / 2) );
		testShader.setParameter( "texture", sf::Shader::CurrentTexture );
		//sf::Vector2f mid( view.getSize().x / 2.f, view.getSize().y / 2.f );
		//testShader.setParameter( "testPoint", mid );
		//window->draw( sprf, &testShader );
		//rt.draw( sprf );
		//rt.setSmooth( true );
		//rt.display();
		
		window->draw( sprf );
		//window->draw( sprf, &testShader );

		

		if( fpsCounter == 30 )
		{
			fpsCounter = 0;
			//goes back into an int after averaging
			fps = oldfps / 30.f;
			oldfps = 0;
			//cout << "fps: " << fps << endl;
		}
		else
		{
			oldfps += (int)( 1 / frameTime );
			
			++fpsCounter;
		}

		sf::Clock testClockgraphics;
		std::stringstream ss; 
		ss << fps;
		fpsDisplay.setString( ss.str() );
		fpsText.setString( ss.str() );
		//fpsDisplay.setPosition( 10 + camera.x * BOX2SF - window->getSize().x / 2.f, 0 + camera.y * BOX2SF 
		//	- window->getSize().y / 2.f );

		ss.str(std::string());
		ss << "Health: " << player->health << "   Deaths: " << testPlayerDeathCount;
		deathDisplay.setString( ss.str() );
		sf::Vector2i pos( 10, 5 );
		
		fpsDisplay.setPosition( window->mapPixelToCoords( pos ) );
		//fpsDisplay.setPosition( pos );
		//fpsDisplay.setPosition( player->GetPosition().x * BOX2SF, player->GetPosition().y * BOX2SF );

		//const sf::View & tempView = window->getView();
		//window->setView( window->getDefaultView() );

		//window->draw( fpsDisplay );
		pos.x = 100;
		//deathDisplay.setPosition( pos );
		//deathDisplay.setPosition( window->mapPixelToCoords( pos ) );
		//window->draw( deathDisplay );

		//window->setView( tempView );
		
		if( m_singleFrameMode )
		{
			const sf::View & pauseTempView = window->getView();
			window->setView( window->getDefaultView() );

			window->draw( *pauseBGSprite );
			//testText.setPosition( pauseMenuTexts[1].getPosition().x, pauseMenuTexts[1].getPosition().y );
			//pauseMenuTexts[0].setPosition( pauseMenuTexts[1].getPosition().x, pauseMenuTexts[1].getPosition().y );
			for( int i = 0; i < 5; ++i )
			{
				if( i == selectedPauseMenu )
					pauseMenuTexts[i].setColor( sf::Color::Red );
				else if( pauseMenuTexts[i].getString() == "Restart Room (Lose 1 life)" && lives == 0 )
				{
					pauseMenuTexts[i].setColor( sf::Color( 20, 20, 20 ) );
				}
				else
				{
					pauseMenuTexts[i].setColor( sf::Color::White );
				}
					

				window->draw( pauseMenuTexts[i] );
			}
			
			//testText.setColor( sf::Color::Blue );
			
			
			//window->draw( pauseMenuTexts[3] );
			//window->draw( fpsDisplay );
			//window->draw( deathDisplay );
			window->setView( pauseTempView );
		}

		const sf::View & tempView = window->getView();
		window->setView( window->getDefaultView() );

		window->draw( fpsText );
		window->setView( tempView );

	//	sf::Clock dispClock;
		//window->setView( mapView );
        window->display();
		//cout << "disp" << endl;
	//	cout << "disptime: " << dispClock.getElapsedTime().asSeconds() << endl;
	//	cout << "drawtime: " << drawClock.getElapsedTime().asSeconds() << endl;
		//int final = testClockgraphics.getElapsedTime().asMicroseconds();
		//if (final > 16700 )
		//	cout << "draw: " << testClockgraphics.getElapsedTime().asMicroseconds() << endl;

    }
	delete player;

	//+ only check for player because of the manual delete
	for( list<TrueActor*>::iterator it = activeActors.begin(); it != activeActors.end(); ++it )
	{
		if( (*it) != player )
			delete (*it);
	}

	for( list<TrueActor*>::iterator it = addedActors.begin(); it != addedActors.end(); ++it )
	{
		if( (*it) != player )
			delete(*it);
	}

	for( list<sf::Texture*>::iterator it = texs.begin(); it != texs.end(); ++it )
	{
		delete texVertexMap[(*it)];
		//+ dealloacate the memory here later
		//texVertexMap[(*it)] = new sf::VertexArray( sf::Quads, viewHalfWidthTiles * 2 * viewHalfHeightTiles * 2 * 4 );//viewHalfWidthTiles * viewHalfHeightTiles * 4 );
	}
}

void Stage::UpdateCamera()
{

}

void Stage::UpdatePostPhysics()
{
	

	for( list<TrueActor*>::iterator it = activeActors.begin(); it != activeActors.end(); ++it )
	{
		if( !(*it)->IsPaused() && !((*it)->isGroup) || (*it)->isGroup )
		{
			(*it)->UpdatePostPhysics();
		}
	}

	for( list<b2Fixture*>::iterator it = eventAreasEntered.begin(); it != eventAreasEntered.end(); ++it )
	{
		UpdateEventArea( (*it), true );
	}
	eventAreasEntered.clear();
	for( list<b2Fixture*>::iterator it = eventAreasExited.begin(); it != eventAreasExited.end(); ++it )
	{
		UpdateEventArea( (*it), false );
	}
	eventAreasExited.clear();
	
	lua_getglobal( L, "UpdatePostPhysics" );
	lua_pcall( L, 0, 0, 0 );

	for( std::map<std::string, lua_State*>::iterator collisionHandlerIt = collisionHandlers.begin(); 
		collisionHandlerIt != collisionHandlers.end(); collisionHandlerIt++ )
	{
		//lets the different tileset colliders do various frame by frame logic
		lua_State *handlerState = (*collisionHandlerIt).second;
		lua_getglobal( handlerState , "UpdatePostPhysics" );
		lua_pcall( handlerState , 0, 0, 0 );
		
	}

	c.UpdatePosition();

	//sf::Vector2f cam( camera.x, camera.y );

	/*sf::Vector2f diff( player->GetPosition().x - camera.x, player->GetPosition().y - camera.y );
	

	if( abs( diff.x ) < .01 )
		diff.x = 0;
	if( abs( diff.y ) < .01)
		diff.y = 0;
	if( diff.x > camMaxVel.x * TIMESTEP )
		diff.x = camMaxVel.x * TIMESTEP;
	if( diff.x < -camMaxVel.x * TIMESTEP )
		diff.x = -camMaxVel.x * TIMESTEP;
	
	if( diff.y > camMaxVel.y * TIMESTEP )
		diff.y = camMaxVel.y * TIMESTEP;
	if( diff.y < -camMaxVel.y * TIMESTEP )
		diff.y = -camMaxVel.y * TIMESTEP;


	
	cam.x += diff.x * follow.x;
	cam.y += diff.y * follow.y;*/

	//view.setCenter( c.GetViewPos() );
	sf::Vector2f viewPos = c.GetViewPos();
	//cout << "viewPos: " << viewPos.x * SF2BOX << ", " << viewPos.y * SF2BOX << endl;
	camera = c.pos;
	//sf::Vector2f viewPos( cam * BOX2SF );
	//viewPos.x = floor( viewPos.x + .5f );
	//viewPos.y = floor( viewPos.y + .5f );
	//sf::Vector2f diff( cam - view.getCenter() );

	
	view.setCenter( viewPos );
	mapView.setCenter( viewPos );
	//mapView.setCenter( viewPos );
	//view.setRotation( player->GetSpriteAngle() * 180 * 3.5 );
	//cout << "sprite angle: " << player->GetSpriteAngle() << endl;

	//camera.x = cam.x;
	//camera.y = cam.y;

	sf::IntRect camRect( viewPos.x - cameraZoom * 1920 / 2, viewPos.y - cameraZoom * 1080/2, cameraZoom * 1920, cameraZoom * 1080 );


	int newLeft = -1, newRight = -1, newTop = -1, newBottom = -1;
	if( camRect.left < currentRoom->left * BOX2SF)
		newLeft = currentRoom->left* BOX2SF;
	if( camRect.left + camRect.width > currentRoom->right * BOX2SF )
		newRight = currentRoom->right* BOX2SF;
	if( camRect.top < currentRoom->top* BOX2SF )
		newTop = currentRoom->top* BOX2SF;
	if( camRect.top + camRect.height > currentRoom->bottom* BOX2SF )
		newBottom = currentRoom->bottom* BOX2SF;

	if( newLeft >= 0 )
	{
		viewPos.x = newLeft + camRect.width / 2.f;
		view.setCenter( viewPos );
		mapView.setCenter( viewPos );
	}
	if( newRight >= 0 )
	{
		viewPos.x = newRight - camRect.width / 2.f;
		view.setCenter( viewPos );
		mapView.setCenter( viewPos );
	}

	if( newTop >= 0 )
	{
		viewPos.y = newTop + camRect.height/ 2.f;
		view.setCenter( viewPos );
		mapView.setCenter( viewPos );
	}
	if( newBottom >= 0 )
	{
		viewPos.y = newBottom - camRect.height / 2.f;
		view.setCenter( viewPos );
		mapView.setCenter( viewPos );
	}
	
	

	for( list<TrueActor*>::iterator it = activeActors.begin(); it != activeActors.end(); ++it )
	{
		if( !(*it)->IsPaused() && !((*it)->isGroup) || (*it)->isGroup )
		{
			(*it)->UpdateSprites();
		}
	}

	
	//view.setCenter( camera.x * SF2BOX, camera.y * SF2BOX );
}

void Stage::SetCameraFollowX( float rate )
{
	follow.x = rate;
}

void Stage::SetCameraFollowY( float rate )
{
	follow.y = rate;
}

void Stage::SetCameraZoom( float zoom )
{
	//cout << "setting zoom to: " << zoom << endl;
	//1920 x 1080
	//view.setSize( xxxxx * zoom, yyyyy * zoom );
	cameraZoom = zoom;
	//view.zoom( zoom );
}

float Stage::GetCameraZoom()
{
	return cameraZoom;
}

void Stage::DebugDraw( sf::RenderTarget *rt )
{
	b2Body * bodies = world->GetBodyList();
	//vector<sf::Shape> stuff;
	for( int i = 0; i < world->GetBodyCount(); ++i )
	{
		b2Fixture * fixtures = bodies->GetFixtureList();
		
		while (fixtures != NULL )
		{
		b2Shape * shape = fixtures->GetShape();
		
		sf::Color color;
		
		uint32 layer = fixtures->GetFilterData().categoryBits;
			//cout << "layer: " << layer << endl;
		switch ( layer )
		{
			//case ( 1 << CollisionLayers::Event ): 
				//color = sf::Color::Blue; 
				//break;
			//case ( 1 << Environment ): 
			//	color = sf::Color::Blue; 
			//	break;
			case ( 1 << CollisionLayers::PlayerHitbox ): 
				color = sf::Color::Red; 
				break;
			case ( 1 << CollisionLayers::PlayerHurtbox ): 
				color = sf::Color::Green; 
				break;
			case ( 1 << CollisionLayers::PlayerPhysicsbox ): 
				color = sf::Color::White;//grey 
				break;
			case ( 1 << CollisionLayers::EnemyHitbox ): 
				color = sf::Color::Red;
				break;
			case ( 1 << CollisionLayers::EnemyHurtbox ): 
				color = sf::Color::Magenta; 
				break;
			case ( 1 << CollisionLayers::EnemyPhysicsbox ): 
			case ( 1 << CollisionLayers::EnemyPhysicsboxWithPlayer ):
				color = sf::Color::Cyan; 
				break;
			case ( 1 << CollisionLayers::PlayerDetectionbox ): 
				color = sf::Color::Cyan;
				break;
			case ( 1 << CollisionLayers::ActorDetectionbox ):
				color = sf::Color::Magenta;
				break;
			case( 1 << CollisionLayers::Door ):
				color = sf::Color::White;
				break;
			case ( 1 << CollisionLayers::Environment ):
				if( debugDrawEnv )
				{
					//color = sf::Color::White;
					break;
				}
			default:
				fixtures = fixtures->GetNext();
				continue;

			//case ( 1 << CollisionLayers::ActorDetectionbox ): 
			//case ( 1 << CollisionLayers::PlayerDetectionbox ): 
			//	color = sf::Color::Transparent; 
			//	break;
				//cout << "error" << endl;
				//assert( 0 );
		};
		//color.a = .5;
		color.a = 150;
		

		b2Shape::Type type = shape->GetType();
		if ( type == b2Shape::e_polygon )
		{
			sf::ConvexShape polygon;
			polygon.setFillColor( color );
			b2PolygonShape * b2polygon = (b2PolygonShape*)shape;
			int vertexCount = b2polygon->GetVertexCount();
			
			polygon.setPointCount( vertexCount );
			for( int j = 0; j < vertexCount; ++j )
			{
				b2Vec2 vertex = b2polygon->GetVertex( j );
				sf::Vector2f p( vertex.x * BOX2SF, vertex.y * BOX2SF );
				polygon.setPoint( j, p );
			}
			polygon.setPosition( bodies->GetPosition().x * BOX2SF,
				bodies->GetPosition().y * BOX2SF);
			polygon.setRotation( bodies->GetAngle() * 180 / PI );
			rt->draw( polygon );
		}
		else if ( type == b2Shape::e_circle )
		{
			sf::CircleShape circle;
			b2CircleShape * b2circle = (b2CircleShape*)shape;
			circle.setRadius( b2circle->m_radius * BOX2SF );
			circle.setFillColor( color );
			circle.setOrigin( circle.getRadius(), circle.getRadius() );
			b2Vec2 truePos;
			circle.setPosition( bodies->GetPosition().x * BOX2SF, bodies->GetPosition().y * BOX2SF );
			truePos.x = bodies->GetPosition().x + cos( bodies->GetAngle() ) * b2circle->m_p.x + 
				sin( bodies->GetAngle() ) * b2circle->m_p.y;
			truePos.y = bodies->GetPosition().y + sin( bodies->GetAngle() ) * b2circle->m_p.x + 
				cos( bodies->GetAngle() ) * b2circle->m_p.y;
			
			circle.setPosition( truePos.x * BOX2SF, truePos.y * BOX2SF );
			//circle.setPosition( (bodies->GetPosition().x + cos( bodies->GetAngle() ) * b2circle->m_p.x
			//	+ sin( bodies->GetAngle() ) * b2circle->m_p.y, 
			//	) * BOX2SF,
			//	(bodies->GetPosition().y + b2circle->m_p.y) * BOX2SF );
			rt->draw( circle );
			/*sf::Shape circle; 
			b2CircleShape * b2circle = (b2CircleShape*)shape;
			circle = sf::Shape::Circle( bodies->GetPosition().x * PPM + b2circle->m_p.x * PPM, 
				bodies->GetPosition().y * PPM + b2circle->m_p.y * PPM, 
				b2circle->m_radius * PPM, color );
			window->Draw( circle );*/
			//stuff.push_back( circle );
		}
		else if ( type == b2Shape::e_chain )
		{
			//cout << "chain shape!" << endl;
			b2ChainShape* c = (b2ChainShape*)shape;
			b2EdgeShape child;
			//c->GetChildEdge( &child, 1 );
			int32 childCount = c->GetChildCount();
			for( uint32 childCounter = 0; childCounter < childCount; ++childCounter )
			{
				
				c->GetChildEdge( &child, childCounter );
				b2Vec2 one = child.m_vertex1;
				b2Vec2 two = child.m_vertex2;
				sf::Vector2f midPoint( (one.x + two.x) / 2, (one.y + two.y) / 2 );
				double playerDistSqr = pow(midPoint.x - player->GetPosition().x, 2) 
					+ pow( midPoint.y - player->GetPosition().y, 2 );
				if( playerDistSqr < 1000 )
				{

				double dist = sqrt(pow(one.x - two.x, 2) + pow( one.y - two.y, 2 ));
				sf::RectangleShape line( sf::Vector2f( dist * BOX2SF, 10 ) );
				line.setFillColor( sf::Color::Black );
				
				//line.setOutlineColor( sf::Color::Green );
				//line.setOutlineThickness( 20 );
				line.setOrigin( line.getLocalBounds().width / 2, line.getLocalBounds().height / 2 );			
				line.setPosition( midPoint );
				line.setPosition( line.getPosition() * BOX2SF );
				line.setRotation( atan2( two.y-one.y, two.x-one.x ) * 180/PI );
				rt->draw(line);
				}
				else
				{
					//cout << "pos: " << midPoint.x << ", " << midPoint.y << endl;
					//cout << "dist: " << playerDistSqr << endl;
				}
			}
			
			
			

			//cerr << "weird" << endl;
		}

		fixtures = fixtures->GetNext();
		}
		bodies = bodies->GetNext();

	}
}



SingleActor * Stage::CreateActor( const std::string &type, b2Vec2 &pos, b2Vec2 &vel, bool facingRight, bool reverse,
	float32 angle, TrueActor *parent )
//void Stage::CreateActor( const std::string &type, float32 posX, float32 posY, float32 velX, float32 velY, 
//	bool facingRight, bool reverse, float32 angle, Actor *parent )
{

	//b2Vec2 pos( posX, posY );
	//b2Vec2 vel( velX, velY );

	if( boost::iequals( type, "player" ) )
	{
		player = new PlayerChar( pos, vel, facingRight, reverse, angle, parent, this );
		addedActors.push_back( player );
		currentRoom->spawn = pos;
		cout << "spawn pos: " << pos.x << ", " << pos.y << endl;
		//activeActors.push_back( player );
		return (SingleActor*)player;
	}
	else
	{
		SingleActor *a = new SingleActor( type, pos, vel, facingRight, reverse, angle, parent, this );
		addedActors.push_back( a );
		return a;
		//activeActors.push_back( a );
	}	
	
}

GroupActor * Stage::CreateActorGroup( const std::string &type, uint32 actorCount, b2Vec2 &pos, b2Vec2 &vel, 
	bool facingRight, bool reverse, float32 angle, TrueActor *parent )
{
	GroupActor *a = new GroupActor( type, actorCount, pos, vel, facingRight, reverse, angle, parent, 
		this );
	addedActors.push_back( a );
	return a;
}

//might be only temporary. should be another way to tell if something is bullet
BulletActor * Stage::CreateBulletGroup( uint32 actorCount, b2Vec2 &pos, b2Vec2 &vel, TrueActor *parent )
{
	BulletActor *a = new BulletActor( "basicturretbullet", actorCount, pos, vel, parent, this );
	addedActors.push_back( a );
	return a;
}

void Stage::UpdateTileModifications()
{
	for( list<StageCollision>::iterator it = collisions.begin(); it != collisions.end(); ++it )
	{
		TileModifyWorld( (*it) );
	}
	collisions.clear();
}

void Stage::TileModifyWorld( StageCollision &sc )
{
	lua_State *state = collisionHandlers[sc.m_type];

	lua_getglobal( state, "ModifyWorld" );
	push( state, sc.m_actor );
	push( state, &sc.m_tile );
	push( state, &sc.m_fraction );
	push( state, &sc.m_normal );
	lua_pushboolean( state, sc.m_enabled );
	lua_pcall( state, 5, 0, 0 );
}

void Stage::UpdateEventArea( b2Fixture *fixture, bool entered )
{
	//instead of sending fixtures to the script and having to worry about that, there will be a bunch
	//of variables here that we simply push into the script, after I figure out everything that a person 
	//might need
	
	uint32 index = (uint32)fixture->GetBody()->GetUserData();
	const b2AABB &aabb = fixture->GetAABB(0);
	b2Vec2 pos( aabb.GetCenter() );
	float width = aabb.GetExtents().x;
	float height = aabb.GetExtents().y;
	lua_getglobal( L, events[index].c_str() );
	lua_pushboolean( L, entered );
	push( L, &pos );
	lua_pushnumber( L, width );
	lua_pushnumber( L, height );
	lua_pcall( L, 4, 0, 0 );

	//pop variables off the script stack
}



//will only collide with the player and stage will hold a pointer to the player anyway
void Stage::EventAreaEnter( b2Fixture *eventAreaFixture )
{
	eventAreasEntered.push_back( eventAreaFixture );
	//whatever happens in here should wait until the next frame
	//uint32 index = (uint32)eventAreaFixture->GetBody()->GetUserData();
	//cout << "entering event area: " <<  events[index] << endl;
}

void Stage::EventAreaLeave( b2Fixture *eventAreaFixture )
{
	eventAreasExited.push_back( eventAreaFixture );
	//uint32 index = (uint32)eventAreaFixture->GetBody()->GetUserData();
	//cout << "leaving event area: " <<  events[index] << endl;
}

bool Stage::HandleCollision( b2Contact *contact )
{
	b2WorldManifold worldManifold;
	contact->GetWorldManifold( &worldManifold );
	b2Fixture *a = contact->GetFixtureA();
	b2Fixture *b = contact->GetFixtureB();
	uint16 aLayer = a->GetFilterData().categoryBits;
	uint16 bLayer = b->GetFilterData().categoryBits;

	
	
	TrueActor *actor = NULL;
	b2Vec2 tilePos;
	uint32 localTileID;
	if( aLayer == 1 << CollisionLayers::Environment )
	{
		actor = (TrueActor*)b->GetBody()->GetUserData();
		b2Vec2 *tempVec= (b2Vec2*)(a->GetUserData());
		tilePos.x = tempVec->x;
		tilePos.y = tempVec->y;
		//GlobalToLocal( id );
		//cout << "id: " << id << endl; 
		
	}
	else
	{
		actor = (TrueActor*)a->GetBody()->GetUserData();
		//bLayer is environment

		b2Vec2 *tempVec= (b2Vec2*)(b->GetUserData());
		tilePos.x = tempVec->x;
		tilePos.y = tempVec->y;
		//GlobalToLocal( id );
		//cout << "id: " << id << endl; 
	}
	//localize the global ID
	GlobalToLocal( localTileID );
	

	assert( actor != NULL );


	b2Vec2 finalPoint;

	bool enabledCurrent = false;
	bool enable = false;

	int firstTileX, firstTileY, firstFractionX, firstFractionY;
	int lastTileX, lastTileY, lastFractionX, lastFractionY;

	uint32 pointCount = contact->GetManifold()->pointCount;
	//cout << "fff: " << contact->GetManifold()->localPoint.x << ", " << contact->GetManifold()->localPoint.y << endl;
	b2Manifold *m = contact->GetManifold();
	b2Vec2 testm = m->localPoint;
	testm = actor->GetBody()->GetWorldPoint( testm );


	//cout << "testm: " << testm.x << ", " << testm.y << endl;
	//cout << "local: " << contact->GetManifold()->points[0].localPoint.x << ", " << contact->GetManifold()->points[0].localPoint.y << endl;
	//cout << "localnorm: " << contact->GetManifold()->localNormal.x << ", " << contact->GetManifold()->localNormal.y << endl;
	//cout << "actor: " << actor->GetPosition().x << ", " << actor->GetPosition().y << endl;
	//cout << "both: " << actor->GetPosition().x + contact->GetManifold()->points[0].localPoint.x << ", " 
		//<< actor->GetPosition().y + contact->GetManifold()->points[0].localPoint.y << endl;

	//^^ worry about this when I have larger than 1 tile sprites

	
	/*if( pointCount == 2 )
	{
		if( worldManifold.points[0].x == worldManifold.points[1].x )
		{
			float spaceInBetween = worldManifold[0].y - worldManifold[1].y;
			int tilesInBetween = (int)abs( spaceInBetween );
		}
		else if( worldManifold.points[0].y == worldManifold.points[1].y )
		{
			float spaceInBetween = worldManifold[0].x - worldManifold[1].x;
			int tilesInBetween = (int)abs(spaceInBetween);

		}
	}*/ 
	for( int pointI = 0; pointI < pointCount; ++pointI )
	{
		finalPoint = worldManifold.points[pointI];
	
		//cout << finalPoint.x << ", " << finalPoint.y << endl;

		float fractionalTileX = finalPoint.x - floor( finalPoint.x );
		float fractionalTileY = finalPoint.y - floor( finalPoint.y );

		int tileX = (int)tilePos.x;
		int tileY = (int)tilePos.y;

		//b2Vec2 tileVec( tileX, tileY );
		//b2Vec2 fraction( fractionalTileX, fractionalTileY );

		//cout << "tile: " << tileX << ", " << tileY << endl;
		
		if( staticTileSets[tileX][tileY] != NULL )
		{
			//cout << "localID: " << localTileID << endl;
			 if( TileHandleCollision( staticTileSets[tileX][tileY]->collisionHandler, actor,
				tilePos, finalPoint, worldManifold.normal ) )
				enable = true;

			// cout << "tile: " << tileVec.x << ", " << tileVec.y << endl;
			// cout << "fraction: " << fraction.x << ", " << fraction.y << endl;
			// cout << "final: " << finalPoint.x << ", " << finalPoint.y << endl;
			// cout << "normal: " << worldManifold.normal.x << ", " << worldManifold.normal.y << endl;
			 //if it gets disabled but it has a fallthrough above it, you want to fall through
			
			 {
			
				tileY = tileY - 1;
			
				if( staticTileSets[tileX][tileY] != NULL )
				{
					//if( !TileHandleCollision( staticTileSets[tileX][tileY]->collisionHandler, actor,
					//	tileVec, fraction, worldManifold.normal ) )
						//	enable = false;
				}
			}
		}
		else
		{
			////enable = true;
			
			{
			
				tileY = tileY - 1;
			//	fraction.y = 1;
				if( staticTileSets[tileX][tileY] != NULL )
				{
					//if( !TileHandleCollision( staticTileSets[tileX][tileY]->collisionHandler, actor,localTileID,
					//	tileVec, fraction, worldManifold.normal ) )
					//		enable = false;
				}
			}
			//cout << "final: " << finalPoint.x << ", " << finalPoint.y << endl;
			//enable = false;
			
		}


		
		
		

		//it will always be positive because we only use positive numbers in this simulation
		/*float threshold = .1f;
		if( fractionalTileX > 1 - threshold )
		{
			tileVec.x += 1;
			if( tileX < stageWidth && staticTileSets[tileX + 1][tileY] != NULL )
			{
				 if( TileHandleCollision( staticTileSets[tileX + 1][tileY]->collisionHandler, actor,
						tileVec, fraction, worldManifold.normal ) )
						enable = true;
			}
		}
		else if( fractionalTileX < threshold )
		{
			tileVec.x -= 1;
			if( tileX > 0 && staticTileSets[tileX - 1][tileY] != NULL )
			{
				 if( TileHandleCollision( staticTileSets[tileX - 1][tileY]->collisionHandler, actor,
						tileVec, fraction, worldManifold.normal ) )
						enable = true;
			}
		}

		if( fractionalTileY > 1 - threshold )
		{
			tileVec.y += 1;
			if( tileY < stageWidth && staticTileSets[tileX][tileY + 1] != NULL )
			{
				 if( TileHandleCollision( staticTileSets[tileX][tileY + 1]->collisionHandler, actor,
						tileVec, fraction, worldManifold.normal ) )
						enable = true;
			}
		}
		else if( fractionalTileY < threshold )
		{
			tileVec.y -= 1;
			if( tileY > 0 && staticTileSets[tileX][tileY - 1] != NULL )
			{
				 if( TileHandleCollision( staticTileSets[tileX][tileY - 1]->collisionHandler, actor,
						tileVec, fraction, worldManifold.normal ) )
						enable = true;
			}
		}*/
	}
	
	return enable;
}

TileSet * Stage::GlobalToLocal( uint32 &id )
{
	if( id == 0 ) return NULL;

	TileSet *ts = NULL;
	for( int i = 0; i < tileSetCount; ++i )
	{
		int setFirstGID = tileSets[i]->firstGID;
		if( setFirstGID + tileSets[i]->TileCount() > id && 
			setFirstGID <= id )
		{
			ts = tileSets[i];
			id = id - setFirstGID;
		}
	}
	return ts;
}

bool Stage::TileHandleRayCast( TrueActor *a, b2Fixture *fixture, b2Vec2 &collision, b2Vec2 &normal )
{
	b2Vec2 *tempVec = (b2Vec2*)(fixture->GetUserData());
	int tileX = tempVec->x;
	int tileY = tempVec->y;
	int localTile = staticLocalID[tileX][tileY];
	b2Vec2 tile = *tempVec;
	//if( TileHandleCollision( staticTileSets[tileX][tileY]->collisionHandler, actor,
	//		tilePos, finalPoint, worldManifold.normal ) )
	//		enable = true;

	//cout << a->actorType << endl;
	lua_State *handler = collisionHandlers[staticTileSets[tileX][tileY]->collisionHandler];
	lua_getglobal( handler, "HandleRayCast" );
	push( handler, localTile );
	push( handler, a );
	push( handler, &tile );
	push( handler, &collision );
	push( handler, &normal );
	lua_pcall( handler, 5, 1, 0 );

	if( !lua_isboolean( handler, -1 ) )
	{
		assert( 0 && "needs to be a boolean" );
	}
	bool enable = lua_toboolean( handler, -1 );
	lua_pop( handler, 1 );

	return enable;
}

bool Stage::TileHandleCollision( const std::string &h, TrueActor *a, b2Vec2 &tile, 
	b2Vec2 &collision, b2Vec2 &normal )
{
	
	int localTile = staticLocalID[(uint32)tile.x][(uint32)tile.y];

	lua_State *handler = 
		collisionHandlers[h];
	lua_getglobal( handler, "HandleCollision" );
	push( handler, localTile );
	push( handler, a );
	push( handler, &tile );
	push( handler, &collision );
	push( handler, &normal );
	lua_pcall( handler, 5, 2, 0 );

	if( !lua_isboolean( handler, -1 ) )
	{
		assert( 0 && "needs to be a boolean" );
	}
	bool enable = lua_toboolean( handler, -1 );
	lua_pop( handler, 1 );

	if( !lua_isboolean( handler, -1 ) )
	{
		assert( 0 && "needs to be a boolean 2" );
	}
	bool c = lua_toboolean( handler, -1 );
	lua_pop( handler, 1 );
	
	bool con = false;

	//if there isn't another copy of the string already push the event onto the list of collisions. if
	//it should be consumed, then it is added to the list so that no more copies of the same effect can be
	//added

	for( list<string>::iterator it = consumed.begin(); it != consumed.end() && !con; ++it )
	{
		if( h == (*it) )
			con = true;
	}

	if( !con )
	{	
		collisions.push_front( StageCollision( h, tile, a, collision, normal, enable ) );
		if( c ) consumed.push_front( h );
	}

	return enable;
}

//param name: the name of the tileset
//param firstgid: the gid of the local id = 0
//param tileList: list of all tiles so far by gid. made into an array after all processing
//param totalTiles: count of total amount of tiles that need to be in the static list. 

TileSet * Stage::LoadTileSet( const string &dir, const std::string &fileName, int firstGID, list< list< b2Vec2 > >* &chains,
	map< uint32, string > &actorTiles )
{
	TileSet *ts = new TileSet;

	ifstream tileSetScript;
	tileSetScript.open( dir + "Maps/" + fileName );
	string tileSetString( istreambuf_iterator<char>( tileSetScript ), (istreambuf_iterator<char>()) );
	tileSetScript.close();

	vector<char> stringCopy( tileSetString.begin(), tileSetString.end() );
	stringCopy.push_back( '\0' );

	if( tileSetString.length() > 0 )
	{
		rapidxml::xml_document<> doc;
		doc.parse<0>( &stringCopy[0] );

		rapidxml::xml_node<> *node = doc.first_node();
		rapidxml::xml_attribute<> *attr = node->first_attribute();

		string name = attr->value();

		attr = attr->next_attribute();

		int tileWidth = boost::lexical_cast<int>( attr->value() );

		attr = attr->next_attribute();

		int tileHeight = boost::lexical_cast<int>( attr->value() );

		string handlerName;
		lua_State *handlerL = NULL;
		if( string(node->first_node()->name()) == "properties" )
		{
			node = node->first_node();
			node = node->first_node();
			attr = node->first_attribute();
			attr = attr->next_attribute();
			handlerName = attr->value();
			node = node->parent();
			node = node->next_sibling();

			if( collisionHandlers.count( handlerName ) == 0 )
			{
				lua_State * Lh = luaL_newstate();
				luaL_openlibs( Lh );
				
				getGlobalNamespace( Lh )
					.beginNamespace( "Game" )
						.beginClass<TrueActor>( "TrueActor" )
							.addFunction( "TileSetIndex", &TrueActor::TileSetIndex )
							.addFunction( "SetSprite", &TrueActor::SetSprite )
							.addFunction( "SetVelocity", &TrueActor::SetVelocity )
							
							.addFunction( "GetVelocity", &TrueActor::GetVelocity )
							.addFunction( "SetPosition", &TrueActor::SetPosition )
							.addFunction( "GetPosition", &TrueActor::GetPosition )
							.addFunction( "CreateBox", &TrueActor::CreateBox )
							.addFunction( "GetWorldLeft", &TrueActor::GetWorldLeft )
							.addFunction( "GetWorldRight", &TrueActor::GetWorldRight )
							.addFunction( "GetWorldTop", &TrueActor::GetWorldTop )
							.addFunction( "GetWorldBottom", &TrueActor::GetWorldBottom )
							.addFunction( "Message", &TrueActor::Message )
							.addData( "health", &TrueActor::health )
							.addData( "type", &TrueActor::actorType, false )
						.endClass()
						.deriveClass<GroupActor,TrueActor>("GroupActor")
						.endClass()
						.deriveClass<SingleActor,TrueActor>("SingleActor")
						.endClass()
						.deriveClass<PlayerChar, SingleActor>("PlayerChar")
							.addFunction( "SetCarryVelocity", &PlayerChar::SetCarryVelocity )
							.addFunction( "GetCarryVelocity", &PlayerChar::GetCarryVelocity )
						.endClass()
						.beginClass<b2Vec2>( "b2Vec2" )
							.addConstructor<void(*)(void)>()
							.addData( "x", &b2Vec2::x )
							.addData( "y", &b2Vec2::y )
						.endClass()
						.beginClass<Stage>( "Stage" )
							//.addFunction( "SetCameraPosition", &Stage::SetCameraPosition )
							//.addFunction( "SetCameraFollowX", &Stage::SetCameraFollowX )
							//.addFunction( "SetCameraFollowY", &Stage::SetCameraFollowY )
							//.addFunction( "SetCameraMaxVelocityX", &Stage::SetCameraMaxVelocityX )
							//.addFunction( "SetCameraMaxVelocityY", &Stage::SetCameraMaxVelocityY )
							//.addFunction( "CreateActor", &Stage::CreateActor )
							//.addFunction( "GetCameraPosition", &Stage::GetCameraPosition )
							.addData( "player", &Stage::player )
						.endClass()
				.endNamespace();


				//^^hardcoding tutorial here
				string luaF =  dir + "Scripts/" + handlerName + ".lua" ;
				int s = luaL_loadfile( Lh, luaF.c_str() );

				push<Stage*>( Lh, this );
				lua_setglobal( Lh, "stage" );

				lua_pcall( Lh, 0, 0, 0 );
				collisionHandlers[handlerName] = Lh;
				handlerL = Lh;
			}
			else
			{
				handlerL = collisionHandlers[handlerName];
			}
		}
		else
			node = node->first_node();

		//node = node->first_node();
		attr = node->first_attribute();

		string imageSource = attr->value(); 
		ts->texture = new sf::Texture;
		ts->texture->setSmooth( false );

		
		bool success = ts->texture->loadFromFile( dir + "Maps/" + fileName.substr( 0, fileName.length() - 3 ) + "png" );

		ts->tileWidth = tileWidth;
		ts->tileHeight = tileHeight;
		ts->firstGID = firstGID;
		ts->imageSource = imageSource;

		cout << "image source: " << imageSource << endl;
		ts->collisionHandler = handlerName;

		//^^hack to test actor sprite smoothness
		if( handlerName == "" )
		{
		//	ts->texture->setSmooth( true );
		}
		

		attr = attr->next_attribute();
		//optional? 
		int imageWidth = boost::lexical_cast<int>( attr->value() );

		attr = attr->next_attribute();

		int imageHeight = boost::lexical_cast<int>( attr->value() );

		node = node->next_sibling();
		
		while( node != NULL && string(node->name()) == "tile" )
		{
			attr = node->first_attribute();

			int localID = boost::lexical_cast<int>( attr->value() );

			node = node->first_node();
			//should only be properties in here, right?
			node = node->first_node();

			while( string(node->name()) == "property" )
			{
				attr = node->first_attribute();

				string propertyName = attr->value();

				attr = attr->next_attribute();
				
				//load in chain shapes 

				if( boost::iequals( propertyName.substr(0, 5), "chain" ) )
				{
					if( chains == NULL )
					{
						uint32 tileCount = ts->TileCount();
						chains = new list<list<b2Vec2>>[tileCount];
					}

					string points = attr->value();
					
					chains[localID].push_back( list<b2Vec2>() );
					
					for( int i = 1; i < points.size(); ++i )
					{
						stringstream xss;
						stringstream yss;
						while( points[i] != ',' )
						{
							xss << points[i];
							++i;
						}
						++i;
						while( points[i] != ')' )
						{
							yss << points[i];
							++i;
						}
						i += 2;

						int x = boost::lexical_cast<int>( xss.str() );
						int y = boost::lexical_cast<int>( yss.str() );

						chains[localID].back().push_back( b2Vec2( x, y ) );
					}
				}
				else if( boost::iequals( propertyName, "actortype" ) ) //does not work with UTF-8
				{
					string actorType = attr->value();

					actorTiles[localID + firstGID] = actorType;
					cout << "registered " << actorType << ", TRUE ID: " << localID + firstGID << " -------------" << endl;
				}
				else if( boost::iequals( propertyName, "airtype" ) )
				{
					string airType = attr->value();
					airHandlers[localID + firstGID] = airType;
				}

				
				if( node->next_sibling() != NULL && 
					string(node->next_sibling()->name()) == "property" )
					node = node->next_sibling();
				else
					break;
			}
			

			node = node->parent();
			node = node->parent();
			if( node->next_sibling() != NULL &&
				string(node->next_sibling()->name()) == "tile" )
				node = node->next_sibling();
			else
				break;
		}

	}

	return ts;
}

void Stage::SetCameraPosition( float x, float y )
{
	//camera.Set( x, y );
	c.pos.Set( x, y );
}

b2Vec2 & Stage::GetCameraPosition()
{
	return camera;
}

void Stage::SetCameraMaxVelocityX( float speed )
{
	camMaxVel.x = speed;
}

void Stage::SetCameraMaxVelocityY( float speed )
{
	camMaxVel.y = speed;
}

void Stage::DebugDrawOn( bool on, bool environment )
{
	debugDrawOn = on;
	debugDrawEnv = environment;
}

void Stage::QuitStage()
{
	/*delete player;

	//+ only check for player because of the manual delete
	for( list<TrueActor*>::iterator it = activeActors.begin(); it != activeActors.end(); ++it )
	{
		if( (*it) != player )
			delete (*it);
	}

	for( list<TrueActor*>::iterator it = addedActors.begin(); it != addedActors.end(); ++it )
	{
		if( (*it) != player )
			delete(*it);
	}

	for( list<sf::Texture*>::iterator it = texs.begin(); it != texs.end(); ++it )
	{
		delete texVertexMap[(*it)];
		//+ dealloacate the memory here later
		//texVertexMap[(*it)] = new sf::VertexArray( sf::Quads, viewHalfWidthTiles * 2 * viewHalfHeightTiles * 2 * 4 );//viewHalfWidthTiles * viewHalfHeightTiles * 4 );
	}*/
}

void Stage::InitStaticVars( sf::RenderWindow * win )
{
	pauseBGTexture = new sf::Texture;
	pauseBGTexture->loadFromFile( "Resources/Menus/pauseBG.png" );
	pauseBGSprite = new sf::Sprite;
	pauseBGSprite->setTexture( *pauseBGTexture );

	pauseMenuTexts[0].setString( "Resume" );
	pauseMenuTexts[1].setString( "Restart Room (Lose 1 life)" );
	pauseMenuTexts[2].setString( "Restart Level (Full lives)" );
	pauseMenuTexts[3].setString( "Quit to Level Menu" );
	pauseMenuTexts[4].setString( "Quit Application" );

	

	if ( !debugFont.loadFromFile( "Resources/Menus/Fonts/arial.ttf" ) )
	{
		cout << "ERROR: COULD NOT OPEN FONT FILE" << endl;
	}


	for( int i = 0; i < 5; ++i )
	{
		pauseMenuTexts[i].setFont( debugFont );
		pauseMenuTexts[i].setColor( sf::Color::White );
		pauseMenuTexts[i].setCharacterSize( 80 );

		pauseMenuTexts[i].setOrigin( pauseMenuTexts[i].getLocalBounds().left + pauseMenuTexts[i].getLocalBounds().width/2.f,
			pauseMenuTexts[i].getLocalBounds().top + pauseMenuTexts[i].getLocalBounds().height/2.f );
		pauseMenuTexts[i].setPosition( win->getSize().x / 2, win->getSize().y / 2 
			- 50 * 4 + 100 * i );
	}

	
}

void Stage::RoomRestart()
{
	lua_getglobal( player->L, "Spawn" );
	lua_pcall( player->L, 0, 0, 0 );
	
	player->actorParams->hitsReceivedSize = 0;
	player->actorParams->bodyCollisionsSize = 0;
	player->actorParams->actorsAttackedSize = 0;

	Room *ownerRoom = currentRoom->GetOwner();
	SetRoom( ownerRoom );

	b2Vec2 &spawnPoint = ownerRoom->GetSpawnPoint();
				
	/*if( currentInput.Left() )
	{
		currentRoom->spawn.x -= .5;
	}
	else if( currentInput.Right() )
	{
		currentRoom->spawn.x += .5;
	}
				
	if( currentInput.Up() )
	{
		currentRoom->spawn.y -= .5;
	}
	else if( currentInput.Down() )
	{
		currentRoom->spawn.y += .5;
	}*/
				
	//player->SetVelocity( 0, 0 );
	player->SetPosition( spawnPoint.x, spawnPoint.y );

	SetCameraPosition( spawnPoint.x, spawnPoint.y );
	SetCameraZoom( 1 );
	c.offset.SetZero();
}

void Stage::LevelRestart()
{
	lua_getglobal( player->L, "Spawn" );
	lua_pcall( player->L, 0, 0, 0 );

	player->actorParams->hitsReceivedSize = 0;
	player->actorParams->bodyCollisionsSize = 0;
	player->actorParams->actorsAttackedSize = 0;

	SetRoom( startRoom );
	b2Vec2 &spawnPoint = startRoom->GetSpawnPoint();
	SetCameraPosition( spawnPoint.x, spawnPoint.y );
	SetCameraZoom( 1 );
	c.offset.SetZero();
	player->SetPosition( spawnPoint.x, spawnPoint.y );
}

bool Stage::HasPlayerPower( const std::string & powerName )
{
	if( powerName == "boosterUpgrade" )
	{
		return (playerPowers & 1);
	}
	else if( powerName == "spacialTether" )
	{
		return playerPowers & ( 1 << 1 );
	}
	else if( powerName == "gravitySwitch" )
	{
		return playerPowers & ( 1 << 3 );
	}
	else
	{
		assert( 0 );
	}
	return false;
}