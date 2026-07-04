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


 #include <iostream>

#include "core/engine.h"

int main() {
    try {
        Engine engine(1920, 1080, "Physics Engine");

        if (!engine.initialize()) {
            std::cerr << "Failed to initialize engine" << std::endl;
            return -1;
        }

        engine.run();
        engine.shutdown();

        return 0;

    } catch (const std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return -1;
    }
}