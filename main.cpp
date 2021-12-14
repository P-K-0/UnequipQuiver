
#include "skse_plugin.h"

extern "C" {

	bool SKSEPlugin_Query(const SKSEInterface * skse, PluginInfo * info)
	{
		return skse_plugin::SKSE_Plugin::Query(skse, info);
	}

	bool SKSEPlugin_Load(const SKSEInterface * skse)
	{
		return skse_plugin::SKSE_Plugin::Load(skse);
	}

#if UNEQUIPQUIVERSE_EXPORTS || UNEQUIPQUIVERAE_EXPORTS
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
