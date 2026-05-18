#include <iostream>
#include "core/engine.h"

int main() {
    try {
        Engine engine(1280, 720, "Physics Engine - Decentralized");
        
        if (!engine.initialize()) {
            std::cerr << "Failed to initialize engine" << std::endl;
            return -1;
        }
        
        engine.run();
        engine.shutdown();
        
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return -1;
    }
}
