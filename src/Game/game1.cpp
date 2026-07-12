
#include "game1.h"
#include "../characters/human_controller.h"
#include "../core/engine.h"
#include <iostream>

void ChessArenaGame::buildLevelDefs() {
    {
        LevelDef level1;
        level1.mapModelPath = "models\\parts1.glb";
        level1.playerStart = glm::vec3(0.0f, 1.0f, 0.0f);
        level1.lightsJsonPath = "lights_level1.json"; 
        level1.spawnPoints.push_back({.position = glm::vec3(0.0f, 1.0f, 18.0f),
                                      .pieceType = ChessPieceType::Queen,
                                      .team = 1,
                                      .respawnDelay = 3.0f,
                                      .respawnsRemaining = 1,
                                      .patrolPoints = {glm::vec3(0, 0, 18), glm::vec3(0, 0, -18)}});
        levels.push_back(std::move(level1));
    }
    {
        LevelDef level2;
        level2.mapModelPath = "models\\parts1.glb";
        level2.playerStart = glm::vec3(0.0f, 1.0f, 0.0f);
        level2.lightsJsonPath = "lights_level2.json";
        level2.spawnPoints.push_back({.position = glm::vec3(-5.0f, 1.0f, 18.0f),
                                      .pieceType = ChessPieceType::Rook,
                                      .team = 1,
                                      .respawnDelay = 3.0f,
                                      .respawnsRemaining = -1,
                                      .patrolPoints = {glm::vec3(-5, 0, 18), glm::vec3(-5, 0, -18)}});
        level2.spawnPoints.push_back({.position = glm::vec3(5.0f, 1.0f, 18.0f),
                                      .pieceType = ChessPieceType::Queen,
                                      .team = 1,
                                      .respawnDelay = 3.0f,
                                      .respawnsRemaining = 1,
                                      .patrolPoints = {glm::vec3(5, 0, 18), glm::vec3(5, 0, -18)}});
        levels.push_back(std::move(level2));
    }
}

bool ChessArenaGame::onInitialize(Engine &eng) {
    engine = &eng;
    buildLevelDefs();
    loadLevel(0);
    return true;
}

/** old initialize for refernece
bool ChessArenaGame::onInitialize(Engine &eng) {

    engine = &eng;
    ModelLoader *modelLoader = engine->getModelLoader();
    ModelTransform *modelTransform = engine->getModelTransform();

    btCollisionShape *basketballShape = nullptr;

    for (int i = 0; i < 1; ++i) {
        for (int j = 0; j < 1; ++j) {
            modelLoader->loadModel("models\\p1.glb", glm::vec3(0.0f), glm::vec3(1.0f));

            int currentIndex = static_cast<int>(modelLoader->getModelCount()) - 1;

            modelLoader->setModelAnimation(currentIndex, 0);

            if (basketballShape == nullptr && modelTransform) {
                Model *m = modelLoader->getModel(currentIndex).model.get();

                basketballShape = m->buildCapsuleColliderFromMesh();

                glm::vec3 scale = modelLoader->getModelScale(currentIndex);
                basketballShape->setLocalScaling(btVector3(scale.x, scale.y, scale.z));
            }

            modelTransform->setTransform(currentIndex, glm::vec3(0.0f, 10.0f, 0.0f), glm::vec3(1.0f), 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));

            if (modelTransform && basketballShape) {
                modelTransform->initializePhysicsBody(
                    currentIndex,
                    80.0f,
                    basketballShape,
                    0.1f);
            }
        }
    }
    modelLoader->loadModel("models\\parts1.glb", glm::vec3(0.0f), glm::vec3(1.0f));

    size_t mapIndex = modelLoader->getModelCount() - 1;

    modelTransform->initializePhysicsBody(
        mapIndex,
        0.0f,
        modelLoader->getModel(mapIndex).model->buildCompoundBoxCollider(),
        0.2f);
    btRigidBody *playerBody = engine->getModelTransform()->getPhysicsBody(0);
    if (playerBody) {
        auto humanCtrl = std::make_unique<HumanController>(engine->getInputSystem(), engine->getCamera());
        playerCharacter = std::make_shared<Character>(
            0, playerBody, engine->getModelLoader(), engine->getPhysicsSystem(),
            std::move(humanCtrl), 0, 1, 2, true);
        engine->getScene()->addEntity(playerCharacter);
    } else {
        std::cerr << "ChessArenaGame::onInitialize: no rigid body at index 0 — player not spawned.\n";
    }

    spawnManager = std::make_unique<SpawnManager>(
        [this](const SpawnPoint &sp) -> std::shared_ptr<Character> {
            if (auto existing = sp.occupant.lock()) {
                existing->respawn(sp.position);
                return existing;
            }
            return spawnBot(modelForPiece(sp.pieceType), sp.position,
                            sp.patrolPoints, sp.pieceType, sp.respawnDelay);
        });

    const float pawnX[] = {0.0f};
    for (float px : pawnX) {
        spawnManager->addSpawnPoint({.position = glm::vec3(px, 1.0f, 18.0f),
                                     .pieceType = ChessPieceType::Queen,
                                     .team = 1,
                                     .respawnDelay = 3.0f,
                                     .patrolPoints = {glm::vec3(px, 0, 18), glm::vec3(px, 0, -18)}});
    }
    spawnManager->spawnAll();
    return true;
}
*/

bool ChessArenaGame::spawnPlayer(const glm::vec3 &position) {
    ModelLoader *modelLoader = engine->getModelLoader();
    ModelTransform *modelTransform = engine->getModelTransform();

    modelLoader->loadModel("models\\p1.glb", glm::vec3(0.0f), glm::vec3(1.0f));
    int currentIndex = static_cast<int>(modelLoader->getModelCount()) - 1;
    modelLoader->setModelAnimation(currentIndex, 0);

    Model *m = modelLoader->getModel(currentIndex).model.get();
    btCollisionShape *playerShape = m->buildCapsuleColliderFromMesh();
    glm::vec3 scale = modelLoader->getModelScale(currentIndex);
    playerShape->setLocalScaling(btVector3(scale.x, scale.y, scale.z));

    modelTransform->setTransform(currentIndex, position, glm::vec3(1.0f), 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
    modelTransform->initializePhysicsBody(currentIndex, 80.0f, playerShape, 0.1f);

    btRigidBody *playerBody = modelTransform->getPhysicsBody(currentIndex);
    if (!playerBody) {
        std::cerr << "ChessArenaGame::spawnPlayer: no rigid body at index " << currentIndex << " — player not spawned.\n";
        return false;
    }

    auto humanCtrl = std::make_unique<HumanController>(engine->getInputSystem(), engine->getCamera());
    playerCharacter = std::make_shared<Character>(
        currentIndex, playerBody, modelLoader, engine->getPhysicsSystem(),
        std::move(humanCtrl), 0, 1, 2, true);

    if (carriedPlayerHealth >= 0.0f) {
        float damage = playerCharacter->getHealth() - carriedPlayerHealth;
        if (damage > 0.0f)
            playerCharacter->takeDamage(damage);
    }

    engine->getScene()->addEntity(playerCharacter);
    return true;
}
void ChessArenaGame::unloadCurrentLevel() {
    if (currentLevel < 0) return;

    Scene *scene = engine->getScene();

    if (playerCharacter) {
        carriedPlayerHealth = playerCharacter->getHealth();
        scene->removeEntity(playerCharacter);
        playerCharacter.reset();
    }

    for (auto &bot : bots)
        scene->removeEntity(bot);
    bots.clear();
    spawnManager.reset();

    engine->getModelTransform()->clear();
    engine->getModelLoader()->clearModels(); 
}

void ChessArenaGame::loadLevel(int index) {
    if (index < 0 || index >= static_cast<int>(levels.size())) {
        std::cerr << "ChessArenaGame::loadLevel: invalid level index " << index << "\n";
        return;
    }

    unloadCurrentLevel();

    currentLevel = index;
    const LevelDef &level = levels[index];

    ModelLoader *modelLoader = engine->getModelLoader();
    ModelTransform *modelTransform = engine->getModelTransform();

    engine->getLightManager()->loadFromFile(level.lightsJsonPath);
    engine->getImGuiSystem()->onLevelChanged(); 
    spawnPlayer(level.playerStart);

    modelLoader->loadModel(level.mapModelPath, glm::vec3(0.0f), glm::vec3(1.0f));
    size_t mapIndex = modelLoader->getModelCount() - 1;
    modelTransform->initializePhysicsBody(
        mapIndex, 0.0f, modelLoader->getModel(mapIndex).model->buildCompoundBoxCollider(), 0.2f);

    spawnManager = std::make_unique<SpawnManager>(
        [this](const SpawnPoint &sp) -> std::shared_ptr<Character> {
            if (auto existing = sp.occupant.lock()) {
                existing->respawn(sp.position);
                return existing;
            }
            return spawnBot(modelForPiece(sp.pieceType), sp.position,
                            sp.patrolPoints, sp.pieceType, sp.respawnDelay);
        });

    for (const auto &sp : level.spawnPoints) {
        spawnManager->addSpawnPoint(sp);
    }
    spawnManager->spawnAll();
}

bool ChessArenaGame::isLevelClear() const {
    return spawnManager  && bots.empty() && spawnManager->allSpawnsExhausted();
}

void ChessArenaGame::onUpdate(float deltaTime) {
    if (spawnManager) {
        spawnManager->update(deltaTime);
        bots = spawnManager->getLivingCharacters();
    }
    if (isLevelClear() && currentLevel + 1 < (int)levels.size())
        loadLevel(currentLevel + 1);
}

void ChessArenaGame::onRenderUI() {
    engine->getImGuiSystem()->renderPlayerHUD(playerCharacter.get());
    engine->getImGuiSystem()->renderBotHealthBars(
        bots, engine->getCamera()->GetViewMatrix(),
        glm::perspective(glm::radians(engine->getCamera()->Zoom),
                         (float)engine->getScreenWidth() / engine->getScreenHeight(), 0.1f, 1000000.0f),
        engine->getScreenWidth(), engine->getScreenHeight());

    ImGui::Begin("Game");
    ImGui::Text("Bots alive: %zu / %zu", bots.size(), spawnManager ? spawnManager->totalSpawnPoints() : 0u);
    ImGui::End();
}

std::string ChessArenaGame::modelForPiece(ChessPieceType t) {
    switch (t) {
    case ChessPieceType::Pawn:
        return "models\\untitled1.glb";
    case ChessPieceType::Rook:
        return "models\\untitled1.glb";
    case ChessPieceType::Knight:
        return "models\\untitled1.glb";
    case ChessPieceType::Bishop:
        return "models\\untitled1.glb";
    case ChessPieceType::Queen:
        return "models\\untitled1.glb";
    case ChessPieceType::King:
        return "models\\untitled1.glb";
    default:
        return "models\\untitled1.glb";
    }
}

float ChessArenaGame::speedForPiece(ChessPieceType t) {
    switch (t) {
    case ChessPieceType::Pawn:
        return 10.0f;
    case ChessPieceType::Rook:
        return 3.0f;
    case ChessPieceType::Knight:
        return 10.0f;
    case ChessPieceType::Bishop:
        return 3.5f;
    case ChessPieceType::Queen:
        return 3.5f;
    case ChessPieceType::King:
        return 10.0f;
    default:
        return 10.0f;
    }
}

std::shared_ptr<Character> ChessArenaGame::spawnBot(
    const std::string &modelPath, const glm::vec3 &spawnPosition,
    std::vector<glm::vec3> patrolRoute, ChessPieceType pieceType, float /*respawnDelay*/) {
    ModelLoader *modelLoader = engine->getModelLoader();
    modelLoader->loadModel(modelPath, spawnPosition, glm::vec3(1.0f));
    size_t botIndex = modelLoader->getModelCount() - 1;
    modelLoader->setModelAnimation(botIndex, 0);

    btCollisionShape *botShape = modelLoader->getModel(botIndex).model->buildCapsuleColliderFromMesh();
    glm::vec3 botScale = modelLoader->getModelScale(botIndex);
    botShape->setLocalScaling(btVector3(botScale.x, botScale.y, botScale.z));

    engine->getModelTransform()->setTransform(botIndex, spawnPosition, glm::vec3(1.0f), 0.0f, glm::vec3(1, 0, 0));
    engine->getModelTransform()->initializePhysicsBody(botIndex, 80.0f, botShape, 0.1f);

    btRigidBody *botBody = engine->getModelTransform()->getPhysicsBody(botIndex);
    if (!botBody) {
        std::cerr << "ChessArenaGame::spawnBot: no rigid body at index " << botIndex << "\n";
        return nullptr;
    }

    auto ctrl = std::make_unique<ChessPieceController>(pieceType, std::move(patrolRoute), speedForPiece(pieceType));
    if (playerCharacter)
        ctrl->setTarget(playerCharacter.get());

    auto bot = std::make_shared<Character>(botIndex, botBody, modelLoader, engine->getPhysicsSystem(),
                                           std::move(ctrl), 0, 1, 2, false);
    engine->getScene()->addEntity(bot);
    bots.push_back(bot);
    return bot;
}