
#include "skse_plugin.h"
#include "papyrus.h"

namespace skse_plugin {

	bool SKSE_Plugin::hasQuery{ false };

	PluginHandle SKSE_Plugin::plugHandle ;

	SKSEPapyrusInterface* SKSE_Plugin::skse_papyrus_interface{ nullptr };
	SKSEMessagingInterface* SKSE_Plugin::skse_msg_interface{ nullptr };
	SKSETaskInterface* SKSE_Plugin::skse_task_interface{ nullptr };

	void SKSE_Plugin::MsgCallback(SKSEMessagingInterface::Message* msg) noexcept
	{
		using namespace UQ_Saves;

		switch (msg->type) {
		case SKSEMessagingInterface::kMessage_DataLoaded:

			UQ_Settings::UQSettings.ReadAllSettings();
			break;

		case SKSEMessagingInterface::kMessage_PostPostLoad:

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

			if (!Addresses::LoadDatabaseSE())
				return false;
		}
		else {

			if (!Addresses::LoadDatabaseAE())
				return false;
		}
	#endif

		if (IsEditor(skse)) return false;
		if (!QueryMessaging(skse)) return false;
		if (!QueryTask(skse)) return false;
		if (!QueryPapyrus(skse)) return false;

		return true;
	}

	_NODISCARD bool SKSE_Plugin::Load(const SKSEInterface* skse) noexcept
	{
		if (!hasQuery)
			if (!Query(skse))
				return false;

		if (!skse_papyrus_interface || !skse_papyrus_interface->Register(papyrus::RegisterFunctions)) 
			return false;

		if (!skse_msg_interface || !skse_msg_interface->RegisterListener(plugHandle, "SKSE", MsgCallback))
			return false;

		return true;
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
		if (!(skse_msg_interface = static_cast<SKSEMessagingInterface*>(skse->QueryInterface(kInterface_Messaging)))) {

			_MESSAGE("Error Query Messaging Interface!");

			return false;
		}

		return true;
	}

	_NODISCARD bool SKSE_Plugin::QueryTask(const SKSEInterface* skse) noexcept
	{
		if (!(skse_task_interface = static_cast<SKSETaskInterface*>(skse->QueryInterface(kInterface_Task)))) {

			_MESSAGE("Error Query Task Interface!");

			return false;
		}

		return true;
	}

	_NODISCARD bool SKSE_Plugin::QueryPapyrus(const SKSEInterface* skse) noexcept
	{
		if (!(skse_papyrus_interface = static_cast<SKSEPapyrusInterface*>(skse->QueryInterface(kInterface_Papyrus)))) {

			_MESSAGE("Error Query Papyrus Interface!");

			return false;
		}

		return true;
	}
}
