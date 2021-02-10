
// Include
#include "lua.h"

#include "instance.h"
#include "objectmanager.h"

#include "scheduler.h"

#include "raknet/server.h"

#include "utils/functions.h"
#include "utils/log.h"

#include <algorithm>

// Helpers
namespace {
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
}

// LuaFunction
namespace LuaFunction {
	// global functions
	auto AddEvent(sol::this_state L, sol::function callback, uint32_t delay, sol::variadic_args args) {
		sol::state_view state(L);

		Game::Instance& game = state["Game"]["Instance"];
		return Scheduler::AddTask(delay, [&game, callback = std::move(callback), args = std::vector<sol::object>(args.begin(), args.end())]() mutable {
			game.AddServerTask([callback = std::move(callback), args = std::move(args)]() mutable {
				callback.call<void>(sol::as_args(args));
			});
		});
	}

	auto StopEvent(uint32_t id) {
		Scheduler::CancelTask(id);
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
		sol::state_view state(L);
		
		Game::Instance& game = state["Game"]["Instance"];
		Game::LuaThread* thread = game.GetLua().GetThread(L);
		if (!thread) {
			return;
		}

		// TODO: add events to game event list as to not crash when a game closes
		Scheduler::AddTask(static_cast<uint32_t>(seconds * 1000), [&game, thread]() mutable {
			game.AddServerTask([thread]() mutable { thread->call(); });
		});
	}

	// Game
	auto Game_GetPlayer(sol::this_state L, uint8_t index) {
		sol::state_view state(L);

		Game::Instance& game = state["Game"]["Instance"];
		return game.GetPlayerByIndex(index);
	}

	// nObjectManager
	auto nObjectManager_GetObject(sol::this_state L, uint32_t id) {
		sol::state_view state(L);

		Game::Instance& game = state["Game"]["Instance"];
		return game.GetObjectManager().Get(id);
	}

	auto nObjectManager_CreateObject(sol::this_state L, uint32_t noun, glm::vec3 position) {
		sol::state_view state(L);

		Game::Instance& game = state["Game"]["Instance"];

		auto object = game.GetObjectManager().Create(noun);
		if (object) {
			object->SetPosition(position);
			game.SendObjectCreate(object);
		}

		return object;
	}

	auto nObjectManager_GetTrigger(sol::this_state L, uint32_t id) {
		sol::state_view state(L);

		Game::Instance& game = state["Game"]["Instance"];
		return game.GetObjectManager().GetTrigger(id);
	}

	auto nObjectManager_CreateTriggerVolume(sol::this_state L, glm::vec3 position, float radius, sol::function onEnter, sol::function onExit, sol::function onStay) {
		sol::state_view state(L);

		Game::Instance& game = state["Game"]["Instance"];

		auto object = game.GetObjectManager().CreateTrigger(position, radius, std::move(onEnter), std::move(onExit), std::move(onStay));
		if (object) {
			object->SetPosition(position);
			game.SendObjectCreate(object);
		}

		return object;
	}

	auto nObjectManager_GetObjectsInRadius(sol::this_state L, glm::vec3 position, float radius, sol::table objectTypes) {
		sol::state_view state(L);

		Game::ObjectManager& objectManager = state["Game"]["ObjectManager"];

		std::vector<Game::NounType> types;
		for (const auto& entry : objectTypes) {
			types.push_back(static_cast<Game::NounType>(entry.second.as<uint32_t>()));
		}

		auto objects = objectManager.GetObjectsInRadius(position, radius, types);
		auto objectCount = static_cast<int>(objects.size());

		auto result = state.create_table(objectCount, 0);
		for (int i = 0; i < objectCount; ++i) {
			result[i + 1] = objects[i];
		}

		return result;
	}

	// Object
	auto Object_Move(sol::this_state L, sol::object objectValue, glm::vec3 position, bool teleport) {
		sol::state_view state(L);

		Game::Instance& game = state["Game"]["Instance"];
		const auto& objectManager = game.GetObjectManager();

		Game::ObjectPtr object = LuaGetObject(objectManager, objectValue);
		if (object) {
			RakNet::LocomotionData data;
			data.SetGoalPosition(position);
			if (teleport) {
				data.mGoalFlags |= 0x020;
			}

			game.MoveObject(object, data);
		}
	}

	auto Object_AddEffect(sol::this_state L, sol::object objectValue, std::string serverEventDef, sol::optional<sol::object> secondaryObjectValue) {
		sol::state_view state(L);

		Game::Instance& game = state["Game"]["Instance"];
		const auto& objectManager = game.GetObjectManager();

		Game::ObjectPtr object = LuaGetObject(objectManager, objectValue);
		if (object) {
			auto effect = utils::hash_id(serverEventDef);
			auto index = object->AddEffect(effect);
			if (index == 0xFF) {
				return index;
			}

			RakNet::ServerEvent serverEvent;
			serverEvent.ServerEventDef = effect;
			serverEvent.objectFxIndex = index;
			serverEvent.bForceAttach = true;
			serverEvent.objectId = object->GetId();
			serverEvent.targetPoint = object->GetPosition();

			if (secondaryObjectValue) {
				Game::ObjectPtr secondaryObject = LuaGetObject(objectManager, *secondaryObjectValue);
				if (secondaryObject) {
					serverEvent.secondaryObjectId = secondaryObject->GetId();
				}
			}

			game.SendServerEvent(serverEvent);
			return serverEvent.objectFxIndex;
		}

		return static_cast<decltype(RakNet::ServerEvent::objectFxIndex)>(0);
	}

	auto Object_RemoveEffectIndex(sol::this_state L, sol::object objectValue, uint8_t index) {
		sol::state_view state(L);

		Game::Instance& game = state["Game"]["Instance"];
		const auto& objectManager = game.GetObjectManager();

		Game::ObjectPtr object = LuaGetObject(objectManager, objectValue);
		if (object) {
			auto removed = object->RemoveEffectByIndex(index);
			if (removed) {
				RakNet::ServerEvent serverEvent;
				serverEvent.objectId = object->GetId();
				serverEvent.objectFxIndex = index;
				serverEvent.bRemove = true;
				serverEvent.bHardStop = true; // variable

				game.SendServerEvent(serverEvent);
			}
		}
	}

	auto Object_SetAnimationState(sol::this_state L, sol::object objectValue, uint32_t animationState, bool overlay) {
		sol::state_view state(L);

		Game::Instance& game = state["Game"]["Instance"];
		game.SendAnimationState(LuaGetObject(game.GetObjectManager(), objectValue), animationState, overlay);
	}

	auto Object_SetGraphicsState(sol::this_state L, sol::object objectValue, uint32_t graphicsState) {
		sol::state_view state(L);

		Game::Instance& game = state["Game"]["Instance"];
		game.SendObjectGfxState(LuaGetObject(game.GetObjectManager(), objectValue), graphicsState);
	}

	auto Object_TakeDamage(sol::this_state L, sol::object objectValue, sol::object attackerValue) {

	}

	auto Object_Heal(sol::this_state L, sol::object objectValue, sol::object attackerValue) {

	}

	// Trigger
	auto Trigger_Attach(sol::this_state L, sol::object triggerValue, sol::object objectValue) {
		sol::state_view state(L);

		Game::ObjectManager& objectManager = state["Game"]["ObjectManager"];

		Game::TriggerVolumePtr trigger = LuaGetTrigger(objectManager, triggerValue);
		Game::ObjectPtr object = LuaGetObject(objectManager, objectValue);
		if (trigger && object) {
			trigger->Attach(object);
		}
	}

	auto Trigger_Detach(sol::this_state L, sol::object triggerValue, sol::object objectValue) {
		sol::state_view state(L);

		Game::ObjectManager& objectManager = state["Game"]["ObjectManager"];

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
	// Lua
	Lua::Lua(Instance& game) : mGame(game) {}
	Lua::~Lua() {
		Terminate();
	}

	void Lua::Initialize() {
		mState.open_libraries();
		mState.set_exception_handler([](lua_State* L, sol::optional<const std::exception&> maybe_exception, sol::string_view description) -> int {
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

		RegisterEnums();
		RegisterFunctions();

		LoadFile("data/lua/global.lua");
	}

	void Lua::Terminate() {
		mState.collect_garbage();
		mState.collect_garbage();
		mState = nullptr;
	}

	sol::protected_function_result Lua::ReportError(lua_State* L, sol::protected_function_result pfr) {
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

	const char* Lua::GetStackTrace(lua_State* L, const char* error) {
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

	bool Lua::LoadFile(const std::string& path) {
		auto result = mState.safe_script_file(path, &Lua::ReportError);
		return result.valid();
	}

	bool Lua::LoadBuffer(const std::string& buffer) {
		auto result = mState.safe_script(buffer, &Lua::ReportError);
		return result.valid();
	}

	lua_State* Lua::GetState() const {
		return mState;
	}

	LuaThread* Lua::GetThread(lua_State* L) const {
		auto it = mThreads.find(L);
		if (it != mThreads.end()) {
			return it->second;
		}
		return nullptr;
	}

	void Lua::RemoveThread(LuaThread* thread) {
		mThreads.erase(mThreads.find(thread->lua_state()));
		delete thread;

		mState.collect_garbage();
		mState.collect_garbage();
	}

	void Lua::RegisterEnums() {
		// Attribute
		mState.new_enum<Attribute::Type>("Attribute", {
			{ "Strength", Attribute::Strength },
			{ "Dexterity", Attribute::Dexterity },
			{ "Mind", Attribute::Mind },
			{ "MaxHealthIncrease", Attribute::MaxHealthIncrease },
			{ "MaxHealth", Attribute::MaxHealth },
			{ "MaxMana", Attribute::MaxMana },
			{ "DamageReduction", Attribute::DamageReduction },
			{ "PhysicalDefense", Attribute::PhysicalDefense },
			{ "PhysicalDamageReduction", Attribute::PhysicalDamageReduction },
			{ "EnergyDefense", Attribute::EnergyDefense },
			{ "CriticalRating", Attribute::CriticalRating },
			{ "NonCombatSpeed", Attribute::NonCombatSpeed },
			{ "CombatSpeed", Attribute::CombatSpeed },
			{ "DamageBuff", Attribute::DamageBuff },
			{ "Silence", Attribute::Silence },
			{ "Immobilized", Attribute::Immobilized },
			{ "DefenseBoostBasicDamage", Attribute::DefenseBoostBasicDamage },
			{ "PhysicalDamageIncrease", Attribute::PhysicalDamageIncrease },
			{ "PhysicalDamageIncreaseFlat", Attribute::PhysicalDamageIncreaseFlat },
			{ "AutoCrit", Attribute::AutoCrit },
			{ "BehindDirectDamageIncrease", Attribute::BehindDirectDamageIncrease },
			{ "BehindOrSideDirectDamageIncrease", Attribute::BehindOrSideDirectDamageIncrease },
			{ "CriticalDamageIncrease", Attribute::CriticalDamageIncrease },
			{ "AttackSpeedScale", Attribute::AttackSpeedScale },
			{ "CooldownScale", Attribute::CooldownScale },
			{ "Frozen", Attribute::Frozen },
			{ "ProjectileSpeedIncrease", Attribute::ProjectileSpeedIncrease },
			{ "AoEResistance", Attribute::AoEResistance },
			{ "EnergyDamageBuff", Attribute::EnergyDamageBuff },
			{ "Intangible", Attribute::Intangible },
			{ "HealingReduction", Attribute::HealingReduction },
			{ "EnergyDamageIncrease", Attribute::EnergyDamageIncrease },
			{ "EnergyDamageIncreaseFlat", Attribute::EnergyDamageIncreaseFlat },
			{ "Immune", Attribute::Immune },
			{ "StealthDetection", Attribute::StealthDetection },
			{ "LifeSteal", Attribute::LifeSteal },
			{ "RejectModifier", Attribute::RejectModifier },
			{ "AoEDamage", Attribute::AoEDamage },
			{ "TechnologyTypeDamage", Attribute::TechnologyTypeDamage },
			{ "SpacetimeTypeDamage", Attribute::SpacetimeTypeDamage },
			{ "LifeTypeDamage", Attribute::LifeTypeDamage },
			{ "ElementsTypeDamage", Attribute::ElementsTypeDamage },
			{ "SupernaturalTypeDamage", Attribute::SupernaturalTypeDamage },
			{ "TechnologyTypeResistance", Attribute::TechnologyTypeResistance },
			{ "SpacetimeTypeResistance", Attribute::SpacetimeTypeResistance },
			{ "LifeTypeResistance", Attribute::LifeTypeResistance },
			{ "ElementsTypeResistance", Attribute::ElementsTypeResistance },
			{ "SupernaturalTypeResistance", Attribute::SupernaturalTypeResistance },
			{ "MovementSpeedBuff", Attribute::MovementSpeedBuff },
			{ "ImmuneToDebuffs", Attribute::ImmuneToDebuffs },
			{ "BuffDuration", Attribute::BuffDuration },
			{ "DebuffDuration", Attribute::DebuffDuration },
			{ "ManaSteal", Attribute::ManaSteal },
			{ "DebuffDurationIncrease", Attribute::DebuffDurationIncrease },
			{ "EnergyDamageReduction", Attribute::EnergyDamageReduction },
			{ "Incorporeal", Attribute::Incorporeal },
			{ "DoTDamageIncrease", Attribute::DoTDamageIncrease },
			{ "MindControlled", Attribute::MindControlled },
			{ "SwapDisabled", Attribute::SwapDisabled },
			{ "ImmuneToRandomTeleport", Attribute::ImmuneToRandomTeleport },
			{ "ImmuneToBanish", Attribute::ImmuneToBanish },
			{ "ImmuneToKnockback", Attribute::ImmuneToKnockback },
			{ "AoeRadius", Attribute::AoeRadius },
			{ "PetDamage", Attribute::PetDamage },
			{ "PetHealth", Attribute::PetHealth },
			{ "CrystalFind", Attribute::CrystalFind },
			{ "DNADropped", Attribute::DNADropped },
			{ "RangeIncrease", Attribute::RangeIncrease },
			{ "OrbEffectiveness", Attribute::OrbEffectiveness },
			{ "OverdriveBuildup", Attribute::OverdriveBuildup },
			{ "OverdriveDuration", Attribute::OverdriveDuration },
			{ "LootFind", Attribute::LootFind },
			{ "Surefooted", Attribute::Surefooted },
			{ "ImmuneToStunned", Attribute::ImmuneToStunned },
			{ "ImmuneToSleep", Attribute::ImmuneToSleep },
			{ "ImmuneToTerrified", Attribute::ImmuneToTerrified },
			{ "ImmuneToSilence", Attribute::ImmuneToSilence },
			{ "ImmuneToCursed", Attribute::ImmuneToCursed },
			{ "ImmuneToPoisonOrDisease", Attribute::ImmuneToPoisonOrDisease },
			{ "ImmuneToBurning", Attribute::ImmuneToBurning },
			{ "ImmuneToRooted", Attribute::ImmuneToRooted },
			{ "ImmuneToSlow", Attribute::ImmuneToSlow },
			{ "ImmuneToPull", Attribute::ImmuneToPull },
			{ "DoTDamageDoneIncrease", Attribute::DoTDamageDoneIncrease },
			{ "AggroIncrease", Attribute::AggroIncrease },
			{ "AggroDecrease", Attribute::AggroDecrease },
			{ "PhysicalDamageDoneIncrease", Attribute::PhysicalDamageDoneIncrease },
			{ "PhysicalDamageDoneByAbilityIncrease", Attribute::PhysicalDamageDoneByAbilityIncrease },
			{ "EnergyDamageDoneIncrease", Attribute::EnergyDamageDoneIncrease },
			{ "EnergyDamageDoneByAbilityIncrease", Attribute::EnergyDamageDoneByAbilityIncrease },
			{ "ChannelTimeDecrease", Attribute::ChannelTimeDecrease },
			{ "CrowdControlDurationDecrease", Attribute::CrowdControlDurationDecrease },
			{ "DoTDurationDecrease", Attribute::DoTDurationDecrease },
			{ "AoEDurationIncrease", Attribute::AoEDurationIncrease },
			{ "HealIncrease", Attribute::HealIncrease },
			{ "OnLockdown", Attribute::OnLockdown },
			{ "HoTDoneIncrease", Attribute::HoTDoneIncrease },
			{ "ProjectileDamageIncrease", Attribute::ProjectileDamageIncrease },
			{ "DeployBonusInvincibilityTime", Attribute::DeployBonusInvincibilityTime },
			{ "PhysicalDamageDecreaseFlat", Attribute::PhysicalDamageDecreaseFlat },
			{ "EnergyDamageDecreaseFlat", Attribute::EnergyDamageDecreaseFlat },
			{ "MinWeaponDamage", Attribute::MinWeaponDamage },
			{ "MaxWeaponDamage", Attribute::MaxWeaponDamage },
			{ "MinWeaponDamagePercent", Attribute::MinWeaponDamagePercent },
			{ "MaxWeaponDamagePercent", Attribute::MaxWeaponDamagePercent },
			{ "DirectAttackDamage", Attribute::DirectAttackDamage },
			{ "DirectAttackDamagePercent", Attribute::DirectAttackDamagePercent },
			{ "GetHitAnimDisabled", Attribute::GetHitAnimDisabled },
			{ "XPBoost", Attribute::XPBoost },
			{ "InvisibleToSecurityTeleporters", Attribute::InvisibleToSecurityTeleporters },
			{ "BodyScale", Attribute::BodyScale }
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
	}

	void Lua::RegisterFunctions() {
		// global functions
		mState["AddEvent"] = &LuaFunction::AddEvent;
		mState["StopEvent"] = &LuaFunction::StopEvent;

		// os
		auto osTable = mState.create_named_table("os");
		osTable["mtime"] = &LuaFunction::OsMtime;

		// glm
		{
			mState.new_usertype<glm::vec3>("vec3",
				sol::constructors<glm::vec3(float, float, float)>(),

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
				"z", &glm::vec3::z
			);
		}

		// nUtil
		auto nUtilTable = mState.create_named_table("nUtil");
		nUtilTable["SPID"] = &LuaFunction::nUtil_SPID;
		nUtilTable["ToGUID"] = &LuaFunction::nUtil_ToGUID;

		// nThread
		auto nThreadTable = mState.create_named_table("nThread");
		nThreadTable["WaitForXSeconds"] = sol::yielding(&LuaFunction::nThread_WaitForXSeconds);

		// Game
		auto gameTable = mState.create_named_table("Game");
		{
			auto gameMetatable = mState.create_table_with();

			// variables
			gameMetatable["Instance"] = &mGame;
			gameMetatable["ObjectManager"] = &mGame.GetObjectManager();
			gameMetatable["Level"] = mGame.GetChainData().GetLevelIndex();

			// functions
			gameMetatable["GetPlayer"] = &LuaFunction::Game_GetPlayer;

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
		}

		// Player
		{
			auto playerType = mState.new_usertype<Player>("Player", sol::no_constructor);
			playerType["GetCharacter"] = &Player::GetCharacterObject;
			playerType["GetDeployedCharacter"] = &Player::GetDeployedCharacterObject;
		}

		// Object
		{
			auto objectType = mState.new_usertype<Object>("Object", sol::no_constructor);
			objectType["GetId"] = &Object::GetId;

			objectType["GetPosition"] = &Object::GetPosition;
			objectType["SetPosition"] = &Object::SetPosition;

			objectType["SetTickOverride"] = &Object::SetTickOverride;

			objectType["Move"] = &LuaFunction::Object_Move;

			objectType["GetHealth"] = &Object::GetHealth;
			objectType["GetMaxHealth"] = &Object::GetMaxHealth;
			objectType["SetHealth"] = &Object::SetHealth;

			objectType["GetMana"] = &Object::GetMana;
			objectType["GetMaxMana"] = &Object::GetMaxMana;
			objectType["SetMana"] = &Object::SetMana;

			objectType["SetAnimationState"] = &LuaFunction::Object_SetAnimationState;
			objectType["SetGraphicsState"] = &LuaFunction::Object_SetGraphicsState;

			objectType["GetAttributeValue"] = &Object::GetAttributeValue;
			objectType["SetAttributeValue"] = &Object::SetAttributeValue;

			objectType["GetScale"] = &Object::GetScale;
			objectType["SetScale"] = &Object::SetScale;

			objectType["GetTeam"] = &Object::GetTeam;
			objectType["SetTeam"] = &Object::SetTeam;

			objectType["IsPlayerControlled"] = &Object::IsPlayerControlled;

			objectType["IsVisible"] = &Object::IsVisible;
			objectType["SetVisible"] = &Object::SetVisible;

			objectType["IsMarkedForDeletion"] = &Object::IsMarkedForDeletion;
			objectType["MarkForDeletion"] = &Object::MarkForDeletion;

			objectType["TakeDamage"] = &LuaFunction::Object_TakeDamage;
			objectType["Heal"] = &LuaFunction::Object_Heal;

			objectType["AddEffect"] = &LuaFunction::Object_AddEffect;
			objectType["RemoveEffectIndex"] = &LuaFunction::Object_RemoveEffectIndex;

			// Agent blackboard
			objectType["GetTargetId"] = &Object::GetTargetId;
			objectType["SetTargetId"] = &Object::SetTargetId;

			objectType["GetStealthType"] = &Object::GetStealthType;
			objectType["SetStealthType"] = &Object::SetStealthType;

			objectType["IsInCombat"] = &Object::IsInCombat;
			objectType["SetInCombat"] = &Object::SetInCombat;

			objectType["IsTargetable"] = &Object::IsTargetable;
			objectType["SetTargetable"] = &Object::SetTargetable;
		}

		// TriggerVolume
		{
			auto triggerType = mState.new_usertype<TriggerVolume>("TriggerVolume", sol::no_constructor, sol::base_classes, sol::bases<Object>());

			triggerType["Attach"] = &LuaFunction::Trigger_Attach;
			triggerType["Detach"] = &LuaFunction::Trigger_Detach;
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
