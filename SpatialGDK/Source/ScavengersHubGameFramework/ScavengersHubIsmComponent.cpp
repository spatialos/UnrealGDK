// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ScavengersHubGameFramework/ScavengersHubIsmComponent.h"

#include "Async/Async.h"

#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

class IScavengersHubPlugin : public IModuleInterface
{
public:
	static inline IScavengersHubPlugin& Get() { return FModuleManager::LoadModuleChecked<IScavengersHubPlugin>("ScavengersHubPlugin"); }
	static inline bool IsAvailable() { return FModuleManager::Get().IsModuleLoaded("ScavengersHubPlugin"); }
};

class FScavengersHubPlugin : public IScavengersHubPlugin
{
	virtual void StartupModule() override {}
	virtual void ShutdownModule() override {}
};

IMPLEMENT_MODULE(FScavengersHubPlugin, ScavengersHubGameFramework)

bool UScavengersHubIsmComponent::ShouldCreatePhysicsState() const
{
	// Suppress creating any collision regardless of settings
	return false;
}

bool UScavengersHubIsmComponent::UpdateInstanceTransform(int32 InstanceIndex, const FTransform& NewInstanceTransform, bool bWorldSpace,
														 bool bMarkRenderStateDirty, bool bTeleport)
{
	if (!PerInstanceSMData.IsValidIndex(InstanceIndex))
	{
		return false;
	}

	FInstancedStaticMeshInstanceData& InstanceData = PerInstanceSMData[InstanceIndex];

	// Render data uses local transform of the instance
	FTransform LocalTransform = bWorldSpace ? NewInstanceTransform.GetRelativeTransform(GetComponentTransform()) : NewInstanceTransform;

	InstanceData.Transform = LocalTransform.ToMatrixWithScale();

	MarkAsModified();

	return true;
}

bool UScavengersHubIsmComponent::SetCustomData(int32 InstanceIndex, const TArray<float>& CustomDataFloats, bool bMarkRenderStateDirty)
{
	const int32 NumToCopy = FMath::Min(CustomDataFloats.Num(), NumCustomDataFloats);

	const int32 LastDestinationFloatIndex = InstanceIndex * NumCustomDataFloats + NumToCopy - 1;
	if (NumToCopy == 0 || !PerInstanceSMCustomData.IsValidIndex(LastDestinationFloatIndex))
	{
		return false;
	}

	FMemory::Memcpy(&PerInstanceSMCustomData[InstanceIndex * NumCustomDataFloats], CustomDataFloats.GetData(),
					NumToCopy * CustomDataFloats.GetTypeSize());

	MarkAsModified();

	return true;
}

void UScavengersHubIsmComponent::MarkAsModified()
{
	// This code does a cheap read from the thread safe bool first and then only does the expensive atomic set if needed.
	//
	// AtomicSet returns the variable's previous value, so this branch will only be executed on the first thread to reach it.
	if (!WasModifiedThisFrame && !WasModifiedThisFrame.AtomicSet(true))
	{
		// Note that this is done even if bMarkRenderStateDirty is false, to ensure that the instance buffer is marked as being
		// edited.
		//
		// It's assumed that callers would only pass bMarkRenderStateDirty as false if they're doing a group of updates and plan
		// to explicitly mark the render state dirty later. There's no realistic use case for wanting the render state to stay
		// undirtied indefinitely after changes have been made.
		Async(EAsyncExecution::TaskGraphMainThread, [WeakThis = TWeakObjectPtr<UScavengersHubIsmComponent>(this)]() {
			if (WeakThis.IsValid())
			{
				// Unfortunately we can't call FInstanceUpdateCmdBuffer::Edit directly from here because it's not exported from
				// its owning module.
				//
				// However, PropagateLightingScenarioChange calls FInstanceUpdateCmdBuffer::Edit and MarkRenderStateDirty, so we
				// can call that instead.
				WeakThis->PropagateLightingScenarioChange();
				WeakThis->WasModifiedThisFrame = false;
			}
		});
	}
}
