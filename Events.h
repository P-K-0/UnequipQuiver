#pragma once

#include "skse_libray_facilities.h"
#include "std_library_facilities.h"

namespace EventsDispatch {

	class TaskEquip : public TaskDelegate {

	public:

		TaskEquip(Actor* act, TESForm* frm)
			: actor{ act }, form{ frm }	{}
		~TaskEquip() {}

		void Run();
		void Dispose() {}

	private:

		Actor* actor{ nullptr };
		TESForm* form{ nullptr };
	};

	class HideNode {

	public:

		explicit HideNode(const std::initializer_list<std::string>& list)
			{ std::copy(list.begin(), list.end(), std::back_inserter(vStrNodes)); }
		HideNode() {}
		virtual ~HideNode() {}

		void Hide(Actor* actor, bool hide);

	private:

		std::vector<std::string> vStrNodes;
		NiNode* root{ nullptr };
		NiAVObject* obj{ nullptr };
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

		_NODISCARD const int size() const { return N; }

		_NODISCARD const T GetLast(int index, T id = T()) { return id == T() ? vAmmo[index] : vmAmmo[index][id]; }

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
				
				//(*it).first;	= ID
				//(*it).second;	= Last
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

		VectorHideNode(const VectorHideNode&) = delete;
		VectorHideNode& operator=(const VectorHideNode&) = delete;
		VectorHideNode(VectorHideNode&&) = delete;
		VectorHideNode& operator=(VectorHideNode&&) = delete;

		VectorHideNode() 
		{
			push_back(HideNode{ "QUIVER", "QUIVERChesko", "QUIVERLeftHipBolt", "HDT QUIVER" });
			push_back(HideNode{ "QUIVER", "BOLTDefault", "BOLTChesko", "BOLTLeftHipBolt", "BOLT", "BOLTXP32", "BOLT_QUIVER", "BOLTABQ" });
		}

		virtual ~VectorHideNode() {}

		_NODISCARD HideNode& operator[](TypeNode index) { return data()[static_cast<int>(index)]; }
	};

	enum class CharacterType : std::uint32_t {

		PC,
		NPC,
		Max
	};

	enum class TypeWeapon : std::uint32_t {
		Nothing,
		Bow,
		CBow,
		AnotherWeapon
	};

	enum class FlagsLastAmmo : std::uint32_t {

		Nothing,
		Stronger,
		Amount,
		StrongerAmount
	};

	static inline FlagsLastAmmo& operator|=(FlagsLastAmmo& a, const FlagsLastAmmo& b)
	{
		a = static_cast<FlagsLastAmmo>(static_cast<std::uint32_t>(a) | static_cast<std::uint32_t>(b));

		return a;
	}

	class Events :
		public BSTEventSink<TESEquipEvent>,
		public BSTEventSink<TESLoadGameEvent>,
		public BSTEventSink<TESObjectLoadedEvent>,
		public BSTEventSink<TESInitScriptEvent>,
		public BSTEventSink<SKSEActionEvent>,
		public BSTEventSink<TESSwitchRaceCompleteEvent> {

	public:

		using LastAmmoEquipped = std::unordered_map<UInt32, LastAmmo<>>;

		Events(const Events&) = delete;
		Events& operator=(const Events&) = delete;
		Events(Events&&) = delete;
		Events& operator=(Events&&) = delete;

		virtual ~Events() {}

		virtual EventResult ReceiveEvent(TESEquipEvent* evn, EventDispatcher<TESEquipEvent>* dispatcher);
		virtual EventResult ReceiveEvent(TESLoadGameEvent* evn, EventDispatcher<TESLoadGameEvent>* dispatcher);
		virtual EventResult ReceiveEvent(TESObjectLoadedEvent* evn, EventDispatcher<TESObjectLoadedEvent>* dispatcher);
		virtual EventResult ReceiveEvent(TESInitScriptEvent* evn, EventDispatcher<TESInitScriptEvent>* dispatcher);
		virtual EventResult ReceiveEvent(TESSwitchRaceCompleteEvent* evn, EventDispatcher<TESSwitchRaceCompleteEvent>* dispatcher);
		virtual EventResult ReceiveEvent(SKSEActionEvent* evn, EventDispatcher<SKSEActionEvent>* dispatcher);

		static void Register();

		static LastAmmoEquipped& GetLastAmmoEquipped() { return lastAmmo; }

	private:

		Events() {}

		template<typename Func = std::function<bool(InventoryEntryData*)>>
		void VisitContainer(Actor* actor, Func func);

		void VisitCell(TESObjectCELL* cell);

		template<typename T, typename Func = std::function<void(Actor*, CharacterType)>>
		EventResult ActorEvent(T refr, Func func);

		SInt32 CountItems(Actor* actor, TESForm* item);

		bool IsWorn(InventoryEntryData* item);
		bool IsShield(TESForm* form);
		TypeWeapon IsBow(TESForm* form);
		int IsBolt(TESForm* form);
		bool IsSpell(TESForm* form);
		bool IsWeapon(TESForm* form);
		bool IsInventoryOpen();

		bool IsActorEnabled(Actor* actor, CharacterType& charType);

		const CharacterType GetCharacterType(Actor* actor) { return (actor == (*g_thePlayer)) ? CharacterType::PC : CharacterType::NPC; }

		void EquipQuiver(Actor* act, TESForm* form, CharacterType charType);
		void UnequipQuiver(Actor* act, TESForm* form = nullptr, TypeWeapon isBow = TypeWeapon::Nothing);

		void HideQuiver(Actor* act, CharacterType charType, bool drawn);
		void HideQuiver(Actor* act, CharacterType charType) { HideQuiver(act, charType, act->actorState.IsWeaponDrawn()); }

		void OnEquip(Actor* actor, TESForm* form, CharacterType charType);
		void OnUnEquip(Actor* actor, TESForm* form, CharacterType charType);

		void CheckLastAmmo(Actor* actor, TESForm* form, CharacterType& charType);
		inline void SetLastAmmo(CharacterType& charType, UInt32 lastAmmo) { if (charType == CharacterType::PC) { lastPlayerAmmo = lastAmmo; } }

		static VectorHideNode hideNode;
		static LastAmmoEquipped lastAmmo;
		
		UInt32 lastPlayerAmmo;

		static Events instance;
	};

	inline const UInt32 GetPlayerID() { return (*g_thePlayer)->formID; }
};
