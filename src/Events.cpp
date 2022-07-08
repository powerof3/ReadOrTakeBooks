#include "Events.h"
#include "Settings.h"

namespace Book::Event
{
	Manager* Manager::GetSingleton()
	{
		static Manager singleton;
		return std::addressof(singleton);
	}

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

	struct detail
	{
		static void UpdateCrosshairs(RE::PlayerCharacter* a_player)
		{
			using func_t = decltype(&UpdateCrosshairs);
			REL::Relocation<func_t> func{ RELOCATION_ID(39535, 40621) };
			return func(a_player);
		}
	};

	EventResult Manager::ProcessEvent(RE::InputEvent* const* a_evn, RE::BSTEventSource<RE::InputEvent*>*)
	{
		using InputType = RE::INPUT_EVENT_TYPE;

		if (!a_evn) {
			return EventResult::kContinue;
		}

		if (auto player = RE::PlayerCharacter::GetSingleton(); !player->Is3DLoaded()) {
			return EventResult::kContinue;
		}

		if (const auto UI = RE::UI::GetSingleton(); UI->IsMenuOpen(RE::Console::MENU_NAME)) {
			return EventResult::kContinue;
		}

        const auto hotKey = Settings::GetSingleton()->GetHotkey();

		for (auto event = *a_evn; event; event = event->next) {
			if (const auto button = event->AsButtonEvent(); button) {
				if (const auto key = static_cast<Key>(button->GetIDCode()); key == hotKey) {
					if (toggleKeyHeld != button->IsPressed()) {
						toggleKeyHeld = button->IsPressed();

						if (auto crossHairPickData = RE::CrosshairPickData::GetSingleton()) {
							auto target = crossHairPickData->target.get();
							auto base = target ? target->GetBaseObject() : nullptr;

							if (base && base->IsBook()) {
								detail::UpdateCrosshairs(RE::PlayerCharacter::GetSingleton());
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
		    detail::UpdateCrosshairs(RE::PlayerCharacter::GetSingleton());
		}

	    return EventResult::kContinue;
    }
}
