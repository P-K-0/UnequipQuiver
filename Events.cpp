
#include "Events.h"
#include "settings.h"
#include "skse_plugin.h"

#if UNEQUIPQUIVERSE_EXPORTS || UNEQUIPQUIVERAE_EXPORTS
#include "Addresses.h"
#endif

namespace EventsDispatch {

#define UNEQUIPITEMEX(actor, item, sound) \
	bool checkRet = UQ_Settings::UQSettings.CheckBlackListAmmo(item->type->formID) | UQ_Settings::UQSettings.CheckExtraDataAmmo(item->type->formID); \
    if (!checkRet) papyrusActor::UnequipItemEx(actor, item->type, 0, false, sound); 

#define UNEQUIPITEM(actor, item) UNEQUIPITEMEX(actor, item, true)

#define EQUIPITEM(actor, item) papyrusActor::EquipItemEx(actor, item, 0, false, false)

	void TaskEquip::Run()
	{
		if (actor && form) {

			EQUIPITEM(actor, form);
		}
	}

	HideNode::HideNode(const std::initializer_list<std::string>& list)
	{
		for (auto& l : list)
			vStrNodes.push_back(l);
	}

	void HideNode::Hide(Actor* actor, bool hide)
	{
		BSFixedString nodeName;

		if ((root = actor->GetNiRootNode(0)))
			for (auto& v : vStrNodes) {

				nodeName = v.c_str();

				if ((obj = root->GetObjectByName(&nodeName.data)))
					hide ? obj->m_flags |= NiAVObject::kFlag_Hidden : obj->m_flags &= ~(NiAVObject::kFlag_Hidden);
			}
	}

	template<typename Func>
	void Events::VisitContainer(Actor* actor, Func func)
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

	void Events::VisitCell(TESObjectCELL* cell)
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
						UnequipQuiver(act);
					});
				}	
			}
		}
	}

	template<typename T, typename Func>
	EventResult Events::ActorEvent(T refr, Func func)
	{
		using namespace UQ_Settings;

		Actor* act = nullptr;

		if (refr && (act = DYNAMIC_CAST(refr, TESObjectREFR, Actor)) && IsActorEnabled(act))
			func(act);

		return kEvent_Continue;
	}

	SInt32 Events::CountItems(Actor* actor, TESForm* item)
	{
		ExtraContainerChanges* containerChanges = static_cast<ExtraContainerChanges*>(actor->extraData.GetByType(kExtraData_ContainerChanges));

		if (containerChanges && containerChanges->data && actor->baseForm) {

			TESContainer* container = DYNAMIC_CAST(actor->baseForm, TESForm, TESContainer);
			InventoryEntryData* entryData = containerChanges->data->FindItemEntry(item);

			if (container && entryData) 
				return container->CountItem(item) + entryData->countDelta;
		}

		return 0;
	}

	bool Events::IsWorn(InventoryEntryData* item)
	{
		if (item->extendDataList) 
			for (UInt32 idx = 0; idx < item->extendDataList->Count(); idx++) {

				BaseExtraList* extraLst = item->extendDataList->GetNthItem(idx);

				if (extraLst && item->type)
					return extraLst->HasType(kExtraData_Worn);
			}

		return false;
	}

	bool Events::IsShield(TESForm* form)
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

	Events::TypeWeapon Events::IsBow(TESForm* form)
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

	int Events::IsBolt(TESForm* form)
	{
		TESAmmo* ammo = nullptr;

		if (form && (ammo = DYNAMIC_CAST(form, TESForm, TESAmmo)))
			return static_cast<int>(ammo->isBolt()) & 1;

		return 0;
	}

	bool Events::IsSpell(TESForm* form)
	{
		if (form && form->formType == kFormType_Spell) {

			SpellItem* spell = DYNAMIC_CAST(form, TESForm, SpellItem);

			if (spell && (spell->data.type == SpellItem::kTypeSpell))
				return true;
		}

		return false;
	}

	bool Events::IsWeapon(TESForm* form)
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

	bool Events::IsActorEnabled(Actor* act)
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

	bool Events::IsInventoryOpen()
	{
		UIStringHolder* stringHolder = UIStringHolder::GetSingleton();
		MenuManager* menu = MenuManager::GetSingleton();

		if (stringHolder && menu)
			return menu->IsMenuOpen(&stringHolder->inventoryMenu);

		return false;
	}

	void Events::EquipQuiver(Actor* act, TESForm* form)
	{
		if (act->formID != PlayerID) {

			EQUIPITEM(act, form);
		}
		else {

			if (skse_plugin::SKSE_Plugin::skse_task_interface && !IsInventoryOpen())
				skse_plugin::SKSE_Plugin::skse_task_interface->AddTask(new TaskEquip(act, form));
			else {

				EQUIPITEM(act, form);
			}
		}
	}

	void Events::UnequipQuiver(Actor* act, TESForm* form, TypeWeapon isBow)
	{
		if (!form)
			form = act->GetEquippedObject(true);

		if (!form)
			form = act->GetEquippedObject(false);

		if (!form) return;

		if (isBow == TypeWeapon::Nothing)
			isBow = IsBow(form);

		using namespace UQ_Settings;

		bool isSpell{ UQSettings.IsEnabledSpell() ? IsSpell(form) : false };
		bool isShield{ UQSettings.IsEnabledShield() ? IsShield(form) : false };
		bool isWeapon{ UQSettings.IsEnabledWeapon() ?
			(UQSettings.CheckWeapBykeywords() ? IsWeapon(form) : (isBow == TypeWeapon::AnotherWeapon)) : false };

		if (!isSpell && !isShield && !isWeapon) return;

		VisitContainer(act, [&](InventoryEntryData* entry) {

			UNEQUIPITEMEX(act, entry, false);

			return false;
		});
	}

	void Events::HideQuiver(Actor* act, bool sheathe)
	{
		using namespace UQ_Settings;

		if (!IsActorEnabled(act)) return;

		if (TESForm* weap = act->GetEquippedObject(true))
			switch (IsBow(weap)) {

			case TypeWeapon::Bow:

				hideNode[TypeNode::Quiver].Hide(act, UQSettings.IsHideQuiverOnSheathe(sheathe));	
				break;

			case TypeWeapon::CBow:

				hideNode[TypeNode::Bolt].Hide(act, UQSettings.IsHideBoltOnSheathe(sheathe));
				break;

			case TypeWeapon::AnotherWeapon:
			default:
				break;
			}
	}

	void Events::OnEquip(Actor* actor, TESForm* form)
	{
		using namespace UQ_Settings;

		if (!actor || !form) return;

		UInt32 refID{ actor->formID };
		TESAmmo* ammo = nullptr;
		TESForm* frm_damage = nullptr;
		int isBolt{ 0 };
		float damage{ 0 };

		if (!form->IsAmmo()) {

			TypeWeapon isBow{ IsBow(form) };

			if (isBow != TypeWeapon::Bow && isBow != TypeWeapon::CBow) {

				UnequipQuiver(actor, form, isBow);
			}
			else {

				UInt32 weap_id{ refID == PlayerID && UQSettings.IsEnabledMultiBow() ? form->formID : 0 };

				auto& lAmmo = lastAmmo[refID];

				VisitContainer(actor, [&](InventoryEntryData* entry) {

					isBolt = IsBolt(entry->type);

					if ((isBolt && isBow == TypeWeapon::CBow) || (!isBolt && isBow == TypeWeapon::Bow))
						switch (UQSettings.GetQuiverReEquipType()) {
						case QuiverReEquipType::QRE_DEFAULT:

							// fix NPCs
							if (refID != PlayerID) {

								EquipQuiver(actor, entry->type);

								return true;
							}

							break;

						case QuiverReEquipType::QRE_LAST:

							if (lAmmo.GetLast(isBolt, weap_id) == entry->type->formID) {

								EquipQuiver(actor, entry->type);

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

				if (damage > 0 && frm_damage) {

					EquipQuiver(actor, frm_damage);
				}
			}

			HideQuiver(actor);
		}
		else {

			if (refID == PlayerID && lastPlayerAmmo != 0 && UQSettings.IsEnabledEquipStronger()) {

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
							isFound = lastPlayerAmmo == entry->type->formID;
					}

					return false;
				});

				lastPlayerAmmo = 0;

				if (damage > 0 && frm_damage && !isFound) {

					EquipQuiver(actor, frm_damage);
				
					return;
				}				
			}
		}
	}

	void Events::OnUnEquip(Actor* actor, TESForm* form)
	{
		using namespace UQ_Settings;

		if (!actor || !form) return;

		UInt32 refID{ actor->formID };

		if (!form->IsAmmo()) {

			TypeWeapon isBow{ IsBow(form) };
			bool unequip{ true };

			if (!UQSettings.IsEnabledBow() && isBow == TypeWeapon::Bow)
				unequip = false;

			if (!UQSettings.IsEnabledCrossbow() && isBow == TypeWeapon::CBow)
				unequip = false;

			if (isBow == TypeWeapon::Bow || isBow == TypeWeapon::CBow) {

				UInt32 weap_id{ 0 };

				if (refID == PlayerID && UQSettings.IsEnabledMultiBow())
					weap_id = form->formID;

				auto& lAmmo = lastAmmo[refID];

				VisitContainer(actor, [&](InventoryEntryData* entry) {

					if (IsWorn(entry) && UQSettings.GetQuiverReEquipType() == QuiverReEquipType::QRE_LAST) 
						lAmmo.SetLast(IsBolt(entry->type), entry->type->formID, weap_id);

					if (unequip) {

						UNEQUIPITEM(actor, entry);
					}

					return false;
				});
			}
		}
		else {

			if (refID == PlayerID)
				lastPlayerAmmo = form->formID;
		}
	}

	EventResult Events::ReceiveEvent(TESEquipEvent* evn, EventDispatcher<TESEquipEvent>* dispatcher)
	{
		if (!evn) return kEvent_Continue;

		return ActorEvent(evn->reference, [&](Actor* act) {

			TESForm* form = LookupFormByID(evn->object);

			if (form && form->Has3D())
				evn->equipped ? OnEquip(act, form) : OnUnEquip(act, form);
			});
	}

	EventResult Events::ReceiveEvent(TESLoadGameEvent* evn, EventDispatcher<TESLoadGameEvent>* dispatcher)
	{
		if (!evn) return kEvent_Continue;

		TESForm* form = LookupFormByID(PlayerID);
		TESObjectREFR* refr = nullptr;

		if (form && (refr = DYNAMIC_CAST(form, TESForm, TESObjectREFR)))
			VisitCell(refr->parentCell);

		return kEvent_Continue;
	}

	EventResult Events::ReceiveEvent(TESObjectLoadedEvent* evn, EventDispatcher<TESObjectLoadedEvent>* dispatcher)
	{
		if (!evn || !evn->loaded) return kEvent_Continue;

		TESForm* form = LookupFormByID(evn->formId);
		TESObjectREFR* refr = nullptr;

		if (form && (refr = DYNAMIC_CAST(form, TESForm, TESObjectREFR)))
			ActorEvent(refr, [&](Actor* act)  { 
			
					HideQuiver(act);
					UnequipQuiver(act);
				});

		return kEvent_Continue;
	}

	EventResult Events::ReceiveEvent(TESInitScriptEvent* evn, EventDispatcher<TESInitScriptEvent>* dispatcher)
	{
		if (evn && evn->reference)
			ActorEvent(evn->reference, [&](Actor* act) { 

					HideQuiver(act); 
					UnequipQuiver(act);
				});

		return kEvent_Continue;
	}
	
	EventResult Events::ReceiveEvent(SKSEActionEvent* evn, EventDispatcher<SKSEActionEvent>* dispatcher)
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

	Events gEvents;

	void RegisterEventDispatch()
	{
		static bool InitDispatcher{ false };

		if (InitDispatcher) return;

#if UNEQUIPQUIVER_EXPORTS
		g_EquipEventDispatcher->AddEventSink(&gEvents);
		g_LoadGameEventDispatcher->AddEventSink(&gEvents);
		g_initScriptEventDispatcher->AddEventSink(&gEvents);
		g_objectLoadedEventDispatcher->AddEventSink(&gEvents);

#elif UNEQUIPQUIVERSE_EXPORTS || UNEQUIPQUIVERAE_EXPORTS

#define ADD_EVENT(ev, member) GetEventDispatcherList()-> ## member ##.AddEventSinkAddr(&ev, Addresses::AddEventSink_Internal_Addr); 

		ADD_EVENT(gEvents, equipDispatcher);
		ADD_EVENT(gEvents, loadGameEventDispatcher);
		ADD_EVENT(gEvents, initScriptDispatcher);
		ADD_EVENT(gEvents, objectLoadedDispatcher);
#endif

		if (skse_plugin::SKSE_Plugin::skse_msg_interface)
			if (auto skse_action = static_cast<EventDispatcher<SKSEActionEvent>*>
				(skse_plugin::SKSE_Plugin::skse_msg_interface->GetEventDispatcher(SKSEMessagingInterface::kDispatcher_ActionEvent))) {

				skse_action->AddEventSink(&gEvents);

				Hooks_Handlers_Commit();
			}

		InitDispatcher = true;

		_DMESSAGE("Event dispatcher registered successfully!");
	}
};
