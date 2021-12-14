#pragma once

#include "skse_libray_facilities.h"
#include "std_library_facilities.h"

namespace EventsDispatch {

	constexpr UInt32 PlayerID = 0x14;

	class HideNode {

	public:

		HideNode(const HideNode&) = delete;
		HideNode(HideNode&&) = delete;
		HideNode& operator=(const HideNode&) = delete;
		HideNode& operator=(HideNode&&) = delete;

		HideNode() = delete;
		
		HideNode(const std::initializer_list<std::string>& list)
		{
			for (auto& l : list)
				vStrNodes.push_back(l);
		}

		void Hide(Actor* actor, bool hide)
		{
			BSFixedString nodeName;

			if ((root = actor->GetNiRootNode(0))) 
				for (auto& v : vStrNodes) {

					nodeName = v.c_str();

					if ((obj = root->GetObjectByName(&nodeName.data))) 
						hide ? obj->m_flags |= NiAVObject::kFlag_Hidden : obj->m_flags &= ~(NiAVObject::kFlag_Hidden);
				}
		}

		virtual ~HideNode() {}

	private:

		std::vector<std::string> vStrNodes;
		NiNode* root = nullptr;
		NiAVObject* obj = nullptr;
	};

	static HideNode hideNode{ "QUIVER", "QUIVERChesko", "QUIVERLeftHipBolt", "HDT QUIVER", "BOLTDefault", "BOLTChesko", "BOLTLeftHipBolt", "BOLT", "BOLTXP32", "BOLT_QUIVER", "BOLTABQ" };

	template<typename T, int N = 2>
	class LastAmmoPair {

		using pair = std::pair<T, T>;
		using vectorAmmo = std::vector<pair>;
		using vectorMultiAmmo = std::vector<std::unordered_map<T, pair>>;

#define GET_VALUE_AMMO(i, id, m) return id == 0 ? ammo[i] ## .m : multi_ammo[i][id] ## .m;
#define SET_VALUE_AMMO(i, id, m, v) \
						if (id == 0) \
							ammo[i] ## .m = v; \
						else \
							multi_ammo[i][id] ## .m = v; 

	public:

		LastAmmoPair(const LastAmmoPair&) = delete;
		LastAmmoPair(LastAmmoPair&&) = delete;
		LastAmmoPair& operator=(const LastAmmoPair&) = delete;
		LastAmmoPair& operator=(LastAmmoPair&&) = delete;

		explicit LastAmmoPair() { 
			ammo.assign(N, std::pair<T, T>()); 
			multi_ammo.assign(N, std::unordered_map<T, pair>());
		}
		~LastAmmoPair() {}

		const int size() const { return N; }

		const T GetLast(int index, T id = T()) { GET_VALUE_AMMO(index, id, first); }
		const T GetLatch(int index, T id = T()) { GET_VALUE_AMMO(index, id, second); }

		void SetLast(int index, T value, T id = T()) { SET_VALUE_AMMO(index, id, first, value); }
		void SetLatch(int index, T value, T id = T()) { SET_VALUE_AMMO(index, id, second, value); }

		void Swap(int index, T value, T id = T()) {
			auto& v = id == 0 ? ammo[index] : multi_ammo[index][id];
			v.first = v.second;
			v.second = value;
		}

		void ClearMultiBow() {
			for (auto& m_a : multi_ammo)
				m_a.clear();
		}

		template<typename Func = std::function<void(T, T, T)>>
		void Visit(Func f)
		{
			for (UInt32 i = 0; i < ammo.size(); i++) {
				f(0, ammo[i].second, i);
				
				auto& b = multi_ammo[i];
				for (auto& it = b.begin(); it != b.end(); it++)
					f((*it).first, (*it).second.second, i);
			}
		}

	private:

		vectorAmmo ammo;
		vectorMultiAmmo multi_ammo;
	};

	using LastAmmoEquipped = std::unordered_map<UInt32, LastAmmoPair<UInt32>>;

	extern LastAmmoEquipped lastAmmo;

#define DECL_CLASS_EVENT(ev) \
	class CLS_ ## ev : public BSTEventSink<ev> { \
	public: \
		virtual EventResult ReceiveEvent(ev * evn, EventDispatcher<ev> * dispacther); \
	}; \
	static CLS_ ## ev g_ ## ev;
	
	DECL_CLASS_EVENT(TESEquipEvent); 
	DECL_CLASS_EVENT(TESLoadGameEvent);
	DECL_CLASS_EVENT(TESObjectLoadedEvent);
	DECL_CLASS_EVENT(TESInitScriptEvent);
	DECL_CLASS_EVENT(SKSEActionEvent);
	
	enum class TypeWeapon {
		Nothing,
		Bow,
		CBow,
		AnotherWeapon
	};

	extern void RegisterEventDispatch();

	extern SKSEMessagingInterface* skse_msg_interface;
};
