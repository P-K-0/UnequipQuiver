
#include "skse_plugin.h"

namespace skse_plugin {

	bool SKSE_Plugin::hasQuery;
	PluginHandle SKSE_Plugin::plugHandle;

	void MsgCallback(SKSEMessagingInterface::Message* msg)
	{
		using namespace UQ_Saves;

		switch (msg->type) {
		case SKSEMessagingInterface::kMessage_DataLoaded:

			UQ_Settings::UQSettings.ReadSettings();
			break;

		case SKSEMessagingInterface::kMessage_PostLoad:

			EventsDispatch::RegisterEventDispatch();
			break;

		case SKSEMessagingInterface::kMessage_PreLoadGame:

			LoadSaves(msg->data, UQFlags::uqLoad);
			break;

		case SKSEMessagingInterface::kMessage_SaveGame:

			LoadSaves(msg->data, UQFlags::uqSave);
			break;

		case SKSEMessagingInterface::kMessage_DeleteGame:

			LoadSaves(msg->data, UQFlags::uqDelete);
			break;
		}
	}

	_NODISCARD bool SKSE_Plugin::Query(const SKSEInterface* skse, PluginInfo* info) noexcept
	{
		plugHandle = skse->GetPluginHandle();

		InitLog();
		InitInfo(info);

	#if UNEQUIPQUIVERSE_EXPORTS || UNEQUIPQUIVERAE_EXPORTS
		if (info) {

			if (!Addresses::LoadDatabaseSE()) return false;
		}
		else {

			if (!Addresses::LoadDatabaseAE()) return false;
		}
	#endif

		if (IsEditor(skse)) return false;
		if (!QueryMessaging(skse)) return false;

		return true;
	}

	_NODISCARD bool SKSE_Plugin::Load(const SKSEInterface* skse) noexcept
	{
		if (!hasQuery)
			if (!Query(skse, nullptr))
				return false;

		return EventsDispatch::skse_msg_interface ? EventsDispatch::skse_msg_interface->RegisterListener(plugHandle, "SKSE", MsgCallback) : false;
	}

	void SKSE_Plugin::InitLog() noexcept
	{
	#if UNEQUIPQUIVER_EXPORTS
		gLog.OpenRelative(CSIDL_MYDOCUMENTS, "\\My Games\\Skyrim\\SKSE\\UnequipQuiver.log");
	#elif UNEQUIPQUIVERSE_EXPORTS || UNEQUIPQUIVERAE_EXPORTS
		gLog.OpenRelative(CSIDL_MYDOCUMENTS, "\\My Games\\Skyrim Special Edition\\SKSE\\UnequipQuiverSE.log");
	#endif
	}

	void SKSE_Plugin::InitInfo(PluginInfo* info) noexcept
	{
		if (!info) return;

		hasQuery = true;

		info->infoVersion = PluginInfo::kInfoVersion;
		info->version = 1;
	#if UNEQUIPQUIVER_EXPORTS
		info->name = "UnequipQuiver";
	#elif UNEQUIPQUIVERSE_EXPORTS || UNEQUIPQUIVERAE_EXPORTS
		info->name = "UnequipQuiverSE";
	#endif
	}

	_NODISCARD bool SKSE_Plugin::IsEditor(const SKSEInterface* skse) noexcept
	{
		if (skse->isEditor) {

			_MESSAGE("loaded in editor, marking as incompatible");

			return true;
		}

		return false;
	}

	_NODISCARD bool SKSE_Plugin::QueryMessaging(const SKSEInterface* skse) noexcept
	{
		if (!(EventsDispatch::skse_msg_interface = (SKSEMessagingInterface*)skse->QueryInterface(kInterface_Messaging))) {

			_MESSAGE("Error Query Messaging Interface!");

			return false;
		}

		return true;
	}
}
