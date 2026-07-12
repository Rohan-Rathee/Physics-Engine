/**
 * @file spawner.h
 * @brief Defines the SpawnManager class that is used in engine .cpp to spawn chess-esque  pieces in the game world.
 * kindoff handles death but be respawning, no deat animation yet.
 * 
 */
#pragma once
#include "character.h"
#include "chess_piece_controller.h"
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <functional>

struct SpawnPoint
{
    glm::vec3      position;
    ChessPieceType pieceType;
    int            team        = 0;      
    float          respawnDelay = 5.0f;  
    int            respawnsRemaining = -1;  

    std::vector<glm::vec3> patrolPoints; 
    std::weak_ptr<Character> occupant;   

    float                    cooldown = 0.0f;
    bool                     isOnCooldown = false;
};

using CharacterFactory = std::function<std::shared_ptr<Character>(const SpawnPoint&)>;
class SpawnManager
{
public:
    explicit SpawnManager(CharacterFactory factory)
        : m_factory(std::move(factory)) {}

    void addSpawnPoint(SpawnPoint sp) { m_points.push_back(std::move(sp)); }
    std::vector<std::shared_ptr<Character>> getLivingCharacters() const;

    void update(float deltaTime);
    void spawnAll();
    size_t totalSpawnPoints() const { return m_points.size(); }
    bool allSpawnsExhausted() const; 

private:  
    std::vector<SpawnPoint> m_points;
    CharacterFactory        m_factory;

    void spawnAt(SpawnPoint& sp);
};