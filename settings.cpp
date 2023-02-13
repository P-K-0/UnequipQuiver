
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
		READSETTINGS(bFavorites);

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

	const bool Settings::CheckFavoritesAmmo(InventoryEntryData* item)
	{
		if (!bFavorites)
			return false;

		if (item->extendDataList)
			for (UInt32 idx = 0; idx < item->extendDataList->Count(); idx++) {

				BaseExtraList* extraLst = item->extendDataList->GetNthItem(idx);

				if (extraLst && item->type)
					return extraLst->HasType(kExtraData_Hotkey);
			}

		return false;
	}

	void Settings::Set(const char* id, float value)
	{

	}

	void Settings::Set(const char* id, const char* str)
	{

	}

	void Settings::Set(const char* id, int value)
	{
		switch (std::hash<std::string>{}(id)) {

		case "iReEquipType"_hash:

			iReEquipType = static_cast<QuiverReEquipType>(value); break;
		}
	}

	void Settings::Set(const char* id, bool value)
	{
		switch (std::hash<std::string>{}(id)) {

		case "bEnablePC"_hash: bEnablePC = value; break;

		case "bEnableNPC"_hash: bEnableNPC = value; break;

		case "bSpell"_hash: bSpell = value; break;

		case "bWeapon"_hash: bWeapon = value; break;

		case "bShield"_hash: bShield = value; break;

		case "bBow"_hash: bBow = value; break;

		case "bCrossbow"_hash: bCrossbow = value; break;

		case "bCheckWeaponByKeywords"_hash: bCheckWeaponByKeywords = value; break;

		case "bEquipStronger"_hash: bEquipStronger = value; break;

		case "bEquipLargerAmount"_hash: bEquipLargerAmount = value; break;

		case "bSavefile"_hash: bSavefile = value; break;

		case "bMultiBow"_hash: bMultiBow = value; break;

		case "bBlackListAmmo"_hash: bBlackListAmmo = value; break;

		case "bBlackListRace"_hash: bBlackListRace = value; break;

		case "bBlackListCharacter"_hash: bBlackListCharacter = value; break;

		case "bExtraData"_hash: bExtraData = value; break;

		case "bFavorites"_hash: bFavorites = value; break;

		case "bHideQuiverOnSheathe"_hash: bHideQuiverOnSheathe[CharacterType::PC] = value; break;

		case "bHideQuiverOnDraw"_hash: bHideQuiverOnDraw[CharacterType::PC] = value; break;

		case "bHideBoltOnSheathe"_hash: bHideBoltOnSheathe[CharacterType::PC] = value; break;

		case "bHideBoltOnDraw"_hash: bHideBoltOnDraw[CharacterType::PC] = value; break;

		case "bHideQuiverOnSheatheNPC"_hash: bHideQuiverOnSheathe[CharacterType::NPC] = value; break;

		case "bHideQuiverOnDrawNPC"_hash: bHideQuiverOnDraw[CharacterType::NPC] = value; break;

		case "bHideBoltOnSheatheNPC"_hash: bHideBoltOnSheathe[CharacterType::NPC] = value; break;

		case "bHideBoltOnDrawNPC"_hash: bHideBoltOnDraw[CharacterType::NPC] = value; break;

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

		std::regex reg{ R"((\s*\w*.\w*):(\w*))"};
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
