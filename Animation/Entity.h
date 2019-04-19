#pragma once
#include "rt3d.h"
#include <glm/glm.hpp>
#include <stack>

class Entity
{

public:
	Entity();
	Entity(glm::vec3 inPosition, glm::vec3 inScale, glm::vec3 inRotation, GLuint inTexture, GLuint inModel, GLuint inVertCount, rt3d::materialStruct inMaterial, bool inCull, bool inIsPlayer)
	{
		position = inPosition;
		scale = inScale;
		rotation = inRotation;
		texture = inTexture;
		model = inModel; // VAO
		// need to initialise the number of verts for the draw call
		vertCount = inVertCount;
		material = inMaterial;
		cull = inCull;
		isPlayer = inIsPlayer;
	}
	void Draw(std::stack<glm::mat4> mvStack, glm::mat4 projection, GLuint shaderProgram, GLfloat playerRotation);
	// also need to be able to update the model position, rotation, scale

	void updatePosition(glm::vec3 inPositionUpdate);
	void updateRotation(GLfloat inRotationUpdate);
	bool getIsPlayer();

protected:
	glm::vec3 position;
	glm::vec3 scale;
	glm::vec3 rotation;
	GLuint texture;
	GLuint model; // VAO, i.e. the mesh identifier (from createmesh)
	// you will also need to record the number of elements in the VAO for drawing (e.g. numVerts)
	GLuint vertCount;
	// XXXXX
	rt3d::materialStruct material;
	bool cull;
	bool isPlayer;

	//virtual void Update(float dt) = 0;
};