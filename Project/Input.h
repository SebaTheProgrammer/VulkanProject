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
        int jump = GLFW_KEY_SPACE; // Add a key for jump
    };

    MovementController()
    {
    }

    void MoveInPlaneXZ( GLFWwindow* window, float dt, GameObject& gameObject )
    {
        // Check if right mouse button is pressed
        if ( glfwGetMouseButton( window, GLFW_MOUSE_BUTTON_RIGHT ) == GLFW_PRESS )
        {
            // Capture mouse movement
            if ( m_FirstMouse )
            {
                glfwGetCursorPos( window, &m_LastX, &m_LastY );
                m_FirstMouse = false;
            }

            double xpos, ypos;
            glfwGetCursorPos( window, &xpos, &ypos );

            float xoffset = ( xpos - m_LastX ) * m_LookSpeed;
            float yoffset = ( m_LastY - ypos ) * m_LookSpeed; // Reversed since y-coordinates range from bottom to top

            m_LastX = xpos;
            m_LastY = ypos;

            m_Yaw += xoffset;
            m_Pitch += yoffset;

            // Constrain the pitch
            if ( m_Pitch > 89.0f )
                m_Pitch = 89.0f;
            if ( m_Pitch < -89.0f )
                m_Pitch = -89.0f;

            gameObject.m_Transform.rotation.x = glm::radians( m_Pitch );
            gameObject.m_Transform.rotation.y = glm::radians( m_Yaw );
        }
        else
        {
            m_FirstMouse = true;
        }

        // Movement
        float yaw = gameObject.m_Transform.rotation.y;
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
        if ( glfwGetKey( window, m_keyMappings.jump ) == GLFW_PRESS )
        {
            Jump( gameObject );
        }

        if ( glm::dot( moveDir, moveDir ) > std::numeric_limits<float>::epsilon() )
        {
            gameObject.m_Transform.translation += glm::normalize( moveDir ) * m_MoveSpeed * dt;
        }

        // Update vertical position
        Update( dt, gameObject );
    }

    void SetMouseInitialPosition( GLFWwindow* window )
    {
        glfwGetCursorPos( window, &m_LastX, &m_LastY );
        m_FirstMouse = false;
    }

    void Jump( GameObject& gameObject )
    {
        if ( !m_IsJumping )
        {
            m_IsJumping = true;
            m_VelocityY = m_JumpStrength;
            m_StartJumpPosition = gameObject.m_Transform.translation.y;
        }
    }

    void Update( float deltaTime, GameObject& gameObject )
    {
        if ( m_IsJumping )
        {
            m_VelocityY += m_Gravity * deltaTime;
            m_CurrentPositionY = gameObject.m_Transform.translation.y - m_VelocityY * deltaTime;

            if ( m_CurrentPositionY >= m_StartJumpPosition )
            {
                m_CurrentPositionY = m_StartJumpPosition;
                m_IsJumping = false;
                m_VelocityY = 0.0f;
            }

            gameObject.m_Transform.translation.y = m_CurrentPositionY;
        }
    }

    KeyMappings m_keyMappings;
    float m_MoveSpeed{ 3.0f };
    float m_LookSpeed{ 0.3f };

private:
    double m_LastX, m_LastY;
    bool m_FirstMouse = true;
    float m_Yaw = 0.0f;
    float m_Pitch = 0.0f;

    // Jump mechanics
    float m_CurrentPositionY;
    float m_VelocityY;
    const float m_Gravity{ -9.8f };
    const float m_JumpStrength{ 4.0f };
    bool m_IsJumping{ false };

    float m_StartJumpPosition;
};
