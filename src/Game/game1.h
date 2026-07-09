#pragma once
#include "../game/game.h"
#include "../characters/spawner.h"
#include "../characters/chess_piece_controller.h"
#include <memory>
#include <vector>

class ChessArenaGame : public IGame
{
public:
    bool onInitialize(Engine& engine) override;
    void onUpdate(float deltaTime) override;
    void onRenderUI() override;
    Character* getCameraTarget() override { return playerCharacter.get(); }

private:
    std::string modelForPiece(ChessPieceType t);
    float speedForPiece(ChessPieceType t);
    std::shared_ptr<Character> spawnBot(
        const std::string& modelPath, const glm::vec3& spawnPosition,
        std::vector<glm::vec3> patrolRoute, ChessPieceType pieceType, float respawnDelay);

    Engine* engine = nullptr;
    std::shared_ptr<Character> playerCharacter;
    std::unique_ptr<SpawnManager> spawnManager;
    std::vector<std::shared_ptr<Character>> bots;
};