module Player;

using namespace boardElements;

Player::Player(uint8_t id, std::string name, std::string password, int highScore, uint8_t remainingLives, int score)
	:
	m_id{ id },
	m_name{ name },
	m_password{ password },
	m_highScore{ highScore },
	m_remainingLives{ remainingLives },
	m_score{ score }
{}

int Player::GetScore() const {
	return m_score;
}

void Player::SetScore(const int& score) {
	m_score = score;
}

uint8_t Player::GetId() const {
	return m_id;
}

uint8_t Player::GetRemainingLives() const {
	return m_remainingLives;
}

int Player::GetHighScore() const {
	return m_highScore;
}

std::string Player::GetName() const {
	return m_name;
}

std::string Player::GetPassword() const {
	return m_password;
}

void Player::SetHighScore(const int& highScore) {
	m_highScore = highScore;
}

void Player::SetName(const std::string& name) {
	m_name = name;
}

void Player::SetRemainingLives(const uint8_t& remainingLives) {
	m_remainingLives = remainingLives;
}

void Player::SetPassword(const std::string& password) {
	m_password = password;
}

void Player::GetAnElimination() {
	m_score++;
	if (m_score > m_highScore) m_highScore = m_score;
}