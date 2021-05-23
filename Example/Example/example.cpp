/* Alphar: 2021.04.10 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <SDL.h> // Using SDL
#include "SDL2_gfxPrimitives.h" // Using SDL2_gfx
#include "SDL_image.h"  // Using SDL2 image extension library 

//Screen dimension constants
const int LEVEL_WIDTH = 1376;
const int LEVEL_HEIGHT = 480;
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int PIKAFRAME = 16;
const int VELOCITY = 5;

//The dimensions of the dot
static const int DOT_WIDTH = 20;
static const int DOT_HEIGHT = 20;

int camX = 0;
int vertVelocity = 0;
bool flyFlag = 0;
double pikaV = 0;
double pikaAcceleration = 0;
int pikaInfo[6] = { 9, SCREEN_WIDTH /2-80, 380, 2, 0, 0 }; //image index, posX, posY, 0Right/1Left/2Stop, jumpFlag, totalKeyDownNumber(left+right)

//Wall test
SDL_Rect wall = {400,327,100,40};
//mCollider
SDL_Rect mCollider = {0,0, DOT_WIDTH ,DOT_HEIGHT };

struct ImageData
{
	char path[100];
	SDL_Texture* texture;
	int width;
	int height;
};


int initSDL(); // Starts up SDL and creates window
void closeSDL(); // Frees media and shuts down SDL
ImageData loadTexture(char* path, bool ckEnable, Uint8 r, Uint8 g, Uint8 b);
void imgRender(SDL_Renderer* renderer, ImageData img, int posX, int posY);
void handleEvent(SDL_Event& e); // Takes key presses and adjusts the dot's velocity
bool checkCollision(SDL_Rect a, SDL_Rect b);
Uint32 pikaAction(Uint32 interval, void* param);


SDL_Window* window = NULL; // The window we'll be rendering to
SDL_Renderer* renderer = NULL; // The window renderer



int initSDL()
{
	// Initialize SDL	
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
	{
		// Error Handling		
		printf("SDL_Init failed: %s\n", SDL_GetError());
		return 1;
	}

	// Create window	
	// SDL_WINDOWPOS_UNDEFINED: Used to indicate that you don't care what the window position is.
	window = SDL_CreateWindow("OOP SDL Tutorial", 50, 50, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (window == NULL)
	{
		printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
		SDL_Quit();
		return 2;
	}

	// Initialize PNG loading	
	int imgFlags = IMG_INIT_PNG;
	if (!(IMG_Init(imgFlags) & imgFlags))
	{
		printf("SDL_image failed: %s\n", IMG_GetError());
		return 3;
	}

	// Create renderer	
	// VSync allows the rendering to update at the same time as when your monitor updates during vertical refresh.
	// For this tutorial it will make sure the animation doesn't run too fast. 
	// Most monitors run at about 60 frames per second and that's the assumption we're making here. 
	// If you have a different monitor refresh rate, that would explain why the animation is running too fast or slow.
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (renderer == NULL)
	{
		SDL_DestroyWindow(window);
		printf("SDL_CreateRenderer failed: %s\n", SDL_GetError());
		SDL_Quit();
		return 3;
	}

	return 0;
}


void closeSDL()
{
	// Destroy renderer	
	// Destroy window	
	// Quit Image subsystem
	// Quit SDL subsystems
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	IMG_Quit();
	SDL_Quit();
}

void handleEvent(SDL_Event& e)
{
	/* The method for "Debunce" */

	// If a key was pressed
	// repeat: non-zero if this is a key repeat
	// https://wiki.libsdl.org/SDL_KeyboardEvent
	//if (e.type == SDL_KEYDOWN)
	if (e.type == SDL_KEYDOWN && e.key.repeat == 0)
	{
		//Adjust the velocity
		switch (e.key.keysym.sym)
		{
			//case SDLK_UP:  break;
			//case SDLK_DOWN:  break;
			case SDLK_UP:
				pikaInfo[4] = 1;
				break;
			case SDLK_LEFT: 
				pikaInfo[3] = 1;
				pikaInfo[5] += 1;
				//pikaV -= VELOCITY; 
				pikaAcceleration = -0.2; 
				break;
			case SDLK_RIGHT: 
				pikaInfo[3] = 0;
				pikaInfo[5] += 1;
				//pikaV += VELOCITY; 
				pikaAcceleration = 0.2;
				break;
		}
	}
	//If a key was released
	//else if (e.type == SDL_KEYUP)
	else if (e.type == SDL_KEYUP && e.key.repeat == 0)
	{
		//Adjust the velocity
		switch (e.key.keysym.sym)
		{
			case SDLK_UP:  
				break;
			//case SDLK_DOWN:  break;
			case SDLK_LEFT: 
				pikaInfo[5] -= 1;
				if (pikaInfo[5] == 0) {
					pikaInfo[3] = 3;
					pikaAcceleration = 0.15;
				}
				else {
					pikaInfo[3] = 0;
					pikaAcceleration = 0.2;
				}
				//pikaV += VELOCITY; 
				break;

			case SDLK_RIGHT: 
				pikaInfo[5] -= 1;
				if (pikaInfo[5] == 0) {
					pikaInfo[3] = 2;
					pikaAcceleration = -0.15;
				}
				else {
					pikaInfo[3] = 1;
					pikaAcceleration = -0.2;
				}
				//pikaV -= VELOCITY; 
				break;
		}
	}
}

ImageData loadTexture(char* path, bool ckEnable, Uint8 r, Uint8 g, Uint8 b)
{
	ImageData img;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path);
	if (loadedSurface == NULL)
	{
		printf("IMG_Load failed: %s\n", IMG_GetError());		
	}
	else
	{
		// Set the color key (transparent pixel) in a surface.		
		// The color key defines a pixel value that will be treated as transparent in a blit. 
		// It is a pixel of the format used by the surface, as generated by SDL_MapRGB().
		// Use SDL_MapRGB() to map an RGB triple to an opaque pixel value for a given pixel format.		
		SDL_SetColorKey(loadedSurface, ckEnable, SDL_MapRGB(loadedSurface->format, r, g, b));

		// Create texture from surface pixels
		img.texture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
		if (img.texture == NULL)
		{
			printf("SDL_CreateTextureFromSurface failed: %s\n", SDL_GetError());
		}

		//Get image dimensions

		if (!strncmp(path, "../images/pikachu/", 18)) {
			img.width = loadedSurface->w/15 ;
			img.height = loadedSurface->h/15 ;
		}
		else {
			img.width = loadedSurface->w;
			img.height = loadedSurface->h;
		}
		

		// Get rid of old loaded surface
		SDL_FreeSurface(loadedSurface);
	}

	//return newTexture;
	return img;
}


void imgRender(SDL_Renderer* renderer, ImageData img, int posX, int posY)
{
	SDL_Rect r;
	r.x = posX;
	r.y = posY;
	r.w = img.width;
	r.h = img.height;
	SDL_RenderCopy(renderer, img.texture, NULL, &r);
}


Uint32 pikaAction(Uint32 interval, void* param)
{
	int* par = (int*)param;

	if (par[4]) {
		if (!flyFlag) {
			vertVelocity = 20;
			flyFlag = 1;
		}
		else if (vertVelocity != -20)
			vertVelocity -= 4;
		else {
			par[4] = 0;
			vertVelocity = 0;
			flyFlag = 0;
		}
			
	}

	par[2] -= vertVelocity;
	mCollider.y = par[2];
	


	switch (par[3]) {
		case 0:      //going right
			par[0] = (par[0] + 1) % (PIKAFRAME / 2) + 8;  // image index
			
			break;
		case 1:      //going left
			par[0] = (par[0] + 1) % (PIKAFRAME / 2);  // image index
			
			break;
		case 2:      //stop facing right
			if (flyFlag) par[0] = (par[0] + 1) % (PIKAFRAME / 2) + 8;   //flying
			else par[0] = 13;       // image index (position stay unchange)
			break;
		case 3:      //stop facing left
			if (flyFlag) par[0] = (par[0] + 1) % (PIKAFRAME / 2);       //flying
			else par[0] = 5;        // image index (position stay unchange)
			break;
	}
	
	return interval;
}

void move(int par[]) {

	if (par[5]) {
		if (pikaAcceleration>=0.1) {
			if (pikaV >= 5) {
				pikaV = 5;
			}
			else {
				pikaV += pikaAcceleration;
				printf("%lf\n", pikaV);
			}
		}
		else if(pikaAcceleration<=-0.1){
			if (pikaV <= -5) {
				pikaV = -5;
			}
			else {
				pikaV += pikaAcceleration;
				printf("%lf\n", pikaV);
			}
		}
	}
	else if (pikaV>=0.1 || pikaV<=-0.1) {
		pikaV += pikaAcceleration;
		printf("%lf\n", pikaV);
	}
	else {
		pikaV = 0;
		pikaAcceleration = 0;
	}
	

	//camX += pikaV;

	par[1] += pikaV;

	if(par[1] >= LEVEL_WIDTH-50 || par[1] <= 0)
		par[1] -= pikaV;

	mCollider.x = par[1];
	
}

bool checkCollision(SDL_Rect a, SDL_Rect b)
{
	//The sides of the rectangles
	int leftA, leftB;
	int rightA, rightB;
	int topA, topB;
	int bottomA, bottomB;

	//Calculate the sides of rect A
	leftA = a.x;
	rightA = a.x + a.w;
	topA = a.y;
	bottomA = a.y + a.h;

	//Calculate the sides of rect B
	leftB = b.x;
	rightB = b.x + b.w;
	topB = b.y;
	bottomB = b.y + b.h;

	//If any of the sides from A are outside of B
	if (bottomA <= topB)
		return false;

	if (topA >= bottomB)
		return false;

	if (rightA <= leftB)
		return false;

	if (leftA >= rightB)
		return false;
	

	//If none of the sides from A are outside B
	return true;
}
// When using SDL, you have to use "int main(int argc, char* args[])"
// int main() and void main() are not allowed
int main(int argc, char* args[])
{
	// yang
	SDL_Rect camRect = { 0,0,640,480 };
	mCollider.x = pikaInfo[1];
	mCollider.y = pikaInfo[2];
	// The working directory is where the vcxproj file is located.
	
	char pikaPath[100] = "../images/pikachu/";

	ImageData pika[PIKAFRAME];


	// Start up SDL and create window
	if (initSDL())
	{
		printf("Failed to initialize SDL!\n");
		return -1;
	}

	for (int i = 0; i < PIKAFRAME; i++)
	{
		char str[100];
		sprintf_s(str, 100, "%s%02d.jpg", pikaPath, i + 1);
		pika[i] = loadTexture(str, true, 0xFF, 0xFF, 0xFF);
	}

	SDL_TimerID timerID_pika = SDL_AddTimer(60, pikaAction, pikaInfo);

	//Main loop flag
	bool quit = false;

	//Event handler
	SDL_Event e;
	SDL_Surface* imgSurface = IMG_Load("../images/124.bmp");

	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, imgSurface);

	SDL_FreeSurface(imgSurface);
	//While application is running
	while (!quit)
	{
		//Handle events on queue
		while (SDL_PollEvent(&e) != 0)
		{
			//User requests quit
			if (e.type == SDL_QUIT)
			{
				quit = true;
			}

			// Handle input for pikachu
			handleEvent(e);
		}

		//yang start3
		//camRect.x = pikaInfo[1] - 220;
		//camRect.y = mPosY - 240;
		move(pikaInfo);
		camRect.x = (pikaInfo[1]+ DOT_WIDTH/2-SCREEN_WIDTH/2);
		camRect.y = (pikaInfo[2] + DOT_HEIGHT / 2 - SCREEN_HEIGHT / 2);
		if (camRect.x < 0)
			camRect.x = 0;
		if (camRect.x > LEVEL_WIDTH - camRect.w)
			camRect.x = LEVEL_WIDTH - camRect.w;
		if (camRect.y < 0)
			camRect.y = 0;
		if (camRect.y > LEVEL_HEIGHT - camRect.h)
			camRect.y = LEVEL_HEIGHT - camRect.h;

		
		//yang end3

		// Clear screen
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
		SDL_RenderClear(renderer);
	
		


		//yang
		SDL_RenderCopy(renderer, texture, &camRect, NULL);

		imgRender(renderer, pika[pikaInfo[0]], pikaInfo[1]- camRect.x, pikaInfo[2]- camRect.y);
		
		// Update screen
		SDL_RenderPresent(renderer);
	}

	// Free loaded image
	
	for (int i = 0; i < PIKAFRAME; i++)
	{
		SDL_DestroyTexture(pika[i].texture);
	}

	//Free resources and close SDL
	closeSDL();

	return 0;
}