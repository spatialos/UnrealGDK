// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialInterop.h"

#include "EntityRegistry.h"
#include "SpatialActorChannel.h"
#include "SpatialConstants.h"
#include "SpatialInteropPipelineBlock.h"
#include "SpatialNetConnection.h"
#include "SpatialNetDriver.h"
#include "SpatialOS.h"
#include "SpatialPackageMapClient.h"

// Needed for the entity template stuff.
#include "EntityBuilder.h"
#include <improbable/standard_library.h>
#include <improbable/unreal/gdk/player.h>
#include <improbable/unreal/gdk/unreal_metadata.h>
#include <improbable/unreal/gdk/global_state_manager.h>

DEFINE_LOG_CATEGORY(LogSpatialGDKInterop);

USpatialInterop::USpatialInterop()
{
}

void USpatialInterop::Init(USpatialOS* Instance, USpatialNetDriver* Driver, FTimerManager* InTimerManager)
{
	SpatialOSInstance = Instance;
	NetDriver = Driver;
	TimerManager = InTimerManager;
	PackageMap = Cast<USpatialPackageMapClient>(Driver->GetSpatialOSNetConnection()->PackageMap);
	bAuthoritativeDestruction = true;
	bCanSpawnReplicatedStablyNamedActors = false;

	// Collect all type binding classes.
	TArray<UClass*> TypeBindingClasses;
	for (TObjectIterator<UClass> It; It; ++It)
	{
		if (It->IsChildOf(USpatialTypeBinding::StaticClass()) && *It != USpatialTypeBinding::StaticClass())
		{
			TypeBindingClasses.Add(*It);
		}
	}

	// Register type binding classes.
	for (UClass* TypeBindingClass : TypeBindingClasses)
	{
		if (UClass* BoundClass = TypeBindingClass->GetDefaultObject<USpatialTypeBinding>()->GetBoundClass())
		{
			UE_LOG(LogSpatialGDKInterop, Log, TEXT("Registered type binding class %s handling replicated properties of %s."), *TypeBindingClass->GetName(), *BoundClass->GetName());
			RegisterInteropType(BoundClass, NewObject<USpatialTypeBinding>(this, TypeBindingClass));
		}
		else
		{
			UE_LOG(LogSpatialGDKInterop, Warning, TEXT("Could not find and register 'bound class' for type binding class %s. If this is a blueprint class, make sure it is referenced by the world."), *TypeBindingClass->GetName());
		}
	}

	TSharedPtr<worker::View> View = SpatialOSInstance->GetView().Pin();

	// Global State Manager setup
	View->OnAddComponent<improbable::unreal::GlobalStateManager>([this](const worker::AddComponentOp<improbable::unreal::GlobalStateManager>& op)
	{
		LinkExistingSingletonActors(op.Data.singleton_name_to_entity_id());
		DeleteIrrelevantReplicatedStablyNamedActors(op.Data.stably_named_path_to_entity_id());
	});

	View->OnComponentUpdate<improbable::unreal::GlobalStateManager>([this](const worker::ComponentUpdateOp<improbable::unreal::GlobalStateManager>& op)
	{
		if (op.Update.singleton_name_to_entity_id().data())
		{
			LinkExistingSingletonActors(*op.Update.singleton_name_to_entity_id().data());
		}
		if (op.Update.stably_named_path_to_entity_id().data())
		{
			DeleteIrrelevantReplicatedStablyNamedActors(*op.Update.stably_named_path_to_entity_id().data());
		}
	});

	View->OnAuthorityChange<improbable::unreal::GlobalStateManager>([this](const worker::AuthorityChangeOp& op)
	{
		if (op.Authority == worker::Authority::kAuthoritative)
		{
			ExecuteInitialSingletonActorReplication(*GetSingletonNameToEntityId());
			RegisterReplicatedStablyNamedActors();
		}
	});

	View->OnReserveEntityIdResponse([this](const worker::ReserveEntityIdResponseOp& Op)
	{
		if (USpatialActorChannel* Channel = RemovePendingActorRequest(Op.RequestId.Id))
		{
			Channel->OnReserveEntityIdResponse(Op);
		}
	});

	View->OnCreateEntityResponse([this](const worker::CreateEntityResponseOp& Op)
	{
		if (USpatialActorChannel* Channel = RemovePendingActorRequest(Op.RequestId.Id))
		{
			Channel->OnCreateEntityResponse(Op);
		}
	});
}

USpatialTypeBinding* USpatialInterop::GetTypeBindingByClass(UClass* Class) const
{
	for (const UClass* CurrentClass = Class; CurrentClass; CurrentClass = CurrentClass->GetSuperClass())
	{
		USpatialTypeBinding* const* BindingIterator = TypeBindings.Find(CurrentClass);
		if (BindingIterator)
		{
			return *BindingIterator;
		}
	}
	return nullptr;
}

worker::RequestId<worker::CreateEntityRequest> USpatialInterop::SendCreateEntityRequest(USpatialActorChannel* Channel, const FVector& Location, const FString& PlayerWorkerId, const TArray<uint16>& RepChanged, const TArray<uint16>& HandoverChanged)
{
	worker::RequestId<worker::CreateEntityRequest> CreateEntityRequestId;
	TSharedPtr<worker::Connection> PinnedConnection = SpatialOSInstance->GetConnection().Pin();
	if (PinnedConnection.IsValid())
	{
		AActor* Actor = Channel->Actor;
		const USpatialTypeBinding* TypeBinding = GetTypeBindingByClass(Actor->GetClass());

		FStringAssetReference ActorClassRef(Actor->GetClass());
		FString PathStr = ActorClassRef.ToString();

		worker::Entity Entity;

		if (TypeBinding)
		{
			Entity = TypeBinding->CreateActorEntity(PlayerWorkerId, Location, PathStr, Channel->GetChangeState(RepChanged, HandoverChanged), Channel);
		}
		else
		{
			std::string ClientWorkerIdString = TCHAR_TO_UTF8(*PlayerWorkerId);

			improbable::WorkerAttributeSet WorkerAttribute{{worker::List<std::string>{"UnrealWorker"}}};
			improbable::WorkerAttributeSet ClientAttribute{{worker::List<std::string>{"UnrealClient"}}};
			improbable::WorkerAttributeSet OwnClientAttribute{{"workerId:" + ClientWorkerIdString}};

			improbable::WorkerRequirementSet WorkersOnly{{WorkerAttribute}};
			improbable::WorkerRequirementSet ClientsOnly{{ClientAttribute}};
			improbable::WorkerRequirementSet OwnClientOnly{{OwnClientAttribute}};
			improbable::WorkerRequirementSet AnyUnrealWorkerOrClient{{WorkerAttribute, ClientAttribute}};

			// Set up unreal metadata.
			improbable::unreal::UnrealMetadata::Data UnrealMetadata;
			if (Channel->Actor->IsFullNameStableForNetworking())
			{
				UnrealMetadata.set_static_path({std::string{TCHAR_TO_UTF8(*Channel->Actor->GetPathName(Channel->Actor->GetWorld()))}});
			}
			if (!ClientWorkerIdString.empty())
			{
				UnrealMetadata.set_owner_worker_id({ClientWorkerIdString});
			}

			uint32 CurrentOffset = 0;
			worker::Map<std::string, std::uint32_t> SubobjectNameToOffset;
			ForEachObjectWithOuter(Channel->Actor, [&UnrealMetadata, &CurrentOffset, &SubobjectNameToOffset](UObject* Object)
			{
				// Objects can only be allocated NetGUIDs if this is true.
				if (Object->IsSupportedForNetworking() && !Object->IsPendingKill() && !Object->IsEditorOnly())
				{
					SubobjectNameToOffset.emplace(TCHAR_TO_UTF8(*(Object->GetName())), CurrentOffset);
					CurrentOffset++;
				}
			});
			UnrealMetadata.set_subobject_name_to_offset(SubobjectNameToOffset);

			// Build entity.
			const improbable::Coordinates SpatialPosition = SpatialConstants::LocationToSpatialOSCoordinates(Location);

			Entity = improbable::unreal::FEntityBuilder::Begin()
				.AddPositionComponent(SpatialPosition, WorkersOnly)
				.AddMetadataComponent(improbable::Metadata::Data{TCHAR_TO_UTF8(*PathStr)})
				.SetPersistence(true)
				.SetReadAcl(AnyUnrealWorkerOrClient)
				.AddComponent<improbable::unreal::UnrealMetadata>(UnrealMetadata, WorkersOnly)
				// For now, just a dummy component we add to every such entity to make sure client has write access to at least one component.
				// todo-giray: Remove once we're using proper (generated) entity templates here.
				.AddComponent<improbable::unreal::PlayerControlClient>(improbable::unreal::PlayerControlClient::Data{}, OwnClientOnly)
				.Build();
		}

		CreateEntityRequestId = PinnedConnection->SendCreateEntityRequest(Entity, Channel->GetEntityId().ToSpatialEntityId(), 0);
		AddPendingActorRequest(CreateEntityRequestId.Id, Channel);

		UE_LOG(LogSpatialGDKInterop, Log, TEXT("%s: Creating entity for actor %s (%lld) using initial changelist. Request ID: %d"),
			*SpatialOSInstance->GetWorkerId(), *Actor->GetName(), Channel->GetEntityId().ToSpatialEntityId(), CreateEntityRequestId.Id);
	}
	else
	{
		UE_LOG(LogSpatialGDKInterop, Warning, TEXT("Failed to obtain reference to SpatialOS connection!"));
	}

	return CreateEntityRequestId;
}

worker::RequestId<worker::ReserveEntityIdRequest> USpatialInterop::SendReserveEntityIdRequest(USpatialActorChannel* Channel)
{
	worker::RequestId<worker::ReserveEntityIdRequest> ReserveEntityIdRequest;
	TSharedPtr<worker::Connection> PinnedConnection = SpatialOSInstance->GetConnection().Pin();
	if (PinnedConnection.IsValid())
	{
		ReserveEntityIdRequest = PinnedConnection->SendReserveEntityIdRequest(0);
		AddPendingActorRequest(ReserveEntityIdRequest.Id, Channel);
		UE_LOG(LogSpatialGDKInterop, Log, TEXT("Opened channel for actor with no entity ID. Initiated reserve entity ID. Request id: %d"), ReserveEntityIdRequest.Id);
	}
	else
	{
		UE_LOG(LogSpatialGDKInterop, Warning, TEXT("Failed to obtain reference to SpatialOS connection!"));
	}
	return ReserveEntityIdRequest;
}

worker::RequestId<worker::DeleteEntityRequest> USpatialInterop::SendDeleteEntityRequest(const FEntityId& EntityId)
{
	worker::RequestId<worker::DeleteEntityRequest> DeleteEntityRequestId;
	TSharedPtr<worker::Connection> PinnedConnection = SpatialOSInstance->GetConnection().Pin();
	if (PinnedConnection.IsValid())
	{
		return PinnedConnection->SendDeleteEntityRequest(EntityId.ToSpatialEntityId(), 0);
	}

	return DeleteEntityRequestId;
}

void USpatialInterop::SendSpatialPositionUpdate(const FEntityId& EntityId, const FVector& Location)
{
	TSharedPtr<worker::Connection> PinnedConnection = SpatialOSInstance->GetConnection().Pin();
	if (!PinnedConnection.IsValid())
	{
		UE_LOG(LogSpatialGDKInterop, Warning, TEXT("Failed to obtain reference to SpatialOS connection!"));
	}
	improbable::Position::Update PositionUpdate;
	PositionUpdate.set_coords(SpatialConstants::LocationToSpatialOSCoordinates(Location));
	PinnedConnection->SendComponentUpdate<improbable::Position>(EntityId.ToSpatialEntityId(), PositionUpdate);
}

void USpatialInterop::SendSpatialUpdate(USpatialActorChannel* Channel, const TArray<uint16>& RepChanged, const TArray<uint16>& HandoverChanged)
{
	const USpatialTypeBinding* Binding = GetTypeBindingByClass(Channel->Actor->GetClass());
	if (!Binding)
	{
		//UE_LOG(LogSpatialGDKInterop, Warning, TEXT("SpatialUpdateInterop: Trying to send Spatial update on unsupported class %s."),
		//	*Channel->Actor->GetClass()->GetName());
		return;
	}
	Binding->SendComponentUpdates(Channel->GetChangeState(RepChanged, HandoverChanged), Channel, Channel->GetEntityId());
}

void USpatialInterop::SendSpatialUpdateSubobject(USpatialActorChannel* Channel, UObject* Subobject, FObjectReplicator* replicator, const TArray<uint16>& RepChanged, const TArray<uint16>& HandoverChanged)
{
	const USpatialTypeBinding* Binding = GetTypeBindingByClass(Subobject->GetClass());
	if (!Binding)
	{
		return;
	}
	Binding->SendComponentUpdates(Channel->GetChangeStateSubobject(Subobject, replicator, RepChanged, HandoverChanged), Channel, Channel->GetEntityId());
}

void USpatialInterop::InvokeRPC(UObject* TargetObject, const UFunction* const Function, void* Parameters)
{
	USpatialTypeBinding* Binding = GetTypeBindingByClass(TargetObject->GetClass());
	if (!Binding)
	{
		UE_LOG(LogSpatialGDKInterop, Warning, TEXT("SpatialUpdateInterop: Trying to send RPC on unsupported class %s."),
			*TargetObject->GetClass()->GetName());
		return;
	}

	Binding->SendRPCCommand(TargetObject, Function, Parameters);
}

void USpatialInterop::ReceiveAddComponent(USpatialActorChannel* Channel, UAddComponentOpWrapperBase* AddComponentOp)
{
	const USpatialTypeBinding* Binding = GetTypeBindingByClass(Channel->Actor->GetClass());
	if (!Binding)
	{
		return;
	}
	Binding->ReceiveAddComponent(Channel, AddComponentOp);
}

void USpatialInterop::ResolvePendingOperations(UObject* Object, const improbable::unreal::UnrealObjectRef& ObjectRef)
{
	if (NetDriver->InteropPipelineBlock->IsInCriticalSection())
	{
		ResolvedObjectQueue.Add(TPair<UObject*, const improbable::unreal::UnrealObjectRef>{ Object, ObjectRef });
		return;
	}

	ResolvePendingOperations_Internal(Object, ObjectRef);
}

void USpatialInterop::OnLeaveCriticalSection()
{
	for (auto& It : ResolvedObjectQueue)
	{
		ResolvePendingOperations_Internal(It.Key, It.Value);
	}

	ResolvedObjectQueue.Empty();
}

void USpatialInterop::ResolvePendingOperations_Internal(UObject* Object, const improbable::unreal::UnrealObjectRef& ObjectRef)
{
	UE_LOG(LogSpatialGDKInterop, Log, TEXT("Resolving pending object refs and RPCs which depend on object: %s %s."), *Object->GetName(), *ObjectRefToString(ObjectRef));
	ResolvePendingOutgoingObjectUpdates(Object);
	ResolvePendingOutgoingRPCs(Object);
	ResolvePendingOutgoingArrayUpdates(Object);
	ResolvePendingIncomingObjectUpdates(Object, ObjectRef);
	ResolvePendingIncomingRPCs(ObjectRef);
}

void USpatialInterop::AddActorChannel(const FEntityId& EntityId, USpatialActorChannel* Channel)
{
	EntityToActorChannel.Add(EntityId, Channel);
}

void USpatialInterop::RemoveActorChannel(const FEntityId& EntityId)
{
	if (EntityToActorChannel.Find(EntityId) == nullptr)
	{
		UE_LOG(LogSpatialGDKInterop, Warning, TEXT("Failed to find entity/channel mapping for %s."), *ToString(EntityId.ToSpatialEntityId()));
		return;
	}

	USpatialActorChannel* Channel = EntityToActorChannel.FindAndRemoveChecked(EntityId);

	for (auto& Pair : PendingOutgoingObjectUpdates)
	{
		Pair.Value.Remove(Channel);
	}

	for (auto& Pair : PendingIncomingObjectUpdates)
	{
		Pair.Value.Remove(Channel);
	}

	PropertyToOPAR.Remove(Channel);
	for (auto& Pair : ObjectToOPAR)
	{
		Pair.Value.Remove(Channel);
	}
}

void USpatialInterop::DeleteEntity(const FEntityId& EntityId)
{
	SendDeleteEntityRequest(EntityId);
	NetDriver->InteropPipelineBlock->CleanupDeletedEntity(EntityId);
}

void USpatialInterop::SendComponentInterests(USpatialActorChannel* ActorChannel, const FEntityId& EntityId)
{
	UClass* ActorClass = ActorChannel->Actor->GetClass();

	const USpatialTypeBinding* Binding = GetTypeBindingByClass(ActorClass);
	if (Binding)
	{
		auto Interest = Binding->GetInterestOverrideMap(NetDriver->GetNetMode() == NM_Client, ActorChannel->GetActor()->GetNetConnection() != nullptr);

		SpatialOSInstance->GetConnection().Pin()->SendComponentInterest(EntityId.ToSpatialEntityId(), Interest);
	}
}

USpatialActorChannel* USpatialInterop::GetActorChannelByEntityId(const FEntityId& EntityId) const
{
	// Get actor channel.
	USpatialActorChannel* const* ActorChannelIt = EntityToActorChannel.Find(EntityId);
	if (!ActorChannelIt)
	{
		// Can't find actor channel for this entity, give up.
		return nullptr;
	}
	return *ActorChannelIt;
}

void USpatialInterop::InvokeRPCSendHandler_Internal(FRPCCommandRequestFunc Function, bool bReliable)
{
	// Attempt to trigger command request by calling the passed RPC request function. This function is generated in the type binding
	// classes which capture the target actor and arguments of the RPC (unpacked from the FFrame) by value, and attempts to serialize
	// the RPC arguments to a command request.
	//
	// The generated function tries to first resolve the target actor using the package map, then any UObject arguments. If any of these
	// fail, `Function()` will return with Result.UnresolvedObject being set to the object which was unable to be resolved by the package
	// map. We then queue this RPC until that object becomes resolved. If all objects are resolved successfully, we instead get back a
	// RequestId returned from Connection->SendCommandRequest<...>(...).
	auto Result = Function();
	if (Result.UnresolvedObject != nullptr)
	{
		// Add to pending RPCs if any actors were unresolved.
		QueueOutgoingRPC_Internal(Result.UnresolvedObject, Function, bReliable);
	}
	else
	{
		// Add to outgoing RPCs if reliable. Otherwise, do nothing, as we don't bother retrying if unreliable.
		if (bReliable)
		{
			OutgoingReliableRPCs.Emplace(Result.RequestId, TSharedPtr<FOutgoingReliableRPC>(new FOutgoingReliableRPC(Function)));
		}
	}
}

void USpatialInterop::InvokeRPCReceiveHandler_Internal(FRPCCommandResponseFunc Function)
{
	auto Result = Function();
	if (Result.IsSet())
	{
		QueueIncomingRPC_Internal(Result.GetValue(), Function);
	}
}

void USpatialInterop::HandleCommandResponse_Internal(const FString& RPCName, FUntypedRequestId RequestId, const FEntityId&, const worker::StatusCode& StatusCode, const FString& Message)
{
	TSharedPtr<FOutgoingReliableRPC>* RequestContextIterator = OutgoingReliableRPCs.Find(RequestId);
	if (!RequestContextIterator)
	{
		// We received a response for an unreliable RPC, ignore.
		return;
	}

	TSharedPtr<FOutgoingReliableRPC> RetryContext = *RequestContextIterator;
	OutgoingReliableRPCs.Remove(RequestId);
	if (StatusCode != worker::StatusCode::kSuccess)
	{
		if (RetryContext->NumAttempts < SpatialConstants::MAX_NUMBER_COMMAND_ATTEMPTS)
		{
			float WaitTime = SpatialConstants::GetCommandRetryWaitTimeSeconds(RetryContext->NumAttempts);
			UE_LOG(LogSpatialGDKInterop, Log, TEXT("%s: retrying in %f seconds. Error code: %d Message: %s"), *RPCName, WaitTime, (int)StatusCode, *Message);

			// Queue retry.
			FTimerHandle RetryTimer;
			FTimerDelegate TimerCallback;
			TimerCallback.BindLambda([this, RetryContext]() {
				auto Result = RetryContext->SendCommandRequest();
				RetryContext->NumAttempts++;
				// As it's possible for an entity to leave a workers checkout radius (and thus become unresolved again), we cannot
				// take for granted that retrying the command will mean that the object is always resolved. In that case, just
				// give up and drop the command as it doesn't make sense to retry it anymore.
				if (Result.UnresolvedObject == nullptr)
				{
					OutgoingReliableRPCs.Emplace(Result.RequestId, RetryContext);
				}
			});
			TimerManager->SetTimer(RetryTimer, TimerCallback, WaitTime, false);
		}
		else
		{
			UE_LOG(LogSpatialGDKInterop, Error, TEXT("%s: failed too many times, giving up (%u attempts). Error code: %d Message: %s"), *RPCName, SpatialConstants::MAX_NUMBER_COMMAND_ATTEMPTS, (int)StatusCode, *Message);
		}
	}
}

void USpatialInterop::QueueOutgoingObjectRepUpdate_Internal(const UObject* UnresolvedObject, USpatialActorChannel* DependentChannel, uint16 Handle)
{
	check(UnresolvedObject);
	check(DependentChannel);
	check(Handle > 0);

	UE_LOG(LogSpatialOSPackageMap, Log, TEXT("Added pending outgoing object ref depending on object: %s, channel: %s, handle: %d."), *UnresolvedObject->GetName(), *DependentChannel->GetName(), Handle);
	PendingOutgoingObjectUpdates.FindOrAdd(UnresolvedObject).FindOrAdd(DependentChannel).Key.Add(Handle);
}

void USpatialInterop::QueueOutgoingObjectHandoverUpdate_Internal(const UObject* UnresolvedObject, USpatialActorChannel* DependentChannel, uint16 Handle)
{
	check(UnresolvedObject);
	check(DependentChannel);
	check(Handle > 0);

	UE_LOG(LogSpatialOSPackageMap, Log, TEXT("Added pending outgoing object ref depending on object: %s, channel: %s, handle: %d."),
		*UnresolvedObject->GetName(), *DependentChannel->GetName(), Handle);
	PendingOutgoingObjectUpdates.FindOrAdd(UnresolvedObject).FindOrAdd(DependentChannel).Value.Add(Handle);
}

void USpatialInterop::QueueOutgoingRPC_Internal(const UObject* UnresolvedObject, FRPCCommandRequestFunc CommandSender, bool bReliable)
{
	check(UnresolvedObject);
	UE_LOG(LogSpatialOSPackageMap, Log, TEXT("Added pending outgoing RPC depending on object: %s."), *UnresolvedObject->GetName());
	PendingOutgoingRPCs.FindOrAdd(UnresolvedObject).Add(TPair<FRPCCommandRequestFunc, bool>{CommandSender, bReliable});
}

void USpatialInterop::QueueIncomingObjectRepUpdate_Internal(const improbable::unreal::UnrealObjectRef& UnresolvedObjectRef, USpatialActorChannel* DependentChannel, UObject* TargetObject, const FRepHandleData* RepHandleData)
{
	check(DependentChannel);
	check(RepHandleData);
	UE_LOG(LogSpatialOSPackageMap, Log, TEXT("Added pending incoming object ref: %s, channel: %s, target object: %s, property: %s. Setting TargetObject->Property = ObjectRef."),
		*ObjectRefToString(UnresolvedObjectRef), *DependentChannel->GetName(), *TargetObject->GetName(), *RepHandleData->Property->GetName());
	PendingIncomingObjectUpdates.FindOrAdd(UnresolvedObjectRef).FindOrAdd(DependentChannel).Key.FindOrAdd(TargetObject).Add(RepHandleData);
}

void USpatialInterop::QueueIncomingObjectHandoverUpdate_Internal(const improbable::unreal::UnrealObjectRef& UnresolvedObjectRef, USpatialActorChannel* DependentChannel, const FHandoverHandleData* RepHandleData)
{
	check(DependentChannel);
	check(RepHandleData);
	UE_LOG(LogSpatialOSPackageMap, Log, TEXT("Added pending incoming object ref depending on object ref: %s, channel: %s, property: %s."),
		*ObjectRefToString(UnresolvedObjectRef), *DependentChannel->GetName(), *RepHandleData->Property->GetName());
	PendingIncomingObjectUpdates.FindOrAdd(UnresolvedObjectRef).FindOrAdd(DependentChannel).Value.Add(RepHandleData);
}

void USpatialInterop::QueueIncomingRPC_Internal(const improbable::unreal::UnrealObjectRef& UnresolvedObjectRef, FRPCCommandResponseFunc Responder)
{
	UE_LOG(LogSpatialOSPackageMap, Log, TEXT("Added pending incoming RPC depending on object ref: %s."), *ObjectRefToString(UnresolvedObjectRef));
	PendingIncomingRPCs.FindOrAdd(UnresolvedObjectRef).Add(Responder);
}

void USpatialInterop::ResetOutgoingArrayRepUpdate_Internal(USpatialActorChannel* DependentChannel, uint16 Handle)
{
	// This is called when trying to send an update on a given property on a given USpatialActorChannel. In case there
	// was a pending outgoing update queued up before, it will now be obsolete, e.g. it's possible that unresolved
	// objects that were queued up are no longer in the array, or there may be new unresolved objects present.

	check(DependentChannel);

	FHandleToOPARMap* HandleToOPARMap = PropertyToOPAR.Find(DependentChannel);
	if (HandleToOPARMap == nullptr)
	{
		return;
	}

	TSharedPtr<FOutgoingPendingArrayRegister>* OPARPtr = HandleToOPARMap->Find(Handle);
	if (OPARPtr == nullptr)
	{
		return;
	}

	TSharedPtr<FOutgoingPendingArrayRegister>& OPAR = *OPARPtr;

	check(OPAR.IsValid());

	UE_LOG(LogSpatialOSPackageMap, Log, TEXT("Resetting pending outgoing array depending on channel: %s, handle: %d."), *DependentChannel->GetName(), Handle);

	// Remove the references from indiviual unresolved objects to this OPAR.
	for (const UObject* UnresolvedObject : OPAR->UnresolvedObjects)
	{
		FChannelToHandleToOPARMap& ChannelToHandleToOPARMap = ObjectToOPAR.FindChecked(UnresolvedObject);
		FHandleToOPARMap& RelevantHandles = ChannelToHandleToOPARMap.FindChecked(DependentChannel);

		RelevantHandles.Remove(Handle);
		if (RelevantHandles.Num() == 0)
		{
			ChannelToHandleToOPARMap.Remove(DependentChannel);
			if (ChannelToHandleToOPARMap.Num() == 0)
			{
				ObjectToOPAR.Remove(UnresolvedObject);
			}
		}
	}

	// Remove the reference from the property's actor channel and handle to this OPAR.
	HandleToOPARMap->Remove(Handle);
	if (HandleToOPARMap->Num() == 0)
	{
		PropertyToOPAR.Remove(DependentChannel);
	}
}

void USpatialInterop::QueueOutgoingArrayRepUpdate_Internal(const TSet<const UObject*>& UnresolvedObjects, USpatialActorChannel* DependentChannel, uint16 Handle)
{
	check(DependentChannel);

	UE_LOG(LogSpatialOSPackageMap, Log, TEXT("Added pending outgoing array: channel: %s, handle: %d. Depending on objects:"),
		*DependentChannel->GetName(), Handle);

	TSharedPtr<FOutgoingPendingArrayRegister> OPAR = MakeShared<FOutgoingPendingArrayRegister>();
	OPAR->UnresolvedObjects = UnresolvedObjects;

	// Add reference from the property's actor channel and handle to the new OPAR.
	FHandleToOPARMap& HandleToOPARMap = PropertyToOPAR.FindOrAdd(DependentChannel);
	// If there was an array queued for the same channel and handle before, it should have been removed in ResetOutgoingArrayRepUpdate_Internal.
	check(HandleToOPARMap.Find(Handle) == nullptr);
	HandleToOPARMap.Add(Handle, OPAR);

	// Add references from individual unresolved objects to this OPAR.
	for (const UObject* UnresolvedObject : UnresolvedObjects)
	{
		FHandleToOPARMap& AnotherHandleToOPARMap = ObjectToOPAR.FindOrAdd(UnresolvedObject).FindOrAdd(DependentChannel);
		check(AnotherHandleToOPARMap.Find(Handle) == nullptr);
		AnotherHandleToOPARMap.Add(Handle, OPAR);

		// Following up on the previous log: listing the unresolved objects
		UE_LOG(LogSpatialOSPackageMap, Log, TEXT("%s"), *UnresolvedObject->GetName());
	}
}

void USpatialInterop::RegisterInteropType(UClass* Class, USpatialTypeBinding* Binding)
{
	Binding->Init(this, PackageMap);
	Binding->BindToView(NetDriver->GetNetMode() == NM_Client);
	TypeBindings.Add(Class, Binding);
}

void USpatialInterop::UnregisterInteropType(UClass* Class)
{
	USpatialTypeBinding** BindingIterator = TypeBindings.Find(Class);
	if (BindingIterator != nullptr)
	{
		USpatialTypeBinding* Binding = *BindingIterator;
		Binding->UnbindFromView();
		TypeBindings.Remove(Class);
	}
}

void USpatialInterop::ResolvePendingOutgoingObjectUpdates(UObject* Object)
{
	auto* DependentChannels = PendingOutgoingObjectUpdates.Find(Object);
	if (DependentChannels == nullptr)
	{
		return;
	}

	for (auto& ChannelProperties : *DependentChannels)
	{
		USpatialActorChannel* DependentChannel = ChannelProperties.Key;
		FPendingOutgoingProperties& Properties = ChannelProperties.Value;
		if (Properties.Key.Num() > 0)
		{
			// Replication change lists always have a 0 at the end.
			Properties.Key.Add(0);
		}

		// This function will only send updates if any of the property lists are non-empty.
		SendSpatialUpdate(DependentChannel, Properties.Key, Properties.Value);
	}
	PendingOutgoingObjectUpdates.Remove(Object);
}

void USpatialInterop::ResolvePendingOutgoingRPCs(UObject* Object)
{
	auto* RPCList = PendingOutgoingRPCs.Find(Object);
	if (RPCList)
	{
		for (auto& RequestFuncPair : *RPCList)
		{
			// We can guarantee that SendCommandRequest won't populate PendingOutgoingRPCs[Object] whilst we're iterating through it,
			// because Object has been resolved when we call ResolvePendingOutgoingRPCs.
			InvokeRPCSendHandler_Internal(RequestFuncPair.Key, RequestFuncPair.Value);
		}
		PendingOutgoingRPCs.Remove(Object);
	}
}

void USpatialInterop::ResolvePendingIncomingObjectUpdates(UObject* IncomingObject, const improbable::unreal::UnrealObjectRef& ObjectRef)
{
	auto* DependentChannels = PendingIncomingObjectUpdates.Find(ObjectRef);
	if (DependentChannels == nullptr)
	{
		return;
	}

	for (auto& ChannelProperties : *DependentChannels)
	{
		USpatialActorChannel* DependentChannel = ChannelProperties.Key;
		FPendingIncomingProperties& Properties = ChannelProperties.Value;

		// Replicated Properties
		TSet<UProperty*> RepNotifies;
		for (TPair<UObject*, TArray<const FRepHandleData*>>& ObjectsRepPropertyUpdate : Properties.Key)
		{
			// Iterate through our list of target objects, either an actor or component
			// For each, we resolve all replicated properties under that object
			UObject* TargetObject = ObjectsRepPropertyUpdate.Key;
			DependentChannel->PreReceiveSpatialUpdate(TargetObject);

			for (const FRepHandleData* RepData : ObjectsRepPropertyUpdate.Value)
			{
				ApplyIncomingReplicatedPropertyUpdate(*RepData, TargetObject, &IncomingObject, RepNotifies);
				UE_LOG(LogSpatialGDKInterop, Log, TEXT("%s: Received queued object replicated property update. Channel actor %s (%lld), target object %s, property %s"),
					*SpatialOSInstance->GetWorkerId(),
					*DependentChannel->Actor->GetName(),
					DependentChannel->GetEntityId().ToSpatialEntityId(),
					*TargetObject->GetName(),
					*RepData->Property->GetName());
			}

			DependentChannel->PostReceiveSpatialUpdate(TargetObject, RepNotifies.Array());
		}

		// Handover
		for (const FHandoverHandleData* HandoverData : Properties.Value)
		{
			ApplyIncomingHandoverPropertyUpdate(*HandoverData, DependentChannel->Actor, &IncomingObject);  // HandoverHandleData will find the correct TargetObject for our replicated property, so we can just pass in the owning actor (channel actor)
			UE_LOG(LogSpatialGDKInterop, Log, TEXT("%s: Received queued object handover property update. Channel actor %s (%lld), property %s"),
				*SpatialOSInstance->GetWorkerId(),
				*DependentChannel->Actor->GetName(),
				DependentChannel->GetEntityId().ToSpatialEntityId(),
				*HandoverData->Property->GetName());
		}
	}

	PendingIncomingObjectUpdates.Remove(ObjectRef);
}

void USpatialInterop::ResolvePendingIncomingRPCs(const improbable::unreal::UnrealObjectRef& ObjectRef)
{
	auto* RPCList = PendingIncomingRPCs.Find(ObjectRef);
	if (RPCList)
	{
		for (auto& Responder : *RPCList)
		{
			// We can guarantee that SendCommandResponse won't populate PendingIncomingRPCs[ObjectRef] whilst we're iterating through it,
			// because ObjectRef has been resolved when we call ResolvePendingIncomingRPCs.
			InvokeRPCReceiveHandler_Internal(Responder);
		}
		PendingIncomingRPCs.Remove(ObjectRef);
	}
}

void USpatialInterop::AddPendingActorRequest(FUntypedRequestId RequestId, USpatialActorChannel* Channel)
{
	PendingActorRequests.Emplace(RequestId, Channel);
}

USpatialActorChannel* USpatialInterop::RemovePendingActorRequest(FUntypedRequestId RequestId)
{
	USpatialActorChannel** Channel = PendingActorRequests.Find(RequestId);
	if (Channel == nullptr)
	{
		return nullptr;
	}
	PendingActorRequests.Remove(RequestId);
	return *Channel;
}

void USpatialInterop::ResolvePendingOutgoingArrayUpdates(UObject* Object)
{
	FChannelToHandleToOPARMap* ChannelToHandleToOPARMap = ObjectToOPAR.Find(Object);
	if (ChannelToHandleToOPARMap == nullptr)
	{
		return;
	}

	for (auto& ChannelProperties : *ChannelToHandleToOPARMap)
	{
		USpatialActorChannel* DependentChannel = ChannelProperties.Key;
		FHandleToOPARMap& HandleToOPARMap = ChannelProperties.Value;

		TArray<uint16> Properties;

		for (auto& HandleOPARPair : HandleToOPARMap)
		{
			uint16 Handle = HandleOPARPair.Key;
			TSharedPtr<FOutgoingPendingArrayRegister>& OPAR = HandleOPARPair.Value;

			// This object is no longer unresolved. If this was the last one, we can now send this array and remove the OPAR.
			OPAR->UnresolvedObjects.Remove(Object);
			if (OPAR->UnresolvedObjects.Num() == 0)
			{
				Properties.Add(Handle);
				// We don't need to add handles for the array elements, but we need to put 0 for the number of array element handles, as well as null terminator for the array.
				Properties.Add(0);
				Properties.Add(0);

				// Remove the reference from the property's channel and handle to this OPAR.
				FHandleToOPARMap& AnotherHandleToOPARMap = PropertyToOPAR.FindChecked(DependentChannel);
				AnotherHandleToOPARMap.Remove(Handle);
				if (AnotherHandleToOPARMap.Num() == 0)
				{
					PropertyToOPAR.Remove(DependentChannel);
				}
			}
		}

		if (Properties.Num() > 0)
		{
			// End with zero to indicate the end of the list of handles.
			Properties.Add(0);

			SendSpatialUpdate(DependentChannel, Properties, TArray<uint16>());
		}
	}

	// For any array that was resolved in this function, this will remove the last instances of the shared ptrs of
	// the OPARs, which will destroy them.
	ObjectToOPAR.Remove(Object);
}

void USpatialInterop::LinkExistingSingletonActors(const PathNameToEntityIdMap& SingletonNameToEntityId)
{
	if (!NetDriver->IsServer())
	{
		return;
	}

	for (const auto& Pair : SingletonNameToEntityId)
	{
		FEntityId SingletonEntityId{ Pair.second };

		// Singleton Entity hasn't been created yet
		if (SingletonEntityId == FEntityId{})
		{
			continue;
		}

		AActor* SingletonActor = nullptr;
		USpatialActorChannel* Channel = nullptr;
		GetSingletonActorAndChannel(UTF8_TO_TCHAR(Pair.first.c_str()), SingletonActor, Channel);

		// Singleton wasn't found or channel is already set up
		if (Channel == nullptr || Channel->Actor != nullptr)
		{
			continue;
		}

		// Add to entity registry
		// This indirectly causes SetChannelActor to not create a new entity for this actor
		NetDriver->GetEntityRegistry()->AddToRegistry(SingletonEntityId, SingletonActor);

		Channel->SetChannelActor(SingletonActor);

		TSharedPtr<worker::View> LockedView = NetDriver->GetSpatialOS()->GetView().Pin();

		// Couldn't get View
		if (LockedView.IsValid())
		{
			continue;
		}

		auto EntityIterator = LockedView->Entities.find(SingletonEntityId.ToSpatialEntityId());
		if (EntityIterator == LockedView->Entities.end())
		{
			// Don't have entity checked out
			continue;
		}

		improbable::unreal::UnrealMetadataData* Metadata = EntityIterator->second.Get<improbable::unreal::UnrealMetadata>().data();
		USpatialPackageMapClient* PackageMap = Cast<USpatialPackageMapClient>(NetDriver->GetSpatialOSNetConnection()->PackageMap);

		// Since the entity already exists, we have to handle setting up the PackageMap properly for this Actor
		PackageMap->ResolveEntityActor(SingletonActor, SingletonEntityId, Metadata->subobject_name_to_offset());
		UE_LOG(LogSpatialGDKInterop, Log, TEXT("Linked Singleton Actor %s with id %d"), *SingletonActor->GetClass()->GetName(), SingletonEntityId.ToSpatialEntityId());
	}
}

void USpatialInterop::ExecuteInitialSingletonActorReplication(const PathNameToEntityIdMap& SingletonNameToEntityId)
{
	for (const auto& pair : SingletonNameToEntityId)
	{
		FEntityId SingletonEntityId{ pair.second };

		// Entity has already been created on another server
		if (SingletonEntityId != FEntityId{})
		{
			continue;
		}

		AActor* SingletonActor = nullptr;
		USpatialActorChannel* Channel = nullptr;
		GetSingletonActorAndChannel(UTF8_TO_TCHAR(pair.first.c_str()), SingletonActor, Channel);

		// Class couldn't be found
		if (Channel == nullptr)
		{
			continue;
		}

		// Set entity id of channel from the GlobalStateManager.
		// If the id was 0, SetChannelActor will create the entity.
		// If the id is not 0, it will start replicating to that entity.
		Channel->SetChannelActor(SingletonActor);

		UE_LOG(LogSpatialGDKInterop, Log, TEXT("Started replication of Singleton Actor %s"), *SingletonActor->GetClass()->GetName());
	}
}

void USpatialInterop::GetSingletonActorAndChannel(FString ClassName, AActor*& OutActor, USpatialActorChannel*& OutChannel)
{
	OutActor = nullptr;
	OutChannel = nullptr;

	UClass* SingletonActorClass = FindObject<UClass>(ANY_PACKAGE, *ClassName);

	if (SingletonActorClass == nullptr)
	{
		UE_LOG(LogSpatialGDKInterop, Error, TEXT("Failed to find Singleton Actor Class."));
		return;
	}

	if(TPair<AActor*, USpatialActorChannel*>* Pair = NetDriver->SingletonActorChannels.Find(SingletonActorClass))
	{
		OutActor = Pair->Key;
		OutChannel = Pair->Value;
		return;
	}

	// Class doesn't exist in our map, have to find actor and create channel
	// Get Singleton Actor in world
	TArray<AActor*> SingletonActorList;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), SingletonActorClass, SingletonActorList);

	if (SingletonActorList.Num() > 1)
	{
		UE_LOG(LogSpatialGDKInterop, Error, TEXT("More than one Singleton Actor exists of type %s"), *ClassName);
		return;
	}

	OutActor = SingletonActorList[0];

	USpatialNetConnection* Connection = Cast<USpatialNetConnection>(NetDriver->ClientConnections[0]);

	OutChannel = (USpatialActorChannel*)Connection->CreateChannel(CHTYPE_Actor, 1);
	NetDriver->SingletonActorChannels.Add(SingletonActorClass, TPair<AActor*, USpatialActorChannel*>(OutActor, OutChannel));
}

void USpatialInterop::UpdateSingletonId(const FString& ClassName, const FEntityId& SingletonEntityId)
{
	std::string SingletonName(TCHAR_TO_UTF8(*ClassName));
	PathNameToEntityIdMap& SingletonNameToEntityId = *GetSingletonNameToEntityId();
	SingletonNameToEntityId[SingletonName] = SingletonEntityId.ToSpatialEntityId();

	improbable::unreal::GlobalStateManager::Update Update;
	Update.set_singleton_name_to_entity_id(SingletonNameToEntityId);

	TSharedPtr<worker::Connection> Connection = SpatialOSInstance->GetConnection().Pin();
	Connection->SendComponentUpdate<improbable::unreal::GlobalStateManager>(worker::EntityId((long)SpatialConstants::GLOBAL_STATE_MANAGER), Update);
}

bool USpatialInterop::IsSingletonClass(UClass* Class)
{
	return Class->HasAnySpatialClassFlags(SPATIALCLASS_Singleton);
}

improbable::unreal::GlobalStateManagerData* USpatialInterop::GetGlobalStateManagerData() const
{
	TSharedPtr<worker::View> View = SpatialOSInstance->GetView().Pin();
	auto It = View->Entities.find(SpatialConstants::GLOBAL_STATE_MANAGER);

	if (It != View->Entities.end())
	{
		improbable::unreal::GlobalStateManagerData* GSM = It->second.Get<improbable::unreal::GlobalStateManager>().data();
		return GSM;
	}

	return nullptr;
}

PathNameToEntityIdMap* USpatialInterop::GetSingletonNameToEntityId() const
{
	if (improbable::unreal::GlobalStateManagerData* GSMD = GetGlobalStateManagerData())
	{
		return &(GSMD->singleton_name_to_entity_id());
	}

	return nullptr;
}

PathNameToEntityIdMap* USpatialInterop::GetStablyNamedPathToEntityId() const
{
	if (improbable::unreal::GlobalStateManagerData* GSMD = GetGlobalStateManagerData())
	{
		return &(GSMD->stably_named_path_to_entity_id());
	}

	return nullptr;
}

void USpatialInterop::ReserveReplicatedStablyNamedActorChannel(USpatialActorChannel* Channel)
{
	ReplicatedStablyNamedActorQueue.Emplace(Channel->Actor, Channel);

	if (CanSpawnReplicatedStablyNamedActors())
	{
		RegisterReplicatedStablyNamedActors();
	}
}

void USpatialInterop::UnreserveReplicatedStablyNamedActor(AActor* Actor)
{
	ReplicatedStablyNamedActorQueue.Remove(Actor);
}

void USpatialInterop::AddReplicatedStablyNamedActorToGSM(const FEntityId& EntityId, AActor* Actor)
{
	PathNameToEntityIdMap* StablyNamedPathToEntityId = GetStablyNamedPathToEntityId();

	if (StablyNamedPathToEntityId == nullptr)
	{
		return;
	}

	std::string Path = TCHAR_TO_UTF8(*Actor->GetPathName(Actor->GetWorld()));

	// If the map already has the entity id, meaning the actor was already spawned, return early
	if (StablyNamedPathToEntityId->count(Path))
	{
		return;
	}

	StablyNamedPathToEntityId->emplace(Path, EntityId.ToSpatialEntityId());

	improbable::unreal::GlobalStateManager::Update Update;
	Update.set_stably_named_path_to_entity_id(*StablyNamedPathToEntityId);

	TSharedPtr<worker::Connection> Connection = SpatialOSInstance->GetConnection().Pin();
	Connection->SendComponentUpdate<improbable::unreal::GlobalStateManager>(worker::EntityId((long)SpatialConstants::GLOBAL_STATE_MANAGER), Update);
}

void USpatialInterop::RegisterReplicatedStablyNamedActors()
{
	bCanSpawnReplicatedStablyNamedActors = true;
	PathNameToEntityIdMap& StablyNamedPathToEntityId = *GetStablyNamedPathToEntityId();

	for (const auto& Pair : ReplicatedStablyNamedActorQueue)
	{
		USpatialActorChannel* Channel = Pair.Value;
		AActor* Actor = Pair.Key;
		std::string Path = TCHAR_TO_UTF8(*Actor->GetPathName(Actor->GetWorld()));

		// If the Actor was already spawned, just register the EntityId
		if (StablyNamedPathToEntityId.count(Path))
		{
			Channel->RegisterEntityId(StablyNamedPathToEntityId[Path]);
			continue;
		}

		// Otherwise, initiate the reservation (and thus creation) process
		Channel->SendReserveEntityIdRequest();
	}

	ReplicatedStablyNamedActorQueue.Empty();
}

void USpatialInterop::DeleteIrrelevantReplicatedStablyNamedActors(const PathNameToEntityIdMap& StablyNamedPathToEntityId)
{
	for (const auto& Pair : StablyNamedPathToEntityId)
	{
		FString FullPath = UTF8_TO_TCHAR(Pair.first.c_str());
		AActor* StableActor = FindObject<AActor>(GetWorld(), *FullPath);
		worker::EntityId EntityId = Pair.second;

		if (StableActor != nullptr && ReplicatedStablyNamedActorTimeoutMap.Find(EntityId) == nullptr)
		{
			TWeakObjectPtr<AActor> ActorPtr = StableActor;
			FTimerHandle TimerHandle;
			FTimerDelegate TimerCallback;
			TimerCallback.BindLambda([ActorPtr, EntityId, this] {
				ReplicatedStablyNamedActorTimeoutMap.Remove(EntityId);

				if (AActor* Actor = ActorPtr.Get())
				{
					if (!PackageMap->GetNetGUIDFromEntityId(EntityId).IsValid())
					{
						UE_LOG(LogSpatialGDKInterop, Log, TEXT("Timed out (deleted) replicated stably named actor: %s"), *Actor->GetName());

						UnreserveReplicatedStablyNamedActor(Actor);

						LocallyDeleteActor(Actor);
					}
				}
			});
			TimerManager->SetTimer(TimerHandle, TimerCallback, SpatialConstants::REPLICATED_STABLY_NAMED_ACTORS_DELETION_TIMEOUT_SECONDS, false);
			ReplicatedStablyNamedActorTimeoutMap.Add(EntityId, TimerCallback);
		}
	}
}

void USpatialInterop::LocallyDeleteActor(AActor* Actor)
{
	StartIgnoringAuthoritativeDestruction();
	if (Actor->GetWorld()->DestroyActor(Actor, true) == false)
	{
		UE_LOG(LogSpatialGDKInterop, Error, TEXT("World->DestroyActor locally failed on %s"), *Actor->GetName());
	}
	StopIgnoringAuthoritativeDestruction();
}
