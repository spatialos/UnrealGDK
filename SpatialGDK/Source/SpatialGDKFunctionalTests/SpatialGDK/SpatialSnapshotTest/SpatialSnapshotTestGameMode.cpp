// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialSnapshotTestGameMode.h"
#include "Net/UnrealNetwork.h"

namespace
{
constexpr bool bTargetBoolProperty = true;
constexpr int32 TargetInt32Property = -1050;
constexpr int64 TargetInt64Property = 8000000;
constexpr float TargetFloatProperty = 1000.0f;
const FString TargetStringProperty = TEXT("Some String");
const FName TargetNameProperty = TEXT("Some Name");

TArray<int> GetTargetIntArrayProperty()
{
	TArray<int> Array;
	for (int i = 0; i != 10; ++i)
	{
		Array.Add(i);
	}
	return Array;
}
} // namespace

ASpatialSnapshotTestGameMode::ASpatialSnapshotTestGameMode()
	: Super()
{
}

void ASpatialSnapshotTestGameMode::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpatialSnapshotTestGameMode, bBoolProperty);
	DOREPLIFETIME(ASpatialSnapshotTestGameMode, Int32Property);
	DOREPLIFETIME(ASpatialSnapshotTestGameMode, Int64Property);
	DOREPLIFETIME(ASpatialSnapshotTestGameMode, FloatProperty);
	DOREPLIFETIME(ASpatialSnapshotTestGameMode, StringProperty);
	DOREPLIFETIME(ASpatialSnapshotTestGameMode, NameProperty);
	DOREPLIFETIME(ASpatialSnapshotTestGameMode, IntArrayProperty);
}

void ASpatialSnapshotTestGameMode::CrossServerSetProperties_Implementation()
{
	bBoolProperty = bTargetBoolProperty;
	Int32Property = TargetInt32Property;
	Int64Property = TargetInt64Property;
	FloatProperty = TargetFloatProperty;
	StringProperty = TargetStringProperty;
	NameProperty = TargetNameProperty;

	IntArrayProperty.Empty();

	TArray<int> TargetArray = GetTargetIntArrayProperty();
	for (int i : TargetArray)
	{
		IntArrayProperty.Add(i);
	}
}

bool ASpatialSnapshotTestGameMode::VerifyBool()
{
	return bBoolProperty == bTargetBoolProperty;
}

bool ASpatialSnapshotTestGameMode::VerifyInt32()
{
	return Int32Property == TargetInt32Property;
}

bool ASpatialSnapshotTestGameMode::VerifyInt64()
{
	return Int64Property == TargetInt64Property;
}

bool ASpatialSnapshotTestGameMode::VerifyFloat()
{
	return FMath::IsNearlyEqual(FloatProperty, TargetFloatProperty);
}

bool ASpatialSnapshotTestGameMode::VerifyString()
{
	return StringProperty == TargetStringProperty;
}

bool ASpatialSnapshotTestGameMode::VerifyName()
{
	return NameProperty == TargetNameProperty;
}

bool ASpatialSnapshotTestGameMode::VerifyIntArray()
{
	TArray<int> TargetIntArray = GetTargetIntArrayProperty();
	if (IntArrayProperty.Num() != TargetIntArray.Num())
	{
		return false;
	}
	for (int i = 0; i != IntArrayProperty.Num(); ++i)
	{
		if (IntArrayProperty[i] != TargetIntArray[i])
		{
			return false;
		}
	}
	return true;
}
