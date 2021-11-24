
#if UNEQUIPQUIVERSE_EXPORTS || UNEQUIPQUIVERAE_EXPORTS
#include "Addresses.h"
#endif

#include "Events.h"
#include "Saves.h"

IDebugLog gLog;

PluginHandle Plug_Handle;

SKSEMessagingInterface* skse_msg_interface = nullptr;

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

extern "C" {

#if !UNEQUIPQUIVERAE_EXPORTS
	bool SKSEPlugin_Query(const SKSEInterface * skse, PluginInfo * info)
	{
		info->infoVersion = PluginInfo::kInfoVersion;
		info->version = 1;
#if UNEQUIPQUIVER_EXPORTS
		info->name = "UnequipQuiver";
		gLog.OpenRelative(CSIDL_MYDOCUMENTS, "\\My Games\\Skyrim\\SKSE\\UnequipQuiver.log");
#elif UNEQUIPQUIVERSE_EXPORTS
		info->name = "UnequipQuiverSE";
		gLog.OpenRelative(CSIDL_MYDOCUMENTS, "\\My Games\\Skyrim Special Edition\\SKSE\\UnequipQuiverSE.log");
#endif

		Plug_Handle = skse->GetPluginHandle();

#if UNEQUIPQUIVERSE_EXPORTS
		if (!Addresses::LoadDatabase())
			return false;
#endif

		if (skse->isEditor) {

			_MESSAGE("loaded in editor, marking as incompatible");

			return false;
		}

#if UNEQUIPQUIVER_EXPORTS
		if (skse->runtimeVersion != RUNTIME_VERSION_1_9_32_0) {

			_MESSAGE("unsupported runtime version %08X", skse->runtimeVersion);

			return false;
		}
#endif

		if (!(skse_msg_interface = (SKSEMessagingInterface*)skse->QueryInterface(kInterface_Messaging))) {

			_MESSAGE("Error Query Messaging Interface!");

			return false;
		}

		return true;
	}

	bool SKSEPlugin_Load(const SKSEInterface * skse)
	{
		return skse_msg_interface ? skse_msg_interface->RegisterListener(Plug_Handle, "SKSE", MsgCallback) : false;
	}
#else
	bool SKSEPlugin_Load(const SKSEInterface* skse)
	{
		gLog.OpenRelative(CSIDL_MYDOCUMENTS, "\\My Games\\Skyrim Special Edition\\SKSE\\UnequipQuiverSE.log");

		Plug_Handle = skse->GetPluginHandle();

		if (skse->isEditor) {

			_MESSAGE("loaded in editor, marking as incompatible");

			return false;
		}

		if (!(skse_msg_interface = (SKSEMessagingInterface*)skse->QueryInterface(kInterface_Messaging))) {

			_MESSAGE("Error Query Messaging Interface!");

			return false;
		}

		return skse_msg_interface ? skse_msg_interface->RegisterListener(Plug_Handle, "SKSE", MsgCallback) : false;
	}

	__declspec(dllexport) SKSEPluginVersionData SKSEPlugin_Version =
	{
		SKSEPluginVersionData::kVersion,
		1,
		"UnequipQuiverSE",
		"PK0",
		"",
		SKSEPluginVersionData::kVersionIndependent_Signatures,
		{ RUNTIME_VERSION_1_6_318, 0 },
		0,
	};
#endif
}

