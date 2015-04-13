#include "camera.hpp"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <SFML/window.hpp>

float original_translation_speed = 1.0f;
float rotation_speed = 1.0f;
float translation_speed = 1.0f;

Camera::Camera() : eye_pos( glm::vec3( 0.0f, 1.0f, 5.0f ) ),
				   view_dir( glm::vec3( 0.0f, 0.0f, -1.0f ) ),
				   up_dir( glm::vec3( 0.0f, 1.0f, 0.0f ) ),
				   proj_mat( glm::perspective( 45.0f, 1.25f, 0.1f, 1000.0f ) )
{
    std::cout << "implicit" << std::endl;
}

Camera::Camera( float fovy, float aspect, float near, float far )
	: eye_pos( glm::vec3( 0.0f, 0.0f, 0.0f ) ),
	  view_dir( glm::vec3( 0.0f, 0.0f, -1.0f ) ),
	  up_dir( glm::vec3( 0.0f, 1.0f, 0.0f ) ),
	  proj_mat( glm::perspective( fovy, aspect, near, far ) )
{
    std::cout << "explicit" << std::endl;
}

Camera::~Camera()
{
}

void Camera::handleInput( float deltaTime )
{
	// adjust the camera position and orientation to account for movement over deltaTime seconds
	// use sf::Keyboard::isKeyPressed( sf::Keyboard::A ) to check if 'a' is currently pressed, etc
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
    {
        translation_speed = 2*original_translation_speed;
    }
    else
    {
        translation_speed = original_translation_speed;
    }

	if (sf::Keyboard::isKeyPressed( sf::Keyboard::W ))
	{
		eye_pos = eye_pos + (translation_speed*deltaTime*view_dir);
	}
	if (sf::Keyboard::isKeyPressed( sf::Keyboard::A ))
	{
		glm::vec3 move_dir = glm::cross(up_dir, view_dir);
		eye_pos = eye_pos + (translation_speed*deltaTime*move_dir);
	}
	if (sf::Keyboard::isKeyPressed( sf::Keyboard::S ))
	{
		eye_pos = eye_pos - (translation_speed*deltaTime*view_dir);
	}
	if (sf::Keyboard::isKeyPressed( sf::Keyboard::D ))
	{
		glm::vec3 move_dir = glm::cross(view_dir, up_dir);
		eye_pos = eye_pos + (translation_speed*deltaTime*move_dir);
	}
	
    if (sf::Keyboard::isKeyPressed( sf::Keyboard::E ))
    {
        eye_pos = eye_pos + translation_speed*deltaTime*up_dir;
    }
    if (sf::Keyboard::isKeyPressed( sf::Keyboard::Q ))
    {
        eye_pos = eye_pos - translation_speed*deltaTime*up_dir;
    }
    
    if (sf::Keyboard::isKeyPressed( sf::Keyboard::Up ))
    {
        glm::vec3 axis = glm::cross(view_dir, up_dir);
        view_dir = glm::rotate(view_dir, rotation_speed*deltaTime, axis);
        up_dir = glm::rotate(up_dir, rotation_speed*deltaTime, axis);
    }
    if (sf::Keyboard::isKeyPressed( sf::Keyboard::Down ))
    {
        glm::vec3 axis = glm::cross(view_dir, up_dir);
        view_dir = glm::rotate(view_dir, -rotation_speed*deltaTime, axis);
        up_dir = glm::rotate(up_dir, -rotation_speed*deltaTime, axis);
    }
    if (sf::Keyboard::isKeyPressed( sf::Keyboard::Left ))
    {
        view_dir = glm::rotate(view_dir, rotation_speed*deltaTime, glm::vec3(0.0f, 1.0f, 0.0f));
        up_dir = glm::rotate(up_dir, rotation_speed*deltaTime, glm::vec3(0.0f, 1.0f, 0.0f));
    }
    if (sf::Keyboard::isKeyPressed( sf::Keyboard::Right ))
    {
        view_dir = glm::rotate(view_dir, -rotation_speed*deltaTime, glm::vec3(0.0f, 1.0f, 0.0f));
        up_dir = glm::rotate(up_dir, -rotation_speed*deltaTime, glm::vec3(0.0f, 1.0f, 0.0f));
    }
    
    if (sf::Keyboard::isKeyPressed( sf::Keyboard::Space ))
    {
        up_dir = glm::vec3(0.0f, 1.0f, 0.0f);
        view_dir = glm::vec3(0.0f, 0.0f, -1.0f);
    }
    
}

// get a read-only handle to the projection matrix
const glm::mat4& Camera::getProjectionMatrix() const
{
	return proj_mat;
}

glm::mat4 Camera::getViewMatrix() const
{
	// construct and return a view matrix from your position representation
    return glm::lookAt(eye_pos, (eye_pos + view_dir), up_dir);
}
