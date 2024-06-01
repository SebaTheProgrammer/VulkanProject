#pragma once
#include <vector>
#include <GameObject.h>
#include "Model.h"
#include "EngineDevice.h"
#include "json.hpp"
#include <fstream>
#include <random>
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
        int instancedGameObjects = jsonData[ "instanced_game_objects" ];
        m_GameObjects.reserve( numGameObjects * instancedGameObjects );

        if( instancedGameObjects > 1)
		{
			return LoadInstancedGameObjects( device, filename, instancedGameObjects );
		}
        else 
        {

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
    }

    std::vector<GameObject>& LoadInstancedGameObjects( EngineDevice& device, const std::string& filename, int howmany = 1 )
    {
        std::ifstream file( filename );
        if ( !file.is_open() ) {
            throw std::runtime_error( "Failed to open JSON file: " + filename );
        }

        json jsonData;
        file >> jsonData;

        int numGameObjects = jsonData[ "num_game_objects" ];
        m_GameObjects.reserve( numGameObjects * howmany );

        float spacing = 5.0f;
        float layerHeight = 10.0f;
        int gridSize = 10;


        for ( int j = 0; j < howmany; j++ )
        {
            int layer = j / ( gridSize * gridSize );
            int indexInLayer = j % ( gridSize * gridSize );
            int xIndex = indexInLayer % gridSize;
            int zIndex = indexInLayer / gridSize;

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

                //random scale:
                std::random_device rd;
                std::mt19937 gen( rd() );
                std::uniform_real_distribution<float> scaleDistribution( 0.2f, 1.5f );
                float randomScale = scaleDistribution( gen );
                gameObject.m_Transform.scale = glm::vec3( randomScale );

                //random rotation:
                float randomAngle = ( ( float ) rand() / ( float ) RAND_MAX ) * 2.0f * M_PI;
                gameObject.m_Transform.rotation = glm::vec3( randomAngle, randomAngle, randomAngle );

                m_GameObjects.emplace_back( std::move( gameObject ) );
            }
        }

        return m_GameObjects;
    }

private:
	std::vector<GameObject> m_GameObjects;
};