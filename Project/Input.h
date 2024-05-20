#pragma once

#include "GameObject.h"
#include "Window.h"
#include <limits>
#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/intersect.hpp>

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
        int jump = GLFW_KEY_SPACE;
        int fall = GLFW_KEY_F;
        int sprint = GLFW_KEY_LEFT_SHIFT;
    };

    MovementController( std::vector<GameObject>& gameObjects )
    {
        for ( int index = 0; index < gameObjects.size(); ++index )
        {
            if ( gameObjects[ index ].m_Model != nullptr )
            {
                std::vector<glm::vec3> triangles = gameObjects[ index ].m_Model->GetModelData().GetTriangles();
                glm::vec3 translation = gameObjects[ index ].m_Transform.translation;
                glm::vec3 rotation = gameObjects[ index ].m_Transform.rotation;
                glm::vec3 scale = gameObjects[ index ].m_Transform.scale;

                TransformTriangles( triangles, translation, rotation, scale );
            }
        }
        m_GameObjects=gameObjects;
    }

    void TransformTriangles( std::vector<glm::vec3>& triangles, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale )
    {
        TransformComponent transform{ translation, scale, rotation };

        glm::mat4 transformationMatrix = transform.mat4();

        for ( glm::vec3& vertex : triangles )
        {
            vertex = glm::vec3( transformationMatrix * glm::vec4( vertex, 1.0f ) );
        }
    }

    void MoveInPlaneXZ( GLFWwindow* window, float dt, GameObject& gameObject )
    {
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
            float yoffset = ( m_LastY - ypos ) * m_LookSpeed;

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
        const glm::vec3 up{ 0.f, 1.f, 0.f };

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
            m_IsFlying = true;
            m_VelocityY=0.0f;
            moveDir -= up;
        }
        if ( glfwGetKey( window, m_keyMappings.moveDown ) == GLFW_PRESS )
        {
            m_IsFlying = true;
            m_VelocityY=0.0f;
            moveDir += up;
        }
        if ( glfwGetKey( window, m_keyMappings.jump ) == GLFW_PRESS )
        {
            Jump( gameObject, m_JumpStrength );
        }
        if ( glfwGetKey( window, m_keyMappings.fall ) == GLFW_PRESS )
        {
            Jump( gameObject, 0.0f );
        }
        if ( glfwGetKey( window, m_keyMappings.sprint ) == GLFW_PRESS )
        {
            m_MoveSpeed = m_MaxMoveSpeed;
        }
        else 
        {
            m_MoveSpeed = 3.0f;
        }

        if ( glm::dot( moveDir, moveDir ) > std::numeric_limits<float>::epsilon() )
        {
            gameObject.m_Transform.translation += glm::normalize( moveDir ) * m_MoveSpeed * dt;
        }

        Update( dt, gameObject );
    }

    void SetMouseInitialPosition( GLFWwindow* window )
    {
        glfwGetCursorPos( window, &m_LastX, &m_LastY );
        m_FirstMouse = false;
    }

    void Jump( GameObject& gameObject, float strength )
    {
        m_IsFlying = false;
        m_IsFalling = true;

        if ( !m_IsJumping ) 
        {
            m_IsJumping=true;
            m_VelocityY = strength;
        }
    }

    void Update( float deltaTime, GameObject& camera )
    {
        if ( m_IsFlying ) { return; }

        m_VelocityY += m_Gravity * deltaTime;
        m_CurrentPositionY = camera.m_Transform.translation.y - m_VelocityY * deltaTime;

        if ( m_IsFalling )
        {
            if ( m_CurrentPositionY >= m_StartJumpPosition )
            {
                m_CurrentPositionY = m_StartJumpPosition;
                m_StartJumpPosition = 0;
                m_VelocityY = 0.0f;
                m_IsJumping = false;
            }

            camera.m_Transform.translation.y = m_CurrentPositionY;
        }

        // Perform raycast to check for intersection with mesh
        glm::vec3 rayOrigin = camera.m_Transform.translation;
        rayOrigin.y -= m_MaxRayCastDistance / 2;
        glm::vec3 rayDirection = glm::vec3( 0.0f, -1.0f, 0.0f );

        bool foundIntersection = false;
        float closestIntersectionDistance = std::numeric_limits<float>::max();

        for ( int index = 0; index < m_GameObjects.size(); ++index )
        {
            if ( m_GameObjects[ index ].m_Model != nullptr )
            {
                std::vector<glm::vec3> triangles = m_GameObjects[ index ].m_Model->GetModelData().triangles;
                TransformTriangles( triangles, m_GameObjects[ index ].m_Transform.translation, m_GameObjects[ index ].m_Transform.rotation, m_GameObjects[ index ].m_Transform.scale );

                for ( size_t i = 0; i < triangles.size(); i += 3 ) {
                    glm::vec3 vertex0 = triangles[ i ];
                    glm::vec3 vertex1 = triangles[ i + 1 ];
                    glm::vec3 vertex2 = triangles[ i + 2 ];

                    glm::vec2 intersectionPoint;

                    // Check if the ray intersects with the current triangle
                    if ( glm::intersectRayTriangle( rayOrigin, rayDirection, vertex0, vertex1, vertex2, intersectionPoint, m_RayCastDistance ) ) {
                        if ( m_RayCastDistance < closestIntersectionDistance && m_RayCastDistance < m_MaxRayCastDistance ) {
                            closestIntersectionDistance = m_RayCastDistance;
                            m_StartJumpPosition = intersectionPoint.y - m_PlayerHeight;
                            m_LowestIntersectionY = std::max( m_LowestIntersectionY, m_StartJumpPosition );
                            foundIntersection = true;
                        }
                    }
                }
            }
        }

        if ( foundIntersection )
        {
            m_StartJumpPosition = m_LowestIntersectionY;
        }
    }

    KeyMappings m_keyMappings;
    float m_MoveSpeed{ 3.0f };
    float m_LookSpeed{ 0.3f };

private:
    double m_LastX{}, m_LastY{};
    bool m_FirstMouse = true;
    float m_Yaw = 0.0f;
    float m_Pitch = 0.0f;

    // Jump mechanics
    float m_CurrentPositionY{};
    float m_VelocityY{};
    const float m_Gravity{ -9.8f };
    float m_JumpStrength{ 4.0f };
    bool m_IsFalling{ true };
    bool m_IsFlying{ false };
    bool m_IsJumping{ false };

    float m_StartJumpPosition{ 0 };
    float m_LowestIntersectionY{ std::numeric_limits<float>::lowest() };
    const float m_MaxRayCastDistance{ 100.f };
    float m_RayCastDistance{ m_MaxRayCastDistance };

    const float m_PlayerHeight{ 2.0f };
    const float m_MaxMoveSpeed{ 30.0f };

    std::vector<GameObject> m_GameObjects;

};
