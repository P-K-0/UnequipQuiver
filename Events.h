#pragma once

#include "skse_libray_facilities.h"
#include "std_library_facilities.h"

namespace EventsDispatch {

	constexpr UInt32 PlayerID = 0x14;

	template<typename T, int N = 2>
	class LastAmmoPair {

		using vectorAmmo = std::vector<std::pair<T, T>>;
		using vectorMultiAmmo = std::vector<std::unordered_map<T, std::pair<T, T>>>;

#define GET_VALUE_AMMO(i, id, m) return id == 0 ? ammo[i] ## .m : multi_ammo[i][id] ## .m;
#define SET_VALUE_AMMO(i, id, m, v) \
						if (id == 0) \
							ammo[i] ## .m = v; \
						else \
							multi_ammo[i][id] ## .m = v; 

	public:

		LastAmmoPair(const LastAmmoPair&) = delete;
		LastAmmoPair(const LastAmmoPair&&) = delete;
		LastAmmoPair operator=(const LastAmmoPair&) = delete;
		LastAmmoPair operator=(const LastAmmoPair&&) = delete;

		explicit LastAmmoPair() { 
			ammo.assign(N, std::pair<T, T>()); 
			multi_ammo.assign(N, std::unordered_map<T, std::pair<T, T>>());
		}
		~LastAmmoPair() {}

		const int size() const { return N; }

		const T GetLast(int index, T id = 0) { GET_VALUE_AMMO(index, id, first); }
		const T GetLatch(int index, T id = 0) { GET_VALUE_AMMO(index, id, second); }

		void SetLast(int index, T value, T id = 0) { SET_VALUE_AMMO(index, id, first, value); }
		void SetLatch(int index, T value, T id = 0) { SET_VALUE_AMMO(index, id, second, value); }

		void Swap(int index, T value, T id = 0) {
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
	class TES_ ## ev : public BSTEventSink<TES ## ev> { \
	public: \
		virtual EventResult ReceiveEvent(TES ## ev * evn, EventDispatcher<TES ## ev> * dispacther); \
	}; \
	static TES_ ## ev ev;

	DECL_CLASS_EVENT(EquipEvent);
	DECL_CLASS_EVENT(LoadGameEvent);

	enum class TypeWeapon {
		Nothing,
		Bow,
		CBow,
		AnotherWeapon
	};

	extern void RegisterEventDispatch();
};
