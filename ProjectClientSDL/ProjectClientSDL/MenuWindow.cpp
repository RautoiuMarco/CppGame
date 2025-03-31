#include "MenuWindow.h"

MenuWindow::MenuWindow(const char* title, int width, int height)
    : m_window(nullptr), m_renderer(nullptr), m_font(nullptr), m_running(true), m_width(width), m_height(height) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        m_running = false;
        return;
    }

    if (TTF_Init() == -1) {
        std::cerr << "Failed to initialize SDL_ttf: " << TTF_GetError() << std::endl;
        SDL_Quit();
        m_running = false;
        return;
    }

    m_window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
    if (!m_window) {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        m_running = false;
        return;
    }

    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
    if (!m_renderer) {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(m_window);
        SDL_Quit();
        m_running = false;
        return;
    }

    SDL_Surface* bgSurface = IMG_Load("assets/background.png");
    if (!bgSurface) {
        std::cerr << "Failed to load background image: " << IMG_GetError() << std::endl;
        m_running = false;
        return;
    }

    m_background = SDL_CreateTextureFromSurface(m_renderer, bgSurface);
    SDL_FreeSurface(bgSurface);

    if (!m_background) {
        std::cerr << "Failed to create texture from background image: " << SDL_GetError() << std::endl;
        m_running = false;
        return;
    }

    m_font = TTF_OpenFont("assets/Cynatar.otf", 50);
    if (!m_font) {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        SDL_DestroyRenderer(m_renderer);
        SDL_DestroyWindow(m_window);
        SDL_Quit();
        m_running = false;
        return;
    }
}

MenuWindow::~MenuWindow() {
    if (m_font) {
        TTF_CloseFont(m_font);
    }

    if (m_renderer) {
        SDL_DestroyRenderer(m_renderer);
    }

    if (m_window) {
        SDL_DestroyWindow(m_window);
    }

    if (m_background) {
        SDL_DestroyTexture(m_background);
    }

    TTF_Quit();
    SDL_Quit();
}

void MenuWindow::RenderText(const std::string& text, int x, int y, SDL_Color color) {
    if (text.empty()) {
        return; // Avoid rendering if the text is empty
    }

    SDL_Surface* textSurface = TTF_RenderText_Solid(m_font, text.c_str(), color);
    if (!textSurface) {
        std::cerr << "Failed to render text surface: " << TTF_GetError() << std::endl;
        return;
    }

    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(m_renderer, textSurface);
    if (!textTexture) {
        std::cerr << "Failed to create text texture: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(textSurface);
        return;
    }

    SDL_Rect destRect = { x, y, textSurface->w, textSurface->h };
    SDL_RenderCopy(m_renderer, textTexture, NULL, &destRect);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

int MenuWindow::HandleMenuSelection(const std::vector<std::string>& options) {
    int selected = 0;
    SDL_Event event;

    while (m_running) {
        SDL_RenderCopy(m_renderer, m_background, NULL, NULL); // Render the background

        for (int i = 0; i < options.size(); ++i) {
            SDL_Color color = (i == selected) ? SDL_Color{ 255, 51, 204, 0 } : SDL_Color{ 255, 255, 255, 255 };
            RenderText(options[i], m_width / 2 - 100, 100 + i * 50, color);
        }

        SDL_RenderPresent(m_renderer);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                m_running = false;
                return -1;
            }
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                case SDLK_UP:
                    selected = (selected - 1 + options.size()) % options.size();
                    break;
                case SDLK_DOWN:
                    selected = (selected + 1) % options.size();
                    break;
                case SDLK_RETURN:
                    return selected;
                }
            }
        }
    }

    return -1;
}

void MenuWindow::SetRunningState(bool running) {
    m_running = running;
}

bool MenuWindow::GetRunningState() {
    return m_running;
}

void MenuWindow::StartScreen(std::string& name, std::string& password)
{
    SDL_Event event;
    std::string nameInputBuffer;
    std::string passwordInputBuffer;
    bool enteringName = true;

    while (true) {
        SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255); // Set background to black
        SDL_RenderClear(m_renderer);

        // Render instructions and input fields
        if (enteringName) {
            int namePromptWidth;
            TTF_SizeText(m_font, "Enter your name:", &namePromptWidth, nullptr);
            RenderText("Enter your name:", (m_width - namePromptWidth) / 2, m_height / 2 - 200, SDL_Color{ 255, 255, 255, 255 });

            int nameInputWidth;
            TTF_SizeText(m_font, nameInputBuffer.c_str(), &nameInputWidth, nullptr);
            RenderText(nameInputBuffer, (m_width - nameInputWidth) / 2, m_height / 2 - 150, SDL_Color{ 255, 255, 255, 255 });
        }
        else {
            int passwordPromptWidth;
            TTF_SizeText(m_font, "Set your password:", &passwordPromptWidth, nullptr);
            RenderText("Set your password:", (m_width - passwordPromptWidth) / 2, m_height / 2 - 200, SDL_Color{ 255, 255, 255, 255 });

            // Display the password as asterisks
            std::string maskedPassword(passwordInputBuffer.length(), '*');
            int passwordInputWidth;
            TTF_SizeText(m_font, maskedPassword.c_str(), &passwordInputWidth, nullptr);
            RenderText(maskedPassword, (m_width - passwordInputWidth) / 2, m_height / 2 - 150, SDL_Color{ 255, 255, 255, 255 });

            // Render the error message if it exists
            if (!m_errorMessage.empty()) {
                int errorMessageWidth;
                TTF_SizeText(m_font, m_errorMessage.c_str(), &errorMessageWidth, nullptr);
                RenderText(m_errorMessage, (m_width - errorMessageWidth) / 2, m_height / 2 - 100, SDL_Color{ 255, 255, 255, 255 });
            }
        }

        SDL_RenderPresent(m_renderer);

        // Event handling
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                m_running = false;
                return;
            }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_RETURN) {
                    if (enteringName && !nameInputBuffer.empty()) {
                        name = nameInputBuffer;
                        enteringName = false;
                    }
                    else if (!enteringName && !passwordInputBuffer.empty()) {
                        // Attempt to join the game
                        JoinGame(name, passwordInputBuffer);
                        // If there's an error clear the password input buffer for re-entry
                        if (!m_errorMessage.empty()) {
                            passwordInputBuffer.clear();
                        }
                        else {
                            password = passwordInputBuffer;
                            return;
                        }
                    }
                }
                else if (event.key.keysym.sym == SDLK_BACKSPACE) {
                    if (enteringName && !nameInputBuffer.empty()) {
                        nameInputBuffer.pop_back();
                    }
                    else if (!enteringName && !passwordInputBuffer.empty()) {
                        passwordInputBuffer.pop_back();
                    }
                }
            }
            else if (event.type == SDL_TEXTINPUT) {
                if (enteringName) {
                    nameInputBuffer += event.text.text;
                }
                else {
                    passwordInputBuffer += event.text.text; // Append entered character to password buffer
                }
            }
        }
        SDL_StartTextInput();
    }
    SDL_StopTextInput();
}

void MenuWindow::TitleScreen() {

    // Load logo.png
    SDL_Surface* logoSurface = IMG_Load("assets/logo.png");
    if (!logoSurface) {
        std::cerr << "Failed to load logo image: " << IMG_GetError() << std::endl;
        SDL_DestroyTexture(m_background);
        m_running = false;
        return;
    }

    SDL_Texture* logoTexture = SDL_CreateTextureFromSurface(m_renderer, logoSurface);
    SDL_FreeSurface(logoSurface);

    if (!logoTexture) {
        std::cerr << "Failed to create texture from logo image: " << SDL_GetError() << std::endl;
        SDL_DestroyTexture(m_background);
        m_running = false;
        return;
    }

    SDL_Event event;
    std::vector<std::string> options = { "Start" };
    int selected = 0;

    while (true) {
        SDL_RenderCopy(m_renderer, m_background, NULL, NULL);

        // Render the logo
        int logoWidth = 300;  
        int logoHeight = 75; 
        SDL_Rect logoRect = { m_width / 2 - logoWidth / 2, 50, logoWidth, logoHeight };
        SDL_RenderCopy(m_renderer, logoTexture, NULL, &logoRect);

        for (int i = 0; i < options.size(); ++i) {
            SDL_Color color = (i == selected) ? SDL_Color{ 255, 51, 204, 0 } : SDL_Color{ 255, 255, 255, 255 };
            RenderText(options[i], m_width / 2 - 50, 200 + i * 50, color);
        }

        SDL_RenderPresent(m_renderer);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                m_running = false;
                SDL_DestroyTexture(logoTexture);
                return;
            }
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                case SDLK_RETURN:
                    SDL_DestroyTexture(logoTexture);
                    return; // Proceed to the main menu
                case SDLK_UP:
                case SDLK_DOWN:
                    break;
                }
            }
        }
    }

    SDL_DestroyTexture(logoTexture);
}

void MenuWindow::MainMenu(std::string& name, std::string& password, bool& launchGame) {
    std::vector<std::string> options = { "- Join Game", "- Settings", "- View Score", "- Exit" };

    while (m_running) {
        int selection = HandleMenuSelection(options);

        switch (selection) {
        case 0:
            JoinGame(name, password);
            m_running = false;
            return;
        case 1:
            SettingsMenu(name, password);
            break;
        case 2:
            ViewScore(name);
            break;
        case 3:
            m_running = false;
            launchGame = false;
            return;
        default:
            break;
        }
    }
}

void MenuWindow::ViewScore(const std::string& name) {
    // Send GET request to retrieve the high score and current score for the user
    auto response = cpr::Get(cpr::Url{ "http://localhost:18080/highScore?name=" + name });

    int highScore = 0;      // Default value for high score
    int score = 0;   // Default value for current score

    if (response.status_code == 200) {
        // Parse the scores from the response
        auto jsonResponse = crow::json::load(response.text);
        highScore = jsonResponse["highScore"].i();
        score = jsonResponse["score"].i();
    }

    SDL_Event event;
    while (true) {
        SDL_RenderCopy(m_renderer, m_background, nullptr, nullptr);

        // Display the high score and current score
        RenderText("Your Score: " + std::to_string(score), m_width / 2 - 100, 150, SDL_Color{ 255, 255, 255, 255 });
        RenderText("Your High Score: " + std::to_string(highScore), m_width / 2 - 100, 100, SDL_Color{ 255, 255, 255, 255 });
        RenderText("Press Enter to go back", m_width / 2 - 100, 200, SDL_Color{ 255, 255, 255, 255 });

        SDL_RenderPresent(m_renderer);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                m_running = false;
                return;
            }
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN) {
                return;
            }
        }
    }
}

void MenuWindow::SettingsMenu(std::string& name, std::string& password) {
    std::vector<std::string> options = { "- Change Name", "- Change Password", "- Change Difficulty", "- View Difficulty", "- Back" };
    int selected = 0;
    SDL_Event event;

    while (m_running) {
        SDL_RenderCopy(m_renderer, m_background, NULL, NULL);

        // Render the menu options
        for (int i = 0; i < options.size(); ++i) {
            SDL_Color color = (i == selected) ? SDL_Color{ 255, 51, 204, 0 } : SDL_Color{ 255, 255, 255, 255 };
            RenderText(options[i], m_width / 2 - 100, 100 + i * 50, color);
        }

        SDL_RenderPresent(m_renderer);

        // Event handling
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                m_running = false;
                return;
            }

            // Handle menu navigation
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                case SDLK_UP:
                    selected = (selected - 1 + options.size()) % options.size();
                    break;
                case SDLK_DOWN:
                    selected = (selected + 1) % options.size();
                    break;
                case SDLK_RETURN:
                    if (selected == 0) {
                        ChangeNameScreen(name);
                    }
                    else if (selected == 1) {
                        ChangePasswordScreen(password);
                    }
                    else if (selected == 2) {
                        ChangeDifficulty();
                    }
                    else if (selected == 3) {
                        ViewDifficulty();
                    }
                    else if (selected == 4) {
                        return; // Back to main menu
                    }
                    break;
                }
            }
        }
    }
}

void MenuWindow::ChangeNameScreen(std::string& name) {
    SDL_Event event;
    std::string inputBuffer;

    while (true) {
        SDL_RenderCopy(m_renderer, m_background, nullptr, nullptr);

        // Render instructions and input field
        RenderText("Enter new name:", m_width / 2 - 100, 100, SDL_Color{ 255, 255, 255, 255 });
        RenderText(inputBuffer, m_width / 2 - 100, 150, SDL_Color{ 255, 255, 255, 255 });

        SDL_RenderPresent(m_renderer);

        // Event handling
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                m_running = false;
                return;
            }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_RETURN) {
                    name = inputBuffer; // Set the new name
                    return;
                }
                else if (event.key.keysym.sym == SDLK_BACKSPACE && !inputBuffer.empty()) {
                    inputBuffer.pop_back();
                }
            }
            else if (event.type == SDL_TEXTINPUT) {
                inputBuffer += event.text.text; // Append character to input buffer
            }
        }
        SDL_StartTextInput();
    }
    SDL_StopTextInput();
}

void MenuWindow::ChangePasswordScreen(std::string& password) {
    SDL_Event event;
    std::string inputBuffer;

    while (true) {
        SDL_RenderCopy(m_renderer, m_background, nullptr, nullptr);

        // Render instructions
        RenderText("Enter new password:", m_width / 2 - 100, 100, SDL_Color{ 255, 255, 255, 255 });

        // Create masked version of the input for display
        std::string maskedInput(inputBuffer.length(), '*');
        RenderText(maskedInput, m_width / 2 - 100, 150, SDL_Color{ 255, 255, 255, 255 });

        SDL_RenderPresent(m_renderer);

        // Event handling
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                m_running = false;
                SDL_StopTextInput();
                return;
            }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_RETURN) {
                    password = inputBuffer; // Set the new password
                    SDL_StopTextInput();
                    return;
                }
                else if (event.key.keysym.sym == SDLK_BACKSPACE && !inputBuffer.empty()) {
                    inputBuffer.pop_back(); // Remove last character
                }
            }
            else if (event.type == SDL_TEXTINPUT) {
                inputBuffer += event.text.text; // Add character to password buffer
            }
        }
        SDL_StartTextInput();
    }
    SDL_StopTextInput();
}

void MenuWindow::ViewDifficulty() {
    auto response = cpr::Get(cpr::Url{ "http://localhost:18080/getDifficulty" });
    if (response.status_code == 200) {
        int currentDifficulty = std::stoi(response.text);
        std::string difficultyName;
        switch (currentDifficulty) {
        case 1: difficultyName = "Easy"; break;
        case 2: difficultyName = "Normal"; break;
        case 3: difficultyName = "Hard"; break;
        case 4: difficultyName = "Very Hard"; break;
        default: difficultyName = "Unknown"; break;
        }

        SDL_Event event;
        while (true) {
            SDL_RenderCopy(m_renderer, m_background, nullptr, nullptr);

            RenderText("Current Difficulty: " + difficultyName, m_width / 2 - 100, 100, SDL_Color{ 255, 255, 255, 255 });
            RenderText("Press Enter to go back", m_width / 2 - 100, 150, SDL_Color{ 255, 255, 255, 255 });

            SDL_RenderPresent(m_renderer);

            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    m_running = false;
                    return;
                }
                if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN) {
                    return;
                }
            }
        }
    }
}

void MenuWindow::ChangeDifficulty() {
    std::vector<std::string> difficultyOptions = { "Easy", "Normal", "Hard", "Very Hard" };
    int selectedDifficulty = 0;
    SDL_Event event;

    while (m_running) {
        SDL_RenderCopy(m_renderer, m_background, nullptr, nullptr);

        // Render the difficulty options
        for (int i = 0; i < difficultyOptions.size(); ++i) {
            SDL_Color color = (i == selectedDifficulty) ? SDL_Color{ 255, 51, 204, 0 } : SDL_Color{ 255, 255, 255, 255 };
            RenderText(difficultyOptions[i], m_width / 2 - 100, 100 + i * 50, color);
        }

        SDL_RenderPresent(m_renderer);

        // Event handling for difficulty selection
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                m_running = false;
                return;
            }

            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                case SDLK_UP:
                    selectedDifficulty = (selectedDifficulty - 1 + difficultyOptions.size()) % difficultyOptions.size();
                    break;
                case SDLK_DOWN:
                    selectedDifficulty = (selectedDifficulty + 1) % difficultyOptions.size();
                    break;
                case SDLK_RETURN:
                    // Change difficulty on the server
                    auto response = cpr::Post(cpr::Url{ "http://localhost:18080/changeDifficulty/" + std::to_string(selectedDifficulty + 1) });
                    if (response.status_code == 200) {
                        std::cout << "Difficulty changed successfully!" << std::endl;
                    }
                    else {
                        std::cerr << "Failed to change difficulty." << std::endl;
                    }
                    return; // Return to settings menu
                }
            }
        }
    }
}

void MenuWindow::JoinGame(std::string& name, std::string& password)
{
    // Clear any previous error messages
    m_errorMessage.clear();

    // Join the game
    crow::json::wvalue joinRequest; // Grouped the requests together in a structured JSON payload
    joinRequest["playerName"] = name;
    joinRequest["password"] = password;

    auto response1 = cpr::Post(
        cpr::Url{ "http://localhost:18080/join" },
        cpr::Body{ joinRequest.dump() },
        cpr::Header{ {"Content-Type", "application/json"} }
    );
    if (response1.status_code != 200) {
        auto jsonResponse = crow::json::load(response1.text);

        if (jsonResponse) {
            if (jsonResponse.has("error")) {
                m_errorMessage = jsonResponse["error"].s();
            }
            else {
                m_errorMessage = "Unknown error occurred.";
            }
        }
        else {
            m_errorMessage = "Failed to parse JSON response.";
        }

        std::cerr << "Error: " << m_errorMessage << std::endl;
        return; // exit if joining fails
    }

    // if join was successful
    auto jsonResponse = crow::json::load(response1.text);
    if (jsonResponse.has("welcomeMessage")) {
        std::cout << jsonResponse["welcomeMessage"].s() << std::endl;
    }

    m_playerId = jsonResponse["playerId"].i();
    m_startScreenActive = true;
}

void MenuWindow::CleanUp() {
    if (m_background) {
        SDL_DestroyTexture(m_background);
        m_background = nullptr;
    }
    if (m_renderer) {
        SDL_DestroyRenderer(m_renderer);
        m_renderer = nullptr;
    }
    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
    if (m_font) {
        TTF_CloseFont(m_font);
        m_font = nullptr;
    }
}
