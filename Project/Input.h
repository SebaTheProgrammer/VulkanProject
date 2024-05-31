////////////////////////////////////////////////////
// Tile:	Input.h                               //
// Title:	Gravity and collision detection       //
// Credits: Vryens Sebastiaan 2024                //
// Version: 0.0.1 (Still working on it :)         //
////////////////////////////////////////////////////

//Need to make it a lot more abstract(templates?), but this is a start
//Need to add a lot more functionality

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
    //Your key mappings
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

    //your objects or just parse all your vertices/triangles to m_TransformedTriangles
    MovementController() = default;
    MovementController( GameObject& gameObject ) { m_GameObjects.emplace_back( gameObject ); UpdateTriangles( m_GameObjects ); };
    MovementController( std::vector<GameObject>& gameObjects ) : m_GameObjects( gameObjects )
    {
        UpdateTriangles( gameObjects );
    }
    MovementController( std::vector<std::vector<glm::vec3>> transformedTriangles ) { m_TransformedTriangles = transformedTriangles; };
    MovementController( std::vector<glm::vec3 > transformedTriangles ){ m_TransformedTriangles.emplace_back( transformedTriangles ); };

    //Need to fix this
    void UpdateTriangles( GameObject& gameObject )
	{
        m_GameObjects.clear();
		m_GameObjects.emplace_back( gameObject );
		m_TransformedTriangles.clear();
		for ( auto& gameObject : m_GameObjects )
		{
			if ( gameObject.m_Model != nullptr )
			{
				//your transforms
				TransformComponent& transform = gameObject.m_Transform;
				//your triangles, you can also just parse them in the function
				std::vector<glm::vec3> triangles = gameObject.m_Model->GetModelData().triangles;

				TransformTriangles( transform, triangles );

				m_TransformedTriangles.emplace_back( std::move( triangles ) );
			}
		}
	}
    void UpdateTriangles( std::vector<GameObject>& gameObjects )
    {
        m_GameObjects = gameObjects;
        m_TransformedTriangles.clear();
        for ( auto& gameObject : m_GameObjects )
        {
            if ( gameObject.m_Model != nullptr )
            {
                //your transforms
                TransformComponent& transform = gameObject.m_Transform;
                //your triangles, you can also just parse them in the function
                std::vector<glm::vec3> triangles = gameObject.m_Model->GetModelData().triangles;

                TransformTriangles( transform, triangles );

                m_TransformedTriangles.emplace_back( std::move( triangles ) );
            }
        }
    }
    void UpdateTriangles( std::vector<std::vector<glm::vec3>> transformed )
    {
        m_TransformedTriangles.clear();
        m_TransformedTriangles = transformed;
    }
    void UpdateTriangles( std::vector<glm::vec3> transformed)
	{
        m_TransformedTriangles.clear();
        AddTriangles( transformed );
	}
    void AddTriangles( std::vector<glm::vec3> triangles)
	{
		m_TransformedTriangles.emplace_back( triangles );
	}

    void UpdatePhysics( GLFWwindow* window, float dt, GameObject& gameObject )
    {
        if ( m_CanMoveWithInput ) 
        {
            CheckInputs( window, dt, gameObject );
        }
        if ( m_HasCollisions ) 
        {
            CheckCollisions( dt, gameObject );
        }
    }

    void Jump( GameObject& gameObject, float strength )
    {
        m_IsFlying = false;
        m_IsFalling = true;

        if ( !m_IsJumping )
        {
            m_IsJumping = true;
            m_VelocityY = strength;
        }
    }

    //Setters
    void CanMoveWithInput( bool canMove ) { m_CanMoveWithInput = canMove; };
    void HasCollisions( bool hasCollisions ) { m_HasCollisions = hasCollisions; };

    void SetMoveSpeed( float moveSpeed ) { m_MoveSpeed = moveSpeed; };
    void SetLookSpeed( float lookSpeed ) { m_LookSpeed = lookSpeed; };
    void SetPlayerHeight( float playerHeight ) { m_PlayerHeight = playerHeight; };
    void SetJumpStrength( float jumpStrength ) { m_JumpStrength = jumpStrength; };
	void SetGravity( float gravity ) { m_Gravity = gravity; };
    void SetBounceStrength( float bounceStrength ) { m_BounceStrength = bounceStrength; };

    void Fly( bool isFlying ) { m_IsFlying = isFlying; m_VelocityY = 0; };
    void Fall( bool isFalling ) { m_IsFalling = isFalling; };

    KeyMappings m_keyMappings;

private:
    //FUNCTIONS
    void CheckCollisions( float deltaTime, GameObject& camera )
    {
        if ( m_IsFlying ) { return; }   //If the player is flying, don't do anything

        m_VelocityY += m_Gravity * deltaTime;
        m_CurrentPositionY = camera.m_Transform.translation.y - m_VelocityY * deltaTime;

        if ( m_IsFalling )
        {
            if ( m_CurrentPositionY >= m_StartJumpPosition )
            {
                m_VelocityY = m_BounceStrength * m_JumpStrength;        //Bounce if needed

                if ( m_BounceStrength > 0 )
                {
                    m_BounceStrength -= m_BounceDecay;
                }
                if ( m_VelocityY == 0 )
                {
                    m_CurrentPositionY = m_StartJumpPosition;
                    m_StartJumpPosition = m_BottomWorldBound;
                    m_IsJumping = false;
                    m_IsFalling = false;
                }
            }

            camera.m_Transform.translation.y = m_CurrentPositionY;
        }

        glm::vec3 rayOrigin = camera.m_Transform.translation;
        rayOrigin.y -= m_PlayerHeight;      //+=m_PlayerHeight if you want to start from the top of the player

        bool foundIntersection = false;
        float closestIntersectionDistance = std::numeric_limits<float>::max();

        glm::vec2 intersectionPoint;
        glm::vec3 vertex0;
        glm::vec3 vertex1;
        glm::vec3 vertex2;

        //Check for intersection with the triangles
        for ( auto& triangles : m_TransformedTriangles )
        {
            for ( size_t i = 0; i < triangles.size(); i += 3 )
            {
                vertex0 = triangles[ i ];
                vertex1 = triangles[ i + 1 ];
                vertex2 = triangles[ i + 2 ];

                //Raycast
                if ( glm::intersectRayTriangle( rayOrigin, m_RayDirection, vertex0, vertex1, vertex2, intersectionPoint, closestIntersectionDistance ) )
                {
                    if ( closestIntersectionDistance < m_MaxRayCastDistance )
                    {
                        if ( std::abs( closestIntersectionDistance ) > m_Epsilon )
                        {
                            glm::vec3 intersectionPoint3D = rayOrigin + m_RayDirection * closestIntersectionDistance;
                            if ( intersectionPoint3D.y == 0 ) { return; }
                            if ( intersectionPoint3D.y > 0 ) { intersectionPoint3D *= -1; }         //Setting for my world, you can change this, or remove it

                            //If the triangle is ok:
                            if ( std::abs( intersectionPoint3D.y ) > m_Epsilon )
                            {
                                m_StartJumpPosition = intersectionPoint3D.y - m_PlayerHeight;
                                m_LowestIntersectionY = std::max( m_LowestIntersectionY, m_StartJumpPosition );
                                foundIntersection = true;
                            }
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
    void Shoot( GameObject& camera ) 
    {
        glm::vec3 rayOrigin = camera.m_Transform.translation;

        glm::vec3 forward = camera.m_Transform.rotation;

        float closestIntersectionDistance = std::numeric_limits<float>::max();

        glm::vec2 intersectionPoint;
        glm::vec3 vertex0;
        glm::vec3 vertex1;
        glm::vec3 vertex2;

        for ( auto& triangles : m_TransformedTriangles )
        {
            for ( size_t i = 0; i < triangles.size(); i += 3 )
            {
                vertex0 = triangles[ i ];
                vertex1 = triangles[ i + 1 ];
                vertex2 = triangles[ i + 2 ];

                if ( glm::intersectRayTriangle( rayOrigin, forward, vertex0, vertex1, vertex2, intersectionPoint, closestIntersectionDistance ) )
                {
                    if ( closestIntersectionDistance < m_MaxRayCastDistance )
                    {
                        if ( std::abs( closestIntersectionDistance ) > m_Epsilon )
                        {
                            glm::vec3 intersectionPoint3D = rayOrigin + m_RayDirection * closestIntersectionDistance;
                            if ( intersectionPoint3D.y == 0 ) { return; }
                            if ( intersectionPoint3D.y > 0 ) { intersectionPoint3D *= -1; }         //Setting for my world, you can change this, or remove it

                            if ( std::abs( intersectionPoint3D.y ) > m_Epsilon )
                            {

                            }
                        }
                    }
                }
            }
        }
    }

    void CheckInputs( GLFWwindow* window, float dt, GameObject& gameObject ) 
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

        if ( glfwGetMouseButton( window, GLFW_MOUSE_BUTTON_LEFT ) == GLFW_PRESS && m_CanShoot==true)
        {
            Shoot( gameObject );
		}

        // Movement
        float yaw = gameObject.m_Transform.rotation.y;
        const glm::vec3 forward{ sin( yaw ), 0.f, cos( yaw ) };
        const glm::vec3 right{ forward.z, 0.f, -forward.x };
        const glm::vec3 up{ 0.f, 1.f, 0.f };

        glm::vec3 moveDir{ 0.f };
        if ( glfwGetKey( window, m_keyMappings.moveForward ) == GLFW_PRESS )
        {
            Fall( true );
            moveDir += forward;
        }
        if ( glfwGetKey( window, m_keyMappings.moveBackward ) == GLFW_PRESS )
        {
            Fall( true );
            moveDir -= forward;
        }
        if ( glfwGetKey( window, m_keyMappings.moveRight ) == GLFW_PRESS )
        {
            Fall( true );
            moveDir += right;
        }
        if ( glfwGetKey( window, m_keyMappings.moveLeft ) == GLFW_PRESS )
        {
            Fall( true );
            moveDir -= right;
        }
        if ( glfwGetKey( window, m_keyMappings.moveUp ) == GLFW_PRESS )
        {
            Fly( true );
            moveDir -= up;
        }
        if ( glfwGetKey( window, m_keyMappings.moveDown ) == GLFW_PRESS )
        {
            Fly( true );
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
            m_MoveSpeed = m_BeginMoveSpeed;
        }

        //Update the position
        if ( glm::dot( moveDir, moveDir ) > std::numeric_limits<float>::epsilon() )
        {
            gameObject.m_Transform.translation += glm::normalize( moveDir ) * m_MoveSpeed * dt;
        }
    };
    void TransformTriangles( TransformComponent& transform, std::vector<glm::vec3>& triangles )
    {
        //Transform the triangles to your world space
        glm::vec3 scale = transform.scale;
        glm::vec3 rotation = transform.rotation;
        glm::vec3 translation = transform.translation;
        glm::mat4 scaleMatrix = glm::scale( glm::mat4( 1.0f ), scale );

        glm::mat4 rotationX = glm::rotate( glm::mat4( 1.0f ), rotation.x, glm::vec3( 1, 0, 0 ) );
        glm::mat4 rotationY = glm::rotate( glm::mat4( 1.0f ), rotation.y, glm::vec3( 0, 1, 0 ) );
        glm::mat4 rotationZ = glm::rotate( glm::mat4( 1.0f ), rotation.z, glm::vec3( 0, 0, 1 ) );

        glm::mat4 rotationMatrix = rotationZ * rotationY * rotationX;

        glm::mat4 translationMatrix = glm::translate( glm::mat4( 1.0f ), translation );

        glm::mat4 transformationMatrix = translationMatrix * rotationMatrix * scaleMatrix;

        for ( glm::vec3& vertex : triangles )
        {
            vertex = glm::vec3( transformationMatrix * glm::vec4( vertex, 1.0f ) );
        }
    }
    void SetMouseInitialPosition( GLFWwindow* window )
    {
        glfwGetCursorPos( window, &m_LastX, &m_LastY );
        m_FirstMouse = false;
    }

    //VARIABLES
    bool m_CanMoveWithInput{ true };            //Set this to false if you want to disable the movement
    bool m_HasCollisions{ true };               //Set this to false if you don't want to check for collisions
    const float m_MaxMoveSpeed{ 10.0f };        //You can change this to your needs
    const float m_BeginMoveSpeed{ 3.0f };       //You can change this to your needs
    float m_MoveSpeed{ m_BeginMoveSpeed };		//You can change this to your needs
    float m_JumpStrength{ 3.0f };			    //You can change this to your needs
    float m_LookSpeed{ 0.3f };				    //You can change this to your needs
    float m_PlayerHeight{ 0.4f };			    //You can change this to your needs
    float m_Gravity{ -9.8f };                   //You can change this to your needs
    const float m_BottomWorldBound{ -0.2f };    //The bottom of your world, this is a safety cap
    const float m_MaxRayCastDistance{ 5.f };    //You can change this to your needs
    const float m_Epsilon{ 0.0001f };           //You can change this to your needs, but don't tweak to much
    glm::vec3 m_RayDirection{ 0.0f, -1.0f, 0.0f };
    float m_BounceStrength{ 0.0f };             //You can change this to your needs
    const float m_BounceDecay{ 1.f };           //You can change this to your needs

    bool m_CanShoot{ true };
    bool m_IsFalling{ true };				    //Set this to true to fall, or use the function Jump
    bool m_IsFlying{ false };                   //Set this to true to fly, or use the function Fly
    bool m_IsJumping{ false };

    float m_CurrentPositionY{};
    float m_VelocityY{};
    float m_StartJumpPosition{ 0 };
    float m_LowestIntersectionY{ std::numeric_limits<float>::lowest() };
    float m_RayCastDistance{ m_MaxRayCastDistance };

    //Camera
    double m_LastX{}, m_LastY{};
    bool m_FirstMouse = true;
    float m_Yaw = 0.0f;
    float m_Pitch = 0.0f;

    //Your game objects, transform and triangles
    std::vector<GameObject> m_GameObjects{};
    std::vector<TransformComponent> m_Transforms{};
    std::vector<std::vector<glm::vec3>> m_TransformedTriangles{};

   /* glm::vec3 translation{};
    glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
    glm::vec3 rotation{}*/
};
