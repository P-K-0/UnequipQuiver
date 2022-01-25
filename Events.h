#pragma once

#include "skse_libray_facilities.h"
#include "std_library_facilities.h"

namespace EventsDispatch {

	constexpr UInt32 PlayerID = 0x14;

	class TaskEquip : public TaskDelegate {

	public:

		TaskEquip(Actor* act, TESForm* frm)
			: actor{ act }, form{ frm }	{}
		virtual ~TaskEquip() {}

		void Run();
		void Dispose() {}

	private:

		Actor* actor{ nullptr };
		TESForm* form{ nullptr };
	};

	class HideNode {

	public:

		explicit HideNode(const std::initializer_list<std::string>& list);
		HideNode() {}
		virtual ~HideNode() {}

		void Hide(Actor* actor, bool hide);

	private:

		std::vector<std::string> vStrNodes;
		NiNode* root = nullptr;
		NiAVObject* obj = nullptr;
	};

	template<typename T = UInt32, int N = 2>
	class LastAmmo {

		using unordered_map = std::unordered_map<T, T>;
		using vectorAmmo = std::vector<T>;
		using vectorMultiAmmo = std::vector<unordered_map>;

	public:

		LastAmmo(const LastAmmo&) = delete;
		LastAmmo(LastAmmo&&) = delete;
		LastAmmo& operator=(const LastAmmo&) = delete;
		LastAmmo& operator=(LastAmmo&&) = delete;

		explicit LastAmmo() {
			vAmmo.assign(N, T());
			vmAmmo.assign(N, unordered_map());
		}

		~LastAmmo() {}

		const int size() const { return N; }

		const T GetLast(int index, T id = T()) { return id == T() ? vAmmo[index] : vmAmmo[index][id]; }

		void SetLast(int index, T value, T id = T()) 
		{ 
			if (id == T())
				vAmmo[index] = value;
			else 
				vmAmmo[index][id] = value;
		}

		void ClearMultiBow() {
			for (auto& map : vmAmmo)
				map.clear();
		}

		template<typename Func = std::function<void(T, T, T)>>
		void Visit(Func f)
		{
			for (size_t i = 0; i < vAmmo.size(); i++) {
				f(T(), vAmmo[i], i);
				
				auto& b = vmAmmo[i];
				for (auto& it = b.begin(); it != b.end(); it++)
					f((*it).first, (*it).second, i);
			}
		}

	private:

		vectorAmmo vAmmo;
		vectorMultiAmmo vmAmmo;
	};

	enum class TypeNode {

		Quiver,
		Bolt
	};

	class VectorHideNode : public std::vector<HideNode> {

	public:

		VectorHideNode() 
		{
			push_back(HideNode{ "QUIVER", "QUIVERChesko", "QUIVERLeftHipBolt", "HDT QUIVER" });
			push_back(HideNode{ "BOLTDefault", "BOLTChesko", "BOLTLeftHipBolt", "BOLT", "BOLTXP32", "BOLT_QUIVER", "BOLTABQ" });
		}

		virtual ~VectorHideNode() {}

		HideNode& operator[](TypeNode index) { return data()[static_cast<int>(index)]; }
	};

	class Events :
		public BSTEventSink<TESEquipEvent>,
		public BSTEventSink<TESLoadGameEvent>,
		public BSTEventSink<TESObjectLoadedEvent>,
		public BSTEventSink<TESInitScriptEvent>,
		public BSTEventSink<SKSEActionEvent>
	{

		enum class TypeWeapon {
			Nothing,
			Bow,
			CBow,
			AnotherWeapon
		};

	public:

		using LastAmmoEquipped = std::unordered_map<UInt32, LastAmmo<>>;

		Events() {}
		virtual ~Events() {}

		virtual EventResult ReceiveEvent(TESEquipEvent* evn, EventDispatcher<TESEquipEvent>* dispatcher);
		virtual EventResult ReceiveEvent(TESLoadGameEvent* evn, EventDispatcher<TESLoadGameEvent>* dispatcher);
		virtual EventResult ReceiveEvent(TESObjectLoadedEvent* evn, EventDispatcher<TESObjectLoadedEvent>* dispatcher);
		virtual EventResult ReceiveEvent(TESInitScriptEvent* evn, EventDispatcher<TESInitScriptEvent>* dispatcher);
		virtual EventResult ReceiveEvent(SKSEActionEvent* evn, EventDispatcher<SKSEActionEvent>* dispatcher);

		LastAmmoEquipped& GetLastAmmoEquipped() { return lastAmmo; }

	private:

		VectorHideNode hideNode;
		LastAmmoEquipped lastAmmo;
		UInt32 lastPlayerAmmo{ 0 };

		template<typename Func = std::function<bool(InventoryEntryData*)>>
		void VisitContainer(Actor* actor, Func func);

		void VisitCell(TESObjectCELL* cell);

		template<typename T, typename Func = std::function<void(Actor*)>>
		EventResult ActorEvent(T refr, Func func);

		SInt32 CountItems(Actor* actor, TESForm* item);

		bool IsWorn(InventoryEntryData* item);
		bool IsShield(TESForm* form);
		TypeWeapon IsBow(TESForm* form);
		int IsBolt(TESForm* form);
		bool IsSpell(TESForm* form);
		bool IsWeapon(TESForm* form);
		bool IsActorEnabled(Actor* act);
		bool IsInventoryOpen();

		void EquipQuiver(Actor* act, TESForm* form);
		void UnequipQuiver(Actor* act, TESForm* form = nullptr, TypeWeapon isBow = TypeWeapon::Nothing);
		void HideQuiver(Actor* act, bool sheathe = true);

		void OnEquip(Actor* actor, TESForm* form);
		void OnUnEquip(Actor* actor, TESForm* form);
	};

	extern Events gEvents;

	extern void RegisterEventDispatch();
};
