#pragma once
#include <iostream>
#include <chrono>
#include <mutex>
import Direction;
#include "Tank.h"

class Bullet {
private:
    // Member Variables
    double m_coordX;
    double m_coordY;
    std::chrono::steady_clock::time_point m_creationTime;
    Direction m_bulletDirection;
    bool m_isActive;
    double m_speed;
    std::mutex m_bulletMutex;
    Tank m_tank;
public:
    // Constructors and Destructor
    Bullet(double x, double y, Direction direction, const Tank& tank);
    Bullet() = default;
    ~Bullet() = default;

    // Public Methods
    void Destroy();
    bool IsActive() const;

    // Getters
    double GetX() const;
    double GetY() const;
    Direction GetDirection() const;
    double GetSpeed() const;
    Tank GetTank() const;

    // Setters
    void SetX(const double& x);
    void SetY(const double& y);
    void SetDirection(const Direction& dir);

    // Mutex Handling
    void LockMutex();
};