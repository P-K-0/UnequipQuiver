#pragma once

#include "std_library_facilities.h"

namespace UQ_Settings {
	
#if UNEQUIPQUIVER_EXPORTS
	constexpr char *UnequipQuiverIni = "Data\\skse\\plugins\\UnequipQuiver.ini";
#elif UNEQUIPQUIVERSE_EXPORTS
	constexpr char *UnequipQuiverIni = "Data\\skse\\plugins\\UnequipQuiverSE.ini";
#endif

	enum class QuiverReEquipType {
		QRE_DEFAULT,
		QRE_LAST,
		QRE_STRONGER
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
	constexpr QuiverReEquipType Default_iReEquipType = QuiverReEquipType::QRE_LAST;

	class UnequipQuiver_Settings {

		const std::string StrDefaultKeywords =
			"WeapTypeStaff,WeapTypeDagger,WeapTypeSword,WeapTypeWarhammer,WeapTypeBattleaxe,WeapTypeWarAxe,WeapTypeGreatsword,WeapTypeMace";

		using WeapKeywords = std::vector<std::string>;
		using size_type = WeapKeywords::size_type;
		using iterator = WeapKeywords::iterator;

	public:

		UnequipQuiver_Settings(const UnequipQuiver_Settings&) = delete;
		UnequipQuiver_Settings(const UnequipQuiver_Settings&&) = delete;
		UnequipQuiver_Settings operator=(const UnequipQuiver_Settings&) = delete;
		UnequipQuiver_Settings operator=(const UnequipQuiver_Settings&&) = delete;

		explicit UnequipQuiver_Settings();
		~UnequipQuiver_Settings() { KeysList.clear(); }

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

		const std::string& GetSavePath() const { return sSavePath; }

		const bool CheckWeapBykeywords() const { return bCheckWeaponByKeywords; }

		const QuiverReEquipType GetQuiverReEquipType() const { return iReEquipType; }

		const std::string& operator[](int i) const { return KeysList[i]; }
		const size_type size() const { return KeysList.size(); }

		const iterator begin() { return KeysList.begin(); }
		const iterator end() { return KeysList.end(); }

	private:

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
		std::string sSavePath;

		QuiverReEquipType iReEquipType{ Default_iReEquipType };
	
		WeapKeywords KeysList;
		WeapKeywords ParseKeywords(const std::string& str);
	};

	extern std::string ReadSetting(const std::string& key, const std::string& def, const std::string& app = "Main", const std::string& filename = UnequipQuiverIni);

	extern UnequipQuiver_Settings UQSettings;
};

