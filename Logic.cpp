#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include "Input.h"
#include <iostream>
#include <list>
#include <assert.h>
#include "Actor.h"
#include <SFML/Window/Keyboard.hpp>
#include "Box2D.h"
#include "Globals.h"
#include "Stage.h"
#include "Collision.h"
#include <sstream>
#include <vld.h>

//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>

using namespace std;

using namespace sf;

ControllerState currentInput;
ControllerState prevInput;

//std::list<Actor*> actors;
//std::list<ControllerState> input2;

int currentFrame = 0;

sf::RenderWindow *window;

GameController controller( 0 );

bool RunStage( const std::string &stageName );
void Shutdown();
void SetCurrentMenu( const std::string menu );


//main menu stuff
int selectedMainMenu = 0;
const int NUM_MAIN_OPTIONS = 3;
sf::Text menuWords[NUM_MAIN_OPTIONS];

int selectedWorldMenu = 0;
const int NUM_WORLD_OPTIONS = 6;
sf::Text worldNames[NUM_WORLD_OPTIONS];
sf::Vector2f worldButtonPositions[NUM_WORLD_OPTIONS];

int selectedLevelMenu = 0;
sf::Sprite levelThumbnails[4];
std::string levelFiles[4];

std::string currentMenu;
sf::View menuView;
//world menu stuff

//level menu stuff
#include <iostream>
#include <cstdio>

int main()
{	
	//freopen( "output.txt", "w", stdout );


	window = new sf::RenderWindow(/*sf::VideoMode(1400, 900)sf::VideoMode::getDesktopMode()*/
			sf::VideoMode( 1920, 1080 ), "platformer", sf::Style::Default, sf::ContextSettings( 0, 0, 0, 0, 0 ));
	sf::Vector2i pos( 0, 0 );
	window->setPosition( pos );
	window->setVerticalSyncEnabled( true );
	//window->setFramerateLimit( 60 );
	window->setMouseCursorVisible( true );

	menuView.setSize( 1920, 1080 );
	menuView.setCenter( 1920 / 2, 1080 / 2 );


	Stage::InitStaticVars( window );
	
	
	


	//MAIN MENU STUFF
	
	
	SetCurrentMenu( "main" );
	//main world level
	
	int mainMenuTextHeight = 80;
	menuWords[0].setString( "Play" );
	menuWords[1].setString( "Options" );
	menuWords[2].setString( "Quit" );

	sf::Font debugFont;
	if ( !debugFont.loadFromFile( "Resources/Menus/Fonts/arial.ttf" ) )
	{
		cout << "ERROR: COULD NOT OPEN FONT FILE" << endl;
	}

	for( int i = 0; i < NUM_MAIN_OPTIONS; ++i )
	{
		menuWords[i].setFont( debugFont );
		menuWords[i].setCharacterSize( mainMenuTextHeight );
		menuWords[i].setColor( sf::Color::White );

		menuWords[i].setOrigin( menuWords[i].getLocalBounds().left + menuWords[i].getLocalBounds().width/2.f,
			menuWords[i].getLocalBounds().top + menuWords[i].getLocalBounds().height/2.f );
		menuWords[i].setPosition( window->getSize().x / 2, window->getSize().y / 2 
			+ ( mainMenuTextHeight + 50 ) * i );
	}

	sf::Texture mainMenuBackgroundTexture;
	if( !mainMenuBackgroundTexture.loadFromFile( "Resources/Menus/mainbg1.png" ) )
	{
		cout << "error loading main background image" << endl;
	}
	sf::Sprite mainMenuSprite;
	mainMenuSprite.setTexture( mainMenuBackgroundTexture );



	//WORLD MENU STUFF

	//currentMenu = "world"; //temporary for testing

	worldNames[0].setString( "Vertical Farming" );
	worldNames[1].setString( "Secret Lab" );
	worldNames[2].setString( "Downtown" );
	worldNames[3].setString( "Skyline" );
	worldNames[4].setString( "Space Station" );
	worldNames[5].setString( "Robotics Factory" );

	worldButtonPositions[0] = sf::Vector2f( 414, 290 );
	worldButtonPositions[1] = sf::Vector2f( 1038, 339 );
	worldButtonPositions[2] = sf::Vector2f( 1556, 323 );
	worldButtonPositions[3] = sf::Vector2f( 393, 860 );
	worldButtonPositions[4] = sf::Vector2f( 982, 929 );
	worldButtonPositions[5] = sf::Vector2f( 1543, 864 );



	int worldNameTextHeight = 80;
	

	for( int i = 0; i < NUM_WORLD_OPTIONS; ++i )
	{
		worldNames[i].setFont( debugFont );
		worldNames[i].setCharacterSize( mainMenuTextHeight );
		worldNames[i].setColor( sf::Color::White );

		worldNames[i].setOrigin( worldNames[i].getLocalBounds().left + worldNames[i].getLocalBounds().width/2.f,
			worldNames[i].getLocalBounds().top + worldNames[i].getLocalBounds().height/2.f );
		worldNames[i].setPosition( window->getSize().x / 2, 50 );
	}

	sf::Texture worldSelectTexture;
	if( !worldSelectTexture.loadFromFile( "Resources/Menus/worldselectscreen.png" ) )
	{
		cout << "error loading world select screen image" << endl;
	}
	sf::Texture worldSelectedButtonTexture;
	if( !worldSelectedButtonTexture.loadFromFile( "Resources/Menus/worldselectedbutton.png" ) )
	{
		cout << "error loading world selected button image" << endl;
	}
	sf::Sprite worldSelectScreenSprite;
	worldSelectScreenSprite.setTexture( worldSelectTexture );
	
	sf::Sprite worldSelectedButtonSprite;
	worldSelectedButtonSprite.setTexture( worldSelectedButtonTexture );
	worldSelectedButtonSprite.setOrigin( worldSelectedButtonSprite.getLocalBounds().left 
		+ worldSelectedButtonSprite.getLocalBounds().width / 2.f, 
		worldSelectedButtonSprite.getLocalBounds().top 
		+ worldSelectedButtonSprite.getLocalBounds().height / 2.f ); 

	//LEVEL MENU STUFF
	sf::Texture verticalFarmingBackground;
	if( !verticalFarmingBackground.loadFromFile( "Resources/Menus/verticalfarminglevelscreen.png" ) )
	{
		cout << "error loading world selected button image" << endl;
	}

	sf::Sprite verticalFarmingBackgroundSprite;
	verticalFarmingBackgroundSprite.setTexture( verticalFarmingBackground );

	sf::Texture levelMenuFrameTexture;
	if( !levelMenuFrameTexture.loadFromFile( "Resources/Menus/levelmenuframe.png" ) )
	{
		cout << "error loading t1thumbnail image" << endl;
	}

	sf::Sprite levelMenuFrameSprite;
	levelMenuFrameSprite.setTexture( levelMenuFrameTexture );

	sf::Texture t1thumbnail;
	if( !t1thumbnail.loadFromFile( "Resources/Worlds/Vertical Farming/Thumbnails/t1thumbnail.png" ) )
	{
		cout << "error loading world selected button image" << endl;
	}

	sf::Texture levelSelectedHighlightTexture;
	if( !levelSelectedHighlightTexture.loadFromFile( "Resources/Menus/levelselectedhighlight.png" ) )
	{
		cout << "error loading world selected button image" << endl;
	}

	sf::Sprite levelSelectedHighlightSprite;
	levelSelectedHighlightSprite.setTexture( levelSelectedHighlightTexture );

	sf::Texture levelSelectedHighlightCircleTexture;
	if( !levelSelectedHighlightCircleTexture.loadFromFile( "Resources/Menus/levelselectedhighlightcircle.png" ) )
	{
		cout << "error loading world selected button image" << endl;
	}

	sf::Sprite levelSelectedHighlightCircleSprite;
	levelSelectedHighlightCircleSprite.setTexture( levelSelectedHighlightCircleTexture );
	levelSelectedHighlightCircleSprite.setPosition( 1490, 766 - 50 );

	levelThumbnails[0].setPosition( 70, 766 );
	levelThumbnails[1].setPosition( 563.65, 766 );
	levelThumbnails[2].setPosition( 1057.30, 766 );

	levelFiles[0] = "design";
	levelFiles[1] = "design2";//"t1-3-1";//"design";//"t2-1";
	levelFiles[2] = "design3";
	//levelFiles[3] = "boss";

	
	
	//sf::Sprite t1thumbnailSprite;
	//t1thumbnailSprite.setTexture( t1thumbnail );
	//t1thumbnailSprite.setPosition( 70, 766 );

	//load and display the images and stuff after you select the world or have it buffered beforehand?


	ControllerState prevMenuInput;
	ControllerState menuInput;
	
	//game menu, then world menu, then level menu, then play the game
	
	bool done = false;
	std::string stage;
	while( !done )
	{
		controller.UpdateState();

		

		prevMenuInput = menuInput;
		

		if( !controller.UpdateState() )
		{
		/*	bool up = Keyboard::isKeyPressed( Keyboard::Up ) || Keyboard::isKeyPressed( Keyboard::Num2 );
			bool down = Keyboard::isKeyPressed( Keyboard::Down ) || Keyboard::isKeyPressed( Keyboard::W );
			bool left = Keyboard::isKeyPressed( Keyboard::Left ) || Keyboard::isKeyPressed( Keyboard::Q );
			bool right = Keyboard::isKeyPressed( Keyboard::Right ) || Keyboard::isKeyPressed( Keyboard::E );

			ControllerState keyboardInput;    
			keyboardInput.B = Keyboard::isKeyPressed( Keyboard::D ) || Keyboard::isKeyPressed( Keyboard::Numpad4 );
			keyboardInput.X = Keyboard::isKeyPressed( Keyboard::F ) || Keyboard::isKeyPressed( Keyboard::Numpad7 );
			keyboardInput.Y = Keyboard::isKeyPressed( Keyboard::A ) || Keyboard::isKeyPressed( Keyboard::Numpad8 );
			keyboardInput.A = Keyboard::isKeyPressed( Keyboard::Space ) || Keyboard::isKeyPressed( Keyboard::Numpad5 );
			keyboardInput.start = Keyboard::isKeyPressed( Keyboard::J );*/

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
				if( prevMenuInput.Up() )
					keyboardInput.pad += 1;
				else if( prevMenuInput.Down() )
					keyboardInput.pad += ( 1 && down ) << 1;
			}
			else
			{
				keyboardInput.pad += 1 && up;
				keyboardInput.pad += ( 1 && down ) << 1;
			}

			if( left && right )
			{
				if( prevMenuInput.Left() )
				{
					keyboardInput.pad += ( 1 && left ) << 2;
				}
				else if( prevMenuInput.Right() )
				{
					keyboardInput.pad += ( 1 && right ) << 3;
				}
			}
			else
			{
				keyboardInput.pad += ( 1 && left ) << 2;
				keyboardInput.pad += ( 1 && right ) << 3;
			}

			menuInput = keyboardInput;
		}
		else
		{
			menuInput = controller.GetState();

			menuInput.X = currentInput.X || menuInput.rightShoulder;
			menuInput.rightShoulder = menuInput.rightTrigger > 10;
			menuInput.Y = currentInput.Y || menuInput.leftShoulder;
			menuInput.leftShoulder = menuInput.leftTrigger > 10;
		
			if( menuInput.leftStickMagnitude > .4 )
			{
				float x = cos( menuInput.leftStickRadians );
				float y = sin( menuInput.leftStickRadians );
				float threshold = .8; //changed for menu stuff
				if( x > threshold )
					menuInput.pad += 1 << 3;
				if( x < -threshold )
					menuInput.pad += 1 << 2;
				if( y > threshold )
					menuInput.pad += 1;
				if( y < -threshold )
					menuInput.pad += 1 << 1;
			}
		}

		window->clear();

		if( currentMenu == "main" )
		{
			if( menuInput.Down() && !prevMenuInput.Down() )
			{
				if( selectedMainMenu == NUM_MAIN_OPTIONS - 1 )
				{
					selectedMainMenu = 0;
				}
				else
				{
					++selectedMainMenu;
				}
			}
			if( menuInput.Up() && !prevMenuInput.Up() )
			{
				if( selectedMainMenu == 0 )
				{
					selectedMainMenu = NUM_MAIN_OPTIONS - 1;
				}
				else
				{
					--selectedMainMenu;
				}
			}

			window->draw( mainMenuSprite );
		
			for( int i = 0; i < NUM_MAIN_OPTIONS; ++i )
			{
				if( i == selectedMainMenu )
				{
					menuWords[i].setColor( sf::Color::Red );
				}
				else
				{
					menuWords[i].setColor( sf::Color::White );
				}
				window->draw( menuWords[i] );
			}

			if( menuInput.A && !prevMenuInput.A )
			{
				if( menuWords[selectedMainMenu].getString() == "Play" )
				{
					SetCurrentMenu( "world" );
				}
				else if( menuWords[selectedMainMenu].getString() == "Quit" )
				{
					done = true;
					break;
					//break;
				}
				
			}
		}
		else if( currentMenu == "world" )
		{
			if( menuInput.Down() && !prevMenuInput.Down() )
			{
				if( selectedWorldMenu > 2 )
				{
					selectedWorldMenu -= 3;
				}
				else
				{
					selectedWorldMenu += 3;
				}
			}
			else if( menuInput.Up() && !prevMenuInput.Up() )
			{
				if( selectedWorldMenu <= 2 )
				{
					selectedWorldMenu += 3;
				}
				else
				{
					selectedWorldMenu -= 3;
				}
			}
			else if( menuInput.Right() && !prevMenuInput.Right() )
			{
				if( selectedWorldMenu == 2 || selectedWorldMenu == 5 )
				{
					selectedWorldMenu -= 2;
				}
				else
				{
					++selectedWorldMenu;
				}
			}
			else if( menuInput.Left() && !prevMenuInput.Left() )
			{
				if( selectedWorldMenu == 0 || selectedWorldMenu == 3 )
				{
					selectedWorldMenu += 2;
				}
				else
				{
					--selectedWorldMenu;
				}
			}

			window->draw( worldSelectScreenSprite );
			worldSelectedButtonSprite.setPosition( worldButtonPositions[selectedWorldMenu] );
			window->draw( worldSelectedButtonSprite );
			window->draw( worldNames[selectedWorldMenu] );

			if( menuInput.B && !prevMenuInput.B )
			{
				SetCurrentMenu( "main" );
			}
			else if( menuInput.A && !prevMenuInput.A )
			{
				SetCurrentMenu( "level" );
			}

		}
		else if( currentMenu == "level" )
		{
			if( menuInput.Right() && !prevMenuInput.Right() )
			{
				if( selectedLevelMenu == 3 )
				{
					selectedLevelMenu = 0;
				}
				else
				{
					++selectedLevelMenu;
				}
			}
			else if( menuInput.Left() && !prevMenuInput.Left() )
			{
				if( selectedLevelMenu == 0 )
				{
					selectedLevelMenu = 3;
				}
				else
				{
					--selectedLevelMenu;
				}
			}


			if( selectedWorldMenu == 0 ) //vertical farming
			{
				window->draw( verticalFarmingBackgroundSprite );
				levelThumbnails[0].setTexture( t1thumbnail );
				//levelThumbnails[1].setTexture();
				//levelThumbnails[2].setTexture();
			}

			if( selectedLevelMenu < 3 )
			{
				levelSelectedHighlightSprite.setPosition( levelThumbnails[selectedLevelMenu].getPosition().x - 40,
					levelThumbnails[selectedLevelMenu].getPosition().y - 40 );
				window->draw( levelSelectedHighlightSprite );
			}
			else
			{
				window->draw( levelSelectedHighlightCircleSprite );
				//selectedLevelMenu
			}

			window->draw( levelMenuFrameSprite );
			for( int i = 0; i < 3; ++i )
			{
				window->draw( levelThumbnails[i] );
			}
			

			if( menuInput.B && !prevMenuInput.B )
			{
				SetCurrentMenu( "world" );
			}
			else if( menuInput.A && !prevMenuInput.A )
			{
				//currentMenu = "level";
				done = RunStage( levelFiles[selectedLevelMenu] );
				window->setView( menuView );
				if( done ) break;
			}
		}
		else
		{
			cout << "error no menu: " << currentMenu << endl;
		}


		window->display();
		//std::cin >> stage;
		//stage = "t2-2";
		//window = new sf::RenderWindow(/*sf::VideoMode(1400, 900)sf::VideoMode::getDesktopMode()
//		//	/*sf::VideoMode( 1024, 768)*/ sf::VideoMode::getFullscreenModes()[0], "", sf::Style::None, sf::ContextSettings( 0, 0, 0, 0, 0 ));
		
		


		/*sf::Event chooseStageEvent;
		while( stage == "" && !done )
		{
			while( window->pollEvent( chooseStageEvent ) )
			{
			
			}

			if( sf::Keyboard::isKeyPressed( sf::Keyboard::Num1 ) )
			{
				stage = "t1-1";
			}
			else if( sf::Keyboard::isKeyPressed( sf::Keyboard::Num2 ) )
			{
				stage = "t2-1";
			}
			else if( sf::Keyboard::isKeyPressed( sf::Keyboard::Num3 ) )
			{
				stage = "t2";
			}
			else if( sf::Keyboard::isKeyPressed( sf::Keyboard::Escape ) )
			{
				done = true;
			}
		}
		if( done )
		{
			break;
		}*/
		
		//RunStage( stage );
		
		//Shutdown();
	}

	Shutdown();
	
	//std::stringstream ss;
	//ss << stage << ".tmx";
	//std::cout << "dims: " << sf::VideoMode::getDesktopMode().width << ", " << sf::VideoMode::getDesktopMode().height << std::endl;
	
	

	
    return 0;
}

void Shutdown()
{

	delete Stage::pauseBGTexture;
	delete Stage::pauseBGSprite;

	window->close();
	delete window;
}

bool RunStage( const std::string &stageName )
{
	//load the stage

	sf::RectangleShape r;
	r.setSize( sf::Vector2f( 45, 55 ) );

	int zeroIndex = 0;
	
	
	std::stringstream ss;

	std::string tempStr = worldNames[selectedWorldMenu].getString();
	ss << "Resources/Worlds/" << tempStr << "/";
	cout << "stageName: " << stageName << endl;
	//Actor *z = new Zero( b2Vec2( 325, 200 ), input );
	Stage *stage = new Stage( controller, window, ss.str() , stageName );
	bool quit = stage->Run();
	
	delete stage;
	
	return quit;
}

void SetCurrentMenu( const std::string menu )
{
	currentMenu = menu;
	selectedMainMenu = 0;
	selectedLevelMenu = 0;
	//selectedWorldMenu = 0;
}