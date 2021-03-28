
// Include
#include "lua.h"

#include "instance.h"
#include "objectmanager.h"
#include "serverevent.h"

#include "utils/functions.h"
#include "utils/log.h"

#include <glm/gtx/norm.hpp>
#include <glm/gtx/compatibility.hpp>

#include <algorithm>
#include <filesystem>

/*
	Darkspore "docs"
		attribute snapshot (
			attribute value = 0x1C + (value * 4)
		)

		ability layout (
			0x000: char namespace[50];
			0x05A: bool requiresAgent;
			0x05C: int allowsMovement;
			0x130: int maxStackCount[8];
			0x150: bool noGlobalCooldown;
			0x151: bool shouldPursue;
			0x153: bool alwaysUseCursorPos;
			0x154: bool channelled;
			0x155: bool showChannelBar;
			0x158: uint32_t minimumChannelTimeMs;
			0x15C: bool faceTargetOnCreate;
			0x152: bool finishOnDeath;
			0x160: uint32_t descriptors;
			0x164: uint32_t debuffDescriptors;
			0x168: uint32_t damageType;
			0x16C: uint32_t damageSource;
			0x170: float damageCoefficient;
			0x174: float healingCoefficient;
			0x178: int modifierPriority;
			0x17C: bool deactivateOnInterrupt;
			0x180: guid statusAnimation;
			0x184: asset statusHeadEffect;
			0x188: asset statusBodyEffect;
			0x18C: asset statusFootEffect;
			0x190: int activationType;
			0x194: int deactivationType;
			0x198: int interfaceType;
			0x19C: int cooldownType;
			0x1A0: int targetType;
			0x074: guid reticleEffect;
			0x078: guid icon;
			0x07C: guid localizedGroup;
			0x080: guid localizedName;
			0x0AC: guid lootGroup;
			0x0B0: guid lootDescription[8];
			0x084: guid localizedDescription[8];
			0x0A8: guid localizedOverdriveDescription;
			0x0D0: float cooldown[8];
			0x0F0: float cooldownVariance{8];
			0x110: float range[8];
			0x1A4: int handledEvents;
			0x1A8: tAssetPropertyVector properties;
			0x1B8: int scalingAttribute;
			0x1BC: int primaryAttributeStat;
			0x1C0: guid primaryAttributeStatDelegate;
			0x1C4: float manaCoefficient;
			0x1E4: bool saveOnDehydrate;
			0x1E5: bool rejectable;
			0x0A4: guid localizedShortDescription;

		)
*/

// Helpers
namespace {
	auto& LuaGetGame(sol::this_state L) {
		Game::Instance& game = sol::state_view(L)["Game"]["Instance"];
		return game;
	}

	auto LuaGetPlayer(const Game::Instance& instance, sol::object& value) {
		if (value.is<Game::PlayerPtr>()) {
			return value.as<Game::PlayerPtr>();
		} else {
			return instance.GetPlayerByIndex(value.as<uint8_t>());
		}
	}

	auto LuaGetObject(const Game::ObjectManager& objectManager, sol::object& value) {
		if (value.is<Game::ObjectPtr>()) {
			return value.as<Game::ObjectPtr>();
		} else {
			return objectManager.Get(value.as<uint32_t>());
		}
	}

	auto LuaGetTrigger(const Game::ObjectManager& objectManager, sol::object& value) {
		if (value.is<Game::TriggerVolumePtr>()) {
			return value.as<Game::TriggerVolumePtr>();
		} else {
			return objectManager.GetTrigger(value.as<uint32_t>());
		}
	}

	auto LuaGetServerEvent(const Game::ObjectManager& objectManager, const sol::table& value) {
		Game::ServerEvent serverEvent;
		serverEvent.mObjectId = value.get_or<uint32_t>("objectId", 0);

		const auto& object = objectManager.Get(serverEvent.mObjectId);
		if (object) {
			serverEvent.mServerEventDef = value.get_or<uint32_t>("asset", 0);
			serverEvent.mEffectIndex = object->AddEffect(serverEvent.mServerEventDef);
			if (serverEvent.mEffectIndex != 0xFF) {
				if (sol::object eventData = value["facing"]; eventData.is<glm::vec3>()) {
					serverEvent.mFacing = eventData.as<glm::vec3>();
				}

				if (sol::object eventData = value["targetPoint"]; eventData.is<glm::vec3>()) {
					serverEvent.mTargetPoint = eventData.as<glm::vec3>();
				}

				serverEvent.mSimpleSwarmEffectId = value.get_or<uint32_t>("simpleSwarmEffectId", 0);
				serverEvent.mSecondaryObjectId = value.get_or<uint32_t>("secondaryObjectId", 0);
				serverEvent.mAttackerId = value.get_or<uint32_t>("attackerId", 0);

				serverEvent.mRemove = value.get_or<bool>("remove", 0);
				serverEvent.mHardStop = value.get_or<bool>("hardStop", 0);
				serverEvent.mForceAttach = value.get_or<bool>("forceAttach", 0);
				serverEvent.mCritical = value.get_or<bool>("critical", 0);
			}
		} else {
			serverEvent.mObjectId = 0;
		}

		return serverEvent;
	}
}

// LuaFunction
namespace LuaFunction {
	// global functions
	auto AddEvent(sol::this_state L, sol::protected_function callback, uint32_t delay, sol::variadic_args args) {
		if (delay > 0) {
			Game::Instance& game = LuaGetGame(L);
			return game.AddTask(delay, [&game, callback = std::move(callback), args = std::vector<sol::object>(args.begin(), args.end())](uint32_t id) mutable {
				game.AddServerTask([&game, id, callback = std::move(callback), args = std::move(args)]() mutable {
					game.CancelTask(id);
					callback.call<void>(sol::as_args(args));
				});
			});
		} else {
			callback.call<void>(args);
			return static_cast<uint32_t>(0);
		}
	}

	auto StopEvent(sol::this_state L, uint32_t id) {
		LuaGetGame(L).CancelTask(id);
	}

	// os
	auto OsMtime() {
		return utils::get_milliseconds();
	}

	// nUtil
	auto nUtil_SPID(std::string value) {
		return utils::hash_id(value);
	}

	auto nUtil_ToGUID(std::string value) {
		return utils::to_number<uint64_t>(value, 0);
	}

	// nThread
	auto nThread_WaitForXSeconds(sol::this_state L, double seconds) {
		Game::Instance& game = LuaGetGame(L);
		Game::LuaThread* thread = game.GetLua().GetThread(L);
		if (!thread) {
			return;
		}

		game.AddTask(static_cast<uint32_t>(seconds * 1000), [&game, thread](uint32_t id) mutable {
			game.AddServerTask([&game, thread, id]() mutable {
				game.CancelTask(id);
				thread->call();
			});
		});
	}

	// Game
	auto Game_GetPlayer(sol::this_state L, uint8_t index) {
		return LuaGetGame(L).GetPlayerByIndex(index);
	}

	auto Game_Notify(sol::this_state L, sol::table serverEventTable) {
		Game::Instance& game = LuaGetGame(L);
		Game::ServerEvent serverEvent = LuaGetServerEvent(game.GetObjectManager(), serverEventTable);

		game.SendServerEvent(serverEvent);
	}

	// nObjectManager
	auto nObjectManager_GetObject(sol::this_state L, uint32_t id) {
		Game::Instance& game = LuaGetGame(L);
		return game.GetObjectManager().Get(id);
	}

	auto nObjectManager_CreateObject(sol::this_state L, uint32_t noun, glm::vec3 position) {
		Game::Instance& game = LuaGetGame(L);

		auto object = game.GetObjectManager().Create(noun);
		if (object) {
			object->SetPosition(position);
		}

		return object;
	}

	auto nObjectManager_GetTrigger(sol::this_state L, uint32_t id) {
		Game::Instance& game = LuaGetGame(L);
		return game.GetObjectManager().GetTrigger(id);
	}

	auto nObjectManager_CreateTriggerVolume(sol::this_state L, glm::vec3 position, float radius) {
		Game::Instance& game = LuaGetGame(L);
		return game.GetObjectManager().CreateTrigger(position, radius);
	}

	auto nObjectManager_GetObjectsInRadius(sol::this_state L, glm::vec3 position, float radius, sol::table objectTypes) {
		Game::Instance& game = LuaGetGame(L);

		std::vector<Game::NounType> types;
		for (const auto& entry : objectTypes) {
			types.push_back(static_cast<Game::NounType>(entry.second.as<uint32_t>()));
		}

		auto objects = game.GetObjectManager().GetObjectsInRadius(position, radius, types);
		auto objectCount = static_cast<int>(objects.size());

		auto result = sol::state_view(L).create_table(objectCount, 0);
		for (int i = 0; i < objectCount; ++i) {
			result[i + 1] = objects[i];
		}

		return result;
	}

	auto nObjectManager_GetObjectsInRadius_SortedByDistance(sol::this_state L, glm::vec3 position, float radius, sol::table objectTypes) {
		Game::Instance& game = LuaGetGame(L);

		std::vector<Game::NounType> types;
		for (const auto& entry : objectTypes) {
			types.push_back(static_cast<Game::NounType>(entry.second.as<uint32_t>()));
		}

		auto objects = game.GetObjectManager().GetObjectsInRadius(position, radius, types);
		auto objectCount = static_cast<int>(objects.size());
		std::sort(objects.begin(), objects.end(), [&position](const auto& lhs, const auto& rhs) {
			return glm::distance(position, lhs->GetPosition()) < glm::distance(position, rhs->GetPosition());
		});

		auto result = sol::state_view(L).create_table(objectCount, 0);
		for (int i = 0; i < objectCount; ++i) {
			result[i + 1] = objects[i];
		}

		return result;
	}

	// Locomotion
	auto Locomotion_TeleportObject(sol::this_state L, sol::object objectValue, glm::vec3 position, sol::optional<bool> keepFacing) {
		Game::Instance& game = LuaGetGame(L);
		Game::ObjectPtr object = LuaGetObject(game.GetObjectManager(), objectValue);
		if (object && object->HasLocomotionData()) {
			const auto& locomotion = object->GetLocomotionData();
			locomotion->Stop();

			const auto& direction = position - object->GetPosition();
			object->SetPosition(position);

			if (keepFacing.value_or(false)) {
				locomotion->SetFacing(direction + object->GetPosition());
			}
		}
	}

	auto Locomotion_Stop(sol::this_state L, sol::object objectValue) {
		Game::Instance& game = LuaGetGame(L);
		Game::ObjectPtr object = LuaGetObject(game.GetObjectManager(), objectValue);
		if (object && object->HasLocomotionData()) {
			object->GetLocomotionData()->Stop();
		}
	}

	auto Locomotion_GetClosestPosition(sol::this_state L, glm::vec3 position) {
		// TODO: add Locomotion::GetClosestPosition (not sure how the fuck I'm gonna do that without getting navmesh data)
		return position;
	}

	// Player
	auto Player_Notify(sol::this_state L, sol::object playerValue, sol::table serverEventTable) {
		Game::Instance& game = LuaGetGame(L);
		Game::PlayerPtr player = LuaGetPlayer(game, playerValue);
		if (player) {
			Game::ServerEvent serverEvent = LuaGetServerEvent(game.GetObjectManager(), serverEventTable);
			game.SendServerEvent(player, serverEvent);
		}
	}

	// Object
	auto Object_GetPrivateTable(sol::this_state L, sol::object objectValue) {
		Game::Instance& game = LuaGetGame(L);
		Game::ObjectPtr object = LuaGetObject(game.GetObjectManager(), objectValue);
		if (object) {
			return game.GetLua().CreatePrivateTable(object->GetId());
		}
		return sol::table(sol::nil);
	}

	auto Object_GetFacing(sol::this_state L, sol::object objectValue) {
		Game::Instance& game = LuaGetGame(L);
		Game::ObjectPtr object = LuaGetObject(game.GetObjectManager(), objectValue);
		if (object && object->HasLocomotionData()) {
			return object->GetLocomotionData()->GetFacing();
		}
		return glm::zero<glm::vec3>();
	}

	auto Object_GetFootprintRadius(sol::this_state L, sol::object objectValue) {
		Game::Instance& game = LuaGetGame(L);
		Game::ObjectPtr object = LuaGetObject(game.GetObjectManager(), objectValue);
		if (object) {
			return object->GetFootprintRadius();
		}
		return 0.f;
	}

	auto Object_Move(sol::this_state L, sol::object objectValue, glm::vec3 position, bool teleport) {
		Game::Instance& game = LuaGetGame(L);
		Game::ObjectPtr object = LuaGetObject(game.GetObjectManager(), objectValue);
		if (object && object->HasLocomotionData()) {
			const auto& locomotionData = object->GetLocomotionData();
			locomotionData->SetGoalPosition(position);
			if (teleport) {
				locomotionData->SetGoalFlags(locomotionData->GetGoalFlags() | 0x020);
			}
			game.MoveObject(object, *locomotionData);
		}
	}

	auto Object_SetAnimationState(sol::this_state L, sol::object objectValue, uint32_t animationState, sol::optional<bool> overlay, sol::optional<float> scale) {
		if (animationState == 0) {
			return;
		}

		Game::Instance& game = LuaGetGame(L);
		game.SendAnimationState(LuaGetObject(game.GetObjectManager(), objectValue), animationState, overlay.value_or(false), scale.value_or(1.f));
	}

	auto Object_SetAnimationStateToDeath(sol::this_state L, sol::object objectValue) {
		// TODO: 
	}

	auto Object_SetAnimationStateToAggro(sol::this_state L, sol::object objectValue, sol::optional<bool> subsequent) {
		float duration = 0;

		Game::Instance& game = LuaGetGame(L);
		Game::ObjectPtr object = LuaGetObject(game.GetObjectManager(), objectValue);
		if (!object) {
			return duration;
		}

		const auto& noun = object->GetNoun();
		if (!noun) {
			return duration;
		}

		const auto& animations = noun->GetCharacterAnimation();
		if (!animations) {
			return duration;
		}

		uint32_t state;
		if (subsequent.value_or(false)) {
			std::tie(state, duration) = animations->GetAnimationData(Game::CharacterAnimationType::SubsequentAggro);
		} else {
			std::tie(state, duration) = animations->GetAnimationData(Game::CharacterAnimationType::Aggro);
		}

		game.SendAnimationState(object, state);
		return duration;
	}

	auto Object_SetGraphicsState(sol::this_state L, sol::object objectValue, uint32_t graphicsState) {
		if (graphicsState == 0) {
			return;
		}

		Game::Instance& game = LuaGetGame(L);
		game.SendObjectGfxState(LuaGetObject(game.GetObjectManager(), objectValue), graphicsState);
	}

	auto Object_GetAttributeSnapshot(sol::this_state L, sol::object objectValue) {
		Game::Instance& game = LuaGetGame(L);
		Game::ObjectPtr object = LuaGetObject(game.GetObjectManager(), objectValue);

		Game::AttributesPtr attributes;
		if (object && object->HasAttributeData()) {
			attributes = std::make_shared<Game::Attributes>(*object->GetAttributeData());
		}

		return attributes;
	}

	auto Object_GetWeaponDamage(sol::this_state L, sol::object objectValue) {
		Game::Instance& game = LuaGetGame(L);
		Game::ObjectPtr object = LuaGetObject(game.GetObjectManager(), objectValue);

		sol::table damageRange = sol::state_view(L).create_table(2);
		if (object && object->HasAttributeData()) {
			const auto& weaponDamage = object->GetAttributeData()->GetWeaponDamage();
			damageRange[1] = std::get<0>(weaponDamage);
			damageRange[2] = std::get<1>(weaponDamage);
		}

		return damageRange;
	}

	auto Object_AddCooldown(sol::this_state L, sol::object objectValue, uint32_t abilityId, uint32_t milliseconds) {
		Game::Instance& game = LuaGetGame(L);
		Game::ObjectPtr object = LuaGetObject(game.GetObjectManager(), objectValue);
		if (object) {
			const auto& [end, duration] = object->AddCooldown(abilityId, milliseconds);
			game.SendCooldownUpdate(object, abilityId, end - duration, duration);
		}
	}

	auto Object_RemoveCooldown(sol::this_state L, sol::object objectValue, uint32_t abilityId, uint32_t milliseconds) {
		Game::Instance& game = LuaGetGame(L);
		Game::ObjectPtr object = LuaGetObject(game.GetObjectManager(), objectValue);
		if (object) {
			const auto& [end, duration] = object->RemoveCooldown(abilityId, milliseconds);
			game.SendCooldownUpdate(object, abilityId, end - duration, duration);
		}
	}

	auto Object_ScaleCooldown(sol::this_state L, sol::object objectValue, uint32_t abilityId, float scale) {
		//
	}

	auto Object_TakeDamage(sol::this_state L,
		sol::object objectValue,
		sol::object attackerValue,
		sol::object attributesSnapshotValue,
		sol::table damageRange,
		Game::DamageType damageType,
		Game::DamageSource damageSource,
		float damageCoefficient,
		int32_t descriptors,
		sol::optional<float> damageMultiplier,
		sol::optional<glm::vec3> direction
	) {
		Game::Instance& game = LuaGetGame(L);
		const auto& objectManager = game.GetObjectManager();

		Game::ObjectPtr object = LuaGetObject(objectManager, objectValue);
		if (object) {
			Game::ObjectPtr attacker = LuaGetObject(objectManager, attackerValue);
			Game::AttributesPtr attributes;
			if (attributesSnapshotValue.is<Game::AttributesPtr>()) {
				attributes = attributesSnapshotValue.as<Game::AttributesPtr>();
			}

			Game::CombatEvent combatEvent;
			combatEvent.mTargetId = object->GetId();
			combatEvent.mDirection = direction.value_or(glm::zero<glm::vec3>());

			const auto& result = object->TakeDamage(
				attributes,
				std::make_tuple(damageRange.raw_get_or(1, 0.f), damageRange.raw_get_or(2, 0.f)),
				damageType,
				damageSource,
				damageCoefficient,
				descriptors,
				damageMultiplier.value_or(1.0f),
				combatEvent.mDirection
			);

			combatEvent.mDeltaHealth = std::get<1>(result);
			combatEvent.mIntegerHpChange = static_cast<int32_t>(combatEvent.mDeltaHealth);

			if (std::get<2>(result)) {
				combatEvent.mFlags = Game::CombatEventFlags::Critical;
			}

			game.SendCombatEvent(combatEvent);
			return result;
		}

		return std::make_tuple(false, 0.f, false);
	}

	auto Object_Heal(sol::this_state L, sol::object objectValue, sol::object attackerValue) {
		return std::make_tuple(0.f, false);
	}

	auto Object_AddEffect(sol::this_state L, sol::object objectValue, sol::object serverEventDef, sol::optional<sol::object> secondaryObjectValue, sol::optional<glm::vec3> targetPointValue) {
		Game::Instance& game = LuaGetGame(L);
		const auto& objectManager = game.GetObjectManager();

		Game::ObjectPtr object = LuaGetObject(objectManager, objectValue);
		if (object) {
			uint32_t effect;
			if (serverEventDef.is<uint32_t>()) {
				effect = serverEventDef.as<uint32_t>();
			} else if (serverEventDef.is<std::string>()) {
				effect = utils::hash_id(serverEventDef.as<std::string>());
			} else {
				effect = 0;
			}

			auto index = object->AddEffect(effect);
			if (index == 0) {
				return index;
			}

			Game::ServerEvent serverEvent;
			serverEvent.mServerEventDef = effect;
			serverEvent.mEffectIndex = index;
			serverEvent.mForceAttach = true;
			serverEvent.mObjectId = object->GetId();

			if (targetPointValue.has_value()) {
				serverEvent.mTargetPoint = targetPointValue.value();
			} else {
				serverEvent.mTargetPoint = object->GetPosition();
			}

			// serverEvent.SetIgnoredPlayerBits(ignoredPlayerBits);
			if (secondaryObjectValue) {
				Game::ObjectPtr secondaryObject = LuaGetObject(objectManager, *secondaryObjectValue);
				if (secondaryObject) {
					serverEvent.mSecondaryObjectId = secondaryObject->GetId();
				}
			}

			game.SendServerEvent(serverEvent);
			return serverEvent.mEffectIndex;
		}

		return static_cast<uint8_t>(0);
	}

	auto Object_RemoveEffect(sol::this_state L, sol::object objectValue, sol::object effectValue, sol::optional<bool> hardStop) {
		uint32_t effect;
		if (effectValue.is<uint32_t>()) {
			effect = effectValue.as<uint32_t>();
		} else if (effectValue.is<std::string>()) {
			effect = utils::hash_id(effectValue.as<std::string>());
		} else {
			return;
		}

		Game::Instance& game = LuaGetGame(L);
		Game::ObjectPtr object = LuaGetObject(game.GetObjectManager(), objectValue);
		if (object) {
			auto index = object->RemoveEffect(effect);
			if (index != 0) {
				Game::ServerEvent serverEvent;
				serverEvent.mObjectId = object->GetId();
				serverEvent.mEffectIndex = index;
				serverEvent.mRemove = true;
				serverEvent.mHardStop = hardStop.value_or(true);
				// serverEvent.SetIgnoredPlayerBits(ignoredPlayerBits);

				game.SendServerEvent(serverEvent);
			}
		}
	}

	auto Object_RemoveEffectIndex(sol::this_state L, sol::object objectValue, uint8_t index, sol::optional<bool> hardStop) {
		if (index == 0) {
			return;
		}

		Game::Instance& game = LuaGetGame(L);
		Game::ObjectPtr object = LuaGetObject(game.GetObjectManager(), objectValue);
		if (object) {
			auto removed = object->RemoveEffectByIndex(index);
			if (removed) {
				Game::ServerEvent serverEvent;
				serverEvent.mObjectId = object->GetId();
				serverEvent.mEffectIndex = index;
				serverEvent.mRemove = true;
				serverEvent.mHardStop = hardStop.value_or(true);
				// serverEvent.SetIgnoredPlayerBits(ignoredPlayerBits);

				game.SendServerEvent(serverEvent);
			}
		}
	}

	auto Object_UseAbility(sol::this_state L, sol::object objectValue, uint32_t abilityId, sol::object targetObjectValue, glm::vec3 targetPosition) {
		Game::Instance& game = LuaGetGame(L);

		const auto& ability = game.GetLua().GetAbility(abilityId);
		if (ability) {
			const auto& objectManager = game.GetObjectManager();

			Game::ObjectPtr object = LuaGetObject(objectManager, objectValue);
			if (object) {
				Game::ObjectPtr targetObject = LuaGetObject(objectManager, targetObjectValue);
				ability->Tick(object, targetObject, targetPosition);
			}
		}
	}

	auto Object_DropLoot(sol::this_state L, sol::object objectValue, sol::object targetObjectValue, Game::DropType dropType) {
		Game::Instance& game = LuaGetGame(L);
		const auto& objectManager = game.GetObjectManager();

		Game::ObjectPtr object = LuaGetObject(objectManager, objectValue);
		Game::ObjectPtr targetObject = LuaGetObject(objectManager, targetObjectValue);
		if (object && targetObject) {
			game.DropLoot(object, targetObject, dropType);
		}
	}

	auto Object_GetTeleporterDestination(sol::this_state L, sol::object objectValue) {
		Game::Instance& game = LuaGetGame(L);
		Game::ObjectPtr object = LuaGetObject(game.GetObjectManager(), objectValue);

		glm::vec3 destination {};
		if (object) {
			const auto& marker = game.GetMarker(object->GetMarkerId());
			if (marker) {
				const auto& teleporterData = marker->GetTeleporterData();
				if (teleporterData) {
					const auto& destinationMarker = game.GetMarker(teleporterData->GetDestinationId());
					if (destinationMarker) {
						destination = destinationMarker->GetPosition();
					}
				}
			}
		}

		return destination;
	}

	// Trigger
	auto Trigger_Attach(sol::this_state L, sol::object triggerValue, sol::object objectValue) {
		Game::Instance& game = LuaGetGame(L);
		const auto& objectManager = game.GetObjectManager();

		Game::TriggerVolumePtr trigger = LuaGetTrigger(objectManager, triggerValue);
		Game::ObjectPtr object = LuaGetObject(objectManager, objectValue);
		if (trigger && object) {
			trigger->Attach(object);
		}
	}

	auto Trigger_Detach(sol::this_state L, sol::object triggerValue, sol::object objectValue) {
		Game::Instance& game = LuaGetGame(L);
		const auto& objectManager = game.GetObjectManager();

		Game::TriggerVolumePtr trigger = LuaGetTrigger(objectManager, triggerValue);
		Game::ObjectPtr object = LuaGetObject(objectManager, objectValue);
		if (trigger && object) {
			trigger->Detach(object);
		}
	}

	// nPlayer
	auto nPlayerPickupCrystal() {

	}
}

// Game
namespace Game {
	// Ability
	Ability::Ability(sol::table&& self) : mSelf(std::move(self)) {
		if (mSelf == sol::nil) {
			throw std::exception("Ability::Ability: self is nil");
		}

		sol::state_view state = mSelf.lua_state();
		mEnvironment = sol::environment(state, sol::create, state.globals());

		if (sol::object value = mSelf["activate"]; value.is<sol::protected_function>()) {
			mEnvironment.set_on(value);
			mHasActivate = true;
		}

		if (sol::object value = mSelf["deactivate"]; value.is<sol::protected_function>()) {
			mEnvironment.set_on(value);
			mHasDeactivate = true;
		}

		if (sol::object value = mSelf["tick"]; value.is<sol::protected_function>()) {
			mEnvironment.set_on(value);
			mHasTick = true;
		}
	}

	void Ability::Reload() {

	}

	// LuaBase
	LuaBase::~LuaBase() {
		CollectGarbage();
	}

	void LuaBase::Initialize() {
		mState.open_libraries();
		sol::set_default_state(mState, &sol::default_at_panic, [](lua_State* L) {
			std::string msg = "An unknown error has triggered the default error handler";
			sol::optional<sol::string_view> maybetopmsg = sol::stack::unqualified_check_get<sol::string_view>(L, 1, &sol::no_panic);
			if (maybetopmsg) {
				const sol::string_view& topmsg = maybetopmsg.value();
				msg.assign(topmsg.data(), topmsg.size());
			}

			luaL_traceback(L, L, msg.c_str(), 1);

			sol::optional<sol::string_view> maybetraceback = sol::stack::unqualified_check_get<sol::string_view>(L, -1, &sol::no_panic);
			if (maybetraceback) {
				const sol::string_view& traceback = maybetraceback.value();
				msg.assign(traceback.data(), traceback.size());
			}

			std::cout << "[sol3] An error occurred and was caught in traceback: ";
			std::cout << msg;
			std::cout << std::endl;

			return sol::stack::push(L, msg);
		}, [](lua_State* L, sol::optional<const std::exception&> maybe_exception, sol::string_view description) -> int {
			std::cout << "An exception occurred in a function, here's what it says ";
			if (maybe_exception) {
				std::cout << "(straight from the exception): ";
				const std::exception& ex = *maybe_exception;
				std::cout << ex.what() << std::endl;
			} else {
				std::cout << "(from the description parameter): ";
				std::cout.write(description.data(), static_cast<std::streamsize>(description.size()));
				std::cout << std::endl;
			}
			return sol::stack::push(L, description);
		});

		LoadBuffer(R"(package.path = package.path .. ";./data/lua/?.lua")");
		RegisterFunctions();

		// Get default packages
		sol::table packagesLoaded = mState["package"]["loaded"];
		mDefaultPackages = mState.create_table();

		for (const auto& [k, v] : packagesLoaded) {
			mDefaultPackages[k] = v;
		}
	}

	void LuaBase::Reload() {
		mState["package"]["loaded"] = mDefaultPackages;
		CollectGarbage();
	}

	sol::protected_function_result LuaBase::ReportError(lua_State* L, sol::protected_function_result pfr) {
		sol::error err = pfr;

		/*
		if (function) {
			printf("%s().\n", function);
		}
		*/

		bool stackTrace = true;
		if (stackTrace) {
			auto trace = GetStackTrace(L, err.what());
			std::cout << trace << std::endl;
		} else {
			// std::string file = getFileById(runningEvent);
			// printf("%s\n%s\n", file.c_str(), err.what());
		}

		return pfr;
	}

	const char* LuaBase::GetStackTrace(lua_State* L, const char* error) {
		lua_getfield(L, LUA_GLOBALSINDEX, "debug");
		if (lua_type(L, -1) != LUA_TTABLE) {
			lua_pop(L, 1);
			return error;
		}

		lua_getfield(L, -1, "traceback");
		if (lua_type(L, -1) != LUA_TFUNCTION) {
			lua_pop(L, 1);
			return error;
		}

		lua_pushstring(L, error);
		lua_call(L, 1, 1);

		const char* result = lua_tostring(L, -1);
		lua_pop(L, 1);

		return result;
	}

	sol::protected_function_result LuaBase::LoadFile(const std::string& path) {
		return mState.safe_script_file(path, &LuaBase::ReportError);
	}

	sol::protected_function_result LuaBase::LoadBuffer(const std::string& buffer) {
		return mState.safe_script(buffer, &LuaBase::ReportError);
	}

	lua_State* LuaBase::GetState() const {
		return mState;
	}

	void LuaBase::CollectGarbage() {
		mState.collect_garbage();
		mState.collect_garbage();
	}

	void LuaBase::RegisterFunctions() {
		// os
		auto osTable = mState.create_named_table("os");
		osTable["mtime"] = &LuaFunction::OsMtime;

		// glm
		{
			mState.new_usertype<glm::vec3>("vec3",
				sol::constructors<glm::vec3(), glm::vec3(float, float, float)>(),

				sol::meta_function::to_string, [](const glm::vec3& v) {
					return "vec3<" + std::to_string(v.x) + ", " + std::to_string(v.y) + ", " + std::to_string(v.z) + ">";
				},

				sol::meta_function::addition, [](const glm::vec3& v1, const glm::vec3& v2) -> glm::vec3 { return v1 + v2; },
				sol::meta_function::subtraction, [](const glm::vec3& v1, const glm::vec3& v2) -> glm::vec3 { return v1 - v2; },
				sol::meta_function::multiplication, sol::overload(
					[](const glm::vec3& v1, const glm::vec3& v2) -> glm::vec3 { return v1 * v2; },
					[](const glm::vec3& v1, float f) -> glm::vec3 { return v1 * f; },
					[](float f, const glm::vec3& v1) -> glm::vec3 { return f * v1; }
				),
				sol::meta_function::division, sol::overload(
					[](const glm::vec3& v1, const glm::vec3& v2) -> glm::vec3 { return v1 / v2; },
					[](const glm::vec3& v1, float f) -> glm::vec3 { return v1 / f; },
					[](float f, const glm::vec3& v1) -> glm::vec3 { return f / v1; }
				),
				
				"x", &glm::vec3::x,
				"y", &glm::vec3::y,
				"z", &glm::vec3::z,
					
				"dot", [](const glm::vec3& v1, const glm::vec3& v2) -> float { return glm::dot(v1, v2); },
				"len", [](const glm::vec3& v) -> float { return glm::length(v); },
				"sqrlen", [](const glm::vec3& v) -> float { return glm::length2(v); },
				"distance", [](const glm::vec3& v1, const glm::vec3& v2) -> float { return glm::distance(v1, v2); },
				"distance2", [](const glm::vec3& v1, const glm::vec3& v2) -> float { return glm::distance2(v1, v2); },
				"normalize", [](const glm::vec3& v) -> glm::vec3 { return glm::normalize(v); },
				"cross", [](const glm::vec3& v1, const glm::vec3& v2) -> glm::vec3 { return glm::cross(v1, v2); },
				"direction", [](const glm::vec3& v1, const glm::vec3& v2) -> glm::vec3 { return glm::normalize(v1 - v2); },
				"lerp", [](const glm::vec3& v1, const glm::vec3& v2, float a) -> glm::vec3 { return glm::lerp(v1, v2, a); },
				"scale", [](const glm::vec3& v, float scale) -> glm::vec3 { return v * scale; }
			);
		}

		// nUtil
		auto nUtilTable = mState.create_named_table("nUtil");
		nUtilTable["SPID"] = &LuaFunction::nUtil_SPID;
		nUtilTable["ToGUID"] = &LuaFunction::nUtil_ToGUID;

		// nMathUtil
		auto nMathUtilTable = mState.create_named_table("nMathUtil");
		nMathUtilTable["CircleIntersectsArc"] = [](const glm::vec3& position, float radius, float arcX, float arcY, float arcFacingX, float arcFacingY, float arcLength, float arcAngle) {
			const auto somefunc = [](const glm::vec2& position, const glm::vec2& arc_position, glm::vec2 other) {
				auto delta = other - arc_position;
				auto something = (position - arc_position) * delta;

				auto var = something.x + something.y;
				auto var2 = glm::length2(something);
				if (var >= 0 && var2 >= var) {
					var /= var2;
					other = delta * var + arc_position;
				}
				
				return glm::length(position - other);
			};

			auto arcPosition = glm::vec2(arcX, arcY);
			auto delta = glm::vec2(position) - arcPosition;
			auto value = glm::sqrt(delta.x * delta.x + delta.y * delta.y);
			if (value < (arcLength + radius)) {

				delta = (1.f / value) * delta;
				delta.x *= arcFacingX;
				delta.y *= arcFacingY;

				arcAngle *= 0.5f;
				if (arcAngle <= glm::acos(delta.x + delta.y)) {
					glm::vec2 facingLength = glm::vec2(arcFacingX, arcFacingY) * arcLength;

					float arcCos = glm::cos(arcAngle);
					float arcSin = glm::sin(arcAngle);
					if (position.z <= somefunc(position, arcPosition, {
						(arcCos * facingLength.x) - (arcSin * facingLength.y) + arcX,
						(arcCos * facingLength.y) + (arcSin * facingLength.x) + arcY
					})) {
						float arcCosNeg = glm::cos(-arcAngle);
						float arcSinNeg = glm::sin(-arcAngle);
						if (position.z <= somefunc(position, arcPosition, {
							(arcCosNeg * facingLength.x) - (arcSinNeg * facingLength.y) + arcX,
							(arcCosNeg * facingLength.y) + (arcSinNeg * facingLength.x) + arcY
						})) {
							return false;
						}
					}
				}
				return true;

			}
			return false;
		};
	}

	// GlobalLua
	decltype(GlobalLua::sInstance) GlobalLua::sInstance;

	GlobalLua::~GlobalLua() {
		//
	}

	GlobalLua& GlobalLua::Instance() {
		return sInstance;
	}

	void GlobalLua::Initialize() {
		LuaBase::Initialize();

		// Load defaults & abilities
		LoadFile("data/lua/global.lua");
		LoadAbilities();
	}

	void GlobalLua::Reload() {
		// temporary
		LuaBase::Reload();
		mLoadedAbilities.clear();
		LoadAbilities();
	}

	sol::bytecode GlobalLua::GetAbility(const std::string& abilityName) const {
		return GetAbility(utils::hash_id(abilityName));
	}

	sol::bytecode GlobalLua::GetAbility(uint32_t abilityId) const {
		auto it = mLoadedAbilities.find(abilityId);
		return it != mLoadedAbilities.end() ? it->second : sol::bytecode();
	}

	void GlobalLua::LoadAbilities() {
		const auto& dataPath = "data/lua/ability/";
		if (!std::filesystem::exists(dataPath)) {
			std::cout << "Cannot load scripts." << std::endl;
			return;
		}

		std::cout << "Loading ability scripts...";

		size_t failed = 0;
		for (const auto& entry : std::filesystem::directory_iterator(dataPath)) {
			const auto& path = entry.path();
			if (entry.is_regular_file() && path.extension() == ".lua") {
				auto result = mState.load_file(path.string());
				if (!result.valid()) {
					if (failed++ == 0) {
						std::cout << std::endl;
					}
					std::cout << "GlobalLua: Could not load '" << path << "'." << std::endl;
					continue;
				}
				
				const auto& [it, inserted] = mLoadedAbilities.try_emplace(utils::hash_id(path.stem().string()));
				if (inserted) {
					sol::protected_function target = result.get<sol::protected_function>();
					it->second = target.dump();
				}
			}
		}

		if (mLoadedAbilities.empty() && failed > 0) {
			std::cout << " failed." << std::endl;
		} else {
			std::cout << " done. (loaded: " << mLoadedAbilities.size() << ", failed: " << failed << ")" << std::endl;
		}
	}

	// Lua
	Lua::Lua(Instance& game) : LuaBase(), mGame(game) {}
	Lua::~Lua() {
		while (!mThreads.empty()) {
			auto it = mThreads.begin();
			delete it->second;
			mThreads.erase(it);
		}
	}

	void Lua::Initialize() {
		LuaBase::Initialize();

		RegisterEnums();
		RegisterFunctions();

		LoadFile("data/lua/global.lua");
	}

	void Lua::Reload() {
		LuaBase::Reload();
		GlobalLua::Instance().Reload();

		// reload abilities
	}

	void Lua::PreloadAbilities() {
		// TODO:
	}

	AbilityPtr Lua::GetAbility(const std::string& abilityName) {
		return GetAbility(utils::hash_id(abilityName));
	}

	AbilityPtr Lua::GetAbility(uint32_t abilityId) {
		auto it = mAbilities.find(abilityId);
		if (it == mAbilities.end()) {
			const auto& abilityCode = GlobalLua::Instance().GetAbility(abilityId);
			if (abilityCode.empty()) {
				return nullptr;
			}

			mState.safe_script(abilityCode.as_string_view());
			it = mAbilities.find(abilityId);

			if (it == mAbilities.end()) {
				std::cout << "Did you forget to call RegisterAbility?" << std::endl;
				return nullptr;
			}
		}
		return it->second;
	}

	sol::table Lua::GetPrivateTable(uint32_t id) const {
		if (auto it = mPrivateTables.find(id); it != mPrivateTables.end()) {
			return it->second;
		}
		return sol::nil;
	}

	sol::table Lua::CreatePrivateTable(uint32_t id) {
		auto [it, inserted] = mPrivateTables.try_emplace(id);
		if (inserted) {
			it->second = mState.create_table();
		}
		return it->second;
	}

	void Lua::RemovePrivateTable(uint32_t id) {
		mPrivateTables.erase(id);
	}

	LuaThread* Lua::GetThread(lua_State* L) const {
		if (auto it = mThreads.find(L); it != mThreads.end()) {
			return it->second;
		}
		return nullptr;
	}

	void Lua::RemoveThread(LuaThread* thread) {
		mThreads.erase(mThreads.find(thread->lua_state()));
		delete thread;

		CollectGarbage();
	}

	void Lua::RegisterEnums() {
		// Attribute
		mState.new_enum<AttributeType>("Attribute", {
			{ "Strength", AttributeType::Strength },
			{ "Dexterity", AttributeType::Dexterity },
			{ "Mind", AttributeType::Mind },
			{ "MaxHealthIncrease", AttributeType::MaxHealthIncrease },
			{ "MaxHealth", AttributeType::MaxHealth },
			{ "MaxMana", AttributeType::MaxMana },
			{ "DamageReduction", AttributeType::DamageReduction },
			{ "PhysicalDefense", AttributeType::PhysicalDefense },
			{ "PhysicalDamageReduction", AttributeType::PhysicalDamageReduction },
			{ "EnergyDefense", AttributeType::EnergyDefense },
			{ "CriticalRating", AttributeType::CriticalRating },
			{ "NonCombatSpeed", AttributeType::NonCombatSpeed },
			{ "CombatSpeed", AttributeType::CombatSpeed },
			{ "DamageBuff", AttributeType::DamageBuff },
			{ "Silence", AttributeType::Silence },
			{ "Immobilized", AttributeType::Immobilized },
			{ "DefenseBoostBasicDamage", AttributeType::DefenseBoostBasicDamage },
			{ "PhysicalDamageIncrease", AttributeType::PhysicalDamageIncrease },
			{ "PhysicalDamageIncreaseFlat", AttributeType::PhysicalDamageIncreaseFlat },
			{ "AutoCrit", AttributeType::AutoCrit },
			{ "BehindDirectDamageIncrease", AttributeType::BehindDirectDamageIncrease },
			{ "BehindOrSideDirectDamageIncrease", AttributeType::BehindOrSideDirectDamageIncrease },
			{ "CriticalDamageIncrease", AttributeType::CriticalDamageIncrease },
			{ "AttackSpeedScale", AttributeType::AttackSpeedScale },
			{ "CooldownScale", AttributeType::CooldownScale },
			{ "Frozen", AttributeType::Frozen },
			{ "ProjectileSpeedIncrease", AttributeType::ProjectileSpeedIncrease },
			{ "AoEResistance", AttributeType::AoEResistance },
			{ "EnergyDamageBuff", AttributeType::EnergyDamageBuff },
			{ "Intangible", AttributeType::Intangible },
			{ "HealingReduction", AttributeType::HealingReduction },
			{ "EnergyDamageIncrease", AttributeType::EnergyDamageIncrease },
			{ "EnergyDamageIncreaseFlat", AttributeType::EnergyDamageIncreaseFlat },
			{ "Immune", AttributeType::Immune },
			{ "StealthDetection", AttributeType::StealthDetection },
			{ "LifeSteal", AttributeType::LifeSteal },
			{ "RejectModifier", AttributeType::RejectModifier },
			{ "AoEDamage", AttributeType::AoEDamage },
			{ "TechnologyTypeDamage", AttributeType::TechnologyTypeDamage },
			{ "SpacetimeTypeDamage", AttributeType::SpacetimeTypeDamage },
			{ "LifeTypeDamage", AttributeType::LifeTypeDamage },
			{ "ElementsTypeDamage", AttributeType::ElementsTypeDamage },
			{ "SupernaturalTypeDamage", AttributeType::SupernaturalTypeDamage },
			{ "TechnologyTypeResistance", AttributeType::TechnologyTypeResistance },
			{ "SpacetimeTypeResistance", AttributeType::SpacetimeTypeResistance },
			{ "LifeTypeResistance", AttributeType::LifeTypeResistance },
			{ "ElementsTypeResistance", AttributeType::ElementsTypeResistance },
			{ "SupernaturalTypeResistance", AttributeType::SupernaturalTypeResistance },
			{ "MovementSpeedBuff", AttributeType::MovementSpeedBuff },
			{ "ImmuneToDebuffs", AttributeType::ImmuneToDebuffs },
			{ "BuffDuration", AttributeType::BuffDuration },
			{ "DebuffDuration", AttributeType::DebuffDuration },
			{ "ManaSteal", AttributeType::ManaSteal },
			{ "DebuffDurationIncrease", AttributeType::DebuffDurationIncrease },
			{ "EnergyDamageReduction", AttributeType::EnergyDamageReduction },
			{ "Incorporeal", AttributeType::Incorporeal },
			{ "DoTDamageIncrease", AttributeType::DoTDamageIncrease },
			{ "MindControlled", AttributeType::MindControlled },
			{ "SwapDisabled", AttributeType::SwapDisabled },
			{ "ImmuneToRandomTeleport", AttributeType::ImmuneToRandomTeleport },
			{ "ImmuneToBanish", AttributeType::ImmuneToBanish },
			{ "ImmuneToKnockback", AttributeType::ImmuneToKnockback },
			{ "AoeRadius", AttributeType::AoeRadius },
			{ "PetDamage", AttributeType::PetDamage },
			{ "PetHealth", AttributeType::PetHealth },
			{ "CrystalFind", AttributeType::CrystalFind },
			{ "DNADropped", AttributeType::DNADropped },
			{ "RangeIncrease", AttributeType::RangeIncrease },
			{ "OrbEffectiveness", AttributeType::OrbEffectiveness },
			{ "OverdriveBuildup", AttributeType::OverdriveBuildup },
			{ "OverdriveDuration", AttributeType::OverdriveDuration },
			{ "LootFind", AttributeType::LootFind },
			{ "Surefooted", AttributeType::Surefooted },
			{ "ImmuneToStunned", AttributeType::ImmuneToStunned },
			{ "ImmuneToSleep", AttributeType::ImmuneToSleep },
			{ "ImmuneToTerrified", AttributeType::ImmuneToTerrified },
			{ "ImmuneToSilence", AttributeType::ImmuneToSilence },
			{ "ImmuneToCursed", AttributeType::ImmuneToCursed },
			{ "ImmuneToPoisonOrDisease", AttributeType::ImmuneToPoisonOrDisease },
			{ "ImmuneToBurning", AttributeType::ImmuneToBurning },
			{ "ImmuneToRooted", AttributeType::ImmuneToRooted },
			{ "ImmuneToSlow", AttributeType::ImmuneToSlow },
			{ "ImmuneToPull", AttributeType::ImmuneToPull },
			{ "DoTDamageDoneIncrease", AttributeType::DoTDamageDoneIncrease },
			{ "AggroIncrease", AttributeType::AggroIncrease },
			{ "AggroDecrease", AttributeType::AggroDecrease },
			{ "PhysicalDamageDoneIncrease", AttributeType::PhysicalDamageDoneIncrease },
			{ "PhysicalDamageDoneByAbilityIncrease", AttributeType::PhysicalDamageDoneByAbilityIncrease },
			{ "EnergyDamageDoneIncrease", AttributeType::EnergyDamageDoneIncrease },
			{ "EnergyDamageDoneByAbilityIncrease", AttributeType::EnergyDamageDoneByAbilityIncrease },
			{ "ChannelTimeDecrease", AttributeType::ChannelTimeDecrease },
			{ "CrowdControlDurationDecrease", AttributeType::CrowdControlDurationDecrease },
			{ "DoTDurationDecrease", AttributeType::DoTDurationDecrease },
			{ "AoEDurationIncrease", AttributeType::AoEDurationIncrease },
			{ "HealIncrease", AttributeType::HealIncrease },
			{ "OnLockdown", AttributeType::OnLockdown },
			{ "HoTDoneIncrease", AttributeType::HoTDoneIncrease },
			{ "ProjectileDamageIncrease", AttributeType::ProjectileDamageIncrease },
			{ "DeployBonusInvincibilityTime", AttributeType::DeployBonusInvincibilityTime },
			{ "PhysicalDamageDecreaseFlat", AttributeType::PhysicalDamageDecreaseFlat },
			{ "EnergyDamageDecreaseFlat", AttributeType::EnergyDamageDecreaseFlat },
			{ "MinWeaponDamage", AttributeType::MinWeaponDamage },
			{ "MaxWeaponDamage", AttributeType::MaxWeaponDamage },
			{ "MinWeaponDamagePercent", AttributeType::MinWeaponDamagePercent },
			{ "MaxWeaponDamagePercent", AttributeType::MaxWeaponDamagePercent },
			{ "DirectAttackDamage", AttributeType::DirectAttackDamage },
			{ "DirectAttackDamagePercent", AttributeType::DirectAttackDamagePercent },
			{ "GetHitAnimDisabled", AttributeType::GetHitAnimDisabled },
			{ "XPBoost", AttributeType::XPBoost },
			{ "InvisibleToSecurityTeleporters", AttributeType::InvisibleToSecurityTeleporters },
			{ "BodyScale", AttributeType::BodyScale }
		});

		// NounType
		mState.new_enum<NounType>("NounType", {
			{ "None", NounType::None },
			{ "Creature", NounType::Creature },
			{ "Vehicle", NounType::Vehicle },
			{ "Obstacle", NounType::Obstacle },
			{ "SpawnPoint", NounType::SpawnPoint },
			{ "PathPoint", NounType::PathPoint },
			{ "Trigger", NounType::Trigger },
			{ "PointLight", NounType::PointLight },
			{ "SpotLight", NounType::SpotLight },
			{ "LineLight", NounType::LineLight },
			{ "ParallelLight", NounType::ParallelLight },
			{ "HemisphereLight", NounType::HemisphereLight },
			{ "Animator", NounType::Animator },
			{ "Animated", NounType::Animated },
			{ "GraphicsControl", NounType::GraphicsControl },
			{ "Material", NounType::Material },
			{ "Flora", NounType::Flora },
			{ "LevelshopObject", NounType::LevelshopObject },
			{ "Terrain", NounType::Terrain },
			{ "Weapon", NounType::Weapon },
			{ "Building", NounType::Building },
			{ "Handle", NounType::Handle },
			{ "HealthOrb", NounType::HealthOrb },
			{ "ManaOrb", NounType::ManaOrb },
			{ "ResurrectOrb", NounType::ResurrectOrb },
			{ "Movie", NounType::Movie },
			{ "Loot", NounType::Loot },
			{ "PlacableEffect", NounType::PlacableEffect },
			{ "LuaJob", NounType::LuaJob },
			{ "AbilityObject", NounType::AbilityObject },
			{ "LevelExitPoint", NounType::LevelExitPoint },
			{ "Decal", NounType::Decal },
			{ "Water", NounType::Water },
			{ "Grass", NounType::Grass },
			{ "Door", NounType::Door },
			{ "Crystal", NounType::Crystal },
			{ "Interactable", NounType::Interactable },
			{ "Projectile", NounType::Projectile },
			{ "DestructibleOrnament", NounType::DestructibleOrnament },
			{ "MapCamera", NounType::MapCamera },
			{ "Occluder", NounType::Occluder },
			{ "SplineCamera", NounType::SplineCamera },
			{ "SplineCameraNode", NounType::SplineCameraNode },
			{ "BossPortal", NounType::BossPortal }
		});

		// ClientEventID
		mState.new_enum<ClientEventID>("ClientEventID", {
			{ "AbilityRequiresTarget", ClientEventID::AbilityRequiresTarget },
			{ "AbilityNotReady", ClientEventID::AbilityNotReady },
			{ "SquadAbilityUnavailable", ClientEventID::SquadAbilityUnavailable },
			{ "OverdriveNotCharged", ClientEventID::OverdriveNotCharged },
			{ "OverdriveReady", ClientEventID::OverdriveReady },
			{ "PlayerReady", ClientEventID::PlayerReady },
			{ "PlayerCashOut", ClientEventID::PlayerCashOut },
			{ "PlayerDeath", ClientEventID::PlayerDeath },
			{ "PlayerFreed", ClientEventID::PlayerFreed },
			{ "PlayerLeftGame", ClientEventID::PlayerLeftGame },
			{ "PlayerTrapped", ClientEventID::PlayerTrapped },
			{ "PlayerUnlockedSecondCreature", ClientEventID::PlayerUnlockedSecondCreature },
			{ "PlayerEnteredTunnel", ClientEventID::PlayerEnteredTunnel },
			{ "PlayerExitedTunnel", ClientEventID::PlayerExitedTunnel },
			{ "GameOver", ClientEventID::GameOver },
			{ "LootRoll", ClientEventID::LootRoll },
			{ "LootPickup", ClientEventID::LootPickup },
			{ "LootPlayerDrop", ClientEventID::LootPlayerDrop },
			{ "CatalystPickup", ClientEventID::CatalystPickup },
			{ "TeleportersDeactivated", ClientEventID::TeleportersDeactivated },
			{ "SwitchUnavailable", ClientEventID::SwitchUnavailable },
			{ "InvalidTargetArea", ClientEventID::InvalidTargetArea },
			{ "CannotSwitchHeroPerished", ClientEventID::CannotSwitchHeroPerished },
			{ "WarningFinalHero", ClientEventID::WarningFinalHero },
			{ "SwitchOnCooldown", ClientEventID::SwitchOnCooldown },
			{ "AllyReconnected", ClientEventID::AllyReconnected },
			{ "AllyDisconnected", ClientEventID::AllyDisconnected },
			{ "NotEnoughMana", ClientEventID::NotEnoughMana },
			{ "HealthLow", ClientEventID::HealthLow },
			{ "ManaLow", ClientEventID::ManaLow },
			{ "HordeIncoming", ClientEventID::HordeIncoming },
			{ "HordeDefeated", ClientEventID::HordeDefeated }
		});

		// StealthType
		mState.new_enum<StealthType>("StealthType", {
			{ "None", StealthType::None },
			{ "Technology", StealthType::Technology },
			{ "Supernatural", StealthType::Supernatural },
			{ "FullyInvisible", StealthType::FullyInvisible }
		});

		// DamageSource
		mState.new_enum<DamageSource>("DamageSource", {
			{ "Physical", DamageSource::Physical },
			{ "Energy", DamageSource::Energy }
		});

		// DamageType
		mState.new_enum<DamageType>("DamageType", {
			{ "Technology", DamageType::Technology },
			{ "Spacetime", DamageType::Spacetime },
			{ "Life", DamageType::Life },
			{ "Elements", DamageType::Elements },
			{ "Supernatural", DamageType::Supernatural },
			{ "Generic", DamageType::Generic }
		});
	}

	void Lua::RegisterFunctions() {
		// global functions
		mState["AddEvent"] = &LuaFunction::AddEvent;
		mState["StopEvent"] = &LuaFunction::StopEvent;
		mState["RegisterAbility"] = [this](std::string abilityName, sol::table abilityTable) {
			auto id = utils::hash_id(abilityName);
			if (auto [it, inserted] = mAbilities.try_emplace(id); inserted) {
				abilityTable["id"] = id;
				it->second = std::make_shared<Ability>(std::move(abilityTable));
			}
		};

		// nThread
		auto nThreadTable = mState.create_named_table("nThread");
		nThreadTable["WaitForXSeconds"] = sol::yielding(&LuaFunction::nThread_WaitForXSeconds);

		// Game
		auto gameTable = mState.create_named_table("Game");
		{
			auto gameMetatable = mState.create_table_with();

			// variables
			gameMetatable["Instance"] = &mGame;
			gameMetatable["Level"] = mGame.GetChainData().GetLevelIndex();

			// functions
			gameMetatable["GetPlayer"] = &LuaFunction::Game_GetPlayer;
			gameMetatable["Notify"] = &LuaFunction::Game_Notify;

			// meta
			gameMetatable[sol::meta_function::new_index] = [](lua_State* L) { return luaL_error(L, "You may not edit the Game table."); };
			gameMetatable[sol::meta_function::index] = gameMetatable;

			gameTable[sol::metatable_key] = gameMetatable;
		}

		// nObjectManager
		{
			auto nObjectManagerTable = mState.create_named_table("nObjectManager");
			nObjectManagerTable["GetObject"] = &LuaFunction::nObjectManager_GetObject;
			nObjectManagerTable["CreateObject"] = &LuaFunction::nObjectManager_CreateObject;

			nObjectManagerTable["GetTrigger"] = &LuaFunction::nObjectManager_GetTrigger;
			nObjectManagerTable["CreateTriggerVolume"] = &LuaFunction::nObjectManager_CreateTriggerVolume;

			nObjectManagerTable["GetObjectsInRadius"] = &LuaFunction::nObjectManager_GetObjectsInRadius;
			nObjectManagerTable["GetObjectsInRadius_SortedByDistance"] = &LuaFunction::nObjectManager_GetObjectsInRadius_SortedByDistance;
		}

		// Locomotion
		{
			auto locomotionTable = mState.create_named_table("Locomotion");

			// object functions (object as arg0)
			locomotionTable["TeleportObject"] = &LuaFunction::Locomotion_TeleportObject;
			locomotionTable["Stop"] = &LuaFunction::Locomotion_Stop;

			// position functions (vec3 as arg0)
			locomotionTable["GetClosestPosition"] = &LuaFunction::Locomotion_GetClosestPosition;
		}

		// Player
		{
			auto playerType = mState.new_usertype<Player>("Player", sol::no_constructor);
			playerType["GetCharacter"] = &Player::GetCharacterObject;
			playerType["GetDeployedCharacter"] = &Player::GetDeployedCharacterObject;

			playerType["Notify"] = &LuaFunction::Player_Notify;
		}

		// Attributes
		{
			auto attributesType = mState.new_usertype<Game::Attributes>("Attributes", sol::no_constructor);
			attributesType["GetValue"] = sol::resolve<float(uint8_t) const>(&Game::Attributes::GetValue);
			attributesType["SetValue"] = sol::resolve<void(uint8_t, float)>(&Game::Attributes::SetValue);
		}

		// Object
		{
			auto objectType = mState.new_usertype<Object>("Object", sol::no_constructor);
			objectType["GetId"] = &Object::GetId;
			objectType["GetPrivateTable"] = &LuaFunction::Object_GetPrivateTable;

			objectType["GetPosition"] = &Object::GetPosition;
			objectType["SetPosition"] = &Object::SetPosition;

			objectType["GetExtent"] = &Object::GetExtent;
			objectType["SetExtent"] = &Object::SetExtent;

			objectType["GetFacing"] = &LuaFunction::Object_GetFacing;
			objectType["GetFootprintRadius"] = &LuaFunction::Object_GetFootprintRadius;

			objectType["SetTickOverride"] = sol::resolve<void(sol::protected_function)>(&Object::SetTickOverride);

			objectType["Move"] = &LuaFunction::Object_Move;

			objectType["GetHealth"] = &Object::GetHealth;
			objectType["GetMaxHealth"] = &Object::GetMaxHealth;
			objectType["SetHealth"] = &Object::SetHealth;

			objectType["GetMana"] = &Object::GetMana;
			objectType["GetMaxMana"] = &Object::GetMaxMana;
			objectType["SetMana"] = &Object::SetMana;

			objectType["SetAnimationState"] = &LuaFunction::Object_SetAnimationState;
			objectType["SetAnimationStateToDeath"] = &LuaFunction::Object_SetAnimationStateToDeath;
			objectType["SetAnimationStateToAggro"] = &LuaFunction::Object_SetAnimationStateToAggro;
			objectType["SetGraphicsState"] = &LuaFunction::Object_SetGraphicsState;

			objectType["GetAttributeSnapshot"] = &LuaFunction::Object_GetAttributeSnapshot;
			objectType["GetAttributeValue"] = sol::resolve<float(uint8_t) const>(&Object::GetAttributeValue);
			objectType["SetAttributeValue"] = sol::resolve<void(uint8_t, float)>(&Object::SetAttributeValue);

			objectType["GetWeaponDamage"] = &LuaFunction::Object_GetWeaponDamage;

			objectType["GetMarkerId"] = &Object::GetMarkerId;
			objectType["SetMarkerId"] = &Object::SetMarkerId;

			objectType["GetScale"] = &Object::GetScale;
			objectType["SetScale"] = &Object::SetScale;

			objectType["GetTeam"] = &Object::GetTeam;
			objectType["SetTeam"] = &Object::SetTeam;

			objectType["IsPlayerControlled"] = &Object::IsPlayerControlled;
			objectType["GetPlayerIndex"] = &Object::GetPlayerIndex;

			objectType["IsVisible"] = &Object::IsVisible;
			objectType["SetVisible"] = &Object::SetVisible;

			objectType["IsMarkedForDeletion"] = &Object::IsMarkedForDeletion;
			objectType["MarkForDeletion"] = &Object::MarkForDeletion;

			objectType["AddCooldown"] = &LuaFunction::Object_AddCooldown;
			objectType["RemoveCooldown"] = &LuaFunction::Object_RemoveCooldown;
			objectType["ScaleCooldown"] = &LuaFunction::Object_ScaleCooldown;

			objectType["TakeDamage"] = &LuaFunction::Object_TakeDamage;
			objectType["Heal"] = &LuaFunction::Object_Heal;

			objectType["AddEffect"] = &LuaFunction::Object_AddEffect;
			objectType["RemoveEffect"] = &LuaFunction::Object_RemoveEffect;
			objectType["RemoveEffectIndex"] = &LuaFunction::Object_RemoveEffectIndex;

			objectType["UseAbility"] = &LuaFunction::Object_UseAbility;
			objectType["DropLoot"] = &LuaFunction::Object_DropLoot;

			// Agent blackboard
			objectType["GetTargetId"] = &Object::GetTargetId;
			objectType["SetTargetId"] = &Object::SetTargetId;

			objectType["GetStealthType"] = &Object::GetStealthType;
			objectType["SetStealthType"] = &Object::SetStealthType;

			objectType["IsInCombat"] = &Object::IsInCombat;
			objectType["SetInCombat"] = &Object::SetInCombat;

			objectType["IsTargetable"] = &Object::IsTargetable;
			objectType["SetTargetable"] = &Object::SetTargetable;

			// Markers
			objectType["GetTeleporterDestination"] = &LuaFunction::Object_GetTeleporterDestination;
		}

		// TriggerVolume
		{
			auto triggerType = mState.new_usertype<TriggerVolume>("TriggerVolume", sol::no_constructor, sol::base_classes, sol::bases<Object>());

			triggerType["Attach"] = &LuaFunction::Trigger_Attach;
			triggerType["Detach"] = &LuaFunction::Trigger_Detach;

			triggerType["SetOnEnterCallback"] = &TriggerVolume::SetOnEnterCallback;
			triggerType["SetOnExitCallback"] = &TriggerVolume::SetOnExitCallback;
			triggerType["SetOnStayCallback"] = &TriggerVolume::SetOnStayCallback;
		}
	}

	// LuaThread
	LuaThread::LuaThread(Lua& lua, sol::function&& func, sol::environment&& env) : mLua(lua), mEnvironment(std::move(env)) {
		sol::state_view state = mEnvironment.lua_state();

		mThread = sol::thread::create(state);
		mCoroutine = sol::coroutine(mThread.thread_state(), std::move(func));

		mEnvironment.set_on(mThread);
	}
}
