#pragma once

#include <iostream>
#include <vector>
#include <crow.h>
#include <stdexcept>
#include <algorithm>
#include "Bullet.h";

import Wall;

using space = std::pair<int, bool>; 
// int = type of space on the board, bool = determines whether it is a start position
using boardElements::Wall;

class Board
{
protected:
    // Member Variables
    int m_height;
    int m_width;
    int m_difficulty = 1;
    std::vector<std::vector<space>> m_board;
    std::vector<Tank> m_players;
    std::vector<Wall> m_walls;
    int m_numberOfPlayers;
    std::list<std::shared_ptr<Bullet>> allBullets;

public:
    // Constructor and Destructor
    Board(int h, int w, int d);
    ~Board() = default;

    // Getters
    int GetHeight() const;
    int GetWidth() const;
    int GetDifficulty() const;
    uint8_t GetNumberOfPlayers() const;
    std::list<std::shared_ptr<Bullet>> GetBullets() const;
    Tank GetPlayer(int playerNumber) const;
    int GetValue(int x, int y);
    std::vector<std::vector<std::pair<int, bool>>> GetBoard() const;

    // Setters
    void SetHeight();
    void SetWidth();
    void SetDifficultyAsValue(int x);
    void SetDifficulty(); // difficulty setter with menu

    // Serializing
    crow::json::wvalue GetPlayerState();
    crow::json::wvalue GetBoardState();

    // State Management
    void Update(double deltaTime, Bullet& bullet);
    void UpdateBoard(crow::json::rvalue body);

    // Game Mechanics
    void RespawnPlayer(Tank& player);
    void Respawn(int x, int y, Tank& player);
    void Shoot(int playerId);
    void Move(int playerId, const char& key);
    bool VerifyBulletCoord(int x, int y) const;

    // Board Manipulation
    void GenerateBoard();
    void GenerateWalls();
    void RenderWalls();
    void PlaceBomb(int x, int y);
    void TriggerBomb(double x, double y);

    // Board Validation
    bool IsWallAt(int x, int y) const;
    int GetSpaceType(double x, double y) const;
    void SetSpaceType(double x, double y, int type);
    std::optional<Tank> GetPlayerBasedOnCoord(int x, int y);
    bool VerifyIfCoordIsPlayer(int x, int y);

    // Player Insertion
    void InsertPlayer(const Tank& player);

private:
    // Helper Functions
    void ClearSurroundings(int x, int y);
    void SetPercentages(int& zeroPercent, int& onePercent, int& twoPercent, int& bombPercent) const;
    void FixSquaring(int k);
    void FixRowsAndColumns();

    // Helper Functions for Player Insertion
    void InsertPlayer1(int i, int j);
    void InsertPlayer2(int i, int j);
    void InsertPlayer3(int i, int j);
    void InsertPlayer4(int i, int j);
};