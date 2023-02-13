
#include "Events.h"
#include "settings.h"
#include "skse_plugin.h"

#if UNEQUIPQUIVERSE_EXPORTS || UNEQUIPQUIVERAE_EXPORTS
#include "Addresses.h"
#endif

namespace EventsDispatch {

#define UNEQUIPITEMEX(actor, item, sound) \
	bool checkRet = settings.CheckBlackListAmmo(item->type->formID) | settings.CheckExtraDataAmmo(item->type->formID) | settings.CheckFavoritesAmmo(item); \
    if (!checkRet) papyrusActor::UnequipItemEx(actor, item->type, 0, false, sound); 

#define UNEQUIPITEM(actor, item) UNEQUIPITEMEX(actor, item, true)

#define EQUIPITEM(actor, item) papyrusActor::EquipItemEx(actor, item, 0, false, false)

	void TaskEquip::Run()
	{
		if (actor && form) {

			EQUIPITEM(actor, form);
		}
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
			for (int index = 0; index < containerData->objList->Count(); index++) {

				InventoryEntryData* entryData = containerData->objList->GetNthItem(index);

				if (entryData && entryData->type && entryData->type->IsAmmo())
					if (func(entryData))
						break;
			}
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

	bool Events::IsFavorites(InventoryEntryData* item)
	{
		if (item->extendDataList)
			for (UInt32 idx = 0; idx < item->extendDataList->Count(); idx++) {
			
				BaseExtraList* extraLst = item->extendDataList->GetNthItem(idx);

				if (extraLst && item->type)
					return extraLst->HasType(kExtraData_Hotkey);
			}

		return false;
	}

	bool Events::IsShield(TESForm* form)
	{
		TESObjectARMO* armo{ nullptr };

		if (form && form->IsArmor() && (armo = DYNAMIC_CAST(form, TESForm, TESObjectARMO)))
			for (int i = 0; i < armo->keyword.numKeywords; i++) {

				BGSKeyword* key = armo->keyword.keywords[i];

				if (key && std::string(key->keyword.Get()) == "ArmorShield")
					return true;
			}

		return false;
	}

	TypeWeapon Events::IsBow(TESForm* form)
	{
		TESObjectWEAP* weap{ nullptr };

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
		TESAmmo* ammo{ nullptr };

		if (form && (ammo = DYNAMIC_CAST(form, TESForm, TESAmmo)))
			return ammo->isBolt() ? 1 : 0;

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
		auto& settings = UQ_Settings::Settings::GetInstance();

		TESObjectWEAP* weap{ nullptr };

		if (form && form->IsWeapon() && (weap = DYNAMIC_CAST(form, TESForm, TESObjectWEAP)) && !settings.empty())
			for (int i = 0; i < weap->keyword.numKeywords; i++) {

				BGSKeyword* keyword = weap->keyword.keywords[i];

				if (keyword && binary_search(settings, keyword->keyword.Get()))
					return true;
			}

		return false;
	}

	bool Events::IsInventoryOpen()
	{
#if UNEQUIPQUIVERSE_EXPORTS || UNEQUIPQUIVERAE_EXPORTS
		UIStringHolder* stringHolder = *g_UIStringHolderExt;
		MenuManager* menu = *g_menuManagerExt;
#else
		UIStringHolder* stringHolder = UIStringHolder::GetSingleton();
		MenuManager* menu = MenuManager::GetSingleton();
#endif

		if (stringHolder && menu)
			return menu->IsMenuOpen(&stringHolder->inventoryMenu) | menu->IsMenuOpen(&stringHolder->favoritesMenu);

		return false;
	}

	void Events::EquipQuiver(Actor* act, TESForm* form, CharacterType charType)
	{
		if (charType == CharacterType::NPC) {

			EQUIPITEM(act, form);

			return;
		}

		if (!IsInventoryOpen()) {

			skse_plugin::SKSE_Plugin::GetInstance().AddTask(new TaskEquip(act, form));
		}
		else {

			EQUIPITEM(act, form);
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

		auto& settings = UQ_Settings::Settings::GetInstance();

		bool isSpell{ settings.IsEnabledSpell() ? IsSpell(form) : false };
		bool isShield{ settings.IsEnabledShield() ? IsShield(form) : false };
		bool isWeapon{ settings.IsEnabledWeapon() ?
			(settings.CheckWeapBykeywords() ? IsWeapon(form) : (isBow == TypeWeapon::AnotherWeapon)) : false };

		if (!isSpell && !isShield && !isWeapon) return;

		VisitContainer(act, [&](InventoryEntryData* entry) {

			UNEQUIPITEMEX(act, entry, false);

			return false;
		});
	}

	void Events::HideQuiver(Actor* act, CharacterType charType, bool drawn)
	{
		auto& settings = UQ_Settings::Settings::GetInstance();

		if (TESForm* weap = act->GetEquippedObject(true))
			switch (IsBow(weap)) {

			case TypeWeapon::Bow:

				hideNode[TypeNode::Quiver].Hide(act, settings.IsHideQuiverOnSheathe(drawn, charType));
				//hideNode[TypeNode::Bolt].Hide(act, settings.IsHideBoltOnSheathe(false, charType));
				
				return;

			case TypeWeapon::CBow:

				hideNode[TypeNode::Bolt].Hide(act, settings.IsHideBoltOnSheathe(drawn, charType));
				//hideNode[TypeNode::Quiver].Hide(act, settings.IsHideQuiverOnSheathe(false, charType));

				return;
			}

		hideNode[TypeNode::Quiver].Hide(act, settings.IsHideQuiverOnSheathe(false, charType));
		hideNode[TypeNode::Bolt].Hide(act, settings.IsHideBoltOnSheathe(false, charType));
	}

	void Events::CheckLastAmmo(Actor* actor, TESForm* form, CharacterType& charType)
	{
		if (charType == CharacterType::PC && lastPlayerAmmo != 0) { 
			
			auto& settings = UQ_Settings::Settings::GetInstance();

			TESForm* frmNewItem{ nullptr };
			TESAmmo* ammo{ nullptr };
			bool isFound{ false };
			float damage{ 0.0 };
			SInt32 count{ 0 };

			int isBoltEquipped = IsBolt(form);

			FlagsLastAmmo flags = settings.IsEnabledEquipStronger() ? FlagsLastAmmo::Stronger : FlagsLastAmmo::Nothing;
			flags |= settings.IsEnabledEquipLargerAmount() ? FlagsLastAmmo::Amount : FlagsLastAmmo::Nothing;

			VisitContainer(actor, [&](InventoryEntryData* entry) {

				TESForm* frmItem{ entry->type };

				if (!(ammo = DYNAMIC_CAST(frmItem, TESForm, TESAmmo)) || isBoltEquipped != IsBolt(frmItem))
					return false;

				SInt32 countItems = CountItems(actor, frmItem);

				switch (flags) {

				case FlagsLastAmmo::Stronger:

					if (damage < ammo->settings.damage) {

						damage = ammo->settings.damage;
						frmNewItem = frmItem;
					}

					break;

				case FlagsLastAmmo::Amount:

					if (count < countItems) {

						count = countItems;
						frmNewItem = frmItem;
					}

					break;

				case FlagsLastAmmo::StrongerAmount:

					if (damage < ammo->settings.damage) {

						damage = ammo->settings.damage;
						count = 0;
					}

					if (damage == ammo->settings.damage && count < countItems) {

						count = countItems;
						frmNewItem = frmItem;
					}

					break;
				}

				if (!isFound && countItems > 0) 
					isFound = lastPlayerAmmo == frmItem->formID;

				return false;
			});

			lastPlayerAmmo = 0;

			if (frmNewItem && !isFound) 
				EquipQuiver(actor, frmNewItem, charType);
		}
	}

	void Events::OnEquip(Actor* actor, TESForm* form, CharacterType charType)
	{
		if (!actor || !form || form->IsArmor())
			return;

		if (form->IsAmmo()) {

			CheckLastAmmo(actor, form, charType);

			return;
		}

		TypeWeapon isBow{ IsBow(form) };

		if (isBow != TypeWeapon::Bow && isBow != TypeWeapon::CBow) {

			UnequipQuiver(actor, form, isBow);
		}
		else {

			auto& settings = UQ_Settings::Settings::GetInstance();

			UInt32 weap_id { charType == CharacterType::PC && settings.IsEnabledMultiBow() ? form->formID : 0 };

			auto& lAmmo = lastAmmo[actor->formID];

			int isBolt{ 0 };
			float damage{ 0.0 };
			TESForm* frm_damage{ nullptr };
			TESAmmo* ammo{ nullptr };

			VisitContainer(actor, [&](InventoryEntryData* entry) {

				TESForm* frmItem{ entry->type };

				isBolt = IsBolt(frmItem);

				if ((isBolt && isBow == TypeWeapon::CBow) || (!isBolt && isBow == TypeWeapon::Bow))
					switch (settings.GetQuiverReEquipType()) {
					case UQ_Settings::QuiverReEquipType::Default:

						// fix NPCs
						if (charType == CharacterType::NPC) {

							EquipQuiver(actor, frmItem, charType);

							return true;
						}

						break;

					case UQ_Settings::QuiverReEquipType::Last:

						if (lAmmo.GetLast(isBolt, weap_id) == frmItem->formID) {

							EquipQuiver(actor, frmItem, charType);

							return true;
						}

						break;

					case UQ_Settings::QuiverReEquipType::Stronger:

						if ((ammo = DYNAMIC_CAST(frmItem, TESForm, TESAmmo)) && damage < ammo->settings.damage) {

							damage = ammo->settings.damage;
							frm_damage = frmItem;
						}

						break;
					}

				return false;
			});

			if (damage > 0 && frm_damage)
				EquipQuiver(actor, frm_damage, charType);
		}

		HideQuiver(actor, charType);
	}

	void Events::OnUnEquip(Actor* actor, TESForm* form, CharacterType charType)
	{
		if (!actor || !form || form->IsArmor())
			return;

		if (form->IsAmmo()) {

			SetLastAmmo(charType, form->formID);

			return;
		}

		TypeWeapon isBow{ IsBow(form) };
		bool unequip{ true };

		auto& settings = UQ_Settings::Settings::GetInstance();

		if (!settings.IsEnabledBow() && isBow == TypeWeapon::Bow)
			unequip = false;

		if (!settings.IsEnabledCrossbow() && isBow == TypeWeapon::CBow)
			unequip = false;

		if (isBow == TypeWeapon::Bow || isBow == TypeWeapon::CBow) {

			UInt32 weap_id{ 0 };

			if (charType == CharacterType::PC && settings.IsEnabledMultiBow())
				weap_id = form->formID;

			auto& lAmmo = lastAmmo[actor->formID];

			VisitContainer(actor, [&](InventoryEntryData* entry) {

				if (IsWorn(entry) && settings.GetQuiverReEquipType() == UQ_Settings::QuiverReEquipType::Last)
					lAmmo.SetLast(IsBolt(entry->type), entry->type->formID, weap_id);

				if (unequip) {

					UNEQUIPITEM(actor, entry);
				}

				return false;
			});
		}
	}

	void Events::VisitCell(TESObjectCELL* cell)
	{
		if (cell) {

#if UNEQUIPQUIVERSE_EXPORTS || UNEQUIPQUIVERAE_EXPORTS

			for (UInt32 idx = 0; idx < cell->refData.maxSize; idx++) {

				if (cell->refData.refArray[idx].unk08 && cell->refData.refArray[idx].ref) {

					ActorEvent(cell->refData.refArray[idx].ref, [&](Actor* act, CharacterType charType) {
#else
			for (UInt32 idx = 0; idx < cell->objectList.count; idx++) {

				if (cell->objectList[idx]) {

					ActorEvent(cell->objectList[idx], [&](Actor* act, CharacterType charType) {
#endif	

						HideQuiver(act, charType);
						UnequipQuiver(act);
					});
				}
			}
		}
	}

	template<typename T, typename Func>
	EventResult Events::ActorEvent(T refr, Func func)
	{
		Actor* act{ nullptr };
		CharacterType charType;

		if (refr && (act = DYNAMIC_CAST(refr, TESObjectREFR, Actor)) && IsActorEnabled(act, charType))
			func(act, charType);

		return kEvent_Continue;
	}

	bool Events::IsActorEnabled(Actor* actor, CharacterType& charType)
	{
		if (actor->IsDead(1) || !actor->race)
			return false;

		auto& settings = UQ_Settings::Settings::GetInstance();

		charType = GetCharacterType(actor);

		if (charType == CharacterType::PC && !settings.IsEnabledPC())
			return false;
			
		if (charType == CharacterType::NPC && !settings.IsEnabledNPC())
			return false;

		if (settings.CheckBlackListRace(actor->race->formID))
			return false;

		if (settings.CheckBlackListCharacter(actor->formID))
			return false;

		return true;
	}

	EventResult Events::ReceiveEvent(TESEquipEvent* evn, EventDispatcher<TESEquipEvent>* dispatcher)
	{
		if (!evn) return kEvent_Continue;

		return ActorEvent(evn->reference, [&](Actor* act, CharacterType charType) {

			TESForm* form = LookupFormByID(evn->object);

			if (form && form->Has3D())
				evn->equipped ? OnEquip(act, form, charType) : OnUnEquip(act, form, charType);
			});
	}

	EventResult Events::ReceiveEvent(TESLoadGameEvent* evn, EventDispatcher<TESLoadGameEvent>* dispatcher)
	{
		if (!evn) return kEvent_Continue;

		TESForm* form = LookupFormByID(GetPlayerID());
		TESObjectREFR* refr{ nullptr };

		if (form && (refr = DYNAMIC_CAST(form, TESForm, TESObjectREFR)))
			VisitCell(refr->parentCell);

		return kEvent_Continue;
	}

	EventResult Events::ReceiveEvent(TESObjectLoadedEvent* evn, EventDispatcher<TESObjectLoadedEvent>* dispatcher)
	{
		if (!evn || !evn->loaded) return kEvent_Continue;

		TESForm* form = LookupFormByID(evn->formId);
		TESObjectREFR* refr{ nullptr };

		if (form && (refr = DYNAMIC_CAST(form, TESForm, TESObjectREFR)))
			ActorEvent(refr, [&](Actor* act, CharacterType charType)  {
			
					HideQuiver(act, charType);
					UnequipQuiver(act);
				});

		return kEvent_Continue;
	}

	EventResult Events::ReceiveEvent(TESInitScriptEvent* evn, EventDispatcher<TESInitScriptEvent>* dispatcher)
	{
		if (evn && evn->reference)
			ActorEvent(evn->reference, [&](Actor* act, CharacterType charType) {

					HideQuiver(act, charType);
					UnequipQuiver(act);
				});

		return kEvent_Continue;
	}

	EventResult Events::ReceiveEvent(TESSwitchRaceCompleteEvent* evn, EventDispatcher<TESSwitchRaceCompleteEvent>* dispatcher)
	{
		if (evn && evn->ref)
			ActorEvent(evn->ref, [&](Actor* act, CharacterType charType) {

				HideQuiver(act, charType);
				UnequipQuiver(act);
			});

		return kEvent_Continue;
	}

	EventResult Events::ReceiveEvent(SKSEActionEvent* evn, EventDispatcher<SKSEActionEvent>* dispatcher)
	{
		CharacterType charType{ CharacterType::NPC };

		if (!evn || !evn->actor || !IsActorEnabled(evn->actor, charType))
			return kEvent_Continue;

		switch (evn->type) {

		case SKSEActionEvent::kType_BeginDraw:
			
			HideQuiver(evn->actor, charType, true);
			break;

		case SKSEActionEvent::kType_BeginSheathe:

			HideQuiver(evn->actor, charType, false);
			break;
		}

		return kEvent_Continue;
	}

	void Events::Register()
	{
		static bool InitDispatcher{ false };

		if (InitDispatcher) return;

#if UNEQUIPQUIVER_EXPORTS
		g_EquipEventDispatcher->AddEventSink(&instance);
		g_LoadGameEventDispatcher->AddEventSink(&instance);
		g_initScriptEventDispatcher->AddEventSink(&instance);
		g_objectLoadedEventDispatcher->AddEventSink(&instance);
		g_switchRaceCompleteEventDispatcher->AddEventSink(&instance);

#elif UNEQUIPQUIVERSE_EXPORTS || UNEQUIPQUIVERAE_EXPORTS

#define ADD_EVENT(ev, member) GetEventDispatcherList()-> ## member ##.AddEventSinkAddr(&ev, Addresses::AddEventSink_Internal_Addr); 

		ADD_EVENT(instance, equipDispatcher);
		ADD_EVENT(instance, loadGameEventDispatcher);
		ADD_EVENT(instance, initScriptDispatcher);
		ADD_EVENT(instance, objectLoadedDispatcher);
		ADD_EVENT(instance, switchRaceCompleteDispatcher);
#endif

		if (skse_plugin::SKSE_Plugin::GetInstance().Sink<SKSEActionEvent>(SKSEMessagingInterface::kDispatcher_ActionEvent, &instance)) {

			Hooks_Handlers_Commit();

			InitDispatcher = true;
		}

		if (InitDispatcher)
			_DMESSAGE("Event dispatcher registered successfully!");
		else
			_ERROR("Event dispacther registration error!");
	}

	VectorHideNode Events::hideNode;
	Events::LastAmmoEquipped Events::lastAmmo;
	Events Events::instance;
};
