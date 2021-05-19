
#include "Events.h"
#include "settings.h"

#if UNEQUIPQUIVERSE_EXPORTS
#include "Addresses.h"
#endif

namespace EventsDispatch {

	LastAmmoEquipped lastAmmo;

	template<typename Func>
	void VisitContainer(Actor *actor, Func func)
	{
		ExtraContainerChanges* containerChanges =
			static_cast<ExtraContainerChanges*>(actor->extraData.GetByType(kExtraData_ContainerChanges));

		ExtraContainerChanges::Data* containerData = containerChanges ? containerChanges->data : NULL;

		if (containerData && containerData->objList)
			for (int index_item = 0; index_item < containerData->objList->Count(); index_item++) {

				InventoryEntryData * entryData = containerData->objList->GetNthItem(index_item);

				if (entryData && entryData->type && entryData->type->IsAmmo())
					if (func(entryData))
						break;
			}
	}

	bool IsShield(TESForm *form)
	{
		TESObjectARMO * armo = nullptr;

		if (form && form->IsArmor() && (armo = DYNAMIC_CAST(form, TESForm, TESObjectARMO)))
			for (int i = 0; i < armo->keyword.numKeywords; i++) {

				BGSKeyword * key = armo->keyword.keywords[i];

				if (key && std::string(key->keyword.Get()) == "ArmorShield") 
					return true;
			}

		return false;
	}

	TypeWeapon IsBow(TESForm *form)
	{
		TESObjectWEAP *weap = nullptr;

		if (form && form->IsWeapon() && (weap = DYNAMIC_CAST(form, TESForm, TESObjectWEAP)))
			switch (weap->type()) {
			case TESObjectWEAP::GameData::kType_Bow: 
			case TESObjectWEAP::GameData::kType_Bow2:		
				return TypeWeapon::Bow;					

			case TESObjectWEAP::GameData::kType_CBow: 
			case TESObjectWEAP::GameData::kType_CrossBow:	
				return TypeWeapon::CBow;				

			default:										
				return TypeWeapon::AnotherWeapon;		
			}

		return TypeWeapon::Nothing;						
	}

	inline int IsBolt(TESForm *form)
	{
		TESAmmo *ammo = nullptr;

		if (form && (ammo = DYNAMIC_CAST(form, TESForm, TESAmmo)))
			return static_cast<int>(ammo->isBolt());

		return 0;
	}

	inline bool IsSpell(TESForm *form)
	{
		if (form && form->formType == kFormType_Spell) {

			SpellItem * spell = DYNAMIC_CAST(form, TESForm, SpellItem);

			if (spell && (spell->data.type == SpellItem::kTypeSpell))
				return true;
		}

		return false;
	}

	bool IsWeapon(TESForm *form)
	{
		using namespace UQ_Settings;

		TESObjectWEAP *weap = nullptr;

		if (form && form->IsWeapon() && (weap = DYNAMIC_CAST(form, TESForm, TESObjectWEAP)))
			for (int i = 0; i < weap->keyword.numKeywords; i++) {

				BGSKeyword *keyword = weap->keyword.keywords[i];

				if (keyword && std::binary_search(UQSettings.begin(), UQSettings.end(), keyword->keyword.Get()))
					return true;
			}

		return false;
	}

	static SInt32 last_count_ammo{ 0 };

	void OnEquip(Actor *actor, TESForm *form)
	{
		using namespace UQ_Settings;

		if (!actor || !form)
			return;

		UInt32 refID{ actor->formID };
		TESAmmo *ammo = nullptr;
		TESForm *frm_damage = nullptr;
		int isBolt{ 0 };
		float damage{ 0 };

		if (!form->IsAmmo()) {

			TypeWeapon isBow{ IsBow(form) };

			if (isBow != TypeWeapon::Bow && isBow != TypeWeapon::CBow) {

				bool isSpell{ UQSettings.IsEnabledSpell() ? IsSpell(form) : false };
				bool isShield{ UQSettings.IsEnabledShield() ? IsShield(form) : false };
				bool isWeapon{ UQSettings.IsEnabledWeapon() ? 
					(UQSettings.CheckWeapBykeywords() ? IsWeapon(form) : (isBow == TypeWeapon::AnotherWeapon)) : false };

				if (!isSpell && !isShield && !isWeapon)
					return;

				VisitContainer(actor, [&](InventoryEntryData *entry) {

					papyrusActor::UnequipItemEx(actor, entry->type, 0, false);

					return false;

				});
			}
			else {

				UInt32 weap_id{ refID == PlayerID && UQSettings.IsEnabledMultiBow() ? form->formID : 0 };

				VisitContainer(actor, [&](InventoryEntryData *entry) {

					isBolt = IsBolt(entry->type);

					if ((isBolt && isBow == TypeWeapon::CBow) || (!isBolt && isBow == TypeWeapon::Bow)) 
						switch (UQSettings.GetQuiverReEquipType()) {
						case QuiverReEquipType::QRE_DEFAULT:

							// fix NPCs
							if (refID != PlayerID) {

								papyrusActor::EquipItemEx(actor, entry->type, 0, false, false);

								return true;
							}

							break;

						case QuiverReEquipType::QRE_LAST:

							if (lastAmmo[refID].GetLast(isBolt, weap_id) == entry->type->formID) {

								papyrusActor::EquipItemEx(actor, entry->type, 0, false, false);

								return true;
							}

							break;

						case QuiverReEquipType::QRE_STRONGER: 

							if ((ammo = DYNAMIC_CAST(entry->type, TESForm, TESAmmo)) && damage < ammo->settings.damage) {

								damage = ammo->settings.damage;

								frm_damage = entry->type;
							}

							break;
						}

					return false;
				});

				if (damage > 0 && frm_damage)
					papyrusActor::EquipItemEx(actor, frm_damage, 0, false, false);
			}
		}
		else {

			if (refID == PlayerID && last_count_ammo == -1) {
			
				last_count_ammo = 0;

				if (UQSettings.IsEnabledEquipStronger()) {

					int isBoltEquipped = IsBolt(form);

					VisitContainer(actor, [&](InventoryEntryData *entry) {

						isBolt = IsBolt(entry->type);

						if (isBolt == isBoltEquipped)
							if ((ammo = DYNAMIC_CAST(entry->type, TESForm, TESAmmo)) && damage < ammo->settings.damage) {

								damage = ammo->settings.damage;
								frm_damage = entry->type;
							}

						return false;
					});

					if (damage > 0 && frm_damage)
						papyrusActor::EquipItemEx(actor, frm_damage, 0, false, false);
				}

				return;
			}

			if (UQSettings.GetQuiverReEquipType() == QuiverReEquipType::QRE_LAST) {

				auto& la = lastAmmo[refID];
				UInt32 weap_id{ 0 };

				if (refID == PlayerID && UQSettings.IsEnabledMultiBow()) {

					TESForm *weap = actor->GetEquippedObject(true);
					TypeWeapon isBow{ TypeWeapon::Nothing };

					if (weap && (isBow = IsBow(weap)) == TypeWeapon::Bow || isBow == TypeWeapon::CBow)
						weap_id = weap->formID;
				}
					
				la.Swap(IsBolt(form), form->formID, weap_id);
			}
		}
	}

	void OnUnEquip(Actor *actor, TESForm *form)
	{
		using namespace UQ_Settings;

		if (!actor || !form)
			return;

		UInt32 refID{ actor->formID };

		if (!form->IsAmmo()) {

			TypeWeapon isBow{ IsBow(form) };

			if (!UQSettings.IsEnabledBow() && isBow == TypeWeapon::Bow)
				return;

			if (!UQSettings.IsEnabledCrossbow() && isBow == TypeWeapon::CBow)
				return;

			if (isBow == TypeWeapon::Bow || isBow == TypeWeapon::CBow) {

				VisitContainer(actor, [&](InventoryEntryData *entry) {

					papyrusActor::UnequipItemEx(actor, entry->type, 0, false);

					return false;
				});
			}
		} 
		else {

			if (refID == PlayerID) 
				VisitContainer(actor, [&](InventoryEntryData * entryData) {

					if (entryData->type && form->formID == entryData->type->formID) {

						last_count_ammo = (last_count_ammo == 1 && entryData->countDelta == 1) ? -1 : entryData->countDelta;

						return true;
					}

					return false;
				});

			if (UQSettings.GetQuiverReEquipType() == QuiverReEquipType::QRE_LAST) {

				int isBolt = IsBolt(form);

				auto& la = lastAmmo[refID];
				UInt32 weap_id{ 0 };

				if (refID == PlayerID && UQSettings.IsEnabledMultiBow()) {

					TESForm *weap = actor->GetEquippedObject(true);
					TypeWeapon isBow{ TypeWeapon::Nothing };

					if (weap && (isBow = IsBow(weap)) == TypeWeapon::Bow || isBow == TypeWeapon::CBow)
						weap_id = weap->formID;
				}

				if (la.GetLatch(isBolt, weap_id) == 0)
					la.SetLatch(isBolt, form->formID, weap_id);
			}
		}
	}

	EventResult TES_EquipEvent::ReceiveEvent(TESEquipEvent * evn, EventDispatcher<TESEquipEvent> * dispatcher)
	{
		using namespace UQ_Settings;

		Actor * act = nullptr;

		if (evn && evn->reference && (act = DYNAMIC_CAST(evn->reference, TESObjectREFR, Actor))) {

			if (act->IsDead(1))
				return kEvent_Continue;

			if (act->formID == PlayerID && !UQSettings.IsEnabledPC())
				return kEvent_Continue;

			if (act->formID != PlayerID && !UQSettings.IsEnabledNPC())
				return kEvent_Continue;

			TESForm *form = LookupFormByID(evn->unk0);
			
			if (form && form->Has3D())
				if (evn->unk2 & 0x10000)
					OnEquip(act, form);
				else
					OnUnEquip(act, form);
		}

		return kEvent_Continue;
	}

	EventResult TES_ObjectLoadedEvent::ReceiveEvent(TESObjectLoadedEvent * evn, EventDispatcher<TESObjectLoadedEvent> * dispatcher)
	{	
		//TESForm *form = LookupFormByID(evn->formId);
		//Actor *actor = nullptr;

		//if (form && (actor = DYNAMIC_CAST(form, TESForm, Actor))) {
		//	
		//	TESForm *weap = actor->GetEquippedObject(true);
		//	TypeWeapon isBow{ TypeWeapon::Nothing };

		//	if (weap && (isBow = IsBow(weap)) == TypeWeapon::AnotherWeapon || isBow == TypeWeapon::Nothing) {

		//		VisitContainer(actor, [&](InventoryEntryData *entry) {

		//			papyrusActor::UnequipItemEx(actor, entry->type, 0, false);

		//			return false;
		//		});
		//	}
		//}

		return kEvent_Continue;
	}

	TES_EquipEvent EquipEvent;
	//TES_ObjectLoadedEvent ObjectLoadedEvent;

	void RegisterEventDispatch()
	{
		static bool InitDispatcher{ false };

		if (InitDispatcher) return;

#if UNEQUIPQUIVER_EXPORTS
		g_EquipEventDispatcher->AddEventSink(&EquipEvent);
		//g_objectLoadedEventDispatcher->AddEventSink(&ObjectLoadedEvent);
#elif UNEQUIPQUIVERSE_EXPORTS
		GetEventDispatcherList()->equipDispatcher.AddEventSinkAddr(&EquipEvent);
#endif

		InitDispatcher = true;

		_DMESSAGE("Event dispatcher registered successfully!");
	}
};
