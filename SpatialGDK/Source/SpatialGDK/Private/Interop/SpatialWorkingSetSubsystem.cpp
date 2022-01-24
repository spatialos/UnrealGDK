#include "Interop/SpatialWorkingSetSubsystem.h"

#include "Algo/Transform.h"
#include "GameFramework/Actor.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/WorkingSetsHandler.h"
#include "SpatialView/ComponentData.h"
#include "SpatialView/ViewCoordinator.h"

DEFINE_LOG_CATEGORY_STATIC(LogSpatialActorSetSubsystem, Display, All);

namespace SpatialGDK
{
static FWorkingSetChangesHandler::FOnWorkingSetEvent ConvertBPDelegate(FOnWorkingSetEventDynamic BlueprintDelegate)
{
	if (BlueprintDelegate.IsBound())
	{
		return FWorkingSetChangesHandler::FOnWorkingSetEvent::CreateUFunction(BlueprintDelegate.GetUObject(),
																			  BlueprintDelegate.GetFunctionName());
	}
	return {};
}

static bool TryParseActorEntityId(const AActor* Actor, const USpatialPackageMapClient& PackageMap, const TCHAR* ActorInfo,
								  Worker_EntityId& OutEntityId)
{
	if (!IsValid(Actor))
	{
		UE_LOG(LogSpatialActorSetSubsystem, Warning, TEXT("Actor is invalid. %s"), ActorInfo);

		return false;
	}
	const Worker_EntityId ActorEntityId = PackageMap.GetEntityIdFromObject(Actor);
	if (ActorEntityId == SpatialConstants::INVALID_ENTITY_ID)
	{
		UE_LOG(LogSpatialActorSetSubsystem, Warning, TEXT("Actor doesn't have an entity ID. %s Actor: %s"), ActorInfo, *Actor->GetName());
		return false;
	}

	if (!Actor->IsActorReady())
	{
		UE_LOG(LogSpatialActorSetSubsystem, Warning, TEXT("Actor isn't ready. %s Actor: %s EntityId: %lld"), ActorInfo, *Actor->GetName(),
			   ActorEntityId);
		return false;
	}
	OutEntityId = ActorEntityId;
	return true;
}

static bool TryParseWorkingSetsEntityIds(TArray<AActor*> Members, AActor* Leader, const USpatialPackageMapClient& PackageMap,
										 SpatialGDK::FWorkingSetState& InOutRequest)
{
	bool bIsRequestValid = true;

	InOutRequest.MemberEntities.Empty(Members.Num());
	for (const AActor* Member : Members)
	{
		Worker_EntityId MemberEntityId;
		if (TryParseActorEntityId(Member, PackageMap, TEXT("Member"), MemberEntityId))
		{
			InOutRequest.MemberEntities.Emplace(MemberEntityId);
		}
		else
		{
			bIsRequestValid = false;
		}
	}

	if (!TryParseActorEntityId(Leader, PackageMap, TEXT("Leader"), InOutRequest.LeaderEntityId))
	{
		bIsRequestValid = false;
	}

	return bIsRequestValid;
}
} // namespace SpatialGDK

bool USpatialActorSetSubsystem::IsSetHandleValid(FActorSetHandle SetHandle, UObject* Context)
{
	USpatialNetDriver* NetDriver = Cast<USpatialNetDriver>(Context->GetWorld()->GetNetDriver());

	return SetHandle.ActorSetEntityHandle != SpatialConstants::INVALID_ENTITY_ID
		   && NetDriver->WorkingSetData->HasMarkerEntity(SetHandle.ActorSetEntityHandle);
}

FActorSetHandle USpatialActorSetSubsystem::CreateActorSet(AActor* Leader, const TArray<AActor*>& Actors, UObject* Context,
														  FOnWorkingSetEventDynamic OnSetCreated)
{
	using namespace SpatialGDK;

	USpatialNetDriver* NetDriver = Cast<USpatialNetDriver>(Context->GetWorld()->GetNetDriver());

	FWorkingSetState Request;
	Request.Epoch = 1;
	if (!TryParseWorkingSetsEntityIds(Actors, Leader, *NetDriver->PackageMap, Request))
	{
		return {};
	}

	FWorkingSetChangesHandler::FOnWorkingSetEvent OnSetCreatedHandler = ConvertBPDelegate(OnSetCreated);

	if (Algo::AllOf(Actors,
					[](const AActor* Actor) {
						return Actor->HasAuthority();
					})
		&& Leader->HasAuthority())
	{
		TSet<ActorLockToken> SetMembersTokens;
		for (AActor* Actor : Actors)
		{
			SetMembersTokens.Emplace(NetDriver->LockingPolicy->AcquireLock(Actor, TEXT("WorkingSet")));
		}
		SetMembersTokens.Emplace(NetDriver->LockingPolicy->AcquireLock(Leader, TEXT("WorkingSet")));

		OnSetCreatedHandler = FWorkingSetChangesHandler::FOnWorkingSetEvent::CreateLambda(
			[UserDelegate = MoveTemp(OnSetCreatedHandler), SetMembersTokens,
			 WeakNetDriver = MakeWeakObjectPtr(NetDriver)](bool bWasSuccessful) {
				if (WeakNetDriver.IsValid())
				{
					for (const ActorLockToken MemberLockToken : SetMembersTokens)
					{
						WeakNetDriver->LockingPolicy->ReleaseLock(MemberLockToken);
					}
				}

				UserDelegate.ExecuteIfBound(bWasSuccessful);
			});
	}

	const Worker_EntityId ActorSetEntityId = NetDriver->PackageMap->AllocateEntityId();

	NetDriver->WorkingSetChangesHandler->ApplyLocalWorkingSetCreationRequest(ActorSetEntityId, Request, OnSetCreatedHandler);

	return { ActorSetEntityId };
}

void USpatialActorSetSubsystem::ModifyActorSet(FActorSetHandle Handle, AActor* Leader, const TArray<AActor*>& Actors, UObject* Context,
											   FOnWorkingSetEventDynamic OnSetModified)
{
	using namespace SpatialGDK;

	USpatialNetDriver* NetDriver = Cast<USpatialNetDriver>(Context->GetWorld()->GetNetDriver());

	const FWorkingSetCommonData* ExistingWorkingSetData = NetDriver->WorkingSetData->GetWorkingSets().Find(Handle.ActorSetEntityHandle);

	if (ExistingWorkingSetData == nullptr)
	{
		UE_LOG(LogSpatialActorSetSubsystem, Warning, TEXT("Trying to modify unknown actor set. MarkerEntityId: %lld"),
			   Handle.ActorSetEntityHandle);
		return;
	}

	FWorkingSetMarkerRequest Request(ExistingWorkingSetData->RequestedState);
	++Request.RequestedState.Epoch;
	if (!TryParseWorkingSetsEntityIds(Actors, Leader, *NetDriver->PackageMap, Request.RequestedState))
	{
		return;
	}

	NetDriver->WorkingSetChangesHandler->ApplyLocalWorkingSetUpdateRequest(Handle.ActorSetEntityHandle, Request,
																		   ConvertBPDelegate(OnSetModified));
}

void USpatialActorSetSubsystem::DisbandActorSet(FActorSetHandle Handle, UObject* Context, FOnWorkingSetEventDynamic OnSetDisbanded)
{
	using namespace SpatialGDK;

	USpatialNetDriver* NetDriver = Cast<USpatialNetDriver>(Context->GetWorld()->GetNetDriver());

	ViewCoordinator& Coordinator = NetDriver->Connection->GetCoordinator();

	Coordinator.SendDeleteEntityRequest(Handle.ActorSetEntityHandle);
}

FActorSetHandle USpatialActorSetSubsystem::GetActorSetHandle(const AActor* Actor, UObject* Context)
{
	using namespace SpatialGDK;

	USpatialNetDriver* NetDriver = Cast<USpatialNetDriver>(Context->GetWorld()->GetNetDriver());

	const Worker_EntityId ActorEntityId = NetDriver->GetActorEntityId(*Actor);

	const Worker_EntityId* OwningEntityId = NetDriver->WorkingSetData->GetOwningMarkerEntityId(ActorEntityId);

	if (OwningEntityId != nullptr)
	{
		return FActorSetHandle{ *OwningEntityId };
	}
	return FActorSetHandle{ SpatialConstants::INVALID_ENTITY_ID };
}

void USpatialActorSetSubsystem::Advance(const SpatialGDK::FSubView& MarkerEntitiesSubview)
{
	using namespace SpatialGDK;
}
