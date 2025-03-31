#include <SDL2/SDL.h>
#include <iostream>
#include <thread>

#include "Window.h"
#include "MenuWindow.h"

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_DisplayMode displayMode;
    SDL_GetCurrentDisplayMode(0, &displayMode);
    int screenWidth = displayMode.w;
    int screenHeight = displayMode.h;
    SDL_Quit();

    int menuWidth = screenWidth;
    int menuHeight = screenHeight;
    int gameWidth = screenWidth;
    int gameHeight = screenHeight;

    std::string playerName, playerPassword;
    bool launchGame = true;

    MenuWindow menuWindow("Battle City", menuWidth, menuHeight);
    menuWindow.StartScreen(playerName, playerPassword);

    if (menuWindow.GetRunningState()) {
        menuWindow.TitleScreen();
    }

    if (menuWindow.GetRunningState()) {
        menuWindow.MainMenu(playerName, playerPassword, launchGame);
    }

    Window myWindow("Battle City", gameWidth, gameHeight);

    menuWindow.CleanUp();

    // Lambda function for periodically updating the game
    if (launchGame == true) {
        auto updateGameWindow = [&]() {
            bool gameRunning = true;
            while (gameRunning) {
                SDL_Event e;
                while (SDL_PollEvent(&e) != 0) {
                    if (e.type == SDL_QUIT) {
                        gameRunning = false;
                    }
                }

                myWindow.Clear();
                myWindow.Render();
                SDL_Delay(16);
            }
            };

        std::thread gameUpdateThread(updateGameWindow);

        myWindow.Run();

        gameUpdateThread.join();
    }

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}