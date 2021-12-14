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

		SKSE_Plugin() noexcept { hasQuery = false; }
		virtual ~SKSE_Plugin() noexcept {}

		static _NODISCARD bool Query(const SKSEInterface* skse, PluginInfo* info) noexcept;
		static _NODISCARD bool Load(const SKSEInterface* skse) noexcept;

	private:

		static void InitLog() noexcept;
		static void InitInfo(PluginInfo* info) noexcept;
		static _NODISCARD bool IsEditor(const SKSEInterface* skse) noexcept;
		static _NODISCARD bool QueryMessaging(const SKSEInterface* skse) noexcept;

		static bool hasQuery;
		static IDebugLog gLog;
		static PluginHandle plugHandle;
	};
}
