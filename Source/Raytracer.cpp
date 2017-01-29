#include <iostream>
#include <Windows.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#define _USE_MATH_DEFINES
#include <math.h>
#include <SDL.h>
#undef main
#include "../Include/SDLauxiliary.h"

#include "../Include/TestModel.h"
#include "../Include/Raytracer.h"
#include "../Include/ModelLoader.h"

#define _DOF_ENABLE_ false

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
glm::vec3 Trace(float x, float y, std::vector<Triangle>& triangles, glm::vec3 cameraPos, glm::vec3 direction);

int main(int argc, char** argv) {
	/*std::cout << "wooowowowo" << std::endl;
	model::Model model("../model.txt");
	int x;
	std::cin >> x;
	return 0;*/
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

	float DOF_focus_length = 2.750f;

	glm::vec3 color;
	glm::vec3 cameraPos(0, 0, -2.0);
	float fov = 80;
	float aspect_ratio = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;
	float focalLength = 1.0f;
	/*
	- Loosely based off of https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-generating-camera-rays/generating-camera-rays
	- Same principle as Pinhole, except re-scaled such that the xScr, yScr are in camera-space.
	*/
	float fovFactor = tan(M_PI * (fov*0.5) / 180);

	float x_rotation = 0.5f;
	float y_rotation = 0.0f;

	for( int y=0; y<SCREEN_HEIGHT; ++y )
	{
		for( int x=0; x<SCREEN_WIDTH; ++x )
		{
			float xScr = (2 * (x - SCREEN_WIDTH / 2) / (float)(SCREEN_WIDTH)) * aspect_ratio * fovFactor;
			float yScr = (2 * (y - SCREEN_HEIGHT / 2) / (float)(SCREEN_HEIGHT)) * fovFactor;
			glm::vec3 direction(xScr, yScr, focalLength);
			direction = glm::normalize(direction);
			//glm::vec3 color( 1.0, 0.0, 0.0 );
			if (_DOF_ENABLE_) {
				glm::vec3 color_buffer(0.0f, 0.0f, 0.0f);
				//Take multiple samples with camera rotated about focus point for DOF. TODO: modify Trace() to accept camera transformations (as mat4)
				int samples = 0;
				for (float xr = -0.02f; xr <= 0.02f; xr += 0.01f) {
					for (float yr = -0.02f; yr <= 0.02f; yr += 0.01f) {
						samples++;
						glm::vec3 focus_point = cameraPos + glm::normalize(glm::vec3(0, 0, focalLength)) * glm::vec3(DOF_focus_length);
						glm::vec3 cameraClone(cameraPos);
						cameraClone -= focus_point; //translate such that focus_point is origin
						cameraClone = glm::rotateX(cameraClone, xr); //rotation in X
						cameraClone = glm::rotateY(cameraClone, yr); //rotation in Y
						//For direction vector: give same rotation as camera
						direction = glm::rotateX(direction, xr);
						direction = glm::rotateY(direction, yr);

						cameraClone += focus_point; //undo translation, effect is camera has rotated about focus_point
						color_buffer += Trace(xScr, yScr, model, cameraClone, direction);
					}
				}
				
				color = color_buffer / glm::vec3((float)samples);
			}
			else {
				color = Trace(x, y, model, cameraPos, direction);
			}
			PutPixelSDL( screen, x, y, color );
		}
		printf("Progress: %f \n", ((float)y / (float)SCREEN_HEIGHT)*100.0f);
	}

	if( SDL_MUSTLOCK(screen) )
		SDL_UnlockSurface(screen);

	SDL_UpdateRect( screen, 0, 0, 0, 0 );
}

//Returns colour of nearest intersecting triangle
glm::vec3 Trace(float xScr, float yScr, std::vector<Triangle>& triangles, glm::vec3 cameraPos, glm::vec3 direction) {
	Intersection closest_intersect;
	glm::vec3 color_buffer = glm::vec3(0.0f, 0.0f, 0.0f);

	Ray ray(cameraPos, direction);
	if (ray.closestIntersection(triangles, closest_intersect)) {

		// SIMPLE LIGHTING
		const glm::vec3 light_position(0.0, -0.75, 0.0);
		glm::vec3 baseColour = triangles[closest_intersect.index].color;
		glm::vec3 dir_to_light = light_position - closest_intersect.position;
		float light_distance = glm::length(dir_to_light);
		float light_factor = 1.0 - glm::clamp((light_distance / 2.5), 0.0, 1.0);
		light_factor = glm::clamp((double)light_factor, 0.25, 1.0)*2.5;

		// Send a ray between the point on the surface and the light. (The *0.01 is because we need to step a little bit off the surface to avoid self-intersection)
		Ray lightRay(closest_intersect.position + dir_to_light*0.01f, glm::normalize(dir_to_light));
		Intersection closest_intersect2;

		// If the ray intersects with something, and the distance to the intersecting object is closer than 
		if (lightRay.closestIntersection(triangles, closest_intersect2)) {
			if (closest_intersect2.distance < light_distance) {
				light_factor = 0.2;
			}
		}

		// Calculate interpolated colour:
		/*
			TEMP (explanation of barycentric coordinates):
				- Barycentric coordinates are an alternative way to represent a triangle as the combination of different weights.
				How it boils down is that a coordinate on the surface of a triangle plane can be given a 3D barycentric coordinate (a, b, c). 

				The position of that coordinate can then be converted to euclidean space by taking  pos = v0*a + v1*b + v2*c
				- Conveniently, this allows us to use it to interpolate values. We can give each vertex a colour, and interpolate 
				between them, we can also give each vertex a uv coordinate (mapping to texture space) and use this same
				math to map each point within a triangle to the correct point in uv-space.
		*/
		glm::vec3 barycentric_coords = triangles[closest_intersect.index].calculateBarycentricCoordinates(closest_intersect.position);
		
		baseColour = glm::vec3(1.0, 0.0, 0.0)*barycentric_coords.x + glm::vec3(0.0, 1.0, 0.0)*barycentric_coords.y + glm::vec3(0.0, 0.0, 1.0)*barycentric_coords.z;

		
		return baseColour*light_factor;
	}
	return color_buffer;
}