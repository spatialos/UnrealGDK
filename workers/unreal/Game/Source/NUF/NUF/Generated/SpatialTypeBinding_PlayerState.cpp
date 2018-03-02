// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// Note that this file has been generated automatically

#include "SpatialTypeBinding_PlayerState.h"
#include "Engine.h"

#include "SpatialOS.h"
#include "EntityBuilder.h"

#include "../SpatialConstants.h"
#include "../SpatialUnrealObjectRef.h"
#include "../SpatialActorChannel.h"
#include "../SpatialPackageMapClient.h"
#include "../SpatialNetDriver.h"
#include "../SpatialInterop.h"

#include "UnrealPlayerStateSingleClientReplicatedDataAddComponentOp.h"
#include "UnrealPlayerStateMultiClientReplicatedDataAddComponentOp.h"

const FRepHandlePropertyMap& USpatialTypeBinding_PlayerState::GetHandlePropertyMap()
{
	static FRepHandlePropertyMap HandleToPropertyMap;
	if (HandleToPropertyMap.Num() == 0)
	{
		UClass* Class = FindObject<UClass>(ANY_PACKAGE, TEXT("PlayerState"));
		HandleToPropertyMap.Add(1, FRepHandleData{nullptr, Class->FindPropertyByName("bHidden"), COND_None});
		HandleToPropertyMap.Add(2, FRepHandleData{nullptr, Class->FindPropertyByName("bReplicateMovement"), COND_None});
		HandleToPropertyMap.Add(3, FRepHandleData{nullptr, Class->FindPropertyByName("bTearOff"), COND_None});
		HandleToPropertyMap.Add(4, FRepHandleData{nullptr, Class->FindPropertyByName("RemoteRole"), COND_None});
		HandleToPropertyMap.Add(5, FRepHandleData{nullptr, Class->FindPropertyByName("Owner"), COND_None});
		HandleToPropertyMap.Add(6, FRepHandleData{nullptr, Class->FindPropertyByName("ReplicatedMovement"), COND_SimulatedOrPhysics});
		HandleToPropertyMap.Add(7, FRepHandleData{Class->FindPropertyByName("AttachmentReplication"), nullptr, COND_Custom});
		HandleToPropertyMap[7].Property = Cast<UStructProperty>(HandleToPropertyMap[7].Parent)->Struct->FindPropertyByName("AttachParent");
		HandleToPropertyMap.Add(8, FRepHandleData{Class->FindPropertyByName("AttachmentReplication"), nullptr, COND_Custom});
		HandleToPropertyMap[8].Property = Cast<UStructProperty>(HandleToPropertyMap[8].Parent)->Struct->FindPropertyByName("LocationOffset");
		HandleToPropertyMap.Add(9, FRepHandleData{Class->FindPropertyByName("AttachmentReplication"), nullptr, COND_Custom});
		HandleToPropertyMap[9].Property = Cast<UStructProperty>(HandleToPropertyMap[9].Parent)->Struct->FindPropertyByName("RelativeScale3D");
		HandleToPropertyMap.Add(10, FRepHandleData{Class->FindPropertyByName("AttachmentReplication"), nullptr, COND_Custom});
		HandleToPropertyMap[10].Property = Cast<UStructProperty>(HandleToPropertyMap[10].Parent)->Struct->FindPropertyByName("RotationOffset");
		HandleToPropertyMap.Add(11, FRepHandleData{Class->FindPropertyByName("AttachmentReplication"), nullptr, COND_Custom});
		HandleToPropertyMap[11].Property = Cast<UStructProperty>(HandleToPropertyMap[11].Parent)->Struct->FindPropertyByName("AttachSocket");
		HandleToPropertyMap.Add(12, FRepHandleData{Class->FindPropertyByName("AttachmentReplication"), nullptr, COND_Custom});
		HandleToPropertyMap[12].Property = Cast<UStructProperty>(HandleToPropertyMap[12].Parent)->Struct->FindPropertyByName("AttachComponent");
		HandleToPropertyMap.Add(13, FRepHandleData{nullptr, Class->FindPropertyByName("Role"), COND_None});
		HandleToPropertyMap.Add(14, FRepHandleData{nullptr, Class->FindPropertyByName("bCanBeDamaged"), COND_None});
		HandleToPropertyMap.Add(15, FRepHandleData{nullptr, Class->FindPropertyByName("Instigator"), COND_None});
		HandleToPropertyMap.Add(16, FRepHandleData{nullptr, Class->FindPropertyByName("Score"), COND_None});
		HandleToPropertyMap.Add(17, FRepHandleData{nullptr, Class->FindPropertyByName("Ping"), COND_SkipOwner});
		HandleToPropertyMap.Add(18, FRepHandleData{nullptr, Class->FindPropertyByName("PlayerName"), COND_None});
		HandleToPropertyMap.Add(19, FRepHandleData{nullptr, Class->FindPropertyByName("PlayerId"), COND_InitialOnly});
		HandleToPropertyMap.Add(20, FRepHandleData{nullptr, Class->FindPropertyByName("bFromPreviousLevel"), COND_None});
		HandleToPropertyMap.Add(21, FRepHandleData{nullptr, Class->FindPropertyByName("bIsABot"), COND_InitialOnly});
		HandleToPropertyMap.Add(22, FRepHandleData{nullptr, Class->FindPropertyByName("bIsInactive"), COND_InitialOnly});
		HandleToPropertyMap.Add(23, FRepHandleData{nullptr, Class->FindPropertyByName("bIsSpectator"), COND_None});
		HandleToPropertyMap.Add(24, FRepHandleData{nullptr, Class->FindPropertyByName("bOnlySpectator"), COND_None});
		HandleToPropertyMap.Add(25, FRepHandleData{nullptr, Class->FindPropertyByName("StartTime"), COND_None});
		HandleToPropertyMap.Add(26, FRepHandleData{nullptr, Class->FindPropertyByName("UniqueId"), COND_InitialOnly});
	}
	return HandleToPropertyMap;
}

UClass* USpatialTypeBinding_PlayerState::GetBoundClass() const
{
	return APlayerState::StaticClass();
}

void USpatialTypeBinding_PlayerState::Init(USpatialInterop* InInterop, USpatialPackageMapClient* InPackageMap)
{
	Super::Init(InInterop, InPackageMap);

}

void USpatialTypeBinding_PlayerState::BindToView()
{
	TSharedPtr<worker::View> View = Interop->GetSpatialOS()->GetView().Pin();
	ViewCallbacks.Init(View);

	if (Interop->GetNetDriver()->GetNetMode() == NM_Client)
	{
		ViewCallbacks.Add(View->OnComponentUpdate<improbable::unreal::UnrealPlayerStateSingleClientReplicatedData>([this](
			const worker::ComponentUpdateOp<improbable::unreal::UnrealPlayerStateSingleClientReplicatedData>& Op)
		{
			USpatialActorChannel* ActorChannel = Interop->GetActorChannelByEntityId(Op.EntityId);
			if (ActorChannel)
			{
				ClientReceiveUpdate_SingleClient(ActorChannel, Op.Update);
			}
			else
			{
				Op.Update.ApplyTo(PendingSingleClientData.FindOrAdd(Op.EntityId));
			}
		}));
		ViewCallbacks.Add(View->OnComponentUpdate<improbable::unreal::UnrealPlayerStateMultiClientReplicatedData>([this](
			const worker::ComponentUpdateOp<improbable::unreal::UnrealPlayerStateMultiClientReplicatedData>& Op)
		{
			USpatialActorChannel* ActorChannel = Interop->GetActorChannelByEntityId(Op.EntityId);
			if (ActorChannel)
			{
				ClientReceiveUpdate_MultiClient(ActorChannel, Op.Update);
			}
			else
			{
				Op.Update.ApplyTo(PendingMultiClientData.FindOrAdd(Op.EntityId));
			}
		}));
	}
}

void USpatialTypeBinding_PlayerState::UnbindFromView()
{
	ViewCallbacks.Reset();
}

worker::ComponentId USpatialTypeBinding_PlayerState::GetReplicatedGroupComponentId(EReplicatedPropertyGroup Group) const
{
	switch (Group)
	{
	case GROUP_SingleClient:
		return improbable::unreal::UnrealPlayerStateSingleClientReplicatedData::ComponentId;
	case GROUP_MultiClient:
		return improbable::unreal::UnrealPlayerStateMultiClientReplicatedData::ComponentId;
	default:
		checkNoEntry();
		return 0;
	}
}

worker::Entity USpatialTypeBinding_PlayerState::CreateActorEntity(const FString& ClientWorkerId, const FVector& Position, const FString& Metadata, const FPropertyChangeState& InitialChanges, USpatialActorChannel* Channel) const
{
	// Setup initial data.
	improbable::unreal::UnrealPlayerStateSingleClientReplicatedData::Data SingleClientData;
	improbable::unreal::UnrealPlayerStateSingleClientReplicatedData::Update SingleClientUpdate;
	bool bSingleClientUpdateChanged = false;
	improbable::unreal::UnrealPlayerStateMultiClientReplicatedData::Data MultiClientData;
	improbable::unreal::UnrealPlayerStateMultiClientReplicatedData::Update MultiClientUpdate;
	bool bMultiClientUpdateChanged = false;
	BuildSpatialComponentUpdate(InitialChanges, Channel, SingleClientUpdate, bSingleClientUpdateChanged, MultiClientUpdate, bMultiClientUpdateChanged);
	SingleClientUpdate.ApplyTo(SingleClientData);
	MultiClientUpdate.ApplyTo(MultiClientData);

	// Create entity.
	std::string ClientWorkerIdString = TCHAR_TO_UTF8(*ClientWorkerId);

	improbable::WorkerAttributeSet WorkerAttribute{{worker::List<std::string>{"UnrealWorker"}}};
	improbable::WorkerAttributeSet ClientAttribute{{worker::List<std::string>{"UnrealClient"}}};
	improbable::WorkerAttributeSet OwningClientAttribute{{"workerId:" + ClientWorkerIdString}};

	improbable::WorkerRequirementSet WorkersOnly{{WorkerAttribute}};
	improbable::WorkerRequirementSet ClientsOnly{{ClientAttribute}};
	improbable::WorkerRequirementSet OwningClientOnly{{OwningClientAttribute}};
	improbable::WorkerRequirementSet AnyUnrealWorkerOrClient{{WorkerAttribute, ClientAttribute}};
	improbable::WorkerRequirementSet AnyUnrealWorkerOrOwningClient{{WorkerAttribute, OwningClientAttribute}};

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
	const improbable::Coordinates SpatialPosition = SpatialConstants::LocationToSpatialOSCoordinates(Position);
	return improbable::unreal::FEntityBuilder::Begin()
		.AddPositionComponent(improbable::Position::Data{SpatialPosition}, WorkersOnly)
		.AddMetadataComponent(improbable::Metadata::Data{TCHAR_TO_UTF8(*Metadata)})
		.SetPersistence(true)
		.SetReadAcl(AnyUnrealWorkerOrClient)
		.AddComponent<improbable::unreal::UnrealMetadata>(UnrealMetadata, WorkersOnly)
		.AddComponent<improbable::unreal::UnrealPlayerStateSingleClientReplicatedData>(SingleClientData, WorkersOnly)
		.AddComponent<improbable::unreal::UnrealPlayerStateMultiClientReplicatedData>(MultiClientData, WorkersOnly)
		.AddComponent<improbable::unreal::UnrealPlayerStateCompleteData>(improbable::unreal::UnrealPlayerStateCompleteData::Data{}, WorkersOnly)
		.AddComponent<improbable::unreal::UnrealPlayerStateClientRPCs>(improbable::unreal::UnrealPlayerStateClientRPCs::Data{}, OwningClientOnly)
		.AddComponent<improbable::unreal::UnrealPlayerStateServerRPCs>(improbable::unreal::UnrealPlayerStateServerRPCs::Data{}, WorkersOnly)
		.Build();
}

void USpatialTypeBinding_PlayerState::SendComponentUpdates(const FPropertyChangeState& Changes, USpatialActorChannel* Channel, const worker::EntityId& EntityId) const
{
	// Build SpatialOS updates.
	improbable::unreal::UnrealPlayerStateSingleClientReplicatedData::Update SingleClientUpdate;
	bool bSingleClientUpdateChanged = false;
	improbable::unreal::UnrealPlayerStateMultiClientReplicatedData::Update MultiClientUpdate;
	bool bMultiClientUpdateChanged = false;
	BuildSpatialComponentUpdate(Changes, Channel, SingleClientUpdate, bSingleClientUpdateChanged, MultiClientUpdate, bMultiClientUpdateChanged);

	// Send SpatialOS updates if anything changed.
	TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
	if (bSingleClientUpdateChanged)
	{
		Connection->SendComponentUpdate<improbable::unreal::UnrealPlayerStateSingleClientReplicatedData>(EntityId, SingleClientUpdate);
	}
	if (bMultiClientUpdateChanged)
	{
		Connection->SendComponentUpdate<improbable::unreal::UnrealPlayerStateMultiClientReplicatedData>(EntityId, MultiClientUpdate);
	}
}

void USpatialTypeBinding_PlayerState::SendRPCCommand(UObject* TargetObject, const UFunction* const Function, FFrame* const Frame)
{
	TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
	auto SenderFuncIterator = RPCToSenderMap.Find(Function->GetFName());
	checkf(*SenderFuncIterator, TEXT("Sender for %s has not been registered with RPCToSenderMap."), *Function->GetFName().ToString());
	(this->*(*SenderFuncIterator))(Connection.Get(), Frame, TargetObject);
}

void USpatialTypeBinding_PlayerState::ReceiveAddComponent(USpatialActorChannel* Channel, UAddComponentOpWrapperBase* AddComponentOp) const
{
	auto* SingleClientAddOp = Cast<UUnrealPlayerStateSingleClientReplicatedDataAddComponentOp>(AddComponentOp);
	if (SingleClientAddOp)
	{
		auto Update = improbable::unreal::UnrealPlayerStateSingleClientReplicatedData::Update::FromInitialData(*SingleClientAddOp->Data.data());
		ClientReceiveUpdate_SingleClient(Channel, Update);
	}
	auto* MultiClientAddOp = Cast<UUnrealPlayerStateMultiClientReplicatedDataAddComponentOp>(AddComponentOp);
	if (MultiClientAddOp)
	{
		auto Update = improbable::unreal::UnrealPlayerStateMultiClientReplicatedData::Update::FromInitialData(*MultiClientAddOp->Data.data());
		ClientReceiveUpdate_MultiClient(Channel, Update);
	}
}

void USpatialTypeBinding_PlayerState::ApplyQueuedStateToChannel(USpatialActorChannel* ActorChannel)
{
	improbable::unreal::UnrealPlayerStateSingleClientReplicatedData::Data* SingleClientData = PendingSingleClientData.Find(ActorChannel->GetEntityId());
	if (SingleClientData)
	{
		auto Update = improbable::unreal::UnrealPlayerStateSingleClientReplicatedData::Update::FromInitialData(*SingleClientData);
		PendingSingleClientData.Remove(ActorChannel->GetEntityId());
		ClientReceiveUpdate_SingleClient(ActorChannel, Update);
	}
	improbable::unreal::UnrealPlayerStateMultiClientReplicatedData::Data* MultiClientData = PendingMultiClientData.Find(ActorChannel->GetEntityId());
	if (MultiClientData)
	{
		auto Update = improbable::unreal::UnrealPlayerStateMultiClientReplicatedData::Update::FromInitialData(*MultiClientData);
		PendingMultiClientData.Remove(ActorChannel->GetEntityId());
		ClientReceiveUpdate_MultiClient(ActorChannel, Update);
	}
}

void USpatialTypeBinding_PlayerState::BuildSpatialComponentUpdate(
	const FPropertyChangeState& Changes,
	USpatialActorChannel* Channel,
	improbable::unreal::UnrealPlayerStateSingleClientReplicatedData::Update& SingleClientUpdate,
	bool& bSingleClientUpdateChanged,
	improbable::unreal::UnrealPlayerStateMultiClientReplicatedData::Update& MultiClientUpdate,
	bool& bMultiClientUpdateChanged) const
{
	// Build up SpatialOS component updates.
	auto& PropertyMap = GetHandlePropertyMap();
	FChangelistIterator ChangelistIterator(Changes.Changed, 0);
	FRepHandleIterator HandleIterator(ChangelistIterator, Changes.Cmds, Changes.BaseHandleToCmdIndex, 0, 1, 0, Changes.Cmds.Num() - 1);
	while (HandleIterator.NextHandle())
	{
		const FRepLayoutCmd& Cmd = Changes.Cmds[HandleIterator.CmdIndex];
		const uint8* Data = Changes.SourceData + HandleIterator.ArrayOffset + Cmd.Offset;
		auto& PropertyMapData = PropertyMap[HandleIterator.Handle];
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending property update. actor %s (%lld), property %s (handle %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*Channel->Actor->GetName(),
			Channel->GetEntityId(),
			*Cmd.Property->GetName(),
			HandleIterator.Handle);
		switch (GetGroupFromCondition(PropertyMapData.Condition))
		{
		case GROUP_SingleClient:
			ServerSendUpdate_SingleClient(Data, HandleIterator.Handle, Cmd.Property, Channel, SingleClientUpdate);
			bSingleClientUpdateChanged = true;
			break;
		case GROUP_MultiClient:
			ServerSendUpdate_MultiClient(Data, HandleIterator.Handle, Cmd.Property, Channel, MultiClientUpdate);
			bMultiClientUpdateChanged = true;
			break;
		}
	}
}

void USpatialTypeBinding_PlayerState::ServerSendUpdate_SingleClient(
	const uint8* RESTRICT Data,
	int32 Handle,
	UProperty* Property,
	USpatialActorChannel* Channel,
	improbable::unreal::UnrealPlayerStateSingleClientReplicatedData::Update& OutUpdate) const
{
}

void USpatialTypeBinding_PlayerState::ServerSendUpdate_MultiClient(
	const uint8* RESTRICT Data,
	int32 Handle,
	UProperty* Property,
	USpatialActorChannel* Channel,
	improbable::unreal::UnrealPlayerStateMultiClientReplicatedData::Update& OutUpdate) const
{
	switch (Handle)
	{
		case 1: // field_bhidden
		{
			bool Value = static_cast<UBoolProperty*>(Property)->GetPropertyValue(Data);

			OutUpdate.set_field_bhidden(Value);
			break;
		}
		case 2: // field_breplicatemovement
		{
			bool Value = static_cast<UBoolProperty*>(Property)->GetPropertyValue(Data);

			OutUpdate.set_field_breplicatemovement(Value);
			break;
		}
		case 3: // field_btearoff
		{
			bool Value = static_cast<UBoolProperty*>(Property)->GetPropertyValue(Data);

			OutUpdate.set_field_btearoff(Value);
			break;
		}
		case 4: // field_remoterole
		{
			TEnumAsByte<ENetRole> Value = *(reinterpret_cast<TEnumAsByte<ENetRole> const*>(Data));

			OutUpdate.set_field_remoterole(uint32_t(Value));
			break;
		}
		case 5: // field_owner
		{
			AActor* Value = *(reinterpret_cast<AActor* const*>(Data));

			if (Value != nullptr)
			{
				FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(Value);
				improbable::unreal::UnrealObjectRef ObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
				if (ObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
				{
					Interop->QueueOutgoingObjectUpdate_Internal(Value, Channel, 5);
				}
				else
				{
					OutUpdate.set_field_owner(ObjectRef);
				}
			}
			else
			{
				OutUpdate.set_field_owner(SpatialConstants::NULL_OBJECT_REF);
			}
			break;
		}
		case 6: // field_replicatedmovement
		{
			FRepMovement Value = *(reinterpret_cast<FRepMovement const*>(Data));

			{
				TArray<uint8> ValueData;
				FMemoryWriter ValueDataWriter(ValueData);
				bool Success;
				Value.NetSerialize(ValueDataWriter, nullptr, Success);
				OutUpdate.set_field_replicatedmovement(std::string((char*)ValueData.GetData(), ValueData.Num()));
			}
			break;
		}
		case 7: // field_attachmentreplication_attachparent
		{
			AActor* Value = *(reinterpret_cast<AActor* const*>(Data));

			if (Value != nullptr)
			{
				FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(Value);
				improbable::unreal::UnrealObjectRef ObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
				if (ObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
				{
					Interop->QueueOutgoingObjectUpdate_Internal(Value, Channel, 7);
				}
				else
				{
					OutUpdate.set_field_attachmentreplication_attachparent(ObjectRef);
				}
			}
			else
			{
				OutUpdate.set_field_attachmentreplication_attachparent(SpatialConstants::NULL_OBJECT_REF);
			}
			break;
		}
		case 8: // field_attachmentreplication_locationoffset
		{
			FVector_NetQuantize100 Value = *(reinterpret_cast<FVector_NetQuantize100 const*>(Data));

			OutUpdate.set_field_attachmentreplication_locationoffset(improbable::Vector3f(Value.X, Value.Y, Value.Z));
			break;
		}
		case 9: // field_attachmentreplication_relativescale3d
		{
			FVector_NetQuantize100 Value = *(reinterpret_cast<FVector_NetQuantize100 const*>(Data));

			OutUpdate.set_field_attachmentreplication_relativescale3d(improbable::Vector3f(Value.X, Value.Y, Value.Z));
			break;
		}
		case 10: // field_attachmentreplication_rotationoffset
		{
			FRotator Value = *(reinterpret_cast<FRotator const*>(Data));

			OutUpdate.set_field_attachmentreplication_rotationoffset(improbable::unreal::UnrealFRotator(Value.Yaw, Value.Pitch, Value.Roll));
			break;
		}
		case 11: // field_attachmentreplication_attachsocket
		{
			FName Value = *(reinterpret_cast<FName const*>(Data));

			OutUpdate.set_field_attachmentreplication_attachsocket(TCHAR_TO_UTF8(*Value.ToString()));
			break;
		}
		case 12: // field_attachmentreplication_attachcomponent
		{
			USceneComponent* Value = *(reinterpret_cast<USceneComponent* const*>(Data));

			if (Value != nullptr)
			{
				FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(Value);
				improbable::unreal::UnrealObjectRef ObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
				if (ObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
				{
					Interop->QueueOutgoingObjectUpdate_Internal(Value, Channel, 12);
				}
				else
				{
					OutUpdate.set_field_attachmentreplication_attachcomponent(ObjectRef);
				}
			}
			else
			{
				OutUpdate.set_field_attachmentreplication_attachcomponent(SpatialConstants::NULL_OBJECT_REF);
			}
			break;
		}
		case 13: // field_role
		{
			TEnumAsByte<ENetRole> Value = *(reinterpret_cast<TEnumAsByte<ENetRole> const*>(Data));

			OutUpdate.set_field_role(uint32_t(Value));
			break;
		}
		case 14: // field_bcanbedamaged
		{
			bool Value = static_cast<UBoolProperty*>(Property)->GetPropertyValue(Data);

			OutUpdate.set_field_bcanbedamaged(Value);
			break;
		}
		case 15: // field_instigator
		{
			APawn* Value = *(reinterpret_cast<APawn* const*>(Data));

			if (Value != nullptr)
			{
				FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(Value);
				improbable::unreal::UnrealObjectRef ObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
				if (ObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
				{
					Interop->QueueOutgoingObjectUpdate_Internal(Value, Channel, 15);
				}
				else
				{
					OutUpdate.set_field_instigator(ObjectRef);
				}
			}
			else
			{
				OutUpdate.set_field_instigator(SpatialConstants::NULL_OBJECT_REF);
			}
			break;
		}
		case 16: // field_score
		{
			float Value = *(reinterpret_cast<float const*>(Data));

			OutUpdate.set_field_score(Value);
			break;
		}
		case 17: // field_ping
		{
			uint8 Value = *(reinterpret_cast<uint8 const*>(Data));

			OutUpdate.set_field_ping(uint32_t(Value));
			break;
		}
		case 18: // field_playername
		{
			FString Value = *(reinterpret_cast<FString const*>(Data));

			OutUpdate.set_field_playername(TCHAR_TO_UTF8(*Value));
			break;
		}
		case 19: // field_playerid
		{
			int32 Value = *(reinterpret_cast<int32 const*>(Data));

			OutUpdate.set_field_playerid(Value);
			break;
		}
		case 20: // field_bfrompreviouslevel
		{
			bool Value = static_cast<UBoolProperty*>(Property)->GetPropertyValue(Data);

			OutUpdate.set_field_bfrompreviouslevel(Value);
			break;
		}
		case 21: // field_bisabot
		{
			bool Value = static_cast<UBoolProperty*>(Property)->GetPropertyValue(Data);

			OutUpdate.set_field_bisabot(Value);
			break;
		}
		case 22: // field_bisinactive
		{
			bool Value = static_cast<UBoolProperty*>(Property)->GetPropertyValue(Data);

			OutUpdate.set_field_bisinactive(Value);
			break;
		}
		case 23: // field_bisspectator
		{
			bool Value = static_cast<UBoolProperty*>(Property)->GetPropertyValue(Data);

			OutUpdate.set_field_bisspectator(Value);
			break;
		}
		case 24: // field_bonlyspectator
		{
			bool Value = static_cast<UBoolProperty*>(Property)->GetPropertyValue(Data);

			OutUpdate.set_field_bonlyspectator(Value);
			break;
		}
		case 25: // field_starttime
		{
			int32 Value = *(reinterpret_cast<int32 const*>(Data));

			OutUpdate.set_field_starttime(Value);
			break;
		}
		case 26: // field_uniqueid
		{
			FUniqueNetIdRepl Value = *(reinterpret_cast<FUniqueNetIdRepl const*>(Data));

			{
				TArray<uint8> ValueData;
				FMemoryWriter ValueDataWriter(ValueData);
				bool Success;
				Value.NetSerialize(ValueDataWriter, nullptr, Success);
				OutUpdate.set_field_uniqueid(std::string((char*)ValueData.GetData(), ValueData.Num()));
			}
			break;
		}
	default:
		checkf(false, TEXT("Unknown replication handle %d encountered when creating a SpatialOS update."));
		break;
	}
}

void USpatialTypeBinding_PlayerState::ClientReceiveUpdate_SingleClient(
	USpatialActorChannel* ActorChannel,
	const improbable::unreal::UnrealPlayerStateSingleClientReplicatedData::Update& Update) const
{
	FBunchPayloadWriter OutputWriter(PackageMap);

	auto& HandleToPropertyMap = GetHandlePropertyMap();
	const bool bAutonomousProxy = ActorChannel->IsClientAutonomousProxy(improbable::unreal::UnrealPlayerStateClientRPCs::ComponentId);
	ConditionMapFilter ConditionMap(ActorChannel, bAutonomousProxy);
	Interop->ReceiveSpatialUpdate(ActorChannel, OutputWriter.GetNetBitWriter());
}

void USpatialTypeBinding_PlayerState::ClientReceiveUpdate_MultiClient(
	USpatialActorChannel* ActorChannel,
	const improbable::unreal::UnrealPlayerStateMultiClientReplicatedData::Update& Update) const
{
	FBunchPayloadWriter OutputWriter(PackageMap);

	auto& HandleToPropertyMap = GetHandlePropertyMap();
	const bool bAutonomousProxy = ActorChannel->IsClientAutonomousProxy(improbable::unreal::UnrealPlayerStateClientRPCs::ComponentId);
	ConditionMapFilter ConditionMap(ActorChannel, bAutonomousProxy);
	if (!Update.field_bhidden().empty())
	{
		// field_bhidden
		uint32 Handle = 1;
		const FRepHandleData* Data = &HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data->Condition))
		{
			uint8 Value;

			Value = (*Update.field_bhidden().data());
			// Because Unreal will look for a specific bit in the serialization function below, we simply set all bits.
			// We will soon move away from using bunches when receiving Spatial updates.
			if (Value)
			{
				Value = 0xFF;
			}

			OutputWriter.SerializeProperty(Handle, Data->Property, &Value);
			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_breplicatemovement().empty())
	{
		// field_breplicatemovement
		uint32 Handle = 2;
		const FRepHandleData* Data = &HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data->Condition))
		{
			uint8 Value;

			Value = (*Update.field_breplicatemovement().data());
			// Because Unreal will look for a specific bit in the serialization function below, we simply set all bits.
			// We will soon move away from using bunches when receiving Spatial updates.
			if (Value)
			{
				Value = 0xFF;
			}

			OutputWriter.SerializeProperty(Handle, Data->Property, &Value);
			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_btearoff().empty())
	{
		// field_btearoff
		uint32 Handle = 3;
		const FRepHandleData* Data = &HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data->Condition))
		{
			uint8 Value;

			Value = (*Update.field_btearoff().data());
			// Because Unreal will look for a specific bit in the serialization function below, we simply set all bits.
			// We will soon move away from using bunches when receiving Spatial updates.
			if (Value)
			{
				Value = 0xFF;
			}

			OutputWriter.SerializeProperty(Handle, Data->Property, &Value);
			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_remoterole().empty())
	{
		// field_remoterole
		uint32 Handle = 4;
		const FRepHandleData* Data = &HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data->Condition))
		{
			TEnumAsByte<ENetRole> Value;

			Value = TEnumAsByte<ENetRole>(uint8((*Update.field_remoterole().data())));

			// Downgrade role from AutonomousProxy to SimulatedProxy if we aren't authoritative over
			// the server RPCs component.
			if (Value == ROLE_AutonomousProxy && !bAutonomousProxy)
			{
				Value = ROLE_SimulatedProxy;
			}

			// On the server, we want to "undo" the swap which will be done automatically by the network system.
			if (Interop->GetNetDriver()->IsServer())
			{
				Handle = 13;
				Data = &HandleToPropertyMap[Handle];
			}

			OutputWriter.SerializeProperty(Handle, Data->Property, &Value);
			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_owner().empty())
	{
		// field_owner
		uint32 Handle = 5;
		const FRepHandleData* Data = &HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data->Condition))
		{
			bool bWriteObjectProperty = true;
			AActor* Value;

			{
				improbable::unreal::UnrealObjectRef ObjectRef = (*Update.field_owner().data());
				check(ObjectRef != SpatialConstants::UNRESOLVED_OBJECT_REF);
				if (ObjectRef == SpatialConstants::NULL_OBJECT_REF)
				{
					Value = nullptr;
				}
				else
				{
					FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(ObjectRef);
					if (NetGUID.IsValid())
					{
						UObject* Object_Raw = PackageMap->GetObjectFromNetGUID(NetGUID, true);
						checkf(Object_Raw, TEXT("An object ref %s should map to a valid object."), *ObjectRefToString(ObjectRef));
						Value = dynamic_cast<AActor*>(Object_Raw);
						checkf(Value, TEXT("Object ref %s maps to object %s with the wrong class."), *ObjectRefToString(ObjectRef), *Object_Raw->GetFullName());
					}
					else
					{
						UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received unresolved object property. Value: %s. actor %s (%llu), property %s (handle %d)"),
							*Interop->GetSpatialOS()->GetWorkerId(),
							*ObjectRefToString(ObjectRef),
							*ActorChannel->Actor->GetName(),
							ActorChannel->GetEntityId(),
							*Data->Property->GetName(),
							Handle);
						bWriteObjectProperty = false;
						Interop->QueueIncomingObjectUpdate_Internal(ObjectRef, ActorChannel, Cast<UObjectPropertyBase>(Data->Property), Handle);
					}
				}
			}

			if (bWriteObjectProperty)
			{
				OutputWriter.SerializeProperty(Handle, Data->Property, &Value);
				UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
					*Interop->GetSpatialOS()->GetWorkerId(),
					*ActorChannel->Actor->GetName(),
					ActorChannel->GetEntityId(),
					*Data->Property->GetName(),
					Handle);
			}
		}
	}
	if (!Update.field_replicatedmovement().empty())
	{
		// field_replicatedmovement
		uint32 Handle = 6;
		const FRepHandleData* Data = &HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data->Condition))
		{
			FRepMovement Value;

			{
				auto& ValueDataStr = (*Update.field_replicatedmovement().data());
				TArray<uint8> ValueData;
				ValueData.Append((uint8*)ValueDataStr.data(), ValueDataStr.size());
				FMemoryReader ValueDataReader(ValueData);
				bool bSuccess;
				Value.NetSerialize(ValueDataReader, nullptr, bSuccess);
			}

			OutputWriter.SerializeProperty(Handle, Data->Property, &Value);
			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_attachmentreplication_attachparent().empty())
	{
		// field_attachmentreplication_attachparent
		uint32 Handle = 7;
		const FRepHandleData* Data = &HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data->Condition))
		{
			bool bWriteObjectProperty = true;
			AActor* Value;

			{
				improbable::unreal::UnrealObjectRef ObjectRef = (*Update.field_attachmentreplication_attachparent().data());
				check(ObjectRef != SpatialConstants::UNRESOLVED_OBJECT_REF);
				if (ObjectRef == SpatialConstants::NULL_OBJECT_REF)
				{
					Value = nullptr;
				}
				else
				{
					FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(ObjectRef);
					if (NetGUID.IsValid())
					{
						UObject* Object_Raw = PackageMap->GetObjectFromNetGUID(NetGUID, true);
						checkf(Object_Raw, TEXT("An object ref %s should map to a valid object."), *ObjectRefToString(ObjectRef));
						Value = dynamic_cast<AActor*>(Object_Raw);
						checkf(Value, TEXT("Object ref %s maps to object %s with the wrong class."), *ObjectRefToString(ObjectRef), *Object_Raw->GetFullName());
					}
					else
					{
						UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received unresolved object property. Value: %s. actor %s (%llu), property %s (handle %d)"),
							*Interop->GetSpatialOS()->GetWorkerId(),
							*ObjectRefToString(ObjectRef),
							*ActorChannel->Actor->GetName(),
							ActorChannel->GetEntityId(),
							*Data->Property->GetName(),
							Handle);
						bWriteObjectProperty = false;
						Interop->QueueIncomingObjectUpdate_Internal(ObjectRef, ActorChannel, Cast<UObjectPropertyBase>(Data->Property), Handle);
					}
				}
			}

			if (bWriteObjectProperty)
			{
				OutputWriter.SerializeProperty(Handle, Data->Property, &Value);
				UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
					*Interop->GetSpatialOS()->GetWorkerId(),
					*ActorChannel->Actor->GetName(),
					ActorChannel->GetEntityId(),
					*Data->Property->GetName(),
					Handle);
			}
		}
	}
	if (!Update.field_attachmentreplication_locationoffset().empty())
	{
		// field_attachmentreplication_locationoffset
		uint32 Handle = 8;
		const FRepHandleData* Data = &HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data->Condition))
		{
			FVector_NetQuantize100 Value;

			{
				auto& Vector = (*Update.field_attachmentreplication_locationoffset().data());
				Value.X = Vector.x();
				Value.Y = Vector.y();
				Value.Z = Vector.z();
			}

			OutputWriter.SerializeProperty(Handle, Data->Property, &Value);
			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_attachmentreplication_relativescale3d().empty())
	{
		// field_attachmentreplication_relativescale3d
		uint32 Handle = 9;
		const FRepHandleData* Data = &HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data->Condition))
		{
			FVector_NetQuantize100 Value;

			{
				auto& Vector = (*Update.field_attachmentreplication_relativescale3d().data());
				Value.X = Vector.x();
				Value.Y = Vector.y();
				Value.Z = Vector.z();
			}

			OutputWriter.SerializeProperty(Handle, Data->Property, &Value);
			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_attachmentreplication_rotationoffset().empty())
	{
		// field_attachmentreplication_rotationoffset
		uint32 Handle = 10;
		const FRepHandleData* Data = &HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data->Condition))
		{
			FRotator Value;

			{
				auto& Rotator = (*Update.field_attachmentreplication_rotationoffset().data());
				Value.Yaw = Rotator.yaw();
				Value.Pitch = Rotator.pitch();
				Value.Roll = Rotator.roll();
			}

			OutputWriter.SerializeProperty(Handle, Data->Property, &Value);
			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_attachmentreplication_attachsocket().empty())
	{
		// field_attachmentreplication_attachsocket
		uint32 Handle = 11;
		const FRepHandleData* Data = &HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data->Condition))
		{
			FName Value;

			Value = FName(((*Update.field_attachmentreplication_attachsocket().data())).data());

			OutputWriter.SerializeProperty(Handle, Data->Property, &Value);
			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_attachmentreplication_attachcomponent().empty())
	{
		// field_attachmentreplication_attachcomponent
		uint32 Handle = 12;
		const FRepHandleData* Data = &HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data->Condition))
		{
			bool bWriteObjectProperty = true;
			USceneComponent* Value;

			{
				improbable::unreal::UnrealObjectRef ObjectRef = (*Update.field_attachmentreplication_attachcomponent().data());
				check(ObjectRef != SpatialConstants::UNRESOLVED_OBJECT_REF);
				if (ObjectRef == SpatialConstants::NULL_OBJECT_REF)
				{
					Value = nullptr;
				}
				else
				{
					FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(ObjectRef);
					if (NetGUID.IsValid())
					{
						UObject* Object_Raw = PackageMap->GetObjectFromNetGUID(NetGUID, true);
						checkf(Object_Raw, TEXT("An object ref %s should map to a valid object."), *ObjectRefToString(ObjectRef));
						Value = dynamic_cast<USceneComponent*>(Object_Raw);
						checkf(Value, TEXT("Object ref %s maps to object %s with the wrong class."), *ObjectRefToString(ObjectRef), *Object_Raw->GetFullName());
					}
					else
					{
						UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received unresolved object property. Value: %s. actor %s (%llu), property %s (handle %d)"),
							*Interop->GetSpatialOS()->GetWorkerId(),
							*ObjectRefToString(ObjectRef),
							*ActorChannel->Actor->GetName(),
							ActorChannel->GetEntityId(),
							*Data->Property->GetName(),
							Handle);
						bWriteObjectProperty = false;
						Interop->QueueIncomingObjectUpdate_Internal(ObjectRef, ActorChannel, Cast<UObjectPropertyBase>(Data->Property), Handle);
					}
				}
			}

			if (bWriteObjectProperty)
			{
				OutputWriter.SerializeProperty(Handle, Data->Property, &Value);
				UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
					*Interop->GetSpatialOS()->GetWorkerId(),
					*ActorChannel->Actor->GetName(),
					ActorChannel->GetEntityId(),
					*Data->Property->GetName(),
					Handle);
			}
		}
	}
	if (!Update.field_role().empty())
	{
		// field_role
		uint32 Handle = 13;
		const FRepHandleData* Data = &HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data->Condition))
		{
			TEnumAsByte<ENetRole> Value;

			Value = TEnumAsByte<ENetRole>(uint8((*Update.field_role().data())));

			// On the server, we want to "undo" the swap which will be done automatically by the network system.
			if (Interop->GetNetDriver()->IsServer())
			{
				Handle = 4;
				Data = &HandleToPropertyMap[Handle];
			}

			OutputWriter.SerializeProperty(Handle, Data->Property, &Value);
			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_bcanbedamaged().empty())
	{
		// field_bcanbedamaged
		uint32 Handle = 14;
		const FRepHandleData* Data = &HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data->Condition))
		{
			uint8 Value;

			Value = (*Update.field_bcanbedamaged().data());
			// Because Unreal will look for a specific bit in the serialization function below, we simply set all bits.
			// We will soon move away from using bunches when receiving Spatial updates.
			if (Value)
			{
				Value = 0xFF;
			}

			OutputWriter.SerializeProperty(Handle, Data->Property, &Value);
			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_instigator().empty())
	{
		// field_instigator
		uint32 Handle = 15;
		const FRepHandleData* Data = &HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data->Condition))
		{
			bool bWriteObjectProperty = true;
			APawn* Value;

			{
				improbable::unreal::UnrealObjectRef ObjectRef = (*Update.field_instigator().data());
				check(ObjectRef != SpatialConstants::UNRESOLVED_OBJECT_REF);
				if (ObjectRef == SpatialConstants::NULL_OBJECT_REF)
				{
					Value = nullptr;
				}
				else
				{
					FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(ObjectRef);
					if (NetGUID.IsValid())
					{
						UObject* Object_Raw = PackageMap->GetObjectFromNetGUID(NetGUID, true);
						checkf(Object_Raw, TEXT("An object ref %s should map to a valid object."), *ObjectRefToString(ObjectRef));
						Value = dynamic_cast<APawn*>(Object_Raw);
						checkf(Value, TEXT("Object ref %s maps to object %s with the wrong class."), *ObjectRefToString(ObjectRef), *Object_Raw->GetFullName());
					}
					else
					{
						UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received unresolved object property. Value: %s. actor %s (%llu), property %s (handle %d)"),
							*Interop->GetSpatialOS()->GetWorkerId(),
							*ObjectRefToString(ObjectRef),
							*ActorChannel->Actor->GetName(),
							ActorChannel->GetEntityId(),
							*Data->Property->GetName(),
							Handle);
						bWriteObjectProperty = false;
						Interop->QueueIncomingObjectUpdate_Internal(ObjectRef, ActorChannel, Cast<UObjectPropertyBase>(Data->Property), Handle);
					}
				}
			}

			if (bWriteObjectProperty)
			{
				OutputWriter.SerializeProperty(Handle, Data->Property, &Value);
				UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
					*Interop->GetSpatialOS()->GetWorkerId(),
					*ActorChannel->Actor->GetName(),
					ActorChannel->GetEntityId(),
					*Data->Property->GetName(),
					Handle);
			}
		}
	}
	if (!Update.field_score().empty())
	{
		// field_score
		uint32 Handle = 16;
		const FRepHandleData* Data = &HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data->Condition))
		{
			float Value;

			Value = (*Update.field_score().data());

			OutputWriter.SerializeProperty(Handle, Data->Property, &Value);
			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_ping().empty())
	{
		// field_ping
		uint32 Handle = 17;
		const FRepHandleData* Data = &HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data->Condition))
		{
			uint8 Value;

			Value = uint8(uint8((*Update.field_ping().data())));

			OutputWriter.SerializeProperty(Handle, Data->Property, &Value);
			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_playername().empty())
	{
		// field_playername
		uint32 Handle = 18;
		const FRepHandleData* Data = &HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data->Condition))
		{
			FString Value;

			Value = FString(UTF8_TO_TCHAR((*Update.field_playername().data()).c_str()));

			OutputWriter.SerializeProperty(Handle, Data->Property, &Value);
			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_playerid().empty())
	{
		// field_playerid
		uint32 Handle = 19;
		const FRepHandleData* Data = &HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data->Condition))
		{
			int32 Value;

			Value = (*Update.field_playerid().data());

			OutputWriter.SerializeProperty(Handle, Data->Property, &Value);
			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_bfrompreviouslevel().empty())
	{
		// field_bfrompreviouslevel
		uint32 Handle = 20;
		const FRepHandleData* Data = &HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data->Condition))
		{
			uint8 Value;

			Value = (*Update.field_bfrompreviouslevel().data());
			// Because Unreal will look for a specific bit in the serialization function below, we simply set all bits.
			// We will soon move away from using bunches when receiving Spatial updates.
			if (Value)
			{
				Value = 0xFF;
			}

			OutputWriter.SerializeProperty(Handle, Data->Property, &Value);
			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_bisabot().empty())
	{
		// field_bisabot
		uint32 Handle = 21;
		const FRepHandleData* Data = &HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data->Condition))
		{
			uint8 Value;

			Value = (*Update.field_bisabot().data());
			// Because Unreal will look for a specific bit in the serialization function below, we simply set all bits.
			// We will soon move away from using bunches when receiving Spatial updates.
			if (Value)
			{
				Value = 0xFF;
			}

			OutputWriter.SerializeProperty(Handle, Data->Property, &Value);
			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_bisinactive().empty())
	{
		// field_bisinactive
		uint32 Handle = 22;
		const FRepHandleData* Data = &HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data->Condition))
		{
			uint8 Value;

			Value = (*Update.field_bisinactive().data());
			// Because Unreal will look for a specific bit in the serialization function below, we simply set all bits.
			// We will soon move away from using bunches when receiving Spatial updates.
			if (Value)
			{
				Value = 0xFF;
			}

			OutputWriter.SerializeProperty(Handle, Data->Property, &Value);
			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_bisspectator().empty())
	{
		// field_bisspectator
		uint32 Handle = 23;
		const FRepHandleData* Data = &HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data->Condition))
		{
			uint8 Value;

			Value = (*Update.field_bisspectator().data());
			// Because Unreal will look for a specific bit in the serialization function below, we simply set all bits.
			// We will soon move away from using bunches when receiving Spatial updates.
			if (Value)
			{
				Value = 0xFF;
			}

			OutputWriter.SerializeProperty(Handle, Data->Property, &Value);
			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_bonlyspectator().empty())
	{
		// field_bonlyspectator
		uint32 Handle = 24;
		const FRepHandleData* Data = &HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data->Condition))
		{
			uint8 Value;

			Value = (*Update.field_bonlyspectator().data());
			// Because Unreal will look for a specific bit in the serialization function below, we simply set all bits.
			// We will soon move away from using bunches when receiving Spatial updates.
			if (Value)
			{
				Value = 0xFF;
			}

			OutputWriter.SerializeProperty(Handle, Data->Property, &Value);
			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_starttime().empty())
	{
		// field_starttime
		uint32 Handle = 25;
		const FRepHandleData* Data = &HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data->Condition))
		{
			int32 Value;

			Value = (*Update.field_starttime().data());

			OutputWriter.SerializeProperty(Handle, Data->Property, &Value);
			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_uniqueid().empty())
	{
		// field_uniqueid
		uint32 Handle = 26;
		const FRepHandleData* Data = &HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data->Condition))
		{
			FUniqueNetIdRepl Value;

			{
				auto& ValueDataStr = (*Update.field_uniqueid().data());
				TArray<uint8> ValueData;
				ValueData.Append((uint8*)ValueDataStr.data(), ValueDataStr.size());
				FMemoryReader ValueDataReader(ValueData);
				bool bSuccess;
				Value.NetSerialize(ValueDataReader, nullptr, bSuccess);
			}

			OutputWriter.SerializeProperty(Handle, Data->Property, &Value);
			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data->Property->GetName(),
				Handle);
		}
	}
	Interop->ReceiveSpatialUpdate(ActorChannel, OutputWriter.GetNetBitWriter());
}
