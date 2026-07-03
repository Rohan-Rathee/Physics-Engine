#include "spawner.h"

#include <iostream>

void SpawnManager::spawnAt(SpawnPoint& sp)

{

    auto ch = m_factory(sp);

    if (!ch)

    {

        std::cerr << "[SpawnManager] Factory returned null for piece type "

                  << (int)sp.pieceType << "\n";

        return;

    }

    sp.occupant      = ch;

    sp.isOnCooldown  = false;
    sp.cooldown      = 0.0f;
}
                            
void SpawnManager::spawnAll()

{

    for (auto& sp : m_points)

        spawnAt(sp);

}

void SpawnManager::update(float dt)

{

    for (auto& sp : m_points)

    {

        auto alive = sp.occupant.lock();

        if (alive && !alive->isDead())

            continue; 

        if (!sp.isOnCooldown)

        {

            

            sp.isOnCooldown = true;

            sp.cooldown     = sp.respawnDelay;

        }

        else

        {

            sp.cooldown -= dt;

            if (sp.cooldown <= 0.0f)

                spawnAt(sp);

        }

    }

}

std::vector<std::shared_ptr<Character>> SpawnManager::getLivingCharacters() const

{

    std::vector<std::shared_ptr<Character>> out;

    for (const auto& sp : m_points)

    {

        auto ch = sp.occupant.lock();

        if (ch && !ch->isDead())

            out.push_back(ch);

    }

    return out;

}