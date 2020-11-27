#ifndef KH_DIALOG_H
#define KH_DIALOG_H

#include <array>
#include <unordered_set>
#include <vector>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include "BitmapText.hpp"
#include "textbox.h"

struct Dialog
{
	TextBox textbox;
	int selection{0};
	std::unordered_set<int> disabledOptions;
	std::vector<dbr::sfml::BitmapText> options;
	std::vector<sf::Vector2i> optionPositions;
	sf::Sprite arrow;
};

void Dialog_Create(Dialog &dialog, int x, int y, int w, int h);
void Dialog_Draw(Dialog &dialog, sf::RenderWindow &window);
void Dialog_Prev(Dialog &dialog);
void Dialog_Next(Dialog &dialog);
void Dialog_SetSize(Dialog &dialog, int w, int h);
void Dialog_SetPosition(Dialog &dialog, int x, int y);
void Dialog_SetSelection(Dialog &dialog, int i);
void Dialog_SetOption(Dialog &dialog, int i, std::string const &str);
void Dialog_DisableOption(Dialog &dialog, int i);
void Dialog_AddLine(Dialog &dialog, int x, int y, std::string const &str);
void Dialog_AddOption(Dialog &dialog, int x, int y, std::string const &str);
void Dialog_SetColors(Dialog &dialog, sf::Color outline, sf::Color bg);

#endif // KH_DIALOG_H
