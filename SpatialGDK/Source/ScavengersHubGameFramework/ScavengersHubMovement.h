#pragma once

#include "Engine/EngineTypes.h"

#include "ScavengersHubMovement.generated.h"

USTRUCT()
struct FQuantisedPosition
{
	GENERATED_BODY()

	UPROPERTY()
	uint64 QuantisedX;

	UPROPERTY()
	uint64 QuantisedY;

	UPROPERTY()
	uint64 QuantisedZ;

	FQuantisedPosition() = default;

	void SetPosition(FVector Position)
	{
		QuantisedX = 100000 + static_cast<int64>(FMath::RoundToInt(Position.X / 10.f));
		QuantisedY = 100000 + static_cast<int64>(FMath::RoundToInt(Position.Y / 10.f));
		QuantisedZ = 100000 + static_cast<int64>(FMath::RoundToInt(Position.Z / 10.f));
	}

	FVector GetPosition() const
	{
		FVector Position = FVector::ZeroVector;
		Position.X = static_cast<float>(static_cast<int64>(QuantisedX) - 100000) * 10.f;
		Position.Y = static_cast<float>(static_cast<int64>(QuantisedY) - 100000) * 10.f;
		Position.Z = static_cast<float>(static_cast<int64>(QuantisedZ) - 100000) * 10.f;

		return Position;
	}
};

USTRUCT()
struct FQuantisedRotation
{
	GENERATED_BODY()

	UPROPERTY()
	uint64 QuantisedPitch;

	UPROPERTY()
	uint64 QuantisedYaw;

	UPROPERTY()
	uint64 QuantisedRoll;

	FQuantisedRotation() = default;

	void SetRotation(FRotator Rotation)
	{
		Rotation.Normalize();
		QuantisedPitch = 181 + static_cast<int64>(FMath::RoundToInt(Rotation.Pitch / 10.f));
		QuantisedYaw = 181 + static_cast<int64>(FMath::RoundToInt(Rotation.Yaw / 10.f));
		QuantisedRoll = 181 + static_cast<int64>(FMath::RoundToInt(Rotation.Roll / 10.f));
	}

	FRotator GetRotation() const
	{
		FRotator Rotation;
		Rotation.Pitch = static_cast<float>(static_cast<int64>(QuantisedPitch) - 181) * 10.f;
		Rotation.Yaw = static_cast<float>(static_cast<int64>(QuantisedYaw) - 181) * 10.f;
		Rotation.Roll = static_cast<float>(static_cast<int64>(QuantisedRoll) - 181) * 10.f;
		return Rotation;
	}
};

USTRUCT()
struct FLowFidelityQuantisedPosition
{
	GENERATED_BODY()

	UPROPERTY()
	uint64 QuantisedX;

	UPROPERTY()
	uint64 QuantisedY;

	UPROPERTY()
	uint64 QuantisedZ;

	FLowFidelityQuantisedPosition() = default;

	void SetPosition(FVector Position)
	{
		QuantisedX = 100000 + static_cast<int64>(FMath::RoundToInt(Position.X / 50.f));
		QuantisedY = 100000 + static_cast<int64>(FMath::RoundToInt(Position.Y / 50.f));
		QuantisedZ = 100000 + static_cast<int64>(FMath::RoundToInt(Position.Z / 25.f));
	}

	FVector GetPosition() const
	{
		FVector Position = FVector::ZeroVector;
		Position.X = static_cast<float>(static_cast<int64>(QuantisedX) - 100000) * 50.f;
		Position.Y = static_cast<float>(static_cast<int64>(QuantisedY) - 100000) * 50.f;
		Position.Z = static_cast<float>(static_cast<int64>(QuantisedZ) - 100000) * 25.f;

		return Position;
	}
};

USTRUCT(BlueprintType)
struct FScavengersHubMovement
{
	GENERATED_BODY()

public:
	// Always in world space
	UPROPERTY(BlueprintReadWrite)
	FVector WorldLocation;

	UPROPERTY(BlueprintReadWrite)
	FRotator Rotation;

	UPROPERTY(BlueprintReadWrite)
	FVector Velocity;

	FScavengersHubMovement()
	{
		WorldLocation = FVector::ZeroVector;
		Rotation = FRotator::ZeroRotator;
		Velocity = FVector::ZeroVector;
	}

	FScavengersHubMovement(FRepMovement Movement)
	{
		WorldLocation = Movement.Location;
		Rotation = Movement.Rotation;
		Velocity = Movement.LinearVelocity;
	}

	FScavengersHubMovement(FTransform Transform)
	{
		WorldLocation = Transform.GetLocation();
		Rotation = Transform.Rotator();
		Velocity = FVector::ZeroVector;
	}

	FScavengersHubMovement(FVector Position, FRotator Rotator)
	{
		WorldLocation = Position;
		Rotation = Rotator;
		Velocity = FVector::ZeroVector;
	}

	FRepMovement ToRepMovement() const
	{
		FRepMovement Movement{};
		Movement.Location = WorldLocation;
		Movement.Rotation = Rotation;
		Movement.LinearVelocity = Velocity;
		return Movement;
	}

	FTransform ToTransform() const
	{
		FTransform Transform = FTransform::Identity;
		Transform.SetLocation(WorldLocation);
		Transform.SetRotation(Rotation.Quaternion());
		return Transform;
	}

	static FScavengersHubMovement Blend(FScavengersHubMovement A, FScavengersHubMovement B, float Alpha)
	{
		if (FMath::IsNearlyEqual(Alpha, 0))
		{
			return A;
		}
		if (FMath::IsNearlyEqual(Alpha, 1))
		{
			return B;
		}
		FScavengersHubMovement Movement;
		Movement.WorldLocation = FMath::Lerp(A.WorldLocation, B.WorldLocation, Alpha);
		Movement.Rotation = FQuat::Slerp(A.Rotation.Quaternion(), B.Rotation.Quaternion(), Alpha).Rotator();
		Movement.Velocity = FMath::Lerp(A.Velocity, B.Velocity, Alpha);
		return Movement;
	}
};

USTRUCT()
struct FScavengersHubMovementContainer
{
	GENERATED_BODY()

	UPROPERTY()
	FScavengersHubMovement Movement;

	void ForceNetUpdate() { ForceNetUpdateValue++; }

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		SerializePackedVector<1, 24>(Movement.WorldLocation, Ar);
		SerializePackedVector<1, 24>(Movement.Velocity, Ar);
		Movement.Rotation.SerializeCompressed(Ar);
		bOutSuccess = true;
		return true;
	}

private:
	UPROPERTY()
	uint8 ForceNetUpdateValue = 0;
};

template <>
struct TStructOpsTypeTraits<FScavengersHubMovementContainer> : public TStructOpsTypeTraitsBase2<FScavengersHubMovementContainer>
{
	enum
	{
		WithNetSerializer = true
	};
};
