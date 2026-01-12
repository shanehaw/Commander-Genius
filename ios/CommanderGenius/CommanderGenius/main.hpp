//
//  main.hpp
//  CommanderGenius
//
//  Created by Shane Haw on 07/01/2026.
//


#include <SDL.h> // Needed to properly handle main() on iOS

#include "CGeniusEntry.h"

int main(int argc, char* argv[]) {
    // This function is extracted into another file to show how to call a project function as the iOS project includes main directly
    // The project is built using CMake and available as a library
    return CommanderGenius_Run(argc, argv);
}
