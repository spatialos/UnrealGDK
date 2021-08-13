#include "ScavengersHubIsmActor.h"

#include "Async/Async.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "ScavengersHubGameFramework/ScavengersHubIsmComponent.h"

AScavengersHubIsmActor::AScavengersHubIsmActor()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	HierarchicalInstancedStaticMeshComponent =
		CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("HierarchicalInstancedStaticMeshComponent"));
	HierarchicalInstancedStaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HierarchicalInstancedStaticMeshComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	CustomInstancedStaticMeshComponent = CreateDefaultSubobject<UScavengersHubIsmComponent>(TEXT("CustomInstancedStaticMeshComponent"));
	CustomInstancedStaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CustomInstancedStaticMeshComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
}

void AScavengersHubIsmActor::BeginPlay()
{
	Super::BeginPlay();

	// If the custom ISM has been disallowed by live config, force bUseCustomInstancedStaticMeshComponent to false
	// if (!ULiveConfig::GetBool(TEXT("morpheus"), TEXT("AllowCustomInstancedStaticMeshComponent"), true))
	//{
	//    bUseCustomInstancedStaticMeshComponent = false;
	//}

	if (bUseCustomInstancedStaticMeshComponent)
	{
		UsedIsmComponent = CustomInstancedStaticMeshComponent;
		HierarchicalInstancedStaticMeshComponent->DestroyComponent();
	}
	else
	{
		UsedIsmComponent = HierarchicalInstancedStaticMeshComponent;
		CustomInstancedStaticMeshComponent->DestroyComponent();
	}
}

void AScavengersHubIsmActor::SetStaticMesh(UStaticMesh* Mesh)
{
	check(HasActorBegunPlay());
	UsedIsmComponent->SetStaticMesh(Mesh);
}

UStaticMesh* AScavengersHubIsmActor::GetStaticMesh()
{
	return UsedIsmComponent->GetStaticMesh();
}

void AScavengersHubIsmActor::SetEntityTransform(Worker_EntityId EntityId, TOptional<FTransform> Transform,
												const TArray<float>& CustomInstanceData, bool IsParallel)
{
	int32* EntityIndex = EntityIdToEntityIndex.Find(EntityId);

	// Skip if we have no ID and no transform
	if (EntityIndex == nullptr && !Transform.IsSet())
	{
		return;
	}

	// If the entity already has an instance and wants to update it, take this fast, thread-safe path.
	if (EntityIndex != nullptr && Transform.IsSet())
	{
		// Update the transform.
		UsedIsmComponent->UpdateInstanceTransform(*EntityIndex, Transform.GetValue(), false, true);

		// It would be neater to simply call SetCustomData without needing a conditional here, but the engine's built-in ISM
		// components crash if you call SetCustomData with a non-empty array when NumCustomDataFloats is zero.
		if (UsedIsmComponent->NumCustomDataFloats > 0)
		{
			UsedIsmComponent->SetCustomData(*EntityIndex, CustomInstanceData, true);
		}
		return;
	}

	// Non-thread-safe operations are needed, so if this is a background thread, queue this up to happen on the game thread
	if (bUseCustomInstancedStaticMeshComponent && IsParallel)
	{
		Async(
			// Note that a copy of CustomInstanceData is deliberately taken here, so that we don't rely on the reference staying
			// valid.
			EAsyncExecution::TaskGraphMainThread,
			[WeakThis = TWeakObjectPtr<AScavengersHubIsmActor>(this), EntityId, Transform, CustomInstanceData]() {
				if (WeakThis.IsValid())
				{
					WeakThis->SetEntityTransform(EntityId, Transform, CustomInstanceData, false);
				}
			});

		return;
	}

	check(IsInGameThread());

	const FTransform OffscreenTransform{ FQuat(0.f, 0.f, 0.f, 1.f), FVector(0.0f, 0.0f, -500000.0f), FVector(1.f) };

	if (EntityIndex != nullptr)
	{
		// This entity ID already has an allocated index.

		// Transform must not be set, otherwise the fast path would have been taken above
		check(!Transform);

		// Clear the transform and free the index.
		UsedIsmComponent->UpdateInstanceTransform(*EntityIndex, OffscreenTransform, false, true);
		EntityIdToEntityIndex.Remove(EntityId);
		FreeEntityIndices.Enqueue(*EntityIndex);
	}
	else
	{
		// This entity ID does not have an allocated index.
		if (Transform)
		{
			// Allocate an index and update the transform.
			int32 NewEntityIndex = 0;
			if (FreeEntityIndices.IsEmpty())
			{
				NewEntityIndex = NextEntityIndex++;
			}
			else
			{
				FreeEntityIndices.Dequeue(NewEntityIndex);
			}
			EntityIdToEntityIndex.Add(EntityId, NewEntityIndex);

			while (NewEntityIndex >= UsedIsmComponent->GetInstanceCount())
			{
				UsedIsmComponent->AddInstance(OffscreenTransform);
			}

			UsedIsmComponent->UpdateInstanceTransform(NewEntityIndex, Transform.GetValue(), false, true);
			if (UsedIsmComponent->NumCustomDataFloats > 0)
			{
				UsedIsmComponent->SetCustomData(NewEntityIndex, CustomInstanceData, true);
			}
		}
	}
}
