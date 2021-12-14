
#include "settings.h"

namespace UQ_Settings {

	std::string ReadSettingIni(const std::string& key, const std::string& def, const std::string& app, const std::string& filename)
	{
		char buffer[4096];

		if (GetPrivateProfileString(app.c_str(), key.c_str(), def.c_str(), &buffer[0], sizeof buffer, filename.c_str()) > 0)
			return std::string(buffer);

		return def;
	}

	template<typename T>
	std::string ReadSettingT(const std::string& key, T def) { return ReadSettingIni(key, std::to_string(static_cast<T>(def))); }

	int ReadSetting(const std::string& key, int def) { return std::stoi(ReadSettingT(key, def)); }
	bool ReadSetting(const std::string& key, bool def) { return std::stoi(ReadSettingT(key, def)); }
	float ReadSetting(const std::string& key, float def) { return std::stof(ReadSettingT(key, def)); }
	QuiverReEquipType ReadSetting(const std::string& key, QuiverReEquipType def) { return static_cast<QuiverReEquipType>(ReadSetting(key, static_cast<int>(def))); }

#define READSETTINGT(key) (key) = ReadSetting(# key, Default_ ## key); \
						  _DMESSAGE(# key " = %i", key);
#define READSETTINGS(key, v) (key) = ReadSettingIni(# key, v)

	void UnequipQuiver_Settings::ReadSettings()
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

		ParseKeywords(ReadSettingIni("sKeywords", Default_sKeywords));
	
		READSETTINGT(bBlackListAmmo);
	
		ParseBlackList(ReadSettingIni("sBlackListAmmo", Default_sBlackListAmmo));

		READSETTINGT(bExtraData);

		READSETTINGT(bHideQuiverOnSheathe);

		READSETTINGT(bHideQuiverOnDraw);

		READSETTINGT(bHideBoltOnSheathe);

		READSETTINGT(bHideBoltOnDraw);
	}

	template<typename Func = std::function<void(const std::string&)>>
	void ParseString(const std::string& str, Func func)
	{
		std::string tmpStr = str;
		size_t found{ 0 };

		while (tmpStr.size() > 0) {

			found = tmpStr.find(",");

			if (found != std::string::npos) {
				func(tmpStr.substr(0, found));
				tmpStr = tmpStr.substr(found + 1);
			}
			else {
				func(tmpStr);
				tmpStr = "";
			}
		}
	}

	void UnequipQuiver_Settings::ParseKeywords(const std::string& str)
	{
		KeysList.clear();

		ParseString(str, [&](const std::string& s) {
			KeysList.push_back(s); 
		});

		if (!KeysList.empty())
			sort(KeysList);	
	}

	void UnequipQuiver_Settings::ParseBlackList(const std::string& str)
	{
		BlackListAmmo.clear();

		std::regex reg{ "(\\w*.\\w*):(\\w*)"};
		std::smatch matches;

		char *end;

		DataHandler * datahandler = DataHandler::GetSingleton();

		if (datahandler && bBlackListAmmo) {

			ParseString(str, [&](const std::string& s) {

				if (std::regex_search(s, matches, reg)) {

					const ModInfo * mod = datahandler->LookupModByName(matches[1].str().c_str());

					if (mod) {

						UInt32 id{ 0 };

#if UNEQUIPQUIVERSE_EXPORTS || UNEQUIPQUIVERAE_EXPORTS
						if (mod->IsLight()) {
							id = 0xfe000000 | (static_cast<UInt32>(mod->lightIndex) << 12);
							id |= std::strtol(matches[2].str().c_str(), &end, 16) & 0xfff;
						}
						else {
							id = (static_cast<UInt32>(mod->modIndex) << 24);
							id |= std::strtol(matches[2].str().c_str(), &end, 16) & 0xffffff;
						}
#elif UNEQUIPQUIVER_EXPORTS
						id = (static_cast<UInt32>(mod->modIndex) << 24);
						id |= std::strtol(matches[2].str().c_str(), &end, 16) & 0xffffff;
#endif

						BlackListAmmo.push_back(id);

						_DMESSAGE("%s\t:\t 0x%.8X", matches[1].str().c_str(), id);
					}
				}
			});
		}

		if (!BlackListAmmo.empty())
			sort(BlackListAmmo);
	}

	UnequipQuiver_Settings UQSettings;
};
