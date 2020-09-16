// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialSnapshotTestActor.h"
#include "Net/UnrealNetwork.h"

namespace
{
constexpr bool bTargetBoolProperty = true;
constexpr int32 TargetInt32Property = -1050;
constexpr int64 TargetInt64Property = 8000000;
constexpr float TargetFloatProperty = 1000.0f;
const wchar_t* TargetStringProperty = TEXT("Some String");
const wchar_t* TargetNameProperty = TEXT("Some String");

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

ASpatialSnapshotTestActor::ASpatialSnapshotTestActor()
	: Super()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.0f;
}

void ASpatialSnapshotTestActor::BeginPlay()
{
	Super::BeginPlay();
}

void ASpatialSnapshotTestActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpatialSnapshotTestActor, bBoolProperty);
	DOREPLIFETIME(ASpatialSnapshotTestActor, Int32Property);
	DOREPLIFETIME(ASpatialSnapshotTestActor, Int64Property);
	DOREPLIFETIME(ASpatialSnapshotTestActor, FloatProperty);
	DOREPLIFETIME(ASpatialSnapshotTestActor, StringProperty);
	DOREPLIFETIME(ASpatialSnapshotTestActor, NameProperty);
	DOREPLIFETIME(ASpatialSnapshotTestActor, IntArrayProperty);
}

void ASpatialSnapshotTestActor::CrossServerSetProperties_Implementation()
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

bool ASpatialSnapshotTestActor::VerifyBool()
{
	return bBoolProperty == bTargetBoolProperty;
}

bool ASpatialSnapshotTestActor::VerifyInt32()
{
	return Int32Property == TargetInt32Property;
}

bool ASpatialSnapshotTestActor::VerifyInt64()
{
	return Int64Property == TargetInt64Property;
}

bool ASpatialSnapshotTestActor::VerifyFloat()
{
	return FMath::IsNearlyEqual(FloatProperty, TargetFloatProperty);
}

bool ASpatialSnapshotTestActor::VerifyString()
{
	return StringProperty == TargetStringProperty;
}

bool ASpatialSnapshotTestActor::VerifyName()
{
	return NameProperty == TargetNameProperty;
}

bool ASpatialSnapshotTestActor::VerifyIntArray()
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
