#include "Board.h"

Board::Board(int h, int w, int d)
	:m_height(h),
	m_width(w),
	m_difficulty(d),
	m_numberOfPlayers(0)
{
	m_board.resize(h, std::vector<space>(w, { 0, false })); //start position value set to 0 by default
}

int Board::GetHeight() const {
	return m_height;
}

int Board::GetWidth() const {
	return m_width;
}

int Board::GetDifficulty() const
{
	return m_difficulty;
}

uint8_t Board::GetNumberOfPlayers() const
{
	return m_numberOfPlayers;
}

std::list<std::shared_ptr<Bullet>> Board::GetBullets() const
{
	return allBullets;
}

void Board::SetHeight() {
	m_height = (rand() % 11) + 20;
}

void Board::SetWidth() {
	m_width = (rand() % 11) + 20;
}

void Board::SetDifficulty() {
	int x;
	std::cout << "Choose difficulty:\n1 - Easy\n2 - Normal\n3 - Hard\n4 - Very Hard\n\n";
	std::cin >> x;


	if (x < 1 || x > 4) {
		std::cout << ("Error: Invalid difficulty level!\nDifficulty will be set to default.");
		x = 1;
	}

	m_difficulty = x;
	system("CLS");
}

void Board::SetDifficultyAsValue(int x) {
	if (x < 1 || x > 4) {
		m_difficulty = 0;
	}
	else {
		m_difficulty = x;
	}
}

crow::json::wvalue Board::GetPlayerState() {
	crow::json::wvalue boardJson;
	crow::json::wvalue::list playersJson;

	for (const Tank& player : m_players) {
		crow::json::wvalue playerJson;
		playerJson["id"] = player.GetId();
		playerJson["name"] = player.GetName();
		playerJson["x"] = player.GetCoordX();
		playerJson["y"] = player.GetCoordY();
		playersJson.push_back(std::move(playerJson));
	}

	boardJson["players"] = std::move(playersJson);
	return boardJson;
}

crow::json::wvalue Board::GetBoardState()
{
	crow::json::wvalue::list boardJson;
	crow::json::wvalue::list rowJson;
	int numRows = m_board.size();
	int numCols = m_board[0].size();

	// Print top border
	for (int col = 0; col < numCols + 2; col++) {
		rowJson.push_back('#');
	}
	boardJson.push_back(std::move(rowJson));
	rowJson.clear();

	// Print board with left and right borders
	for (int i = 0; i < numRows; i++) {
		rowJson.push_back('#');  // Left border

		for (int j = 0; j < numCols; j++) {
			for (auto& player : m_players) {
				if (player.GetCoordX() == i && player.GetCoordY() == j) {
					rowJson.push_back('P');  // Mark the player's position
					j++;
				}
			}

			switch (m_board[i][j].first) {
			case 0: {
				rowJson.push_back(' ');
				break;
			}
			case 1: {
				rowJson.push_back('+');// Breakable walls
				break;
			}
			case 2: {
				rowJson.push_back('#');  // Unbreakable walls
				break;
			}
			default: {
				rowJson.push_back(' ');
				break;
			}
			}

		}
		rowJson.push_back('#');
		boardJson.push_back(std::move(rowJson));
		rowJson.clear();
	}

	// Print bottom border
	for (int col = 0; col < numCols + 2; col++) {
		rowJson.push_back('#');
	}
	boardJson.push_back(std::move(rowJson));
	crow::json::wvalue matrix;
	matrix["board"] = std::move(boardJson);
	return matrix;
}

std::optional<Tank> Board::GetPlayerBasedOnCoord(int x, int y)
{
	auto it = std::ranges::find_if(m_players, [x, y](const Tank& player) {
		return player.GetCoordX() == x && player.GetCoordY() == y;
		});

	if (it != m_players.end()) {
		return *it;
	}
	return std::nullopt;
}

void Board::RespawnPlayer(Tank& player)
{
	int respawnPosition = rand() % 4;
	switch (respawnPosition) {
	case 0:
		Respawn(1, 1, player);
		break;
	case 1:
		Respawn(1, m_width - 2, player);
		break;
	case 2:
		Respawn(m_height - 2, 1, player);
		break;
	case 3:
		Respawn(m_height - 2, m_width - 2, player);
		break;
	default:
		throw std::invalid_argument("Invalid respawnPosition value");
	}
}

void Board::Respawn(int x, int y, Tank& player)
{
	player.SetCoordX(x);
	player.SetCoordY(y);
	m_board[x][y].first = 0;
	ClearSurroundings(x, y);
}

//helper function to modify how the board will be generated based off the selected difficulty
void Board::SetPercentages(int& zeroPercent, int& onePercent, int& twoPercent, int& bombPercent) const {
	switch (m_difficulty) {
	case 1: // easy
		zeroPercent = 60;
		onePercent = 20;
		twoPercent = 15;
		bombPercent = 5;
		break;
	case 2: // normal
		zeroPercent = 55;
		onePercent = 20;
		twoPercent = 25;
		bombPercent = 10;
		break;
	case 3: // hard
		zeroPercent = 50;
		onePercent = 25;
		twoPercent = 15;
		bombPercent = 10;
		break;
	case 4: //very hard
		zeroPercent = 40;
		onePercent = 30;
		twoPercent = 20;
		bombPercent = 10;
		break;
	default:
		throw std::runtime_error("Error: Difficulty not set correctly!");
	}
}

void Board::GenerateBoard() {
	srand(time(0));

	int zeroPercent, onePercent, twoPercent, bombPercent;
	SetPercentages(zeroPercent, onePercent, twoPercent, bombPercent);

	int maxBombs = 3 + m_difficulty;
	int currentBombs = 0;

	m_walls.clear();
	for (int i = 0; i < m_height; ++i) {
		for (int j = 0; j < m_width; ++j) {
			int random_value = std::rand() % 100; // Generates a random number between 0 and 99
			if (random_value < zeroPercent) {
				m_board[i][j].first = 0; // Zeroes
			}
			else if (random_value < zeroPercent + onePercent) {
				m_board[i][j].first = 1; // Ones
				m_walls.emplace_back(j, i, 1);
			}
			else if (random_value < zeroPercent + onePercent + twoPercent) {
				m_board[i][j].first = 2; // Twos
				m_walls.emplace_back(j, i, 2);
			}
			else if (currentBombs < maxBombs) {
				m_board[i][j].first = 3;
				m_walls.emplace_back(j, i, 3);
				currentBombs++;
			}
			else {
				m_board[i][j].first = 0;
			}

			m_board[i][j].second = false;
		}
	}

	int numRows = m_board.size();
	int numCols = m_board[0].size();
	if (numRows < numCols) {
		for (int i = 3; i <= numRows; i++)
			FixSquaring(numRows - 1);
	}
	else {
		for (int i = 3; i <= numCols; i++)
			FixSquaring(numCols - 1);
	}

	FixRowsAndColumns();
}

void Board::FixSquaring(int k)
{
	int numRows = m_board.size();
	int numCols = m_board[0].size();

	for (int i = 0; i <= numRows; i++) {
		for (int j = 0; j <= numCols; j++) {
			bool isSquare = true;

			// Check if all elements in the current k x k submatrix are 2
			for (int x = i; x < k; ++x) {
				for (int y = j; y < k; ++y) {
					if (m_board[x][y].first != 2) {
						isSquare = false;
						break;
					}
				}
				if (!isSquare) break;
			}

			// If a square is found, fix it by changing one element to 0
			if (isSquare) {
				m_board[i][j].first = 0; // Change the top-left element of the square
				return; // Fix only one circling for simplicity
			}
		}
	}
}

void Board::GenerateWalls() {
	for (int i = 0; i < m_height; ++i) {
		for (int j = 0; j < m_width; ++j) {
			if (m_board[i][j].first == 1 || m_board[i][j].first == 2 || m_board[i][j].first == 3) {
				Wall wall(j, i, m_board[i][j].first);
				m_walls.push_back(wall);
			}
		}
	}
}

void Board::FixRowsAndColumns() {
	for (auto& row : m_board) {
		bool allTwos = std::all_of(row.begin(), row.end(), [](const std::pair<int, bool>& cell) {
			return cell.first == 2; 
			});

		if (allTwos) {
			std::transform(row.begin(), row.begin() + row.size() / 2, row.begin(), [](std::pair<int, bool> cell) {
				cell.first = 0; 
				return cell;
				});
		}
	}

	for (int col = 0; col < m_width; ++col) {
		bool allTwos = true;
		for (int row = 0; row < m_height; ++row) {
			if (m_board[row][col].first != 2) {
				allTwos = false;
				break;
			}
		}

		if (allTwos) {
			for (int row = 0; row < m_height / 2; ++row) {
				m_board[row][col].first = 0; 
			}
		}
	};
}

// Helper function to set all spaces surrounding the start positions to 0
void Board::ClearSurroundings(int i, int j) {
	// Define the relative positions around (i, j)
	std::vector<std::pair<int, int>> surroundingOffsets = {
		{-1, -1}, {-1, 0}, {-1, 1},
		{ 0, -1},          { 0, 1},
		{ 1, -1}, { 1, 0}, { 1, 1}
	};

	for (const auto& offset : surroundingOffsets) {
		int ni = i + offset.first;
		int nj = j + offset.second;

		// Check if within bounds
		if (ni >= 0 && ni < m_height && nj >= 0 && nj < m_width) {
			m_board[ni][nj].first = 0; // Set to path
		}
	}
}


Tank Board::GetPlayer(int playerNumber) const {
	return m_players[playerNumber];
}

int Board::GetValue(int x, int y) {
	return m_board[x][y].first;
}

void Board::InsertPlayer(const Tank& player)
{
	m_players.push_back(player);
	m_numberOfPlayers++;
	std::vector<std::pair<int, int>> diagonalOffsets = { {1, 1} }; // Start positions will be the corners, but omitting the walls on the edges

	for (const auto& offset : diagonalOffsets) {
		switch (m_numberOfPlayers) {
		case 1: // Top-left corner
			InsertPlayer1(offset.first, offset.second);
			break;

		case 2: // Top-right corner
			InsertPlayer2(offset.first, m_width - 1 - offset.second);
			break;

		case 3: // Bottom-left corner
			InsertPlayer3(m_height - 1 - offset.first, offset.second);
			break;

		case 4: // Bottom-right corner
			InsertPlayer4(m_height - 1 - offset.first, m_width - 1 - offset.second);
			break;

		default:
			throw std::invalid_argument("Invalid number of players");
		}
	}

}

void Board::InsertPlayer1(int i, int j) {
	if (i < m_height && j < m_width) {
		m_board[i][j].first = 0;
		m_board[i][j].second = true;
		ClearSurroundings(i, j);
		m_players[0].SetCoordX(i);
		m_players[0].SetCoordY(j);
	}
}

void Board::InsertPlayer2(int i, int j) {
	if (i < m_height && j >= 0) {
		m_board[i][j].second = true;
		m_board[i][j].first = 0;
		ClearSurroundings(i, j);
		m_players[1].SetCoordX(i);
		m_players[1].SetCoordY(j);
	}
}

void Board::InsertPlayer3(int i, int j) {
	if (i >= 0 && j < m_width) {
		m_board[i][j].second = true;
		m_board[i][j].first = 0;
		ClearSurroundings(i, j);
		m_players[2].SetCoordX(i);
		m_players[2].SetCoordY(j);
	}
}

void Board::InsertPlayer4(int i, int j) {
	if (i >= 0 && j >= 0) {
		m_board[i][j].second = true;
		m_board[i][j].first = 0;
		ClearSurroundings(i, j);
		m_players[3].SetCoordX(i);
		m_players[3].SetCoordY(j);
	}
}

bool Board::VerifyIfCoordIsPlayer(int x, int y) {
	auto it = std::find_if(m_players.begin(), m_players.end(), [x, y](const Tank& player) {
		return player.GetCoordX() == x && player.GetCoordY() == y;
		});

	return it != m_players.end(); 
}

void Board::TriggerBomb(double x, double y) {
	const int radius = 10;
	for (int i = x - radius; i <= x + radius; ++i) {
		for (int j = y - radius; j <= y + radius; ++j) {
			if (i >= 0 && i < m_height && j >= 0 && j < m_width) {
				if (abs(x - i) + abs(y - j) <= radius) {
					if (m_board[i][j].first == 1) {
						m_board[i][j].first = 0;
					}
					for (auto it = m_players.begin(); it != m_players.end();) {
						if (it->GetCoordX() == i && it->GetCoordY() == j) {
							it = m_players.erase(it);
						}
						else {
							++it;
						}
					}
				}
			}
		}
	}
}

void Board::Update(double deltaTime, Bullet& bullet)
{
	if (bullet.IsActive()) {

		switch (bullet.GetDirection()) {
		case Direction::UP:
			bullet.SetY(bullet.GetY() - bullet.GetSpeed() * deltaTime);
			break;
		case Direction::DOWN:
			bullet.SetY(bullet.GetY() + bullet.GetSpeed() * deltaTime);
			break;
		case Direction::LEFT:
			bullet.SetX(bullet.GetX() - bullet.GetSpeed() * deltaTime);
			break;
		case Direction::RIGHT:
			bullet.SetX(bullet.GetX() + bullet.GetSpeed() * deltaTime);
			break;
		}

		for (auto& currentBullet : allBullets) {
			if (currentBullet.get() != &bullet && currentBullet->IsActive() &&
				currentBullet->GetX() == bullet.GetX() && currentBullet->GetY() == bullet.GetY()) {
				currentBullet->Destroy();
				bullet.Destroy();
				return;
			}
		}

		int spaceType = GetSpaceType(bullet.GetY() - 1, bullet.GetX() - 1);
		if (spaceType == 2) {
			bullet.Destroy();
		}
		else if (spaceType == 1) {
			SetSpaceType(bullet.GetY() - 1, bullet.GetX() - 1, 0);
			bullet.Destroy();

			if (m_board[bullet.GetY() - 1][bullet.GetX() - 1].first == 3) {
				TriggerBomb(bullet.GetY() - 1, bullet.GetX() - 1);
			}
		}
		if (VerifyIfCoordIsPlayer(bullet.GetY() - 1, bullet.GetX() - 1)) {
			auto player = GetPlayerBasedOnCoord(bullet.GetY() - 1, bullet.GetX() - 1);
			if (player) {
				RespawnPlayer(*player);
			}
			bullet.GetTank().GetAnElimination();
			bullet.Destroy();
		}
	}
}

void Board::Shoot(int playerId) {
	if (!m_players[playerId].CanShoot()) return;

	m_players[playerId].SetLastShootTime(std::chrono::steady_clock::now());

	// Use shared_ptr for safe memory management
	auto bullet = std::make_shared<Bullet>(
		m_players[playerId].GetCoordY() + 1,
		m_players[playerId].GetCoordX() + 1,
		m_players[playerId].GetDirection(),
		m_players[playerId]
	);

	bullet->LockMutex();
	allBullets.push_back(bullet);

	std::thread bulletThread([this, bullet]() {
		while (bullet->GetY() < m_height && bullet->GetY() >= 0 &&
			bullet->GetX() >= 0 && bullet->GetX() < m_width &&
			bullet->IsActive()) {
			Update(1, *bullet);
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}

		// Deactivate the bullet and remove it safely
		bullet->Destroy();
		bullet->LockMutex();
		allBullets.remove(bullet); // Remove from the container
		});

	bulletThread.detach(); // Let the thread run independently
}
void Board::Move(int playerId, const char& key) {
	if (key == 'W' || key == 'w')m_players[playerId].SetDirection(Direction::UP);
	if (key == 'S' || key == 's')m_players[playerId].SetDirection(Direction::DOWN);
	if (key == 'A' || key == 'a')m_players[playerId].SetDirection(Direction::LEFT);
	if (key == 'D' || key == 'd')m_players[playerId].SetDirection(Direction::RIGHT);

	switch (m_players[playerId].GetDirection()) {
	case Direction::UP:
		if (m_players[playerId].GetCoordX() - 1 >= 0)
			if (GetValue(m_players[playerId].GetCoordX() - 1, m_players[playerId].GetCoordY()) == 0)
				m_players[playerId].SetCoordX(m_players[playerId].GetCoordX() - 1);
		break;
	case Direction::DOWN:
		if (m_players[playerId].GetCoordX() + 1 < GetHeight())
			if (GetValue(m_players[playerId].GetCoordX() + 1, m_players[playerId].GetCoordY()) == 0)
				m_players[playerId].SetCoordX(m_players[playerId].GetCoordX() + 1);
		break;
	case Direction::LEFT:
		if (m_players[playerId].GetCoordY() - 1 >= 0)
			if (GetValue(m_players[playerId].GetCoordX(), m_players[playerId].GetCoordY() - 1) == 0)
				m_players[playerId].SetCoordY(m_players[playerId].GetCoordY() - 1);
		break;
	case Direction::RIGHT:
		if (m_players[playerId].GetCoordY() + 1 < GetWidth())
			if (GetValue(m_players[playerId].GetCoordX(), m_players[playerId].GetCoordY() + 1) == 0)
				m_players[playerId].SetCoordY(m_players[playerId].GetCoordY() + 1);
		break;
	}
}

bool Board::VerifyBulletCoord(int x, int y) const
{
	return std::ranges::any_of(allBullets, [x, y](const auto& bullet) {
		return bullet->GetX() == x && bullet->GetY() == y;
		});
}

int Board::GetSpaceType(double x, double y) const {
	if (x >= 0 && x < m_height && y >= 0 && y < m_width) {
		return m_board[x][y].first;
	}
	return -1;
}

void Board::SetSpaceType(double x, double y, int type) {
	if (x >= 0 && x < m_height && y >= 0 && y < m_width) {
		m_board[x][y].first = type;
	}
}

std::vector<std::vector<std::pair<int, bool>>> Board::GetBoard() const
{
	return m_board;
}