#pragma once

#include "std_library_facilities.h"
#include "skse_libray_facilities.h"
#include "Skeleton.h"
#include "Events.h"

#include "SimpleIni.h"

namespace UQ_Settings {
	
#if UNEQUIPQUIVER_EXPORTS
	constexpr char *UnequipQuiverIni = "Data\\skse\\plugins\\UnequipQuiver.ini";
#elif UNEQUIPQUIVERSE_EXPORTS || UNEQUIPQUIVERAE_EXPORTS
	constexpr char *UnequipQuiverIni = "Data\\skse\\plugins\\UnequipQuiverSE.ini";
	constexpr char* UnequipQuiverIniMCM = "Data\\MCM\\Config\\UnequipQuiverSE\\settings.ini";
	constexpr char* UnequipQuiverIniSettings = "Data\\MCM\\Settings\\UnequipQuiverSE.ini";
#endif

	enum class QuiverReEquipType {

		Default,
		Last,
		Stronger
	};

	constexpr bool Default_bEnablePC = true;
	constexpr bool Default_bEnableNPC = true;
	constexpr bool Default_bSpell = true;
	constexpr bool Default_bWeapon = true;
	constexpr bool Default_bShield = true;
	constexpr bool Default_bBow = true;
	constexpr bool Default_bCrossbow = true;
	constexpr bool Default_bCheckWeaponByKeywords = false;
	constexpr bool Default_bSavefile = false;
	constexpr bool Default_bMultiBow = false;
	constexpr bool Default_bEquipStronger = false;
	constexpr bool Default_bEquipLargerAmount = false;
	constexpr bool Default_bBlackListAmmo = true;
	constexpr bool Default_bBlackListRace = false;
	constexpr bool Default_bBlackListCharacter = false;
	constexpr bool Default_bExtraData = true;
	constexpr bool Default_bFavorites = false;
	constexpr bool Default_bHideQuiverOnSheathe = false;
	constexpr bool Default_bHideQuiverOnDraw = false;
	constexpr bool Default_bHideBoltOnSheathe = false;
	constexpr bool Default_bHideBoltOnDraw = false;
	constexpr QuiverReEquipType Default_iReEquipType = QuiverReEquipType::Last;
	constexpr char* Default_sKeywords = "WeapTypeStaff,WeapTypeDagger,WeapTypeSword,WeapTypeWarhammer,WeapTypeBattleaxe,WeapTypeWarAxe,WeapTypeGreatsword,WeapTypeMace";
	constexpr char* Default_sBlackListAmmo = "Dawnguard.esm:1A958";
	constexpr char* Default_sBlackListRace = "";
	constexpr char* Default_sBlackListCharacter = "";
	constexpr char* Default_sSavePath = "";

	constexpr char* NodeUQ = "NoUnequipAmmo";

	template<class C, typename T>
	inline bool binary_search(C& c, const T& val) { return std::binary_search(c.begin(), c.end(), val); }

	template<class C>
	inline void sort(C& c) { std::sort(c.begin(), c.end()); }

	template<typename CIn, typename  COut>
	inline void lower(const CIn& in, COut& out) { std::transform(in.begin(), in.end(), out.begin(), std::tolower); }

	class IniSettings {

	public:

		IniSettings() = delete;
		IniSettings(const IniSettings&) = delete;
		IniSettings(IniSettings&&) = delete;
		IniSettings& operator=(const IniSettings&) = delete;
		IniSettings& operator=(IniSettings&&) = delete;

		IniSettings(const std::string& Filename)
		{
			ini.SetUnicode();
			lastErr = ini.LoadFile(Filename.c_str());
		}

		~IniSettings() {}

		template<typename T>
		T ReadAs(const std::string& key, const std::string& app, T def = T()) { return ReadSetting(key, app, def); }

		operator bool() { return lastErr == 0; }

	private:

		std::string ReadSettingIni(const std::string& key, const std::string& app, const std::string def);

		int ReadSetting(const std::string& key, const std::string& app, int def) { return std::stoi(ReadSettingIni(key, app, std::to_string(def))); }
		bool ReadSetting(const std::string& key, const std::string& app, bool def) { return std::stoi(ReadSettingIni(key, app, std::to_string(def))); }
		float ReadSetting(const std::string& key, const std::string& app, float def) { return std::stof(ReadSettingIni(key, app, std::to_string(def))); }
		QuiverReEquipType ReadSetting(const std::string& key, const std::string app, QuiverReEquipType def)
			{ return static_cast<QuiverReEquipType>(ReadSetting(key, app, static_cast<int>(def))); }
		std::string ReadSetting(const std::string& key, const std::string& app, const std::string def) { return ReadSettingIni(key, app, def); }

		CSimpleIni ini;
		SI_Error lastErr{};
	};

	using CharacterType = EventsDispatch::CharacterType;

	template<class Tval>
	class CharacterSelected {

	public:

		CharacterSelected() { for (auto& e : enum_) e = Tval(); }
		CharacterSelected(const bool initValue) { for (auto& e : enum_) e = initValue; }
		virtual ~CharacterSelected() {}

		Tval& operator[](CharacterType index) { return enum_[static_cast<size_t>(index)]; }
		const Tval& operator[](CharacterType index) const { return enum_[static_cast<size_t>(index)]; }

	private:

		Tval enum_[static_cast<size_t>(CharacterType::Max)];
	};

	class Settings {

		using WeapKeywords = std::vector<std::string>;
		using BlackList = std::vector<UInt32>;
		using size_type = WeapKeywords::size_type;
		using iterator = WeapKeywords::iterator;

	public:

		Settings(const Settings&) = delete;
		Settings(Settings&&) = delete;
		Settings& operator=(const Settings&) = delete;
		Settings& operator=(Settings&&) = delete;

		static Settings& GetInstance() { return instance; }

		virtual ~Settings() {}

		void ReadSettings(const std::string& Filename, bool readDefault = true);
		void ReadAllSettings();

		const bool IsEnabledPC() const { return bEnablePC; }
		const bool IsEnabledNPC() const { return bEnableNPC; }
		const bool IsEnabledSpell() const { return bSpell; }
		const bool IsEnabledWeapon() const { return bWeapon; }
		const bool IsEnabledShield() const { return bShield; }
		const bool IsEnabledBow() const { return bBow; }
		const bool IsEnabledCrossbow() const { return bCrossbow; }
		const bool IsEnabledSavefile() const { return bSavefile; }
		const bool IsEnabledMultiBow() const { return bMultiBow; }
		const bool IsEnabledEquipStronger() const { return bEquipStronger; }
		const bool IsEnabledEquipLargerAmount() const { return bEquipLargerAmount; }
		const bool IsEnabledBlackListAmmo() const { return bBlackListAmmo; }
		const bool IsEnabledBlackListRace() const { return bBlackListRace; }
		const bool IsEnabledBlackListCharacter() const { return bBlackListCharacter; }
		const bool IsEnabledExtraData() const {	return bExtraData; }
		const bool IsHideQuiverOnSheathe(bool drawn, CharacterType type) const { return drawn ? bHideQuiverOnDraw[type] : bHideQuiverOnSheathe[type]; }
		const bool IsHideBoltOnSheathe(bool drawn, CharacterType type) const { return drawn ? bHideBoltOnDraw[type] : bHideBoltOnSheathe[type]; }

		const std::string& GetSavePath() const { return sSavePath; }

		const bool CheckWeapBykeywords() const { return bCheckWeaponByKeywords; }

		const QuiverReEquipType GetQuiverReEquipType() const { return iReEquipType; }

		const std::string& operator[](int i) const { return KeysList[i]; }

		const size_type size() const { return KeysList.size(); }
		const bool empty() const { return KeysList.empty(); }

		const iterator begin() { return KeysList.begin(); }
		const iterator end() { return KeysList.end(); }

		const bool CheckBlackListAmmo(const UInt32 id) const { return (!BlackListAmmo.empty() && bBlackListAmmo ? binary_search(BlackListAmmo, id) : false); }
		const bool CheckBlackListRace(const UInt32 id) const { return (!BlackListRace.empty() && bBlackListRace ? binary_search(BlackListRace, id) : false); }
		const bool CheckBlackListCharacter(const UInt32 id) const { return (!BlackListCharacter.empty() && bBlackListCharacter ? binary_search(BlackListCharacter, id) : false); }

		const bool CheckExtraDataAmmo(const UInt32 id) const;
		const bool CheckFavoritesAmmo(InventoryEntryData* item);

		void Set(const char* id, float value);
		void Set(const char* id, const char* str);
		void Set(const char* id, int value);
		void Set(const char* id, bool value);

	private:

		Settings() {}

		bool bEnablePC{ Default_bEnablePC };
		bool bEnableNPC{ Default_bEnableNPC };
		bool bSpell{ Default_bSpell };
		bool bWeapon{ Default_bWeapon };
		bool bShield{ Default_bShield };
		bool bBow{ Default_bBow };
		bool bCrossbow{ Default_bCrossbow };
		bool bCheckWeaponByKeywords{ Default_bCheckWeaponByKeywords };
		bool bSavefile{ Default_bSavefile };
		bool bMultiBow{ Default_bMultiBow };
		bool bEquipStronger{ Default_bEquipStronger };
		bool bEquipLargerAmount{ Default_bEquipLargerAmount };
		bool bBlackListAmmo{ Default_bBlackListAmmo };
		bool bBlackListRace{ Default_bBlackListRace };
		bool bBlackListCharacter{ Default_bBlackListCharacter};
		bool bExtraData{ Default_bExtraData };
		bool bFavorites{ Default_bFavorites };

		CharacterSelected<bool> bHideQuiverOnSheathe{ Default_bHideQuiverOnSheathe };
		CharacterSelected<bool> bHideQuiverOnDraw{ Default_bHideQuiverOnDraw };
		CharacterSelected<bool> bHideBoltOnSheathe{ Default_bHideBoltOnSheathe };
		CharacterSelected<bool> bHideBoltOnDraw{ Default_bHideBoltOnDraw };

		QuiverReEquipType iReEquipType{ Default_iReEquipType };

		std::string sSavePath;
		std::string sKeywords;

		std::string sBlackListAmmo;
		std::string sBlackListRace;
		std::string sBlackListCharacter;

		WeapKeywords KeysList;

		BlackList BlackListAmmo;
		BlackList BlackListRace;
		BlackList BlackListCharacter;

		void ParseKeywords(const std::string& str);
		void ParseBlackList(const std::string& str, BlackList& blackList);

		static Settings instance;
	};
};
