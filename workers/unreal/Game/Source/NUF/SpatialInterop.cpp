// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialInterop.h"

#include "SpatialConstants.h"
#include "SpatialActorChannel.h"
#include "SpatialNetConnection.h"
#include "SpatialNetDriver.h"
#include "SpatialOS.h"
#include "EntityRegistry.h"
#include "SpatialPackageMapClient.h"

#include "Generated/SpatialTypeBinding_Character.h"
#include "Generated/SpatialTypeBinding_PlayerController.h"
#include "Generated/SpatialTypeBinding_GameStateBase.h"
#include "Generated/SpatialTypeBinding_PlayerState.h"

DEFINE_LOG_CATEGORY(LogSpatialOSInterop);

USpatialInterop::USpatialInterop() 
{
}

void USpatialInterop::Init(bool bClient, USpatialOS* Instance, USpatialNetDriver* Driver, FTimerManager* InTimerManager)
{
	bIsClient = bClient;
	SpatialOSInstance = Instance;
	NetDriver = Driver;
	TimerManager = InTimerManager;
	PackageMap = Driver->ServerConnection ? Cast<USpatialPackageMapClient>(Driver->ServerConnection->PackageMap) :
											Cast<USpatialPackageMapClient>(Driver->GetSpatialOSNetConnection()->PackageMap);

	RegisterInteropType(ACharacter::StaticClass(), NewObject<USpatialTypeBinding_Character>(this));
	RegisterInteropType(APlayerController::StaticClass(), NewObject<USpatialTypeBinding_PlayerController>(this));
	RegisterInteropType(AGameStateBase::StaticClass(), NewObject<USpatialTypeBinding_GameStateBase>(this));
	RegisterInteropType(APlayerState::StaticClass(), NewObject<USpatialTypeBinding_PlayerState>(this));
}

void USpatialInterop::Tick(float DeltaTime)
{
	// todo sami - check before commit
	//Leaving it here for now, we'll remove if it ends up unused.
}

USpatialActorChannel* USpatialInterop::GetClientActorChannel(const worker::EntityId & EntityId) const
{
	// Get actor channel.
	USpatialActorChannel* const* ActorChannelIt = EntityToClientActorChannel.Find(EntityId);
	if (!ActorChannelIt)
	{
		// Can't find actor channel for this entity, give up.
		return nullptr;
	}
	return *ActorChannelIt;
}

void USpatialInterop::AddClientActorChannel(const worker::EntityId& EntityId, USpatialActorChannel* Channel)
{
	EntityToClientActorChannel.Add(EntityId, Channel);
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

void USpatialInterop::SendCommandRequest(FRPCRequestFunction Function)
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
		PackageMap->AddPendingRPC(Result.UnresolvedObject, Function);
	}
	else
	{
		// Add to outgoing RPCs.
		OutgoingRPCs.Emplace(Result.RequestId, TSharedPtr<FRPCRetryContext>(new FRPCRetryContext(Function)));
	}
}

void USpatialInterop::HandleCommandResponse(const FString& RPCName, FUntypedRequestId RequestId, const worker::EntityId& EntityId, const worker::StatusCode& StatusCode, const FString& Message)
{
	TSharedPtr<FRPCRetryContext>* RequestContextIterator = OutgoingRPCs.Find(RequestId);
	if (!RequestContextIterator)
	{
		UE_LOG(LogSpatialOSInterop, Error, TEXT("%s: received an response which we did not send. Entity ID: %lld, Request ID: %d"), *RPCName, EntityId, RequestId);
		return;
	}

	TSharedPtr<FRPCRetryContext> RetryContext = *RequestContextIterator;
	OutgoingRPCs.Remove(RequestId);
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
				OutgoingRPCs.Emplace(Result.RequestId, RetryContext);
			});
			// TODO(David): Commenting out for now to avoid a potentially buggy retry solution interfering with getting the character to move.
			TimerManager->SetTimer(RetryTimer, TimerCallback, WaitTime, false);
		}
		else
		{
			UE_LOG(LogSpatialOSInterop, Error, TEXT("%s: failed too many times, giving up (%u attempts). Error code: %d Message: %s"),
				*RPCName, SpatialConstants::MAX_NUMBER_COMMAND_ATTEMPTS, (int)StatusCode, *Message);
		}
	}
}

void USpatialInterop::SetComponentInterests(USpatialActorChannel* ActorChannel, const worker::EntityId& EntityId)
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
			UE_LOG(LogSpatialOSInterop, Warning, TEXT("We are the owning client, therefore we want single client updates. Client ID: %s"),
				*SpatialOSInstance->GetWorkerConfiguration().GetWorkerId());
		}
	}
}
