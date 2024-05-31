#pragma once
#include <string>
#include <vector>
#include "Model.h"
#include <memory>
#include <glm/gtc/matrix_transform.hpp>
#include "FrameInfo.h"

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

	std::shared_ptr<Model> m_Model{};
	glm::vec3 m_Color{ 1.0f, 1.0f, 1.0f };
	TransformComponent m_Transform{};

private:
	GameObject( id_t id ) : m_id{ id } {};

		id_t m_id;
};