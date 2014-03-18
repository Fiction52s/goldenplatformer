#include "Action.h"
#include <assert.h>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace sf;


Action::Action( Actor *a, const std::string &name )
	:actor( a ), name( name ),
	currentFrame( 0 )//, m_body( NULL )
{
	//std::stringstream ss;
	////ss << a->name << "/" << name << ".action";

	//ss << "Actions/" << name << ".action";
	//std::ifstream ifs;
	//
	//ifs.open( ss.str() );

	//if( !ifs.is_open() )
	//{
	//	std::cout << ss.str() << std::endl;
	//	assert( 0 && "file not open!" );
	//}

	//std::string keyword;
	//while( ifs.good() )
	//{
	//	ifs >> keyword;
	//	if( keyword == "" )
	//		continue;

	//	if( keyword == "startx=" )
	//	{
	//		ifs >> startSprite.x;
	//	}
	//	else if( keyword == "starty=" )
	//	{
	//		ifs >> startSprite.y;
	//	}
	//	else if( keyword == "width=" )
	//	{
	//		ifs >> spriteSize.x;
	//	}
	//	else if( keyword == "height=" )
	//	{
	//		ifs >> spriteSize.y;
	//	}
	//	else if( keyword == "numFrames=" )
	//	{
	//		ifs >> lastFrame;
	//		--lastFrame; //-- because lastFrame is one less than numFrames
	//	}
	//	else if( keyword == "hitboxStartFrame=" )
	//	{
	//		ifs >> hitboxStartFrame;
	//	}
	//	else if( keyword == "hitboxLastFrame=" )
	//	{
	//		ifs >> hitboxLastFrame;
	//	}
	//}

	//ifs.close();
	//if( hitboxLastFrame < hitboxStartFrame )
	//{
	//	std::cout << "WARNING: hitboxlast frame is less than hitboxstartframe" 
	//		<< std::endl;
	//	hitboxLastFrame = lastFrame;
	//}

	//ss.str( std::string() );
	//ss << "Actions/" << name << ".phys";
	//ifs.open( ss.str() );

	//if( ifs.is_open() )
	//{
	//	body = new DynamicObject();
	//	
	//	while( ifs.good() )
	//	{
	//		ifs >> keyword;
	//		if( keyword == "" )
	//			continue;

	//		else if( keyword == "rect=" )
	//		{
	//			AABB *bodyBox = new AABB;
	//			ifs >> bodyBox->pos.x;
	//			ifs >> bodyBox->pos.y;
	//			ifs >> bodyBox->size.x;
	//			ifs >> bodyBox->size.y;
	//			bodyBox->owner = body;
	//			body->maxVel = Vector2f( 600, 2000 );
	//			//^^this should read from a file
	//			body->mass = 1;
	//			body->boxes.push_back( bodyBox );
	//			body->userData = (sf::Uint32)actor;
	//		}
	//	}
	//}
}

//returns false
void Action::Update()
{
	assert( isActive );

	if(	currentFrame == hitboxStartFrame )
	{
		//actor->objectHitbox = hitbox;
	}
	if( currentFrame > hitboxLastFrame )
	{
		//actor->objectHitbox = NULL;
	}

	IntRect sub;
	sub.left = currentFrame  * spriteSize.x + startSprite.x;
	sub.top = startSprite.y;
	sub.width = spriteSize.x;
	sub.height = spriteSize.y;

	int levelsDown = 0;
	while( sub.left > actor->sprite->getTexture()->getSize().x )
	{
		sub.left -= actor->sprite->getTexture()->getSize().x;
		assert( sub.left > 0 );
		sub.top += spriteSize.y;
	}
	
	actor->sprite->setTextureRect( sub );
	actor->sprite->setOrigin( sub.width / 2.f, sub.height / 2.f );

	//++currentFrame;

	if( currentFrame > lastFrame )
		isActive = false;
}

void Action::SetToFrame( int frame )
{
	currentFrame = frame;
	isActive = true;
	//actor->objectBody = body;
}
