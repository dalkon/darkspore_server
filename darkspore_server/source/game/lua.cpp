
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

// Helpers
namespace {
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
		sol::state_view state(L);

		Game::Instance& game = state["Game"]["Instance"];
		if (delay > 0) {
			game.AddTask(delay, [&game, callback = std::move(callback), args = std::vector<sol::object>(args.begin(), args.end())](uint32_t id) mutable {
				game.AddServerTask([&game, id, callback = std::move(callback), args = std::move(args)]() mutable {
					game.CancelTask(id);
					callback.call<void>(sol::as_args(args));
				});
			});
		} else {
			callback.call<void>(args);
		}
	}

	auto StopEvent(sol::this_state L, uint32_t id) {
		sol::state_view state(L);

		Game::Instance& game = state["Game"]["Instance"];
		game.CancelTask(id);
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

		game.AddTask(static_cast<uint32_t>(seconds * 1000), [&game, thread](uint32_t id) mutable {
			game.AddServerTask([&game, thread, id]() mutable {
				game.CancelTask(id);
				thread->call();
			});
		});
	}

	// Game
	auto Game_GetPlayer(sol::this_state L, uint8_t index) {
		sol::state_view state(L);

		Game::Instance& game = state["Game"]["Instance"];
		return game.GetPlayerByIndex(index);
	}

	auto Game_Notify(sol::this_state L, sol::table serverEventTable) {
		sol::state_view state(L);

		Game::Instance& game = state["Game"]["Instance"];
		Game::ServerEvent serverEvent = LuaGetServerEvent(game.GetObjectManager(), serverEventTable);

		game.SendServerEvent(serverEvent);
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

	auto nObjectManager_CreateTriggerVolume(sol::this_state L, glm::vec3 position, float radius) {
		sol::state_view state(L);

		Game::Instance& game = state["Game"]["Instance"];

		auto object = game.GetObjectManager().CreateTrigger(position, radius);
		if (object) {
			game.SendObjectCreate(object);
		}

		return object;
	}

	auto nObjectManager_GetObjectsInRadius(sol::this_state L, glm::vec3 position, float radius, sol::table objectTypes) {
		sol::state_view state(L);

		Game::Instance& game = state["Game"]["Instance"];

		std::vector<Game::NounType> types;
		for (const auto& entry : objectTypes) {
			types.push_back(static_cast<Game::NounType>(entry.second.as<uint32_t>()));
		}

		auto objects = game.GetObjectManager().GetObjectsInRadius(position, radius, types);
		auto objectCount = static_cast<int>(objects.size());

		auto result = state.create_table(objectCount, 0);
		for (int i = 0; i < objectCount; ++i) {
			result[i + 1] = objects[i];
		}

		return result;
	}

	// Player
	auto Player_Notify(sol::this_state L, sol::object playerValue, sol::table serverEventTable) {
		sol::state_view state(L);

		Game::Instance& game = state["Game"]["Instance"];
		Game::PlayerPtr player = LuaGetPlayer(game, playerValue);
		if (player) {
			Game::ServerEvent serverEvent = LuaGetServerEvent(game.GetObjectManager(), serverEventTable);
			game.SendServerEvent(player, serverEvent);
		}
	}

	// Object
	auto Object_GetFacing(sol::this_state L, sol::object objectValue) {
		sol::state_view state(L);

		Game::Instance& game = state["Game"]["Instance"];
		const auto& objectManager = game.GetObjectManager();

		Game::ObjectPtr object = LuaGetObject(objectManager, objectValue);
		if (object) {
			return object->GetLocomotionData().mFacing;
		} else {
			return glm::zero<glm::vec3>();
		}
	}

	auto Object_GetFootprintRadius(sol::this_state L, sol::object objectValue) {
		sol::state_view state(L);

		Game::Instance& game = state["Game"]["Instance"];
		Game::ObjectPtr object = LuaGetObject(game.GetObjectManager(), objectValue);
		if (object) {
			return object->GetFootprintRadius();
		}

		return 0.f;
	}

	auto Object_Move(sol::this_state L, sol::object objectValue, glm::vec3 position, bool teleport) {
		sol::state_view state(L);

		Game::Instance& game = state["Game"]["Instance"];
		Game::ObjectPtr object = LuaGetObject(game.GetObjectManager(), objectValue);
		if (object) {
			RakNet::LocomotionData data {};
			data.SetGoalPosition(position);
			if (teleport) {
				data.mGoalFlags |= 0x020;
			}

			game.MoveObject(object, data);
		}
	}

	auto Object_SetAnimationState(sol::this_state L, sol::object objectValue, uint32_t animationState, sol::optional<bool> overlay) {
		sol::state_view state(L);

		Game::Instance& game = state["Game"]["Instance"];
		game.SendAnimationState(LuaGetObject(game.GetObjectManager(), objectValue), animationState, overlay.value_or(false));
	}

	auto Object_SetGraphicsState(sol::this_state L, sol::object objectValue, uint32_t graphicsState) {
		sol::state_view state(L);

		Game::Instance& game = state["Game"]["Instance"];
		game.SendObjectGfxState(LuaGetObject(game.GetObjectManager(), objectValue), graphicsState);
	}

	auto Object_GetAttributesSnapshot(sol::this_state L, sol::object objectValue) {
		sol::state_view state(L);

		Game::Instance& game = state["Game"]["Instance"];
		Game::ObjectPtr object = LuaGetObject(game.GetObjectManager(), objectValue);

		Game::AttributesPtr attributes;
		if (object && object->HasAttributeData()) {
			attributes = std::make_shared<Game::Attributes>(*object->GetAttributeData());
		}

		return attributes;
	}

	auto Object_GetWeaponDamage(sol::this_state L, sol::object objectValue) {
		sol::state_view state(L);

		Game::Instance& game = state["Game"]["Instance"];
		Game::ObjectPtr object = LuaGetObject(game.GetObjectManager(), objectValue);

		sol::table damageRange = state.create_table(2);
		if (object && object->HasAttributeData()) {
			const auto& weaponDamage = object->GetAttributeData()->GetWeaponDamage();
			damageRange[1] = std::get<0>(weaponDamage);
			damageRange[2] = std::get<1>(weaponDamage);
		}

		return damageRange;
	}

	auto Object_AddCooldown(sol::this_state L, sol::object objectValue, uint32_t abilityId, int64_t milliseconds) {
		sol::state_view state(L);

		Game::Instance& game = state["Game"]["Instance"];
		Game::ObjectPtr object = LuaGetObject(game.GetObjectManager(), objectValue);
		if (object) {
			game.SendCooldownUpdate(object, abilityId, milliseconds);
		}
	}

	auto Object_RemoveCooldown(sol::this_state L, sol::object objectValue, uint32_t abilityId, int64_t milliseconds) {
	}

	auto Object_ScaleCooldown(sol::this_state L, sol::object objectValue, uint32_t abilityId, float scale) {
		//
	}

	auto Object_TakeDamage(sol::this_state L,
		sol::object objectValue, sol::object attackerValue, sol::object attributesSnapshotValue,
		sol::table damageRange,
		Game::DamageType damageType, Game::DamageSource damageSource, float damageCoefficient,
		int32_t descriptors, float damageMultiplier, glm::vec3 direction
	) {
		sol::state_view state(L);

		Game::Instance& game = state["Game"]["Instance"];
		const auto& objectManager = game.GetObjectManager();

		Game::ObjectPtr object = LuaGetObject(objectManager, objectValue);
		if (object) {
			Game::ObjectPtr attacker = LuaGetObject(objectManager, attackerValue);
			Game::AttributesPtr attributes;
			if (attributesSnapshotValue.is<Game::AttributesPtr>()) {
				attributes = attributesSnapshotValue.as<Game::AttributesPtr>();
			}

			const auto& result = object->TakeDamage(
				attributes,
				std::make_tuple(damageRange.raw_get_or(1, 0.f), damageRange.raw_get_or(2, 0.f)),
				damageType, damageSource, damageCoefficient,
				descriptors, damageMultiplier, direction
			);

			Game::CombatEvent combatEvent;
			combatEvent.mTargetId = object->GetId();
			combatEvent.mDirection = direction;
			combatEvent.mDeltaHealth = std::get<1>(result);

			// I dunno, add some confusion effect maybe for this to be of any use
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

			Game::ServerEvent serverEvent;
			serverEvent.mServerEventDef = effect;
			serverEvent.mEffectIndex = index;
			serverEvent.mForceAttach = true;
			serverEvent.mObjectId = object->GetId();
			serverEvent.mTargetPoint = object->GetPosition();
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

		return static_cast<decltype(Game::ServerEvent::mEffectIndex)>(0);
	}

	auto Object_RemoveEffectIndex(sol::this_state L, sol::object objectValue, uint8_t index, sol::optional<bool> hardStop) {
		sol::state_view state(L);

		Game::Instance& game = state["Game"]["Instance"];
		const auto& objectManager = game.GetObjectManager();

		Game::ObjectPtr object = LuaGetObject(objectManager, objectValue);
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

	auto Object_DropLoot(sol::this_state L, sol::object objectValue, sol::object targetObjectValue, Game::DropType dropType) {
		sol::state_view state(L);

		Game::Instance& game = state["Game"]["Instance"];
		const auto& objectManager = game.GetObjectManager();

		Game::ObjectPtr object = LuaGetObject(objectManager, objectValue);
		Game::ObjectPtr targetObject = LuaGetObject(objectManager, targetObjectValue);
		if (object && targetObject) {
			game.DropLoot(object, targetObject, dropType);
		}
	}

	auto Object_GetTeleporterDestination(sol::this_state L, sol::object objectValue) {
		sol::state_view state(L);

		Game::Instance& game = state["Game"]["Instance"];
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
		sol::state_view state(L);

		Game::Instance& game = state["Game"]["Instance"];
		const auto& objectManager = game.GetObjectManager();

		Game::TriggerVolumePtr trigger = LuaGetTrigger(objectManager, triggerValue);
		Game::ObjectPtr object = LuaGetObject(objectManager, objectValue);
		if (trigger && object) {
			trigger->Attach(object);
		}
	}

	auto Trigger_Detach(sol::this_state L, sol::object triggerValue, sol::object objectValue) {
		sol::state_view state(L);

		Game::Instance& game = state["Game"]["Instance"];
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
	// LuaBase
	LuaBase::~LuaBase() {
		mState.collect_garbage();
		mState.collect_garbage();
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
	}

	void LuaBase::Reload() {
		// TODO:
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

	void LuaBase::RegisterFunctions() {
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
					failed++;
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
		mEventTable = mState.create_table();

		RegisterEnums();
		RegisterFunctions();

		LoadFile("data/lua/global.lua");
	}

	void Lua::Reload() {
		mEventTable = mState.create_table();
		LuaBase::Reload();
		GlobalLua::Instance().Reload();
	}

	void Lua::PreloadAbilities() {
		// TODO:
	}

	sol::table Lua::GetAbility(const std::string& abilityName) {
		return GetAbility(utils::hash_id(abilityName));
	}

	sol::table Lua::GetAbility(uint32_t abilityId) {
		sol::table ability = mEventTable.raw_get_or<sol::table>(abilityId, sol::nil);
		if (ability == sol::nil) {
			auto abilityCode = GlobalLua::Instance().GetAbility(abilityId);
			if (abilityCode.empty()) {
				return sol::nil;
			}

			mState.safe_script(abilityCode.as_string_view());

			ability = mEventTable.raw_get_or<sol::table>(abilityId, sol::nil);
			if (ability == sol::nil) {
				std::cout << "Did you forget to call RegisterAbility?" << std::endl;
			}
		}
		return ability;
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
			mEventTable.raw_set(utils::hash_id(abilityName), abilityTable);
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
			objectType["SetGraphicsState"] = &LuaFunction::Object_SetGraphicsState;

			objectType["GetAttributesSnapshot"] = &LuaFunction::Object_GetAttributesSnapshot;
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
			objectType["RemoveEffectIndex"] = &LuaFunction::Object_RemoveEffectIndex;

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
