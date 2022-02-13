
#ifndef _GAME_LUA_HEADER
#define _GAME_LUA_HEADER

// Include
#include "attributes.h"

#define SOL_SAFE_FUNCTION_OBJECTS 1
#define SOL_PRINT_ERRORS 1
#define SOL_USING_CXX_LUAJIT 1
#include <sol/sol.hpp>

#include <unordered_map>
#include <unordered_set>
#include <functional>

// Game
namespace Game {
	// Predefined
	class Instance;
	class Lua;
	class Ability;

	using AbilityPtr = std::shared_ptr<Ability>;

	// LuaThread
	class LuaThread {
		public:
			using ResumeCondition = std::function<bool(lua_State*, sol::variadic_results&)>;

		public:
			LuaThread(Lua& lua);

			auto lua_state() const { return mThread.thread_state(); }
			auto status() const { return mCoroutine.status(); }

			sol::object get_value(const std::string& key) const;
			void set_value(const std::string& key, sol::object&& value);

			bool resume();

			void set_resume_condition(const ResumeCondition& condition);
			void set_resume_condition(ResumeCondition&& condition);

			template<typename... Args>
			void call(const sol::function& func, Args&&... args) {
				mCoroutine = sol::coroutine(mThread.thread_state(), sol::ref_index(func.registry_index()));
				
				auto result = mCoroutine(std::forward<Args>(args)...);
				if (result.valid()) {
					post_call();
				}
			}

			template<typename... Args>
			void call(const sol::environment& env, const sol::function& func, Args&&... args) {
				env.set_on(mThread);
				call(func, std::forward<Args>(args)...);
			}

			void post_call();
		
		private:
			Lua& mLua;
			
			std::unordered_map<std::string, sol::object> mValues;

			ResumeCondition mResumeCondition;

			sol::thread mThread;
			sol::coroutine mCoroutine;
			sol::environment mEnvironment;

			friend class Lua;
	};

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

			void Update();
			void PreloadAbilities();

			AbilityPtr GetAbility(const std::string& abilityName);
			AbilityPtr GetAbility(uint32_t abilityId);

			sol::table GetPrivateTable(uint32_t id) const;
			sol::table CreatePrivateTable(uint32_t id);
			void RemovePrivateTable(uint32_t id);

			LuaThread* SpawnThread();
			LuaThread* GetThread(lua_State* L) const;
			void YieldThread(LuaThread* thread);

			template<typename... Args>
			void CallCoroutine(std::nullptr_t, const sol::function& func, Args&&... args) {
				SpawnThread()->call(func, std::forward<Args>(args)...);
			}
			template<typename... Args>
			void CallCoroutine(const sol::environment& env, const sol::function& func, Args&&... args) {
				SpawnThread()->call(env, func, std::forward<Args>(args)...);
			}
			void ReturnThreadToPool(LuaThread* thread);

		private:
			void RegisterEnums();
			void RegisterFunctions();

		private:
			Instance& mGame;

			std::unordered_map<lua_State*, LuaThread*> mThreads;
			std::unordered_map<uint32_t, AbilityPtr> mAbilities;
			std::unordered_map<uint32_t, sol::table> mPrivateTables;

			std::unordered_set<LuaThread*> mYieldedThreads;

			std::vector<LuaThread*> mThreadPool;
	};

	// Ability
	class Ability {
		public:
			Ability(Lua& lua, sol::table&& self);

			void Reload();

			Descriptors GetDescriptors() const;

			template<typename... Args>
			void Activate(Args&&... args) const {
				if (mActivateFn) {
					mLua.CallCoroutine(mEnvironment, mActivateFn, mSelf, std::forward<Args>(args)...);
				}
			}

			template<typename... Args>
			void Deactivate(Args&&... args) const {
				if (mDeactivateFn) {
					mLua.CallCoroutine(mEnvironment, mDeactivateFn, mSelf, std::forward<Args>(args)...);
				}
			}

			template<typename... Args>
			void Tick(Args&&... args) const {
				if (mTickFn) {
					mLua.CallCoroutine(mEnvironment, mTickFn, mSelf, std::forward<Args>(args)...);
				}
			}

		private:
			Lua& mLua;

			Descriptors mDescriptors;
			
			sol::table mSelf;
			sol::environment mEnvironment;

			sol::function mActivateFn;
			sol::function mDeactivateFn;
			sol::function mTickFn;
	};
}

#endif
