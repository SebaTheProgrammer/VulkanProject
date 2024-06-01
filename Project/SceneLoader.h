#pragma once
#include <vector>
#include <GameObject.h>
#include "Model.h"
#include "EngineDevice.h"
#include "json.hpp"
#include <fstream>
#define M_PI       3.14159265358979323846

using json = nlohmann::json;

class SceneLoader
{
public:
    std::vector<GameObject>& LoadGameObjects( EngineDevice& device, const std::string& filename )
    {
        std::ifstream file( filename );
        if ( !file.is_open() ) {
            throw std::runtime_error( "Failed to open JSON file: " + filename );
        }

        json jsonData;
        file >> jsonData;

        int numGameObjects = jsonData[ "num_game_objects" ];
        m_GameObjects.reserve( numGameObjects );

        for ( int i = 0; i < numGameObjects; i++ )
        {
            std::string objFilePath = jsonData[ "game_objects" ][ i ][ "obj_file_path" ].get<std::string>();
            glm::vec3 location = glm::vec3(
                jsonData[ "game_objects" ][ i ][ "location" ][ 0 ].get<float>(),
                jsonData[ "game_objects" ][ i ][ "location" ][ 1 ].get<float>(),
                jsonData[ "game_objects" ][ i ][ "location" ][ 2 ].get<float>()
            );
            float scale = jsonData[ "game_objects" ][ i ][ "scale" ].get<float>();

            std::shared_ptr<Model> model = Model::CreateModelFromFile( device, objFilePath );
            auto gameObject = GameObject::Create();

            gameObject.m_Model = model;
            gameObject.m_Transform.translation = location;
            gameObject.m_Transform.scale = glm::vec3( scale );

            m_GameObjects.emplace_back( std::move( gameObject ) );
        }
        return m_GameObjects;
    }

    std::vector<GameObject>& LoadInstancedGameObjects( EngineDevice& device, const std::string& filename, int howmany = 0 )
    {
        std::ifstream file( filename );
        if ( !file.is_open() ) {
            throw std::runtime_error( "Failed to open JSON file: " + filename );
        }

        json jsonData;
        file >> jsonData;

        int numGameObjects = jsonData[ "num_game_objects" ];
        m_GameObjects.reserve( numGameObjects );

        float spacing = 5.0f; // Distance between objects
        float layerHeight = 10.0f; // Height of each layer
        int gridSize = 10; // Calculate grid size for each layer


        for ( int j = 0; j < howmany; j++ )
        {
            int layer = j / ( gridSize * gridSize ); // Calculate current layer
            int indexInLayer = j % ( gridSize * gridSize ); // Index within the current layer
            int xIndex = indexInLayer % gridSize; // X index in grid
            int zIndex = indexInLayer / gridSize; // Z index in grid

            for ( int i = 0; i < numGameObjects; i++ )
            {
                std::string objFilePath = jsonData[ "game_objects" ][ i ][ "obj_file_path" ].get<std::string>();
                glm::vec3 location = glm::vec3(
                    jsonData[ "game_objects" ][ i ][ "location" ][ 0 ].get<float>(),
                    jsonData[ "game_objects" ][ i ][ "location" ][ 1 ].get<float>(),
                    jsonData[ "game_objects" ][ i ][ "location" ][ 2 ].get<float>()
                );
                float scale = jsonData[ "game_objects" ][ i ][ "scale" ].get<float>();

                std::shared_ptr<Model> model = Model::CreateModelFromFile( device, objFilePath );
                auto gameObject = GameObject::Create();
                gameObject.m_Model = model;


                gameObject.m_Transform.translation = location;
                gameObject.m_Transform.translation.x += xIndex * spacing;
                gameObject.m_Transform.translation.z += zIndex * spacing;
                gameObject.m_Transform.translation.y += layer * layerHeight;
                gameObject.m_Transform.scale = glm::vec3( scale );


                m_GameObjects.emplace_back( std::move( gameObject ) );
            }
        }

        return m_GameObjects;
    }

private:
	std::vector<GameObject> m_GameObjects;
};