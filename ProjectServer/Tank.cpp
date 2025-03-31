#include "Tank.h"

Tank::Tank(uint8_t id, std::string name, std::string password, int highScore, uint8_t remainingLives, int score)
	:
	Player( id,name,password,highScore,remainingLives,score ),
	m_coordX(0),
	m_coordY(0),
	m_isAlive(true),
	m_speed(0.0),
	m_lastUpdateTime(std::chrono::steady_clock::now()),
	m_lastShootTime(std::chrono::steady_clock::now() - std::chrono::seconds(4))
{}

Tank::Tank(uint8_t id, std::string name, std::string password, int highScore, uint8_t remainingLives, int score, int coordX, int coordY, double startSpeed, bool isAlive)
	: 
	Player(id, name, password, highScore, remainingLives, score),
	m_coordX(coordX),
	m_coordY(coordY),
	m_speed(startSpeed),
	m_isAlive(isAlive),
	m_lastUpdateTime(std::chrono::steady_clock::now()),
	m_lastShootTime(std::chrono::steady_clock::now() - std::chrono::seconds(4))
{}


void Tank::Destroy() {
	m_isAlive = false;
}

bool Tank::CanShoot() {
	auto currentTime = std::chrono::steady_clock::now();
	std::chrono::duration<double> elapsedSeconds = currentTime - m_lastShootTime;
	return elapsedSeconds.count() >= m_cooldown;
}

double Tank::GetCoordX() const {
	return m_coordX;
}

double Tank::GetCoordY() const {
	return m_coordY;
}

double Tank::GetSpeed() const {
	return m_speed;
}

Direction Tank::GetDirection() const {
	return m_direction;
}

void Tank::SetCoordX(const double& coordX) {
	m_coordX = coordX;
}

void Tank::SetCoordY(const double& coordY) {
	m_coordY = coordY;
}

void Tank::SetDirection(const Direction& direction)
{
	m_direction = direction;
}

void Tank::SetLastShootTime(const std::chrono::steady_clock::time_point& time)
{
	m_lastShootTime = time;
}

void Tank::SetSpeed(double speed) {
	m_speed = speed;
}