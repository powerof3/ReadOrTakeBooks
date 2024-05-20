#include "Settings.h"

bool Settings::LoadSettings()
{
	const auto path = std::format("Data/SKSE/Plugins/{}.ini", Version::PROJECT);

	CSimpleIniA ini;
	ini.SetUnicode();

	ini.LoadFile(path.c_str());

	ini::get_value(ini, action, "Settings", "Default action", ";Default action upon activating books\n;0 - Automatic (take read books) | 1 - Take | 2 - Read.");
	ini::get_value(ini, hotKey, "Settings", "Alternate action hotkey", ";Press hotkey + Activate key to take instead of read, or vice versa. Default is Left Shift\n;DXScanCodes : https://ck.uesp.net/wiki/Input_Script");
	ini::get_value(ini, autoUseSpellTomes, "Settings", "Auto use spell tomes", ";Learn spells from spell tomes directly.");

	(void) ini.SaveFile(path.c_str());

	return true;
}

DefaultAction Settings::GetDefaultAction() const
{
	return action;
}

Key Settings::GetHotkey() const
{
    return hotKey;
}

bool Settings::GetAutoUseSpellTomes() const
{
    return autoUseSpellTomes;
}
