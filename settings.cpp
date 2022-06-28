
#include "settings.h"

namespace UQ_Settings {

	std::string IniSettings::ReadSettingIni(const std::string& key, const std::string& app, const std::string def)
	{
		const char* buffer = ini.GetValue(app.c_str(), key.c_str(), def.c_str());

		return std::string(buffer);
	}

	void Settings::ReadSettings(const std::string& Filename, bool readDefault)
	{
		_DMESSAGE("ReadSettings : %s", Filename.c_str());

		IniSettings ini{ Filename };

		if (!ini) return;

#define READSETTINGS(val) \
		val = ini.ReadAs(# val, "Main", readDefault ? Default_ ## val : val); \
		_DMESSAGE(# val " = %i", val);

#define READSETTINGSS(val) \
		val = ini.ReadAs(# val, "Main", readDefault ? Default_ ## val : val); \
		_DMESSAGE(# val " = %s", val.c_str());

#define READSETTINGSENUM(val) \
		val[CharacterType::PC] = ini.ReadAs(# val, "Main", readDefault ? Default_ ## val : val[CharacterType::PC]); \
		val[CharacterType::NPC] = ini.ReadAs(# val "NPC", "Main", readDefault ? Default_ ## val : val[CharacterType::NPC]); \
		_DMESSAGE(# val " = %i", val[CharacterType::PC]); \
		_DMESSAGE(# val "NPC = %i", val[CharacterType::NPC]); 

		READSETTINGS(bEnablePC);

		READSETTINGS(bEnableNPC);

		READSETTINGS(bSpell);

		READSETTINGS(bWeapon);

		READSETTINGS(bShield);

		READSETTINGS(bBow);

		READSETTINGS(bCrossbow);

		READSETTINGS(iReEquipType);

		READSETTINGS(bCheckWeaponByKeywords);

		READSETTINGS(bSavefile);

		READSETTINGSS(sSavePath);
		if (!sSavePath.empty() && sSavePath.substr(sSavePath.length() -1, 1) != "\\")
			sSavePath += "\\";

		READSETTINGS(bMultiBow);

		READSETTINGS(bEquipStronger);
		READSETTINGS(bEquipLargerAmount);

		READSETTINGSS(sKeywords);
		ParseKeywords(sKeywords);

		READSETTINGS(bBlackListAmmo);
		READSETTINGSS(sBlackListAmmo);
		ParseBlackList(sBlackListAmmo, BlackListAmmo);

		READSETTINGS(bBlackListRace);
		READSETTINGSS(sBlackListRace);
		ParseBlackList(sBlackListRace, BlackListRace);

		READSETTINGS(bBlackListCharacter);
		READSETTINGSS(sBlackListCharacter);
		ParseBlackList(sBlackListCharacter, BlackListCharacter);

		READSETTINGS(bExtraData);

		READSETTINGSENUM(bHideQuiverOnSheathe);

		READSETTINGSENUM(bHideQuiverOnDraw);

		READSETTINGSENUM(bHideBoltOnSheathe);

		READSETTINGSENUM(bHideBoltOnDraw);
	}

	void Settings::ReadAllSettings()
	{
#if UNEQUIPQUIVERSE_EXPORTS || UNEQUIPQUIVERAE_EXPORTS
		DataHandler* data = DataHandler::GetSingleton();
		const ModInfo* info{ nullptr };

		if (data && (info = data->LookupModByName("UnequipQuiverSE.esp")) && info->IsActive()) {

			ReadSettings(UnequipQuiverIniMCM);
			ReadSettings(UnequipQuiverIniSettings, false);

			return;
		}
#endif

		ReadSettings(UnequipQuiverIni);
	}

	const bool Settings::CheckExtraDataAmmo(const UInt32 id) const
	{
		if (!bExtraData)
			return false;

		Skeleton::Skeleton skeleton;

		return skeleton.HasExtraData(id, NodeUQ);
	}

	void Settings::Set(const char* id, float value)
	{

	}

	void Settings::Set(const char* id, const char* str)
	{

	}

	void Settings::Set(const char* id, int value)
	{
		for (auto& s : vSettings)
			if (_strcmpi(s.first.c_str(), id) == 0)
				switch (s.second) {

				case SettingsIndex::ReEquipType: iReEquipType = static_cast<QuiverReEquipType>(value); return;

				}
	}

	void Settings::Set(const char* id, bool value)
	{
		for (auto& s : vSettings)
			if (_strcmpi(s.first.c_str(), id) == 0)
				switch (s.second) {

				case SettingsIndex::EnablePC: bEnablePC = value; return;

				case SettingsIndex::EnableNPC: bEnableNPC = value; return;

				case SettingsIndex::Spell: bSpell = value; return;

				case SettingsIndex::Weapon: bWeapon = value; return;

				case SettingsIndex::Shield: bShield = value; return;

				case SettingsIndex::Bow: bBow = value; return;

				case SettingsIndex::Crossbow: bCrossbow = value; return;

				case SettingsIndex::CheckWeaponByKeywords: bCheckWeaponByKeywords = value; return;

				case SettingsIndex::EquipStronger: bEquipStronger = value; return;

				case SettingsIndex::EquipLargerAmount: bEquipLargerAmount = value; return;

				case SettingsIndex::Savefile: bSavefile = value; return;

				case SettingsIndex::MultiBow: bMultiBow = value; return;

				case SettingsIndex::BlackListAmmo: bBlackListAmmo = value; return;

				case SettingsIndex::BlackListRace: bBlackListRace = value; return;

				case SettingsIndex::BlackListCharacter: bBlackListCharacter = value; return;

				case SettingsIndex::ExtraData: bExtraData = value; return;

				case SettingsIndex::HideQuiverOnSheathe: bHideQuiverOnSheathe[CharacterType::PC] = value; return;

				case SettingsIndex::HideQuiverOnDraw: bHideQuiverOnDraw[CharacterType::PC] = value; return;

				case SettingsIndex::HideBoltOnSheathe: bHideBoltOnSheathe[CharacterType::PC] = value; return;

				case SettingsIndex::HideBoltOnDraw: bHideBoltOnDraw[CharacterType::PC] = value; return;

				case SettingsIndex::HideQuiverOnSheatheNPC: bHideQuiverOnSheathe[CharacterType::NPC] = value; return;

				case SettingsIndex::HideQuiverOnDrawNPC: bHideQuiverOnDraw[CharacterType::NPC] = value; return;

				case SettingsIndex::HideBoltOnSheatheNPC: bHideBoltOnSheathe[CharacterType::NPC] = value; return;

				case SettingsIndex::HideBoltOnDrawNPC: bHideBoltOnDraw[CharacterType::NPC] = value; return;

				}
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

	void Settings::ParseKeywords(const std::string& str)
	{
		KeysList.clear();

		ParseString(str, [&](const std::string& s) {
			KeysList.push_back(s); 
		});

		if (!KeysList.empty())
			sort(KeysList);	
	}

	void Settings::ParseBlackList(const std::string& str, BlackList& blackList)
	{
		blackList.clear();

		std::regex reg{ "(\\w*.\\w*):(\\w*)"};
		std::smatch matches;

		char *end;

		DataHandler * datahandler = DataHandler::GetSingleton();

		if (datahandler) {

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

						blackList.push_back(id);

						_DMESSAGE("%s\t:\t 0x%.8X", matches[1].str().c_str(), id);
					}
				}
			});
		}

		if (!blackList.empty())
			sort(blackList);
	}

	Settings Settings::instance;
};
