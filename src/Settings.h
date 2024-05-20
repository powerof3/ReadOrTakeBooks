#pragma once

enum DefaultAction
{
	kAuto = 0,
	kTake,
	kRead,
};

class Settings : public ISingleton<Settings>
{
public:
	bool LoadSettings();

    [[nodiscard]] DefaultAction GetDefaultAction() const;
	[[nodiscard]] Key GetHotkey() const;

	bool GetAutoUseSpellTomes() const;

private:
	DefaultAction action{ kAuto };
	Key hotKey{ 42 };

	bool autoUseSpellTomes{true};
};
