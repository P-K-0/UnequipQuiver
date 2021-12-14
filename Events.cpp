
#include "Events.h"
#include "settings.h"

#if UNEQUIPQUIVERSE_EXPORTS || UNEQUIPQUIVERAE_EXPORTS
#include "Addresses.h"
#endif

namespace EventsDispatch {

#define UNEQUIPITEMEX(actor, item, sound) \
	bool checkRet = UQ_Settings::UQSettings.CheckBlackListAmmo(item->type->formID) | UQ_Settings::UQSettings.CheckExtraDataAmmo(item->type->formID); \
    if (!checkRet) papyrusActor::UnequipItemEx(actor, item->type, 0, false, sound); 

#define UNEQUIPITEM(actor, item) UNEQUIPITEMEX(actor, item, true)

#define EQUIPITEM(actor, item) papyrusActor::EquipItemEx(actor, item, 0, false, false)

	LastAmmoEquipped lastAmmo;

	bool IsWorn(InventoryEntryData* item)
	{
		if (item->extendDataList) 
			for (UInt32 idx = 0; idx < item->extendDataList->Count(); idx++) {

				BaseExtraList* extraLst = item->extendDataList->GetNthItem(idx);

				if (extraLst && item->type)
					return extraLst->HasType(kExtraData_Worn);
			}

		return false;
	}

	inline SInt32 CountItems(Actor* actor, TESForm* item)
	{
		ExtraContainerChanges* containerChanges = static_cast<ExtraContainerChanges*>(actor->extraData.GetByType(kExtraData_ContainerChanges));

		if (containerChanges && containerChanges->data && actor->baseForm) {

			TESContainer* container = DYNAMIC_CAST(actor->baseForm, TESForm, TESContainer);
			InventoryEntryData* entryData = containerChanges->data->FindItemEntry(item);

			if (container && entryData) {

				SInt32 baseCount = container->CountItem(item);
				SInt32 itemCount = entryData->countDelta;

				return baseCount + itemCount;
			}
		}

		return 0;
	}

	template<typename Func = std::function<bool(InventoryEntryData*)>>
	void VisitContainer(Actor* actor, Func func)
	{
		ExtraContainerChanges* containerChanges = static_cast<ExtraContainerChanges*>(actor->extraData.GetByType(kExtraData_ContainerChanges));

		ExtraContainerChanges::Data* containerData = containerChanges ? containerChanges->data : NULL;

		if (containerData && containerData->objList)
			for (int index_item = 0; index_item < containerData->objList->Count(); index_item++) {

				InventoryEntryData* entryData = containerData->objList->GetNthItem(index_item);

				if (entryData && entryData->type && entryData->type->IsAmmo())
					if (func(entryData))
						break;
			}
	}

	bool IsShield(TESForm* form)
	{
		TESObjectARMO* armo = nullptr;

		if (form && form->IsArmor() && (armo = DYNAMIC_CAST(form, TESForm, TESObjectARMO)))
			for (int i = 0; i < armo->keyword.numKeywords; i++) {

				BGSKeyword* key = armo->keyword.keywords[i];

				if (key && std::string(key->keyword.Get()) == "ArmorShield")
					return true;
			}

		return false;
	}

	TypeWeapon IsBow(TESForm* form)
	{
		TESObjectWEAP* weap = nullptr;

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

	inline int IsBolt(TESForm* form)
	{
		TESAmmo* ammo = nullptr;

		if (form && (ammo = DYNAMIC_CAST(form, TESForm, TESAmmo)))
			return static_cast<int>(ammo->isBolt()) & 1;

		return 0;
	}

	inline bool IsSpell(TESForm* form)
	{
		if (form && form->formType == kFormType_Spell) {

			SpellItem* spell = DYNAMIC_CAST(form, TESForm, SpellItem);

			if (spell && (spell->data.type == SpellItem::kTypeSpell))
				return true;
		}

		return false;
	}

	bool IsWeapon(TESForm* form)
	{
		using namespace UQ_Settings;

		TESObjectWEAP* weap = nullptr;

		if (form && form->IsWeapon() && (weap = DYNAMIC_CAST(form, TESForm, TESObjectWEAP)) && !UQSettings.empty())
			for (int i = 0; i < weap->keyword.numKeywords; i++) {

				BGSKeyword* keyword = weap->keyword.keywords[i];

				if (keyword && binary_search(UQSettings, keyword->keyword.Get()))
					return true;
			}

		return false;
	}

	inline bool IsActorEnabled(Actor* act)
	{
		using namespace UQ_Settings;

		if (act->IsDead(1))
			return false;

		if (act->formID == PlayerID && !UQSettings.IsEnabledPC())
			return false;

		if (act->formID != PlayerID && !UQSettings.IsEnabledNPC())
			return false;

		return true;
	}
	void HideQuiver(Actor* act, bool sheathe = true)
	{
		if (!IsActorEnabled(act)) return;

		if (TESForm* weap = act->GetEquippedObject(true))
			switch (IsBow(weap)) {

			case TypeWeapon::Bow:

				hideNode.Hide(act, UQ_Settings::UQSettings.IsHideQuiverOnSheathe(sheathe));
				break;

			case TypeWeapon::CBow:

				hideNode.Hide(act, UQ_Settings::UQSettings.IsHideBoltOnSheathe(sheathe));
				break;

			default:

				VisitContainer(act, [&](InventoryEntryData* entry) {

					UNEQUIPITEMEX(act, entry, false);

					return false;
				});

				break;
			}
	}

	static UInt32 last_ammo{ 0 };
	static UInt32 last_weap{ 0 };

	void OnEquip(Actor* actor, TESForm* form)
	{
		using namespace UQ_Settings;

		if (!actor || !form)
			return;

		UInt32 refID{ actor->formID };
		TESAmmo* ammo = nullptr;
		TESForm* frm_damage = nullptr;
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

				VisitContainer(actor, [&](InventoryEntryData* entry) {

					UNEQUIPITEM(actor, entry);

					return false;
					});
			}
			else {

				UInt32 weap_id{ refID == PlayerID && UQSettings.IsEnabledMultiBow() ? form->formID : 0 };

				VisitContainer(actor, [&](InventoryEntryData* entry) {

					isBolt = IsBolt(entry->type);

					if ((isBolt && isBow == TypeWeapon::CBow) || (!isBolt && isBow == TypeWeapon::Bow))
						switch (UQSettings.GetQuiverReEquipType()) {
						case QuiverReEquipType::QRE_DEFAULT:

							// fix NPCs
							if (refID != PlayerID) {

								EQUIPITEM(actor, entry->type);

								return true;
							}

							break;

						case QuiverReEquipType::QRE_LAST:

							if (lastAmmo[refID].GetLast(isBolt, weap_id) == entry->type->formID) {

								EQUIPITEM(actor, entry->type);

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
					EQUIPITEM(actor, frm_damage);
			}

			HideQuiver(actor);
		}
		else {

			if (refID == PlayerID && last_ammo != 0 && UQSettings.IsEnabledEquipStronger()) {

				int isBoltEquipped = IsBolt(form);
				bool isFound{ false };

				VisitContainer(actor, [&](InventoryEntryData* entry) {

					isBolt = IsBolt(entry->type);

					if (isBolt == isBoltEquipped) {

						if ((ammo = DYNAMIC_CAST(entry->type, TESForm, TESAmmo)) && damage < ammo->settings.damage) {

							damage = ammo->settings.damage;
							frm_damage = entry->type;
						}

						if (!isFound && CountItems(actor, entry->type) > 0)
							isFound = last_ammo == entry->type->formID;
					}

					return false;
					});

				last_ammo = 0;

				if (damage > 0 && frm_damage && !isFound) {

					EQUIPITEM(actor, frm_damage);
					
					return;
				}				
			}

			if (UQSettings.GetQuiverReEquipType() == QuiverReEquipType::QRE_LAST) {

				auto& la = lastAmmo[refID];
				UInt32 weap_id{ 0 };

				if (refID == PlayerID && UQSettings.IsEnabledMultiBow()) {

					TESForm* weap = actor->GetEquippedObject(true);
					TypeWeapon isBow{ TypeWeapon::Nothing };

					if (weap && (isBow = IsBow(weap)) == TypeWeapon::Bow || isBow == TypeWeapon::CBow)
						weap_id = weap->formID;
				}

				la.Swap(IsBolt(form), form->formID, weap_id);
			}
		}
	}

	void OnUnEquip(Actor* actor, TESForm* form)
	{
		using namespace UQ_Settings;

		if (!actor || !form) return;

		UInt32 refID{ actor->formID };

		if (!form->IsAmmo()) {

			TypeWeapon isBow{ IsBow(form) };

			if (!UQSettings.IsEnabledBow() && isBow == TypeWeapon::Bow)
				return;

			if (!UQSettings.IsEnabledCrossbow() && isBow == TypeWeapon::CBow)
				return;

			if (isBow == TypeWeapon::Bow || isBow == TypeWeapon::CBow) {

				if (refID = PlayerID && UQSettings.IsEnabledMultiBow())
					last_weap = form->formID;

				VisitContainer(actor, [&](InventoryEntryData* entry) {

					UNEQUIPITEM(actor, entry);

					return false;
					});
			}
		}
		else {

			if (refID == PlayerID) 
				last_ammo = form->formID;

			if (UQSettings.GetQuiverReEquipType() == QuiverReEquipType::QRE_LAST) {

				int isBolt = IsBolt(form);

				auto& la = lastAmmo[refID];
				UInt32 weap_id{ 0 };

				if (refID == PlayerID && UQSettings.IsEnabledMultiBow()) 
					weap_id = last_weap;

				if (la.GetLatch(isBolt, weap_id) == 0)
					la.SetLatch(isBolt, form->formID, weap_id);
			}
		}
	}

	template<typename T, typename Func = std::function<void(Actor*)>>
	inline auto ActorEvent(T refr, Func func)
	{
		using namespace UQ_Settings;

		Actor* act = nullptr;

		if (refr && (act = DYNAMIC_CAST(refr, TESObjectREFR, Actor)) && IsActorEnabled(act))
			func(act);

		return kEvent_Continue;
	}

	void VisitCell(TESObjectCELL* cell)
	{
		if (cell) {

#if UNEQUIPQUIVERSE_EXPORTS || UNEQUIPQUIVERAE_EXPORTS

			for (UInt32 idx = 0; idx < cell->refData.maxSize; idx++) {

				if (cell->refData.refArray[idx].unk08 && cell->refData.refArray[idx].ref) {

					ActorEvent(cell->refData.refArray[idx].ref, [&](Actor* act) {
#else
			for (UInt32 idx = 0; idx < cell->objectList.count; idx++) {

				if (cell->objectList[idx]) {

					ActorEvent(cell->objectList[idx], [&](Actor* act) {
#endif	

						HideQuiver(act);

					});
				}	
			}
		}
	}

	EventResult CLS_TESEquipEvent::ReceiveEvent(TESEquipEvent* evn, EventDispatcher<TESEquipEvent>* dispatcher)
	{
		if (!evn) return kEvent_Continue;

		return ActorEvent(evn->reference, [&](Actor* act) {

			TESForm* form = LookupFormByID(evn->object);

			if (form && form->Has3D())
				evn->equipped ? OnEquip(act, form) : OnUnEquip(act, form);
			});
	}

	EventResult CLS_TESLoadGameEvent::ReceiveEvent(TESLoadGameEvent* evn, EventDispatcher<TESLoadGameEvent>* dispatcher)
	{
		if (!evn) return kEvent_Continue;

		TESForm* form = LookupFormByID(PlayerID);
		TESObjectREFR* refr = nullptr;

		if (form && (refr = DYNAMIC_CAST(form, TESForm, TESObjectREFR)))
			VisitCell(refr->parentCell);

		return kEvent_Continue;
	}

	EventResult CLS_TESObjectLoadedEvent::ReceiveEvent(TESObjectLoadedEvent* evn, EventDispatcher<TESObjectLoadedEvent>* dispatcher)
	{
		if (!evn || !evn->loaded) return kEvent_Continue;

		TESForm* form = LookupFormByID(evn->formId);
		TESObjectREFR* refr = nullptr;

		if (form && (refr = DYNAMIC_CAST(form, TESForm, TESObjectREFR)))
			ActorEvent(refr, [&](Actor* act) { HideQuiver(act); });

		return kEvent_Continue;
	}

	EventResult CLS_TESInitScriptEvent::ReceiveEvent(TESInitScriptEvent* evn, EventDispatcher<TESInitScriptEvent>* dispatcher)
	{
		if (evn && evn->reference)
			ActorEvent(evn->reference, [&](Actor* act) { HideQuiver(act); });

		return kEvent_Continue;
	}
	
	EventResult CLS_SKSEActionEvent::ReceiveEvent(SKSEActionEvent* evn, EventDispatcher<SKSEActionEvent>* dispatcher)
	{
		if (evn && evn->actor) 
			switch (evn->type) {

			case SKSEActionEvent::kType_BeginDraw:

				HideQuiver(evn->actor, false);
				break;

			case SKSEActionEvent::kType_BeginSheathe:

				HideQuiver(evn->actor);
				break;
			}

		return kEvent_Continue;
	}

	SKSEMessagingInterface* skse_msg_interface = nullptr;

	void RegisterEventDispatch()
	{
		static bool InitDispatcher{ false };

		if (InitDispatcher) return;

#if UNEQUIPQUIVER_EXPORTS
		g_EquipEventDispatcher->AddEventSink(&g_TESEquipEvent);
		g_LoadGameEventDispatcher->AddEventSink(&g_TESLoadGameEvent);
		g_initScriptEventDispatcher->AddEventSink(&g_TESInitScriptEvent);
		g_objectLoadedEventDispatcher->AddEventSink(&g_TESObjectLoadedEvent);

#elif UNEQUIPQUIVERSE_EXPORTS || UNEQUIPQUIVERAE_EXPORTS

#define ADD_EVENT(ev, member) GetEventDispatcherList()-> ## member ##.AddEventSinkAddr(&g_ ## ev, Addresses::AddEventSink_Internal_Addr); 

		ADD_EVENT(TESEquipEvent, equipDispatcher);
		ADD_EVENT(TESLoadGameEvent, loadGameEventDispatcher);
		ADD_EVENT(TESInitScriptEvent, initScriptDispatcher);
		ADD_EVENT(TESObjectLoadedEvent, objectLoadedDispatcher);
#endif

		if (skse_msg_interface) {

			auto skse_action = static_cast<EventDispatcher<SKSEActionEvent>*>(skse_msg_interface->GetEventDispatcher(SKSEMessagingInterface::kDispatcher_ActionEvent));
			
			if (skse_action) {

				skse_action->AddEventSinkAddr(&g_SKSEActionEvent, Addresses::AddEventSink_Internal_Addr);

				Hooks_Handlers_Commit();
			}
		}

		InitDispatcher = true;

		_DMESSAGE("Event dispatcher registered successfully!");
	}
};
