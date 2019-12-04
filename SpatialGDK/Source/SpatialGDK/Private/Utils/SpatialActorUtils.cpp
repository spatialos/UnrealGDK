// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/SpatialActorUtils.h"

#include "GameFramework/Actor.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "Interop/SpatialClassInfoManager.h"

namespace SpatialGDK
{

const TSet<FName> SpatialActorUtils::GetServerWorkerTypes()
{
	return GetDefault<USpatialGDKSettings>()->ServerWorkerTypes;
}

const WorkerRequirementSet SpatialActorUtils::GetAuthoritativeWorkerRequirementSet(const AActor& Actor)
{
	const UWorld* World = Actor.GetWorld();
	const USpatialNetDriver* NetDriver = Cast<USpatialNetDriver>(World->GetNetDriver());
	if (NetDriver)
	{
		const FClassInfo& Info = NetDriver->ClassInfoManager->GetOrCreateClassInfoByClass(Actor.GetClass());
		const WorkerAttributeSet WorkerAttribute{ Info.WorkerType.ToString() };

		return { WorkerAttribute };
	}

	return {};
}

const WorkerRequirementSet SpatialActorUtils::GetAnyServerRequirementSet()
{
	WorkerRequirementSet AnyServerRequirementSet;
	for (const FName& WorkerType : GetServerWorkerTypes())
	{
		AnyServerRequirementSet.Add({ WorkerType.ToString() });
	}

	return { AnyServerRequirementSet };
}

const WorkerRequirementSet SpatialActorUtils::GetAnyServerOrClientRequirementSet(const AActor& Actor)
{
	WorkerRequirementSet AnyServerOrClientRequirementSet = { SpatialConstants::UnrealClientAttributeSet };
	AnyServerOrClientRequirementSet.Append(GetAnyServerRequirementSet());

	return { AnyServerOrClientRequirementSet };
}

const WorkerRequirementSet SpatialActorUtils::GetAnyServerOrOwningClientRequirementSet(const AActor& Actor)
{
	WorkerRequirementSet AnyServerOrOwningClientRequirementSet = GetAnyServerRequirementSet();
	AnyServerOrOwningClientRequirementSet.Append(GetOwningClientOnlyRequirementSet(Actor));

	return { AnyServerOrOwningClientRequirementSet };
}

const WorkerRequirementSet SpatialActorUtils::GetOwningClientOnlyRequirementSet(const AActor& Actor)
{
	WorkerAttributeSet OwningClientAttributeSet = { GetOwnerWorkerAttribute(&Actor) };
	return { OwningClientAttributeSet };
}

const FString SpatialActorUtils::GetOwnerWorkerAttribute(const AActor* Actor)
{
	if (const USpatialNetConnection* NetConnection = Cast<USpatialNetConnection>(Actor->GetNetConnection()))
	{
		return NetConnection->WorkerAttribute;
	}

	return FString();
}

} // namespace SpatialGDK
