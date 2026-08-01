/**
 * @file main.cpp
 * @brief Entry point of the engine.
 *
 * Creates the engine instance, initializes all core systems,
 * starts the main game loop, and performs a clean shutdown.
 * personal note
 * ------------------------
 * Keep this file simple.
 * If main() starts getting crowded, move the logic into Engine.
 * ------------------------
 * update
 * no further changes needed here, no errors and everything is now outsourced to Engine class 
 */


#include "core/engine.h"
#include "Game/game1.h"

int main()
{
    Engine engine(1920, 1080, "My Engine");
    ChessArenaGame game;

    if (!engine.initialize(&game))
        return -1;

    engine.run();
    return 0;
}