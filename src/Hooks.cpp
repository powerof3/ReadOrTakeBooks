#include "Hooks.h"
#include "Events.h"
#include "Settings.h"

namespace Book::Hooks
{
	struct GetActivateText
	{
		struct detail
		{
		public:
			static std::string get_book_text(RE::TESObjectBOOK* a_this, RE::TESObjectREFR* a_activator)
			{
				const auto settings = Settings::GetSingleton();
				const auto bookSpell = a_this->GetSpell();
				const auto isHotkeyPressed = Event::Manager::GetSingleton()->IsHotkeyPressed();

				if (bookSpell && !settings->GetAutoUseSpellTomes()) {
					return get_activate_label(a_activator, true);
				}

				const auto take_label_impl = [&]() {
					return get_activate_label(a_activator, !isHotkeyPressed);
				};
				const auto read_label = [&]() {
					return get_activate_label(a_activator, isHotkeyPressed);
				};
				const auto take_label = [&]() {
					return a_this->CanBeTaken() ? take_label_impl() : read_label();
				};

				const auto spell_tome_label = [&]() {
					const auto actor = a_activator->As<RE::Actor>();
					if (actor && actor->HasSpell(bookSpell) || a_this->IsRead()) {
						return take_label();
					}
					return read_label();
				};

				switch (settings->GetDefaultAction()) {
				case kTake:
					return take_label();
				case kAuto:
					{
						if (bookSpell) {
							return spell_tome_label();
						}
						if (a_this->IsRead()) {
							return take_label();
						}
						return read_label();
					}
				case kRead:
					return read_label();
				default:
					return get_activate_label(a_activator, false);
				}
			}

		private:
			static std::string get_activate_label(RE::TESObjectREFR* a_activator, bool a_take)
			{
				if (a_take) {
					return a_activator->IsCrimeToActivate() ? RE::GameSettingCollection::GetSingleton()->GetSetting("sSteal")->GetString() :
					                                          RE::GameSettingCollection::GetSingleton()->GetSetting("sTake")->GetString();
				} else {
					return RE::GameSettingCollection::GetSingleton()->GetSetting("sRead")->GetString();
				}
			}
		};

		static bool thunk(RE::TESObjectBOOK* a_this, RE::TESObjectREFR* a_activator, RE::BSString& a_dst)
		{
			a_dst = std::format("{}\n{}", detail::get_book_text(a_this, a_activator), a_activator->GetDisplayFullName());

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
					const auto transform = root ? root->world : RE::NiTransform{};

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
					const auto settings = Settings::GetSingleton();
					const auto bookSpell = a_this->GetSpell();

					if (bookSpell && !settings->GetAutoUseSpellTomes()) {
						if (!detail::activate_book(a_this, a_targetRef, a_activatorRef, a_targetCount, false)) {
							return false;
						}
					} else {
						const auto isHotkeyPressed = Event::Manager::GetSingleton()->IsHotkeyPressed();

						const auto take_impl = [&]() {
							return detail::activate_book(a_this, a_targetRef, a_activatorRef, a_targetCount, isHotkeyPressed);
						};
						const auto read = [&]() {
							return detail::activate_book(a_this, a_targetRef, a_activatorRef, a_targetCount, !isHotkeyPressed);
						};
						const auto take = [&]() {
							return a_this->CanBeTaken() ? take_impl() : read();
						};

						switch (Settings::GetSingleton()->GetDefaultAction()) {
						case kAuto:
							{
								if (bookSpell) {
									return a_this->IsRead() ? take() : read();
								}
								if (a_this->IsRead()) {
									return take();
								}
								return read();
							}
						case kTake:
							return take();
						case kRead:
							return read();
						}
					}
				} else {
					if (!detail::activate_book(a_this, a_targetRef, a_activatorRef, a_targetCount, false)) {
						return false;
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
		Settings::GetSingleton()->LoadSettings();

		stl::write_vfunc<RE::TESObjectBOOK, Activate>();
		stl::write_vfunc<RE::TESObjectBOOK, GetActivateText>();
	}
}
