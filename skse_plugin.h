#pragma once

#if UNEQUIPQUIVERSE_EXPORTS || UNEQUIPQUIVERAE_EXPORTS
#include "Addresses.h"
#endif

#include "Events.h"
#include "Saves.h"

namespace skse_plugin {

	class SKSE_Plugin {

	public:

		SKSE_Plugin(const SKSE_Plugin&) = delete;
		SKSE_Plugin(SKSE_Plugin&&) = delete;
		SKSE_Plugin& operator=(const SKSE_Plugin&) = delete;
		SKSE_Plugin& operator=(SKSE_Plugin&&) = delete;

		static SKSE_Plugin& GetInstance() { return instance; }

		virtual ~SKSE_Plugin() noexcept {}

		_NODISCARD bool Query(const SKSEInterface* skse, PluginInfo* info = nullptr) noexcept;
		_NODISCARD bool Load(const SKSEInterface* skse) noexcept;

		_NODISCARD SKSEPapyrusInterface* GetPapyrusInterface() { return skse_papyrus_interface; }
		_NODISCARD SKSEMessagingInterface* GetMessageInterface() { return skse_msg_interface; }
		_NODISCARD SKSETaskInterface* GetTaskInterface() { return skse_task_interface; }

		template<typename T>
		_NODISCARD bool Sink(std::uint32_t dispatcher, BSTEventSink<T>* sink) 
		{
			if (!skse_msg_interface) return false;

			auto events = static_cast<EventDispatcher<T>*>(skse_msg_interface->GetEventDispatcher(dispatcher)); 

			if (!events) return false;

#if UNEQUIPQUIVERSE_EXPORTS || UNEQUIPQUIVERAE_EXPORTS
			events->AddEventSinkAddr(sink, Addresses::AddEventSink_Internal_Addr);
#else
			events->AddEventSink(sink);
#endif

			return true;
		}

		void AddTask(TaskDelegate* task)
		{
			if (!skse_task_interface) return;

			skse_task_interface->AddTask(task);
		}

	private:

		SKSE_Plugin() {}

		void InitLog() noexcept;
		void InitInfo(PluginInfo* info) noexcept;

		_NODISCARD bool IsEditor(const SKSEInterface* skse) noexcept;

		_NODISCARD bool QueryMessaging(const SKSEInterface* skse) noexcept;
		_NODISCARD bool QueryTask(const SKSEInterface* skse) noexcept;
		_NODISCARD bool QueryPapyrus(const SKSEInterface* skse) noexcept;

		static void MsgCallback(SKSEMessagingInterface::Message* msg) noexcept;

		bool hasQuery{ false };

		IDebugLog gLog{};

		PluginHandle plugHandle{};

		SKSEPapyrusInterface* skse_papyrus_interface{ nullptr };
		SKSEMessagingInterface* skse_msg_interface{ nullptr };
		SKSETaskInterface* skse_task_interface{ nullptr };

		static SKSE_Plugin instance;
	};
}
