#include "Hooks.h"
#include "Events.h"
#include "Settings.h"

namespace Book::Hooks
{
	struct GetActivateText
	{
		struct detail
		{
			static std::string get_activate_label(RE::TESObjectREFR* a_activator, bool a_take)
			{
				if (a_take) {
					return a_activator->IsCrimeToActivate() ? RE::GameSettingCollection::GetSingleton()->GetSetting("sSteal")->GetString() :
                                                              RE::GameSettingCollection::GetSingleton()->GetSetting("sTake")->GetString();
				} else {
					return RE::GameSettingCollection::GetSingleton()->GetSetting("sRead")->GetString();
				}
			}

			static std::string get_book_text(RE::TESObjectBOOK* a_this, RE::TESObjectREFR* a_activator)
			{
				auto settings = Settings::GetSingleton();
				auto autoUseSpellTome = settings->GetAutoUseSpellTomes();
				auto getSpellTome = a_this->TeachesSpell() ? a_this->GetSpell() : nullptr;

				if (getSpellTome && !autoUseSpellTome) {
					return get_activate_label(a_activator, true);
				}

				const auto isHotkeyPressed = Event::Manager::GetSingleton()->IsHotkeyPressed();

				const auto get_take_label = [&]() {
					return get_activate_label(a_activator, !isHotkeyPressed);
				};
				const auto get_read_label = [&]() {
					return get_activate_label(a_activator, isHotkeyPressed);
				};

				const auto get_label_for_spell_tome = [&]() {
					const auto actor = a_activator->As<RE::Actor>();
					if (actor && actor->HasSpell(getSpellTome) || a_this->IsRead()) {
						return get_take_label();
					}
					return get_read_label();
				};

				switch (settings->GetDefaultAction()) {
				case kTake:
					return get_take_label();
				case kAuto:
					{
						if (getSpellTome) {
							return get_label_for_spell_tome();
						}
						if (a_this->IsRead()) {
							return get_take_label();
						}
						return get_read_label();
					}
				case kRead:
					return get_read_label();
				default:
					return get_activate_label(a_activator, false);
				}
			}
		};

		static bool thunk(RE::TESObjectBOOK* a_this, RE::TESObjectREFR* a_activator, RE::BSString& a_dst)
		{
			a_dst = fmt::format("{}\n{}", detail::get_book_text(a_this, a_activator), a_activator->GetDisplayFullName());

			return true;
		}
		static inline REL::Relocation<decltype(thunk)> func;

		static inline constexpr std::size_t size = 0x4C;
	};

	struct Activate
	{
		struct detail
		{
			static bool activate_book(RE::TESObjectBOOK* a_this, RE::TESObjectREFR* a_targetRef, RE::TESObjectREFR* a_activatorRef, std::int32_t a_targetCount, bool a_read)
			{
				const bool teachesSpell = a_this->TeachesSpell();

				if (a_read && !teachesSpell) {
					const auto root = a_targetRef->Get3D();
					const RE::NiTransform transform = root ? root->world : RE::NiTransform{};

					RE::BSString str;
					a_this->GetDescription(str, nullptr);
					RE::BookMenu::OpenBookMenu(str, &a_targetRef->extraList, a_targetRef, a_this, transform.translate, transform.rotate, transform.scale, true);
				} else {
					if (!a_this->CanBeTaken()) {
						return false;
					}

					if (const auto actor = a_activatorRef->As<RE::Actor>(); actor) {
						actor->PickUpObject(a_targetRef, a_targetCount);
						if (teachesSpell && a_read && a_this->Read(actor)) {
							actor->RemoveItem(a_this, 1, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
						}
					}
				}
				return true;
			}
		};

		static bool thunk(RE::TESObjectBOOK* a_this, RE::TESObjectREFR* a_targetRef, RE::TESObjectREFR* a_activatorRef, std::uint8_t, RE::TESBoundObject*, std::int32_t a_targetCount)
		{
			if (a_activatorRef) {
				if (a_activatorRef->IsPlayerRef()) {
					auto settings = Settings::GetSingleton();
					auto autoUseSpellTome = settings->GetAutoUseSpellTomes();
					auto getSpellTome = a_this->TeachesSpell() ? a_this->GetSpell() : nullptr;

					if (getSpellTome && !autoUseSpellTome) {
						if (!detail::activate_book(a_this, a_targetRef, a_activatorRef, a_targetCount, false)) {
							return false;
						}
					} else {
						const auto isHotkeyPressed = Event::Manager::GetSingleton()->IsHotkeyPressed();

						const auto take = [&]() {
							return detail::activate_book(a_this, a_targetRef, a_activatorRef, a_targetCount, isHotkeyPressed);
						};
						const auto read = [&]() {
							return detail::activate_book(a_this, a_targetRef, a_activatorRef, a_targetCount, !isHotkeyPressed);
						};

						switch (Settings::GetSingleton()->GetDefaultAction()) {
						case kAuto:
							{
								if (getSpellTome) {
									return a_this->IsRead() ? take() : read();
								}
								if (a_this->IsRead()) {
									return take();
								} else {
									return read();
								}
							}
						case kTake:
							return take();
						case kRead:
							return read();
						}
					}
				}
			}

			return true;
		}
		static inline REL::Relocation<decltype(thunk)> func;

		static inline constexpr std::size_t size = 0x37;
	};

	void Install()
	{
		stl::write_vfunc<RE::TESObjectBOOK, Activate>();
		stl::write_vfunc<RE::TESObjectBOOK, GetActivateText>();
	}
}
