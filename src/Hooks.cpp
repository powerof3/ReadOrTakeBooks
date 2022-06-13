#include "Hooks.h"
#include "Events.h"
#include "Settings.h"

namespace Book::Hooks
{
	struct GetActivateText
	{
		struct detail
		{
			static const char* get_gmst_string(const char* a_gmst)
			{
				return RE::GameSettingCollection::GetSingleton()->GetSetting(a_gmst)->GetString();
			}

			static std::string get_take_or_steal(RE::TESObjectREFR* a_activator)
			{
				return a_activator->IsCrimeToActivate() ? get_gmst_string("sSteal") : get_gmst_string("sTake");
			}

			static std::string get_book_text(RE::TESObjectBOOK* a_this, RE::TESObjectREFR* a_activator)
			{
				if (a_this->TeachesSpell() && a_this->GetSpell()) {
					return get_take_or_steal(a_activator);
				} else {
					switch (Settings::GetSingleton()->GetDefaultAction()) {
					case kTake:
						{
							return !Event::Manager::GetSingleton()->GetToggleState() ? get_take_or_steal(a_activator) : get_gmst_string("sRead");
						}
					case kAuto:
						{
							if (a_this->IsRead()) {
								return !Event::Manager::GetSingleton()->GetToggleState() ? get_take_or_steal(a_activator) : get_gmst_string("sRead");
							}
							return !Event::Manager::GetSingleton()->GetToggleState() ? get_gmst_string("sRead") : get_take_or_steal(a_activator);
						}
					case kRead:
						{
							return !Event::Manager::GetSingleton()->GetToggleState() ? get_gmst_string("sRead") : get_take_or_steal(a_activator);
						}
					default:
						return get_gmst_string("sRead");
					}
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
			static void open_book_menu(RE::TESObjectBOOK* a_this, RE::TESObjectREFR* a_ref)
			{
				auto root = a_ref->Get3D();
				const RE::NiTransform transform = root ? root->world : RE::NiTransform{};

				RE::BSString str;
				a_this->GetDescription(str, nullptr);
				RE::BookMenu::OpenBookMenu(str, &a_ref->extraList, a_ref, a_this, transform.translate, transform.rotate, transform.scale, true);
			}

			static bool pick_up_book(const RE::TESObjectBOOK* a_this, RE::TESObjectREFR* a_targetRef, RE::TESObjectREFR* a_activatorRef, std::int32_t a_targetCount)
			{
				if (!a_this->CanBeTaken()) {
					return false;
				}

				if (const auto actor = a_activatorRef->As<RE::Actor>(); actor) {
					actor->PickUpObject(a_targetRef, a_targetCount);
				}

				return true;
			}
		};

		static bool thunk(RE::TESObjectBOOK* a_this, RE::TESObjectREFR* a_targetRef, RE::TESObjectREFR* a_activatorRef, std::uint8_t, RE::TESBoundObject*, std::int32_t a_targetCount)
		{
			if (a_activatorRef) {
				if (a_activatorRef->IsPlayerRef() && !a_this->TeachesSpell()) {
					switch (Settings::GetSingleton()->GetDefaultAction()) {
					case kTake:
						{
							if (!Event::Manager::GetSingleton()->GetToggleState()) {
								return detail::pick_up_book(a_this, a_targetRef, a_activatorRef, a_targetCount);
							} else {
								detail::open_book_menu(a_this, a_targetRef);
							}
						}
						break;
					case kAuto:
						{
							if (a_this->IsRead()) {
								if (!Event::Manager::GetSingleton()->GetToggleState()) {
									return detail::pick_up_book(a_this, a_targetRef, a_activatorRef, a_targetCount);
								} else {
									detail::open_book_menu(a_this, a_targetRef);
								}
							} else {
								if (!Event::Manager::GetSingleton()->GetToggleState()) {
									detail::open_book_menu(a_this, a_targetRef);
								} else {
									return detail::pick_up_book(a_this, a_targetRef, a_activatorRef, a_targetCount);
								}
							}
						}
						break;
					case kRead:
						{
							if (!Event::Manager::GetSingleton()->GetToggleState()) {
								detail::open_book_menu(a_this, a_targetRef);
							} else {
								return detail::pick_up_book(a_this, a_targetRef, a_activatorRef, a_targetCount);
							}
						}
						break;
					default:
						detail::open_book_menu(a_this, a_targetRef);
						break;
					}
				} else if (!detail::pick_up_book(a_this, a_targetRef, a_activatorRef, a_targetCount)) {
					return false;
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
