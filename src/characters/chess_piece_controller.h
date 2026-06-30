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
                         float moveSpeed = 4.0f);

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


    enum class RookAxis { X, Z };
    RookAxis m_rookAxis = RookAxis::X;





    enum class DashPhase { Dashing, Pausing };
    DashPhase m_dashPhase    = DashPhase::Dashing;
    float     m_dashProgress = 0.0f;
    glm::vec3 m_dashDir      = glm::vec3(0,0,1);

    static constexpr float kAxisChangePause = 1.0f;





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