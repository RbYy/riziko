
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
const int HIGH_WIDTH = 168;
const int SEVEN_WIDTH = 78;
const int BUTTON_HEIGHT = 100;
const int TOTAL_BUTTONS = 3;
int gcardID;
int gScore;
char gWIN;


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
		int mCurrentSprite;  // Currently used global sprite
		int mButtonID;  // 0, 1, 2; for high, 7, low
};

bool init();  //Starts up SDL and creates window

bool loadCards();  //Loads cards, put them into array, [0] is the card backside

void close();  //Frees media and shuts down SDL

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
TTF_Font* greetingFont = NULL;

//Mouse button sprites
SDL_Rect gSpriteClips[9]; // three different states for three buttons
LTexture gButtonSpriteSheetTexture; 

LButton gButtons[TOTAL_BUTTONS]; //Buttons objects

LTexture gCardTexture [53]; //card textures

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
	if(mTexture != NULL)
	{
		SDL_DestroyTexture(mTexture);
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}


void LTexture::render( int x, int y, SDL_Rect* clip = NULL )
{
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

class MessageTexture: public LTexture // generate message texture for rendering
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
			TTF_CloseFont(font);
			mTexture = Message;
			return mTexture !=NULL;
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

void LButton::handleEvent(SDL_Event* e ) // handle mouse events in PLAY state
{
	if( e->type == SDL_MOUSEMOTION || e->type == SDL_MOUSEBUTTONDOWN || e->type == SDL_MOUSEBUTTONUP)
	{
		// Get mouse position
		int x, y, result;
		int winLose;
		SDL_GetMouseState(&x, &y);
		bool inside = true; // mouse inside button flag

		// mouse outside contitions
		if( x < mPosition.x ) inside = false;  //Mouse is left of the button
		else if(x > mPosition.x + HIGH_WIDTH) inside = false; //Mouse is right of the button
		else if(y < mPosition.y) inside = false;  //Mouse above the button
		else if(y > mPosition.y + BUTTON_HEIGHT) inside = false;  //Mouse below the button

		//Mouse is outside button
		if( !inside ) mCurrentSprite = 3 * (mButtonID+1)-3;  //default button (white border)
		
		else  //Mouse is inside button
		{
			switch(e->type)
			{
				case SDL_MOUSEMOTION:
				mCurrentSprite = 1 + 3 * mButtonID;
				break;
			
				case SDL_MOUSEBUTTONDOWN:
				mCurrentSprite = 2 + 3 * mButtonID;
				break;	

				case SDL_MOUSEBUTTONUP:
		        gcardID = rand() % 52 +1;  // pick a random number from 1 to 52 (also used to generate a file name)
		        result = gcardID % 13; 
        		if (result == 0) result = 13;  // "convert" to numberts 1 to 13 (A, 1 .. Q, K)
        		winLose = gScore; // tomporary variable for comparing to gScore. Tells if the player wins or loses a turn
        		switch(mButtonID)  // click button
        		{
        			case 0:  // High 
	        			if (result > 7) gScore++;
	        			else gScore--;
						break;
        			case 1:  // Seven
	        			if (result == 7) gScore+=4;
	        			else gScore--;
						break;
        			case 2:  // Low
        				if (result < 7) gScore++;
        				else gScore--; 
						break;
        		}
        		if (gScore > winLose) 
        			gWIN = 'w';  // used for rendering win/lose message after every turn
        		else gWIN = 'l';

        		
				mCurrentSprite = 1 + 3 * mButtonID; // set a hover sprite
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
	SDL_Init(SDL_INIT_VIDEO);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");  
	//Create window
	gWindow = SDL_CreateWindow(
		"Riziko", 
		SDL_WINDOWPOS_UNDEFINED, 
		SDL_WINDOWPOS_UNDEFINED, 
		SCREEN_WIDTH, 
		SCREEN_HEIGHT, 
		SDL_WINDOW_SHOWN);

	//Create renderer for window
	gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);

	SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);  //Initialize renderer color

	int imgFlags = IMG_INIT_PNG;  //Initialize PNG loading
	IMG_Init(imgFlags);
	TTF_Init();
	return true;
}

bool buildButtons() // creates buttons from spritesheet
{
	//Load sprites
	gButtonSpriteSheetTexture.loadFromFile( "spritesheet.png" );

	for( int i = 0; i < 3; ++i )
	{ // seven button
		gSpriteClips[ i ].x = SEVEN_WIDTH;
		gSpriteClips[ i ].y = i * BUTTON_HEIGHT + i*5;
		gSpriteClips[ i ].w = HIGH_WIDTH;
		gSpriteClips[ i ].h = BUTTON_HEIGHT;
	}

	for( int i = 3; i < 6; ++i )
	{  // seven button
		gSpriteClips[ i ].x = 0;
		gSpriteClips[ i ].y = (i-3) * BUTTON_HEIGHT;
		gSpriteClips[ i ].w = SEVEN_WIDTH;
		gSpriteClips[ i ].h = BUTTON_HEIGHT;
	}

	for( int i = 6; i < 9; ++i )
	{  // low button
		gSpriteClips[ i ].x = SEVEN_WIDTH + HIGH_WIDTH;
		gSpriteClips[ i ].y = (i-6) * BUTTON_HEIGHT;
		gSpriteClips[ i ].w = HIGH_WIDTH;
		gSpriteClips[ i ].h = BUTTON_HEIGHT;
	}		
	// position buttons to the right side of the window


	for(int i=0; i< TOTAL_BUTTONS; ++i)
	{
		gButtons[i].setPosition(SCREEN_WIDTH - HIGH_WIDTH - 100, BUTTON_HEIGHT * (i + 1));
		gButtons[i].setButtonID(i);
	}
	return true;
}

bool loadCards() //  loads the cards and generate textures
{
	string pngPath, cardNr;
	for (int i=0; i<53; ++i)
	{
		cardNr = to_string(i);
		pngPath = "cards/" + cardNr + ".png";
		gCardTexture[i].loadFromFile( pngPath );
	}
	return true;
}

void close()
{
	gCardTexture[gcardID].free();  //Free loaded images	
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;
	
	IMG_Quit();  //Quit SDL subsystems
	SDL_Quit();
	TTF_Quit();
}

int main( int argc, char* args[] )
{
	srand(time(NULL));
	const int WIN_SCORE = 25;
	const int LOSE_SCORE = 0;
	const int START_SCORE = 10;
	gScore = START_SCORE;

	enum GameStates 
	{ 
		START, 
		PLAY, 
		GAME_OVER
	} gameState;

	init(); //Start up SDL and create window
	loadCards(); // loads all the cards	
	buildButtons(); //  build buttons from sprites
	SDL_Color White = {255, 255, 255};
	MessageTexture 
		greetingText,
		continueText,
		replayText,
		resultText,
		scoreText,
		gameOverDefeatText,
		presentationText,
		gameOverWinText,
		winText,
		loseText;

	// text messages textures
	replayText.create("klik za novo igro", greetingFont, 15, White);
	gameOverDefeatText.create("Konec igre. Izgubili ste.", greetingFont, 25, White);
	greetingText.create("Pozdravljeni", greetingFont, 40, White);
	gameOverWinText.create("Konec igre. ZMAGA!", greetingFont, 25, White);
	presentationText.create("Dobrodosli v poenostavljeni igri RIZIKO", greetingFont, 30, White);
	continueText.create("klikni za nadaljevanje", greetingFont, 20, White);
	winText.create("BRAVO! Tocka za vas!", greetingFont, 30, White);
	loseText.create("Narobe. Izgubili ste tocko", greetingFont, 20, White);

	bool quit = false;  //Main loop flag
	SDL_Event e;  //Event handler
	gameState = START;
	while(!quit)  // game loop
	{
		while( SDL_PollEvent( &e ) != 0 ) // handling even loop
		{
			if( e.type == SDL_QUIT ) quit = true;  //  user requests quit - click on X

			switch(gameState)
			{
				case PLAY: // event handling PLAY state
					for(int i = 0; i < TOTAL_BUTTONS; ++i) 
					{
						gButtons[ i ].handleEvent(&e);
					}
					break;

				case GAME_OVER: // event handling GAME_OVER state
					if (e.type == SDL_MOUSEBUTTONUP)
					{
						gameState = PLAY;
						gScore = START_SCORE; //reset score
						gcardID = 0;  //reset starting card to card backside
					}
					break;

				case START: // event handling START state
					if (e.type == SDL_MOUSEBUTTONUP)
						{
						gameState = PLAY;
						gScore = START_SCORE;
						gcardID = 0;  //reset starting card to card backside
						}
					break;		
			}
		}

		SDL_SetRenderDrawColor(gRenderer, 0x00, 0x80, 0x00, 0x00);
		SDL_RenderClear(gRenderer);

		switch(gameState) //rendering game states
		{
			case START:
				greetingText.render(200, 100);
				presentationText.render(60, 200);
				continueText.render(130, 380);
				break;

			case PLAY:
				gCardTexture[gcardID].render(100, 100);
				for(int i = 0; i < TOTAL_BUTTONS; ++i)
				{	
					scoreText.create("stanje: "+ to_string(gScore), greetingFont, 20, White);
					scoreText.render(110, 360);
					gButtons[i].render();

					if (gWIN == 'w') 
						winText.render(110, 420);

					else if (gWIN == 'l') 
						loseText.render(110, 420);
					
					if (gScore == LOSE_SCORE || gScore == WIN_SCORE) 
						gameState = GAME_OVER;
				}
				break;

			case GAME_OVER:
				gWIN = NULL;
				if (gScore == LOSE_SCORE)
				{
					gameOverDefeatText.render(200, 120);
					replayText.render(200, 160);
				}
				else
				{
					gameOverWinText.render(200, 120);
					replayText.render(200, 160);
				}
				break;		
		}
		SDL_RenderPresent( gRenderer );	
	}
	close();

	return 0;
}
