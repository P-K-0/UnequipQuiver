
#include "Skeleton.h"

namespace Skeleton {

	template<typename Func>
	void Skeleton::Visit(const std::string& filename, Func func)
	{
		if (filename.empty()) return;

		const size_t size_mem{ sizeof(NiStream) };
		UInt8 memory[size_mem]{};

		NiStream* nistream = (NiStream*)(memory);

		std::string file{ "Data\\Meshes\\" + filename };

		BSResourceNiBinaryStream binaryStream(file.c_str());

		if (binaryStream.IsValid() && nistream) {

			NiStreamCtor(nistream);

			if (nistream->LoadStream(&binaryStream) && nistream->m_rootObjects.m_data)
				for (UInt32 idx = 0; idx < nistream->m_rootObjects.m_emptyRunStart; idx++) {

					NiObject* obj = nistream->m_rootObjects.m_data[idx];

					if (obj && func(obj))
						break;
				}

			NiStreamDtor(nistream);
		}
	}

	template<typename Func>
	void Skeleton::Visit(UInt32 id, Func func)
	{
		TESForm* frm{ LookupFormByID(id) };
		TESAmmo* ammo{ nullptr };

		if (frm && (ammo = DYNAMIC_CAST(frm, TESForm, TESAmmo)) && ammo->settings.projectile)
			Visit(ammo->settings.projectile->model.GetModelName(), func);
	}

	bool Skeleton::HasExtraData(NiAVObject* obj, const std::string& name)
	{
		BSFixedString str = name.c_str();

		if (NiExtraData* extraData = obj->GetExtraData(str)) {

			NiBooleanExtraData* boolExtraData = ni_cast(extraData, NiBooleanExtraData);

			if (boolExtraData)
				return boolExtraData->m_data;
		}

		return false;
	}

	bool Skeleton::HasExtraData(const std::string& filename, const std::string& name)
	{
		bool ret{ false };

		Visit(filename, [&](NiObject* obj) {

			NiNode* node = obj->GetAsNiNode();

			if (node) 
				for (UInt32 ichild = 0; ichild < node->m_children.m_emptyRunStart; ichild++) {

					NiAVObject* obj_av = node->m_children.m_data[ichild];

					if (obj_av && HasExtraData(obj_av, name))
						return ret = true;
				}

			return false;
			});

		return ret;
	}

	bool Skeleton::HasExtraData(UInt32 id, const std::string& name)
	{
		TESForm* frm{ LookupFormByID(id) };
		TESAmmo* ammo{ nullptr };
		bool ret{ false };

		if (frm && (ammo = DYNAMIC_CAST(frm, TESForm, TESAmmo)) && ammo->settings.projectile) {

			Visit(ammo->settings.projectile->model.GetModelName(), [&](NiObject* obj) {

				NiNode* node = obj->GetAsNiNode();

				if (node)
					for (UInt32 ichild = 0; ichild < node->m_children.m_emptyRunStart; ichild++) {

						NiAVObject* obj_av = node->m_children.m_data[ichild];

						if (obj_av && HasExtraData(obj_av, name))
							return ret = true;
					}

				return false;
				});
		}

		return ret;
	}
}