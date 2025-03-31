#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <vector>
#include <string>
#include <cpr/cpr.h>
#include <crow.h>

class MenuWindow {
private:
    // Member Variables
    SDL_Window* m_window;
    SDL_Renderer* m_renderer;
    SDL_Texture* m_background;
    TTF_Font* m_font;
    bool m_running;
    int m_width, m_height;
    int m_playerId;
    std::string m_errorMessage;
    bool m_startScreenActive = false;

public:
    // Constructor and Destructor
    MenuWindow(const char* title, int width, int height);
    ~MenuWindow();

    // Running State
    bool GetRunningState();
    void SetRunningState(bool running);

    // Menu Functions
    void TitleScreen();
    void MainMenu(std::string& name, std::string& password, bool& launchGame);
    void SettingsMenu(std::string& name, std::string& password);
    void ChangeNameScreen(std::string& name);
    void ChangePasswordScreen(std::string& password);
    void ChangeDifficulty();
    void ViewDifficulty();
    void ViewScore(const std::string& name);
    void JoinGame(std::string& name, std::string& password);
    void StartScreen(std::string& name, std::string& password);

    // Cleanup
    void CleanUp();

private:
    // Helper Functions
    void RenderText(const std::string& text, int x, int y, SDL_Color color);
    int HandleMenuSelection(const std::vector<std::string>& options);
};