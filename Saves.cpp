
#include "Saves.h"

namespace UQ_Saves {

	std::ofstream& operator<<(std::ofstream& ofs, UInt32& v) { ofs.write(as_byte(v), sizeof(UInt32)); return ofs; }

	std::ofstream& operator<<(std::ofstream& ofs, std::string& s)
	{
		UInt32 l{ static_cast<UInt32>(s.length()) };
		ofs << l;
		ofs.write(s.c_str(), l);

		return ofs;
	}

	std::ofstream& operator<<(std::ofstream& ofs, const char *s)
	{
		std::string t_s{ s };
		ofs << t_s;
		return ofs;
	}

	std::ifstream& operator>>(std::ifstream& ifs, UInt32& v) { ifs.read(as_byte(v), sizeof(UInt32)); return ifs; }

	std::ifstream& operator>>(std::ifstream& ifs, std::string& v)
	{
		UInt32 l{ 0 };
		ifs >> l;

		if (l > 0) {
			char *str_temp = new char[l + 1];
			ifs.read(&str_temp[0], l);
			str_temp[l] = '\0';
			v = str_temp;
			delete[] str_temp;
		}

		return ifs;
	}

#define GETVALUE_MOD(p, name, id) \
			std::string name; \
			UInt32 id{ 0 }; \
			ifs >> name >> id; \
			const ModInfo *p = datahandler->LookupModByName(name.c_str()); \
			if (p) \
				id |= (static_cast<UInt32>(p->modIndex) << 24); 

#define SETVALUE_MOD(tid, id) \
					UInt32 tid { id }; \
					mod = datahandler->modList.loadedMods[tid >> 24]; \
					if (mod) { \
						tid &= 0xffffff; \
						ofs << mod->name << tid; \
					} \
					else { \
						tid = 0; \
						ofs << "Skyrim.esm" << tid; \
					}

	UQSaves::UQSaves(const char * filename, const UQFlags& flag, EventsDispatch::LastAmmoEquipped& last)
	{
		using namespace UQ_Settings;

		std::string file_bin = UQSettings.GetSavePath().empty() ? GetSavePath() : UQSettings.GetSavePath();

		if (file_bin.empty())
			return;

		file_bin += EraseSuffix(filename);
		file_bin += ".bin";

		bool ret{ false };

		switch (flag) {
		case UQFlags::uqLoad:
			ret = Read(file_bin, last);
			break;
		case UQFlags::uqSave:	
			ret = Write(file_bin, last);
			break;
		case UQFlags::uqDelete:
			ret = Delete(file_bin);
			break;
		case UQFlags::uqNothing: 
			return;
		default:
			return;
		}

		_DMESSAGE("File : %s %s%s.", file_bin.c_str(), ret ? "" : "not ", str_flag[UQFlagsToInt(flag)].c_str());
	}

	bool UQSaves::Read(const std::string& filename, EventsDispatch::LastAmmoEquipped& last)
	{
		std::ifstream ifs{ filename, std::ios::binary };

		if (!ifs)
			return false;

		int index{ 0 };
		DataHandler * datahandler = DataHandler::GetSingleton();

		if (datahandler) {

			auto& l = last[EventsDispatch::PlayerID];

			l.ClearMultiBow();

			UInt32 index;

			while (!ifs.eof()) {

				ifs >> index;

				GETVALUE_MOD(weap_mod, weap_name, weap_id);
				GETVALUE_MOD(ammo_mod, ammo_name, ammo_id);

				if (!weap_name.empty() && !ammo_name.empty())
					l.SetLatch(index, ammo_id, weap_id);
			}

			return true;
		}

		return false;
	}

	bool UQSaves::Write(const std::string& filename, EventsDispatch::LastAmmoEquipped& last)
	{
		std::ofstream ofs{ filename, std::ios::binary };

		if (!ofs)
			return false;

		DataHandler * datahandler = DataHandler::GetSingleton();

		if (datahandler) {

			ModInfo *mod = nullptr;
			auto& l = last[EventsDispatch::PlayerID];

			l.Visit([&](UInt32 weap_id, UInt32 ammo_id, UInt32 index) {

				ofs << index;

				SETVALUE_MOD(wid, weap_id);
				SETVALUE_MOD(aid, ammo_id);
			});

			return true;
		}

		return false;
	}

	bool UQSaves::Delete(const std::string& filename) { return static_cast<bool>(::DeleteFile(filename.c_str())); }

	std::string UQSaves::EraseSuffix(const std::string& filename, const std::string& suffix)
	{
		if (filename.length() >= suffix.length()) {

			size_t npos{ filename.length() - suffix.length() };

			if (_strcmpi(filename.substr(npos).c_str(), suffix.c_str()) == 0)
				return filename.substr(0, npos);
		}

		return filename;
	}

	std::string UQSaves::GetSavePath()
	{
		if (!file_path.empty())
			return file_path;

		char path[MAX_PATH];
		HRESULT err;

		if (SUCCEEDED(err = SHGetFolderPath(NULL, CSIDL_MYDOCUMENTS, NULL, SHGFP_TYPE_CURRENT, path)) && !(file_path = path).empty()) {

			file_path = path;
			file_path += MyGamesSkyrim;

			std::string skyrim_ini{ file_path };
			skyrim_ini += "skyrim.ini";

			std::string sLocalSavePath = UQ_Settings::ReadSetting("sLocalSavePath", "", "General", skyrim_ini);

			return file_path += !sLocalSavePath.empty() ? sLocalSavePath : "Saves\\";
		}

		return "";
	}

	void LoadSaves(const void * const filename, UQFlags flags)
	{
		using namespace UQ_Settings;

		if (UQSettings.IsEnabledSavefile() && UQSettings.GetQuiverReEquipType() == QuiverReEquipType::QRE_LAST)
			UQSaves save{ static_cast<const char *>(filename), flags, EventsDispatch::lastAmmo };
	}
};
