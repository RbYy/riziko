
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
using namespace std;

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

//Button constants
const int BUTTON_WIDTH = 168;
const int BUTTON_HEIGHT = 100;
const int TOTAL_BUTTONS = 3;
int gcardID = 0;
int gScore = 10;



//Texture wrapper class
class LTexture
{
	public:
		LTexture(); 

		~LTexture(); 

		bool loadFromFile( string path );  //Loads image at specified path

		void free();  //Deallocates texture

		void render( int x, int y, SDL_Rect* clip ); 
		
		int getWidth();  //Gets image dimensions
		int getHeight();

	protected:
		SDL_Texture* mTexture; 

		int mWidth;  //Image dimensions
		int mHeight;
};

//The mouse button
class LButton
{
	public:
		LButton();
		void setPosition( int x, int y );
		void setButtonID( int id);
		void handleEvent(SDL_Event* e );
		void render();

	private:
		//Top left position
		SDL_Point mPosition;

		int mCurrentSprite;  //Currently used global sprite
		int mButtonID;  // 0, 1, 2 for high, 7, low
};

bool init();  //Starts up SDL and creates window

bool loadCards();  //Loads cards, put them into array, [0] is card back

void close();  //Frees media and shuts down SDL

SDL_Window* gWindow = NULL;

SDL_Renderer* gRenderer = NULL;

//Mouse button sprites
SDL_Rect gSpriteClips[ 9 ]; // three different states for three buttons
LTexture gButtonSpriteSheetTexture;

LButton gButtons[ TOTAL_BUTTONS ]; //Buttons objects
TTF_Font* greetingFont = NULL;
TTF_Font* resultFont = NULL;
TTF_Font* instructionFont = NULL;

LTexture cardTexture [53]; //card textures

LTexture::LTexture()
{
	//Initialize
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

LTexture::~LTexture(){ free(); }


bool LTexture::loadFromFile( string path )
{
	free();  //destroy a preexisting texture

	SDL_Texture* newTexture = NULL;  //the final texture

	SDL_Surface* loadedSurface = IMG_Load( path.c_str() ); 

    newTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );

	mWidth = loadedSurface->w; 
	mHeight = loadedSurface->h;
	
	SDL_FreeSurface( loadedSurface );  //destroy old loaded surface

	mTexture = newTexture;
	return mTexture != NULL;
}

void LTexture::free()
{	//Free texture if it exists
	if( mTexture != NULL )
	{
		SDL_DestroyTexture( mTexture );
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}


void LTexture::render( int x, int y, SDL_Rect* clip = NULL )
{
    //Set rendering space and render to screen
    SDL_Rect renderQuad = { x, y, mWidth, mHeight };

    //Set clip rendering dimensions
    if( clip != NULL )
    {
        renderQuad.w = clip->w;
        renderQuad.h = clip->h;
    }
    //Render to screen
    SDL_RenderCopy( gRenderer, mTexture, clip, &renderQuad );
}

int LTexture::getWidth()
{
	return mWidth;
}

int LTexture::getHeight()
{
	return mHeight;
}

class MessageTexture: public LTexture
{
	public:
		bool create(string text, TTF_Font* font, int fontSize, SDL_Color color)
		{
			free();
			font = TTF_OpenFont("./FreeSans.ttf", fontSize); 
			TTF_SizeText(font, text.c_str(), &mWidth, &mHeight);
			SDL_Surface* surfaceMessage = TTF_RenderText_Solid(font, text.c_str(), color);
			SDL_Texture* Message = SDL_CreateTextureFromSurface(gRenderer, surfaceMessage);
			SDL_FreeSurface(surfaceMessage);
			TTF_CloseFont( font );
			mTexture = Message;
		}
};

LButton::LButton()
{
	mPosition.x = 0;
	mPosition.y = 0;
	mButtonID = 0;
}

void LButton::setPosition( int x, int y )
{
	mPosition.x = x;
	mPosition.y = y;
}

void LButton::setButtonID(int id)
{ 
	mButtonID = id;
	mCurrentSprite = mButtonID * 3;
}

void LButton::handleEvent(SDL_Event* e )
{
	// mouse event
	if( e->type == SDL_MOUSEMOTION || e->type == SDL_MOUSEBUTTONDOWN || e->type == SDL_MOUSEBUTTONUP)
	{
		//Get mouse position
		int x, y, result;
		SDL_GetMouseState( &x, &y );

		//Check if mouse is in button
		bool inside = true;

		//Mouse is left of the button
		if( x < mPosition.x )
		{
			inside = false;
		}
		//Mouse is right of the button
		else if( x > mPosition.x + BUTTON_WIDTH )
		{
			inside = false;
		}
		//Mouse above the button
		else if( y < mPosition.y )
		{
			inside = false;
		}
		//Mouse below the button
		else if( y > mPosition.y + BUTTON_HEIGHT )
		{
			inside = false;
		}

		//Mouse is outside button
		if( !inside )
		{
			mCurrentSprite = 3 * (mButtonID+1)-3;
		}
		//Mouse is inside button
		else
		{
			//Set mouse over sprite
			mCurrentSprite = 3 * (mButtonID+1)-3;

			switch( e->type )
			{
				case SDL_MOUSEMOTION:
				mCurrentSprite = 1 + 3 * mButtonID;
				break;
			
				case SDL_MOUSEBUTTONDOWN:
				mCurrentSprite = 2 + 3 * mButtonID;
				break;	

				case SDL_MOUSEBUTTONUP:
		        gcardID = (rand() % 52)+1;
		        result = gcardID % 13;
        		if (result == 0) result = 13;
        		switch(mButtonID)
        		{
        			case 0:  // High
	        			if (result > 7) 
	        			{	
	        				gScore++;
	        			}
	        			else
        				{
        					gScore--;
        				}
						break;
        			case 1:  // Seven
	        			if (result == 7)
	        			{

	        				gScore+=4;
	        			}
	        			else
	        			{
	        				gScore--;
	        			}
						break;
        			case 2:  // Low
        				if (result < 7)
        				{
        					gScore++;
        				}
        				else 
        				{
        					gScore--;
        				}
						break;
        		}
				mCurrentSprite = 1 + 3 * mButtonID;
				break;							
			}
		}
	}
}
	
void LButton::render()
{
	//Show current button sprite
	gButtonSpriteSheetTexture.render( mPosition.x, mPosition.y, &gSpriteClips[ mCurrentSprite ] );
}



bool init()
{	
	SDL_Init( SDL_INIT_VIDEO );
	SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" );  //Set texture filtering to linear
	//Create window
	gWindow = SDL_CreateWindow(
		"Riziko", 
		SDL_WINDOWPOS_UNDEFINED, 
		SDL_WINDOWPOS_UNDEFINED, 
		SCREEN_WIDTH, 
		SCREEN_HEIGHT, 
		SDL_WINDOW_SHOWN );

	//Create renderer for window
	gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED );

	SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );  //Initialize renderer color

	int imgFlags = IMG_INIT_PNG;  //Initialize PNG loading
	IMG_Init( imgFlags );
	TTF_Init();
	return true;
}

bool buildButtons() // creates buttons from spritesheet
{
	//Load sprites
	gButtonSpriteSheetTexture.loadFromFile( "spritesheet.png" );

	for( int i = 0; i < 3; ++i )
	{ // seven button
		gSpriteClips[ i ].x = 78;
		gSpriteClips[ i ].y = i * 100;
		gSpriteClips[ i ].w = BUTTON_WIDTH;
		gSpriteClips[ i ].h = BUTTON_HEIGHT;
	}

	for( int i = 3; i < 6; ++i )
	{  // high button
		gSpriteClips[ i ].x = 0;
		gSpriteClips[ i ].y = (i-3) * 100;
		gSpriteClips[ i ].w = 78;
		gSpriteClips[ i ].h = BUTTON_HEIGHT;
	}

	for( int i = 6; i < 9; ++i )
	{  // low button
		gSpriteClips[ i ].x = 246;
		gSpriteClips[ i ].y = (i-6) * 100;
		gSpriteClips[ i ].w = 167;
		gSpriteClips[ i ].h = BUTTON_HEIGHT;
	}		
	// position buttons to the right side of the window
	gButtons[ 0 ].setPosition( SCREEN_WIDTH - BUTTON_WIDTH, 20 );
	gButtons[ 1 ].setPosition( SCREEN_WIDTH - BUTTON_WIDTH, BUTTON_HEIGHT+20 );
	gButtons[ 2 ].setPosition( SCREEN_WIDTH - BUTTON_WIDTH, 2*BUTTON_HEIGHT +30);

	for(int i=0; i< TOTAL_BUTTONS; ++i)
	{
		gButtons[i].setButtonID(i);
	}
	return true;
}


bool loadCards()
{
	string pngPath, cardNr;
	for ( int i=0; i<53; ++i )
	{
		cardNr = to_string( i );
		pngPath = "cards/" + cardNr + ".png";
		cardTexture[i].loadFromFile( pngPath );
	}
	return true;
}

void close()
{
	cardTexture[gcardID].free();  //Free loaded images	
	SDL_DestroyRenderer( gRenderer );
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;
	gRenderer = NULL;
	
	IMG_Quit();  //Quit SDL subsystems
	SDL_Quit();
	TTF_Quit();
}

enum GameStates { START, PLAY, GAME_OVER};
GameStates gameState;

int main( int argc, char* args[] )
{
	srand(time(NULL));
	init(); //Start up SDL and create window
	buildButtons();
	SDL_Color White = {255, 255, 255};
	MessageTexture 
		greetingText,
		instructionsText,
		continueText,
		replayText,
		resultText, 
		scoreText,
		gameOverDefeatText,
		presentationText,
		gameOverWinText;

	// text messages textures
	replayText.create("Pritisni ENTER za novo igro", greetingFont, 15, White);
	gameOverDefeatText.create("Konec igre. Izgubili ste.", greetingFont, 25, White);
	greetingText.create("Pozdravljeni", greetingFont, 40, White);
	gameOverWinText.create("Konec igre. ZMAGA!", greetingFont, 25, White);
	presentationText.create("Dobrodosli v poenostavljeni igri RIZIKO", greetingFont, 30, White);
	continueText.create("za nadaljevanje pritisnite tipko ENTER", greetingFont, 20, White);

	loadCards(); // loads all the cards
	bool quit = false;  //Main loop flag

	SDL_Event e;  //Event handler
	gameState = START;
	while( !quit )  // game loop
	{
		while( SDL_PollEvent( &e ) != 0 ) // handling even loop
		{
			if( e.type == SDL_QUIT )
			{	//User requests quit - click on X
				quit = true;
			}
			for(int i = 0; i < TOTAL_BUTTONS; ++i)
			{
				gButtons[ i ].handleEvent(&e);
			}	
			if (gameState == GAME_OVER)
			{
				if (e.type == SDL_KEYDOWN)
					if (e.key.keysym.sym == SDLK_RETURN)
					{
						gameState = PLAY;
						gScore = 10; //reset score
						gcardID = 0;  //reset starting card to card back
					}
			}
			if (gameState == START)
			{
				if (e.type == SDL_KEYDOWN)
					if (e.key.keysym.sym == SDLK_RETURN)
					{
						gameState = PLAY;
						gScore = 10;
					}
			}			
		}

		SDL_SetRenderDrawColor( gRenderer, 0x00, 0x80, 0x00, 0x00);
		SDL_RenderClear( gRenderer );

		if (gameState == START)
		{
			greetingText.render(200, 100);
			presentationText.render(60, 200);
			continueText.render(130, 380);
		}

		if (gameState == PLAY)
		{
			cardTexture[gcardID].render(100, 100);
			for( int i = 0; i < TOTAL_BUTTONS; ++i )
			{	
				scoreText.create("stanje: "+ to_string(gScore), greetingFont, 20, White);
				scoreText.render(110, 360);
				gButtons[i].render();
				if ((gScore == 0) || (gScore == 25)) gameState = GAME_OVER;
			}	
		}

		if (gameState == GAME_OVER)
		{
			if (gScore == 0)
			{
				gameOverDefeatText.render(200, 120);
				replayText.render(200, 160);
			}
			if (gScore == 25)
			{
				gameOverWinText.render(200, 120);
				replayText.render(200, 160);
			}			
		}
		SDL_RenderPresent( gRenderer );	
	}	

	close();

	return 0;
}
