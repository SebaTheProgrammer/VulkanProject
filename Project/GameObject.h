#pragma once
#include <string>
#include <vector>
#include "Model.h"
#include <memory>
#include <glm/gtc/matrix_transform.hpp>

struct TransformComponent
{
	glm::vec3 translation{};
	glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
	glm::vec3 rotation{};

	//Tait-Bryan angles with axis order y, x, z
    //https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
    glm::mat4 mat4() 
    {
        const float c3 = glm::cos( rotation.z );
        const float s3 = glm::sin( rotation.z );
        const float c2 = glm::cos( rotation.x );
        const float s2 = glm::sin( rotation.x );
        const float c1 = glm::cos( rotation.y );
        const float s1 = glm::sin( rotation.y );
        return glm::mat4
        {
            {
                scale.x * ( c1 * c3 + s1 * s2 * s3 ),
                scale.x * ( c2 * s3 ),
                scale.x * ( c1 * s2 * s3 - c3 * s1 ),
                0.0f,
            },
            {
                scale.y * ( c3 * s1 * s2 - c1 * s3 ),
                scale.y * ( c2 * c3 ),
                scale.y * ( c1 * c3 * s2 + s1 * s3 ),
                0.0f,
            },
            {
                scale.z * ( c2 * s1 ),
                scale.z * ( -s2 ),
                scale.z * ( c1 * c2 ),
                0.0f,
            },
            {translation.x, translation.y, translation.z, 1.0f}
        };
    }

    glm::mat3 normalMatrix()
    {
        const float c3 = glm::cos( rotation.z );
        const float s3 = glm::sin( rotation.z );
        const float c2 = glm::cos( rotation.x );
        const float s2 = glm::sin( rotation.x );
        const float c1 = glm::cos( rotation.y );
        const float s1 = glm::sin( rotation.y );
        const glm::vec3 invScale = 1.0f / scale;

        return glm::mat3{
            {
                invScale.x * ( c1 * c3 + s1 * s2 * s3 ),
                invScale.x * ( c2 * s3 ),
                invScale.x * ( c1 * s2 * s3 - c3 * s1 ),
            },
            {
                invScale.y * ( c3 * s1 * s2 - c1 * s3 ),
                invScale.y * ( c2 * c3 ),
                invScale.y * ( c1 * c3 * s2 + s1 * s3 ),
            },
            {
                invScale.z * ( c2 * s1 ),
                invScale.z * ( -s2 ),
                invScale.z * ( c1 * c2 ),
            },
        };
    }
};

struct PointLightComponent
{
	float intensity{ 1.0f };
};

class GameObject
{
public:
    using id_t = unsigned int;

    static GameObject Create()
    {
        static id_t currentId = 0;
        return GameObject{ currentId++ };
    }

    static GameObject CreateLightPoint(
        float intensity = 10.f, float radius = 1.f,
        glm::vec3 color = glm::vec3{ 1.f } )
    {
        GameObject gameObj = Create();
        gameObj.m_Color = color;
        gameObj.m_Transform.scale.x = radius;
        gameObj.m_PointLight = std::make_unique<PointLightComponent>();
        gameObj.m_PointLight->intensity = intensity;

        return gameObj;
    }

    id_t GetId() const { return m_id; }

    glm::vec3 m_Color{ 1.0f, 1.0f, 1.0f };
    TransformComponent m_Transform{};

    std::shared_ptr<Model> m_Model{};
    std::unique_ptr<PointLightComponent> m_PointLight = nullptr;

    GameObject( const GameObject& other ) = delete;
    GameObject& operator=( const GameObject& other ) = delete;
    GameObject( GameObject&& other ) noexcept = default;
    GameObject& operator=( GameObject&& other ) noexcept = default;
    ~GameObject() = default;

private:
    GameObject( id_t id ) : m_id{ id } {};

    id_t m_id;
};