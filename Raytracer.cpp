#include <iostream>
#include <Windows.h>
#include <vector>
#include <glm/glm.hpp>
#define _USE_MATH_DEFINES
#include <math.h>
#include <SDL.h>
#undef main
#include "Raytracer/SDLauxiliary.h"

#include "Raytracer/TestModel.h"
#include "Raytracer/Raytracer.h"

//VS14 FIX
FILE _iob[] = { *stdin, *stdout, *stderr };
extern "C" FILE * __cdecl __iob_func(void) {
	return _iob;
}
/*
using namespace std;
using glm::vec3;
using glm::mat3;*/

/* ----------------------------------------------------------------------------*/
/* GLOBAL VARIABLES                                                            */

const int SCREEN_WIDTH = 500;
const int SCREEN_HEIGHT = 500;
SDL_Surface* screen;
int t;

/* ----------------------------------------------------------------------------*/
/* FUNCTIONS                                                                   */

void Update();
void Draw(std::vector<Triangle>& model);
glm::vec3 Trace(float x, float y, std::vector<Triangle>& triangles);

int main(int argc, char** argv) {
	screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT );
	t = SDL_GetTicks();	// Set start value for timer.

	std::vector<Triangle> model = std::vector<Triangle>();
	LoadTestModel(model);

	while( NoQuitMessageSDL() )
	{
		Update();
		Draw(model);
	}

	SDL_SaveBMP( screen, "screenshot.bmp" );
	return 0;
}

void Update() {
	// Compute frame time:
	int t2 = SDL_GetTicks();
	float dt = float(t2-t);
	t = t2;
	std::cout << "Render time: " << dt << " ms." << std::endl;
}

void Draw(std::vector<Triangle>& model) {
	if( SDL_MUSTLOCK(screen) )
		SDL_LockSurface(screen);

	for( int y=0; y<SCREEN_HEIGHT; ++y )
	{
		for( int x=0; x<SCREEN_WIDTH; ++x )
		{
			//glm::vec3 color( 1.0, 0.0, 0.0 );
			glm::vec3 color = Trace(x, y, model);
			PutPixelSDL( screen, x, y, color );
		}
	}

	if( SDL_MUSTLOCK(screen) )
		SDL_UnlockSurface(screen);

	SDL_UpdateRect( screen, 0, 0, 0, 0 );
}

//Returns colour of nearest intersecting triangle
glm::vec3 Trace(float x, float y, std::vector<Triangle>& triangles) {
	float fov = 30;
	float aspect_ratio = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;
	float angle = tan(M_PI * fov / 360);

	float focalLength = -40.0f;
	glm::vec3 cameraPos(0, 0, 0);
	glm::vec3 direction(x-(float)SCREEN_WIDTH/2, y-(float)SCREEN_HEIGHT/2, focalLength);

	Ray ray(cameraPos, direction);
	Intersection closest_intersect;
	if (ray.closestIntersection(triangles, closest_intersect)) {
		return triangles[closest_intersect.index].color;
	}
	return glm::vec3(0.0f, 0.0f, 0.0f);
}