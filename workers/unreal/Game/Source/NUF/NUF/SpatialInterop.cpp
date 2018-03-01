// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialInterop.h"

#include "SpatialConstants.h"
#include "SpatialActorChannel.h"
#include "SpatialNetConnection.h"
#include "SpatialNetDriver.h"
#include "SpatialOS.h"
#include "EntityRegistry.h"
#include "SpatialPackageMapClient.h"

#include "Generated/SpatialTypeBindingList.h"

// Needed for the entity template stuff.
#include <improbable/standard_library.h>
#include <improbable/unreal/player.h>
#include <improbable/unreal/unreal_metadata.h>
#include "EntityBuilder.h"
#include "EntityTemplate.h"

DEFINE_LOG_CATEGORY(LogSpatialOSInterop);

USpatialInterop::USpatialInterop() 
{
}

void USpatialInterop::Init(USpatialOS* Instance, USpatialNetDriver* Driver, FTimerManager* InTimerManager)
{
	SpatialOSInstance = Instance;
	NetDriver = Driver;
	TimerManager = InTimerManager;
	PackageMap = Cast<USpatialPackageMapClient>(Driver->GetSpatialOSNetConnection()->PackageMap);

	// Register type binding classes.
	TArray<UClass*> TypeBindingClasses = GetGeneratedTypeBindings();
	for (UClass* TypeBindingClass : TypeBindingClasses)
	{
		UClass* BoundClass = TypeBindingClass->GetDefaultObject<USpatialTypeBinding>()->GetBoundClass();
		UE_LOG(LogSpatialOSInterop, Log, TEXT("Registered type binding class %s handling replicated properties of %s."), *TypeBindingClass->GetName(), *BoundClass->GetName());
		RegisterInteropType(BoundClass, NewObject<USpatialTypeBinding>(this, TypeBindingClass));
	}
}

USpatialTypeBinding* USpatialInterop::GetTypeBindingByClass(UClass* Class) const
{
	for (const UClass* CurrentClass = Class; CurrentClass; CurrentClass = CurrentClass->GetSuperClass())
	{
		USpatialTypeBinding* const* BindingIterator = TypeBinding.Find(CurrentClass);
		if (BindingIterator)
		{
			return *BindingIterator;
		}
	}
	return nullptr;
}

worker::RequestId<worker::CreateEntityRequest> USpatialInterop::SendCreateEntityRequest(USpatialActorChannel* Channel, const FVector& Location, const FString& PlayerWorkerId, const TArray<uint16>& Changed)
{
	worker::RequestId<worker::CreateEntityRequest> CreateEntityRequestId;
	TSharedPtr<worker::Connection> PinnedConnection = SpatialOSInstance->GetConnection().Pin();
	if (PinnedConnection.IsValid())
	{
		AActor* Actor = Channel->Actor;
		const USpatialTypeBinding* TypeBinding = GetTypeBindingByClass(Actor->GetClass());

		FStringAssetReference ActorClassRef(Actor->GetClass());
		FString PathStr = ActorClassRef.ToString();

		if (TypeBinding)
		{
			auto Entity = TypeBinding->CreateActorEntity(PlayerWorkerId, Location, PathStr, Channel->GetChangeState(Changed), Channel);
			CreateEntityRequestId = PinnedConnection->SendCreateEntityRequest(Entity, Channel->GetEntityId(), 0);
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

			// Build entity.
			const improbable::Coordinates SpatialPosition = SpatialConstants::LocationToSpatialOSCoordinates(Location);

			auto Entity = improbable::unreal::FEntityBuilder::Begin()
				.AddPositionComponent(SpatialPosition, WorkersOnly)
				.AddMetadataComponent(improbable::Metadata::Data{TCHAR_TO_UTF8(*PathStr)})
				.SetPersistence(true)
				.SetReadAcl(AnyUnrealWorkerOrClient)
				.AddComponent<improbable::unreal::UnrealMetadata>(UnrealMetadata, WorkersOnly)
				// For now, just a dummy component we add to every such entity to make sure client has write access to at least one component.
				// todo-giray: Remove once we're using proper (generated) entity templates here.
				.AddComponent<improbable::unreal::PlayerControlClient>(improbable::unreal::PlayerControlClient::Data{}, OwnClientOnly)
				.Build();

			CreateEntityRequestId = PinnedConnection->SendCreateEntityRequest(Entity, Channel->GetEntityId(), 0);
		}
		UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Creating entity for actor %s (%llu) using initial changelist. Request ID: %d"),
			*SpatialOSInstance->GetWorkerId(), *Actor->GetName(), Channel->GetEntityId(), CreateEntityRequestId.Id);
	}
	else
	{
		UE_LOG(LogSpatialOSInterop, Warning, TEXT("Failed to obtain reference to SpatialOS connection!"));
	}
	return CreateEntityRequestId;
}

void USpatialInterop::SendSpatialPositionUpdate(const worker::EntityId& EntityId, const FVector& Location)
{
	TSharedPtr<worker::Connection> PinnedConnection = SpatialOSInstance->GetConnection().Pin();
	if (!PinnedConnection.IsValid())
	{
		UE_LOG(LogSpatialOSInterop, Warning, TEXT("Failed to obtain reference to SpatialOS connection!"));
	}
	improbable::Position::Update PositionUpdate;
	PositionUpdate.set_coords(SpatialConstants::LocationToSpatialOSCoordinates(Location));
	PinnedConnection->SendComponentUpdate<improbable::Position>(EntityId, PositionUpdate);
}

void USpatialInterop::SendSpatialUpdate(USpatialActorChannel* Channel, const TArray<uint16>& Changed)
{
	const USpatialTypeBinding* Binding = GetTypeBindingByClass(Channel->Actor->GetClass());
	if (!Binding)
	{
		//UE_LOG(LogSpatialOSInterop, Warning, TEXT("SpatialUpdateInterop: Trying to send Spatial update on unsupported class %s."),
		//	*Channel->Actor->GetClass()->GetName());
		return;
	}
	Binding->SendComponentUpdates(Channel->GetChangeState(Changed), Channel, Channel->GetEntityId());
}

void USpatialInterop::ReceiveSpatialUpdate(USpatialActorChannel* Channel, FNetBitWriter& IncomingPayload)
{
	// Add null terminator to payload.
	uint32 Terminator = 0;
	IncomingPayload.SerializeIntPacked(Terminator);

	// Build bunch data to send to the actor channel.
	FNetBitWriter BunchData(nullptr, 0);
	// Write header.
	BunchData.WriteBit(1); // bHasRepLayout
	BunchData.WriteBit(1); // bIsActor
	// Write property info.
	uint32 PayloadSize = IncomingPayload.GetNumBits();
	BunchData.SerializeIntPacked(PayloadSize);
	BunchData.SerializeBits(IncomingPayload.GetData(), IncomingPayload.GetNumBits());

	// Create bunch and send to actor channel.
	FInBunch Bunch(Channel->Connection, BunchData.GetData(), BunchData.GetNumBits());
	Bunch.ChIndex = Channel->ChIndex;
	Bunch.bHasMustBeMappedGUIDs = false;
	Bunch.bIsReplicationPaused = false;
	Channel->UActorChannel::ReceivedBunch(Bunch);
}

void USpatialInterop::InvokeRPC(AActor* TargetActor, const UFunction* const Function, FFrame* const Frame)
{
	USpatialTypeBinding* Binding = GetTypeBindingByClass(TargetActor->GetClass());
	if (!Binding)
	{
		UE_LOG(LogSpatialOSInterop, Warning, TEXT("SpatialUpdateInterop: Trying to send RPC on unsupported class %s."),
			*TargetActor->GetClass()->GetName());
		return;
	}

	Binding->SendRPCCommand(Frame->Object, Function, Frame);
}

void USpatialInterop::ResolvePendingOperations(UObject* Object, const improbable::unreal::UnrealObjectRef& ObjectRef)
{
	UE_LOG(LogSpatialOSInterop, Log, TEXT("Resolving pending object refs and RPCs which depend on object: %s %s."), *Object->GetName(), *ObjectRefToString(ObjectRef));
	ResolvePendingOutgoingObjectUpdates(Object);
	ResolvePendingOutgoingRPCs(Object);
	ResolvePendingIncomingObjectUpdates(Object, ObjectRef);
	ResolvePendingIncomingRPCs(ObjectRef);
}

void USpatialInterop::AddActorChannel(const worker::EntityId& EntityId, USpatialActorChannel* Channel)
{
	EntityToActorChannel.Add(EntityId, Channel);

	// Apply queued updates for this entity ID to the new actor channel.
	USpatialTypeBinding* Binding = GetTypeBindingByClass(Channel->Actor->GetClass());
	if (Binding)
	{
		Binding->ApplyQueuedStateToChannel(Channel);
	}

	// Set up component interests to receive single client component updates (now that roles have been set up).
	if (NetDriver->GetNetMode() == NM_Client)
	{
		SetComponentInterests_Client(Channel, EntityId);
	}
}

void USpatialInterop::RemoveActorChannel(worker::EntityId EntityId)
{
	EntityToActorChannel.Remove(EntityId);
}

USpatialActorChannel* USpatialInterop::GetActorChannelByEntityId(const worker::EntityId & EntityId) const
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

void USpatialInterop::SendCommandRequest_Internal(FRPCCommandRequestFunc Function, bool bReliable)
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

void USpatialInterop::SendCommandResponse_Internal(FRPCCommandResponseFunc Function)
{
	auto Result = Function();
	if (Result.IsSet())
	{
		QueueIncomingRPC_Internal(Result.GetValue(), Function);
	}
}

void USpatialInterop::HandleCommandResponse_Internal(const FString& RPCName, FUntypedRequestId RequestId, const worker::EntityId& EntityId, const worker::StatusCode& StatusCode, const FString& Message)
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
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: retrying in %f seconds. Error code: %d Message: %s"), *RPCName, WaitTime, (int)StatusCode, *Message);

			// Queue retry.
			FTimerHandle RetryTimer;
			FTimerDelegate TimerCallback;
			TimerCallback.BindLambda([this, RetryContext]()
			{
				auto Result = RetryContext->SendCommandRequest();
				RetryContext->NumAttempts++;
				check(Result.UnresolvedObject == nullptr);
				OutgoingReliableRPCs.Emplace(Result.RequestId, RetryContext);
			});
			TimerManager->SetTimer(RetryTimer, TimerCallback, WaitTime, false);
		}
		else
		{
			UE_LOG(LogSpatialOSInterop, Error, TEXT("%s: failed too many times, giving up (%u attempts). Error code: %d Message: %s"),
				*RPCName, SpatialConstants::MAX_NUMBER_COMMAND_ATTEMPTS, (int)StatusCode, *Message);
		}
	}
}

void USpatialInterop::QueueOutgoingObjectUpdate_Internal(UObject* UnresolvedObject, USpatialActorChannel* DependentChannel, uint16 Handle)
{
	check(UnresolvedObject);
	check(DependentChannel);
	check(Handle > 0);

	UE_LOG(LogSpatialOSPackageMap, Log, TEXT("Added pending outgoing object ref depending on object: %s, channel: %s, handle: %d."),
		*UnresolvedObject->GetName(), *DependentChannel->GetName(), Handle);

	TArray<USpatialActorChannel*>& Channels = ChannelsAwaitingOutgoingObjectResolve.FindOrAdd(UnresolvedObject);
	Channels.AddUnique(DependentChannel);

	TArray<uint16>& Handles = PendingOutgoingObjectRefHandles.FindOrAdd(DependentChannel);
	Handles.AddUnique(Handle);
}

void USpatialInterop::QueueOutgoingRPC_Internal(UObject* UnresolvedObject, FRPCCommandRequestFunc CommandSender, bool bReliable)
{
	check(UnresolvedObject);
	UE_LOG(LogSpatialOSPackageMap, Log, TEXT("Added pending outgoing RPC depending on object: %s."), *UnresolvedObject->GetName());
	PendingOutgoingRPCs.FindOrAdd(UnresolvedObject).Add(TPair<FRPCCommandRequestFunc, bool>{CommandSender, bReliable});
}

void USpatialInterop::QueueIncomingObjectUpdate_Internal(const improbable::unreal::UnrealObjectRef& UnresolvedObjectRef, USpatialActorChannel* DependentChannel, UObjectPropertyBase* Property, uint16 Handle)
{
	check(DependentChannel);
	check(Property);
	UE_LOG(LogSpatialOSPackageMap, Log, TEXT("Added pending incoming object ref depending on object ref: %s, channel: %s, property: %s."),
		*ObjectRefToString(UnresolvedObjectRef), *DependentChannel->GetName(), *Property->GetName());
	PendingIncomingObjectRefProperties.FindOrAdd(UnresolvedObjectRef).FindOrAdd(DependentChannel).Add({Property, Handle});
}

void USpatialInterop::QueueIncomingRPC_Internal(const improbable::unreal::UnrealObjectRef& UnresolvedObjectRef, FRPCCommandResponseFunc Responder)
{
	UE_LOG(LogSpatialOSPackageMap, Log, TEXT("Added pending incoming RPC depending on object ref: %s."), *ObjectRefToString(UnresolvedObjectRef));
	PendingIncomingRPCs.FindOrAdd(UnresolvedObjectRef).Add(Responder);
}

void USpatialInterop::RegisterInteropType(UClass* Class, USpatialTypeBinding* Binding)
{
	Binding->Init(this, PackageMap);
	Binding->BindToView();
	TypeBinding.Add(Class, Binding);
}

void USpatialInterop::UnregisterInteropType(UClass* Class)
{
	USpatialTypeBinding** BindingIterator = TypeBinding.Find(Class);
	if (BindingIterator != nullptr)
	{
		USpatialTypeBinding* Binding = *BindingIterator;
		Binding->UnbindFromView();
		TypeBinding.Remove(Class);
	}
}

void USpatialInterop::SetComponentInterests_Client(USpatialActorChannel* ActorChannel, const worker::EntityId& EntityId)
{
	UClass* ActorClass = ActorChannel->Actor->GetClass();
	// Are we the autonomous proxy?
	if (ActorChannel->Actor->Role == ROLE_AutonomousProxy)
	{
		// We want to receive single client updates.
		const USpatialTypeBinding* Binding = GetTypeBindingByClass(ActorClass);
		if (Binding)
		{
			worker::Map<worker::ComponentId, worker::InterestOverride> Interest;
			Interest.emplace(Binding->GetReplicatedGroupComponentId(GROUP_SingleClient), worker::InterestOverride{true});
			SpatialOSInstance->GetConnection().Pin()->SendComponentInterest(EntityId, Interest);
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: We are the owning client of %s (%llu), therefore we want single client updates."),
				*SpatialOSInstance->GetWorkerConfiguration().GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				EntityId);
		}
	}
}

void USpatialInterop::ResolvePendingOutgoingObjectUpdates(UObject* Object)
{
	TArray<USpatialActorChannel*>* DependentChannels = ChannelsAwaitingOutgoingObjectResolve.Find(Object);
	if (DependentChannels == nullptr)
	{
		return;
	}

	for (auto DependentChannel : *DependentChannels)
	{
		TArray<uint16>* Handles = PendingOutgoingObjectRefHandles.Find(DependentChannel);
		if (Handles && Handles->Num() > 0)
		{
			// Changelists always have a 0 at the end.
			Handles->Add(0);

			SendSpatialUpdate(DependentChannel, *Handles);
			PendingOutgoingObjectRefHandles.Remove(DependentChannel);
		}
	}
	ChannelsAwaitingOutgoingObjectResolve.Remove(Object);
}

void USpatialInterop::ResolvePendingOutgoingRPCs(UObject* Object)
{
	TArray<TPair<FRPCCommandRequestFunc, bool>>* RPCList = PendingOutgoingRPCs.Find(Object);
	if (RPCList)
	{
		for (auto& RequestFuncPair : *RPCList)
		{
			// We can guarantee that SendCommandRequest won't populate PendingOutgoingRPCs[Object] whilst we're iterating through it,
			// because Object has been resolved when we call ResolvePendingOutgoingRPCs.
			SendCommandRequest_Internal(RequestFuncPair.Key, RequestFuncPair.Value);
		}
		PendingOutgoingRPCs.Remove(Object);
	}
}

void USpatialInterop::ResolvePendingIncomingObjectUpdates(UObject* Object, const improbable::unreal::UnrealObjectRef& ObjectRef)
{
	TMap<USpatialActorChannel*, TArray<FPendingIncomingObjectProperty>>* DependentChannels = PendingIncomingObjectRefProperties.Find(ObjectRef);
	if (DependentChannels == nullptr)
	{
		return;
	}

	for (auto& ChannelProperties : *DependentChannels)
	{
		USpatialActorChannel* DependentChannel = ChannelProperties.Key;
		TArray<FPendingIncomingObjectProperty>& Properties = ChannelProperties.Value;

		// Build incoming bunch with resolved UObject data.
		FBunchPayloadWriter Writer(PackageMap);
		for (auto& Property : Properties)
		{
			Writer.SerializeProperty(Property.Handle, Property.ObjectProperty, &Object);
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received queued object property update. actor %s (%llu), property %s (handle %d)"),
				*SpatialOSInstance ->GetWorkerId(),
				*DependentChannel->Actor->GetName(),
				DependentChannel->GetEntityId(),
				*Property.ObjectProperty->GetName(),
				Property.Handle);
		}
		ReceiveSpatialUpdate(DependentChannel, Writer.GetNetBitWriter());
	}

	PendingIncomingObjectRefProperties.Remove(ObjectRef);
}

void USpatialInterop::ResolvePendingIncomingRPCs(const improbable::unreal::UnrealObjectRef& ObjectRef)
{
	TArray<FRPCCommandResponseFunc>* RPCList = PendingIncomingRPCs.Find(ObjectRef);
	if (RPCList)
	{
		for (auto& Responder : *RPCList)
		{
			// We can guarantee that SendCommandResponse won't populate PendingIncomingRPCs[ObjectRef] whilst we're iterating through it,
			// because ObjectRef has been resolved when we call ResolvePendingIncomingRPCs.
			SendCommandResponse_Internal(Responder);
		}
		PendingIncomingRPCs.Remove(ObjectRef);
	}
}
