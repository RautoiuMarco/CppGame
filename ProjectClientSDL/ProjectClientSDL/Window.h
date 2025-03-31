#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <cpr/cpr.h>
#include <crow.h>
#include <vector>
#include <thread>
#include <conio.h>
#include <limits>
#include <map>

class Window
{
private:
    // Member Variables
    SDL_Window* m_window;
    bool m_running;
    SDL_Renderer* renderer;
    int m_width, m_height;
    int m_playerId;
    std::vector<std::vector<int>> m_board;
    std::map<int, SDL_Texture*> m_textures;
    std::map<int, std::vector<SDL_Texture*>> m_multiTextures;

public:
    // Constructor and Destructor
    Window(const char* title, int width, int height);
    ~Window();

    // Main Loop
    void Run();
    void Render();
    void Clear();

    // Texture Management
    void LoadTextures();
    SDL_Texture* GetTileTextureBasedOnBoardValue(int boardValue);

    // Game Logic
    void UpdateBoard();
    void PlayerAction(int playerId, std::string action);

    // Utility
    void GetTime();
};

