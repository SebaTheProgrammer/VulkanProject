#pragma once
#include <cassert>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

struct Camera
{
    Camera() = default;

    Camera( const glm::vec3& _origin, float _fovAngle, GLFWwindow* window ) :
        origin{ _origin },
        fovAngle{ _fovAngle },
        window{ window }
    {
        // Initialize key states
        keyWState = GLFW_RELEASE;
        keySState = GLFW_RELEASE;
        keyAState = GLFW_RELEASE;
        keyDState = GLFW_RELEASE;
        keyQState = GLFW_RELEASE;
        keyEState = GLFW_RELEASE;
        keyShiftState = GLFW_RELEASE;
    }

    glm::vec3 origin{ 0,0,-50.f };
    float fovAngle{ 45.f };
    float fov{ glm::tan( glm::radians( fovAngle ) / 2.f ) };
    float speed{};
    const float normalspeed{ 15 };
    const float boostedspeed{ 30 };
    const float rotspeed{ 0.005f };

    glm::vec3 forward{ glm::vec3( 0, 0, 1 ) };
    glm::vec3 up{ glm::vec3( 0, 1, 0 ) };
    glm::vec3 right{ glm::vec3( 1, 0, 0 ) };

    float totalPitch{};
    float totalYaw{};

    glm::mat4 invViewMatrix{};
    glm::mat4 viewMatrix{};
    glm::mat4 projectionMatrix{};

    const float nearPlane{ 0.1f };
    const float farPlane{ 1000.f };
    float aspectRatio{};

    // GLFW window handle
    GLFWwindow* window;

    int keyWState;
    int keySState;
    int keyAState;
    int keyDState;
    int keyQState;
    int keyEState;
    int keyShiftState;

    void Initialize( float _fovAngle = 90.f, glm::vec3 _origin = { 0.f,0.f,0.f } )
    {
        fovAngle = _fovAngle;
        fov = glm::tan( glm::radians( fovAngle ) / 2.f );

        origin = _origin;
        speed = normalspeed;
    }

    void CalculateViewMatrix()
    {
        glm::mat4 translationMatrix = glm::translate( glm::mat4( 1.0f ), origin );
        glm::mat4 rotationMatrix = glm::rotate( glm::mat4( 1.0f ), totalPitch, up ) * glm::rotate( glm::mat4( 1.0f ), totalYaw, glm::vec3( 0, 1, 0 ) );

        forward = glm::vec3( rotationMatrix * glm::vec4( 0, 0, 1, 0 ) );
        right = glm::normalize( glm::cross( up, forward ) );
        up = glm::normalize( glm::cross( forward, right ) );

        invViewMatrix = rotationMatrix * translationMatrix;
        viewMatrix = glm::inverse( invViewMatrix );
    }

    void CalculateProjectionMatrix()
    {
        int width, height;
        glfwGetFramebufferSize( window, &width, &height );
        aspectRatio = static_cast< float >( width ) / height;

        projectionMatrix = glm::perspective( glm::radians( fovAngle ), aspectRatio, nearPlane, farPlane );
    }

    void Update( float deltaTime )
    {
        // Update key states
        keyWState = glfwGetKey( window, GLFW_KEY_W );
        keySState = glfwGetKey( window, GLFW_KEY_S );
        keyAState = glfwGetKey( window, GLFW_KEY_A );
        keyDState = glfwGetKey( window, GLFW_KEY_D );
        keyQState = glfwGetKey( window, GLFW_KEY_Q );
        keyEState = glfwGetKey( window, GLFW_KEY_E );
        keyShiftState = glfwGetKey( window, GLFW_KEY_LEFT_SHIFT );

        // Check the state of each key
        if ( keyWState == GLFW_PRESS )
        {
            //I have a working 3D camera, that's why there is a breakpoint here so you can see the camera 'moving'
            origin += forward * speed * deltaTime;
        }
        if ( keySState == GLFW_PRESS )
        {
            origin -= forward * speed * deltaTime;
        }
        if ( keyAState == GLFW_PRESS )
        {
            origin -= right * speed * deltaTime;
        }
        if ( keyDState == GLFW_PRESS )
        {
            origin += right * speed * deltaTime;
        }
        if ( keyQState == GLFW_PRESS )
        {
            origin -= up * deltaTime * speed;
        }
        if ( keyEState == GLFW_PRESS )
        {
            origin += up * deltaTime * speed;
        }
        if ( keyShiftState == GLFW_PRESS )
        {
            speed = boostedspeed;
        }
        else
        {
            speed = normalspeed;
        }

        CalculateViewMatrix();
        CalculateProjectionMatrix();
    }

    const glm::mat4& GetViewMatrix()
    {
        return viewMatrix;
    }
    const glm::mat4& GetProjectionMatrix()
    {
        return projectionMatrix;
    }
};
