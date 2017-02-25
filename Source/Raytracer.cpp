#include <iostream>

#ifdef _WIN32
#include <Windows.h>
#endif

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#define _USE_MATH_DEFINES
#include <math.h>
#undef size_t
#include <SDL.h>
#undef main
#include "../Include/SDLauxiliary.h"

#include "../Include/TestModel.h"
#include "../Include/Raytracer.h"
#include "../Include/ModelLoader.h"
#include "../Include/bitmap_image.hpp"
#include "../Include/PhotonMap.h"
#include "../Include/Renderer.h"

//PARAMETERS
#define PHOTON_GATHER_RANGE 0.1
#define PREVIEW_RENDER true

#define _DOF_ENABLE_ false
#define _AA_ENABLE false
#define _AA_FACTOR 16.0f
#define _TEXTURE_ENABLE_ false
#define _GLOBAL_ILLUMINATION_ENABLE_ false
#define _PHOTON_MAPPING_ENABLE_ false
#define _CAUSTICS_ENABLE_ false
#define _SOFT_SHADOWS false
#define _SOFT_SHADOW_SAMPLES 10


//VS14 FIX
#ifdef _WIN32
FILE _iob[] = { *stdin, *stdout, *stderr };
extern "C" FILE * __cdecl __iob_func(void) {
	return _iob;
}
#endif

/*
using namespace std;
using glm::vec3;
using glm::mat3;*/
using model::Octree;
using model::AABB;

/* ----------------------------------------------------------------------------*/
/* GLOBAL VARIABLES                                                            */

const int SCREEN_WIDTH = 500;
const int SCREEN_HEIGHT = 500;
SDL_Surface* screen;
bitmap_image *texture;
bitmap_image *normal_texture;
Material *default_material, *glass_material, *metal_material;
int t;

/* ----------------------------------------------------------------------------*/
/* FUNCTIONS                                                                   */

void Update();
void Draw(model::Scene& scene, photonmap::PhotonMap& photon_map, photonmap::PhotonMapper& photon_mapper);
glm::vec3 Trace(glm::vec3 cameraPos, glm::vec3 direction, photonmap::PhotonMap& photon_map, model::Scene& scene, photonmap::PhotonMapper& photon_mapper, int depth=0);

int main(int argc, char** argv) {

   /* glm::vec3 vector_a(0.245, 0.454, 0.6568);
    vector_a = glm::normalize(vector_a);
    glm::vec3 vector_b = glm::refract(glm::normalize(vector_a), glm::normalize(-glm::vec3(0.4, 0.5, 0.6)), 0.7f);
    glm::vec3 vector_c = glm::refract(glm::normalize(vector_b), glm::normalize(-glm::vec3(0.4, 0.5, 0.6)), 1.0f/0.7f);

    std::cout << vector_a.x << " " << vector_a.y << " " << vector_a.z << std::endl;
    std::cout << vector_b.x << " " << vector_b.y << " " << vector_b.z << std::endl;
    std::cout << vector_c.x << " " << vector_c.y << " " << vector_c.z << std::endl;

    int a;
    std::cin >> a;
    */
	screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT );

    // Create materials
    default_material = new Material();
    glass_material = new Material();
    glass_material->materialSetTypeRefractive(1.5f);
    //glass_material->setSpecularFactor(0.75f);
    //glass_material->setSpecularPower(4.0f);

    metal_material = new Material();
    metal_material->materialSetTypeReflective(0.75f, 0.50f);


    // Load models
    
	std::vector<Triangle> model = std::vector<Triangle>();
	LoadTestModel(model);
    model::Model cornell_box(model);
    cornell_box.setUseOptimisationStructure(false);
    

	model::Model m("dragon.obj");
    //m.setUseOptimisationStructure(false);
    // Assign the glass material to the model
    for (auto& t : *m.getFaces()) {
        t.setMaterial(glass_material);
        //t.color = glm::vec3(0.78f, 0.90f, 1.0f);
        /*t.n0 = -t.n0;
        t.n1 = -t.n1;
        t.n2 = -t.n2;*/
    }
    m.generateOctree();

	model::Scene scene;
    
    scene.addModel(&cornell_box);
    scene.addModel(&m); // Bench
	
	model::LightSource basic_light(glm::vec3(0.0, -0.85, 0.0), glm::vec3(0, 1.0, 0), 8);
	scene.addLight(basic_light);

    std::cout << "SCENE: " << std::endl;
    for (auto m : scene.models) {
        std::cout << "    MODEL: " << m->getFaces()->size() << " triangles" << std::endl;
        std::cout << "              MIN: " << m->getBoundingBox().min.x << " " << m->getBoundingBox().min.y << " " << m->getBoundingBox().min.z << std::endl;
        std::cout << "              MAX: " << m->getBoundingBox().max.x << " " << m->getBoundingBox().max.y << " " << m->getBoundingBox().max.z << std::endl;
    }

	//model::Model cornell_box = model::Model("Bench_WoodMetal.obj");
	//model::Model cornell_box = model::Model("../model.txt");
	//scene.addModel(&cornell_box);
	//std::vector<Triangle>* model = cornell_box.getFaces();

	//printf("%d\n", model.size());
	//LoadTestModel(model, cornell_box);

	
	
	texture = new bitmap_image("Resources/T1.bmp");
	normal_texture = new bitmap_image("Resources/N1.bmp");

    int t = SDL_GetTicks();
    std::cout << "GENERATING PHOTON MAP: " << std::endl;


#if _PHOTON_MAPPING_ENABLE_ == 1
	photonmap::PhotonMapper photon_mapper(scene, 100000, 10); //Number of photons, number of bounces
	photonmap::PhotonMap photon_map(&photon_mapper);
	//scene.removeFront();
	glm::vec3 campos(0.0, 0.0, -2.0);

    int t2 = SDL_GetTicks();
    float dt = float(t2 - t);
    t = t2;
    std::cout << "Photon map generated in time: " << dt << " ms." << std::endl;

	Uint8* keystate = SDL_GetKeyState(0);
	bool run = true;
	while (/*keystate[SDLK_SPACE]*/run &&  NoQuitMessageSDL()) {

		keystate = SDL_GetKeyState(0);
		if (keystate[SDLK_UP]) {
			// Move camera forward
			campos.z += 0.0025;
		}
		if (keystate[SDLK_DOWN]) {
			// Move camera backward
			campos.z -= 0.0025;
		}
		if (keystate[SDLK_LEFT]) {
			// Move camera to the left
			campos.x -= 0.0025;
		}
		if (keystate[SDLK_RIGHT]) {
			// Move 
			campos.x += 0.0025;
		}
		if (keystate[SDLK_PAGEUP]) {
			// Move camera to the left
			campos.y -= 0.0025;
		}
		if (keystate[SDLK_PAGEDOWN]) {
			// Move 
			campos.y += 0.0025;
		}
		if (keystate[SDLK_SPACE]) {
			run = false;
			continue;
		}

		// Render photons
		if (SDL_MUSTLOCK(screen))
			SDL_LockSurface(screen);
		SDL_FillRect(screen, NULL, 0x000000);

		// TEMP DEBUG RENDER PHOTONS

		//std::vector<photonmap::PhotonInfo>& photoninfo = photon_mapper.getDirectPhotons();
		//std::vector<photonmap::PhotonInfo>& photoninfo = photon_mapper.getIndirectPhotons();
		//std::vector<photonmap::PhotonInfo>& photoninfo = photon_mapper.getShadowPhotons(); SDL_FillRect(screen, NULL, 0xFFFFFF);
        std::vector<photonmap::PhotonInfo>& photoninfo = photon_mapper.getCausticPhotons();

		//printf("\n \nPHOTONS: %d \n", photoninfo.size());
		int count = 0;
		for (auto pi : photoninfo) {
			glm::vec3 pos = pi.position;
			// Project position to 2D:
			float xScr = (pos.x- campos.x) / (pos.z-campos.z);
			float yScr = (pos.y-campos.y) / (pos.z-campos.z);

			// Scale and bias
			xScr += 1;
			yScr += 1;
			xScr *= 0.5;
			yScr *= 0.5;
			xScr *= SCREEN_WIDTH;
			yScr *= SCREEN_HEIGHT;

			// Draw point (only if the point isn't behind the camera, otherwise we get weird wrapping)
			if ((pos.z - campos.z) > 0.0) {
				PutPixelSDL(screen, xScr, yScr, pi.color*100.0f);
			}
			count++;
		}

		if (SDL_MUSTLOCK(screen))
			SDL_UnlockSurface(screen);

		SDL_UpdateRect(screen, 0, 0, 0, 0);
	}
#endif
		//int a;
		//std::cin >> a;


	//while( NoQuitMessageSDL() )
	{
        t = SDL_GetTicks();	// Set start value for timer.
		//Update();
		//Draw(cornell_box.getFaces());
#if _PHOTON_MAPPING_ENABLE_ == 1
		Draw(scene, photon_map, photon_mapper);
#else
        photonmap::PhotonMapper photon_mapper(scene, 0, 0); //Number of photons, number of bounces
        photonmap::PhotonMap photon_map(&photon_mapper);
        Draw(scene, photon_map, photon_mapper);
#endif
        // Compute frame time:
        int t2 = SDL_GetTicks();
        float dt = float(t2 - t);
        t = t2;
        std::cout << "Render time: " << dt << " ms." << std::endl;
	}
	std::cout << "Render Complete\n";
	while (NoQuitMessageSDL());
	delete texture;
	SDL_SaveBMP( screen, "screenshot.bmp" );
	return 0;
}

void Update() {
	// Compute frame time:
	/*int t2 = SDL_GetTicks();
	float dt = float(t2 - t);
	t = t2;
	std::cout << "Render time: " << dt << " ms." << std::endl;*/
}

void Draw(model::Scene& scene, photonmap::PhotonMap& photon_map, photonmap::PhotonMapper& photon_mapper) {
    
    // Perform draw
    if (SDL_MUSTLOCK(screen))
		SDL_LockSurface(screen);

	float DOF_focus_length = 2.750f;

	glm::vec3 color;

	glm::vec3 cameraPos(0.0, 0.0, -2.0);
	glm::vec3 cameraDirection(0.0f, 0.0f, 0.0f);
	//glm::vec3 cameraPos(8.0, -8.0, -10.0);
	//glm::vec3 cameraDirection(-(float)M_PI / 5.0f, -(float)M_PI / 6.0f, 0.0f);

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


	#pragma omp parallel for schedule(dynamic, 8)
	for (int y = 0; y<SCREEN_HEIGHT; ++y) {
		for (int x = 0; x<SCREEN_WIDTH; ++x) {

			float xScr = (2 * (x - SCREEN_WIDTH / 2) / (float)(SCREEN_WIDTH)) * aspect_ratio * fovFactor;
			float yScr = (2 * (y - SCREEN_HEIGHT / 2) / (float)(SCREEN_HEIGHT)) * fovFactor;
			glm::vec3 direction(xScr, yScr, focalLength);
			direction = glm::normalize(direction);

			direction = glm::rotateX(direction, cameraDirection.x);
			direction = glm::rotateY(direction, cameraDirection.y);
			direction = glm::rotateY(direction, cameraDirection.z);
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
						color_buffer += Trace(cameraClone, direction, photon_map, scene, photon_mapper);
					}
				}

				color = color_buffer / glm::vec3((float)samples);
			}
			else {

				if (_AA_ENABLE) {
					// Anti-aliasing:
					glm::vec3 colorAA(0.0, 0.0, 0.0);

					const float _AA_SAMP = 1.0f/(glm::sqrt(_AA_FACTOR)-1.0f);
					for (float xAA = -0.5f / (float)SCREEN_WIDTH; xAA <= 0.5f / (float)SCREEN_WIDTH; xAA += _AA_SAMP / (float)SCREEN_WIDTH) {
						for (float yAA = -0.5f / (float)SCREEN_HEIGHT; yAA <= 0.5f / (float)SCREEN_HEIGHT; yAA += _AA_SAMP / (float)SCREEN_HEIGHT) {

							float xScr = (2 * (x - SCREEN_WIDTH / 2) / (float)(SCREEN_WIDTH)) * aspect_ratio * fovFactor;
							float yScr = (2 * (y - SCREEN_HEIGHT / 2) / (float)(SCREEN_HEIGHT)) * fovFactor;

							glm::vec3 direction(xScr + xAA, yScr + yAA, focalLength);
							direction = glm::normalize(direction);

							direction = glm::rotateX(direction, cameraDirection.x);
							direction = glm::rotateY(direction, cameraDirection.y);
							direction = glm::rotateY(direction, cameraDirection.z);

							colorAA += Trace(cameraPos, direction, photon_map, scene, photon_mapper);
						}
					}


					//color = Trace(xScr, yScr, model, cameraPos, direction);
					color = colorAA / _AA_FACTOR;
				}
				else {
					color = Trace(cameraPos, direction, photon_map, scene, photon_mapper);
				}

			}
			PutPixelSDL(screen, x, y, color);
		}
#if PREVIEW_RENDER == 1
        if (SDL_MUSTLOCK(screen))
            SDL_UnlockSurface(screen);

        SDL_UpdateRect(screen, 0, 0, 0, 0);
        if (SDL_MUSTLOCK(screen))
            SDL_LockSurface(screen);
#endif
		printf("Progress: %f \n", ((float)y / (float)SCREEN_HEIGHT) * 100.0f);
	}


	if (SDL_MUSTLOCK(screen))
		SDL_UnlockSurface(screen);

	SDL_UpdateRect(screen, 0, 0, 0, 0);
}

//Returns colour of nearest intersecting triangle
glm::vec3 Trace(glm::vec3 cameraPos, glm::vec3 direction, photonmap::PhotonMap& photon_map, model::Scene& scene, photonmap::PhotonMapper& photon_mapper, int depth) {
    // Recursion depth limit breaker:
    if (depth > 5) { /*std::cout << "RECURSION DEPTH EXCEEDED" << std::endl;*/ return glm::vec3(0.0f, 0.0f, 0.0f); }


    Intersection closest_intersect;
    glm::vec3 color_buffer = glm::vec3(0.0f, 0.0f, 0.0f);

    Ray ray(cameraPos, direction);
    if (ray.closestIntersection(scene, closest_intersect)) {
        glm::vec3 baseColour = closest_intersect.triangle->color;





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
        Triangle& t = *closest_intersect.triangle;
        glm::vec3 barycentric_coords = t.calculateBarycentricCoordinates(closest_intersect.position);
        glm::vec3 surface_normal = (t.n0*barycentric_coords.x + t.n1*barycentric_coords.y + t.n2*barycentric_coords.z);
       // glm::vec3 surface_normal_spec = ((-t.n0)*barycentric_coords.x + (-t.n1)*barycentric_coords.y + (-t.n2)*barycentric_coords.z);
#if _TEXTURE_ENABLE_ == 1
        glm::vec2 baseColourUV = t.uv0*barycentric_coords.x + t.uv1*barycentric_coords.y + t.uv2*barycentric_coords.z;
        int tw = texture->width();
        int th = texture->height();

        int tx = (int)((float)(tw)*baseColourUV.x);
        int ty = (int)((float)(th)*baseColourUV.y);


        rgb_t colour;
        //std::cout << "TX: " << tx << " TY: " << ty << std::endl;
        if (tx >= 0 && tx < tw && ty >= 0 && ty <= th && _TEXTURE_ENABLE_) {
            texture->get_pixel(tx, ty, colour);

            baseColour.r = (float)colour.red / 255.0f;
            baseColour.g = (float)colour.green / 225.0f;
            baseColour.b = (float)colour.blue / 255.0f;
        }
        baseColour *= t.color;

        // Get normal
        
        glm::vec3 texture_normal;

        // Get normal map data:
        tw = normal_texture->width();
        th = normal_texture->height();

        tx = (int)((float)(tw)*baseColourUV.x);
        ty = (int)((float)(th)*baseColourUV.y);


        rgb_t norm;
        //std::cout << "TX: " << tx << " TY: " << ty << std::endl;
        if (tx >= 0 && tx < tw && ty >= 0 && ty <= th) {
            normal_texture->get_pixel(tx, ty, norm);

            texture_normal.r = (float)norm.red / 255.0f;
            texture_normal.g = (float)norm.green / 225.0f;
            texture_normal.b = (float)norm.blue / 255.0f;
        }

        // Combine normal with texture normal:
        glm::vec3 combined_normal = texture_normal*2.0f - 1.0f;

        //glm::vec3 tangent   = glm::normalize(t.v1 - t.v0);
        glm::vec3 edge1 = t.v1 - t.v0;
        glm::vec3 edge2 = t.v2 - t.v0;

        float du1 = t.uv1.x - t.uv0.x;
        float dv1 = t.uv1.y - t.uv1.y;
        float du2 = t.uv2.x - t.uv0.x;
        float dv2 = t.uv2.y - t.uv0.y;

        glm::vec3 dp2perp = glm::cross(edge2, surface_normal);
        glm::vec3 dp1perp = glm::cross(surface_normal, edge1);
        glm::vec3 tangent = dp2perp * du1 + dp1perp * du2;
        glm::vec3 bitangent = dp2perp * dv1 + dp1perp * dv2;

        tangent = glm::cross(surface_normal, bitangent);
        bitangent = glm::cross(surface_normal, tangent);

        float invmax = glm::inversesqrt(glm::max(glm::dot(tangent, tangent), glm::dot(bitangent, bitangent)));

        glm::mat3x3 TBN = glm::mat3x3(tangent*invmax, bitangent*invmax, surface_normal);
        combined_normal = glm::normalize(TBN*texture_normal);
#endif
        //PHOTON MAPPING RENDERER SECTION
        glm::vec3 photon_radiance(0.0f);
        if (_PHOTON_MAPPING_ENABLE_&&_GLOBAL_ILLUMINATION_ENABLE_) {
            //photon_radiance = photon_map.gatherPhotons(closest_intersect.position, triangles[closest_intersect.index].getNormal());
            //baseColour = photon_map.gatherPhotons(closest_intersect.position, ray.direction);
            std::vector<std::pair<size_t, float>> direct_photons_in_range, shadow_photons_in_range, indirect_photons_in_range;

            //photon_map.getDirectPhotonsRadius(closest_intersect.position, PHOTON_GATHER_RANGE, direct_photons_in_range);
            //photon_map.getShadowPhotonsRadius(closest_intersect.position, PHOTON_GATHER_RANGE, shadow_photons_in_range);
            //photon_map.getIndirectPhotonsRadius(closest_intersect.position, PHOTON_GATHER_RANGE, indirect_photons_in_range);

            /*const glm::vec3 light_pos = scene.light_sources[0]->position;
            const glm::vec3 light_dir = glm::normalize(light_pos - closest_intersect.position);
            const glm::vec3 light_normal = scene.light_sources[0]->direction;
            float light_factor = glm::dot(-light_dir, -ray.direction);*/


            //colorAccumulator
            glm::vec3 total_colour_energy, total_light_energy, total_energy;
            float samples_intensity = 0;
            float samples_colour_bleed = 0;

            /*
                EXPERIMENT:
                    - So I found that creating a weighting function:
                        samples_colour_bleed += normal_factor*glm::length(energy);

                        Gives incredibly smooth colour bleed results, though you loose the effect of lighting

                    - So I sample the energy in two different ways now. The first samples the pure colour contribution of the colour bleed,
                        the second measures the lighting intensity contribution of each point. These are weighted slightly differently and then
                        get combined after.

            */
            for (auto pht : indirect_photons_in_range) {
                size_t id = pht.first;
                glm::vec3 energy = photon_mapper.indirect_photons.photons[id].color;
                float distance = pht.second;

                // Check that sample direction matches surface normal
                glm::vec3 normal = surface_normal;//t.normal;
                glm::vec3 photon_direction = photon_mapper.indirect_photons.photons[id].direction;
                float     normal_factor = 1.0 - glm::dot(photon_direction, /*t.normal*/normal);
                if (normal_factor >= 0) {
                    //float distance_factor = 1.0; // If using sampling method 1), disable this
                    float distance_factor = 1.0f - glm::clamp(distance / PHOTON_GATHER_RANGE, 0.0, 1.0);

                    samples_colour_bleed += normal_factor*glm::length(energy); //<-- gives a very smooth result, but technically not all that correct (example: http://i.imgur.com/g8EIXIJ.png)
                    samples_intensity += normal_factor; // <-- Only weight samples by their contribution. This reduces visual artifacts
                    total_colour_energy += energy*normal_factor*distance_factor;//*(1.0f  - (float)glm::sqrt(distance)/ (float)sqrt(PHOTON_GATHER_RANGE));
                    total_light_energy += glm::vec3(glm::length(energy))*distance_factor;
                }

            }
            // 1) SAMPLE BASED METHOD, DOES NOT REDUCE BRIGHTNESS BASED ON PHOTON DENSITY, but is smooth
            /*total_colour_energy /= samples_colour_bleed;
            total_light_energy /= samples_intensity;
            total_energy = (total_colour_energy+total_light_energy)*0.5f;*/

            // 2) PHOTON DENSITY REDUCTION
            //total_colour_energy /= 200;
            total_colour_energy /= samples_colour_bleed;
            total_light_energy /= 2;
            total_energy = (total_light_energy*total_colour_energy)/*0.5f*/;

            //photon_radiance = closest_intersect.color*total_energy;
            photon_radiance = RENDERER::finalGather(closest_intersect.position, closest_intersect.triangle, scene, photon_map, photon_mapper);
        }

        // SIMPLE LIGHTING
        float light_factor = 0.0;
        float SpecularFactor = 0.0;
        float material_specular_power = 0.0f;
        if (t.hasMaterial()) {
            material_specular_power = t.getMaterial()->getSpecularPower();
        }

        if (_SOFT_SHADOWS) {
            for (int count = 0; count < _SOFT_SHADOW_SAMPLES; count++) {


                const glm::vec3 light_position = glm::linearRand(glm::vec3(-0.35, -0.90f, -0.35), glm::vec3(0.35, -0.98f, 0.35));


                glm::vec3 dir_to_light = light_position - closest_intersect.position;
                float light_distance = glm::length(dir_to_light);
                float light_factor_x = 1.0f - glm::clamp((light_distance / 2.35f), 0.0f, 1.0f);
                light_factor_x = glm::clamp((double)light_factor_x, 0.0, 1.0);
                light_factor_x *= glm::dot(/*t.normal*/surface_normal, glm::normalize(dir_to_light));
                //light_factor_x *= glm::dot(-dir_to_light, glm::vec3(0.0f, 1.0f, 0.0f)); // <-- light points downwards

                // Send a ray between the point on the surface and the light. (The *0.01 is because we need to step a little bit off the surface to avoid self-intersection)
                Ray lightRay(closest_intersect.position + dir_to_light*0.01f, glm::normalize(dir_to_light));
                Intersection closest_intersect2;

                // If the ray intersects with something, and the distance to the intersecting object is closer than 
                if (lightRay.closestIntersection(scene, closest_intersect2)) {
                    if (closest_intersect2.distance < light_distance) {
                        light_factor_x = 0.0;
                    }
                }
                light_factor += light_factor_x;
            }
            light_factor /= _SOFT_SHADOW_SAMPLES;
            light_factor = glm::clamp(light_factor, 0.0f, 1.0f)*1.5f;
        } else {
            //const glm::vec3 light_position(-30.0, -30, 0.0);
            const glm::vec3 light_position(0.0, -0.75, 0.0);
            glm::vec3 dir_to_light = light_position - closest_intersect.position;
            float light_distance = glm::length(dir_to_light);
            light_factor = 1.0f - glm::clamp((light_distance / 2.5f), 0.0f, 1.0f);
            light_factor = glm::clamp((double)light_factor, 0.0, 1.0);
            light_factor *= glm::dot(/*t.normal*/surface_normal, glm::normalize(dir_to_light));

            // Send a ray between the point on the surface and the light. (The *0.01 is because we need to step a little bit off the surface to avoid self-intersection)
            Ray lightRay(closest_intersect.position + dir_to_light*0.01f, glm::normalize(dir_to_light));
            Intersection closest_intersect2;

            // If the ray intersects with something, and the distance to the intersecting object is closer than 
            if (lightRay.closestIntersection(scene, closest_intersect2)) {
                if (closest_intersect2.distance < light_distance) {
                    light_factor = 0.0;
                }
            }
        }


        if (t.hasMaterial()) {
            // Specularity
            const glm::vec3 light_position(0.0, -0.75, 0.0);
            glm::vec3 dir_to_light = light_position - closest_intersect.position;

            glm::vec3 LightReflect = glm::normalize(glm::reflect(-dir_to_light, surface_normal));
            SpecularFactor = glm::dot(ray.direction, LightReflect);
            if (SpecularFactor > 0) {
                SpecularFactor = pow(SpecularFactor, material_specular_power);
            } else {
                SpecularFactor = 0.0;
            }

            SpecularFactor *= t.getMaterial()->getSpecularFactor();
        } else {
            SpecularFactor = 0.0f;
        }
        

        //light_factor = 0.0f;

        // *********************************************************************** //
        // CAUSTICS
        /*
            Both photon mapping and caustics need to be turned on
        */
        glm::vec3 caustic_factor(0.0, 0.0, 0.0);
        if (_PHOTON_MAPPING_ENABLE_&&_CAUSTICS_ENABLE_) {
            // TODO: Pull caustic data from photon map
            std::vector<std::pair<size_t, float>> caustic_photons_in_range;
            photon_map.getCausticPhotonsRadius(closest_intersect.position, PHOTON_GATHER_RANGE/**0.0025*/*0.001, caustic_photons_in_range);

            glm::vec3 total_energy = glm::vec3(0.0);
            for (auto pht : caustic_photons_in_range) {
                size_t id = pht.first;
                glm::vec3 energy = photon_mapper.caustic_photons.photons[id].color;
                float distance = pht.second;

                total_energy += energy;
            }
            total_energy /= 350/*550*/;

            caustic_factor = total_energy;
        }
        //caustic_factor = glm::pow(caustic_factor, glm::vec3(1.75f))*0.75f;
        caustic_factor = glm::clamp(caustic_factor, 0.0f, 0.60f);
        // *********************************************************************** //
        // GET MATERIAL TYPE
        /*
          Depending on material type, we may send out new rays to pull reflection/refraction data
        */
        glm::vec3 light_colour(1.0f, 0.90f, 0.65f);
        glm::vec3 output_colour;

        // Determine default base colour
        glm::vec3 ambient_factor = glm::vec3(0.05f, 0.05f, 0.05f);
        glm::vec3 ambient_colour = glm::vec3(1.0f, 1.0f, 1.0f);
        baseColour = baseColour * (photon_radiance + light_factor*light_colour + ambient_factor*ambient_colour + SpecularFactor) + caustic_factor;

        if (t.hasMaterial()) {
            Material* material = t.getMaterial();
            switch (material->getType()) {

                /*default:
                case MaterialType::NORMAL: {
                    // If normal, just output the base colour and the photon gather properties
                    output_colour = baseColour * (photon_radiance + light_factor*light_colour /*+ SpecularFactor*//*);
                } break;*/

                case MaterialType::REFLECTIVE:
                {
                    // send out a reflection ray, and combine with the current base colour at this position
                    // TODO: alter the sent out ray to create glossiness
                    glm::vec3 refl_dir = glm::reflect(direction, glm::normalize(surface_normal));
                    glm::vec3 glossy_dir = MATH::CosineWeightedHemisphereDirection(surface_normal);

                    refl_dir = glm::mix(refl_dir, glossy_dir, 1.0f - material->getGlossiness());
                    glm::vec3 refl_ray_pos = closest_intersect.position + refl_dir*0.00001f;

                    // Get reflection colour
                    glm::vec3 reflect_colour = Trace(refl_ray_pos, refl_dir, photon_map, scene, photon_mapper, depth + 1);


                    // Combine reflect colour with base colour by factor
                    output_colour = t.color*glm::mix(reflect_colour, baseColour, 1.0f - material->getReflectionFactor()) + SpecularFactor;
                } break;


                case MaterialType::REFRACTIVE:
                {
                    // SEND OUT A RAY INSIDE A MEDIUM
                    float refractive_index_air = 1.00003f;
                    float refractive_index_material = material->getRefractiveIndex();
                    float eta = 1.0;

                    // If entering triangle, eta = refractive_index_air/refractive_index_material, else eta = refractive_index_material/refractive_index_air
                   /* if (glm::dot(glm::normalize(surface_normal), glm::normalize(direction)) < 0.0) {
                        eta = refractive_index_air / refractive_index_material;
                    } else {
                        eta = refractive_index_material / refractive_index_air;
                    }
                    glm::vec3 direction_f = glm::vec3((float)direction.x, (float)direction.y, (float)direction.z);
                    glm::vec3 surface_normal_f = glm::vec3((float)surface_normal.x, (float)surface_normal.y, (float)surface_normal.z);*/
                    
                    //eta = 1.0f;

                    glm::vec3 refr_dir;
                    if (glm::dot(glm::normalize(surface_normal), glm::normalize(direction)) < 0.0) {
                        eta = refractive_index_air / refractive_index_material;
                        refr_dir = glm::normalize(glm::refract(glm::normalize(direction), glm::normalize(surface_normal), eta));
                    }
                    else {
                        eta = refractive_index_material / refractive_index_air;
                        refr_dir = -glm::normalize(glm::refract(glm::normalize(-direction), glm::normalize(surface_normal), eta));
                    }
                   
                 /*   std::cout << "Nml: "<< surface_normal.x << " " << surface_normal.y << " " << surface_normal.z << std::endl;
                    std::cout << direction.x << " " << direction.y << " " << direction.z << std::endl;
                    std::cout << refr_dir.x << " " << refr_dir.y << " " << refr_dir.z << std::endl;*/
                    
                    /*if (direction != refr_dir) {
                        refr_dir = glm::normalize(glm::refract(glm::normalize(direction), glm::normalize(-surface_normal), eta));
                    }*/

                   // char a;
                    //std::cin >> a;

                    //refr_dir = direction+glm::vec3(0.25f);
                    glm::vec3 refr_ray_pos = closest_intersect.position + refr_dir*0.0001f;

                    glm::vec3 refract_colour = Trace(refr_ray_pos, refr_dir, photon_map, scene, photon_mapper, depth + 1);
                    output_colour = t.color*refract_colour + SpecularFactor;
                    //output_colour = Trace(triangles, closest_intersect.position + refr_dir*0.0001f, refr_dir, photon_map, scene, photon_mapper, depth + 1);
                } break;

            }
        } else {
            // If no special material properties
            output_colour = baseColour;
        }


        //return (surface_normal + glm::vec3(1.0)) / 2.0f;
        //return (combined_normal +glm::vec3(1.0))/2.0f;

        return output_colour;
    }
    return color_buffer;
}