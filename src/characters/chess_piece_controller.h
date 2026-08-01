/**
 * @file chess_piece_controller.h
 * @brief Defines the ChessPieceController class for controlling chess-like pieces in the game world.
 * has the logic of where i want the engine demo game to go towards, kinda like a musics beat based (unimplemented) PVE where you use timing to do crit or smth to the boring no musics chess pieces, 
 * but with their own characteristincs damnn my font makes big words a (\w{4}\s?) chore *characteristics (ahh only n messed me up)
 *                                                                         ^ regex
 * anyways
 * theyu have their own movement patterns and stuff, like the pawns try to surround you by aiming towards a point in a  circle around you, and knight gets very close and then retreats/dashes sideways, rook and bishop take small steps and dash on axis alignment, and qween gets to do anything
 * will make dashers like big tanks, with chargeup, and king be like a few stories tall when i complete the models
 * bit roguelike and bit like rhythm inspired by beatsperminute BPM, and robobeat.
 * viewer please remember this is a game engine, not a game, and the chess pieces are just a demo of the engine's capabilities, not a game in itself. 
 * it is in no way a demo of my game design skills, which are .... not good, for the lack of words, and i am not a game designer, just a programmer who likes to make things move around in 3d space and look pretty.
 */
 #pragma once
#include "controller.h"
#include <glm/glm.hpp>
#include <vector>

class Character;

enum class ChessPieceType { Pawn, Rook, Knight, Bishop, Queen, King };

class ChessPieceController : public IController
{
public:
    ChessPieceController(ChessPieceType type,
                         std::vector<glm::vec3> patrolPoints,
                         float moveSpeed = 2.0f);

    void setTarget(const Character* target) { m_target = target; }
    ChessPieceType getPieceType() const { return m_type; }

    ControlInput getInput(float deltaTime, const glm::vec3& pos) override;

private:
    ChessPieceType         m_type;
    std::vector<glm::vec3> m_patrol;
    const Character*       m_target  = nullptr;
    int                    m_patrolIdx = 0;
    float                  m_speed;

    enum class KnightPhase { MoveForward, MoveSide, Pause };
    KnightPhase  m_knightPhase    = KnightPhase::MoveForward;
    float        m_knightProgress = 0.0f; 

    glm::vec3    m_knightForward  = glm::vec3(0,0,1);
    glm::vec3    m_knightSide     = glm::vec3(1,0,0);
    bool         m_knightSideDir  = true; 

    enum class BishopPhase
    {
        Approach,
        Dash
    };

    BishopPhase m_bishopPhase = BishopPhase::Approach;
    glm::vec3 m_bishopDiag;

    enum class RookAxis { X, Z };
    RookAxis m_rookAxis = RookAxis::X;

    enum class RookPhase { Approach, Dash };
    RookPhase m_rookPhase = RookPhase::Approach;

    enum class DashPhase { Dashing, Pausing };
    DashPhase m_dashPhase    = DashPhase::Dashing;
    float     m_dashProgress = 0.0f;           

    glm::vec3 m_dashDir      = glm::vec3(0,0,1); 

    float m_axisChangePause = 1.0f; 

    ControlInput dashStep(float dt, const glm::vec3& desiredDir,
                          float dashDistance, float speed);

    float m_formationAngle = 0.0f; 

    static constexpr float kSurroundRadius = 3.5f;

    glm::vec3 surroundPoint() const;

    ControlInput pawnInput (float dt, const glm::vec3& pos);
    ControlInput rookInput (float dt, const glm::vec3& pos);
    ControlInput knightInput(float dt, const glm::vec3& pos);
    ControlInput bishopInput(float dt, const glm::vec3& pos);
    ControlInput queenInput (float dt, const glm::vec3& pos);
    ControlInput kingInput  (float dt, const glm::vec3& pos);

    glm::vec3 currentWaypoint() const;
    bool      reachedWaypoint(const glm::vec3& pos, float radius = 1.2f) const;
    void      advanceWaypoint();

    static glm::vec3 snapToAxis(glm::vec3 dir, RookAxis axis);

    static glm::vec3 snapToDiagonal(glm::vec3 dir);
};