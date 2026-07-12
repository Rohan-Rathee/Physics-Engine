#pragma once
#include "../game/game.h"
#include "../characters/spawner.h"
#include "../characters/chess_piece_controller.h"
#include <memory>
#include <vector>
#include <string>

struct LevelDef {
    std::string mapModelPath;
    glm::vec3   playerStart;
    std::vector<SpawnPoint> spawnPoints;
    std::string lightsJsonPath = "lights.json"; 
};

class ChessArenaGame : public IGame
{
public:
    bool onInitialize(Engine& engine) override;
    void onUpdate(float deltaTime) override;
    void onRenderUI() override;
    Character* getCameraTarget() override { return playerCharacter.get(); }

private:
    void buildLevelDefs();
    void loadLevel(int index);
    void unloadCurrentLevel();
    bool isLevelClear() const;
    bool spawnPlayer(const glm::vec3& position);

    std::string modelForPiece(ChessPieceType t);
    float speedForPiece(ChessPieceType t);
    std::shared_ptr<Character> spawnBot(
        const std::string& modelPath, const glm::vec3& spawnPosition,
        std::vector<glm::vec3> patrolRoute, ChessPieceType pieceType, float respawnDelay);

    Engine* engine = nullptr;
    std::shared_ptr<Character> playerCharacter;
    std::unique_ptr<SpawnManager> spawnManager;
    std::vector<std::shared_ptr<Character>> bots;

    std::vector<LevelDef> levels;
    int   currentLevel = -1;
    float carriedPlayerHealth = -1.0f; 
};