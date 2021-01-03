
#ifndef _GAME_PLAYER_HEADER
#define _GAME_PLAYER_HEADER

// Include
#include "predefined.h"
#include "object.h"

// Game
namespace Game {
	// Player
	class Player {
		public:
			static uint32_t GetXPForLevel(uint32_t level);

		public:
			Player(Instance& instance, const SporeNet::UserPtr& user, uint8_t playerIndex);

			RakNet::labsPlayer& GetData();
			const RakNet::labsPlayer& GetData() const;

			uint8_t GetId() const;

			void Setup();

			void GetStatus(uint32_t& status, float& progress) const;
			void SetStatus(uint32_t status, float progress);

			ObjectPtr GetCharacterObject(uint32_t index) const;

			SporeNet::SquadPtr GetSquad() const;
			void SetSquad(const SporeNet::SquadPtr& squad);

			const RakNet::labsCharacter& GetCharacter(uint32_t index) const;
			void SetCharacter(RakNet::labsCharacter&& character, uint32_t index);

			const RakNet::labsCrystal& GetCrystal(uint32_t index) const;
			void SetCrystal(RakNet::labsCrystal&& crystal, uint32_t index);

			uint16_t GetUpdateBits() const;
			void SetUpdateBits(uint16_t bits);
			void ResetUpdateBits();

		private:
			void UpdateCrystalBonuses();

		private:
			Instance& mInstance;

			std::array<ObjectPtr, 3> mCharacterObjects;

			SporeNet::UserPtr mUser;
			SporeNet::SquadPtr mSquad;

			RakNet::labsPlayer mData;

			uint16_t mUpdateBits = 0;
	};

	using PlayerPtr = std::shared_ptr<Player>;
}

#endif
