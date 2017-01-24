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
	float fov = 80;
	float aspect_ratio = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;




	float focalLength = 1.0f;
	glm::vec3 cameraPos(0, 0, -2.0);
	//glm::vec3 direction(x-(float)SCREEN_WIDTH/2, y-(float)SCREEN_HEIGHT/2, focalLength);


	/*
		- Loosely based off of https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-generating-camera-rays/generating-camera-rays
		- Same principle as Pinhole, except re-scaled such that the xScr, yScr are in camera-space.
	
	*/
	float fovFactor = tan(M_PI * (fov*0.5) / 180);
	float xScr = (2 * (x-SCREEN_WIDTH/2) / (float)(SCREEN_WIDTH)) * aspect_ratio * fovFactor;
	float yScr = (2 * (y-SCREEN_HEIGHT/2) / (float)(SCREEN_HEIGHT)) * fovFactor;
	glm::vec3 direction(xScr, yScr, focalLength);
	direction = glm::normalize(direction);

	Ray ray(cameraPos, direction);
	Intersection closest_intersect;
	if (ray.closestIntersection(triangles, closest_intersect)) {


		// SIMPLE LIGHTING
		const glm::vec3 light_position(0.0, -0.75, 0.0);
		glm::vec3 baseColour = triangles[closest_intersect.index].color;
		glm::vec3 dir_to_light = light_position - closest_intersect.position;
		float light_distance = glm::length(dir_to_light);
		float light_factor = 1.0-glm::clamp((light_distance / 2.5), 0.0, 1.0);
		light_factor = glm::clamp((double)light_factor, 0.25, 1.0)*2.5;

		// Send a ray between the point on the surface and the light. (The *0.01 is because we need to step a little bit off the surface to avoid self-intersection)
		Ray lightRay(closest_intersect.position+dir_to_light*0.01f, glm::normalize(dir_to_light));
		Intersection closest_intersect2;

		// If the ray intersects with something, and the distance to the intersecting object is closer than 
		if (lightRay.closestIntersection(triangles, closest_intersect2)) {
			if (closest_intersect2.distance < light_distance) {
				light_factor = 0.2;
			}
		}



		return baseColour*light_factor;
	}
	return glm::vec3(0.0f, 0.0f, 0.0f);
}