#pragma once

#include <conio.h>
#include <iostream>
#include <string>
#include <cpr/cpr.h>
#include <list>
#include <chrono>
#include <thread>

import Direction;
import Player;

using boardElements::Player;

class Tank :
	public Player
{
private:
	// Member Variables
	int m_coordX;
	int m_coordY;
	bool m_isAlive;
	double m_speed;
	double m_lastMoveTime;
	Direction m_direction;
	double m_cooldown = 4.0;
	std::chrono::steady_clock::time_point m_lastShootTime;
	std::chrono::steady_clock::time_point m_lastUpdateTime;

public:
	// Constructors and Destructor
	Tank(uint8_t id, std::string name, std::string password, int highScore, uint8_t remainingLives, int score);
	Tank(uint8_t id, std::string name, std::string password, int highScore, uint8_t remainingLives, int score, int coordX, int coordY, double startSpeed, bool isAlive);
	~Tank() = default;

	// Game Logic
	void Destroy();
	bool CanShoot();

	// State Management
	void UpdatePosition();

	// Getters
	double GetCoordX() const;
	double GetCoordY() const;
	double GetSpeed() const;
	Direction GetDirection() const;

	// Setters
	void SetCoordX(const double& coordX);
	void SetCoordY(const double& coordY);
	void SetDirection(const Direction& direction);
	void SetLastShootTime(const std::chrono::steady_clock::time_point& time);
	void SetSpeed(double speed);
};