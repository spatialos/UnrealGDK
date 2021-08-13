// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Components/InstancedStaticMeshComponent.h"
#include "HAL/ThreadSafeBool.h"

#include "ScavengersHubIsmComponent.generated.h"

// A custom subclass of Unreal's Instanced Static Mesh Component that allows instances to be updated concurrently from multiple
// threads.
//
// This component is designed to be used at runtime rather than edit time and has the following limitations:
// * Doesn't update navigation when an instance is updated
// * Doesn't mark the owning package or level as dirty when an instance is updated, as instances are not expected to be changed at
// edit time
// * Collision is not supported

UCLASS()
class SCAVENGERSHUBGAMEFRAMEWORK_API UScavengersHubIsmComponent : public UInstancedStaticMeshComponent
{
	GENERATED_BODY()
public:
	// ActorComponent interface
	virtual bool ShouldCreatePhysicsState() const override;

	// UInstancedStaticMeshComponent interface
	virtual bool UpdateInstanceTransform(int32 InstanceIndex, const FTransform& NewInstanceTransform, bool bWorldSpace = false,
										 bool bMarkRenderStateDirty = false, bool bTeleport = false) override;

	virtual bool SetCustomData(int32 InstanceIndex, const TArray<float>& CustomDataFloats, bool bMarkRenderStateDirty = false) override;

private:
	void MarkAsModified();

	FThreadSafeBool WasModifiedThisFrame;
};
