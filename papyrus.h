#pragma once

#include "std_library_facilities.h"
#include "skse_libray_facilities.h"
#include "settings.h"

namespace papyrus {

#if UNEQUIPQUIVERSE_EXPORTS || UNEQUIPQUIVERAE_EXPORTS
	void OnSettingChangeInt(StaticFunctionTag* base, BSFixedString a_ID, SInt32 value)
		{ UQ_Settings::Settings::GetInstance().Set(a_ID.c_str(), value); }

	void OnSettingChangeBool(StaticFunctionTag* base, BSFixedString a_ID, bool value)
		{ UQ_Settings::Settings::GetInstance().Set(a_ID.c_str(), value); }

	void OnSettingChangeFloat(StaticFunctionTag* base, BSFixedString a_ID, float value)
		{ UQ_Settings::Settings::GetInstance().Set(a_ID.c_str(), value); }

	void OnSettingChangeString(StaticFunctionTag* base, BSFixedString a_ID, BSFixedString value)
		{ UQ_Settings::Settings::GetInstance().Set(a_ID.c_str(), value.c_str()); }

	bool RegisterFunctions(VMClassRegistry* registry)
	{
		registry->RegisterFunction(new NativeFunction2<StaticFunctionTag, void, BSFixedString, SInt32>("OnSettingChangeInt", "UQ_Papyrus", OnSettingChangeInt, registry));
		registry->RegisterFunction(new NativeFunction2<StaticFunctionTag, void, BSFixedString, bool>("OnSettingChangeBool", "UQ_Papyrus", OnSettingChangeBool, registry));
		registry->RegisterFunction(new NativeFunction2<StaticFunctionTag, void, BSFixedString, float>("OnSettingChangeFloat", "UQ_Papyrus", OnSettingChangeFloat, registry));
		registry->RegisterFunction(new NativeFunction2<StaticFunctionTag, void, BSFixedString, BSFixedString>("OnSettingChangeString", "UQ_Papyrus", OnSettingChangeString, registry));

		_MESSAGE("Papyrus functions registered successfully!");

		return true;
	}
#endif
}