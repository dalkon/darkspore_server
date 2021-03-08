
// Include
#include "locomotion.h"
#include "object.h"

#include "raknet/types.h"

#include <algorithm>

// Game
namespace Game {
	// LobParameters
	void LobParameters::WriteTo(RakNet::BitStream& stream) const {
		using RakNet::bytes_to_bits;
		using RakNet::ReallocateStream;

		constexpr auto size = bytes_to_bits(0x54);

		auto writeOffset = ReallocateStream(stream, size);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x18));
		Write(stream, mUpDirection);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x30));
		Write(stream, mBounceNumber);
		Write(stream, mBounceRestitution);
		Write(stream, mGroundCollisionOnly);
		Write(stream, mStopBounceOnCreatures);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x3C));
		Write(stream, mPlaneDirection);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x48));
		Write(stream, mPlaneLinear);
		Write(stream, mUpLinear);
		Write(stream, mUpQuadratic);

		stream.SetWriteOffset(writeOffset + size);
	}

	void LobParameters::WriteReflection(RakNet::BitStream& stream) const {
		RakNet::reflection_serializer<9> reflector(stream);
		reflector.begin();
		reflector.write<0>(mPlaneLinear);
		reflector.write<1>(mUpLinear);
		reflector.write<2>(mUpQuadratic);
		reflector.write<3>(mUpDirection);
		reflector.write<4>(mPlaneDirection);
		reflector.write<5>(mBounceNumber);
		reflector.write<6>(mBounceRestitution);
		reflector.write<7>(mGroundCollisionOnly);
		reflector.write<8>(mStopBounceOnCreatures);
		reflector.end();
	}

	// ProjectileParameters
	void ProjectileParameters::WriteTo(RakNet::BitStream& stream) const {
		using RakNet::bytes_to_bits;
		using RakNet::ReallocateStream;

		constexpr auto size = bytes_to_bits(0x3C);

		auto writeOffset = ReallocateStream(stream, size);

		stream.SetWriteOffset(writeOffset);
		Write(stream, mSpeed);
		Write(stream, mAcceleration);
		Write(stream, mJinkInfo);
		Write(stream, mRange);
		Write(stream, mSpinRate);
		Write(stream, mDirection);
		Write(stream, mProjectileFlags);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x24));
		Write(stream, mHomingDelay);
		Write(stream, mTurnRate);
		Write(stream, mTurnAcceleration);
		Write(stream, mPiercing);
		Write(stream, mIgnoreGroundCollide);
		Write(stream, mIgnoreCreatureCollide);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x34));
		Write(stream, mEccentricity);
		Write(stream, mCombatantSweepHeight);

		stream.SetWriteOffset(writeOffset + size);
	}

	void ProjectileParameters::WriteReflection(RakNet::BitStream& stream) const {
		RakNet::reflection_serializer<15> reflector(stream);
		reflector.begin();
		reflector.write<0>(mSpeed);
		reflector.write<1>(mAcceleration);
		reflector.write<2>(mJinkInfo);
		reflector.write<3>(mRange);
		reflector.write<4>(mSpinRate);
		reflector.write<5>(mDirection);
		reflector.write<6>(mProjectileFlags);
		reflector.write<7>(mHomingDelay);
		reflector.write<8>(mTurnRate);
		reflector.write<9>(mTurnAcceleration);
		reflector.write<10>(mEccentricity);
		reflector.write<11>(mPiercing);
		reflector.write<12>(mIgnoreGroundCollide);
		reflector.write<13>(mIgnoreCreatureCollide);
		reflector.write<14>(mCombatantSweepHeight);
		reflector.end();
	}

	// Locomotion
	Locomotion::Locomotion(Object& object) : mObject(object) {}

	const glm::vec3& Locomotion::GetGoalPosition() const {
		return mGoalPosition;
	}

	const glm::vec3& Locomotion::GetPartialGoalPosition() const {
		return mPartialGoalPosition;
	}

	const glm::vec3& Locomotion::GetFacing() const {
		return mFacing;
	}

	const glm::vec3& Locomotion::GetExternalLinearVelocity() const {
		return mExternalLinearVelocity;
	}

	const glm::vec3& Locomotion::GetExternalForce() const {
		return mExternalForce;
	}

	const glm::vec3& Locomotion::GetTargetPosition() const {
		return mTargetPosition;
	}

	const glm::vec3& Locomotion::GetExpectedGeoCollision() const {
		return mExpectedGeoCollision;
	}

	const glm::vec3& Locomotion::GetInitialDirection() const {
		return mInitialDirection;
	}

	const glm::vec3& Locomotion::GetOffset() const {
		return mOffset;
	}

	uint32_t Locomotion::GetGoalFlags() const {
		return mGoalFlags;
	}

	uint32_t Locomotion::GetTargetId() const {
		return mTargetId;
	}

	float Locomotion::GetAllowedStopDistance() const {
		return mAllowedStopDistance;
	}

	float Locomotion::GetDesiredStopDistance() const {
		return mDesiredStopDistance;
	}

	void Locomotion::SetGoalPosition(const glm::vec3& position) {
		// reset data
		mTargetId = 0;
		mFacing = glm::zero<glm::vec3>();
		mTargetPosition = glm::zero<glm::vec3>();
		mExternalLinearVelocity = glm::zero<glm::vec3>();

		// set data
		mGoalFlags = 0x001;
		mGoalPosition = position;
		mAllowedStopDistance = 0;
		mDesiredStopDistance = 0;

		mObject.SetFlags(mObject.GetFlags() | Object::Flags::UpdateLocomotion);
	}

	void Locomotion::SetGoalPositionWithDistance(const glm::vec3& position, float distance) {
		SetGoalPosition(position);

		mAllowedStopDistance = distance;
		mDesiredStopDistance = distance;
	}

	void Locomotion::SetPartialGoalPosition(const glm::vec3& position) {
		mPartialGoalPosition = position;
	}
	
	void Locomotion::SetGoalObject(const ObjectPtr& object, float distance) {
		if (!object) {
			return;
		}

		// distance = GetAllowedStoppingDistance(object) + distance;
		SetGoalPosition(object->GetPosition());

		// mGoalFlags |= (1 & 0x40);
		// mGoalFlags |= (2 & 0x80);

		mAllowedStopDistance = distance;
		mDesiredStopDistance = distance;

		mObject.SetFlags(mObject.GetFlags() | Object::Flags::UpdateLocomotion);
	}

	void Locomotion::SetGoalObjectEx(const ObjectPtr& object, float distance) {
		if (!object) {
			return;
		}

		// reset data
		mFacing = glm::zero<glm::vec3>();
		mTargetPosition = glm::zero<glm::vec3>();
		mExternalLinearVelocity = glm::zero<glm::vec3>();

		// set data
		mGoalFlags = 0x400;
		// mGoalFlags = 1 & 0x040;
		mTargetId = object->GetId();
		mGoalPosition = object->GetPosition();
		mAllowedStopDistance = distance;
		mDesiredStopDistance = distance;

		mObject.SetFlags(mObject.GetFlags() | Object::Flags::UpdateLocomotion);
	}

	void Locomotion::SetGoalFlags(uint32_t flags) {
		mGoalFlags = flags;
	}
	
	void Locomotion::Stop() {
		// reset data
		mTargetId = 0;
		mFacing = glm::zero<glm::vec3>();

		// set data
		mGoalFlags = 0x020;

		mObject.SetFlags(mObject.GetFlags() | Object::Flags::UpdateLocomotion);
	}
	
	void Locomotion::MoveToPointWhileFacingTarget(const glm::vec3& position, const ObjectPtr& object) {
		// reset data
		mFacing = glm::zero<glm::vec3>();
		mTargetPosition = glm::zero<glm::vec3>();
		mExternalLinearVelocity = glm::zero<glm::vec3>();

		// set data
		mGoalFlags = 0x001 | 0x100;
		mGoalPosition = position;
		if (object) {
			mTargetId = object->GetId();
		} else {
			mTargetId = 0;
		}

		mAllowedStopDistance = 0;
		mDesiredStopDistance = 0;

		// extra_func(0x100);
		mObject.SetFlags(mObject.GetFlags() | Object::Flags::UpdateLocomotion);
	}
	
	void Locomotion::ApplyExternalVelocity(const glm::vec3& velocity) {
		// reset data
		mTargetId = 0;
		mFacing = glm::zero<glm::vec3>();

		// set data
		mGoalFlags = 0x008;
		mExternalLinearVelocity = velocity;

		mObject.SetFlags(mObject.GetFlags() | Object::Flags::UpdateLocomotion);
	}

	void Locomotion::ClearExternalVelocity(const glm::vec3& velocity) {
		// set data
		mGoalFlags = 0x020;
		mExternalLinearVelocity = glm::zero<glm::vec3>();

		mObject.SetFlags(mObject.GetFlags() | Object::Flags::UpdateLocomotion);
	}

	void Locomotion::WriteTo(RakNet::BitStream& stream) const {
		using RakNet::bytes_to_bits;
		using RakNet::ReallocateStream;

		constexpr auto size = bytes_to_bits(0x290);

		auto writeOffset = ReallocateStream(stream, size);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x08));
		Write(stream, mReflectedLastUpdate);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x44));
		mProjectileParameters.WriteTo(stream);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x84));
		Write(stream, mExpectedGeoCollision);
		Write(stream, mTargetId);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x9C));
		Write(stream, mInitialDirection);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0xD8));
		Write(stream, mLobStartTime);
		Write(stream, mLobPrevSpeedModifier);
		mLobParameters.WriteTo(stream);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x138));
		Write(stream, mOffset);
		Write(stream, mGoalFlags);
		Write(stream, mGoalPosition);
		Write(stream, mPartialGoalPosition);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x178));
		Write(stream, mFacing);
		Write(stream, mExternalLinearVelocity);
		Write(stream, mExternalForce);
		Write(stream, mAllowedStopDistance);
		Write(stream, mDesiredStopDistance);

		stream.SetWriteOffset(writeOffset + bytes_to_bits(0x1AC));
		Write(stream, mTargetPosition);

		stream.SetWriteOffset(writeOffset + size);
	}

	void Locomotion::WriteReflection(RakNet::BitStream& stream) const {
		RakNet::reflection_serializer<18> reflector(stream);
		reflector.begin();
		reflector.write<0>(mLobStartTime);
		reflector.write<1>(mLobPrevSpeedModifier);
		reflector.write<2>(mLobParameters);
		reflector.write<3>(mProjectileParameters);
		reflector.write<4>(mGoalFlags);
		reflector.write<5>(mGoalPosition);
		reflector.write<6>(mPartialGoalPosition);
		reflector.write<7>(mFacing);
		reflector.write<8>(mExternalLinearVelocity);
		reflector.write<9>(mExternalForce);
		reflector.write<10>(mAllowedStopDistance);
		reflector.write<11>(mDesiredStopDistance);
		reflector.write<12>(mTargetId);
		reflector.write<13>(mTargetPosition);
		reflector.write<14>(mExpectedGeoCollision);
		reflector.write<15>(mInitialDirection);
		reflector.write<16>(mOffset);
		reflector.write<17>(mReflectedLastUpdate);
		reflector.end();
	}
}
