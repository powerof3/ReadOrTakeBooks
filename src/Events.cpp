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
			inputMgr->AddEventSink(GetSingleton());

			logger::info("Registered for hotkey event");
		}
	}

	bool Manager::GetToggleState() const
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

		auto player = RE::PlayerCharacter::GetSingleton();
		if (!player || !player->Is3DLoaded()) {
			return EventResult::kContinue;
		}

        const auto hotKey = Settings::GetSingleton()->GetHotkey();

		for (auto event = *a_evn; event; event = event->next) {
			if (const auto button = event->AsButtonEvent(); button) {
				if (const auto key = static_cast<Key>(button->GetIDCode()); key == hotKey) {
					if (toggleKeyHeld != button->IsPressed()) {
						toggleKeyHeld = button->IsPressed();

						auto crossHairPickData = RE::CrosshairPickData::GetSingleton();
						auto target = crossHairPickData ? crossHairPickData->target.get() : RE::TESObjectREFRPtr{};
						auto base = target ? target->GetBaseObject() : nullptr;

						if (base && base->IsBook()) {
							detail::UpdateCrosshairs(RE::PlayerCharacter::GetSingleton());
						}
					}
				}
			}
		}

		return EventResult::kContinue;
	}
}
