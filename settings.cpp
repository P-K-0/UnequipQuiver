
#include "settings.h"

namespace UQ_Settings {

	std::string ReadSetting(const std::string& key, const std::string& def, const std::string& app, const std::string& filename)
	{
		char buffer[4096];

		if (GetPrivateProfileString(app.c_str(), key.c_str(), def.c_str(), &buffer[0], sizeof buffer, filename.c_str()) > 0)
			return std::string(buffer);

		return def;
	}

	template<typename T>
	T ReadSettingT(const std::string& key, const T def) { return static_cast<T>(std::stoi(ReadSetting(key, std::to_string(static_cast<int>(def))))); }

#define READSETTINGT(key) (key) = ReadSettingT(# key, Default_ ## key)
#define READSETTINGS(key, v) (key) = ReadSetting(# key, v)

	UnequipQuiver_Settings::UnequipQuiver_Settings()
	{
		READSETTINGT(bEnablePC);

		READSETTINGT(bEnableNPC);

		READSETTINGT(bSpell);

		READSETTINGT(bWeapon);

		READSETTINGT(bShield);

		READSETTINGT(bBow);

		READSETTINGT(bCrossbow);

		READSETTINGT(iReEquipType);

		READSETTINGT(bCheckWeaponByKeywords);

		READSETTINGT(bSavefile);

		READSETTINGS(sSavePath, "");
		if (!sSavePath.empty() && sSavePath.substr(sSavePath.length() -1, 1) != "\\")
			sSavePath += "\\";

		READSETTINGT(bMultiBow);

		READSETTINGT(bEquipStronger);

		KeysList = ParseKeywords(ReadSetting("sKeywords", StrDefaultKeywords));
		
		std::sort(KeysList.begin(), KeysList.end());
	}

	UnequipQuiver_Settings::WeapKeywords UnequipQuiver_Settings::ParseKeywords(const std::string& str)
	{
		WeapKeywords tmpKeys;
		std::string tmpStr = str;
		size_t f;

		while (tmpStr.size() > 0) {

			f = tmpStr.find(",");

			if (f != std::string::npos) {
				tmpKeys.push_back(tmpStr.substr(0, f));
				tmpStr = tmpStr.substr(f + 1);
			} else {
				tmpKeys.push_back(tmpStr);
				tmpStr = "";
			}
		}

		return tmpKeys;
	}

	UnequipQuiver_Settings UQSettings;
};
