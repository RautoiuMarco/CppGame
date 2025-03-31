#include "Bullet.h";

Bullet::Bullet(double x, double y, Direction direction, const Tank& tank)
    :
    m_coordX(x),
    m_coordY(y),
    m_bulletDirection(direction),
    m_speed(0.25),
    m_isActive(true),
    m_creationTime(std::chrono::steady_clock::now()),
    m_tank{tank}
{
    switch(direction)
    {
    case Direction::UP:
        m_coordY -= 1.0;
    case Direction::DOWN:
        m_coordY += 1.0;
    case Direction::LEFT:
        m_coordX -= 1.0;
    case Direction::RIGHT:
        m_coordX += 1.0;
    }
}

void Bullet::Destroy() {
    if (!m_isActive) return;
    m_isActive = false;
    m_speed = 0;
}

double Bullet::GetX() const {
    return m_coordX;
}

double Bullet::GetY() const {
    return m_coordY;
}

Direction Bullet::GetDirection() const
{
    return m_bulletDirection;
}

double Bullet::GetSpeed() const
{
    return m_speed;
}

Tank Bullet::GetTank() const
{
    return m_tank;
}

void Bullet::SetX(const double& x)
{
    m_coordX = x;
}

void Bullet::SetY(const double& y)
{
    m_coordY = y;
}

void Bullet::SetDirection(const Direction& dir)
{
    m_bulletDirection = dir;
}

bool Bullet::IsActive() const {
    return m_isActive;
}

void Bullet::LockMutex()
{
    std::lock_guard<std::mutex> lock(m_bulletMutex);
}
