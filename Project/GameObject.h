#pragma once
#include <string>
#include <vector>
#include "Model.h"
#include <memory>

struct Transform2DComponent
{
	glm::vec2 translation{};
	glm::vec2 scale{ 1.0f, 1.0f };
	float rotation{ 0.0f };

	glm::mat2 mat2() 
	{ 
		const float s = glm::sin( rotation );
		const float c = glm::cos( rotation );

		glm::mat2 rotationMat{
			{ c, -s },
			{ s, c }};

		glm::mat2 scaleMat{
			{ scale.x, 0.f }, 
			{ 0.f, scale.y }};

		return rotationMat*scaleMat;
	};
};

class GameObject
{
public:
	using id_t =unsigned int;

	static GameObject Create()
	{
		static id_t currentId = 0;
		return GameObject{ currentId++ };
	}

	id_t GetId() const { return m_id; }

	std::shared_ptr<Model> model{};
	glm::vec3 color{ 1.0f, 1.0f, 1.0f };
	Transform2DComponent transform2d{};

private:
	GameObject( id_t id ) : m_id{ id } {};

		id_t m_id;
};