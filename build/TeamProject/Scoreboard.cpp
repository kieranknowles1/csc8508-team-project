#include "Scoreboard.h"
#include <vector>
#include <algorithm>
#include <GameTechRendererInterface.h>

namespace NCL::CSC8503 {

    void Scoreboard::AddPlayer(Lobbies::User user) {
        Player player(user, TeamColor::BLUE, 0);
        players.insert(std::make_pair(user.GetUserID(), player));
    }

    void Scoreboard::SetColor(Lobbies::User user) {
        players.at(user.GetUserID()).color = TeamColor::GREEN;
    }

    void Scoreboard::SetScore() {
        for (auto& player : players) {
            player.second.score++;
        }
    }

    void Scoreboard::UpdateScoreboard() {
        SortPlayers();
		UpdateTextValues();
    }

    void Scoreboard::SortPlayers() {
        std::vector<std::pair<unsigned int, Player>> playerVector(players.begin(), players.end());
        std::sort(playerVector.begin(), playerVector.end(), [](const auto& a, const auto& b) {
            return a.second.score > b.second.score;
        });

        players.clear();
        for (const auto& pair : playerVector) {
            players.insert(pair);
        }
    }

    void Scoreboard::InitScoreboardUI() {
        Maths::Vector4 scoreboardColor = Maths::Vector4(0.1f, 0.1f, 0.1f, 0.7f);
        Maths::Vector4 boxColor = Maths::Vector4(0.0f, 0.0f, 0.0f, 0.7f);
        Maths::Vector4 borderColor = Maths::Vector4(0.5f, 0.5f, 0.5f, 0.7f); // Highlight color for borders
        Maths::Vector4 textColor = Maths::Vector4(0.0f, 0.0f, 0.0f, 1.0f); // Black text color
        Maths::Vector2 totalSize = Maths::Vector2(0.7f, 0.7f);
        Maths::Vector2 screenCenter = Maths::Vector2(0.5f, 0.5f);
        float borderThickness = 0.005f; // Thickness of the border
        
        int playerCount = players.size();
        int columns = 3;

        Maths::Vector2 boxSize = Maths::Vector2(totalSize.x / columns, totalSize.y / 9);

        AddUIElement({ screenCenter, totalSize, scoreboardColor, nullptr, true });

        for (int row = 0; row < 9; ++row) {
            Maths::Vector2 rowPosition = Vector2(
                screenCenter.x,
                screenCenter.y - (totalSize.y / 2.0f) + (boxSize.y / 2.0f) + (row * boxSize.y)
            );

            // Add row border
            AddUIElement({ rowPosition, Vector2(totalSize.x, boxSize.y) + Vector2(borderThickness, borderThickness), borderColor, nullptr, true });

            for (int col = 0; col < columns; ++col) {
                Maths::Vector2 position = Vector2(
                    screenCenter.x - totalSize.x / 2.0f + boxSize.x / 2.0f + col * boxSize.x,
                    screenCenter.y - totalSize.y / 2.0f + boxSize.y / 2.0f + row * boxSize.y
                );

                // Add main box
                AddUIElement({ position, boxSize, boxColor, nullptr, true });

                // Add text
                Maths::Vector2 textPosition = Vector2(
                    position.x - boxSize.x / 2.0f + 0.01f, // Slightly offset to the left
                    position.y + (boxSize.y * 0.25f) // Slightly offset to the bottom
                );

                //title row
                if (row == 0) {
                    std::string text;
                    if (col == 0) {
                        text = "User:";
                    }
                    else if (col == 1) {
                        text = "Colour:";
                    }
                    else {
                        text = "Score:";
                    }
                    AddUITextElement({ textPosition, textColor, text, true });
                }

                // Empty row
                else if (row > players) {
                    AddUITextElement({ textPosition, textColor, "", true });
                }
                // Data rows
                else {
                    std::string text;
                    if (col == 0) {
                        text = players.at(row-1).user.GetDisplayName();
                    }
                    else if (col == 1) {
                        text = TeamColorToString(players.at(row - 1).color);
                    }
                    else {
                        text = std::to_string(players.at(row - 1).score);
                    }
                    AddScoreboardText({ textPosition, text });
                }
            }
        }
    }


    void Scoreboard::UpdateTextValues() {
        for (int i = 0; i < players.size() * 3; i += 3) {
			scoreboardTextElements[i].text = players.at(i).user.GetDisplayName();
			scoreboardTextElements[i + 1].text = TeamColorToString(players.at(i).color);
			scoreboardTextElements[i + 2].text = std::to_string(players.at(i).score);
        }
    }
}
