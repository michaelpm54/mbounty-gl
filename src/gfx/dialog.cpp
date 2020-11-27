#include "dialog.h"

#include <cassert>

#include "bounty.h"
#include "engine.h"

void Dialog_UpdateArrow(Dialog &dialog);

void Dialog_Next(Dialog &dialog)
{
	int count = 0;
	int sel = dialog.selection;
	do {
		sel = (sel + 1) % dialog.options.size();
		count++;
	} while (count < dialog.options.size() && dialog.disabledOptions.contains(sel));

	Dialog_SetSelection(dialog, sel);
}

void Dialog_Prev(Dialog &dialog)
{
	int count = 0;
	int sel = dialog.selection;
	do {
		sel = sel = (sel - 1 + dialog.options.size()) % dialog.options.size();
		count++;
	} while (count < dialog.options.size() && dialog.disabledOptions.contains(sel));

	Dialog_SetSelection(dialog, sel);
}

void Dialog_SetPosition(Dialog &dialog, int x_, int y_)
{
	TextBox_SetPosition(dialog.textbox, x_, y_);

	for (int i = 0; i < dialog.options.size(); i++)
	{
		dialog.options[i].setPosition({
			(dialog.textbox.x * 8) + (static_cast<float>(dialog.optionPositions[i].x) * 8),
			(dialog.textbox.y * 8) + (static_cast<float>(dialog.optionPositions[i].y) * 8) + (static_cast<float>(i) * 8),
		});
	}
}

void Dialog_InitOptions(Dialog &dialog)
{
	dialog.options.clear();
	dialog.optionPositions.clear();
	dialog.selection = 0;
	dialog.arrow.setTexture(Engine::s_arrow);
}

void Dialog_Create(Dialog &dialog, int x, int y, int w, int h)
{
	TextBox_Create(dialog.textbox, x, y, w, h);
	TextBox_SetSize(dialog.textbox, w, h);
	Dialog_InitOptions(dialog);
	Dialog_SetPosition(dialog, x, y);
}

void Dialog_AddOption(Dialog &dialog, int x, int y, std::string const &str)
{
	auto &opt = dialog.options.emplace_back(dbr::sfml::BitmapText{str, Engine::s_font});
	opt.setPosition(sf::Vector2f(dialog.textbox.x * 8 + x * 8, dialog.textbox.y * 8 + y * 8));
	dialog.optionPositions.push_back({x, y});
	Dialog_SetSelection(dialog, 0);
}

void Dialog_SetOption(Dialog &dialog, int i, std::string const &str)
{
	assert(i > -1 && i < dialog.options.size());
	dialog.options[i].setString(str);
}

void Dialog_SetSelection(Dialog &dialog, int i)
{
	assert(i > -1 && i < dialog.options.size());

	if (dialog.options.empty())
		return;

	dialog.selection = i;

	Dialog_UpdateArrow(dialog);
}

void Dialog_Draw(Dialog &dialog, sf::RenderWindow &window)
{
	TextBox_Draw(dialog.textbox, window);

	window.draw(dialog.arrow);

	for (auto const &opt : dialog.options)
		window.draw(opt);
}

void Dialog_UpdateArrow(Dialog &dialog)
{
	if (dialog.options.empty())
		return;

	dialog.arrow.setPosition(dialog.options[dialog.selection].getPosition() - sf::Vector2f(16, 0));
}

void Dialog_DisableOption(Dialog &dialog, int i)
{
	if (dialog.disabledOptions.contains(i))
		return;

	dialog.disabledOptions.insert(i);
	if (dialog.selection == i)
		Dialog_Next(dialog);
}

void Dialog_SetColors(Dialog &dialog, sf::Color outline, sf::Color bg)
{
	TextBox_SetColors(dialog.textbox, outline, bg);
}

void Dialog_AddLine(Dialog &dialog, int x, int y, std::string const &str)
{
	TextBox_AddLine(dialog.textbox, x, y, str);
}
