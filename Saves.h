#pragma once

#include "skse_libray_facilities.h"
#include "std_library_facilities.h"
#include "Events.h"
#include "settings.h"

namespace UQ_Saves {

#if UNEQUIPQUIVER_EXPORTS
	constexpr char *MyGamesSkyrim = "\\My Games\\Skyrim\\";
#elif UNEQUIPQUIVERSE_EXPORTS || UNEQUIPQUIVERAE_EXPORTS
	constexpr char *MyGamesSkyrim = "\\My Games\\Skyrim Special Edition\\";
#endif

	constexpr char *SuffixEss = ".ess";

	template<typename T>
	char * as_byte(T& v)
	{
		void *addr = &v;

		return static_cast<char *>(addr);
	}

	enum class UQFlags {
		uqNothing,
		uqLoad,
		uqSave,
		uqDelete
	};

	inline int UQFlagsToInt(const UQFlags& f) { return static_cast<int>(f); }

	static const std::vector<std::string> str_flag = { "", "loaded", "saved", "deleted" };

	class UQSaves {

	public:

		UQSaves() = delete;

		UQSaves(const UQSaves&) = delete;
		UQSaves(UQSaves&&) = delete;

		UQSaves& operator=(const UQSaves&) = delete;
		UQSaves& operator=(UQSaves&&) = delete;

		explicit UQSaves(const std::string filename, const UQFlags& flag, EventsDispatch::LastAmmoEquipped& last);

		~UQSaves() {}

	private:

		bool Read(const std::string& filename, EventsDispatch::LastAmmoEquipped& last);	
		bool Write(const std::string& filename, EventsDispatch::LastAmmoEquipped& last);
		bool Delete(const std::string& filename);

		std::string EraseSuffix(const std::string& filename, const std::string& suffix = SuffixEss);
		std::string GetSavePath();

		std::string file_path;
	};

	extern void LoadSaves(const void * const filename, UQFlags flags);
};
