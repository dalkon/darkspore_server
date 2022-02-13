
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
			0x000: char namespace[0x50];
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

	auto LuaGetAbility(sol::this_state L) {
		Game::Ability* ability = sol::state_view(L)["_ABILITY"];
		return ability;
	}

	auto LuaGetPlayer(const Game::Instance& instance, sol::object& value) {
		Game::PlayerPtr result;
		if (value.valid()) {
			result = value.as<Game::PlayerPtr>();
			if (!result) {
				result = instance.GetPlayerByIndex(value.as<uint8_t>());
			}
		}
		return result;
	}

	auto LuaGetObject(const Game::ObjectManager& objectManager, sol::object& value) {
		Game::ObjectPtr result;
		if (value.valid()) {
			result = value.as<Game::ObjectPtr>();
			if (!result) {
				result = objectManager.Get(value.as<uint32_t>());
			}
		}
		return result;
	}

	auto LuaGetTrigger(const Game::ObjectManager& objectManager, sol::object& value) {
		Game::TriggerVolumePtr result;
		if (value.valid()) {
			result = value.as<Game::TriggerVolumePtr>();
			if (!result) {
				result = objectManager.GetTrigger(value.as<uint32_t>());
			}
		}
		return result;
	}

	auto LuaGetServerEvent(const Game::ObjectManager& objectManager, const sol::table& value) {
		Game::ServerEvent serverEvent;
		serverEvent.mServerEventDef = value.get_or<uint32_t>("asset", 0);
		serverEvent.mObjectId = value.get_or<uint32_t>("objectId", 0);
		serverEvent.mSimpleSwarmEffectId = value.get_or<uint32_t>("simpleSwarmEffectId", 0);

		if (sol::object eventData = value["position"]; eventData.is<glm::vec3>()) { serverEvent.mPosition = eventData.as<glm::vec3>(); }
		if (sol::object eventData = value["facing"]; eventData.is<glm::vec3>()) { serverEvent.mFacing = eventData.as<glm::vec3>(); }
		if (sol::object eventData = value["targetPoint"]; eventData.is<glm::vec3>()) { serverEvent.mTargetPoint = eventData.as<glm::vec3>(); }

		const auto& object = objectManager.Get(serverEvent.mObjectId);
		if (object) {
			serverEvent.mEffectIndex = object->AddEffect(serverEvent.mServerEventDef);
			if (serverEvent.mEffectIndex != 0xFF) {
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

	auto LuaGetProjectileParameters(const sol::table& value) {
		Game::ProjectileParameters parameters;

		if (sol::object eventData = value["direction"]; eventData.is<glm::vec3>()) { parameters.mDirection = eventData.as<glm::vec3>(); }

		parameters.mJinkInfo = value.get_or<uint32_t>("jinkInfo", 0);

		parameters.mSpeed = value.get_or<float>("speed", 0);
		parameters.mAcceleration = value.get_or<float>("acceleration", 0);
		parameters.mRange = value.get_or<float>("range", 0);
		parameters.mSpinRate = value.get_or<float>("spinRate", 0);
		parameters.mHomingDelay = value.get_or<float>("homingDelay", 0);
		parameters.mTurnRate = value.get_or<float>("turnRate", 0);
		parameters.mTurnAcceleration = value.get_or<float>("turnAcceleration", 0);
		parameters.mEccentricity = value.get_or<float>("eccentricity", 0);
		parameters.mCombatantSweepHeight = value.get_or<float>("combatantSweepHeight", 0);

		parameters.mProjectileFlags = value.get_or<uint8_t>("projectileFlags", 0);

		parameters.mPiercing = value.get_or<bool>("piercing", 0);
		parameters.mIgnoreGroundCollide = value.get_or<bool>("ignoreGroundCollide", 0);
		parameters.mIgnoreCreatureCollide = value.get_or<bool>("ignoreCreatureCollide", 0);

		return parameters;
	}
}

// LuaFunction
namespace LuaFunction {
	// global functions
	auto AddEvent(sol::this_state L, sol::protected_function callback, uint32_t delay, sol::variadic_args args) {
		if (delay > 0) {
			auto& game = LuaGetGame(L);
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
	namespace nUtil {
		auto SPID(std::string value) {
			return utils::hash_id(value);
		}

		auto ToGUID(std::string value) {
			return utils::to_number<uint64_t>(value, 0);
		}
	}

	// nMathUtil
	namespace nMathUtil {
		auto TransformVector(const glm::vec3& v, const glm::quat& q) {
			return v * glm::eulerAngles(q);
		}

		auto RotateVectorByAxisAngle(const glm::vec3& v, const glm::vec3& axis, float angle) {
			return v * glm::angleAxis(angle, axis);
		}

		auto CircleIntersectsArc(const glm::vec3& position, float radius, float arcX, float arcY, float arcFacingX, float arcFacingY, float arcLength, float arcAngle) {
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
			auto value = glm::length(delta);
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
		}
	}

	// nThread
	namespace nThread {
		auto GetValue(sol::this_state L, std::string key) {
			auto& game = LuaGetGame(L);
			auto& lua = game.GetLua();

			auto thread = lua.GetThread(L);
			if (!thread) {
				return sol::make_object(L, sol::lua_nil);
			}

			return thread->get_value(key);
		}

		auto SetValue(sol::this_state L, std::string key, sol::object value) {
			auto& game = LuaGetGame(L);
			auto& lua = game.GetLua();

			auto thread = lua.GetThread(L);
			if (thread) {
				thread->set_value(key, std::move(value));
			}
		}

		auto Sleep(sol::this_state L, int32_t milliseconds) {
			if (milliseconds <= 0) {
				return;
			}

			auto& game = LuaGetGame(L);
			auto& lua = game.GetLua();

			auto thread = lua.GetThread(L);
			if (!thread) {
				return;
			}

			uint64_t time = utils::get_milliseconds() + milliseconds;
			thread->set_resume_condition([time](lua_State* L, sol::variadic_results& results) { return utils::get_milliseconds() >= time; });

			lua.YieldThread(thread);
		}

		auto WaitForHitpointsAbove(sol::this_state L, sol::object objectValue, float value, float deathTimer) {
			auto& game = LuaGetGame(L);
			auto& lua = game.GetLua();

			auto thread = lua.GetThread(L);
			if (!thread) {
				return;
			}

			auto& objectManager = game.GetObjectManager();
			auto object = LuaGetObject(objectManager, objectValue);
			if (object) {
				thread->set_resume_condition([&objectManager, objectId = object->GetId(), value](lua_State* L, sol::variadic_results& results) {
					auto object = objectManager.Get(objectId);
					if (!object) {
						return true;
					}

					// Check death timer thingy
					//

					return object->GetHealth() > value;
				});

				lua.YieldThread(thread);
			}
		}

		auto WaitForJumpComplete(sol::this_state L, sol::object objectValue) {
			auto& game = LuaGetGame(L);
			auto& lua = game.GetLua();

			auto thread = lua.GetThread(L);
			if (!thread) {
				return;
			}

			auto& objectManager = game.GetObjectManager();
			auto object = LuaGetObject(objectManager, objectValue);
			if (object) {
				/*
				thread->set_resume_condition([&objectManager, objectId = object->GetId(), value] {
					auto object = objectManager.Get(objectId);
					if (!object) {
						return true;
					}

					// Check death timer thingy
					//

					return object->GetHealth() > value;
				});

				lua.YieldThread(thread);
				*/
			}
		}

		auto WaitForXSeconds(sol::this_state L, double seconds, sol::object attributesSnapshotValue, sol::optional<bool> channeling) {
			if (seconds <= 0) {
				return;
			}

			auto& game = LuaGetGame(L);
			auto& lua = game.GetLua();

			auto thread = lua.GetThread(L);
			if (!thread) {
				return;
			}

			auto ability = LuaGetAbility(L);
			if (ability && attributesSnapshotValue.is<Game::AttributesPtr>()) {
				const auto descriptors = ability->GetDescriptors();
				const auto& attributes = attributesSnapshotValue.as<Game::AttributesPtr>();

				if (utils::enum_helper<Game::Descriptors>::test(descriptors, Game::Descriptors::IsBasic)) {
					seconds /= std::max(-0.9f, attributes->GetValue(Game::AttributeType::AttackSpeedScale)) + 1;
				}

				if (utils::enum_helper<Game::Descriptors>::test(descriptors, Game::Descriptors::IsChannel) && channeling.value_or(false)) {
					seconds *= 1.f - attributes->GetValue(Game::AttributeType::ChannelTimeDecrease);
					seconds = std::max(0.05, seconds);
				}
			}

			uint64_t time = utils::get_milliseconds() + static_cast<uint64_t>(seconds * 1000);
			thread->set_resume_condition([time](lua_State* L, sol::variadic_results& results) { return utils::get_milliseconds() >= time; });

			lua.YieldThread(thread);

			// Old code (it fucking sucks)
			/*
			game.AddTask(static_cast<uint32_t>(seconds * 1000), [&game, thread](uint32_t id) mutable {
				game.AddServerTask([&game, thread, id]() mutable {
					game.CancelTask(id);
					thread->call();
				});
			});
			*/
		}

		auto WaitUntilTime(sol::this_state L, double seconds, sol::object attributesSnapshotValue, sol::optional<bool> channeling) {
			if (seconds <= 0) {
				return;
			}

			auto& game = LuaGetGame(L);
			auto& lua = game.GetLua();

			auto thread = lua.GetThread(L);
			if (!thread) {
				return;
			}

			auto ability = LuaGetAbility(L);
			if (ability && attributesSnapshotValue.is<Game::AttributesPtr>()) {
				const auto descriptors = ability->GetDescriptors();
				const auto& attributes = attributesSnapshotValue.as<Game::AttributesPtr>();

				if (utils::enum_helper<Game::Descriptors>::test(descriptors, Game::Descriptors::IsBasic)) {
					seconds /= std::max(-0.9f, attributes->GetValue(Game::AttributeType::AttackSpeedScale)) + 1;
				}

				if (utils::enum_helper<Game::Descriptors>::test(descriptors, Game::Descriptors::IsChannel) && channeling.value_or(false)) {
					seconds *= 1.f - attributes->GetValue(Game::AttributeType::ChannelTimeDecrease);
					seconds = std::max(0.05, seconds);
				}

				if (seconds <= 0) {
					return;
				}
			}

			uint64_t time = utils::get_milliseconds() + static_cast<uint64_t>(seconds * 1000);
			thread->set_resume_condition([time](lua_State* L, sol::variadic_results& results) { return utils::get_milliseconds() >= time; });

			lua.YieldThread(thread);
		}

		auto WaitForProjectile(sol::this_state L, 
			sol::object projectileObjectValue,
			sol::table projectileParametersTable,
			sol::optional<sol::object> targetValue,
			sol::optional<sol::protected_function> callbackValue
		) {
			const auto CalculateExpectedGeoCollision = [=](const auto& locomotion, const auto& parameters) {
				locomotion->SetHasCollidedWithCreature(false);
				locomotion->SetHasCollided(false);
				locomotion->SetReflectedLastUpdate(1000);

				// *(undefined*)&object->locomotion->field_0x10 = 0;
				// *(undefined4*)&object->locomotion->field_0x14 = 0;

				auto something = parameters.mDirection * parameters.mRange;
				/*
				pLVar1 = object->locomotion;
				local_28.z = (pLVar1->projectile_parameters).range;
				local_28.x = local_28.z * (pLVar1->projectile_parameters).direction.x;
				local_28.y = (pLVar1->projectile_parameters).direction.y * local_28.z;
				local_28.z = (pLVar1->projectile_parameters).direction.z * local_28.z;
				bVar2 = FUN_00a33800(&local_1c, object, &local_28);
				*/
				const auto& object = locomotion->GetObject();
				if (object.IsColliding()) {
					locomotion->SetExpectedGeoCollision(object.GetPosition());
				}
			};

			const auto ProjectileFlagsOne = [=](const Game::ObjectPtr& projectile, const Game::ObjectPtr& target) {
				const auto& projectileLocomotion = projectile->GetLocomotionData();
				if (!projectileLocomotion) {
					return;
				}

				projectileLocomotion->SetHasCollidedWithCreature(false);
				projectileLocomotion->SetHasCollided(false);
				projectileLocomotion->SetReflectedLastUpdate(1000);

				uint32_t targetId;
				if (target) {
					targetId = target->GetId();
				} else {
					targetId = 0;
				}

				projectileLocomotion->SetTargetId(targetId);
				projectile->SetMovementType(Game::MovementType::HomingProjectile);

				auto& parameters = projectileLocomotion->GetProjectileParameters();
				if (parameters.mEccentricity > 0) {
					parameters.mJinkInfo = utils::random::get<uint8_t>(0, 0xFF) | std::clamp<uint8_t>(static_cast<uint8_t>(parameters.mEccentricity * 255.f), 0, 0xFF);
				}

				projectileLocomotion->SetInitialDirection(parameters.mDirection);
				if (target) {
					const auto& projectileCenter = projectile->GetCenterPoint();
					const auto& targetCenter = target->GetCenterPoint();

					auto delta = projectileCenter - targetCenter;
					auto deltaLength = glm::length(delta);

					projectile->SetLinearVelocity(glm::normalize(delta) * parameters.mSpeed);

					// What?
					float jinkValue = deltaLength * 0.75f;
					if (2.f < (deltaLength - jinkValue)) {
						jinkValue = deltaLength - 2.f;
					}

					parameters.mJinkInfo |= (static_cast<uint32_t>(jinkValue * 10.f) << 16);
				} else {
					auto newDirection = glm::normalize(glm::vec3(parameters.mDirection.x, parameters.mDirection.y, 0));
					parameters.mDirection = newDirection;

					projectile->SetLinearVelocity(newDirection * parameters.mSpeed);

					CalculateExpectedGeoCollision(projectileLocomotion, parameters);
					// *(undefined4*)&object->locomotion->field_0xd0 = 0x7f7fffff;
				}

				/*
				pLVar5 = object->locomotion;
				bVar4 = *(byte *)((int)&(pLVar5->projectile_parameters).jink_info + 2);
				uVar3 = *(undefined *)((int)&(pLVar5->projectile_parameters).jink_info + 1);
				*(undefined *)&pLVar5->field_0xc5 = *(undefined *)&(pLVar5->projectile_parameters).jink_info;
				*(undefined *)&pLVar5->field_0xc6 = uVar3;
				*(float *)&pLVar5->field_0xd0 = (float)(uint)bVar4 * 0.1;
				*/
			};

			auto& game = LuaGetGame(L);
			auto& lua = game.GetLua();

			auto thread = lua.GetThread(L);
			if (!thread) {
				return;
			}

			auto& objectManager = game.GetObjectManager();
			auto projectile = LuaGetObject(objectManager, projectileObjectValue);
			if (projectile && projectile->HasLocomotionData()) {
				const auto& locomotion = projectile->GetLocomotionData();

				auto& parameters = locomotion->GetProjectileParameters();
				parameters = LuaGetProjectileParameters(projectileParametersTable);

				const auto& owner = projectile->GetOwnerObject();
				if (owner) {
					parameters.mRange *= 1.f + owner->GetAttributeValue(Game::AttributeType::RangeIncrease);
					/*
					owner->GetAttributeValue(Game::AttributeType::ProjectileSpeedIncrease)
					projectile->GetAttributeValue(Game::AttributeType::MovementSpeedBuff)
					*/
				}

				projectile->SetLinearVelocity(parameters.mDirection * parameters.mSpeed);
				projectile->SetAngularVelocity(parameters.mDirection * parameters.mSpinRate);

				auto dot = glm::dot(parameters.mDirection, parameters.mDirection);
				if (dot < 0) {
					parameters.mDirection = projectile->GetLocomotionData()->GetFacing();
					dot = glm::length(parameters.mDirection);
				}
				
				/* dunno what this does
				if (dot > 0) {
					piVar7 = (int*)FUN_009c1050();
					pvVar6 = (vec3*)(**(code**)(*piVar7 + 0x10))(&local_1c, &projectile->position);
					pqVar8 = FUN_007b69f0(&qStack16, pvVar1, pvVar6);
					(projectile->orientation).x = pqVar8->x;
					(projectile->orientation).y = pqVar8->y;
					(projectile->orientation).z = pqVar8->z;
					(projectile->orientation).w = pqVar8->w;
				}
				*/

				if (parameters.mProjectileFlags & 1) {
					Game::ObjectPtr target;
					if (targetValue.has_value()) {
						target = LuaGetObject(objectManager, *targetValue);
					}
					ProjectileFlagsOne(projectile, target);
				} else {
					CalculateExpectedGeoCollision(locomotion, parameters);
				}

				thread->set_resume_condition([&objectManager, id = projectile->GetId(), position = projectile->GetPosition(), range = parameters.mRange](lua_State* L, sol::variadic_results& results) {
					const Game::ObjectPtr& object = objectManager.Get(id);
					if (!object) {
						results.push_back({ L, sol::in_place, Game::ObjectPtr() });
						results.push_back({ L, sol::in_place, true });
						results.push_back({ L, sol::in_place, true });
						results.push_back({ L, sol::in_place, glm::zero<glm::vec3>() });
						results.push_back({ L, sol::in_place, 0 });
						return true;
					}

					auto delta = position - object->GetPosition();
					auto distance = range - glm::length(delta);
					if (distance > 0) {
						const auto& locomotionData = object->GetLocomotionData();

						bool hasCollidedWithCreature = locomotionData->HasCollidedWithCreature();
						bool hasCollided = locomotionData->HasCollided();
						if (!hasCollidedWithCreature && !hasCollided) {
							return false;
						}

						results.push_back({ L, sol::in_place, Game::ObjectPtr() });
						results.push_back({ L, sol::in_place, hasCollidedWithCreature });
						results.push_back({ L, sol::in_place, hasCollided });
					} else {
						results.push_back({ L, sol::in_place, Game::ObjectPtr() });
						results.push_back({ L, sol::in_place, false });
						results.push_back({ L, sol::in_place, false });
					}

					results.push_back({ L, sol::in_place, object->GetPosition() });
					results.push_back({ L, sol::in_place, distance });
					return true;
				});

				lua.YieldThread(thread);
			}
		}

		auto Create(sol::this_state L, sol::protected_function callback, sol::variadic_args args) {
			// TODO: add so each object has a "thread"? (same as darkspore)
			LuaGetGame(L).GetLua().CallCoroutine(nullptr, callback, args);
		}
	}

	// Game
	auto Game_GetPlayer(sol::this_state L, uint8_t index) {
		return LuaGetGame(L).GetPlayerByIndex(index);
	}

	auto Game_Notify(sol::this_state L, sol::table serverEventTable) {
		auto& game = LuaGetGame(L);

		auto serverEvent = LuaGetServerEvent(game.GetObjectManager(), serverEventTable);
		game.SendServerEvent(serverEvent);
	}

	// nObjectManager
	auto nObjectManager_GetObject(sol::this_state L, uint32_t id) {
		auto& game = LuaGetGame(L);
		return game.GetObjectManager().Get(id);
	}

	auto nObjectManager_CreateObject(sol::this_state L, uint32_t noun, glm::vec3 position) {
		auto& game = LuaGetGame(L);

		auto object = game.GetObjectManager().Create(noun);
		if (object) {
			game.SendObjectUpdate(object); // create and update the object
			object->SetPosition(position);
		}

		return object;
	}

	auto nObjectManager_GetTrigger(sol::this_state L, uint32_t id) {
		auto& game = LuaGetGame(L);
		return game.GetObjectManager().GetTrigger(id);
	}

	auto nObjectManager_CreateTriggerVolume(sol::this_state L, glm::vec3 position, float radius) {
		auto& game = LuaGetGame(L);
		return game.GetObjectManager().CreateTrigger(position, radius);
	}

	auto nObjectManager_GetObjectsInRadius(sol::this_state L, glm::vec3 position, float radius, sol::table objectTypes) {
		auto& game = LuaGetGame(L);

		std::vector<Game::NounType> types;
		for (const auto& entry : objectTypes) {
			types.push_back(static_cast<Game::NounType>(entry.second.as<uint32_t>()));
		}

		auto objects = game.GetObjectManager().GetObjectsInRadius(Game::BoundingSphere(position, radius), types);
		auto objectCount = static_cast<int>(objects.size());

		auto result = sol::state_view(L).create_table(objectCount, 0);
		for (int i = 0; i < objectCount; ++i) {
			result[i + 1] = objects[i];
		}

		return result;
	}

	auto nObjectManager_GetObjectsInRadius_SortedByDistance(sol::this_state L, glm::vec3 position, float radius, sol::table objectTypes) {
		auto& game = LuaGetGame(L);

		std::vector<Game::NounType> types;
		for (const auto& entry : objectTypes) {
			types.push_back(static_cast<Game::NounType>(entry.second.as<uint32_t>()));
		}

		auto objects = game.GetObjectManager().GetObjectsInRadius(Game::BoundingSphere(position, radius), types);
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
	namespace Locomotion {
		auto On(sol::this_state L, sol::object objectValue, sol::variadic_args args) {
			auto& game = LuaGetGame(L);
			auto object = LuaGetObject(game.GetObjectManager(), objectValue);
			if (object && object->HasLocomotionData()) {

			}
		}

		auto TeleportAndFace(sol::this_state L, sol::object objectValue, glm::vec3 position, glm::vec3 facingPosition) {
			auto& game = LuaGetGame(L);
			auto object = LuaGetObject(game.GetObjectManager(), objectValue);
			if (object && object->HasLocomotionData()) {
				object->SetInputSyncStamp(object->GetInputSyncStamp() + 1);

				const auto& locomotion = object->GetLocomotionData();
				locomotion->Stop();

				locomotion->SetGoalPosition(position);
				locomotion->SetFacing(facingPosition);
				locomotion->SetGoalFlags(locomotion->GetGoalFlags() | 0x020);

				game.MoveObject(object, *locomotion);
			}
		}

		auto TeleportObject(sol::this_state L, sol::object objectValue, glm::vec3 position, sol::optional<bool> keepFacing) {
			auto& game = LuaGetGame(L);
			auto object = LuaGetObject(game.GetObjectManager(), objectValue);
			if (object && object->HasLocomotionData()) {
				object->SetInputSyncStamp(object->GetInputSyncStamp() + 1);

				const auto& locomotion = object->GetLocomotionData();
				locomotion->Stop();

				const auto& direction = position - object->GetPosition();
				object->SetPosition(position);

				if (keepFacing.value_or(false)) {
					locomotion->SetFacing(direction + object->GetPosition());
				}
			}
		}

		auto JumpInDirection(sol::this_state L, sol::object objectValue, glm::vec3 position, sol::optional<bool> keepFacing) {
			auto& game = LuaGetGame(L);
			auto object = LuaGetObject(game.GetObjectManager(), objectValue);
			if (object && object->HasLocomotionData()) {
				object->SetInputSyncStamp(object->GetInputSyncStamp() + 1);

				const auto& locomotion = object->GetLocomotionData();
				locomotion->Stop();

				const auto& direction = position - object->GetPosition();
				object->SetPosition(position);

				if (keepFacing.value_or(false)) {
					locomotion->SetFacing(direction + object->GetPosition());
				}
			}
		}

		auto SlideToPoint(sol::this_state L, sol::object objectValue, glm::vec3 position, float speed) {
			auto& game = LuaGetGame(L);
			auto object = LuaGetObject(game.GetObjectManager(), objectValue);
			if (object && object->HasLocomotionData()) {
				const auto& locomotion = object->GetLocomotionData();
				locomotion->SlideToPoint(position, speed);

				game.MoveObject(object, *locomotion);
			}
		}

		auto MoveToPointExact(sol::this_state L, sol::object objectValue, glm::vec3 position, sol::optional<bool> setSomeFlag) {
			auto& game = LuaGetGame(L);
			auto object = LuaGetObject(game.GetObjectManager(), objectValue);
			if (object && object->HasLocomotionData()) {
				const auto& locomotion = object->GetLocomotionData();
				locomotion->SetGoalPosition(position);

				if (setSomeFlag.value_or(false)) {
					locomotion->SetGoalFlags(locomotion->GetGoalFlags() | 0x800);
				}

				game.MoveObject(object, *locomotion);
			}
		}

		auto MoveToCircleEdge(sol::this_state L, sol::object objectValue, glm::vec3 position, float distance, sol::optional<bool> setSomeFlag) {
			auto& game = LuaGetGame(L);
			auto object = LuaGetObject(game.GetObjectManager(), objectValue);
			if (object && object->HasLocomotionData()) {
				const auto& locomotion = object->GetLocomotionData();
				locomotion->SetGoalPositionWithDistance(position, distance);

				if (setSomeFlag.value_or(false)) {
					locomotion->SetGoalFlags(locomotion->GetGoalFlags() | 0x800);
				}

				game.MoveObject(object, *locomotion);
			}
		}

		auto MoveToPointWithinRange(sol::this_state L, sol::object objectValue, glm::vec3 position, float distance, sol::optional<bool> setSomeFlag) {
			auto& game = LuaGetGame(L);
			auto& lua = game.GetLua();

			auto thread = lua.GetThread(L);
			if (!thread) {
				return;
			}

			auto& objectManager = game.GetObjectManager();
			auto object = LuaGetObject(objectManager, objectValue);
			if (object && object->HasLocomotionData()) {
				const auto& locomotion = object->GetLocomotionData();
				locomotion->SetGoalPositionWithDistance(position, distance);

				if (setSomeFlag.value_or(false)) {
					locomotion->SetGoalFlags(locomotion->GetGoalFlags() | 0x800);
				}

				game.MoveObject(object, *locomotion);

				thread->set_resume_condition([&objectManager, objectId = object->GetId(), position, distance](lua_State* L, sol::variadic_results& results) {
					auto object = objectManager.Get(objectId);
					if (!object) {
						return true;
					}

					auto delta = object->GetPosition() - position;
					if ((object->GetFootprintRadius() + distance) <= glm::length(delta)) {
						return false;
					}

					return true;
				});

				lua.YieldThread(thread);
			}
		}

		auto MoveToObject(sol::this_state L, sol::object objectValue, sol::object targetValue, sol::optional<float> distance, sol::optional<bool> setSomeFlag) {
			auto& game = LuaGetGame(L);
			auto& objectManager = game.GetObjectManager();

			auto object = LuaGetObject(objectManager, objectValue);
			if (object && object->HasLocomotionData()) {
				auto target = LuaGetObject(objectManager, targetValue);
				if (!target) {
					return;
				}

				const auto& locomotion = object->GetLocomotionData();
				locomotion->SetGoalObject(target, distance.value_or(0.f) + target->GetFootprintRadius(), false, false);

				if (setSomeFlag.value_or(false)) {
					locomotion->SetGoalFlags(locomotion->GetGoalFlags() | 0x800);
				}

				game.MoveObject(object, *locomotion);
			}
		}

		auto FollowAsPet(sol::this_state L, sol::object objectValue, sol::object targetValue, sol::optional<float> distance) {
			auto& game = LuaGetGame(L);
			auto& objectManager = game.GetObjectManager();

			auto object = LuaGetObject(objectManager, objectValue);
			if (object && object->HasLocomotionData()) {
				auto target = LuaGetObject(objectManager, targetValue);
				if (!target) {
					return;
				}

				const auto& locomotion = object->GetLocomotionData();
				locomotion->SetGoalObjectEx(target, distance.value_or(0.f) + target->GetFootprintRadius(), false);

				game.MoveObject(object, *locomotion);
			}
		}

		auto ApplyExternalVelocity(sol::this_state L, sol::object objectValue, glm::vec3 velocity) {
			auto& game = LuaGetGame(L);
			auto& objectManager = game.GetObjectManager();

			auto object = LuaGetObject(objectManager, objectValue);
			if (object && object->HasLocomotionData()) {
				const auto& locomotion = object->GetLocomotionData();
				locomotion->ApplyExternalVelocity(velocity);

				game.MoveObject(object, *locomotion);
			}
		}

		auto ClearExternalVelocity(sol::this_state L, sol::object objectValue) {
			auto& game = LuaGetGame(L);
			auto& objectManager = game.GetObjectManager();

			auto object = LuaGetObject(objectManager, objectValue);
			if (object && object->HasLocomotionData()) {
				const auto& locomotion = object->GetLocomotionData();
				locomotion->ClearExternalVelocity();

				game.MoveObject(object, *locomotion);
			}
		}

		auto ClearTargetObject(sol::this_state L, sol::object objectValue) {
			auto& game = LuaGetGame(L);
			auto& objectManager = game.GetObjectManager();

			auto object = LuaGetObject(objectManager, objectValue);
			if (object && object->HasLocomotionData()) {
				const auto& locomotion = object->GetLocomotionData();
				locomotion->ClearTargetObject();

				game.MoveObject(object, *locomotion);
			}
		}

		auto FaceObjectDuringMove(sol::this_state L, sol::object objectValue, sol::object targetValue) {
			auto& game = LuaGetGame(L);
			auto& objectManager = game.GetObjectManager();

			auto object = LuaGetObject(objectManager, objectValue);
			if (object && object->HasLocomotionData()) {
				auto target = LuaGetObject(objectManager, targetValue);
				if (!target) {
					return;
				}

				const auto& locomotion = object->GetLocomotionData();
				locomotion->FaceObjectDuringMove(target);

				game.MoveObject(object, *locomotion);
			}
		}

		auto TurnToFace(sol::this_state L, sol::object objectValue, glm::vec3 position, sol::optional<bool> setSomeFlag) {
			auto& game = LuaGetGame(L);
			auto& objectManager = game.GetObjectManager();

			auto object = LuaGetObject(objectManager, objectValue);
			if (object && object->HasLocomotionData()) {
				const auto& locomotion = object->GetLocomotionData();
				locomotion->SetFacing(position);

				if (setSomeFlag.value_or(false)) {
					locomotion->SetGoalFlags(locomotion->GetGoalFlags() | 0x800);
				}
			}
		}

		auto TurnToFaceTargetObject(sol::this_state L, sol::object objectValue, sol::object targetValue) {
			auto& game = LuaGetGame(L);
			auto& objectManager = game.GetObjectManager();

			auto object = LuaGetObject(objectManager, objectValue);
			if (object && object->HasLocomotionData()) {
				auto target = LuaGetObject(objectManager, targetValue);
				if (!target) {
					return;
				}

				const auto& locomotion = object->GetLocomotionData();
				locomotion->TurnToFaceTargetObject(target);
			}
		}

		auto MoveToPointWhileFacingTarget(sol::this_state L, sol::object objectValue, glm::vec3 position, sol::object targetValue) {
			auto& game = LuaGetGame(L);
			auto& objectManager = game.GetObjectManager();

			auto object = LuaGetObject(objectManager, objectValue);
			if (object && object->HasLocomotionData()) {
				auto target = LuaGetObject(objectManager, targetValue);
				if (!target) {
					return;
				}

				const auto& locomotion = object->GetLocomotionData();
				locomotion->MoveToPointWhileFacingTarget(position, target);

				game.MoveObject(object, *locomotion);
			}
		}

		auto Stop(sol::this_state L, sol::object objectValue) {
			auto& game = LuaGetGame(L);
			auto object = LuaGetObject(game.GetObjectManager(), objectValue);
			if (object && object->HasLocomotionData()) {
				const auto& locomotion = object->GetLocomotionData();
				locomotion->Stop();

				game.MoveObject(object, *locomotion);
			}
		}

		auto GetClosestPosition(sol::this_state L, glm::vec3 position) {
			// TODO: add Locomotion::GetClosestPosition (not sure how the fuck I'm gonna do that without getting navmesh data)
			return position;
		}

		auto GetClosestPositionFromPoint(sol::this_state L, glm::vec3 position) {
			// TODO: add Locomotion::GetClosestPositionFromPoint (not sure how the fuck I'm gonna do that without getting navmesh data)
			return position;
		}

		auto GetTargetPosition(sol::this_state L, sol::object objectValue) {
			auto& game = LuaGetGame(L);
			auto object = LuaGetObject(game.GetObjectManager(), objectValue);
			if (object && object->HasLocomotionData()) {
				const auto& locomotion = object->GetLocomotionData();
				return locomotion->GetTargetPosition();
			}

			return glm::zero<glm::vec3>();
		}

		auto SetTargetPosition(sol::this_state L, sol::object objectValue, glm::vec3 position) {
			auto& game = LuaGetGame(L);
			auto object = LuaGetObject(game.GetObjectManager(), objectValue);
			if (object && object->HasLocomotionData()) {
				const auto& locomotion = object->GetLocomotionData();
				locomotion->SetTargetPosition(position);
			}
		}
	}

	// Player
	auto Player_Notify(sol::this_state L, sol::object playerValue, sol::table serverEventTable) {
		auto& game = LuaGetGame(L);
		auto player = LuaGetPlayer(game, playerValue);
		if (player) {
			auto serverEvent = LuaGetServerEvent(game.GetObjectManager(), serverEventTable);
			game.SendServerEvent(player, serverEvent);
		}
	}

	// Object
	namespace Object {
		auto IsValid(sol::this_state L, sol::object objectValue) {
			// object:IsValid() could be useful later.
			if (objectValue.is<Game::ObjectPtr>()) {
				return objectValue.as<Game::ObjectPtr>() != nullptr;
			}
			return false;
		}

		auto GetName(sol::this_state L, sol::object objectValue) {
			std::string result;

			auto& game = LuaGetGame(L);
			auto object = LuaGetObject(game.GetObjectManager(), objectValue);
			if (object) {
				const auto& noun = object->GetNoun();
				if (noun) {
					result = noun->GetName();
				}
			}

			return result;
		}

		auto GetPrivateTable(sol::this_state L, sol::object objectValue) {
			auto& game = LuaGetGame(L);
			auto object = LuaGetObject(game.GetObjectManager(), objectValue);
			if (object) {
				return game.GetLua().CreatePrivateTable(object->GetId());
			}
			return sol::table(sol::nil);
		}

		auto GetFacing(sol::this_state L, sol::object objectValue) {
			auto& game = LuaGetGame(L);
			auto object = LuaGetObject(game.GetObjectManager(), objectValue);
			if (object && object->HasLocomotionData()) {
				return object->GetLocomotionData()->GetFacing();
			}
			return glm::zero<glm::vec3>();
		}

		auto GetFootprintRadius(sol::this_state L, sol::object objectValue) {
			auto& game = LuaGetGame(L);
			auto object = LuaGetObject(game.GetObjectManager(), objectValue);
			if (object) {
				return object->GetFootprintRadius();
			}
			return 0.f;
		}

		auto Move(sol::this_state L, sol::object objectValue, glm::vec3 position, bool teleport) {
			auto& game = LuaGetGame(L);
			auto object = LuaGetObject(game.GetObjectManager(), objectValue);
			if (object && object->HasLocomotionData()) {
				const auto& locomotionData = object->GetLocomotionData();
				locomotionData->SetGoalPosition(position);
				if (teleport) {
					locomotionData->SetGoalFlags(locomotionData->GetGoalFlags() | 0x020);
				}
				game.MoveObject(object, *locomotionData);
			}
		}

		auto PlayAnimationSequence(sol::this_state L, sol::object objectValue, sol::table sequenceTable) {
			auto ability = LuaGetAbility(L);
			if (!ability) {
				std::cout << utils::timestamp << "[Lua error]: You may not call " << utils::yellow << "Object:PlayAnimationSequence" << utils::normal << " outside of an ability." << std::endl;
				return;
			}

			auto& game = LuaGetGame(L);
			auto object = LuaGetObject(game.GetObjectManager(), objectValue);
			if (object) {

			}
		}

		auto SetAnimationState(sol::this_state L, sol::object objectValue, uint32_t animationState, sol::optional<bool> overlay, sol::optional<float> scale) {
			if (animationState == 0) {
				return;
			}

			auto& game = LuaGetGame(L);
			game.SendAnimationState(LuaGetObject(game.GetObjectManager(), objectValue), animationState, overlay.value_or(false), scale.value_or(1.f));
		}

		auto SetAnimationStateToDeath(sol::this_state L, sol::object objectValue) {
			// TODO: 
		}

		auto SetAnimationStateToAggro(sol::this_state L, sol::object objectValue, sol::optional<bool> subsequent) {
			float duration = 0;

			auto& game = LuaGetGame(L);
			auto object = LuaGetObject(game.GetObjectManager(), objectValue);
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

		auto SetGraphicsState(sol::this_state L, sol::object objectValue, uint32_t graphicsState) {
			if (graphicsState == 0) {
				return;
			}

			auto& game = LuaGetGame(L);
			game.SendObjectGfxState(LuaGetObject(game.GetObjectManager(), objectValue), graphicsState);
		}

		auto GetAttributeSnapshot(sol::this_state L, sol::object objectValue) {
			const auto& game = LuaGetGame(L);
			const auto& object = LuaGetObject(game.GetObjectManager(), objectValue);

			Game::AttributesPtr attributes;
			if (object && object->HasAttributeData()) {
				attributes = std::make_shared<Game::Attributes>(*object->GetAttributeData());
				attributes->SetOwnerObject(object);
			}

			return attributes;
		}

		auto SetAttributeSnapshot(sol::this_state L, sol::object objectValue, sol::object attributeSnapshotValue) {
			auto& game = LuaGetGame(L);

			const auto& object = LuaGetObject(game.GetObjectManager(), objectValue);
			if (object) {
				// attributes = std::make_shared<Game::Attributes>(*object->GetAttributeData());
				// attributes->SetOwnerObject(object);
			}
		}

		auto GetWeaponDamage(sol::this_state L, sol::object objectValue) {
			auto& game = LuaGetGame(L);
			auto object = LuaGetObject(game.GetObjectManager(), objectValue);

			sol::table damageRange = sol::state_view(L).create_table(2);
			if (object && object->HasAttributeData()) {
				const auto& weaponDamage = object->GetAttributeData()->GetWeaponDamage();
				damageRange[1] = std::get<0>(weaponDamage);
				damageRange[2] = std::get<1>(weaponDamage);
			}

			return damageRange;
		}

		auto IsCombatant(sol::this_state L, sol::object objectValue) {
			auto& game = LuaGetGame(L);
			auto object = LuaGetObject(game.GetObjectManager(), objectValue);
			if (object) {
				return object->HasCombatantData();
			}
			return false;
		}

		auto AddCooldown(sol::this_state L, sol::object objectValue, uint32_t abilityId, uint32_t milliseconds) {
			auto& game = LuaGetGame(L);
			auto object = LuaGetObject(game.GetObjectManager(), objectValue);
			if (object) {
				const auto& [end, duration] = object->AddCooldown(abilityId, milliseconds);
				game.SendCooldownUpdate(object, abilityId, end - duration, duration);
			}
		}

		auto RemoveCooldown(sol::this_state L, sol::object objectValue, uint32_t abilityId, uint32_t milliseconds) {
			auto& game = LuaGetGame(L);
			auto object = LuaGetObject(game.GetObjectManager(), objectValue);
			if (object) {
				const auto& [end, duration] = object->RemoveCooldown(abilityId, milliseconds);
				game.SendCooldownUpdate(object, abilityId, end - duration, duration);
			}
		}

		auto ScaleCooldown(sol::this_state L, sol::object objectValue, uint32_t abilityId, float scale) {
			//
		}

		auto TakeDamage(sol::this_state L,
			sol::object objectValue,
			sol::object attackerValue,
			sol::object attributeSnapshotValue,
			sol::table damageRange,
			Game::DamageType damageType,
			Game::DamageSource damageSource,
			float damageCoefficient,
			Game::Descriptors descriptors,
			sol::optional<float> damageMultiplier,
			sol::optional<glm::vec3> direction
		) {
			auto& game = LuaGetGame(L);
			const auto& objectManager = game.GetObjectManager();

			auto object = LuaGetObject(objectManager, objectValue);
			if (object) {
				auto attacker = LuaGetObject(objectManager, attackerValue);

				Game::AttributesPtr attributes;
				if (attributeSnapshotValue.is<Game::AttributesPtr>()) {
					attributes = attributeSnapshotValue.as<Game::AttributesPtr>();
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

		auto Heal(sol::this_state L,
			sol::object objectValue,
			sol::object attackerValue,
			sol::object attributeSnapshotValue,
			sol::table healRange,
			float healCoefficient,
			sol::optional<Game::Descriptors> descriptors,
			sol::optional<bool> critical,
			sol::optional<bool> canResurrect
		) {
			auto& game = LuaGetGame(L);
			const auto& objectManager = game.GetObjectManager();

			auto object = LuaGetObject(objectManager, objectValue);
			if (object) {
				auto attacker = LuaGetObject(objectManager, attackerValue);

				Game::AttributesPtr attributes;
				if (attributeSnapshotValue.is<Game::AttributesPtr>()) {
					attributes = attributeSnapshotValue.as<Game::AttributesPtr>();
				}

				Game::CombatEvent combatEvent;
				combatEvent.mTargetId = object->GetId();

				const auto& result = object->Heal(
					attributes,
					std::make_tuple(healRange.raw_get_or(1, 0.f), healRange.raw_get_or(2, 0.f)),
					healCoefficient,
					descriptors.value_or(Game::Descriptors::IsBasic),
					critical.value_or(false),
					false
				);

				combatEvent.mDeltaHealth = std::get<0>(result);
				combatEvent.mIntegerHpChange = static_cast<int32_t>(combatEvent.mDeltaHealth);

				if (std::get<1>(result)) {
					combatEvent.mFlags = Game::CombatEventFlags::Critical;
				}

				game.SendCombatEvent(combatEvent);
				return result;
			}

			return std::make_tuple(0.f, false);
		}

		auto RequestAbility(sol::this_state L, sol::object objectValue, sol::object attackerValue, uint32_t abilityId, int32_t rank) {
			// darkspore original = object.request_ability(ability, object, target, position, ...)
		}

		auto RequestModifier(sol::this_state L, sol::object objectValue, sol::object attackerValue, uint32_t abilityId, int32_t rank) {

		}

		auto AddEffect(sol::this_state L, sol::object objectValue, sol::object serverEventDef, sol::optional<sol::object> secondaryObjectValue, sol::optional<glm::vec3> targetPointValue) {
			auto& game = LuaGetGame(L);
			const auto& objectManager = game.GetObjectManager();

			auto object = LuaGetObject(objectManager, objectValue);
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
					auto secondaryObject = LuaGetObject(objectManager, *secondaryObjectValue);
					if (secondaryObject) {
						serverEvent.mSecondaryObjectId = secondaryObject->GetId();
					}
				}

				game.SendServerEvent(serverEvent);
				return serverEvent.mEffectIndex;
			}

			return static_cast<uint8_t>(0);
		}

		auto RemoveEffect(sol::this_state L, sol::object objectValue, sol::object effectValue, sol::optional<bool> hardStop) {
			uint32_t effect;
			if (effectValue.is<uint32_t>()) {
				effect = effectValue.as<uint32_t>();
			} else if (effectValue.is<std::string>()) {
				effect = utils::hash_id(effectValue.as<std::string>());
			} else {
				return;
			}

			auto& game = LuaGetGame(L);
			auto object = LuaGetObject(game.GetObjectManager(), objectValue);
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

		auto RemoveEffectIndex(sol::this_state L, sol::object objectValue, uint8_t index, sol::optional<bool> hardStop) {
			if (index == 0) {
				return;
			}

			auto& game = LuaGetGame(L);
			auto object = LuaGetObject(game.GetObjectManager(), objectValue);
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

		auto UseAbility(sol::this_state L, sol::object objectValue, uint32_t abilityId, sol::object targetObjectValue, glm::vec3 targetPosition) {
			auto& game = LuaGetGame(L);

			const auto& ability = game.GetLua().GetAbility(abilityId);
			if (ability) {
				const auto& objectManager = game.GetObjectManager();

				auto object = LuaGetObject(objectManager, objectValue);
				if (object) {
					auto targetObject = LuaGetObject(objectManager, targetObjectValue);
					ability->Tick(object, targetObject, targetPosition);
				}
			}
		}

		auto DropLoot(sol::this_state L, sol::object objectValue, sol::object targetObjectValue, Game::DropType dropType) {
			auto& game = LuaGetGame(L);
			const auto& objectManager = game.GetObjectManager();

			auto object = LuaGetObject(objectManager, objectValue);
			auto targetObject = LuaGetObject(objectManager, targetObjectValue);
			if (object && targetObject) {
				game.DropLoot(object, targetObject, dropType);
			}
		}

		auto GetSharedAbilityOffset(sol::this_state L, sol::object objectValue) {
			auto& game = LuaGetGame(L);
			auto object = LuaGetObject(game.GetObjectManager(), objectValue);

			glm::vec3 offset {};
			if (object) {
				const auto& noun = object->GetNoun();
				if (noun) {
					const auto& playerClassData = noun->GetPlayerClassData();
					if (playerClassData) {
						offset = playerClassData->GetSharedAbilityOffset();
					}
				}
			}

			return offset;
		}

		auto GetTeleporterDestination(sol::this_state L, sol::object objectValue) {
			auto& game = LuaGetGame(L);
			auto object = LuaGetObject(game.GetObjectManager(), objectValue);

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

		auto IsStealthed(sol::this_state L, sol::object objectValue) {
			auto& game = LuaGetGame(L);
			auto object = LuaGetObject(game.GetObjectManager(), objectValue);
			if (object) {
				return object->GetStealthType() != Game::StealthType::None;
			}
			return false;
		}
	}

	// Trigger
	auto Trigger_Attach(sol::this_state L, sol::object triggerValue, sol::object objectValue) {
		auto& game = LuaGetGame(L);
		const auto& objectManager = game.GetObjectManager();

		auto trigger = LuaGetTrigger(objectManager, triggerValue);
		auto object = LuaGetObject(objectManager, objectValue);
		if (trigger && object) {
			trigger->Attach(object);
		}
	}

	auto Trigger_Detach(sol::this_state L, sol::object triggerValue, sol::object objectValue) {
		auto& game = LuaGetGame(L);
		const auto& objectManager = game.GetObjectManager();

		auto trigger = LuaGetTrigger(objectManager, triggerValue);
		auto object = LuaGetObject(objectManager, objectValue);
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
	// LuaThread
	LuaThread::LuaThread(Lua& lua) : mLua(lua) {
		mThread = sol::thread::create(lua.GetState());
	}

	sol::object LuaThread::get_value(const std::string& key) const {
		if (auto it = mValues.find(key); it != mValues.end()) {
			return it->second;
		}
		return sol::make_object(lua_state(), sol::lua_nil);
	}

	void LuaThread::set_value(const std::string& key, sol::object&& value) {
		mValues[key] = value;
	}

	bool LuaThread::resume() {
		if (!mCoroutine.valid()) {
			return true;
		}

		const auto call_status = status();
		if (call_status != sol::call_status::ok && call_status != sol::call_status::yielded) {
			return false;
		}

		sol::variadic_results results;
		if (mResumeCondition && !mResumeCondition(lua_state(), results)) {
			return false;
		}

		mResumeCondition = nullptr;
		mCoroutine(sol::as_args(results));

		return mCoroutine.status() == sol::call_status::ok;
	}

	void LuaThread::set_resume_condition(const ResumeCondition& condition) { mResumeCondition = condition; }
	void LuaThread::set_resume_condition(ResumeCondition&& condition) { mResumeCondition = std::move(condition); }

	void LuaThread::post_call() {
		if (mCoroutine.status() == sol::call_status::ok) {
			mLua.ReturnThreadToPool(this);
		}
	}

	// LuaBase
	LuaBase::~LuaBase() {
		CollectGarbage();
	}

	void LuaBase::Initialize() {
		mState.open_libraries();
		sol::set_default_state(mState);

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

				sol::meta_function::to_string, [](const glm::vec3& v) { return std::format("vec3<{}, {}, {}>", v.x, v.y, v.z); },
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

			mState.new_usertype<glm::quat>("quat",
				sol::constructors<glm::quat(), glm::quat(float, float, float, float)>(),

				sol::meta_function::to_string, [](const glm::quat& q) { return std::format("quat<{}, {}, {}, {}>", q.x, q.y, q.z, q.w); },

				"x", &glm::quat::x,
				"y", &glm::quat::y,
				"z", &glm::quat::z,
				"w", &glm::quat::w
			);
		}

		// nUtil
		auto nUtilTable = mState.create_named_table("nUtil");
		nUtilTable["SPID"] = &LuaFunction::nUtil::SPID;
		nUtilTable["ToGUID"] = &LuaFunction::nUtil::ToGUID;

		// nMathUtil
		auto nMathUtilTable = mState.create_named_table("nMathUtil");
		nMathUtilTable["TransformVector"] = &LuaFunction::nMathUtil::TransformVector;
		nMathUtilTable["RotateVectorByAxisAngle"] = &LuaFunction::nMathUtil::RotateVectorByAxisAngle;
		nMathUtilTable["CircleIntersectsArc"] = &LuaFunction::nMathUtil::CircleIntersectsArc;
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

					sol::error err = result;
					std::string what = err.what();

					std::cout << "GlobalLua: Could not load " << path << "." << std::endl;
					std::cout << err.what() << std::endl << std::endl;
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

	void Lua::Update() {
		for (auto it = mYieldedThreads.begin(); it != mYieldedThreads.end(); ) {
			auto thread = *it;
			if (thread->resume()) {
				it = mYieldedThreads.erase(it);
				ReturnThreadToPool(thread);
			} else {
				++it;
			}
		}
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

	LuaThread* Lua::SpawnThread() {
		LuaThread* thread;
		if (mThreadPool.empty()) {
			thread = new LuaThread(*this);
			mThreads.emplace(thread->lua_state(), thread);
		} else {
			thread = mThreadPool.back();
			mThreadPool.pop_back();
		}
		return thread;
	}

	LuaThread* Lua::GetThread(lua_State* L) const {
		if (auto it = mThreads.find(L); it != mThreads.end()) {
			return it->second;
		}
		return nullptr;
	}

	void Lua::YieldThread(LuaThread* thread) {
		if (thread) {
			mYieldedThreads.insert(thread);
		}
	}

	void Lua::ReturnThreadToPool(LuaThread* thread) {
		constexpr size_t MAX_LUA_THREADS = 256;
		if (mThreads.size() > MAX_LUA_THREADS) {
			mThreads.erase(mThreads.find(thread->lua_state()));
			delete thread;
		} else {
			thread->mValues.clear(); // Remove any stored values.
			mThreadPool.push_back(thread);
		}
		// CollectGarbage();
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

		// Descriptor
		mState.new_enum<Descriptors>("Descriptors", {
			{ "IsMelee", Descriptors::IsMelee },
			{ "IsBasic", Descriptors::IsBasic },
			{ "IsDoT", Descriptors::IsDoT },
			{ "IsAoE", Descriptors::IsAoE },
			{ "IsBuff", Descriptors::IsBuff },
			{ "IsDebuff", Descriptors::IsDebuff },
			{ "IsPhysicalDamage", Descriptors::IsPhysicalDamage },
			{ "IsEnergyDamage", Descriptors::IsEnergyDamage },
			{ "IsCosmetic", Descriptors::IsCosmetic },
			{ "IsHaste", Descriptors::IsHaste },
			{ "IsChannel", Descriptors::IsChannel },
			{ "HitReactNone", Descriptors::HitReactNone },
			{ "IsHoT", Descriptors::IsHoT },
			{ "IsProjectile", Descriptors::IsProjectile },
			{ "IgnorePlayerCount", Descriptors::IgnorePlayerCount },
			{ "IgnoreDifficulty", Descriptors::IgnoreDifficulty },
			{ "IsSelfResurrect", Descriptors::IsSelfResurrect },
			{ "IsInteract", Descriptors::IsInteract },
			{ "IsThorns", Descriptors::IsThorns }
		});

		// NpcType
		mState.new_enum<NpcType>("NpcType", {
			{ "Invalid", NpcType::Invalid },
			{ "Minion", NpcType::Minion },
			{ "Special", NpcType::Special },
			{ "Boss", NpcType::Boss },
			{ "Destructible", NpcType::Destructible },
			{ "Interactable", NpcType::Interactable },
			{ "Agent", NpcType::Agent },
			{ "Victim", NpcType::Victim },
			{ "Captain", NpcType::Captain }
		});
	}

	void Lua::RegisterFunctions() {
		// global functions
		mState["AddEvent"] = &LuaFunction::AddEvent;
		mState["StopEvent"] = &LuaFunction::StopEvent;
		mState["RegisterAbility"] = [this](std::string abilityName, sol::table abilityTable) {
			// TODO: validate ability
			if (abilityTable == sol::nil) {
				return;
			}

			auto id = utils::hash_id(abilityName);
			if (auto [it, inserted] = mAbilities.try_emplace(id); inserted) {
				abilityTable["id"] = id;
				it->second = std::make_shared<Ability>(*this, std::move(abilityTable));
			}
		};

		// nThread
		{
			auto nThreadTable = mState.create_named_table("nThread");
			nThreadTable["GetValue"] = &LuaFunction::nThread::GetValue;
			nThreadTable["SetValue"] = &LuaFunction::nThread::SetValue;

			nThreadTable["Sleep"] = sol::yielding(&LuaFunction::nThread::Sleep);
			nThreadTable["WaitForHitpointsAbove"] = sol::yielding(&LuaFunction::nThread::WaitForHitpointsAbove);
			nThreadTable["WaitForXSeconds"] = sol::yielding(&LuaFunction::nThread::WaitForXSeconds);
			nThreadTable["WaitUntilTime"] = sol::yielding(&LuaFunction::nThread::WaitUntilTime);
			nThreadTable["WaitForProjectile"] = sol::yielding(&LuaFunction::nThread::WaitForProjectile);

			nThreadTable["Create"] = &LuaFunction::nThread::Create;
		}

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

			// event functions
			locomotionTable["On"] = &LuaFunction::Locomotion::On;

			// object functions (object as arg0)
			locomotionTable["TeleportAndFace"] = &LuaFunction::Locomotion::TeleportAndFace;
			locomotionTable["TeleportObject"] = &LuaFunction::Locomotion::TeleportObject;
			locomotionTable["JumpInDirection"] = &LuaFunction::Locomotion::JumpInDirection;
			locomotionTable["SlideToPoint"] = &LuaFunction::Locomotion::SlideToPoint;
			locomotionTable["MoveToPointExact"] = &LuaFunction::Locomotion::MoveToPointExact;
			locomotionTable["MoveToCircleEdge"] = &LuaFunction::Locomotion::MoveToCircleEdge;
			locomotionTable["MoveToPointWithinRange"] = &LuaFunction::Locomotion::MoveToPointWithinRange;
			locomotionTable["MoveToObject"] = &LuaFunction::Locomotion::MoveToObject;
			locomotionTable["FollowAsPet"] = &LuaFunction::Locomotion::FollowAsPet;

			locomotionTable["ApplyExternalVelocity"] = &LuaFunction::Locomotion::ApplyExternalVelocity;
			locomotionTable["ClearExternalVelocity"] = &LuaFunction::Locomotion::ClearExternalVelocity;
			locomotionTable["ClearTargetObject"] = &LuaFunction::Locomotion::ClearTargetObject;

			locomotionTable["FaceObjectDuringMove"] = &LuaFunction::Locomotion::FaceObjectDuringMove;
			locomotionTable["TurnToFace"] = &LuaFunction::Locomotion::TurnToFace;
			locomotionTable["TurnToFaceTargetObject"] = &LuaFunction::Locomotion::TurnToFaceTargetObject;
			locomotionTable["MoveToPointWhileFacingTarget"] = &LuaFunction::Locomotion::MoveToPointWhileFacingTarget;
			
			locomotionTable["Stop"] = &LuaFunction::Locomotion::Stop;

			// position functions (vec3 as arg0)
			locomotionTable["GetClosestPosition"] = &LuaFunction::Locomotion::GetClosestPosition;
			locomotionTable["GetClosestPositionFromPoint"] = &LuaFunction::Locomotion::GetClosestPositionFromPoint;
			
			locomotionTable["GetTargetPosition"] = &LuaFunction::Locomotion::GetTargetPosition;
			locomotionTable["SetTargetPosition"] = &LuaFunction::Locomotion::SetTargetPosition;
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

			attributesType["GetOwnerObject"] = &Game::Attributes::GetOwnerObject;
		}

		// Object
		{
			auto objectType = mState.new_usertype<Object>("Object", sol::no_constructor);
			objectType["IsValid"] = &LuaFunction::Object::IsValid;

			objectType["GetId"] = &Object::GetId;
			objectType["GetName"] = &LuaFunction::Object::GetName;
			objectType["GetPrivateTable"] = &LuaFunction::Object::GetPrivateTable;

			objectType["GetPosition"] = &Object::GetPosition;
			objectType["SetPosition"] = &Object::SetPosition;

			objectType["GetOrientation"] = &Object::GetOrientation;
			objectType["SetOrientation"] = &Object::SetOrientation;

			objectType["GetExtent"] = &Object::GetExtent;
			objectType["SetExtent"] = &Object::SetExtent;

			objectType["GetType"] = &Object::GetType;
			objectType["GetNpcType"] = &Object::GetNpcType;

			objectType["GetFacing"] = &LuaFunction::Object::GetFacing;
			objectType["GetFootprintRadius"] = &Object::GetFootprintRadius;
			objectType["GetCenterPoint"] = &Object::GetCenterPoint;

			objectType["SetTickOverride"] = sol::resolve<void(sol::protected_function)>(&Object::SetTickOverride);

			objectType["Move"] = &LuaFunction::Object::Move;

			objectType["GetHealth"] = &Object::GetHealth;
			objectType["GetMaxHealth"] = &Object::GetMaxHealth;
			objectType["SetHealth"] = &Object::SetHealth;

			objectType["GetMana"] = &Object::GetMana;
			objectType["GetMaxMana"] = &Object::GetMaxMana;
			objectType["SetMana"] = &Object::SetMana;

			objectType["GetOwnerObject"] = &Object::GetOwnerObject;
			objectType["SetOwnerObject"] = &Object::SetOwnerObject;

			objectType["PlayAnimationSequence"] = &LuaFunction::Object::PlayAnimationSequence;
			objectType["SetAnimationState"] = &LuaFunction::Object::SetAnimationState;
			objectType["SetAnimationStateToDeath"] = &LuaFunction::Object::SetAnimationStateToDeath;
			objectType["SetAnimationStateToAggro"] = &LuaFunction::Object::SetAnimationStateToAggro;
			objectType["SetGraphicsState"] = &LuaFunction::Object::SetGraphicsState;

			objectType["GetAttributeSnapshot"] = &LuaFunction::Object::GetAttributeSnapshot;
			objectType["SetAttributeSnapshot"] = &LuaFunction::Object::SetAttributeSnapshot;

			objectType["GetAttributeValue"] = sol::resolve<float(uint8_t) const>(&Object::GetAttributeValue);
			objectType["SetAttributeValue"] = sol::resolve<void(uint8_t, float)>(&Object::SetAttributeValue);

			objectType["GetWeaponDamage"] = &LuaFunction::Object::GetWeaponDamage;

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

			objectType["IsCombatant"] = &LuaFunction::Object::IsCombatant;

			objectType["AddCooldown"] = &LuaFunction::Object::AddCooldown;
			objectType["RemoveCooldown"] = &LuaFunction::Object::RemoveCooldown;
			objectType["ScaleCooldown"] = &LuaFunction::Object::ScaleCooldown;

			objectType["TakeDamage"] = &LuaFunction::Object::TakeDamage;
			objectType["Heal"] = &LuaFunction::Object::Heal;
			objectType["RequestAbility"] = &LuaFunction::Object::RequestAbility;
			objectType["RequestModifier"] = &LuaFunction::Object::RequestModifier;

			objectType["AddEffect"] = &LuaFunction::Object::AddEffect;
			objectType["RemoveEffect"] = &LuaFunction::Object::RemoveEffect;
			objectType["RemoveEffectIndex"] = &LuaFunction::Object::RemoveEffectIndex;

			objectType["UseAbility"] = &LuaFunction::Object::UseAbility;
			objectType["DropLoot"] = &LuaFunction::Object::DropLoot;

			objectType["GetSharedAbilityOffset"] = &LuaFunction::Object::GetSharedAbilityOffset;

			// Agent blackboard
			objectType["GetTargetId"] = &Object::GetTargetId;
			objectType["SetTargetId"] = &Object::SetTargetId;

			objectType["IsStealthed"] = &LuaFunction::Object::IsStealthed;
			objectType["GetStealthType"] = &Object::GetStealthType;
			objectType["SetStealthType"] = &Object::SetStealthType;

			objectType["IsInCombat"] = &Object::IsInCombat;
			objectType["SetInCombat"] = &Object::SetInCombat;

			objectType["IsTargetable"] = &Object::IsTargetable;
			objectType["SetTargetable"] = &Object::SetTargetable;

			// Markers
			objectType["GetTeleporterDestination"] = &LuaFunction::Object::GetTeleporterDestination;
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

	// Ability
	Ability::Ability(Lua& lua, sol::table&& self) : mLua(lua), mSelf(std::move(self)) {
		if (mSelf == sol::nil) {
			throw std::exception("Ability::Ability: self is nil");
		}

		sol::state_view state = mSelf.lua_state();
		mEnvironment = sol::environment(state, sol::create, state.globals());
		mEnvironment["_ABILITY"] = this;

		if (sol::object value = mSelf["activate"]; value.is<sol::function>()) {
			mEnvironment.set_on(value);
			mActivateFn = value.as<sol::function>();
		}

		if (sol::object value = mSelf["deactivate"]; value.is<sol::function>()) {
			mEnvironment.set_on(value);
			mDeactivateFn = value.as<sol::function>();
		}

		if (sol::object value = mSelf["tick"]; value.is<sol::function>()) {
			mEnvironment.set_on(value);
			mTickFn = value.as<sol::function>();
		}

		// Fetch properties
		mDescriptors = mSelf.get_or("descriptors", Descriptors::IsBasic);
	}

	void Ability::Reload() {

	}

	Descriptors Ability::GetDescriptors() const {
		return mDescriptors;
	}
}
