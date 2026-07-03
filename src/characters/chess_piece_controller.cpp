#include "chess_piece_controller.h"
#include "character.h"
#include <glm/gtc/constants.hpp>
#include <cmath>
#include <cstdlib>  

namespace
{

    int g_pawnFormationCounter = 0;
    constexpr int kFormationSlots = 16;
}

ChessPieceController::ChessPieceController(ChessPieceType type,
                                            std::vector<glm::vec3> patrol,
                                            float speed)
    : m_type(type), m_patrol(std::move(patrol)), m_speed(speed)
{
    if (m_type == ChessPieceType::Pawn)
    {
        int slot = g_pawnFormationCounter++ % kFormationSlots;
        m_formationAngle = glm::radians(360.0f / kFormationSlots) * (float)slot;
    }

    if (m_type == ChessPieceType::Queen)
        m_axisChangePause = 1.8f; 

}

glm::vec3 ChessPieceController::currentWaypoint() const
{
    if (m_target) return m_target->getPosition();
    if (m_patrol.empty()) return glm::vec3(0);
    return m_patrol[m_patrolIdx];
}

bool ChessPieceController::reachedWaypoint(const glm::vec3& pos, float r) const
{
    glm::vec3 wp = currentWaypoint();
    glm::vec2 flat(pos.x - wp.x, pos.z - wp.z);
    return glm::length(flat) <= r;
}

void ChessPieceController::advanceWaypoint()
{
    if (!m_target && !m_patrol.empty())
        m_patrolIdx = (m_patrolIdx + 1) % (int)m_patrol.size();
}

glm::vec3 ChessPieceController::surroundPoint() const
{
    glm::vec3 base = currentWaypoint();
    if (!m_target) return base; 

    glm::vec3 offset(std::cos(m_formationAngle) * kSurroundRadius,
                      0.0f,
                      std::sin(m_formationAngle) * kSurroundRadius);
    return base + offset;
}

ControlInput ChessPieceController::dashStep(float dt, const glm::vec3& desiredDir,
                                            float dashDistance, float speed)
{
    ControlInput out;

    if (m_dashPhase == DashPhase::Pausing)
    {
        out.aimDir = m_dashDir; 

        m_dashProgress += dt / m_axisChangePause;
        if (m_dashProgress >= 1.0f)
        {
            m_dashProgress = 0.0f;
            m_dashPhase    = DashPhase::Dashing;
        }
        return out; 

    }

    if (m_dashProgress == 0.0f)
        m_dashDir = desiredDir;

    out.moveDir = m_dashDir * speed;
    out.aimDir  = m_dashDir;

    m_dashProgress += dt * speed / dashDistance;
    if (m_dashProgress >= 1.0f)
    {
        m_dashProgress = 0.0f;
        m_dashPhase    = DashPhase::Pausing;
    }
    return out;
}

glm::vec3 ChessPieceController::snapToAxis(glm::vec3 dir, RookAxis axis)
{
    if (axis == RookAxis::X)
        return glm::vec3(dir.x >= 0 ? 1.0f : -1.0f, 0, 0);
    else
   
    return glm::vec3(0, 0, dir.z >= 0 ? 1.0f : -1.0f);
}
ControlInput ChessPieceController::rookInput(float dt, const glm::vec3& pos)
{  
    static constexpr float kSmallDash = 0.1f;
    static constexpr float kLongDash  = 3.0f;
    static constexpr float kCommitDist = 0.7f;
    if (reachedWaypoint(pos))
    {
        advanceWaypoint();  
        m_rookPhase = RookPhase::Approach;
    }

    glm::vec3 to = currentWaypoint() - pos;
    to.y = 0.0f;

    if (glm::length(to) < 0.1f)
        return {};

    float dx = std::abs(to.x);
    float dz = std::abs(to.z);

    if (m_rookPhase == RookPhase::Approach)
    {
        

        if (dx < dz)
            m_rookAxis = RookAxis::X;
        else
            m_rookAxis = RookAxis::Z;

        

        

        if (dx < kCommitDist || dz < kCommitDist)
        {
            m_rookPhase = RookPhase::Dash;
        }
        else
        {
            return dashStep(
                dt,
                snapToAxis(to, m_rookAxis),
                kSmallDash,
                m_speed * 0.5f
            );
        }
    }

    RookAxis dashAxis;

    if (dx < kCommitDist)
    {
        dashAxis = RookAxis::Z;
    }
    else if (dz < kCommitDist)
    {
        dashAxis = RookAxis::X;
    }
    else
    {
        

        

        m_rookPhase = RookPhase::Approach;
        return {};
    }

    DashPhase before = m_dashPhase;

    ControlInput out = dashStep(
        dt,
        snapToAxis(to, dashAxis),
        kLongDash,
        m_speed
    );

    if (before == DashPhase::Dashing &&
        m_dashPhase == DashPhase::Pausing)
    {
        m_rookPhase = RookPhase::Approach;
    }

    return out;
}

ControlInput ChessPieceController::knightInput(float dt, const glm::vec3& pos)
{
    ControlInput out;
    const float FWD_DIST   = 2.0f;  

    const float SIDE_DIST  = 1.0f;  

    const float PAUSE_TIME = 0.3f;

    switch (m_knightPhase)
    {
        case KnightPhase::MoveForward:
        {
            

            glm::vec3 toWP = currentWaypoint() - pos;
            toWP.y = 0;
            if (glm::length(toWP) > 0.1f)
                m_knightForward = glm::normalize(toWP);

            

            m_knightSide = m_knightSideDir
                ? glm::normalize(glm::cross(m_knightForward, glm::vec3(0,1,0)))
                : glm::normalize(glm::cross(glm::vec3(0,1,0), m_knightForward));

            out.moveDir = m_knightForward * m_speed * 1.5f;
            out.aimDir  = m_knightForward;

            m_knightProgress += dt * m_speed / FWD_DIST;
            if (m_knightProgress >= 1.0f)
            {
                m_knightProgress = 0.0f;
                m_knightPhase    = KnightPhase::MoveSide;
            }
            break;
        }
        case KnightPhase::MoveSide:
        {
            out.moveDir = m_knightSide * m_speed;
            out.aimDir  = m_knightForward; 

            m_knightProgress += dt * m_speed / SIDE_DIST;
            if (m_knightProgress >= 1.0f)
            {
                m_knightProgress    = 0.0f;
                m_knightPhase       = KnightPhase::Pause;
                m_knightSideDir     = !m_knightSideDir; 

                if (reachedWaypoint(pos)) advanceWaypoint();
            }
            break;
        }
        case KnightPhase::Pause:
        {
            

            m_knightProgress += dt / PAUSE_TIME;
            if (m_knightProgress >= 1.0f)
            {
                m_knightProgress = 0.0f;
                m_knightPhase    = KnightPhase::MoveForward;
            }
            break;
        }
    }
    return out;
}

glm::vec3 ChessPieceController::snapToDiagonal(glm::vec3 dir)
{

    const glm::vec3 diags[4] = {
        glm::normalize(glm::vec3( 1, 0,  1)),
        glm::normalize(glm::vec3( 1, 0, -1)),
        glm::normalize(glm::vec3(-1, 0,  1)),
        glm::normalize(glm::vec3(-1, 0, -1)),
    };
    float best  = -2.0f;
    int   bestI = 0;
    for (int i = 0; i < 4; ++i)
    {
        float d = glm::dot(dir, diags[i]);
        if (d > best) { best = d; bestI = i; }
    }
    return diags[bestI];
}

ControlInput ChessPieceController::bishopInput(float dt, const glm::vec3& pos)
{
    static constexpr float kSmallDash  = 1.0f;
    static constexpr float kLongDash   = 1.2f;
    static constexpr float kCommitDist = 0.6f;

    if (reachedWaypoint(pos))
    {
        advanceWaypoint();
        m_bishopPhase = BishopPhase::Approach;
    }

    glm::vec3 to = currentWaypoint() - pos;
    to.y = 0.0f;

    if (glm::length(to) < 0.1f)
        return {};

    if (m_bishopPhase == BishopPhase::Approach)
    {
        

        m_bishopDiag = snapToDiagonal(glm::normalize(to));

        

        float along = glm::dot(to, m_bishopDiag);
        glm::vec3 projected = m_bishopDiag * along;

        float offDiagonal = glm::length(to - projected);

        

        if (offDiagonal < kCommitDist)
        {
            m_bishopPhase = BishopPhase::Dash;
        }
        else
        {
            

            return dashStep(
                dt,
                m_bishopDiag,
                kSmallDash,
                m_speed * 0.5f
            );
        }
    }

    float along = glm::dot(to, m_bishopDiag);

    if (along < 0.0f)
    {
        m_bishopPhase = BishopPhase::Approach;
        return {};
    }

    DashPhase before = m_dashPhase;

    ControlInput out = dashStep(
        dt,
        m_bishopDiag,
        kLongDash,
        m_speed
    );

    if (before == DashPhase::Dashing &&
        m_dashPhase == DashPhase::Pausing)
    {
        m_bishopPhase = BishopPhase::Approach;
    }

    return out;
}

ControlInput ChessPieceController::pawnInput(float dt, const glm::vec3& pos)
{
    static constexpr float kDashDistance = 0.1f;

    if (reachedWaypoint(pos)) advanceWaypoint();

    glm::vec3 destination = currentWaypoint(); 

    if (m_target)
    {
        glm::vec3 toTarget = destination - pos;
        toTarget.y = 0;
        bool withinChaseRange = glm::length(toTarget) <= kSurroundRadius;
        if (!withinChaseRange)
            destination = surroundPoint(); 

        

        

    }

    glm::vec3 to = destination - pos;
    to.y = 0;
    if (glm::length(to) < 0.1f) return {};
    glm::vec3 dir = glm::normalize(to);
    return dashStep(dt, dir, kDashDistance, m_speed * 0.5f);
}

ControlInput ChessPieceController::queenInput(float dt, const glm::vec3& pos)
{
    if (reachedWaypoint(pos))
        advanceWaypoint();

    glm::vec3 to = currentWaypoint() - pos;
    to.y = 0.0f;


    glm::vec3 dir = glm::normalize(to);


    float dashDistance = 8.0f;

    return dashStep(
        dt,
        dir,
        dashDistance,
        m_speed * 1.4f   
    );
}

ControlInput ChessPieceController::kingInput(float dt, const glm::vec3& pos)
{
    static constexpr float kDashDistance = 0.3f; 

    if (reachedWaypoint(pos)) advanceWaypoint();
    glm::vec3 to = currentWaypoint() - pos;
    to.y = 0;
    if (glm::length(to) < 0.1f) return {};
    glm::vec3 dir = glm::normalize(to);
    return dashStep(dt, dir, kDashDistance, m_speed * 0.5f);
}

ControlInput ChessPieceController::getInput(float dt, const glm::vec3& pos)
{
    switch (m_type)
    {
        case ChessPieceType::Pawn:   return pawnInput(dt, pos);
        case ChessPieceType::Rook:   return rookInput(dt, pos);
        case ChessPieceType::Knight: return knightInput(dt, pos);
        case ChessPieceType::Bishop: return bishopInput(dt, pos);
        case ChessPieceType::Queen:  return queenInput(dt, pos);
        case ChessPieceType::King:   return kingInput(dt, pos);
    }
    return {};
}