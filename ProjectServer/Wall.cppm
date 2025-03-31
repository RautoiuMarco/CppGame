#pragma once
export module Wall;

#include <vector>

namespace boardElements {
	export class Wall {
	private:
		// Member Variables
		int m_coordX, m_coordY, m_type;

	public:
		// Constructor
		Wall(int coordX, int coordY, int type);

		// Getters
		int GetCoordX() const;
		int GetCoordY() const;
		int GetType() const;

		// Methods
		bool isDestructible() const;
		bool IsBomb() const;

		// Setters
		void SetCoordX(int x);
		void SetCoordY(int y);
		void SetType(int type);

		// Game Logic
		std::vector<std::pair<int, int>> TriggerExplosion() const;
	};
}