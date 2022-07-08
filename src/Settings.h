#pragma once

enum DefaultAction
{
	kAuto = 0,
	kTake,
	kRead,
};

class Settings
{
public:
    [[nodiscard]] static Settings* GetSingleton()
	{
		static Settings singleton;
		return std::addressof(singleton);
	}

	bool LoadSettings();

    [[nodiscard]] DefaultAction GetDefaultAction() const;
	[[nodiscard]] Key GetHotkey() const;

	bool GetAutoUseSpellTomes() const;

private:
	DefaultAction action{ kAuto };
	Key hotKey{ Key::kLeftShift };

	bool autoUseSpellTomes{true};
};
