#include "Entity.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 
#include <stack>

#define DEG_TO_RADIAN 0.017453293

void Entity::Draw(std::stack<glm::mat4> mvStack, glm::mat4 projection, GLuint shaderProgram, GLfloat playerRotation)
{

	if (cull == true) {
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
	}
	glUseProgram(shaderProgram);
	// pass in the view (camera) and projection (perspective) matrices as two matrices
	// push the view matrix to the stack

	glBindTexture(GL_TEXTURE_2D, texture);
	rt3d::setMaterial(shaderProgram, material);
	mvStack.push(mvStack.top());
	// perform the model transformation (translate, rotate, scale) on the view matrix, which is at the top of the stack
	// the transform data is private to the entity
	mvStack.top() = glm::translate(mvStack.top(), position);
	mvStack.top() = glm::rotate(mvStack.top(), float(rotation.x*DEG_TO_RADIAN), glm::vec3(-1.0f, 0.0f, 0.0f));
	mvStack.top() = glm::rotate(mvStack.top(), float(rotation.y*DEG_TO_RADIAN), glm::vec3(0.0f, -1.0f, 0.0f));
	mvStack.top() = glm::rotate(mvStack.top(), float(rotation.z*DEG_TO_RADIAN), glm::vec3(0.0f, 0.0f, -1.0f));
	//////mvStack.top() = glm::rotate(mvStack.top(), float((playerRotation - 90.0f)*DEG_TO_RADIAN), glm::vec3(0.0f, 0.0f, -1.0f));
	mvStack.top() = glm::scale(mvStack.top(), scale);
	// make sure that the projection matrix has been set (either outside or in this method)
	rt3d::setUniformMatrix4fv(shaderProgram, "projection", glm::value_ptr(projection));
	rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
	rt3d::drawMesh(model, /* the number of verts to draw*/vertCount, GL_TRIANGLES);
	mvStack.pop();
	if (cull == true) {
		glCullFace(GL_BACK);
	}
}


void Entity::updatePosition(glm::vec3 inPositionUpdate)
{
	position = inPositionUpdate;
}

void Entity::updateRotation(GLfloat inRotationUpdate)
{
	rotation.y = inRotationUpdate;
}

bool Entity::getIsPlayer()
{
	return isPlayer;
}
