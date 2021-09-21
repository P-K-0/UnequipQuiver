#pragma once

#include "std_library_facilities.h"
#include "skse_libray_facilities.h"

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
	constexpr bool Default_bBlackListAmmo = true;
	constexpr QuiverReEquipType Default_iReEquipType = QuiverReEquipType::QRE_LAST;

	constexpr char *Default_sKeywords = "WeapTypeStaff,WeapTypeDagger,WeapTypeSword,WeapTypeWarhammer,WeapTypeBattleaxe,WeapTypeWarAxe,WeapTypeGreatsword,WeapTypeMace";

	constexpr char *Default_sBlackListAmmo = "Dawnguard.esm:1A958";

	template<class C, typename T>
	inline bool binary_search(C& c, const T& val) { return std::binary_search(c.begin(), c.end(), val); }

	template<class C>
	inline void sort(C& c) { std::sort(c.begin(), c.end()); }

	class UnequipQuiver_Settings {

		using WeapKeywords = std::vector<std::string>;
		using BlackList = std::vector<UInt32>;
		using size_type = WeapKeywords::size_type;
		using iterator = WeapKeywords::iterator;

	public:

		UnequipQuiver_Settings(const UnequipQuiver_Settings&) = delete;
		UnequipQuiver_Settings(const UnequipQuiver_Settings&&) = delete;
		UnequipQuiver_Settings operator=(const UnequipQuiver_Settings&) = delete;
		UnequipQuiver_Settings operator=(const UnequipQuiver_Settings&&) = delete;

		UnequipQuiver_Settings() {}
		~UnequipQuiver_Settings() {}

		void ReadSettings();

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
		const bool IsEnabledBlackList() const { return bBlackListAmmo; }

		const std::string& GetSavePath() const { return sSavePath; }

		const bool CheckWeapBykeywords() const { return bCheckWeaponByKeywords; }

		const QuiverReEquipType GetQuiverReEquipType() const { return iReEquipType; }

		const std::string& operator[](int i) const { return KeysList[i]; }

		const size_type size() const { return KeysList.size(); }
		const bool empty() const { return KeysList.empty(); }

		const iterator begin() { return KeysList.begin(); }
		const iterator end() { return KeysList.end(); }

		const bool CheckBlackListAmmo(const UInt32 id) const { return (!BlackListAmmo.empty() && bBlackListAmmo ? binary_search(BlackListAmmo, id) : false); }

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
		bool bBlackListAmmo{ Default_bBlackListAmmo };

		std::string sSavePath;

		QuiverReEquipType iReEquipType{ Default_iReEquipType };
	
		WeapKeywords KeysList;
		BlackList BlackListAmmo;

		void ParseKeywords(const std::string& str);
		void ParseBlackList(const std::string& str);
	};

	extern std::string ReadSettingIni(const std::string& key, const std::string& def, const std::string& app = "Main", const std::string& filename = UnequipQuiverIni);

	extern UnequipQuiver_Settings UQSettings;
};

