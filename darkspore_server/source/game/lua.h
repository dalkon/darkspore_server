
#ifndef _GAME_LUA_HEADER
#define _GAME_LUA_HEADER

// Include
#define SOL_STRINGS_ARE_NUMBERS 1
#define SOL_SAFE_FUNCTION 1

#include <sol/sol.hpp>
#include <unordered_map>

// Game
namespace Game {
	// Predefined
	class Instance;
	class LuaThread;

	// Lua
	class Lua {
		public:
			Lua(Instance& game);
			~Lua();

			void Initialize();
			void Terminate();

			static sol::protected_function_result ReportError(lua_State* L, sol::protected_function_result pfr);
			static const char* GetStackTrace(lua_State* L, const char* error);

			bool LoadFile(const std::string& path);
			bool LoadBuffer(const std::string& buffer);

			lua_State* GetState() const;
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

			sol::state mState;
			sol::table mEventTable;

			std::unordered_map<lua_State*, LuaThread*> mThreads;
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
