#include "Settings.h"

bool Settings::LoadSettings()
{
	constexpr auto path = L"Data/SKSE/Plugins/po3_SometimesTakeBooks.ini";

	CSimpleIniA ini;
	ini.SetUnicode();

	ini.LoadFile(path);

	const auto get_value = [&]<class T>(T& a_value, const char* a_section, const char* a_key, const char* a_comment) {
		a_value = string::lexical_cast<T>(ini.GetValue(a_section, a_key, std::to_string(a_value).c_str()));
		ini.SetValue(a_section, a_key, std::to_string(a_value).c_str(), a_comment);
	};

	get_value(action, "Settings", "Default action", ";Default action upon activating books\n;0 - Automatic (take read books) | 1 - Read | 1 - Take.");
	get_value(hotKey, "Settings", "Alternate action hotkey", ";Press hotkey + Activate key to take instead of read, or vice versa. Default is Left Shift\n;Uses DirectX scancodes (https://wiki.nexusmods.com/index.php/DirectX_Scancodes_And_How_To_Use_Them)");

	(void) ini.SaveFile(path);

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
