#pragma once
export module Player;

#include <cstdint>
#include <string>
#include <cctype>

namespace boardElements {
	export class Player
    {
    public:
        // Member Variables
        uint8_t m_id;
        int m_highScore;
        int m_score;
        std::string m_name;
        uint8_t m_remainingLives;
        std::string m_password;

    public:
        // Constructors and Destructor
        Player(uint8_t id, std::string name, std::string password, int highScore, uint8_t remainingLives, int score);
        Player() = default;
        ~Player() = default;

        // Getters
        int GetScore() const;
        uint8_t GetId() const;
        uint8_t GetRemainingLives() const;
        int GetHighScore() const;
        std::string GetName() const;
        std::string GetPassword() const;

        // Setters
        void SetScore(const int& score);
        void SetHighScore(const int& highScore);
        void SetName(const std::string& name);
        void SetRemainingLives(const uint8_t& remainingLives);
        void SetPassword(const std::string& password);

        // Game Logic
        void GetAnElimination();
    };
}