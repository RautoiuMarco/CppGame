#include "Window.h"

// Grid settings
const int GRID_ROWS = 10;
const int GRID_COLS = 10;

Window::Window(const char* title, int width, int height)
    : m_window(nullptr),
    renderer(nullptr),
    m_running(true),
    m_width(width),
    m_height(height)
{
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        m_running = false;
        return;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "Failed to initialize SDL_image: " << IMG_GetError() << std::endl;
        m_running = false;
        SDL_Quit();
        return;
    }

    // Create the SDL window
    m_window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width,
        height,
        SDL_WINDOW_SHOWN
    );

    if (!m_window) {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        m_running = false;
        SDL_Quit();
        return;
    }

    // Create the SDL renderer
    renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
        m_running = false;
        SDL_DestroyWindow(m_window);
        SDL_Quit();
        return;
    }

    LoadTextures();

    UpdateBoard();
}

Window::~Window() {
    for (auto& [key, texture] : m_textures) {
        SDL_DestroyTexture(texture);
    }

    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }

    if (m_window) {
        SDL_DestroyWindow(m_window);
    }

    IMG_Quit();
    SDL_Quit();
}

void Window::Run() {
    std::thread pollingThread([&]() {
        while (m_running) {
            auto response = cpr::Get(cpr::Url{ "http://localhost:18080/game" });
            if (response.status_code == 200) {
                UpdateBoard();
                Render();
            }
            else {
                std::cerr << "Error: Failed to fetch board state. Status code: "
                    << response.status_code << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        });

    SDL_Event event;
    while (m_running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                m_running = false;
            }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    // Notify the server
                    auto response = cpr::Post(cpr::Url{ "http://localhost:18080/closeGame" });
                    std::cout << "Server response: " << response.text << std::endl;
                    m_running = false; // Close the game window
                }
                else {
                    SDL_Keycode keyPressed = event.key.keysym.sym;
                    std::string key(1, keyPressed);
                    PlayerAction(m_playerId, key);
                }
            }
        }
        SDL_Delay(16); // Simulate ~60 FPS
    }

    // Cleanup resources
    if (pollingThread.joinable()) {
        pollingThread.join(); // Ensure the polling thread is joined
    }

    // Free SDL resources
    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }
    if (m_window) {
        SDL_DestroyWindow(m_window);
    }
    IMG_Quit(); // Quit SDL_Image
    SDL_Quit(); // Quit SDL subsystems
}

void Window::Render() {
    Clear(); // Clear the window

    int cellWidth = m_width / m_board[0].size();
    int cellHeight = m_height / m_board.size();
    auto response = cpr::Get(cpr::Url{ "http://localhost:18080/bulletsCoord" });
    auto coord = crow::json::load(response.text);
    auto bulletsCoord = coord["bullets"];
    SDL_Texture* bullet = m_textures[71];

    std::map<std::pair<int, int>, SDL_Texture*> tankTextureMap;
    std::vector<std::pair<int, int>> tankPositions;

    // Collect tank positions
    for (int row = 0; row < m_board.size(); ++row) {
        for (int col = 0; col < m_board[0].size(); ++col) {
            if (m_board[row][col] == 80) {
                tankPositions.emplace_back(row, col);
            }
        }
    }

    for (size_t i = 0; i < tankPositions.size(); ++i) {
        int textureIndex = i % m_multiTextures[80].size(); // Cycle through available textures
        tankTextureMap[tankPositions[i]] = m_multiTextures[80][textureIndex];
    }

    // Render the board
    for (int row = 0; row < m_board.size(); ++row) {
        for (int col = 0; col < m_board[0].size(); ++col) {
            SDL_Rect destRect = { col * cellWidth, row * cellHeight, cellWidth, cellHeight };
            int value = m_board[row][col];

            if (value == 35) {
                bool topDisplayed = false;
                if (!topDisplayed) {
                    SDL_RenderCopy(renderer, m_multiTextures[35][0], NULL, &destRect);
                    topDisplayed = true;
                }
                else {
                    SDL_RenderCopy(renderer, m_multiTextures[35][1], NULL, &destRect);
                }
            }
            else if (value == 80) {
                // Render tanks with assigned textures
                auto it = tankTextureMap.find({ row, col });
                if (it != tankTextureMap.end()) {
                    SDL_RenderCopy(renderer, it->second, NULL, &destRect);
                }
            }
            else {
                // Render other tiles
                SDL_Texture* currentTile = GetTileTextureBasedOnBoardValue(value);
                if (currentTile) {
                    SDL_RenderCopy(renderer, currentTile, NULL, &destRect);
                }
            }
        }
    }

    // Render bullets
    for (auto bulletCoord : bulletsCoord) {
        SDL_Rect pos = { bulletCoord["coordX"].d() * cellWidth, bulletCoord["coordY"].d() * cellHeight, cellWidth, cellHeight };
        SDL_RenderCopy(renderer, bullet, NULL, &pos);
    }
    SDL_RenderPresent(renderer);
}



void Window::LoadTextures()
{
    SDL_Surface* surfaceBase = IMG_Load("assets/apartments_base.png");
    SDL_Surface* surfaceTop = IMG_Load("assets/apartments_top.png");

    if (!surfaceBase || !surfaceTop) {
        std::cerr << "Failed to load apartment images: " << IMG_GetError() << std::endl;
        m_running = false;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(m_window);
        SDL_Quit();
        return;
    }

    SDL_Texture* textureBase = SDL_CreateTextureFromSurface(renderer, surfaceBase);
    SDL_Texture* textureTop = SDL_CreateTextureFromSurface(renderer, surfaceTop);
    SDL_FreeSurface(surfaceBase);
    SDL_FreeSurface(surfaceTop);

    if (!textureBase || !textureTop) {
        std::cerr << "Failed to create textures for apartments: " << SDL_GetError() << std::endl;
        m_running = false;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(m_window);
        SDL_Quit();
        return;
    }

    m_multiTextures[35] = { textureTop, textureBase };

    // Load textures for value 80 (players)
    const std::vector<std::pair<int, const char*>> texturePaths = {
        {32, "assets/tile.png"},
        {43, "assets/car.png"},
        {71, "assets/pellet.png"},
    };
    for (const auto& [key, path] : texturePaths) {
        SDL_Surface* surface = IMG_Load(path);
        if (!surface) {
            std::cerr << "Failed to load image: " << path << " - " << IMG_GetError() << std::endl;
            m_running = false;
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(m_window);
            SDL_Quit();
            return;
        }
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);

        if (!texture) {
            std::cerr << "Failed to create texture for " << path << ": " << SDL_GetError() << std::endl;
            m_running = false;
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(m_window);
            SDL_Quit();
            return;
        }

        m_textures[key] = texture;
    }

    const std::vector<std::pair<int, const char*>> playerTextures = {
        {0, "assets/player1.png"},
        {1, "assets/player2.png"},
        {2, "assets/player3.png"},
        {3, "assets/player4.png"},
    };

    for (const auto& [index, path] : playerTextures) {
        SDL_Surface* surface = IMG_Load(path);
        if (!surface) {
            std::cerr << "Failed to load player texture: " << path << " - " << IMG_GetError() << std::endl;
            m_running = false;
            return;
        }
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);

        if (!texture) {
            std::cerr << "Failed to create texture for " << path << ": " << SDL_GetError() << std::endl;
            m_running = false;
            return;
        }

        m_multiTextures[80].push_back(texture);
    }
}


SDL_Texture* Window::GetTileTextureBasedOnBoardValue(int boardValue)
{
    if (m_textures.count(boardValue)) {
        return m_textures[boardValue];
    }
    return nullptr;
}

void Window::UpdateBoard()
{
    auto response = cpr::Get(cpr::Url{ "http://localhost:18080/game" });
    const auto& board = crow::json::load(response.text);
    if (response.status_code != 200) {
        std::cerr << "Error: HTTP request failed with status code " << response.status_code << std::endl;
    }
    if (!board) {
        std::cerr << "Error: Failed to parse JSON response." << std::endl;
    }
    std::vector<std::vector<int>> auxBoard;
    if (board.has("board")) {
        const auto& matrix = board["board"];
        for (size_t i = 0; i < matrix.size(); ++i) {
            const auto& row = matrix[i];
            std::vector<int> vec;
            for (size_t j = 0; j < row.size(); ++j) {
                vec.push_back(row[j].i());
            }
            auxBoard.push_back(vec);
        }
    }
    m_board = auxBoard;
}

void Window::GetTime()
{
    // Fetch the game time from the server
    auto timeResponse = cpr::Get(cpr::Url{ "http://localhost:18080/time" });
    if (timeResponse.status_code == 200) {
        std::cout << "Game Time: " << timeResponse.text << " seconds" << std::endl;
    }
    else {
        std::cerr << "Error: Failed to fetch game time. Status code: "
            << timeResponse.status_code << std::endl;
    }
}

void Window::PlayerAction(int playerId, std::string action)
{
    // Send the move command to the server
    auto response = cpr::Get(cpr::Url{ "http://localhost:18080/action/" + std::to_string(playerId) + "/" + action });
}

void Window::Clear() {
    SDL_RenderClear(renderer);
}