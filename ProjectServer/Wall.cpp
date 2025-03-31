module Wall;

using boardElements::Wall;

Wall::Wall(int x, int y, int type)
	: m_coordX(x), m_coordY(y), m_type(type) {}

int Wall::GetCoordX() const {
	return m_coordX;
}
int Wall::GetCoordY() const {
	return m_coordY;
}
int Wall::GetType() const {
	return m_type;
}

bool Wall::isDestructible() const {
	return m_type == 1 || m_type == 3;
}

bool Wall::IsBomb() const {
	return m_type == 3;
}

void Wall::SetCoordX(int x) {
	this->m_coordX = x;
}

void Wall::SetCoordY(int y) {
	this->m_coordY = y;
}

void Wall::SetType(int type) {
	this->m_type = type;
}

std::vector<std::pair<int, int>> Wall::TriggerExplosion() const {
	std::vector<std::pair<int, int>> affectedArea;

	for (int dx = -1; dx <= 1; ++dx) {
		for (int dy = -1; dy <= 1; ++dy) {
			int affectedX = m_coordX + dx;
			int affectedY = m_coordY + dy;
			affectedArea.emplace_back(affectedX, affectedY);
		}
	}

	return affectedArea;
}