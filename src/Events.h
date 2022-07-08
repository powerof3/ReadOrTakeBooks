#pragma once

namespace Book::Event
{
	using EventResult = RE::BSEventNotifyControl;

	class Manager final :
		public RE::BSTEventSink<RE::InputEvent*>,
		public RE::BSTEventSink<RE::MenuOpenCloseEvent>
	{
	public:
		static Manager* GetSingleton();

		static void Register();

		bool IsHotkeyPressed() const;

	private:
		EventResult ProcessEvent(RE::InputEvent* const* a_evn, RE::BSTEventSource<RE::InputEvent*>*) override;
		EventResult ProcessEvent(const RE::MenuOpenCloseEvent* a_evn, RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override;

		Manager() = default;
		Manager(const Manager&) = delete;
		Manager(Manager&&) = delete;

		~Manager() override = default;

		Manager& operator=(const Manager&) = delete;
		Manager& operator=(Manager&&) = delete;

		bool toggleKeyHeld{ false };
	};
}
