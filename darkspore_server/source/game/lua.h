
#ifndef _GAME_LUA_HEADER
#define _GAME_LUA_HEADER

// Include
#include <sol/sol.hpp>
#include <unordered_map>

// Game
namespace Game {
	// Predefined
	class Instance;
	class LuaThread;

	// Ability
	class Ability {
		public:
			Ability(sol::table&& self);

			void Reload();

			template<typename... Args>
			void Activate(Args&&... args) const {
				if (mHasActivate) {
					sol::protected_function fn = mSelf["activate"];
					fn.call<void>(mSelf, std::forward<Args>(args)...);
				}
			}

			template<typename... Args>
			void Deactivate(Args&&... args) const {
				if (mHasDeactivate) {
					sol::protected_function fn = mSelf["deactivate"];
					fn.call<void>(mSelf, std::forward<Args>(args)...);
				}
			}

			template<typename... Args>
			void Tick(Args&&... args) const {
				if (mHasTick) {
					sol::protected_function fn = mSelf["tick"];
					fn.call<void>(mSelf, std::forward<Args>(args)...);
				}
			}

		private:
			sol::table mSelf;
			sol::environment mEnvironment;

			bool mHasActivate = false;
			bool mHasDeactivate = false;
			bool mHasTick = false;
	};

	using AbilityPtr = std::shared_ptr<Ability>;

	// LuaBase
	class LuaBase {
		public:
			LuaBase() = default;
			virtual ~LuaBase();

			virtual void Initialize();
			virtual void Reload();

			static sol::protected_function_result ReportError(lua_State* L, sol::protected_function_result pfr);
			static const char* GetStackTrace(lua_State* L, const char* error);

			sol::protected_function_result LoadFile(const std::string& path);
			sol::protected_function_result LoadBuffer(const std::string& buffer);

			lua_State* GetState() const;

			void CollectGarbage();

		private:
			void RegisterFunctions();

		protected:
			sol::state mState;

		private:
			sol::table mDefaultPackages;
	};

	// GlobalLua
	class GlobalLua : public LuaBase {
		public:
			~GlobalLua();

			static GlobalLua& Instance();

			void Initialize() override;
			void Reload() override;

			sol::bytecode GetAbility(const std::string& abilityName) const;
			sol::bytecode GetAbility(uint32_t abilityId) const;

		private:
			void LoadAbilities();

		private:
			static GlobalLua sInstance;

			std::unordered_map<uint32_t, sol::bytecode> mLoadedAbilities;
	};

	// Lua
	class Lua : public LuaBase {
		public:
			Lua(Instance& game);
			~Lua();

			void Initialize() override;
			void Reload() override;
			void PreloadAbilities();

			AbilityPtr GetAbility(const std::string& abilityName);
			AbilityPtr GetAbility(uint32_t abilityId);

			sol::table GetPrivateTable(uint32_t id) const;
			sol::table CreatePrivateTable(uint32_t id);
			void RemovePrivateTable(uint32_t id);

			LuaThread* GetThread(lua_State* L) const;

			template<typename... Args>
			void CreateThread(sol::function func, sol::environment env, Args&&... args) {
				auto thread = new LuaThread(*this, std::move(func), std::move(env));
				mThreads.emplace(thread->lua_state(), thread);
				thread->call(std::forward<Args>(args)...);
			}
			void RemoveThread(LuaThread* thread);

		private:
			void RegisterEnums();
			void RegisterFunctions();

		private:
			Instance& mGame;

			std::unordered_map<lua_State*, LuaThread*> mThreads;
			std::unordered_map<uint32_t, AbilityPtr> mAbilities;
			std::unordered_map<uint32_t, sol::table> mPrivateTables;
	};

	// LuaThread
	class LuaThread {
		public:
			LuaThread(Lua& lua, sol::function&& func, sol::environment&& env);

			auto lua_state() const { return mThread.thread_state(); }

			template<typename... Args>
			void call(Args&&... args) {
				mCoroutine(std::forward<Args>(args)...);
				if (mCoroutine.status() == sol::call_status::ok) {
					mLua.RemoveThread(this);
				}
			}
		
		private:
			Lua& mLua;

			sol::thread mThread;
			sol::coroutine mCoroutine;
			sol::environment mEnvironment;
	};
}

#endif
