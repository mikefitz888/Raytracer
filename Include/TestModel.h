#ifndef TEST_MODEL_CORNEL_BOX_H
#define TEST_MODEL_CORNEL_BOX_H

// Defines a simple test model: The Cornel Box

#include <glm/glm.hpp>
#include <vector>
#include "Triangle.h"
#include "ModelLoader.h"

// Loads the Cornell Box. It is scaled to fill the volume:
// -1 <= x <= +1
// -1 <= y <= +1
// -1 <= z <= +1
void LoadTestModel(std::vector<Triangle>& triangles, model::Model& scene) {
	using glm::vec3;

	// Defines colors:
	vec3 red(0.75f, 0.15f, 0.15f);
	vec3 yellow(0.75f, 0.75f, 0.15f);
	vec3 green(0.15f, 0.75f, 0.15f);
	vec3 cyan(0.15f, 0.75f, 0.75f);
	vec3 blue(0.15f, 0.15f, 0.75f);
	vec3 purple(0.75f, 0.15f, 0.75f);
	vec3 white(0.75f, 0.75f, 0.75f);

	scene.addMaterial(red);
	scene.addMaterial(yellow);
	scene.addMaterial(green);
	scene.addMaterial(cyan);
	scene.addMaterial(blue);
	scene.addMaterial(purple);
	scene.addMaterial(white);

	triangles.clear();
	triangles.reserve(5 * 2 * 3);

	// ---------------------------------------------------------------------------
	// Room

	float L = 555;			// Length of Cornell Box side.

	vec3 A(L, 0, 0);
	vec3 B(0, 0, 0);
	vec3 C(L, 0, L);
	vec3 D(0, 0, L);

	vec3 E(L, L, 0);
	vec3 F(0, L, 0);
	vec3 G(L, L, L);
	vec3 H(0, L, L);

	scene.addVertex(A); //0
	scene.addVertex(B); //1
	scene.addVertex(C); //2
	scene.addVertex(D); //3
	scene.addVertex(E); //4
	scene.addVertex(F); //5
	scene.addVertex(G); //6
	scene.addVertex(H); //7

	// Floor:
	triangles.push_back(Triangle(C, B, A, green)); scene.addFace(2, 1, 0, 2);
	triangles.push_back(Triangle(C, D, B, green)); scene.addFace(2, 3, 1, 2);

	// Left wall
	triangles.push_back(Triangle(A, E, C, purple)); scene.addFace(0, 4, 2, 5);
	triangles.push_back(Triangle(C, E, G, purple)); scene.addFace(2, 4, 6, 5);

	// Right wall
	triangles.push_back(Triangle(F, B, D, yellow)); scene.addFace(5, 1, 3, 1);
	triangles.push_back(Triangle(H, F, D, yellow)); scene.addFace(7, 5, 3, 1);

	// Ceiling
	triangles.push_back(Triangle(E, F, G, cyan)); scene.addFace(4, 5, 6, 3);
	triangles.push_back(Triangle(F, H, G, cyan)); scene.addFace(5, 7, 6, 3);

	// Back wall
	triangles.push_back(Triangle(G, D, C, white)); scene.addFace(6, 3, 2, 6);
	triangles.push_back(Triangle(G, H, D, white)); scene.addFace(6, 7, 3, 6);

	// ---------------------------------------------------------------------------
	// Short block

	A = vec3(290, 0, 114);
	B = vec3(130, 0, 65);
	C = vec3(240, 0, 272);
	D = vec3(82, 0, 225);

	E = vec3(290, 165, 114);
	F = vec3(130, 165, 65);
	G = vec3(240, 165, 272);
	H = vec3(82, 165, 225);

	scene.addVertex(A);
	scene.addVertex(B);
	scene.addVertex(C);
	scene.addVertex(D);
	scene.addVertex(E);
	scene.addVertex(F);
	scene.addVertex(G);
	scene.addVertex(H);

	// Front
	triangles.push_back(Triangle(E, B, A, red));
	triangles.push_back(Triangle(E, F, B, red));

	// Front
	triangles.push_back(Triangle(F, D, B, red));
	triangles.push_back(Triangle(F, H, D, red));

	// BACK
	triangles.push_back(Triangle(H, C, D, red));
	triangles.push_back(Triangle(H, G, C, red));

	// LEFT
	triangles.push_back(Triangle(G, E, C, red));
	triangles.push_back(Triangle(E, A, C, red));

	// TOP
	triangles.push_back(Triangle(G, F, E, red));
	triangles.push_back(Triangle(G, H, F, red));

	// ---------------------------------------------------------------------------
	// Tall block

	A = vec3(423, 0, 247);
	B = vec3(265, 0, 296);
	C = vec3(472, 0, 406);
	D = vec3(314, 0, 456);

	E = vec3(423, 330, 247);
	F = vec3(265, 330, 296);
	G = vec3(472, 330, 406);
	H = vec3(314, 330, 456);

	scene.addVertex(A);
	scene.addVertex(B);
	scene.addVertex(C);
	scene.addVertex(D);
	scene.addVertex(E);
	scene.addVertex(F);
	scene.addVertex(G);
	scene.addVertex(H);

	// Front
	triangles.push_back(Triangle(E, B, A, blue));
	triangles.push_back(Triangle(E, F, B, blue));

	// Front
	triangles.push_back(Triangle(F, D, B, blue));
	triangles.push_back(Triangle(F, H, D, blue));

	// BACK
	triangles.push_back(Triangle(H, C, D, blue));
	triangles.push_back(Triangle(H, G, C, blue));

	// LEFT
	triangles.push_back(Triangle(G, E, C, blue));
	triangles.push_back(Triangle(E, A, C, blue));

	// TOP
	triangles.push_back(Triangle(G, F, E, blue));
	triangles.push_back(Triangle(G, H, F, blue));


	// ----------------------------------------------
	// Scale to the volume [-1,1]^3

	/*for (size_t i = 0; i<triangles.size(); ++i) {
		triangles[i].v0 *= 2 / L;
		triangles[i].v1 *= 2 / L;
		triangles[i].v2 *= 2 / L;

		triangles[i].v0 -= vec3(1, 1, 1);
		triangles[i].v1 -= vec3(1, 1, 1);
		triangles[i].v2 -= vec3(1, 1, 1);

		triangles[i].v0.x *= -1;
		triangles[i].v1.x *= -1;
		triangles[i].v2.x *= -1;

		triangles[i].v0.y *= -1;
		triangles[i].v1.y *= -1;
		triangles[i].v2.y *= -1;

		triangles[i].ComputeNormal();
	}*/
}

#endif