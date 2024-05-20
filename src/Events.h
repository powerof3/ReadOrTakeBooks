#pragma once

namespace Book::Event
{
	using EventResult = RE::BSEventNotifyControl;

	class Manager final :
		public ISingleton<Manager>,
		public RE::BSTEventSink<RE::InputEvent*>,
		public RE::BSTEventSink<RE::MenuOpenCloseEvent>
	{
	public:
		static void Register();

		bool IsHotkeyPressed() const;

	private:
		EventResult ProcessEvent(RE::InputEvent* const* a_evn, RE::BSTEventSource<RE::InputEvent*>*) override;
		EventResult ProcessEvent(const RE::MenuOpenCloseEvent* a_evn, RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override;

		// members
		bool toggleKeyHeld{ false };
	};
}
