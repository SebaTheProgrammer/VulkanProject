#pragma once

#include "GameObject.h"
#include "Window.h"
#include <limits>
#include <iostream> // For debugging

class MovementController
{
public:
    struct KeyMappings
    {
        int moveLeft = GLFW_KEY_A;
        int moveRight = GLFW_KEY_D;
        int moveForward = GLFW_KEY_W;
        int moveBackward = GLFW_KEY_S;
        int moveUp = GLFW_KEY_E;
        int moveDown = GLFW_KEY_Q;
        int lookLeft = GLFW_KEY_LEFT;
        int lookRight = GLFW_KEY_RIGHT;
        int lookUp = GLFW_KEY_UP;
        int lookDown = GLFW_KEY_DOWN;
    };

    void MoveInPlaneXZ( GLFWwindow* window, float dt, GameObject& gameObject )
    {
        // Check if right mouse button is pressed
        if ( glfwGetMouseButton( window, GLFW_MOUSE_BUTTON_RIGHT ) == GLFW_PRESS )
        {
            // Capture mouse movement
            if ( firstMouse )
            {
                glfwGetCursorPos( window, &lastX, &lastY );
                firstMouse = false;
            }

            double xpos, ypos;
            glfwGetCursorPos( window, &xpos, &ypos );

            float xoffset = ( xpos - lastX ) * m_LookSpeed;
            float yoffset = ( lastY - ypos ) * m_LookSpeed; // Reversed since y-coordinates range from bottom to top

            lastX = xpos;
            lastY = ypos;

            yaw += xoffset;
            pitch += yoffset;

            // Constrain the pitch
            if ( pitch > 89.0f )
                pitch = 89.0f;
            if ( pitch < -89.0f )
                pitch = -89.0f;

            gameObject.transform.rotation.x = glm::radians( pitch );
            gameObject.transform.rotation.y = glm::radians( yaw ); 
        }
        else
        {
            firstMouse = true;
        }

        // Movement
        float yaw = gameObject.transform.rotation.y;
        const glm::vec3 forward{ sin( yaw ), 0.f, cos( yaw ) };
        const glm::vec3 right{ forward.z, 0.f, -forward.x };
        const glm::vec3 up{ 0.f, 1.f, 0.f }; // Y-axis positive for upward movement

        glm::vec3 moveDir{ 0.f };
        if ( glfwGetKey( window, m_keyMappings.moveForward ) == GLFW_PRESS )
        {
            moveDir += forward;
        }
        if ( glfwGetKey( window, m_keyMappings.moveBackward ) == GLFW_PRESS )
        {
            moveDir -= forward;
        }
        if ( glfwGetKey( window, m_keyMappings.moveRight ) == GLFW_PRESS )
        {
            moveDir += right;
        }
        if ( glfwGetKey( window, m_keyMappings.moveLeft ) == GLFW_PRESS )
        {
            moveDir -= right;
        }
        if ( glfwGetKey( window, m_keyMappings.moveUp ) == GLFW_PRESS )
        {
            moveDir -= up;
        }
        if ( glfwGetKey( window, m_keyMappings.moveDown ) == GLFW_PRESS )
        {
            moveDir += up;
        }

        if ( glm::dot( moveDir, moveDir ) > std::numeric_limits<float>::epsilon() )
        {
            gameObject.transform.translation += glm::normalize( moveDir ) * m_MoveSpeed * dt;
        }
    }

    void SetMouseInitialPosition( GLFWwindow* window )
    {
        glfwGetCursorPos( window, &lastX, &lastY );
        firstMouse = false;
    }

    KeyMappings m_keyMappings;
    float m_MoveSpeed{ 3.0f };
    float m_LookSpeed{ 0.3f };

private:
    double lastX, lastY;
    bool firstMouse = true;
    float yaw = 0.0f;
    float pitch = 0.0f;
};
