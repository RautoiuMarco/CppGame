#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#include <crow.h>
#include <thread>
#include <iostream>
#include <unordered_map>
#include <mutex>
#include <fstream>
#include <atomic>

#include "Board.h"
#include "PlayerDatabase.h"
#include "..\PasswordManager\PasswordManager.h" 

std::atomic<int> gameTimer(0);
std::mutex gameMutex;
using namespace http;
using namespace sql;

int main() {
	crow::SimpleApp app;
	Storage storage = createStorage("players.sqlite");
	storage.sync_schema();
	srand(std::time(0));

	int m = 20, n = 20, d = 1;

	Board b(m, n, d);
	b.SetDifficulty();
	b.GenerateBoard();

	std::thread([&]() {
		while (true) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
			++gameTimer;
		}
		}).detach();

	CROW_ROUTE(app, "/time").methods("GET"_method)([&]() {
		return crow::response(std::to_string(gameTimer.load()));
		});

	CROW_ROUTE(app, "/bulletsCoord").methods("GET"_method)([&b]() {
		crow::json::wvalue jsonResponse;
		crow::json::wvalue::list bulletsList;

		for (const auto bullet : b.GetBullets()) {
			crow::json::wvalue bulletJson;
			bulletJson["coordX"] = (*bullet).GetX();
			bulletJson["coordY"] = (*bullet).GetY();
			bulletsList.push_back(std::move(bulletJson));
		}
		jsonResponse["bullets"] = std::move(bulletsList);
		return crow::response(jsonResponse);
		});

	CROW_ROUTE(app, "/player").methods("POST"_method, "GET"_method)
		([&storage](const crow::request& req) {
		if (req.method == crow::HTTPMethod::POST) {
			auto body = crow::json::load(req.body);
			if (!body) {
				return crow::response(400, "Invalid JSON");
			}

			if (!body.has("name") || !body.has("password")) {
				return crow::response(400, "Missing 'name' or 'password' field");
			}

			std::string name = body["name"].s();
			std::string password = body["password"].s();

			try {
				auto existingPlayer = storage.get_all<Player>(where(c(&Player::m_name) == name));

				if (!existingPlayer.empty()) {
					const auto& player = existingPlayer.front();

					if (player.GetPassword() == password) {
						crow::json::wvalue response;
						response["id"] = player.GetId();
						response["name"] = player.GetName();

						return crow::response(200, response);
					}
					else {
						return crow::response(403, "Incorrect password");
					}
				}

				auto playerId = storage.insert(Player{
					0,
					name,
					password,
					0,
					0,
					0
					});

				auto playerEntry = storage.get<Player>(playerId);

				crow::json::wvalue response;
				response["id"] = playerEntry.GetId();
				response["name"] = playerEntry.GetName();

				return crow::response(200, response);
			}
			catch (const std::exception& e) {
				return crow::response(500, "Failed to process the request: " + std::string(e.what()));
			}
		}
		else if (req.method == crow::HTTPMethod::GET) {
			try {
				auto allPlayers = storage.get_all<Player>();

				// Create a JSON array to hold player data
				crow::json::wvalue response;

				crow::json::wvalue::list playerList;

				for (const auto& player : allPlayers) {
					crow::json::wvalue playerData;
					playerData["id"] = player.GetId();
					playerData["name"] = player.GetName();
					playerList.push_back(std::move(playerData));
				}
				response["players"] = std::move(playerList);
				return crow::response(200, response);
			}
			catch (const std::exception& e) {
				return crow::response(500, "Failed to fetch players: " + std::string(e.what()));
			}
		}

		return crow::response(405, "Method Not Allowed");
			});

	CROW_ROUTE(app, "/join").methods("POST"_method)([&b, &storage](const crow::request& req) {
		std::lock_guard<std::mutex> lock(gameMutex); // Protect the shared state
		auto jsonData = crow::json::load(req.body);

		if (!jsonData || !jsonData.has("playerName") || !jsonData.has("password")) {
			crow::json::wvalue errorResponse;
			errorResponse["error"] = "Invalid request payload";
			std::cerr << "Error: " << errorResponse.dump() << std::endl;
			return crow::response(400, errorResponse.dump());
		}

		std::string playerName = jsonData["playerName"].s();
		std::string playerPassword = jsonData["password"].s();

		auto existingPlayer = storage.get_all<Player>(where(c(&Player::m_name) == playerName));
		if (!existingPlayer.empty()) {
			// Player exists, validate the password
			const auto& player = existingPlayer.front();

			if (player.GetPassword() == playerPassword) {
				crow::json::wvalue response;
				response["message"] = "Player already exists";
				response["playerId"] = player.GetId();
				response["board"] = b.GetPlayerState();
				response["welcomeMessage"] = "Welcome back to the game, " + playerName + "!";
				std::cout << "Joining existing player: " << playerName << " with ID: " << player.GetId() << std::endl;
				return crow::response(response.dump());
			}
			else {
				crow::json::wvalue errorResponse;
				errorResponse["error"] = "Incorrect password";
				std::cout << "Incorrect password for player: " << playerName << std::endl;
				return crow::response(403, errorResponse.dump());
			}
		}

		if (!VerifyPassword(playerPassword)) {
			crow::json::wvalue errorResponse;
			errorResponse["error"] = "Password does not meet security requirements. Please try again.";
			std::cerr << "Error: " << errorResponse.dump() << std::endl;
			return crow::response(400, errorResponse.dump());
		}

		// Insert a new record
		try {
			auto playerId = storage.insert(Player{
				0,
				playerName,
				playerPassword,
				0,
				3,
				0
				});

			auto playerEntry = storage.get<Player>(playerId);
			std::cout << "Inserted new player: " << playerName << " with ID: " << playerEntry.GetId() << std::endl;

			crow::json::wvalue response;
			response["message"] = "Player added";
			response["playerId"] = playerEntry.GetId();
			response["board"] = b.GetPlayerState();
			response["welcomeMessage"] = "Welcome to the game, " + playerName + "!";

			Tank newPlayer(playerEntry.GetId(), playerName, playerPassword, 0, 3, 0);
			b.InsertPlayer(newPlayer);

			return crow::response(response.dump());
		}
		catch (const std::exception& e) {
			crow::json::wvalue errorResponse;
			errorResponse["error"] = "Failed to insert player";
			errorResponse["details"] = e.what();
			std::cerr << "Error: " << errorResponse.dump() << std::endl;
			return crow::response(500, errorResponse.dump());
		}
		});

	CROW_ROUTE(app, "/game").methods("GET"_method)([&b](const crow::request& req) {
		// Lambda function
		auto createGameResponse = [&]() {
			return crow::response(b.GetBoardState().dump());
			};

		if (req.method == "GET"_method) {
			return createGameResponse();  // Lambda call
		}

		return crow::response(405, "Method Not Allowed");
		});

	CROW_ROUTE(app, "/action/<int>/<string>")([&b](int playerId, std::string key) {
		std::lock_guard<std::mutex> lock(gameMutex);// Protect the shared state

		// Player Log
		std::ofstream logFile("log.txt", std::ios_base::app);
		if (logFile.is_open()) {
			logFile << "Log file created. Server started.\n";
			logFile << "Player ID: " << playerId << ", Action: " << key << ", Timestamp: "
				<< std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) << "\n";
		}

		if (key[0] != 'f' && key[0] != 'F')
			b.Move(playerId, key[0]);
		else
		{
			b.Shoot(playerId);
		}

		crow::json::wvalue updatedBoard = b.GetBoardState();  // Get the updated board state
		return crow::response{ updatedBoard };
		});

	CROW_ROUTE(app, "/highScore").methods("GET"_method)([&storage](const crow::request& req) {
		std::string playerName = req.url_params.get("name");

		if (playerName.empty()) {
			return crow::response(400, "Missing 'name' parameter");
		}

		try {
			auto player = storage.get_all<Player>(where(c(&Player::m_name) == playerName));
			if (!player.empty()) {
				int score = player.front().GetScore();
				int highScore = player.front().GetHighScore();
				crow::json::wvalue response;
				response["score"] = score;
				response["highScore"] = highScore;
				return crow::response(response);
			}
			else {
				return crow::response(404, "Player not found");
			}
		}
		catch (const std::exception& e) {
			return crow::response(500, "Server error: " + std::string(e.what()));
		}
		});

	std::unordered_map<int, int> difficultyVotes;

	CROW_ROUTE(app, "/changeDifficulty/<int>").methods("POST"_method)([&b](int difficulty) {
		if (difficulty < 1 || difficulty > 4) {
			return crow::response(400, "Invalid difficulty level");
		}

		b.SetDifficultyAsValue(difficulty);
		b.GenerateBoard();

		return crow::response(200, "Difficulty updated and board regenerated");
		});

	CROW_ROUTE(app, "/getDifficulty").methods("GET"_method)([&b]() {
		return crow::response(std::to_string(b.GetDifficulty()));
		});

	CROW_ROUTE(app, "/closeGame").methods("POST"_method)([&]() {
		// Code to handle the game closure logic
		std::cout << "Game is closing." << std::endl;

		return crow::response(200, "Game closed successfully");
	});

	app.port(18080).multithreaded().run();
	return 0;
}