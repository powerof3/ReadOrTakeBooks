#include "Events.h"
#include "Settings.h"

namespace Book::Event
{
	void Manager::Register()
	{
		logger::info("{:*^30}", "EVENTS");

		if (const auto inputMgr = RE::BSInputDeviceManager::GetSingleton()) {
			inputMgr->AddEventSink<RE::InputEvent*>(GetSingleton());

			logger::info("Registered for hotkey event");
		}
		if (const auto menuMgr = RE::UI::GetSingleton()) {
			menuMgr->AddEventSink<RE::MenuOpenCloseEvent>(GetSingleton());

			logger::info("Registered for menu event");
		}
	}

	bool Manager::IsHotkeyPressed() const
	{
		return toggleKeyHeld;
	}

	EventResult Manager::ProcessEvent(RE::InputEvent* const* a_evn, RE::BSTEventSource<RE::InputEvent*>*)
	{
		using InputType = RE::INPUT_EVENT_TYPE;

		if (!a_evn) {
			return EventResult::kContinue;
		}

		const auto player = RE::PlayerCharacter::GetSingleton();
		if (!player || !player->Is3DLoaded()) {
			return EventResult::kContinue;
		}

		const auto UI = RE::UI::GetSingleton();
		if (!UI || UI->IsMenuOpen(RE::Console::MENU_NAME) || UI->GameIsPaused()) {
			return EventResult::kContinue;
		}

        const auto hotKey = Settings::GetSingleton()->GetHotkey();

		for (auto event = *a_evn; event; event = event->next) {
			if (const auto button = event->AsButtonEvent(); button) {
				const auto device = event->GetDevice();

				auto key = button->GetIDCode();

				switch (device) {
				case RE::INPUT_DEVICE::kMouse:
					key += SKSE::InputMap::kMacro_MouseButtonOffset;
					break;
				case RE::INPUT_DEVICE::kGamepad:
					key = SKSE::InputMap::GamepadMaskToKeycode(key);
					break;
				default:
					break;
				}

				if (key == hotKey) {
					if (toggleKeyHeld != button->IsHeld()) {
						toggleKeyHeld = button->IsHeld();

						if (const auto crossHairPickData = RE::CrosshairPickData::GetSingleton()) {
                            const auto target = crossHairPickData->target.get();
							auto base = target ? target->GetBaseObject() : nullptr;

							if (base && base->IsBook()) {
								player->UpdateCrosshairs();
							}
						}
					}
				}
			}
		}

		return EventResult::kContinue;
	}

    EventResult Manager::ProcessEvent(const RE::MenuOpenCloseEvent* a_evn, RE::BSTEventSource<RE::MenuOpenCloseEvent>*)
    {
		if (a_evn && !a_evn->opening && a_evn->menuName == RE::BookMenu::MENU_NAME) {
			if (const auto UI = RE::UI::GetSingleton(); UI->IsMenuOpen(RE::InventoryMenu::MENU_NAME)) {
				return EventResult::kContinue;
			}
			RE::PlayerCharacter::GetSingleton()->UpdateCrosshairs();
		}

	    return EventResult::kContinue;
    }
}
