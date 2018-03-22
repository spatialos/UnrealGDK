// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// Note that this file has been generated automatically

#include "SpatialTypeBinding_Character.h"
#include "Engine.h"

#include "SpatialOS.h"
#include "EntityBuilder.h"

#include "../SpatialConstants.h"
#include "../SpatialConditionMapFilter.h"
#include "../SpatialUnrealObjectRef.h"
#include "../SpatialActorChannel.h"
#include "../SpatialPackageMapClient.h"
#include "../SpatialNetDriver.h"
#include "../SpatialInterop.h"

#include "UnrealCharacterSingleClientRepDataAddComponentOp.h"
#include "UnrealCharacterMultiClientRepDataAddComponentOp.h"

const FRepHandlePropertyMap& USpatialTypeBinding_Character::GetRepHandlePropertyMap() const
{
	static FRepHandlePropertyMap HandleToPropertyMap;
	if (HandleToPropertyMap.Num() == 0)
	{
		UClass* Class = FindObject<UClass>(ANY_PACKAGE, TEXT("Character"));
		HandleToPropertyMap.Add(1, FRepHandleData(Class, {"bHidden"}, COND_None, REPNOTIFY_OnChanged));
		HandleToPropertyMap.Add(2, FRepHandleData(Class, {"bReplicateMovement"}, COND_None, REPNOTIFY_OnChanged));
		HandleToPropertyMap.Add(3, FRepHandleData(Class, {"bTearOff"}, COND_None, REPNOTIFY_OnChanged));
		HandleToPropertyMap.Add(4, FRepHandleData(Class, {"RemoteRole"}, COND_None, REPNOTIFY_OnChanged));
		HandleToPropertyMap.Add(5, FRepHandleData(Class, {"Owner"}, COND_None, REPNOTIFY_OnChanged));
		HandleToPropertyMap.Add(6, FRepHandleData(Class, {"ReplicatedMovement"}, COND_SimulatedOrPhysicsNoReplay, REPNOTIFY_OnChanged));
		HandleToPropertyMap.Add(7, FRepHandleData(Class, {"AttachmentReplication", "AttachParent"}, COND_Custom, REPNOTIFY_OnChanged));
		HandleToPropertyMap.Add(8, FRepHandleData(Class, {"AttachmentReplication", "LocationOffset"}, COND_Custom, REPNOTIFY_OnChanged));
		HandleToPropertyMap.Add(9, FRepHandleData(Class, {"AttachmentReplication", "RelativeScale3D"}, COND_Custom, REPNOTIFY_OnChanged));
		HandleToPropertyMap.Add(10, FRepHandleData(Class, {"AttachmentReplication", "RotationOffset"}, COND_Custom, REPNOTIFY_OnChanged));
		HandleToPropertyMap.Add(11, FRepHandleData(Class, {"AttachmentReplication", "AttachSocket"}, COND_Custom, REPNOTIFY_OnChanged));
		HandleToPropertyMap.Add(12, FRepHandleData(Class, {"AttachmentReplication", "AttachComponent"}, COND_Custom, REPNOTIFY_OnChanged));
		HandleToPropertyMap.Add(13, FRepHandleData(Class, {"Role"}, COND_None, REPNOTIFY_OnChanged));
		HandleToPropertyMap.Add(14, FRepHandleData(Class, {"bCanBeDamaged"}, COND_None, REPNOTIFY_OnChanged));
		HandleToPropertyMap.Add(15, FRepHandleData(Class, {"Instigator"}, COND_None, REPNOTIFY_OnChanged));
		HandleToPropertyMap.Add(16, FRepHandleData(Class, {"PlayerState"}, COND_None, REPNOTIFY_OnChanged));
		HandleToPropertyMap.Add(17, FRepHandleData(Class, {"RemoteViewPitch"}, COND_SkipOwner, REPNOTIFY_OnChanged));
		HandleToPropertyMap.Add(18, FRepHandleData(Class, {"Controller"}, COND_None, REPNOTIFY_OnChanged));
		HandleToPropertyMap.Add(19, FRepHandleData(Class, {"ReplicatedBasedMovement", "MovementBase"}, COND_SimulatedOnly, REPNOTIFY_OnChanged));
		HandleToPropertyMap.Add(20, FRepHandleData(Class, {"ReplicatedBasedMovement", "BoneName"}, COND_SimulatedOnly, REPNOTIFY_OnChanged));
		HandleToPropertyMap.Add(21, FRepHandleData(Class, {"ReplicatedBasedMovement", "Location"}, COND_SimulatedOnly, REPNOTIFY_OnChanged));
		HandleToPropertyMap.Add(22, FRepHandleData(Class, {"ReplicatedBasedMovement", "Rotation"}, COND_SimulatedOnly, REPNOTIFY_OnChanged));
		HandleToPropertyMap.Add(23, FRepHandleData(Class, {"ReplicatedBasedMovement", "bServerHasBaseComponent"}, COND_SimulatedOnly, REPNOTIFY_OnChanged));
		HandleToPropertyMap.Add(24, FRepHandleData(Class, {"ReplicatedBasedMovement", "bRelativeRotation"}, COND_SimulatedOnly, REPNOTIFY_OnChanged));
		HandleToPropertyMap.Add(25, FRepHandleData(Class, {"ReplicatedBasedMovement", "bServerHasVelocity"}, COND_SimulatedOnly, REPNOTIFY_OnChanged));
		HandleToPropertyMap.Add(26, FRepHandleData(Class, {"AnimRootMotionTranslationScale"}, COND_SimulatedOnly, REPNOTIFY_OnChanged));
		HandleToPropertyMap.Add(27, FRepHandleData(Class, {"ReplicatedServerLastTransformUpdateTimeStamp"}, COND_SimulatedOnlyNoReplay, REPNOTIFY_OnChanged));
		HandleToPropertyMap.Add(28, FRepHandleData(Class, {"ReplicatedMovementMode"}, COND_SimulatedOnly, REPNOTIFY_OnChanged));
		HandleToPropertyMap.Add(29, FRepHandleData(Class, {"bIsCrouched"}, COND_SimulatedOnly, REPNOTIFY_OnChanged));
		HandleToPropertyMap.Add(30, FRepHandleData(Class, {"JumpMaxHoldTime"}, COND_None, REPNOTIFY_OnChanged));
		HandleToPropertyMap.Add(31, FRepHandleData(Class, {"JumpMaxCount"}, COND_None, REPNOTIFY_OnChanged));
		HandleToPropertyMap.Add(32, FRepHandleData(Class, {"RepRootMotion", "bIsActive"}, COND_SimulatedOnlyNoReplay, REPNOTIFY_OnChanged));
		HandleToPropertyMap.Add(33, FRepHandleData(Class, {"RepRootMotion", "AnimMontage"}, COND_SimulatedOnlyNoReplay, REPNOTIFY_OnChanged));
		HandleToPropertyMap.Add(34, FRepHandleData(Class, {"RepRootMotion", "Position"}, COND_SimulatedOnlyNoReplay, REPNOTIFY_OnChanged));
		HandleToPropertyMap.Add(35, FRepHandleData(Class, {"RepRootMotion", "Location"}, COND_SimulatedOnlyNoReplay, REPNOTIFY_OnChanged));
		HandleToPropertyMap.Add(36, FRepHandleData(Class, {"RepRootMotion", "Rotation"}, COND_SimulatedOnlyNoReplay, REPNOTIFY_OnChanged));
		HandleToPropertyMap.Add(37, FRepHandleData(Class, {"RepRootMotion", "MovementBase"}, COND_SimulatedOnlyNoReplay, REPNOTIFY_OnChanged));
		HandleToPropertyMap.Add(38, FRepHandleData(Class, {"RepRootMotion", "MovementBaseBoneName"}, COND_SimulatedOnlyNoReplay, REPNOTIFY_OnChanged));
		HandleToPropertyMap.Add(39, FRepHandleData(Class, {"RepRootMotion", "bRelativePosition"}, COND_SimulatedOnlyNoReplay, REPNOTIFY_OnChanged));
		HandleToPropertyMap.Add(40, FRepHandleData(Class, {"RepRootMotion", "bRelativeRotation"}, COND_SimulatedOnlyNoReplay, REPNOTIFY_OnChanged));
		HandleToPropertyMap.Add(41, FRepHandleData(Class, {"RepRootMotion", "AuthoritativeRootMotion"}, COND_SimulatedOnlyNoReplay, REPNOTIFY_OnChanged));
		HandleToPropertyMap.Add(42, FRepHandleData(Class, {"RepRootMotion", "Acceleration"}, COND_SimulatedOnlyNoReplay, REPNOTIFY_OnChanged));
		HandleToPropertyMap.Add(43, FRepHandleData(Class, {"RepRootMotion", "LinearVelocity"}, COND_SimulatedOnlyNoReplay, REPNOTIFY_OnChanged));
	}
	return HandleToPropertyMap;
}

const FMigratableHandlePropertyMap& USpatialTypeBinding_Character::GetMigratableHandlePropertyMap() const
{
	static FMigratableHandlePropertyMap HandleToPropertyMap;
	if (HandleToPropertyMap.Num() == 0)
	{
		UClass* Class = FindObject<UClass>(ANY_PACKAGE, TEXT("Character"));
		HandleToPropertyMap.Add(1, FMigratableHandleData(Class, {"CharacterMovement", "GroundMovementMode"}));
		HandleToPropertyMap.Add(2, FMigratableHandleData(Class, {"CharacterMovement", "MovementMode"}));
		HandleToPropertyMap.Add(3, FMigratableHandleData(Class, {"CharacterMovement", "CustomMovementMode"}));
	}
	return HandleToPropertyMap;
}

UClass* USpatialTypeBinding_Character::GetBoundClass() const
{
	return ACharacter::StaticClass();
}

void USpatialTypeBinding_Character::Init(USpatialInterop* InInterop, USpatialPackageMapClient* InPackageMap)
{
	Super::Init(InInterop, InPackageMap);

	RPCToSenderMap.Emplace("RootMotionDebugClientPrintOnScreen", &USpatialTypeBinding_Character::RootMotionDebugClientPrintOnScreen_SendCommand);
	RPCToSenderMap.Emplace("ClientCheatWalk", &USpatialTypeBinding_Character::ClientCheatWalk_SendCommand);
	RPCToSenderMap.Emplace("ClientCheatGhost", &USpatialTypeBinding_Character::ClientCheatGhost_SendCommand);
	RPCToSenderMap.Emplace("ClientCheatFly", &USpatialTypeBinding_Character::ClientCheatFly_SendCommand);
	RPCToSenderMap.Emplace("ClientVeryShortAdjustPosition", &USpatialTypeBinding_Character::ClientVeryShortAdjustPosition_SendCommand);
	RPCToSenderMap.Emplace("ClientAdjustRootMotionSourcePosition", &USpatialTypeBinding_Character::ClientAdjustRootMotionSourcePosition_SendCommand);
	RPCToSenderMap.Emplace("ClientAdjustRootMotionPosition", &USpatialTypeBinding_Character::ClientAdjustRootMotionPosition_SendCommand);
	RPCToSenderMap.Emplace("ClientAdjustPosition", &USpatialTypeBinding_Character::ClientAdjustPosition_SendCommand);
	RPCToSenderMap.Emplace("ClientAckGoodMove", &USpatialTypeBinding_Character::ClientAckGoodMove_SendCommand);
	RPCToSenderMap.Emplace("ServerMoveOld", &USpatialTypeBinding_Character::ServerMoveOld_SendCommand);
	RPCToSenderMap.Emplace("ServerMoveDualHybridRootMotion", &USpatialTypeBinding_Character::ServerMoveDualHybridRootMotion_SendCommand);
	RPCToSenderMap.Emplace("ServerMoveDual", &USpatialTypeBinding_Character::ServerMoveDual_SendCommand);
	RPCToSenderMap.Emplace("ServerMove", &USpatialTypeBinding_Character::ServerMove_SendCommand);
}

void USpatialTypeBinding_Character::BindToView()
{
	TSharedPtr<worker::View> View = Interop->GetSpatialOS()->GetView().Pin();
	ViewCallbacks.Init(View);

	if (Interop->GetNetDriver()->GetNetMode() == NM_Client)
	{
		ViewCallbacks.Add(View->OnComponentUpdate<improbable::unreal::UnrealCharacterSingleClientRepData>([this](
			const worker::ComponentUpdateOp<improbable::unreal::UnrealCharacterSingleClientRepData>& Op)
		{
			USpatialActorChannel* ActorChannel = Interop->GetActorChannelByEntityId(Op.EntityId);
			if (ActorChannel)
			{
				ReceiveUpdate_SingleClient(ActorChannel, Op.Update);
			}
			else
			{
				Op.Update.ApplyTo(PendingSingleClientData.FindOrAdd(Op.EntityId));
			}
		}));
		ViewCallbacks.Add(View->OnComponentUpdate<improbable::unreal::UnrealCharacterMultiClientRepData>([this](
			const worker::ComponentUpdateOp<improbable::unreal::UnrealCharacterMultiClientRepData>& Op)
		{
			USpatialActorChannel* ActorChannel = Interop->GetActorChannelByEntityId(Op.EntityId);
			if (ActorChannel)
			{
				ReceiveUpdate_MultiClient(ActorChannel, Op.Update);
			}
			else
			{
				Op.Update.ApplyTo(PendingMultiClientData.FindOrAdd(Op.EntityId));
			}
		}));
	}

	using ClientRPCCommandTypes = improbable::unreal::UnrealCharacterClientRPCs::Commands;
	ViewCallbacks.Add(View->OnCommandRequest<ClientRPCCommandTypes::Rootmotiondebugclientprintonscreen>(std::bind(&USpatialTypeBinding_Character::RootMotionDebugClientPrintOnScreen_OnCommandRequest, this, std::placeholders::_1)));
	ViewCallbacks.Add(View->OnCommandRequest<ClientRPCCommandTypes::Clientcheatwalk>(std::bind(&USpatialTypeBinding_Character::ClientCheatWalk_OnCommandRequest, this, std::placeholders::_1)));
	ViewCallbacks.Add(View->OnCommandRequest<ClientRPCCommandTypes::Clientcheatghost>(std::bind(&USpatialTypeBinding_Character::ClientCheatGhost_OnCommandRequest, this, std::placeholders::_1)));
	ViewCallbacks.Add(View->OnCommandRequest<ClientRPCCommandTypes::Clientcheatfly>(std::bind(&USpatialTypeBinding_Character::ClientCheatFly_OnCommandRequest, this, std::placeholders::_1)));
	ViewCallbacks.Add(View->OnCommandRequest<ClientRPCCommandTypes::Clientveryshortadjustposition>(std::bind(&USpatialTypeBinding_Character::ClientVeryShortAdjustPosition_OnCommandRequest, this, std::placeholders::_1)));
	ViewCallbacks.Add(View->OnCommandRequest<ClientRPCCommandTypes::Clientadjustrootmotionsourceposition>(std::bind(&USpatialTypeBinding_Character::ClientAdjustRootMotionSourcePosition_OnCommandRequest, this, std::placeholders::_1)));
	ViewCallbacks.Add(View->OnCommandRequest<ClientRPCCommandTypes::Clientadjustrootmotionposition>(std::bind(&USpatialTypeBinding_Character::ClientAdjustRootMotionPosition_OnCommandRequest, this, std::placeholders::_1)));
	ViewCallbacks.Add(View->OnCommandRequest<ClientRPCCommandTypes::Clientadjustposition>(std::bind(&USpatialTypeBinding_Character::ClientAdjustPosition_OnCommandRequest, this, std::placeholders::_1)));
	ViewCallbacks.Add(View->OnCommandRequest<ClientRPCCommandTypes::Clientackgoodmove>(std::bind(&USpatialTypeBinding_Character::ClientAckGoodMove_OnCommandRequest, this, std::placeholders::_1)));
	ViewCallbacks.Add(View->OnCommandResponse<ClientRPCCommandTypes::Rootmotiondebugclientprintonscreen>(std::bind(&USpatialTypeBinding_Character::RootMotionDebugClientPrintOnScreen_OnCommandResponse, this, std::placeholders::_1)));
	ViewCallbacks.Add(View->OnCommandResponse<ClientRPCCommandTypes::Clientcheatwalk>(std::bind(&USpatialTypeBinding_Character::ClientCheatWalk_OnCommandResponse, this, std::placeholders::_1)));
	ViewCallbacks.Add(View->OnCommandResponse<ClientRPCCommandTypes::Clientcheatghost>(std::bind(&USpatialTypeBinding_Character::ClientCheatGhost_OnCommandResponse, this, std::placeholders::_1)));
	ViewCallbacks.Add(View->OnCommandResponse<ClientRPCCommandTypes::Clientcheatfly>(std::bind(&USpatialTypeBinding_Character::ClientCheatFly_OnCommandResponse, this, std::placeholders::_1)));
	ViewCallbacks.Add(View->OnCommandResponse<ClientRPCCommandTypes::Clientveryshortadjustposition>(std::bind(&USpatialTypeBinding_Character::ClientVeryShortAdjustPosition_OnCommandResponse, this, std::placeholders::_1)));
	ViewCallbacks.Add(View->OnCommandResponse<ClientRPCCommandTypes::Clientadjustrootmotionsourceposition>(std::bind(&USpatialTypeBinding_Character::ClientAdjustRootMotionSourcePosition_OnCommandResponse, this, std::placeholders::_1)));
	ViewCallbacks.Add(View->OnCommandResponse<ClientRPCCommandTypes::Clientadjustrootmotionposition>(std::bind(&USpatialTypeBinding_Character::ClientAdjustRootMotionPosition_OnCommandResponse, this, std::placeholders::_1)));
	ViewCallbacks.Add(View->OnCommandResponse<ClientRPCCommandTypes::Clientadjustposition>(std::bind(&USpatialTypeBinding_Character::ClientAdjustPosition_OnCommandResponse, this, std::placeholders::_1)));
	ViewCallbacks.Add(View->OnCommandResponse<ClientRPCCommandTypes::Clientackgoodmove>(std::bind(&USpatialTypeBinding_Character::ClientAckGoodMove_OnCommandResponse, this, std::placeholders::_1)));

	using ServerRPCCommandTypes = improbable::unreal::UnrealCharacterServerRPCs::Commands;
	ViewCallbacks.Add(View->OnCommandRequest<ServerRPCCommandTypes::Servermoveold>(std::bind(&USpatialTypeBinding_Character::ServerMoveOld_OnCommandRequest, this, std::placeholders::_1)));
	ViewCallbacks.Add(View->OnCommandRequest<ServerRPCCommandTypes::Servermovedualhybridrootmotion>(std::bind(&USpatialTypeBinding_Character::ServerMoveDualHybridRootMotion_OnCommandRequest, this, std::placeholders::_1)));
	ViewCallbacks.Add(View->OnCommandRequest<ServerRPCCommandTypes::Servermovedual>(std::bind(&USpatialTypeBinding_Character::ServerMoveDual_OnCommandRequest, this, std::placeholders::_1)));
	ViewCallbacks.Add(View->OnCommandRequest<ServerRPCCommandTypes::Servermove>(std::bind(&USpatialTypeBinding_Character::ServerMove_OnCommandRequest, this, std::placeholders::_1)));
	ViewCallbacks.Add(View->OnCommandResponse<ServerRPCCommandTypes::Servermoveold>(std::bind(&USpatialTypeBinding_Character::ServerMoveOld_OnCommandResponse, this, std::placeholders::_1)));
	ViewCallbacks.Add(View->OnCommandResponse<ServerRPCCommandTypes::Servermovedualhybridrootmotion>(std::bind(&USpatialTypeBinding_Character::ServerMoveDualHybridRootMotion_OnCommandResponse, this, std::placeholders::_1)));
	ViewCallbacks.Add(View->OnCommandResponse<ServerRPCCommandTypes::Servermovedual>(std::bind(&USpatialTypeBinding_Character::ServerMoveDual_OnCommandResponse, this, std::placeholders::_1)));
	ViewCallbacks.Add(View->OnCommandResponse<ServerRPCCommandTypes::Servermove>(std::bind(&USpatialTypeBinding_Character::ServerMove_OnCommandResponse, this, std::placeholders::_1)));
}

void USpatialTypeBinding_Character::UnbindFromView()
{
	ViewCallbacks.Reset();
}

worker::ComponentId USpatialTypeBinding_Character::GetReplicatedGroupComponentId(EReplicatedPropertyGroup Group) const
{
	switch (Group)
	{
	case GROUP_SingleClient:
		return improbable::unreal::UnrealCharacterSingleClientRepData::ComponentId;
	case GROUP_MultiClient:
		return improbable::unreal::UnrealCharacterMultiClientRepData::ComponentId;
	default:
		checkNoEntry();
		return 0;
	}
}

worker::Entity USpatialTypeBinding_Character::CreateActorEntity(const FString& ClientWorkerId, const FVector& Position, const FString& Metadata, const FPropertyChangeState& InitialChanges, USpatialActorChannel* Channel) const
{
	// Setup initial data.
	improbable::unreal::UnrealCharacterSingleClientRepData::Data SingleClientData;
	improbable::unreal::UnrealCharacterSingleClientRepData::Update SingleClientUpdate;
	bool bSingleClientUpdateChanged = false;
	improbable::unreal::UnrealCharacterMultiClientRepData::Data MultiClientData;
	improbable::unreal::UnrealCharacterMultiClientRepData::Update MultiClientUpdate;
	bool bMultiClientUpdateChanged = false;
	improbable::unreal::UnrealCharacterMigratableData::Data MigratableData;
	improbable::unreal::UnrealCharacterMigratableData::Update MigratableDataUpdate;
	bool bMigratableDataUpdateChanged = false;
	BuildSpatialComponentUpdate(InitialChanges, Channel, SingleClientUpdate, bSingleClientUpdateChanged, MultiClientUpdate, bMultiClientUpdateChanged, MigratableDataUpdate, bMigratableDataUpdateChanged);
	SingleClientUpdate.ApplyTo(SingleClientData);
	MultiClientUpdate.ApplyTo(MultiClientData);
	MigratableDataUpdate.ApplyTo(MigratableData);

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

	uint32 CurrentOffset = 1;
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
	const improbable::Coordinates SpatialPosition = SpatialConstants::LocationToSpatialOSCoordinates(Position);
	return improbable::unreal::FEntityBuilder::Begin()
		.AddPositionComponent(improbable::Position::Data{SpatialPosition}, WorkersOnly)
		.AddMetadataComponent(improbable::Metadata::Data{TCHAR_TO_UTF8(*Metadata)})
		.SetPersistence(true)
		.SetReadAcl(AnyUnrealWorkerOrClient)
		.AddComponent<improbable::unreal::UnrealMetadata>(UnrealMetadata, WorkersOnly)
		.AddComponent<improbable::unreal::UnrealCharacterSingleClientRepData>(SingleClientData, WorkersOnly)
		.AddComponent<improbable::unreal::UnrealCharacterMultiClientRepData>(MultiClientData, WorkersOnly)
		.AddComponent<improbable::unreal::UnrealCharacterMigratableData>(MigratableData, WorkersOnly)
		.AddComponent<improbable::unreal::UnrealCharacterClientRPCs>(improbable::unreal::UnrealCharacterClientRPCs::Data{}, OwningClientOnly)
		.AddComponent<improbable::unreal::UnrealCharacterServerRPCs>(improbable::unreal::UnrealCharacterServerRPCs::Data{}, WorkersOnly)
		.AddComponent<nuf::PossessPawn>(nuf::PossessPawn::Data{}, WorkersOnly)
		.Build();
}

void USpatialTypeBinding_Character::SendComponentUpdates(const FPropertyChangeState& Changes, USpatialActorChannel* Channel, const FEntityId& EntityId) const
{
	// Build SpatialOS updates.
	improbable::unreal::UnrealCharacterSingleClientRepData::Update SingleClientUpdate;
	bool bSingleClientUpdateChanged = false;
	improbable::unreal::UnrealCharacterMultiClientRepData::Update MultiClientUpdate;
	bool bMultiClientUpdateChanged = false;
	improbable::unreal::UnrealCharacterMigratableData::Update MigratableDataUpdate;
	bool bMigratableDataUpdateChanged = false;
	BuildSpatialComponentUpdate(Changes, Channel, SingleClientUpdate, bSingleClientUpdateChanged, MultiClientUpdate, bMultiClientUpdateChanged, MigratableDataUpdate, bMigratableDataUpdateChanged);

	// Send SpatialOS updates if anything changed.
	TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
	if (bSingleClientUpdateChanged)
	{
		Connection->SendComponentUpdate<improbable::unreal::UnrealCharacterSingleClientRepData>(EntityId.ToSpatialEntityId(), SingleClientUpdate);
	}
	if (bMultiClientUpdateChanged)
	{
		Connection->SendComponentUpdate<improbable::unreal::UnrealCharacterMultiClientRepData>(EntityId.ToSpatialEntityId(), MultiClientUpdate);
	}
	if (bMigratableDataUpdateChanged)
	{
		Connection->SendComponentUpdate<improbable::unreal::UnrealCharacterMigratableData>(EntityId.ToSpatialEntityId(), MigratableDataUpdate);
	}
}

void USpatialTypeBinding_Character::SendRPCCommand(UObject* TargetObject, const UFunction* const Function, FFrame* const Frame)
{
	TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
	auto SenderFuncIterator = RPCToSenderMap.Find(Function->GetFName());
	checkf(*SenderFuncIterator, TEXT("Sender for %s has not been registered with RPCToSenderMap."), *Function->GetFName().ToString());
	(this->*(*SenderFuncIterator))(Connection.Get(), Frame, TargetObject);
}

void USpatialTypeBinding_Character::ReceiveAddComponent(USpatialActorChannel* Channel, UAddComponentOpWrapperBase* AddComponentOp) const
{
	auto* SingleClientAddOp = Cast<UUnrealCharacterSingleClientRepDataAddComponentOp>(AddComponentOp);
	if (SingleClientAddOp)
	{
		auto Update = improbable::unreal::UnrealCharacterSingleClientRepData::Update::FromInitialData(*SingleClientAddOp->Data.data());
		ReceiveUpdate_SingleClient(Channel, Update);
	}
	auto* MultiClientAddOp = Cast<UUnrealCharacterMultiClientRepDataAddComponentOp>(AddComponentOp);
	if (MultiClientAddOp)
	{
		auto Update = improbable::unreal::UnrealCharacterMultiClientRepData::Update::FromInitialData(*MultiClientAddOp->Data.data());
		ReceiveUpdate_MultiClient(Channel, Update);
	}
}

void USpatialTypeBinding_Character::ApplyQueuedStateToChannel(USpatialActorChannel* ActorChannel)
{
	improbable::unreal::UnrealCharacterSingleClientRepData::Data* SingleClientData = PendingSingleClientData.Find(ActorChannel->GetEntityId());
	if (SingleClientData)
	{
		auto Update = improbable::unreal::UnrealCharacterSingleClientRepData::Update::FromInitialData(*SingleClientData);
		PendingSingleClientData.Remove(ActorChannel->GetEntityId());
		ReceiveUpdate_SingleClient(ActorChannel, Update);
	}
	improbable::unreal::UnrealCharacterMultiClientRepData::Data* MultiClientData = PendingMultiClientData.Find(ActorChannel->GetEntityId());
	if (MultiClientData)
	{
		auto Update = improbable::unreal::UnrealCharacterMultiClientRepData::Update::FromInitialData(*MultiClientData);
		PendingMultiClientData.Remove(ActorChannel->GetEntityId());
		ReceiveUpdate_MultiClient(ActorChannel, Update);
	}
}

void USpatialTypeBinding_Character::BuildSpatialComponentUpdate(
	const FPropertyChangeState& Changes,
	USpatialActorChannel* Channel,
	improbable::unreal::UnrealCharacterSingleClientRepData::Update& SingleClientUpdate,
	bool& bSingleClientUpdateChanged,
	improbable::unreal::UnrealCharacterMultiClientRepData::Update& MultiClientUpdate,
	bool& bMultiClientUpdateChanged,
	improbable::unreal::UnrealCharacterMigratableData::Update& MigratableDataUpdate,
	bool& bMigratableDataUpdateChanged) const
{
	const FRepHandlePropertyMap& RepPropertyMap = GetRepHandlePropertyMap();
	const FMigratableHandlePropertyMap& MigPropertyMap = GetMigratableHandlePropertyMap();

	// Populate the replicated data component updates from the replicated property changelist.
	FChangelistIterator ChangelistIterator(Changes.RepChanged, 0);
	FRepHandleIterator HandleIterator(ChangelistIterator, Changes.RepCmds, Changes.RepBaseHandleToCmdIndex, 0, 1, 0, Changes.RepCmds.Num() - 1);
	while (HandleIterator.NextHandle())
	{
		const FRepLayoutCmd& Cmd = Changes.RepCmds[HandleIterator.CmdIndex];
		const FRepHandleData& PropertyMapData = RepPropertyMap[HandleIterator.Handle];
		const uint8* Data = Changes.SourceData + HandleIterator.ArrayOffset + Cmd.Offset;
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending property update. actor %s (%lld), property %s (handle %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*Channel->Actor->GetName(),
			Channel->GetEntityId().ToSpatialEntityId(),
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

	// Populate the migrated data component update from the migrated property changelist.
	for (uint16 ChangedHandle : Changes.MigChanged)
	{
		const FMigratableHandleData& PropertyMapData = MigPropertyMap[ChangedHandle];
		const uint8* Data = PropertyMapData.GetPropertyData(Changes.SourceData);
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending migratable property update. actor %s (%lld), property %s (handle %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*Channel->Actor->GetName(),
			Channel->GetEntityId().ToSpatialEntityId(),
			*PropertyMapData.Property->GetName(),
			HandleIterator.Handle);
		ServerSendUpdate_Migratable(Data, ChangedHandle, PropertyMapData.Property, Channel, MigratableDataUpdate);
		bMigratableDataUpdateChanged = true;
	}
}

void USpatialTypeBinding_Character::ServerSendUpdate_SingleClient(const uint8* RESTRICT Data, int32 Handle, UProperty* Property, USpatialActorChannel* Channel, improbable::unreal::UnrealCharacterSingleClientRepData::Update& OutUpdate) const
{
}

void USpatialTypeBinding_Character::ServerSendUpdate_MultiClient(const uint8* RESTRICT Data, int32 Handle, UProperty* Property, USpatialActorChannel* Channel, improbable::unreal::UnrealCharacterMultiClientRepData::Update& OutUpdate) const
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
					Interop->QueueOutgoingObjectRepUpdate_Internal(Value, Channel, 5);
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
				Value.NetSerialize(ValueDataWriter, PackageMap, Success);
				OutUpdate.set_field_replicatedmovement(std::string(reinterpret_cast<char*>(ValueData.GetData()), ValueData.Num()));
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
					Interop->QueueOutgoingObjectRepUpdate_Internal(Value, Channel, 7);
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
					Interop->QueueOutgoingObjectRepUpdate_Internal(Value, Channel, 12);
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
					Interop->QueueOutgoingObjectRepUpdate_Internal(Value, Channel, 15);
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
		case 16: // field_playerstate
		{
			APlayerState* Value = *(reinterpret_cast<APlayerState* const*>(Data));

			if (Value != nullptr)
			{
				FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(Value);
				improbable::unreal::UnrealObjectRef ObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
				if (ObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
				{
					Interop->QueueOutgoingObjectRepUpdate_Internal(Value, Channel, 16);
				}
				else
				{
					OutUpdate.set_field_playerstate(ObjectRef);
				}
			}
			else
			{
				OutUpdate.set_field_playerstate(SpatialConstants::NULL_OBJECT_REF);
			}
			break;
		}
		case 17: // field_remoteviewpitch
		{
			uint8 Value = *(reinterpret_cast<uint8 const*>(Data));

			OutUpdate.set_field_remoteviewpitch(uint32_t(Value));
			break;
		}
		case 18: // field_controller
		{
			AController* Value = *(reinterpret_cast<AController* const*>(Data));

			if (Value != nullptr)
			{
				FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(Value);
				improbable::unreal::UnrealObjectRef ObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
				if (ObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
				{
					Interop->QueueOutgoingObjectRepUpdate_Internal(Value, Channel, 18);
				}
				else
				{
					OutUpdate.set_field_controller(ObjectRef);
				}
			}
			else
			{
				OutUpdate.set_field_controller(SpatialConstants::NULL_OBJECT_REF);
			}
			break;
		}
		case 19: // field_replicatedbasedmovement_movementbase
		{
			UPrimitiveComponent* Value = *(reinterpret_cast<UPrimitiveComponent* const*>(Data));

			if (Value != nullptr)
			{
				FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(Value);
				improbable::unreal::UnrealObjectRef ObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
				if (ObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
				{
					Interop->QueueOutgoingObjectRepUpdate_Internal(Value, Channel, 19);
				}
				else
				{
					OutUpdate.set_field_replicatedbasedmovement_movementbase(ObjectRef);
				}
			}
			else
			{
				OutUpdate.set_field_replicatedbasedmovement_movementbase(SpatialConstants::NULL_OBJECT_REF);
			}
			break;
		}
		case 20: // field_replicatedbasedmovement_bonename
		{
			FName Value = *(reinterpret_cast<FName const*>(Data));

			OutUpdate.set_field_replicatedbasedmovement_bonename(TCHAR_TO_UTF8(*Value.ToString()));
			break;
		}
		case 21: // field_replicatedbasedmovement_location
		{
			FVector_NetQuantize100 Value = *(reinterpret_cast<FVector_NetQuantize100 const*>(Data));

			OutUpdate.set_field_replicatedbasedmovement_location(improbable::Vector3f(Value.X, Value.Y, Value.Z));
			break;
		}
		case 22: // field_replicatedbasedmovement_rotation
		{
			FRotator Value = *(reinterpret_cast<FRotator const*>(Data));

			OutUpdate.set_field_replicatedbasedmovement_rotation(improbable::unreal::UnrealFRotator(Value.Yaw, Value.Pitch, Value.Roll));
			break;
		}
		case 23: // field_replicatedbasedmovement_bserverhasbasecomponent
		{
			bool Value = static_cast<UBoolProperty*>(Property)->GetPropertyValue(Data);

			OutUpdate.set_field_replicatedbasedmovement_bserverhasbasecomponent(Value);
			break;
		}
		case 24: // field_replicatedbasedmovement_brelativerotation
		{
			bool Value = static_cast<UBoolProperty*>(Property)->GetPropertyValue(Data);

			OutUpdate.set_field_replicatedbasedmovement_brelativerotation(Value);
			break;
		}
		case 25: // field_replicatedbasedmovement_bserverhasvelocity
		{
			bool Value = static_cast<UBoolProperty*>(Property)->GetPropertyValue(Data);

			OutUpdate.set_field_replicatedbasedmovement_bserverhasvelocity(Value);
			break;
		}
		case 26: // field_animrootmotiontranslationscale
		{
			float Value = *(reinterpret_cast<float const*>(Data));

			OutUpdate.set_field_animrootmotiontranslationscale(Value);
			break;
		}
		case 27: // field_replicatedserverlasttransformupdatetimestamp
		{
			float Value = *(reinterpret_cast<float const*>(Data));

			OutUpdate.set_field_replicatedserverlasttransformupdatetimestamp(Value);
			break;
		}
		case 28: // field_replicatedmovementmode
		{
			uint8 Value = *(reinterpret_cast<uint8 const*>(Data));

			OutUpdate.set_field_replicatedmovementmode(uint32_t(Value));
			break;
		}
		case 29: // field_biscrouched
		{
			bool Value = static_cast<UBoolProperty*>(Property)->GetPropertyValue(Data);

			OutUpdate.set_field_biscrouched(Value);
			break;
		}
		case 30: // field_jumpmaxholdtime
		{
			float Value = *(reinterpret_cast<float const*>(Data));

			OutUpdate.set_field_jumpmaxholdtime(Value);
			break;
		}
		case 31: // field_jumpmaxcount
		{
			int32 Value = *(reinterpret_cast<int32 const*>(Data));

			OutUpdate.set_field_jumpmaxcount(Value);
			break;
		}
		case 32: // field_reprootmotion_bisactive
		{
			bool Value = static_cast<UBoolProperty*>(Property)->GetPropertyValue(Data);

			OutUpdate.set_field_reprootmotion_bisactive(Value);
			break;
		}
		case 33: // field_reprootmotion_animmontage
		{
			UAnimMontage* Value = *(reinterpret_cast<UAnimMontage* const*>(Data));

			if (Value != nullptr)
			{
				FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(Value);
				improbable::unreal::UnrealObjectRef ObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
				if (ObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
				{
					Interop->QueueOutgoingObjectRepUpdate_Internal(Value, Channel, 33);
				}
				else
				{
					OutUpdate.set_field_reprootmotion_animmontage(ObjectRef);
				}
			}
			else
			{
				OutUpdate.set_field_reprootmotion_animmontage(SpatialConstants::NULL_OBJECT_REF);
			}
			break;
		}
		case 34: // field_reprootmotion_position
		{
			float Value = *(reinterpret_cast<float const*>(Data));

			OutUpdate.set_field_reprootmotion_position(Value);
			break;
		}
		case 35: // field_reprootmotion_location
		{
			FVector_NetQuantize100 Value = *(reinterpret_cast<FVector_NetQuantize100 const*>(Data));

			OutUpdate.set_field_reprootmotion_location(improbable::Vector3f(Value.X, Value.Y, Value.Z));
			break;
		}
		case 36: // field_reprootmotion_rotation
		{
			FRotator Value = *(reinterpret_cast<FRotator const*>(Data));

			OutUpdate.set_field_reprootmotion_rotation(improbable::unreal::UnrealFRotator(Value.Yaw, Value.Pitch, Value.Roll));
			break;
		}
		case 37: // field_reprootmotion_movementbase
		{
			UPrimitiveComponent* Value = *(reinterpret_cast<UPrimitiveComponent* const*>(Data));

			if (Value != nullptr)
			{
				FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(Value);
				improbable::unreal::UnrealObjectRef ObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
				if (ObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
				{
					Interop->QueueOutgoingObjectRepUpdate_Internal(Value, Channel, 37);
				}
				else
				{
					OutUpdate.set_field_reprootmotion_movementbase(ObjectRef);
				}
			}
			else
			{
				OutUpdate.set_field_reprootmotion_movementbase(SpatialConstants::NULL_OBJECT_REF);
			}
			break;
		}
		case 38: // field_reprootmotion_movementbasebonename
		{
			FName Value = *(reinterpret_cast<FName const*>(Data));

			OutUpdate.set_field_reprootmotion_movementbasebonename(TCHAR_TO_UTF8(*Value.ToString()));
			break;
		}
		case 39: // field_reprootmotion_brelativeposition
		{
			bool Value = static_cast<UBoolProperty*>(Property)->GetPropertyValue(Data);

			OutUpdate.set_field_reprootmotion_brelativeposition(Value);
			break;
		}
		case 40: // field_reprootmotion_brelativerotation
		{
			bool Value = static_cast<UBoolProperty*>(Property)->GetPropertyValue(Data);

			OutUpdate.set_field_reprootmotion_brelativerotation(Value);
			break;
		}
		case 41: // field_reprootmotion_authoritativerootmotion
		{
			FRootMotionSourceGroup Value = *(reinterpret_cast<FRootMotionSourceGroup const*>(Data));

			{
				TArray<uint8> ValueData;
				FMemoryWriter ValueDataWriter(ValueData);
				bool Success;
				Value.NetSerialize(ValueDataWriter, PackageMap, Success);
				OutUpdate.set_field_reprootmotion_authoritativerootmotion(std::string(reinterpret_cast<char*>(ValueData.GetData()), ValueData.Num()));
			}
			break;
		}
		case 42: // field_reprootmotion_acceleration
		{
			FVector_NetQuantize10 Value = *(reinterpret_cast<FVector_NetQuantize10 const*>(Data));

			OutUpdate.set_field_reprootmotion_acceleration(improbable::Vector3f(Value.X, Value.Y, Value.Z));
			break;
		}
		case 43: // field_reprootmotion_linearvelocity
		{
			FVector_NetQuantize10 Value = *(reinterpret_cast<FVector_NetQuantize10 const*>(Data));

			OutUpdate.set_field_reprootmotion_linearvelocity(improbable::Vector3f(Value.X, Value.Y, Value.Z));
			break;
		}
	default:
		checkf(false, TEXT("Unknown replication handle %d encountered when creating a SpatialOS update."));
		break;
	}
}

void USpatialTypeBinding_Character::ServerSendUpdate_Migratable(const uint8* RESTRICT Data, int32 Handle, UProperty* Property, USpatialActorChannel* Channel, improbable::unreal::UnrealCharacterMigratableData::Update& OutUpdate) const
{
	switch (Handle)
	{
		case 1: // field_charactermovement_groundmovementmode
		{
			TEnumAsByte<EMovementMode> Value = *(reinterpret_cast<TEnumAsByte<EMovementMode> const*>(Data));

			OutUpdate.set_field_charactermovement_groundmovementmode(uint32_t(Value));
			break;
		}
		case 2: // field_charactermovement_movementmode
		{
			TEnumAsByte<EMovementMode> Value = *(reinterpret_cast<TEnumAsByte<EMovementMode> const*>(Data));

			OutUpdate.set_field_charactermovement_movementmode(uint32_t(Value));
			break;
		}
		case 3: // field_charactermovement_custommovementmode
		{
			uint8 Value = *(reinterpret_cast<uint8 const*>(Data));

			OutUpdate.set_field_charactermovement_custommovementmode(uint32_t(Value));
			break;
		}
	default:
		checkf(false, TEXT("Unknown migration property handle %d encountered when creating a SpatialOS update."));
		break;
	}
}

void USpatialTypeBinding_Character::ReceiveUpdate_SingleClient(USpatialActorChannel* ActorChannel, const improbable::unreal::UnrealCharacterSingleClientRepData::Update& Update) const
{
	Interop->PreReceiveSpatialUpdate(ActorChannel);

	TArray<UProperty*> RepNotifies;
	Interop->PostReceiveSpatialUpdate(ActorChannel, RepNotifies);
}

void USpatialTypeBinding_Character::ReceiveUpdate_MultiClient(USpatialActorChannel* ActorChannel, const improbable::unreal::UnrealCharacterMultiClientRepData::Update& Update) const
{
	Interop->PreReceiveSpatialUpdate(ActorChannel);

	TArray<UProperty*> RepNotifies;
	const bool bIsServer = Interop->GetNetDriver()->IsServer();
	const bool bAutonomousProxy = ActorChannel->IsClientAutonomousProxy(improbable::unreal::UnrealCharacterClientRPCs::ComponentId);
	const FRepHandlePropertyMap& HandleToPropertyMap = GetRepHandlePropertyMap();
	FSpatialConditionMapFilter ConditionMap(ActorChannel, bAutonomousProxy);

	if (!Update.field_bhidden().empty())
	{
		// field_bhidden
		uint16 Handle = 1;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (bIsServer || ConditionMap.IsRelevant(RepData->Condition))
		{
			uint8* PropertyData = RepData->GetPropertyData(reinterpret_cast<uint8*>(ActorChannel->Actor));
			bool Value = static_cast<UBoolProperty*>(RepData->Property)->GetPropertyValue(PropertyData);

			Value = (*Update.field_bhidden().data());

			ApplyIncomingReplicatedPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received replicated property update. actor %s (%lld), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId().ToSpatialEntityId(),
				*RepData->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_breplicatemovement().empty())
	{
		// field_breplicatemovement
		uint16 Handle = 2;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (bIsServer || ConditionMap.IsRelevant(RepData->Condition))
		{
			uint8* PropertyData = RepData->GetPropertyData(reinterpret_cast<uint8*>(ActorChannel->Actor));
			bool Value = static_cast<UBoolProperty*>(RepData->Property)->GetPropertyValue(PropertyData);

			Value = (*Update.field_breplicatemovement().data());

			ApplyIncomingReplicatedPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received replicated property update. actor %s (%lld), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId().ToSpatialEntityId(),
				*RepData->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_btearoff().empty())
	{
		// field_btearoff
		uint16 Handle = 3;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (bIsServer || ConditionMap.IsRelevant(RepData->Condition))
		{
			uint8* PropertyData = RepData->GetPropertyData(reinterpret_cast<uint8*>(ActorChannel->Actor));
			bool Value = static_cast<UBoolProperty*>(RepData->Property)->GetPropertyValue(PropertyData);

			Value = (*Update.field_btearoff().data());

			ApplyIncomingReplicatedPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received replicated property update. actor %s (%lld), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId().ToSpatialEntityId(),
				*RepData->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_remoterole().empty())
	{
		// field_remoterole
		uint16 Handle = 4;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (bIsServer || ConditionMap.IsRelevant(RepData->Condition))
		{
			// On the client, we need to swap Role/RemoteRole.
			if (!bIsServer)
			{
				Handle = 13;
				RepData = &HandleToPropertyMap[Handle];
			}

			uint8* PropertyData = RepData->GetPropertyData(reinterpret_cast<uint8*>(ActorChannel->Actor));
			TEnumAsByte<ENetRole> Value = *(reinterpret_cast<TEnumAsByte<ENetRole> const*>(PropertyData));

			Value = TEnumAsByte<ENetRole>(uint8((*Update.field_remoterole().data())));

			// Downgrade role from AutonomousProxy to SimulatedProxy if we aren't authoritative over
			// the server RPCs component.
			if (!bIsServer && Value == ROLE_AutonomousProxy && !bAutonomousProxy)
			{
				Value = ROLE_SimulatedProxy;
			}

			ApplyIncomingReplicatedPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received replicated property update. actor %s (%lld), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId().ToSpatialEntityId(),
				*RepData->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_owner().empty())
	{
		// field_owner
		uint16 Handle = 5;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (bIsServer || ConditionMap.IsRelevant(RepData->Condition))
		{
			bool bWriteObjectProperty = true;
			uint8* PropertyData = RepData->GetPropertyData(reinterpret_cast<uint8*>(ActorChannel->Actor));
			AActor* Value = *(reinterpret_cast<AActor* const*>(PropertyData));

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
						UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received unresolved object property. Value: %s. actor %s (%lld), property %s (handle %d)"),
							*Interop->GetSpatialOS()->GetWorkerId(),
							*ObjectRefToString(ObjectRef),
							*ActorChannel->Actor->GetName(),
							ActorChannel->GetEntityId().ToSpatialEntityId(),
							*RepData->Property->GetName(),
							Handle);
						bWriteObjectProperty = false;
						Interop->QueueIncomingObjectRepUpdate_Internal(ObjectRef, ActorChannel, RepData);
					}
				}
			}

			if (bWriteObjectProperty)
			{
				ApplyIncomingReplicatedPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

				UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received replicated property update. actor %s (%lld), property %s (handle %d)"),
					*Interop->GetSpatialOS()->GetWorkerId(),
					*ActorChannel->Actor->GetName(),
					ActorChannel->GetEntityId().ToSpatialEntityId(),
					*RepData->Property->GetName(),
					Handle);
			}
		}
	}
	if (!Update.field_replicatedmovement().empty())
	{
		// field_replicatedmovement
		uint16 Handle = 6;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (bIsServer || ConditionMap.IsRelevant(RepData->Condition))
		{
			uint8* PropertyData = RepData->GetPropertyData(reinterpret_cast<uint8*>(ActorChannel->Actor));
			FRepMovement Value = *(reinterpret_cast<FRepMovement const*>(PropertyData));

			{
				auto& ValueDataStr = (*Update.field_replicatedmovement().data());
				TArray<uint8> ValueData;
				ValueData.Append(reinterpret_cast<const uint8*>(ValueDataStr.data()), ValueDataStr.size());
				FMemoryReader ValueDataReader(ValueData);
				bool bSuccess;
				Value.NetSerialize(ValueDataReader, PackageMap, bSuccess);
			}

			ApplyIncomingReplicatedPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received replicated property update. actor %s (%lld), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId().ToSpatialEntityId(),
				*RepData->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_attachmentreplication_attachparent().empty())
	{
		// field_attachmentreplication_attachparent
		uint16 Handle = 7;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (bIsServer || ConditionMap.IsRelevant(RepData->Condition))
		{
			bool bWriteObjectProperty = true;
			uint8* PropertyData = RepData->GetPropertyData(reinterpret_cast<uint8*>(ActorChannel->Actor));
			AActor* Value = *(reinterpret_cast<AActor* const*>(PropertyData));

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
						UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received unresolved object property. Value: %s. actor %s (%lld), property %s (handle %d)"),
							*Interop->GetSpatialOS()->GetWorkerId(),
							*ObjectRefToString(ObjectRef),
							*ActorChannel->Actor->GetName(),
							ActorChannel->GetEntityId().ToSpatialEntityId(),
							*RepData->Property->GetName(),
							Handle);
						bWriteObjectProperty = false;
						Interop->QueueIncomingObjectRepUpdate_Internal(ObjectRef, ActorChannel, RepData);
					}
				}
			}

			if (bWriteObjectProperty)
			{
				ApplyIncomingReplicatedPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

				UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received replicated property update. actor %s (%lld), property %s (handle %d)"),
					*Interop->GetSpatialOS()->GetWorkerId(),
					*ActorChannel->Actor->GetName(),
					ActorChannel->GetEntityId().ToSpatialEntityId(),
					*RepData->Property->GetName(),
					Handle);
			}
		}
	}
	if (!Update.field_attachmentreplication_locationoffset().empty())
	{
		// field_attachmentreplication_locationoffset
		uint16 Handle = 8;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (bIsServer || ConditionMap.IsRelevant(RepData->Condition))
		{
			uint8* PropertyData = RepData->GetPropertyData(reinterpret_cast<uint8*>(ActorChannel->Actor));
			FVector_NetQuantize100 Value = *(reinterpret_cast<FVector_NetQuantize100 const*>(PropertyData));

			{
				auto& Vector = (*Update.field_attachmentreplication_locationoffset().data());
				Value.X = Vector.x();
				Value.Y = Vector.y();
				Value.Z = Vector.z();
			}

			ApplyIncomingReplicatedPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received replicated property update. actor %s (%lld), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId().ToSpatialEntityId(),
				*RepData->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_attachmentreplication_relativescale3d().empty())
	{
		// field_attachmentreplication_relativescale3d
		uint16 Handle = 9;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (bIsServer || ConditionMap.IsRelevant(RepData->Condition))
		{
			uint8* PropertyData = RepData->GetPropertyData(reinterpret_cast<uint8*>(ActorChannel->Actor));
			FVector_NetQuantize100 Value = *(reinterpret_cast<FVector_NetQuantize100 const*>(PropertyData));

			{
				auto& Vector = (*Update.field_attachmentreplication_relativescale3d().data());
				Value.X = Vector.x();
				Value.Y = Vector.y();
				Value.Z = Vector.z();
			}

			ApplyIncomingReplicatedPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received replicated property update. actor %s (%lld), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId().ToSpatialEntityId(),
				*RepData->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_attachmentreplication_rotationoffset().empty())
	{
		// field_attachmentreplication_rotationoffset
		uint16 Handle = 10;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (bIsServer || ConditionMap.IsRelevant(RepData->Condition))
		{
			uint8* PropertyData = RepData->GetPropertyData(reinterpret_cast<uint8*>(ActorChannel->Actor));
			FRotator Value = *(reinterpret_cast<FRotator const*>(PropertyData));

			{
				auto& Rotator = (*Update.field_attachmentreplication_rotationoffset().data());
				Value.Yaw = Rotator.yaw();
				Value.Pitch = Rotator.pitch();
				Value.Roll = Rotator.roll();
			}

			ApplyIncomingReplicatedPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received replicated property update. actor %s (%lld), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId().ToSpatialEntityId(),
				*RepData->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_attachmentreplication_attachsocket().empty())
	{
		// field_attachmentreplication_attachsocket
		uint16 Handle = 11;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (bIsServer || ConditionMap.IsRelevant(RepData->Condition))
		{
			uint8* PropertyData = RepData->GetPropertyData(reinterpret_cast<uint8*>(ActorChannel->Actor));
			FName Value = *(reinterpret_cast<FName const*>(PropertyData));

			Value = FName(((*Update.field_attachmentreplication_attachsocket().data())).data());

			ApplyIncomingReplicatedPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received replicated property update. actor %s (%lld), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId().ToSpatialEntityId(),
				*RepData->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_attachmentreplication_attachcomponent().empty())
	{
		// field_attachmentreplication_attachcomponent
		uint16 Handle = 12;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (bIsServer || ConditionMap.IsRelevant(RepData->Condition))
		{
			bool bWriteObjectProperty = true;
			uint8* PropertyData = RepData->GetPropertyData(reinterpret_cast<uint8*>(ActorChannel->Actor));
			USceneComponent* Value = *(reinterpret_cast<USceneComponent* const*>(PropertyData));

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
						UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received unresolved object property. Value: %s. actor %s (%lld), property %s (handle %d)"),
							*Interop->GetSpatialOS()->GetWorkerId(),
							*ObjectRefToString(ObjectRef),
							*ActorChannel->Actor->GetName(),
							ActorChannel->GetEntityId().ToSpatialEntityId(),
							*RepData->Property->GetName(),
							Handle);
						bWriteObjectProperty = false;
						Interop->QueueIncomingObjectRepUpdate_Internal(ObjectRef, ActorChannel, RepData);
					}
				}
			}

			if (bWriteObjectProperty)
			{
				ApplyIncomingReplicatedPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

				UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received replicated property update. actor %s (%lld), property %s (handle %d)"),
					*Interop->GetSpatialOS()->GetWorkerId(),
					*ActorChannel->Actor->GetName(),
					ActorChannel->GetEntityId().ToSpatialEntityId(),
					*RepData->Property->GetName(),
					Handle);
			}
		}
	}
	if (!Update.field_role().empty())
	{
		// field_role
		uint16 Handle = 13;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (bIsServer || ConditionMap.IsRelevant(RepData->Condition))
		{
			// On the client, we need to swap Role/RemoteRole.
			if (!bIsServer)
			{
				Handle = 4;
				RepData = &HandleToPropertyMap[Handle];
			}

			uint8* PropertyData = RepData->GetPropertyData(reinterpret_cast<uint8*>(ActorChannel->Actor));
			TEnumAsByte<ENetRole> Value = *(reinterpret_cast<TEnumAsByte<ENetRole> const*>(PropertyData));

			Value = TEnumAsByte<ENetRole>(uint8((*Update.field_role().data())));

			ApplyIncomingReplicatedPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received replicated property update. actor %s (%lld), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId().ToSpatialEntityId(),
				*RepData->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_bcanbedamaged().empty())
	{
		// field_bcanbedamaged
		uint16 Handle = 14;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (bIsServer || ConditionMap.IsRelevant(RepData->Condition))
		{
			uint8* PropertyData = RepData->GetPropertyData(reinterpret_cast<uint8*>(ActorChannel->Actor));
			bool Value = static_cast<UBoolProperty*>(RepData->Property)->GetPropertyValue(PropertyData);

			Value = (*Update.field_bcanbedamaged().data());

			ApplyIncomingReplicatedPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received replicated property update. actor %s (%lld), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId().ToSpatialEntityId(),
				*RepData->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_instigator().empty())
	{
		// field_instigator
		uint16 Handle = 15;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (bIsServer || ConditionMap.IsRelevant(RepData->Condition))
		{
			bool bWriteObjectProperty = true;
			uint8* PropertyData = RepData->GetPropertyData(reinterpret_cast<uint8*>(ActorChannel->Actor));
			APawn* Value = *(reinterpret_cast<APawn* const*>(PropertyData));

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
						UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received unresolved object property. Value: %s. actor %s (%lld), property %s (handle %d)"),
							*Interop->GetSpatialOS()->GetWorkerId(),
							*ObjectRefToString(ObjectRef),
							*ActorChannel->Actor->GetName(),
							ActorChannel->GetEntityId().ToSpatialEntityId(),
							*RepData->Property->GetName(),
							Handle);
						bWriteObjectProperty = false;
						Interop->QueueIncomingObjectRepUpdate_Internal(ObjectRef, ActorChannel, RepData);
					}
				}
			}

			if (bWriteObjectProperty)
			{
				ApplyIncomingReplicatedPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

				UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received replicated property update. actor %s (%lld), property %s (handle %d)"),
					*Interop->GetSpatialOS()->GetWorkerId(),
					*ActorChannel->Actor->GetName(),
					ActorChannel->GetEntityId().ToSpatialEntityId(),
					*RepData->Property->GetName(),
					Handle);
			}
		}
	}
	if (!Update.field_playerstate().empty())
	{
		// field_playerstate
		uint16 Handle = 16;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (bIsServer || ConditionMap.IsRelevant(RepData->Condition))
		{
			bool bWriteObjectProperty = true;
			uint8* PropertyData = RepData->GetPropertyData(reinterpret_cast<uint8*>(ActorChannel->Actor));
			APlayerState* Value = *(reinterpret_cast<APlayerState* const*>(PropertyData));

			{
				improbable::unreal::UnrealObjectRef ObjectRef = (*Update.field_playerstate().data());
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
						Value = dynamic_cast<APlayerState*>(Object_Raw);
						checkf(Value, TEXT("Object ref %s maps to object %s with the wrong class."), *ObjectRefToString(ObjectRef), *Object_Raw->GetFullName());
					}
					else
					{
						UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received unresolved object property. Value: %s. actor %s (%lld), property %s (handle %d)"),
							*Interop->GetSpatialOS()->GetWorkerId(),
							*ObjectRefToString(ObjectRef),
							*ActorChannel->Actor->GetName(),
							ActorChannel->GetEntityId().ToSpatialEntityId(),
							*RepData->Property->GetName(),
							Handle);
						bWriteObjectProperty = false;
						Interop->QueueIncomingObjectRepUpdate_Internal(ObjectRef, ActorChannel, RepData);
					}
				}
			}

			if (bWriteObjectProperty)
			{
				ApplyIncomingReplicatedPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

				UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received replicated property update. actor %s (%lld), property %s (handle %d)"),
					*Interop->GetSpatialOS()->GetWorkerId(),
					*ActorChannel->Actor->GetName(),
					ActorChannel->GetEntityId().ToSpatialEntityId(),
					*RepData->Property->GetName(),
					Handle);
			}
		}
	}
	if (!Update.field_remoteviewpitch().empty())
	{
		// field_remoteviewpitch
		uint16 Handle = 17;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (bIsServer || ConditionMap.IsRelevant(RepData->Condition))
		{
			uint8* PropertyData = RepData->GetPropertyData(reinterpret_cast<uint8*>(ActorChannel->Actor));
			uint8 Value = *(reinterpret_cast<uint8 const*>(PropertyData));

			Value = uint8(uint8((*Update.field_remoteviewpitch().data())));

			ApplyIncomingReplicatedPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received replicated property update. actor %s (%lld), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId().ToSpatialEntityId(),
				*RepData->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_controller().empty())
	{
		// field_controller
		uint16 Handle = 18;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (bIsServer || ConditionMap.IsRelevant(RepData->Condition))
		{
			bool bWriteObjectProperty = true;
			uint8* PropertyData = RepData->GetPropertyData(reinterpret_cast<uint8*>(ActorChannel->Actor));
			AController* Value = *(reinterpret_cast<AController* const*>(PropertyData));

			{
				improbable::unreal::UnrealObjectRef ObjectRef = (*Update.field_controller().data());
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
						Value = dynamic_cast<AController*>(Object_Raw);
						checkf(Value, TEXT("Object ref %s maps to object %s with the wrong class."), *ObjectRefToString(ObjectRef), *Object_Raw->GetFullName());
					}
					else
					{
						UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received unresolved object property. Value: %s. actor %s (%lld), property %s (handle %d)"),
							*Interop->GetSpatialOS()->GetWorkerId(),
							*ObjectRefToString(ObjectRef),
							*ActorChannel->Actor->GetName(),
							ActorChannel->GetEntityId().ToSpatialEntityId(),
							*RepData->Property->GetName(),
							Handle);
						bWriteObjectProperty = false;
						Interop->QueueIncomingObjectRepUpdate_Internal(ObjectRef, ActorChannel, RepData);
					}
				}
			}

			if (bWriteObjectProperty)
			{
				ApplyIncomingReplicatedPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

				UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received replicated property update. actor %s (%lld), property %s (handle %d)"),
					*Interop->GetSpatialOS()->GetWorkerId(),
					*ActorChannel->Actor->GetName(),
					ActorChannel->GetEntityId().ToSpatialEntityId(),
					*RepData->Property->GetName(),
					Handle);
			}
		}
	}
	if (!Update.field_replicatedbasedmovement_movementbase().empty())
	{
		// field_replicatedbasedmovement_movementbase
		uint16 Handle = 19;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (bIsServer || ConditionMap.IsRelevant(RepData->Condition))
		{
			bool bWriteObjectProperty = true;
			uint8* PropertyData = RepData->GetPropertyData(reinterpret_cast<uint8*>(ActorChannel->Actor));
			UPrimitiveComponent* Value = *(reinterpret_cast<UPrimitiveComponent* const*>(PropertyData));

			{
				improbable::unreal::UnrealObjectRef ObjectRef = (*Update.field_replicatedbasedmovement_movementbase().data());
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
						Value = dynamic_cast<UPrimitiveComponent*>(Object_Raw);
						checkf(Value, TEXT("Object ref %s maps to object %s with the wrong class."), *ObjectRefToString(ObjectRef), *Object_Raw->GetFullName());
					}
					else
					{
						UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received unresolved object property. Value: %s. actor %s (%lld), property %s (handle %d)"),
							*Interop->GetSpatialOS()->GetWorkerId(),
							*ObjectRefToString(ObjectRef),
							*ActorChannel->Actor->GetName(),
							ActorChannel->GetEntityId().ToSpatialEntityId(),
							*RepData->Property->GetName(),
							Handle);
						bWriteObjectProperty = false;
						Interop->QueueIncomingObjectRepUpdate_Internal(ObjectRef, ActorChannel, RepData);
					}
				}
			}

			if (bWriteObjectProperty)
			{
				ApplyIncomingReplicatedPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

				UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received replicated property update. actor %s (%lld), property %s (handle %d)"),
					*Interop->GetSpatialOS()->GetWorkerId(),
					*ActorChannel->Actor->GetName(),
					ActorChannel->GetEntityId().ToSpatialEntityId(),
					*RepData->Property->GetName(),
					Handle);
			}
		}
	}
	if (!Update.field_replicatedbasedmovement_bonename().empty())
	{
		// field_replicatedbasedmovement_bonename
		uint16 Handle = 20;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (bIsServer || ConditionMap.IsRelevant(RepData->Condition))
		{
			uint8* PropertyData = RepData->GetPropertyData(reinterpret_cast<uint8*>(ActorChannel->Actor));
			FName Value = *(reinterpret_cast<FName const*>(PropertyData));

			Value = FName(((*Update.field_replicatedbasedmovement_bonename().data())).data());

			ApplyIncomingReplicatedPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received replicated property update. actor %s (%lld), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId().ToSpatialEntityId(),
				*RepData->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_replicatedbasedmovement_location().empty())
	{
		// field_replicatedbasedmovement_location
		uint16 Handle = 21;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (bIsServer || ConditionMap.IsRelevant(RepData->Condition))
		{
			uint8* PropertyData = RepData->GetPropertyData(reinterpret_cast<uint8*>(ActorChannel->Actor));
			FVector_NetQuantize100 Value = *(reinterpret_cast<FVector_NetQuantize100 const*>(PropertyData));

			{
				auto& Vector = (*Update.field_replicatedbasedmovement_location().data());
				Value.X = Vector.x();
				Value.Y = Vector.y();
				Value.Z = Vector.z();
			}

			ApplyIncomingReplicatedPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received replicated property update. actor %s (%lld), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId().ToSpatialEntityId(),
				*RepData->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_replicatedbasedmovement_rotation().empty())
	{
		// field_replicatedbasedmovement_rotation
		uint16 Handle = 22;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (bIsServer || ConditionMap.IsRelevant(RepData->Condition))
		{
			uint8* PropertyData = RepData->GetPropertyData(reinterpret_cast<uint8*>(ActorChannel->Actor));
			FRotator Value = *(reinterpret_cast<FRotator const*>(PropertyData));

			{
				auto& Rotator = (*Update.field_replicatedbasedmovement_rotation().data());
				Value.Yaw = Rotator.yaw();
				Value.Pitch = Rotator.pitch();
				Value.Roll = Rotator.roll();
			}

			ApplyIncomingReplicatedPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received replicated property update. actor %s (%lld), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId().ToSpatialEntityId(),
				*RepData->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_replicatedbasedmovement_bserverhasbasecomponent().empty())
	{
		// field_replicatedbasedmovement_bserverhasbasecomponent
		uint16 Handle = 23;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (bIsServer || ConditionMap.IsRelevant(RepData->Condition))
		{
			uint8* PropertyData = RepData->GetPropertyData(reinterpret_cast<uint8*>(ActorChannel->Actor));
			bool Value = static_cast<UBoolProperty*>(RepData->Property)->GetPropertyValue(PropertyData);

			Value = (*Update.field_replicatedbasedmovement_bserverhasbasecomponent().data());

			ApplyIncomingReplicatedPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received replicated property update. actor %s (%lld), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId().ToSpatialEntityId(),
				*RepData->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_replicatedbasedmovement_brelativerotation().empty())
	{
		// field_replicatedbasedmovement_brelativerotation
		uint16 Handle = 24;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (bIsServer || ConditionMap.IsRelevant(RepData->Condition))
		{
			uint8* PropertyData = RepData->GetPropertyData(reinterpret_cast<uint8*>(ActorChannel->Actor));
			bool Value = static_cast<UBoolProperty*>(RepData->Property)->GetPropertyValue(PropertyData);

			Value = (*Update.field_replicatedbasedmovement_brelativerotation().data());

			ApplyIncomingReplicatedPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received replicated property update. actor %s (%lld), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId().ToSpatialEntityId(),
				*RepData->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_replicatedbasedmovement_bserverhasvelocity().empty())
	{
		// field_replicatedbasedmovement_bserverhasvelocity
		uint16 Handle = 25;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (bIsServer || ConditionMap.IsRelevant(RepData->Condition))
		{
			uint8* PropertyData = RepData->GetPropertyData(reinterpret_cast<uint8*>(ActorChannel->Actor));
			bool Value = static_cast<UBoolProperty*>(RepData->Property)->GetPropertyValue(PropertyData);

			Value = (*Update.field_replicatedbasedmovement_bserverhasvelocity().data());

			ApplyIncomingReplicatedPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received replicated property update. actor %s (%lld), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId().ToSpatialEntityId(),
				*RepData->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_animrootmotiontranslationscale().empty())
	{
		// field_animrootmotiontranslationscale
		uint16 Handle = 26;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (bIsServer || ConditionMap.IsRelevant(RepData->Condition))
		{
			uint8* PropertyData = RepData->GetPropertyData(reinterpret_cast<uint8*>(ActorChannel->Actor));
			float Value = *(reinterpret_cast<float const*>(PropertyData));

			Value = (*Update.field_animrootmotiontranslationscale().data());

			ApplyIncomingReplicatedPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received replicated property update. actor %s (%lld), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId().ToSpatialEntityId(),
				*RepData->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_replicatedserverlasttransformupdatetimestamp().empty())
	{
		// field_replicatedserverlasttransformupdatetimestamp
		uint16 Handle = 27;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (bIsServer || ConditionMap.IsRelevant(RepData->Condition))
		{
			uint8* PropertyData = RepData->GetPropertyData(reinterpret_cast<uint8*>(ActorChannel->Actor));
			float Value = *(reinterpret_cast<float const*>(PropertyData));

			Value = (*Update.field_replicatedserverlasttransformupdatetimestamp().data());

			ApplyIncomingReplicatedPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received replicated property update. actor %s (%lld), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId().ToSpatialEntityId(),
				*RepData->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_replicatedmovementmode().empty())
	{
		// field_replicatedmovementmode
		uint16 Handle = 28;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (bIsServer || ConditionMap.IsRelevant(RepData->Condition))
		{
			uint8* PropertyData = RepData->GetPropertyData(reinterpret_cast<uint8*>(ActorChannel->Actor));
			uint8 Value = *(reinterpret_cast<uint8 const*>(PropertyData));

			Value = uint8(uint8((*Update.field_replicatedmovementmode().data())));

			ApplyIncomingReplicatedPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received replicated property update. actor %s (%lld), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId().ToSpatialEntityId(),
				*RepData->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_biscrouched().empty())
	{
		// field_biscrouched
		uint16 Handle = 29;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (bIsServer || ConditionMap.IsRelevant(RepData->Condition))
		{
			uint8* PropertyData = RepData->GetPropertyData(reinterpret_cast<uint8*>(ActorChannel->Actor));
			bool Value = static_cast<UBoolProperty*>(RepData->Property)->GetPropertyValue(PropertyData);

			Value = (*Update.field_biscrouched().data());

			ApplyIncomingReplicatedPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received replicated property update. actor %s (%lld), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId().ToSpatialEntityId(),
				*RepData->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_jumpmaxholdtime().empty())
	{
		// field_jumpmaxholdtime
		uint16 Handle = 30;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (bIsServer || ConditionMap.IsRelevant(RepData->Condition))
		{
			uint8* PropertyData = RepData->GetPropertyData(reinterpret_cast<uint8*>(ActorChannel->Actor));
			float Value = *(reinterpret_cast<float const*>(PropertyData));

			Value = (*Update.field_jumpmaxholdtime().data());

			ApplyIncomingReplicatedPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received replicated property update. actor %s (%lld), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId().ToSpatialEntityId(),
				*RepData->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_jumpmaxcount().empty())
	{
		// field_jumpmaxcount
		uint16 Handle = 31;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (bIsServer || ConditionMap.IsRelevant(RepData->Condition))
		{
			uint8* PropertyData = RepData->GetPropertyData(reinterpret_cast<uint8*>(ActorChannel->Actor));
			int32 Value = *(reinterpret_cast<int32 const*>(PropertyData));

			Value = (*Update.field_jumpmaxcount().data());

			ApplyIncomingReplicatedPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received replicated property update. actor %s (%lld), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId().ToSpatialEntityId(),
				*RepData->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_reprootmotion_bisactive().empty())
	{
		// field_reprootmotion_bisactive
		uint16 Handle = 32;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (bIsServer || ConditionMap.IsRelevant(RepData->Condition))
		{
			uint8* PropertyData = RepData->GetPropertyData(reinterpret_cast<uint8*>(ActorChannel->Actor));
			bool Value = static_cast<UBoolProperty*>(RepData->Property)->GetPropertyValue(PropertyData);

			Value = (*Update.field_reprootmotion_bisactive().data());

			ApplyIncomingReplicatedPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received replicated property update. actor %s (%lld), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId().ToSpatialEntityId(),
				*RepData->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_reprootmotion_animmontage().empty())
	{
		// field_reprootmotion_animmontage
		uint16 Handle = 33;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (bIsServer || ConditionMap.IsRelevant(RepData->Condition))
		{
			bool bWriteObjectProperty = true;
			uint8* PropertyData = RepData->GetPropertyData(reinterpret_cast<uint8*>(ActorChannel->Actor));
			UAnimMontage* Value = *(reinterpret_cast<UAnimMontage* const*>(PropertyData));

			{
				improbable::unreal::UnrealObjectRef ObjectRef = (*Update.field_reprootmotion_animmontage().data());
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
						Value = dynamic_cast<UAnimMontage*>(Object_Raw);
						checkf(Value, TEXT("Object ref %s maps to object %s with the wrong class."), *ObjectRefToString(ObjectRef), *Object_Raw->GetFullName());
					}
					else
					{
						UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received unresolved object property. Value: %s. actor %s (%lld), property %s (handle %d)"),
							*Interop->GetSpatialOS()->GetWorkerId(),
							*ObjectRefToString(ObjectRef),
							*ActorChannel->Actor->GetName(),
							ActorChannel->GetEntityId().ToSpatialEntityId(),
							*RepData->Property->GetName(),
							Handle);
						bWriteObjectProperty = false;
						Interop->QueueIncomingObjectRepUpdate_Internal(ObjectRef, ActorChannel, RepData);
					}
				}
			}

			if (bWriteObjectProperty)
			{
				ApplyIncomingReplicatedPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

				UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received replicated property update. actor %s (%lld), property %s (handle %d)"),
					*Interop->GetSpatialOS()->GetWorkerId(),
					*ActorChannel->Actor->GetName(),
					ActorChannel->GetEntityId().ToSpatialEntityId(),
					*RepData->Property->GetName(),
					Handle);
			}
		}
	}
	if (!Update.field_reprootmotion_position().empty())
	{
		// field_reprootmotion_position
		uint16 Handle = 34;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (bIsServer || ConditionMap.IsRelevant(RepData->Condition))
		{
			uint8* PropertyData = RepData->GetPropertyData(reinterpret_cast<uint8*>(ActorChannel->Actor));
			float Value = *(reinterpret_cast<float const*>(PropertyData));

			Value = (*Update.field_reprootmotion_position().data());

			ApplyIncomingReplicatedPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received replicated property update. actor %s (%lld), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId().ToSpatialEntityId(),
				*RepData->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_reprootmotion_location().empty())
	{
		// field_reprootmotion_location
		uint16 Handle = 35;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (bIsServer || ConditionMap.IsRelevant(RepData->Condition))
		{
			uint8* PropertyData = RepData->GetPropertyData(reinterpret_cast<uint8*>(ActorChannel->Actor));
			FVector_NetQuantize100 Value = *(reinterpret_cast<FVector_NetQuantize100 const*>(PropertyData));

			{
				auto& Vector = (*Update.field_reprootmotion_location().data());
				Value.X = Vector.x();
				Value.Y = Vector.y();
				Value.Z = Vector.z();
			}

			ApplyIncomingReplicatedPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received replicated property update. actor %s (%lld), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId().ToSpatialEntityId(),
				*RepData->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_reprootmotion_rotation().empty())
	{
		// field_reprootmotion_rotation
		uint16 Handle = 36;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (bIsServer || ConditionMap.IsRelevant(RepData->Condition))
		{
			uint8* PropertyData = RepData->GetPropertyData(reinterpret_cast<uint8*>(ActorChannel->Actor));
			FRotator Value = *(reinterpret_cast<FRotator const*>(PropertyData));

			{
				auto& Rotator = (*Update.field_reprootmotion_rotation().data());
				Value.Yaw = Rotator.yaw();
				Value.Pitch = Rotator.pitch();
				Value.Roll = Rotator.roll();
			}

			ApplyIncomingReplicatedPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received replicated property update. actor %s (%lld), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId().ToSpatialEntityId(),
				*RepData->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_reprootmotion_movementbase().empty())
	{
		// field_reprootmotion_movementbase
		uint16 Handle = 37;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (bIsServer || ConditionMap.IsRelevant(RepData->Condition))
		{
			bool bWriteObjectProperty = true;
			uint8* PropertyData = RepData->GetPropertyData(reinterpret_cast<uint8*>(ActorChannel->Actor));
			UPrimitiveComponent* Value = *(reinterpret_cast<UPrimitiveComponent* const*>(PropertyData));

			{
				improbable::unreal::UnrealObjectRef ObjectRef = (*Update.field_reprootmotion_movementbase().data());
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
						Value = dynamic_cast<UPrimitiveComponent*>(Object_Raw);
						checkf(Value, TEXT("Object ref %s maps to object %s with the wrong class."), *ObjectRefToString(ObjectRef), *Object_Raw->GetFullName());
					}
					else
					{
						UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received unresolved object property. Value: %s. actor %s (%lld), property %s (handle %d)"),
							*Interop->GetSpatialOS()->GetWorkerId(),
							*ObjectRefToString(ObjectRef),
							*ActorChannel->Actor->GetName(),
							ActorChannel->GetEntityId().ToSpatialEntityId(),
							*RepData->Property->GetName(),
							Handle);
						bWriteObjectProperty = false;
						Interop->QueueIncomingObjectRepUpdate_Internal(ObjectRef, ActorChannel, RepData);
					}
				}
			}

			if (bWriteObjectProperty)
			{
				ApplyIncomingReplicatedPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

				UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received replicated property update. actor %s (%lld), property %s (handle %d)"),
					*Interop->GetSpatialOS()->GetWorkerId(),
					*ActorChannel->Actor->GetName(),
					ActorChannel->GetEntityId().ToSpatialEntityId(),
					*RepData->Property->GetName(),
					Handle);
			}
		}
	}
	if (!Update.field_reprootmotion_movementbasebonename().empty())
	{
		// field_reprootmotion_movementbasebonename
		uint16 Handle = 38;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (bIsServer || ConditionMap.IsRelevant(RepData->Condition))
		{
			uint8* PropertyData = RepData->GetPropertyData(reinterpret_cast<uint8*>(ActorChannel->Actor));
			FName Value = *(reinterpret_cast<FName const*>(PropertyData));

			Value = FName(((*Update.field_reprootmotion_movementbasebonename().data())).data());

			ApplyIncomingReplicatedPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received replicated property update. actor %s (%lld), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId().ToSpatialEntityId(),
				*RepData->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_reprootmotion_brelativeposition().empty())
	{
		// field_reprootmotion_brelativeposition
		uint16 Handle = 39;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (bIsServer || ConditionMap.IsRelevant(RepData->Condition))
		{
			uint8* PropertyData = RepData->GetPropertyData(reinterpret_cast<uint8*>(ActorChannel->Actor));
			bool Value = static_cast<UBoolProperty*>(RepData->Property)->GetPropertyValue(PropertyData);

			Value = (*Update.field_reprootmotion_brelativeposition().data());

			ApplyIncomingReplicatedPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received replicated property update. actor %s (%lld), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId().ToSpatialEntityId(),
				*RepData->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_reprootmotion_brelativerotation().empty())
	{
		// field_reprootmotion_brelativerotation
		uint16 Handle = 40;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (bIsServer || ConditionMap.IsRelevant(RepData->Condition))
		{
			uint8* PropertyData = RepData->GetPropertyData(reinterpret_cast<uint8*>(ActorChannel->Actor));
			bool Value = static_cast<UBoolProperty*>(RepData->Property)->GetPropertyValue(PropertyData);

			Value = (*Update.field_reprootmotion_brelativerotation().data());

			ApplyIncomingReplicatedPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received replicated property update. actor %s (%lld), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId().ToSpatialEntityId(),
				*RepData->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_reprootmotion_authoritativerootmotion().empty())
	{
		// field_reprootmotion_authoritativerootmotion
		uint16 Handle = 41;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (bIsServer || ConditionMap.IsRelevant(RepData->Condition))
		{
			uint8* PropertyData = RepData->GetPropertyData(reinterpret_cast<uint8*>(ActorChannel->Actor));
			FRootMotionSourceGroup Value = *(reinterpret_cast<FRootMotionSourceGroup const*>(PropertyData));

			{
				auto& ValueDataStr = (*Update.field_reprootmotion_authoritativerootmotion().data());
				TArray<uint8> ValueData;
				ValueData.Append(reinterpret_cast<const uint8*>(ValueDataStr.data()), ValueDataStr.size());
				FMemoryReader ValueDataReader(ValueData);
				bool bSuccess;
				Value.NetSerialize(ValueDataReader, PackageMap, bSuccess);
			}

			ApplyIncomingReplicatedPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received replicated property update. actor %s (%lld), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId().ToSpatialEntityId(),
				*RepData->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_reprootmotion_acceleration().empty())
	{
		// field_reprootmotion_acceleration
		uint16 Handle = 42;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (bIsServer || ConditionMap.IsRelevant(RepData->Condition))
		{
			uint8* PropertyData = RepData->GetPropertyData(reinterpret_cast<uint8*>(ActorChannel->Actor));
			FVector_NetQuantize10 Value = *(reinterpret_cast<FVector_NetQuantize10 const*>(PropertyData));

			{
				auto& Vector = (*Update.field_reprootmotion_acceleration().data());
				Value.X = Vector.x();
				Value.Y = Vector.y();
				Value.Z = Vector.z();
			}

			ApplyIncomingReplicatedPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received replicated property update. actor %s (%lld), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId().ToSpatialEntityId(),
				*RepData->Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_reprootmotion_linearvelocity().empty())
	{
		// field_reprootmotion_linearvelocity
		uint16 Handle = 43;
		const FRepHandleData* RepData = &HandleToPropertyMap[Handle];
		if (bIsServer || ConditionMap.IsRelevant(RepData->Condition))
		{
			uint8* PropertyData = RepData->GetPropertyData(reinterpret_cast<uint8*>(ActorChannel->Actor));
			FVector_NetQuantize10 Value = *(reinterpret_cast<FVector_NetQuantize10 const*>(PropertyData));

			{
				auto& Vector = (*Update.field_reprootmotion_linearvelocity().data());
				Value.X = Vector.x();
				Value.Y = Vector.y();
				Value.Z = Vector.z();
			}

			ApplyIncomingReplicatedPropertyUpdate(*RepData, ActorChannel->Actor, static_cast<const void*>(&Value), RepNotifies);

			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received replicated property update. actor %s (%lld), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId().ToSpatialEntityId(),
				*RepData->Property->GetName(),
				Handle);
		}
	}
	Interop->PostReceiveSpatialUpdate(ActorChannel, RepNotifies);
}

void USpatialTypeBinding_Character::ReceiveUpdate_Migratable(USpatialActorChannel* ActorChannel, const improbable::unreal::UnrealCharacterMigratableData::Update& Update) const
{
	const FMigratableHandlePropertyMap& HandleToPropertyMap = GetMigratableHandlePropertyMap();

	if (!Update.field_charactermovement_groundmovementmode().empty())
	{
		// field_charactermovement_groundmovementmode
		uint16 Handle = 1;
		const FMigratableHandleData* MigratableData = &HandleToPropertyMap[Handle];
		uint8* PropertyData = MigratableData->GetPropertyData(reinterpret_cast<uint8*>(ActorChannel->Actor));
		TEnumAsByte<EMovementMode> Value = *(reinterpret_cast<TEnumAsByte<EMovementMode> const*>(PropertyData));

		Value = TEnumAsByte<EMovementMode>(uint8((*Update.field_charactermovement_groundmovementmode().data())));

		ApplyIncomingMigratablePropertyUpdate(*MigratableData, ActorChannel->Actor, static_cast<const void*>(&Value));

		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received migratable property update. actor %s (%lld), property %s (handle %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*ActorChannel->Actor->GetName(),
			ActorChannel->GetEntityId().ToSpatialEntityId(),
			*MigratableData->Property->GetName(),
			Handle);
	}
	if (!Update.field_charactermovement_movementmode().empty())
	{
		// field_charactermovement_movementmode
		uint16 Handle = 2;
		const FMigratableHandleData* MigratableData = &HandleToPropertyMap[Handle];
		uint8* PropertyData = MigratableData->GetPropertyData(reinterpret_cast<uint8*>(ActorChannel->Actor));
		TEnumAsByte<EMovementMode> Value = *(reinterpret_cast<TEnumAsByte<EMovementMode> const*>(PropertyData));

		Value = TEnumAsByte<EMovementMode>(uint8((*Update.field_charactermovement_movementmode().data())));

		ApplyIncomingMigratablePropertyUpdate(*MigratableData, ActorChannel->Actor, static_cast<const void*>(&Value));

		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received migratable property update. actor %s (%lld), property %s (handle %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*ActorChannel->Actor->GetName(),
			ActorChannel->GetEntityId().ToSpatialEntityId(),
			*MigratableData->Property->GetName(),
			Handle);
	}
	if (!Update.field_charactermovement_custommovementmode().empty())
	{
		// field_charactermovement_custommovementmode
		uint16 Handle = 3;
		const FMigratableHandleData* MigratableData = &HandleToPropertyMap[Handle];
		uint8* PropertyData = MigratableData->GetPropertyData(reinterpret_cast<uint8*>(ActorChannel->Actor));
		uint8 Value = *(reinterpret_cast<uint8 const*>(PropertyData));

		Value = uint8(uint8((*Update.field_charactermovement_custommovementmode().data())));

		ApplyIncomingMigratablePropertyUpdate(*MigratableData, ActorChannel->Actor, static_cast<const void*>(&Value));

		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received migratable property update. actor %s (%lld), property %s (handle %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*ActorChannel->Actor->GetName(),
			ActorChannel->GetEntityId().ToSpatialEntityId(),
			*MigratableData->Property->GetName(),
			Handle);
	}
}

void USpatialTypeBinding_Character::RootMotionDebugClientPrintOnScreen_SendCommand(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UStrProperty, InString);

	auto Sender = [this, Connection, TargetObject, InString]() mutable -> FRPCCommandRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC RootMotionDebugClientPrintOnScreen queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return {TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealRootMotionDebugClientPrintOnScreenRequest Request;
		Request.set_field_instring(TCHAR_TO_UTF8(*InString));

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: RootMotionDebugClientPrintOnScreen, target: %s %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			*ObjectRefToString(TargetObjectRef));
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealCharacterClientRPCs::Commands::Rootmotiondebugclientprintonscreen>(TargetObjectRef.entity(), Request, 0);
		return {RequestId.Id};
	};
	Interop->SendCommandRequest_Internal(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_Character::ClientCheatWalk_SendCommand(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	auto Sender = [this, Connection, TargetObject]() mutable -> FRPCCommandRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientCheatWalk queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return {TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientCheatWalkRequest Request;

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientCheatWalk, target: %s %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			*ObjectRefToString(TargetObjectRef));
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientcheatwalk>(TargetObjectRef.entity(), Request, 0);
		return {RequestId.Id};
	};
	Interop->SendCommandRequest_Internal(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_Character::ClientCheatGhost_SendCommand(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	auto Sender = [this, Connection, TargetObject]() mutable -> FRPCCommandRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientCheatGhost queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return {TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientCheatGhostRequest Request;

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientCheatGhost, target: %s %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			*ObjectRefToString(TargetObjectRef));
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientcheatghost>(TargetObjectRef.entity(), Request, 0);
		return {RequestId.Id};
	};
	Interop->SendCommandRequest_Internal(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_Character::ClientCheatFly_SendCommand(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	auto Sender = [this, Connection, TargetObject]() mutable -> FRPCCommandRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientCheatFly queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return {TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientCheatFlyRequest Request;

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientCheatFly, target: %s %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			*ObjectRefToString(TargetObjectRef));
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientcheatfly>(TargetObjectRef.entity(), Request, 0);
		return {RequestId.Id};
	};
	Interop->SendCommandRequest_Internal(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_Character::ClientVeryShortAdjustPosition_SendCommand(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UFloatProperty, TimeStamp);
	P_GET_STRUCT(FVector, NewLoc)
	P_GET_OBJECT(UPrimitiveComponent, NewBase);
	P_GET_PROPERTY(UNameProperty, NewBaseBoneName);
	P_GET_UBOOL(bHasBase);
	P_GET_UBOOL(bBaseRelativePosition);
	P_GET_PROPERTY(UByteProperty, ServerMovementMode);

	auto Sender = [this, Connection, TargetObject, TimeStamp, NewLoc, NewBase, NewBaseBoneName, bHasBase, bBaseRelativePosition, ServerMovementMode]() mutable -> FRPCCommandRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientVeryShortAdjustPosition queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return {TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientVeryShortAdjustPositionRequest Request;
		Request.set_field_timestamp(TimeStamp);
		Request.set_field_newloc(improbable::Vector3f(NewLoc.X, NewLoc.Y, NewLoc.Z));
		if (NewBase != nullptr)
		{
			FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(NewBase);
			improbable::unreal::UnrealObjectRef ObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
			if (ObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
			{
				UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientVeryShortAdjustPosition queued. NewBase is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
				return {NewBase};
			}
			else
			{
				Request.set_field_newbase(ObjectRef);
			}
		}
		else
		{
			Request.set_field_newbase(SpatialConstants::NULL_OBJECT_REF);
		}
		Request.set_field_newbasebonename(TCHAR_TO_UTF8(*NewBaseBoneName.ToString()));
		Request.set_field_bhasbase(bHasBase);
		Request.set_field_bbaserelativeposition(bBaseRelativePosition);
		Request.set_field_servermovementmode(uint32_t(ServerMovementMode));

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientVeryShortAdjustPosition, target: %s %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			*ObjectRefToString(TargetObjectRef));
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientveryshortadjustposition>(TargetObjectRef.entity(), Request, 0);
		return {RequestId.Id};
	};
	Interop->SendCommandRequest_Internal(Sender, /*bReliable*/ false);
}

void USpatialTypeBinding_Character::ClientAdjustRootMotionSourcePosition_SendCommand(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UFloatProperty, TimeStamp);
	P_GET_STRUCT(FRootMotionSourceGroup, ServerRootMotion)
	P_GET_UBOOL(bHasAnimRootMotion);
	P_GET_PROPERTY(UFloatProperty, ServerMontageTrackPosition);
	P_GET_STRUCT(FVector, ServerLoc)
	P_GET_STRUCT(FVector_NetQuantizeNormal, ServerRotation)
	P_GET_PROPERTY(UFloatProperty, ServerVelZ);
	P_GET_OBJECT(UPrimitiveComponent, ServerBase);
	P_GET_PROPERTY(UNameProperty, ServerBoneName);
	P_GET_UBOOL(bHasBase);
	P_GET_UBOOL(bBaseRelativePosition);
	P_GET_PROPERTY(UByteProperty, ServerMovementMode);

	auto Sender = [this, Connection, TargetObject, TimeStamp, ServerRootMotion, bHasAnimRootMotion, ServerMontageTrackPosition, ServerLoc, ServerRotation, ServerVelZ, ServerBase, ServerBoneName, bHasBase, bBaseRelativePosition, ServerMovementMode]() mutable -> FRPCCommandRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientAdjustRootMotionSourcePosition queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return {TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientAdjustRootMotionSourcePositionRequest Request;
		Request.set_field_timestamp(TimeStamp);
		Request.set_field_serverrootmotion_bhasadditivesources(ServerRootMotion.bHasAdditiveSources);
		Request.set_field_serverrootmotion_bhasoverridesources(ServerRootMotion.bHasOverrideSources);
		Request.set_field_serverrootmotion_lastpreadditivevelocity(improbable::Vector3f(ServerRootMotion.LastPreAdditiveVelocity.X, ServerRootMotion.LastPreAdditiveVelocity.Y, ServerRootMotion.LastPreAdditiveVelocity.Z));
		Request.set_field_serverrootmotion_bisadditivevelocityapplied(ServerRootMotion.bIsAdditiveVelocityApplied);
		Request.set_field_serverrootmotion_lastaccumulatedsettings_flags(uint32_t(ServerRootMotion.LastAccumulatedSettings.Flags));
		Request.set_field_bhasanimrootmotion(bHasAnimRootMotion);
		Request.set_field_servermontagetrackposition(ServerMontageTrackPosition);
		Request.set_field_serverloc(improbable::Vector3f(ServerLoc.X, ServerLoc.Y, ServerLoc.Z));
		Request.set_field_serverrotation(improbable::Vector3f(ServerRotation.X, ServerRotation.Y, ServerRotation.Z));
		Request.set_field_servervelz(ServerVelZ);
		if (ServerBase != nullptr)
		{
			FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(ServerBase);
			improbable::unreal::UnrealObjectRef ObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
			if (ObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
			{
				UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientAdjustRootMotionSourcePosition queued. ServerBase is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
				return {ServerBase};
			}
			else
			{
				Request.set_field_serverbase(ObjectRef);
			}
		}
		else
		{
			Request.set_field_serverbase(SpatialConstants::NULL_OBJECT_REF);
		}
		Request.set_field_serverbonename(TCHAR_TO_UTF8(*ServerBoneName.ToString()));
		Request.set_field_bhasbase(bHasBase);
		Request.set_field_bbaserelativeposition(bBaseRelativePosition);
		Request.set_field_servermovementmode(uint32_t(ServerMovementMode));

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientAdjustRootMotionSourcePosition, target: %s %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			*ObjectRefToString(TargetObjectRef));
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientadjustrootmotionsourceposition>(TargetObjectRef.entity(), Request, 0);
		return {RequestId.Id};
	};
	Interop->SendCommandRequest_Internal(Sender, /*bReliable*/ false);
}

void USpatialTypeBinding_Character::ClientAdjustRootMotionPosition_SendCommand(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UFloatProperty, TimeStamp);
	P_GET_PROPERTY(UFloatProperty, ServerMontageTrackPosition);
	P_GET_STRUCT(FVector, ServerLoc)
	P_GET_STRUCT(FVector_NetQuantizeNormal, ServerRotation)
	P_GET_PROPERTY(UFloatProperty, ServerVelZ);
	P_GET_OBJECT(UPrimitiveComponent, ServerBase);
	P_GET_PROPERTY(UNameProperty, ServerBoneName);
	P_GET_UBOOL(bHasBase);
	P_GET_UBOOL(bBaseRelativePosition);
	P_GET_PROPERTY(UByteProperty, ServerMovementMode);

	auto Sender = [this, Connection, TargetObject, TimeStamp, ServerMontageTrackPosition, ServerLoc, ServerRotation, ServerVelZ, ServerBase, ServerBoneName, bHasBase, bBaseRelativePosition, ServerMovementMode]() mutable -> FRPCCommandRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientAdjustRootMotionPosition queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return {TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientAdjustRootMotionPositionRequest Request;
		Request.set_field_timestamp(TimeStamp);
		Request.set_field_servermontagetrackposition(ServerMontageTrackPosition);
		Request.set_field_serverloc(improbable::Vector3f(ServerLoc.X, ServerLoc.Y, ServerLoc.Z));
		Request.set_field_serverrotation(improbable::Vector3f(ServerRotation.X, ServerRotation.Y, ServerRotation.Z));
		Request.set_field_servervelz(ServerVelZ);
		if (ServerBase != nullptr)
		{
			FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(ServerBase);
			improbable::unreal::UnrealObjectRef ObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
			if (ObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
			{
				UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientAdjustRootMotionPosition queued. ServerBase is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
				return {ServerBase};
			}
			else
			{
				Request.set_field_serverbase(ObjectRef);
			}
		}
		else
		{
			Request.set_field_serverbase(SpatialConstants::NULL_OBJECT_REF);
		}
		Request.set_field_serverbonename(TCHAR_TO_UTF8(*ServerBoneName.ToString()));
		Request.set_field_bhasbase(bHasBase);
		Request.set_field_bbaserelativeposition(bBaseRelativePosition);
		Request.set_field_servermovementmode(uint32_t(ServerMovementMode));

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientAdjustRootMotionPosition, target: %s %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			*ObjectRefToString(TargetObjectRef));
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientadjustrootmotionposition>(TargetObjectRef.entity(), Request, 0);
		return {RequestId.Id};
	};
	Interop->SendCommandRequest_Internal(Sender, /*bReliable*/ false);
}

void USpatialTypeBinding_Character::ClientAdjustPosition_SendCommand(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UFloatProperty, TimeStamp);
	P_GET_STRUCT(FVector, NewLoc)
	P_GET_STRUCT(FVector, NewVel)
	P_GET_OBJECT(UPrimitiveComponent, NewBase);
	P_GET_PROPERTY(UNameProperty, NewBaseBoneName);
	P_GET_UBOOL(bHasBase);
	P_GET_UBOOL(bBaseRelativePosition);
	P_GET_PROPERTY(UByteProperty, ServerMovementMode);

	auto Sender = [this, Connection, TargetObject, TimeStamp, NewLoc, NewVel, NewBase, NewBaseBoneName, bHasBase, bBaseRelativePosition, ServerMovementMode]() mutable -> FRPCCommandRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientAdjustPosition queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return {TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientAdjustPositionRequest Request;
		Request.set_field_timestamp(TimeStamp);
		Request.set_field_newloc(improbable::Vector3f(NewLoc.X, NewLoc.Y, NewLoc.Z));
		Request.set_field_newvel(improbable::Vector3f(NewVel.X, NewVel.Y, NewVel.Z));
		if (NewBase != nullptr)
		{
			FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(NewBase);
			improbable::unreal::UnrealObjectRef ObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
			if (ObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
			{
				UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientAdjustPosition queued. NewBase is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
				return {NewBase};
			}
			else
			{
				Request.set_field_newbase(ObjectRef);
			}
		}
		else
		{
			Request.set_field_newbase(SpatialConstants::NULL_OBJECT_REF);
		}
		Request.set_field_newbasebonename(TCHAR_TO_UTF8(*NewBaseBoneName.ToString()));
		Request.set_field_bhasbase(bHasBase);
		Request.set_field_bbaserelativeposition(bBaseRelativePosition);
		Request.set_field_servermovementmode(uint32_t(ServerMovementMode));

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientAdjustPosition, target: %s %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			*ObjectRefToString(TargetObjectRef));
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientadjustposition>(TargetObjectRef.entity(), Request, 0);
		return {RequestId.Id};
	};
	Interop->SendCommandRequest_Internal(Sender, /*bReliable*/ false);
}

void USpatialTypeBinding_Character::ClientAckGoodMove_SendCommand(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UFloatProperty, TimeStamp);

	auto Sender = [this, Connection, TargetObject, TimeStamp]() mutable -> FRPCCommandRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientAckGoodMove queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return {TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientAckGoodMoveRequest Request;
		Request.set_field_timestamp(TimeStamp);

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientAckGoodMove, target: %s %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			*ObjectRefToString(TargetObjectRef));
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientackgoodmove>(TargetObjectRef.entity(), Request, 0);
		return {RequestId.Id};
	};
	Interop->SendCommandRequest_Internal(Sender, /*bReliable*/ false);
}

void USpatialTypeBinding_Character::ServerMoveOld_SendCommand(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UFloatProperty, OldTimeStamp);
	P_GET_STRUCT(FVector_NetQuantize10, OldAccel)
	P_GET_PROPERTY(UByteProperty, OldMoveFlags);

	auto Sender = [this, Connection, TargetObject, OldTimeStamp, OldAccel, OldMoveFlags]() mutable -> FRPCCommandRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ServerMoveOld queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return {TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealServerMoveOldRequest Request;
		Request.set_field_oldtimestamp(OldTimeStamp);
		Request.set_field_oldaccel(improbable::Vector3f(OldAccel.X, OldAccel.Y, OldAccel.Z));
		Request.set_field_oldmoveflags(uint32_t(OldMoveFlags));

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ServerMoveOld, target: %s %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			*ObjectRefToString(TargetObjectRef));
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealCharacterServerRPCs::Commands::Servermoveold>(TargetObjectRef.entity(), Request, 0);
		return {RequestId.Id};
	};
	Interop->SendCommandRequest_Internal(Sender, /*bReliable*/ false);
}

void USpatialTypeBinding_Character::ServerMoveDualHybridRootMotion_SendCommand(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UFloatProperty, TimeStamp0);
	P_GET_STRUCT(FVector_NetQuantize10, InAccel0)
	P_GET_PROPERTY(UByteProperty, PendingFlags);
	P_GET_PROPERTY(UUInt32Property, View0);
	P_GET_PROPERTY(UFloatProperty, TimeStamp);
	P_GET_STRUCT(FVector_NetQuantize10, InAccel)
	P_GET_STRUCT(FVector_NetQuantize100, ClientLoc)
	P_GET_PROPERTY(UByteProperty, NewFlags);
	P_GET_PROPERTY(UByteProperty, ClientRoll);
	P_GET_PROPERTY(UUInt32Property, View);
	P_GET_OBJECT(UPrimitiveComponent, ClientMovementBase);
	P_GET_PROPERTY(UNameProperty, ClientBaseBoneName);
	P_GET_PROPERTY(UByteProperty, ClientMovementMode);

	auto Sender = [this, Connection, TargetObject, TimeStamp0, InAccel0, PendingFlags, View0, TimeStamp, InAccel, ClientLoc, NewFlags, ClientRoll, View, ClientMovementBase, ClientBaseBoneName, ClientMovementMode]() mutable -> FRPCCommandRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ServerMoveDualHybridRootMotion queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return {TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealServerMoveDualHybridRootMotionRequest Request;
		Request.set_field_timestamp0(TimeStamp0);
		Request.set_field_inaccel0(improbable::Vector3f(InAccel0.X, InAccel0.Y, InAccel0.Z));
		Request.set_field_pendingflags(uint32_t(PendingFlags));
		Request.set_field_view0(uint32_t(View0));
		Request.set_field_timestamp(TimeStamp);
		Request.set_field_inaccel(improbable::Vector3f(InAccel.X, InAccel.Y, InAccel.Z));
		Request.set_field_clientloc(improbable::Vector3f(ClientLoc.X, ClientLoc.Y, ClientLoc.Z));
		Request.set_field_newflags(uint32_t(NewFlags));
		Request.set_field_clientroll(uint32_t(ClientRoll));
		Request.set_field_view(uint32_t(View));
		if (ClientMovementBase != nullptr)
		{
			FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(ClientMovementBase);
			improbable::unreal::UnrealObjectRef ObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
			if (ObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
			{
				UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ServerMoveDualHybridRootMotion queued. ClientMovementBase is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
				return {ClientMovementBase};
			}
			else
			{
				Request.set_field_clientmovementbase(ObjectRef);
			}
		}
		else
		{
			Request.set_field_clientmovementbase(SpatialConstants::NULL_OBJECT_REF);
		}
		Request.set_field_clientbasebonename(TCHAR_TO_UTF8(*ClientBaseBoneName.ToString()));
		Request.set_field_clientmovementmode(uint32_t(ClientMovementMode));

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ServerMoveDualHybridRootMotion, target: %s %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			*ObjectRefToString(TargetObjectRef));
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealCharacterServerRPCs::Commands::Servermovedualhybridrootmotion>(TargetObjectRef.entity(), Request, 0);
		return {RequestId.Id};
	};
	Interop->SendCommandRequest_Internal(Sender, /*bReliable*/ false);
}

void USpatialTypeBinding_Character::ServerMoveDual_SendCommand(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UFloatProperty, TimeStamp0);
	P_GET_STRUCT(FVector_NetQuantize10, InAccel0)
	P_GET_PROPERTY(UByteProperty, PendingFlags);
	P_GET_PROPERTY(UUInt32Property, View0);
	P_GET_PROPERTY(UFloatProperty, TimeStamp);
	P_GET_STRUCT(FVector_NetQuantize10, InAccel)
	P_GET_STRUCT(FVector_NetQuantize100, ClientLoc)
	P_GET_PROPERTY(UByteProperty, NewFlags);
	P_GET_PROPERTY(UByteProperty, ClientRoll);
	P_GET_PROPERTY(UUInt32Property, View);
	P_GET_OBJECT(UPrimitiveComponent, ClientMovementBase);
	P_GET_PROPERTY(UNameProperty, ClientBaseBoneName);
	P_GET_PROPERTY(UByteProperty, ClientMovementMode);

	auto Sender = [this, Connection, TargetObject, TimeStamp0, InAccel0, PendingFlags, View0, TimeStamp, InAccel, ClientLoc, NewFlags, ClientRoll, View, ClientMovementBase, ClientBaseBoneName, ClientMovementMode]() mutable -> FRPCCommandRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ServerMoveDual queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return {TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealServerMoveDualRequest Request;
		Request.set_field_timestamp0(TimeStamp0);
		Request.set_field_inaccel0(improbable::Vector3f(InAccel0.X, InAccel0.Y, InAccel0.Z));
		Request.set_field_pendingflags(uint32_t(PendingFlags));
		Request.set_field_view0(uint32_t(View0));
		Request.set_field_timestamp(TimeStamp);
		Request.set_field_inaccel(improbable::Vector3f(InAccel.X, InAccel.Y, InAccel.Z));
		Request.set_field_clientloc(improbable::Vector3f(ClientLoc.X, ClientLoc.Y, ClientLoc.Z));
		Request.set_field_newflags(uint32_t(NewFlags));
		Request.set_field_clientroll(uint32_t(ClientRoll));
		Request.set_field_view(uint32_t(View));
		if (ClientMovementBase != nullptr)
		{
			FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(ClientMovementBase);
			improbable::unreal::UnrealObjectRef ObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
			if (ObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
			{
				UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ServerMoveDual queued. ClientMovementBase is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
				return {ClientMovementBase};
			}
			else
			{
				Request.set_field_clientmovementbase(ObjectRef);
			}
		}
		else
		{
			Request.set_field_clientmovementbase(SpatialConstants::NULL_OBJECT_REF);
		}
		Request.set_field_clientbasebonename(TCHAR_TO_UTF8(*ClientBaseBoneName.ToString()));
		Request.set_field_clientmovementmode(uint32_t(ClientMovementMode));

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ServerMoveDual, target: %s %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			*ObjectRefToString(TargetObjectRef));
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealCharacterServerRPCs::Commands::Servermovedual>(TargetObjectRef.entity(), Request, 0);
		return {RequestId.Id};
	};
	Interop->SendCommandRequest_Internal(Sender, /*bReliable*/ false);
}

void USpatialTypeBinding_Character::ServerMove_SendCommand(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UFloatProperty, TimeStamp);
	P_GET_STRUCT(FVector_NetQuantize10, InAccel)
	P_GET_STRUCT(FVector_NetQuantize100, ClientLoc)
	P_GET_PROPERTY(UByteProperty, CompressedMoveFlags);
	P_GET_PROPERTY(UByteProperty, ClientRoll);
	P_GET_PROPERTY(UUInt32Property, View);
	P_GET_OBJECT(UPrimitiveComponent, ClientMovementBase);
	P_GET_PROPERTY(UNameProperty, ClientBaseBoneName);
	P_GET_PROPERTY(UByteProperty, ClientMovementMode);

	auto Sender = [this, Connection, TargetObject, TimeStamp, InAccel, ClientLoc, CompressedMoveFlags, ClientRoll, View, ClientMovementBase, ClientBaseBoneName, ClientMovementMode]() mutable -> FRPCCommandRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ServerMove queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return {TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealServerMoveRequest Request;
		Request.set_field_timestamp(TimeStamp);
		Request.set_field_inaccel(improbable::Vector3f(InAccel.X, InAccel.Y, InAccel.Z));
		Request.set_field_clientloc(improbable::Vector3f(ClientLoc.X, ClientLoc.Y, ClientLoc.Z));
		Request.set_field_compressedmoveflags(uint32_t(CompressedMoveFlags));
		Request.set_field_clientroll(uint32_t(ClientRoll));
		Request.set_field_view(uint32_t(View));
		if (ClientMovementBase != nullptr)
		{
			FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(ClientMovementBase);
			improbable::unreal::UnrealObjectRef ObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
			if (ObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
			{
				UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ServerMove queued. ClientMovementBase is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
				return {ClientMovementBase};
			}
			else
			{
				Request.set_field_clientmovementbase(ObjectRef);
			}
		}
		else
		{
			Request.set_field_clientmovementbase(SpatialConstants::NULL_OBJECT_REF);
		}
		Request.set_field_clientbasebonename(TCHAR_TO_UTF8(*ClientBaseBoneName.ToString()));
		Request.set_field_clientmovementmode(uint32_t(ClientMovementMode));

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ServerMove, target: %s %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			*ObjectRefToString(TargetObjectRef));
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealCharacterServerRPCs::Commands::Servermove>(TargetObjectRef.entity(), Request, 0);
		return {RequestId.Id};
	};
	Interop->SendCommandRequest_Internal(Sender, /*bReliable*/ false);
}

void USpatialTypeBinding_Character::RootMotionDebugClientPrintOnScreen_OnCommandRequest(const worker::CommandRequestOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Rootmotiondebugclientprintonscreen>& Op)
{
	auto Receiver = [this, Op]() mutable -> FRPCCommandResponseResult
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RootMotionDebugClientPrintOnScreen_OnCommandRequest: Target object %s is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ObjectRefToString(TargetObjectRef));
			return {TargetObjectRef};
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		ACharacter* TargetObject = Cast<ACharacter>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: RootMotionDebugClientPrintOnScreen_OnCommandRequest: Object Ref %s (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*ObjectRefToString(TargetObjectRef),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: RootMotionDebugClientPrintOnScreen_OnCommandRequest: Object Ref %s (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*ObjectRefToString(TargetObjectRef),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Declare parameters.
		FString InString;

		// Extract from request data.
		InString = FString(UTF8_TO_TCHAR(Op.Request.field_instring().c_str()));

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: RootMotionDebugClientPrintOnScreen, target: %s %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			*ObjectRefToString(TargetObjectRef));
		TargetObject->RootMotionDebugClientPrintOnScreen_Implementation(InString);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealCharacterClientRPCs::Commands::Rootmotiondebugclientprintonscreen>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse_Internal(Receiver);
}

void USpatialTypeBinding_Character::ClientCheatWalk_OnCommandRequest(const worker::CommandRequestOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientcheatwalk>& Op)
{
	auto Receiver = [this, Op]() mutable -> FRPCCommandResponseResult
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientCheatWalk_OnCommandRequest: Target object %s is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ObjectRefToString(TargetObjectRef));
			return {TargetObjectRef};
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		ACharacter* TargetObject = Cast<ACharacter>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientCheatWalk_OnCommandRequest: Object Ref %s (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*ObjectRefToString(TargetObjectRef),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientCheatWalk_OnCommandRequest: Object Ref %s (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*ObjectRefToString(TargetObjectRef),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientCheatWalk, target: %s %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			*ObjectRefToString(TargetObjectRef));
		TargetObject->ClientCheatWalk_Implementation();

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientcheatwalk>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse_Internal(Receiver);
}

void USpatialTypeBinding_Character::ClientCheatGhost_OnCommandRequest(const worker::CommandRequestOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientcheatghost>& Op)
{
	auto Receiver = [this, Op]() mutable -> FRPCCommandResponseResult
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientCheatGhost_OnCommandRequest: Target object %s is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ObjectRefToString(TargetObjectRef));
			return {TargetObjectRef};
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		ACharacter* TargetObject = Cast<ACharacter>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientCheatGhost_OnCommandRequest: Object Ref %s (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*ObjectRefToString(TargetObjectRef),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientCheatGhost_OnCommandRequest: Object Ref %s (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*ObjectRefToString(TargetObjectRef),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientCheatGhost, target: %s %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			*ObjectRefToString(TargetObjectRef));
		TargetObject->ClientCheatGhost_Implementation();

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientcheatghost>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse_Internal(Receiver);
}

void USpatialTypeBinding_Character::ClientCheatFly_OnCommandRequest(const worker::CommandRequestOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientcheatfly>& Op)
{
	auto Receiver = [this, Op]() mutable -> FRPCCommandResponseResult
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientCheatFly_OnCommandRequest: Target object %s is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ObjectRefToString(TargetObjectRef));
			return {TargetObjectRef};
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		ACharacter* TargetObject = Cast<ACharacter>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientCheatFly_OnCommandRequest: Object Ref %s (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*ObjectRefToString(TargetObjectRef),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientCheatFly_OnCommandRequest: Object Ref %s (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*ObjectRefToString(TargetObjectRef),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientCheatFly, target: %s %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			*ObjectRefToString(TargetObjectRef));
		TargetObject->ClientCheatFly_Implementation();

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientcheatfly>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse_Internal(Receiver);
}

void USpatialTypeBinding_Character::ClientVeryShortAdjustPosition_OnCommandRequest(const worker::CommandRequestOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientveryshortadjustposition>& Op)
{
	auto Receiver = [this, Op]() mutable -> FRPCCommandResponseResult
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientVeryShortAdjustPosition_OnCommandRequest: Target object %s is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ObjectRefToString(TargetObjectRef));
			return {TargetObjectRef};
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		UCharacterMovementComponent* TargetObject = Cast<UCharacterMovementComponent>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientVeryShortAdjustPosition_OnCommandRequest: Object Ref %s (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*ObjectRefToString(TargetObjectRef),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientVeryShortAdjustPosition_OnCommandRequest: Object Ref %s (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*ObjectRefToString(TargetObjectRef),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Declare parameters.
		float TimeStamp;
		FVector NewLoc;
		UPrimitiveComponent* NewBase;
		FName NewBaseBoneName;
		bool bHasBase;
		bool bBaseRelativePosition;
		uint8 ServerMovementMode;

		// Extract from request data.
		TimeStamp = Op.Request.field_timestamp();
		{
			auto& Vector = Op.Request.field_newloc();
			NewLoc.X = Vector.x();
			NewLoc.Y = Vector.y();
			NewLoc.Z = Vector.z();
		}
		{
			improbable::unreal::UnrealObjectRef ObjectRef = Op.Request.field_newbase();
			check(ObjectRef != SpatialConstants::UNRESOLVED_OBJECT_REF);
			if (ObjectRef == SpatialConstants::NULL_OBJECT_REF)
			{
				NewBase = nullptr;
			}
			else
			{
				FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(ObjectRef);
				if (NetGUID.IsValid())
				{
					UObject* Object_Raw = PackageMap->GetObjectFromNetGUID(NetGUID, true);
					checkf(Object_Raw, TEXT("An object ref %s should map to a valid object."), *ObjectRefToString(ObjectRef));
					NewBase = dynamic_cast<UPrimitiveComponent*>(Object_Raw);
					checkf(NewBase, TEXT("Object ref %s maps to object %s with the wrong class."), *ObjectRefToString(ObjectRef), *Object_Raw->GetFullName());
				}
				else
				{
					UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientVeryShortAdjustPosition_OnCommandRequest: NewBase %s is not resolved on this worker."),
						*Interop->GetSpatialOS()->GetWorkerId(),
						*ObjectRefToString(ObjectRef));
					return {ObjectRef};
				}
			}
		}
		NewBaseBoneName = FName((Op.Request.field_newbasebonename()).data());
		bHasBase = Op.Request.field_bhasbase();
		bBaseRelativePosition = Op.Request.field_bbaserelativeposition();
		ServerMovementMode = uint8(uint8(Op.Request.field_servermovementmode()));

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientVeryShortAdjustPosition, target: %s %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			*ObjectRefToString(TargetObjectRef));
		TargetObject->ClientVeryShortAdjustPosition_Implementation(TimeStamp, NewLoc, NewBase, NewBaseBoneName, bHasBase, bBaseRelativePosition, ServerMovementMode);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientveryshortadjustposition>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse_Internal(Receiver);
}

void USpatialTypeBinding_Character::ClientAdjustRootMotionSourcePosition_OnCommandRequest(const worker::CommandRequestOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientadjustrootmotionsourceposition>& Op)
{
	auto Receiver = [this, Op]() mutable -> FRPCCommandResponseResult
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientAdjustRootMotionSourcePosition_OnCommandRequest: Target object %s is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ObjectRefToString(TargetObjectRef));
			return {TargetObjectRef};
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		UCharacterMovementComponent* TargetObject = Cast<UCharacterMovementComponent>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientAdjustRootMotionSourcePosition_OnCommandRequest: Object Ref %s (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*ObjectRefToString(TargetObjectRef),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientAdjustRootMotionSourcePosition_OnCommandRequest: Object Ref %s (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*ObjectRefToString(TargetObjectRef),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Declare parameters.
		float TimeStamp;
		FRootMotionSourceGroup ServerRootMotion;
		bool bHasAnimRootMotion;
		float ServerMontageTrackPosition;
		FVector ServerLoc;
		FVector_NetQuantizeNormal ServerRotation;
		float ServerVelZ;
		UPrimitiveComponent* ServerBase;
		FName ServerBoneName;
		bool bHasBase;
		bool bBaseRelativePosition;
		uint8 ServerMovementMode;

		// Extract from request data.
		TimeStamp = Op.Request.field_timestamp();
		ServerRootMotion.bHasAdditiveSources = Op.Request.field_serverrootmotion_bhasadditivesources();
		ServerRootMotion.bHasOverrideSources = Op.Request.field_serverrootmotion_bhasoverridesources();
		{
			auto& Vector = Op.Request.field_serverrootmotion_lastpreadditivevelocity();
			ServerRootMotion.LastPreAdditiveVelocity.X = Vector.x();
			ServerRootMotion.LastPreAdditiveVelocity.Y = Vector.y();
			ServerRootMotion.LastPreAdditiveVelocity.Z = Vector.z();
		}
		ServerRootMotion.bIsAdditiveVelocityApplied = Op.Request.field_serverrootmotion_bisadditivevelocityapplied();
		ServerRootMotion.LastAccumulatedSettings.Flags = uint8(uint8(Op.Request.field_serverrootmotion_lastaccumulatedsettings_flags()));
		bHasAnimRootMotion = Op.Request.field_bhasanimrootmotion();
		ServerMontageTrackPosition = Op.Request.field_servermontagetrackposition();
		{
			auto& Vector = Op.Request.field_serverloc();
			ServerLoc.X = Vector.x();
			ServerLoc.Y = Vector.y();
			ServerLoc.Z = Vector.z();
		}
		{
			auto& Vector = Op.Request.field_serverrotation();
			ServerRotation.X = Vector.x();
			ServerRotation.Y = Vector.y();
			ServerRotation.Z = Vector.z();
		}
		ServerVelZ = Op.Request.field_servervelz();
		{
			improbable::unreal::UnrealObjectRef ObjectRef = Op.Request.field_serverbase();
			check(ObjectRef != SpatialConstants::UNRESOLVED_OBJECT_REF);
			if (ObjectRef == SpatialConstants::NULL_OBJECT_REF)
			{
				ServerBase = nullptr;
			}
			else
			{
				FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(ObjectRef);
				if (NetGUID.IsValid())
				{
					UObject* Object_Raw = PackageMap->GetObjectFromNetGUID(NetGUID, true);
					checkf(Object_Raw, TEXT("An object ref %s should map to a valid object."), *ObjectRefToString(ObjectRef));
					ServerBase = dynamic_cast<UPrimitiveComponent*>(Object_Raw);
					checkf(ServerBase, TEXT("Object ref %s maps to object %s with the wrong class."), *ObjectRefToString(ObjectRef), *Object_Raw->GetFullName());
				}
				else
				{
					UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientAdjustRootMotionSourcePosition_OnCommandRequest: ServerBase %s is not resolved on this worker."),
						*Interop->GetSpatialOS()->GetWorkerId(),
						*ObjectRefToString(ObjectRef));
					return {ObjectRef};
				}
			}
		}
		ServerBoneName = FName((Op.Request.field_serverbonename()).data());
		bHasBase = Op.Request.field_bhasbase();
		bBaseRelativePosition = Op.Request.field_bbaserelativeposition();
		ServerMovementMode = uint8(uint8(Op.Request.field_servermovementmode()));

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientAdjustRootMotionSourcePosition, target: %s %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			*ObjectRefToString(TargetObjectRef));
		TargetObject->ClientAdjustRootMotionSourcePosition_Implementation(TimeStamp, ServerRootMotion, bHasAnimRootMotion, ServerMontageTrackPosition, ServerLoc, ServerRotation, ServerVelZ, ServerBase, ServerBoneName, bHasBase, bBaseRelativePosition, ServerMovementMode);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientadjustrootmotionsourceposition>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse_Internal(Receiver);
}

void USpatialTypeBinding_Character::ClientAdjustRootMotionPosition_OnCommandRequest(const worker::CommandRequestOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientadjustrootmotionposition>& Op)
{
	auto Receiver = [this, Op]() mutable -> FRPCCommandResponseResult
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientAdjustRootMotionPosition_OnCommandRequest: Target object %s is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ObjectRefToString(TargetObjectRef));
			return {TargetObjectRef};
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		UCharacterMovementComponent* TargetObject = Cast<UCharacterMovementComponent>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientAdjustRootMotionPosition_OnCommandRequest: Object Ref %s (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*ObjectRefToString(TargetObjectRef),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientAdjustRootMotionPosition_OnCommandRequest: Object Ref %s (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*ObjectRefToString(TargetObjectRef),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Declare parameters.
		float TimeStamp;
		float ServerMontageTrackPosition;
		FVector ServerLoc;
		FVector_NetQuantizeNormal ServerRotation;
		float ServerVelZ;
		UPrimitiveComponent* ServerBase;
		FName ServerBoneName;
		bool bHasBase;
		bool bBaseRelativePosition;
		uint8 ServerMovementMode;

		// Extract from request data.
		TimeStamp = Op.Request.field_timestamp();
		ServerMontageTrackPosition = Op.Request.field_servermontagetrackposition();
		{
			auto& Vector = Op.Request.field_serverloc();
			ServerLoc.X = Vector.x();
			ServerLoc.Y = Vector.y();
			ServerLoc.Z = Vector.z();
		}
		{
			auto& Vector = Op.Request.field_serverrotation();
			ServerRotation.X = Vector.x();
			ServerRotation.Y = Vector.y();
			ServerRotation.Z = Vector.z();
		}
		ServerVelZ = Op.Request.field_servervelz();
		{
			improbable::unreal::UnrealObjectRef ObjectRef = Op.Request.field_serverbase();
			check(ObjectRef != SpatialConstants::UNRESOLVED_OBJECT_REF);
			if (ObjectRef == SpatialConstants::NULL_OBJECT_REF)
			{
				ServerBase = nullptr;
			}
			else
			{
				FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(ObjectRef);
				if (NetGUID.IsValid())
				{
					UObject* Object_Raw = PackageMap->GetObjectFromNetGUID(NetGUID, true);
					checkf(Object_Raw, TEXT("An object ref %s should map to a valid object."), *ObjectRefToString(ObjectRef));
					ServerBase = dynamic_cast<UPrimitiveComponent*>(Object_Raw);
					checkf(ServerBase, TEXT("Object ref %s maps to object %s with the wrong class."), *ObjectRefToString(ObjectRef), *Object_Raw->GetFullName());
				}
				else
				{
					UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientAdjustRootMotionPosition_OnCommandRequest: ServerBase %s is not resolved on this worker."),
						*Interop->GetSpatialOS()->GetWorkerId(),
						*ObjectRefToString(ObjectRef));
					return {ObjectRef};
				}
			}
		}
		ServerBoneName = FName((Op.Request.field_serverbonename()).data());
		bHasBase = Op.Request.field_bhasbase();
		bBaseRelativePosition = Op.Request.field_bbaserelativeposition();
		ServerMovementMode = uint8(uint8(Op.Request.field_servermovementmode()));

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientAdjustRootMotionPosition, target: %s %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			*ObjectRefToString(TargetObjectRef));
		TargetObject->ClientAdjustRootMotionPosition_Implementation(TimeStamp, ServerMontageTrackPosition, ServerLoc, ServerRotation, ServerVelZ, ServerBase, ServerBoneName, bHasBase, bBaseRelativePosition, ServerMovementMode);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientadjustrootmotionposition>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse_Internal(Receiver);
}

void USpatialTypeBinding_Character::ClientAdjustPosition_OnCommandRequest(const worker::CommandRequestOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientadjustposition>& Op)
{
	auto Receiver = [this, Op]() mutable -> FRPCCommandResponseResult
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientAdjustPosition_OnCommandRequest: Target object %s is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ObjectRefToString(TargetObjectRef));
			return {TargetObjectRef};
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		UCharacterMovementComponent* TargetObject = Cast<UCharacterMovementComponent>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientAdjustPosition_OnCommandRequest: Object Ref %s (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*ObjectRefToString(TargetObjectRef),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientAdjustPosition_OnCommandRequest: Object Ref %s (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*ObjectRefToString(TargetObjectRef),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Declare parameters.
		float TimeStamp;
		FVector NewLoc;
		FVector NewVel;
		UPrimitiveComponent* NewBase;
		FName NewBaseBoneName;
		bool bHasBase;
		bool bBaseRelativePosition;
		uint8 ServerMovementMode;

		// Extract from request data.
		TimeStamp = Op.Request.field_timestamp();
		{
			auto& Vector = Op.Request.field_newloc();
			NewLoc.X = Vector.x();
			NewLoc.Y = Vector.y();
			NewLoc.Z = Vector.z();
		}
		{
			auto& Vector = Op.Request.field_newvel();
			NewVel.X = Vector.x();
			NewVel.Y = Vector.y();
			NewVel.Z = Vector.z();
		}
		{
			improbable::unreal::UnrealObjectRef ObjectRef = Op.Request.field_newbase();
			check(ObjectRef != SpatialConstants::UNRESOLVED_OBJECT_REF);
			if (ObjectRef == SpatialConstants::NULL_OBJECT_REF)
			{
				NewBase = nullptr;
			}
			else
			{
				FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(ObjectRef);
				if (NetGUID.IsValid())
				{
					UObject* Object_Raw = PackageMap->GetObjectFromNetGUID(NetGUID, true);
					checkf(Object_Raw, TEXT("An object ref %s should map to a valid object."), *ObjectRefToString(ObjectRef));
					NewBase = dynamic_cast<UPrimitiveComponent*>(Object_Raw);
					checkf(NewBase, TEXT("Object ref %s maps to object %s with the wrong class."), *ObjectRefToString(ObjectRef), *Object_Raw->GetFullName());
				}
				else
				{
					UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientAdjustPosition_OnCommandRequest: NewBase %s is not resolved on this worker."),
						*Interop->GetSpatialOS()->GetWorkerId(),
						*ObjectRefToString(ObjectRef));
					return {ObjectRef};
				}
			}
		}
		NewBaseBoneName = FName((Op.Request.field_newbasebonename()).data());
		bHasBase = Op.Request.field_bhasbase();
		bBaseRelativePosition = Op.Request.field_bbaserelativeposition();
		ServerMovementMode = uint8(uint8(Op.Request.field_servermovementmode()));

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientAdjustPosition, target: %s %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			*ObjectRefToString(TargetObjectRef));
		TargetObject->ClientAdjustPosition_Implementation(TimeStamp, NewLoc, NewVel, NewBase, NewBaseBoneName, bHasBase, bBaseRelativePosition, ServerMovementMode);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientadjustposition>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse_Internal(Receiver);
}

void USpatialTypeBinding_Character::ClientAckGoodMove_OnCommandRequest(const worker::CommandRequestOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientackgoodmove>& Op)
{
	auto Receiver = [this, Op]() mutable -> FRPCCommandResponseResult
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientAckGoodMove_OnCommandRequest: Target object %s is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ObjectRefToString(TargetObjectRef));
			return {TargetObjectRef};
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		UCharacterMovementComponent* TargetObject = Cast<UCharacterMovementComponent>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientAckGoodMove_OnCommandRequest: Object Ref %s (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*ObjectRefToString(TargetObjectRef),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientAckGoodMove_OnCommandRequest: Object Ref %s (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*ObjectRefToString(TargetObjectRef),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Declare parameters.
		float TimeStamp;

		// Extract from request data.
		TimeStamp = Op.Request.field_timestamp();

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientAckGoodMove, target: %s %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			*ObjectRefToString(TargetObjectRef));
		TargetObject->ClientAckGoodMove_Implementation(TimeStamp);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientackgoodmove>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse_Internal(Receiver);
}

void USpatialTypeBinding_Character::ServerMoveOld_OnCommandRequest(const worker::CommandRequestOp<improbable::unreal::UnrealCharacterServerRPCs::Commands::Servermoveold>& Op)
{
	auto Receiver = [this, Op]() mutable -> FRPCCommandResponseResult
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ServerMoveOld_OnCommandRequest: Target object %s is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ObjectRefToString(TargetObjectRef));
			return {TargetObjectRef};
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		UCharacterMovementComponent* TargetObject = Cast<UCharacterMovementComponent>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ServerMoveOld_OnCommandRequest: Object Ref %s (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*ObjectRefToString(TargetObjectRef),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ServerMoveOld_OnCommandRequest: Object Ref %s (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*ObjectRefToString(TargetObjectRef),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Declare parameters.
		float OldTimeStamp;
		FVector_NetQuantize10 OldAccel;
		uint8 OldMoveFlags;

		// Extract from request data.
		OldTimeStamp = Op.Request.field_oldtimestamp();
		{
			auto& Vector = Op.Request.field_oldaccel();
			OldAccel.X = Vector.x();
			OldAccel.Y = Vector.y();
			OldAccel.Z = Vector.z();
		}
		OldMoveFlags = uint8(uint8(Op.Request.field_oldmoveflags()));

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ServerMoveOld, target: %s %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			*ObjectRefToString(TargetObjectRef));
		TargetObject->ServerMoveOld_Implementation(OldTimeStamp, OldAccel, OldMoveFlags);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealCharacterServerRPCs::Commands::Servermoveold>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse_Internal(Receiver);
}

void USpatialTypeBinding_Character::ServerMoveDualHybridRootMotion_OnCommandRequest(const worker::CommandRequestOp<improbable::unreal::UnrealCharacterServerRPCs::Commands::Servermovedualhybridrootmotion>& Op)
{
	auto Receiver = [this, Op]() mutable -> FRPCCommandResponseResult
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ServerMoveDualHybridRootMotion_OnCommandRequest: Target object %s is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ObjectRefToString(TargetObjectRef));
			return {TargetObjectRef};
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		UCharacterMovementComponent* TargetObject = Cast<UCharacterMovementComponent>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ServerMoveDualHybridRootMotion_OnCommandRequest: Object Ref %s (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*ObjectRefToString(TargetObjectRef),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ServerMoveDualHybridRootMotion_OnCommandRequest: Object Ref %s (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*ObjectRefToString(TargetObjectRef),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Declare parameters.
		float TimeStamp0;
		FVector_NetQuantize10 InAccel0;
		uint8 PendingFlags;
		uint32 View0;
		float TimeStamp;
		FVector_NetQuantize10 InAccel;
		FVector_NetQuantize100 ClientLoc;
		uint8 NewFlags;
		uint8 ClientRoll;
		uint32 View;
		UPrimitiveComponent* ClientMovementBase;
		FName ClientBaseBoneName;
		uint8 ClientMovementMode;

		// Extract from request data.
		TimeStamp0 = Op.Request.field_timestamp0();
		{
			auto& Vector = Op.Request.field_inaccel0();
			InAccel0.X = Vector.x();
			InAccel0.Y = Vector.y();
			InAccel0.Z = Vector.z();
		}
		PendingFlags = uint8(uint8(Op.Request.field_pendingflags()));
		View0 = uint32(Op.Request.field_view0());
		TimeStamp = Op.Request.field_timestamp();
		{
			auto& Vector = Op.Request.field_inaccel();
			InAccel.X = Vector.x();
			InAccel.Y = Vector.y();
			InAccel.Z = Vector.z();
		}
		{
			auto& Vector = Op.Request.field_clientloc();
			ClientLoc.X = Vector.x();
			ClientLoc.Y = Vector.y();
			ClientLoc.Z = Vector.z();
		}
		NewFlags = uint8(uint8(Op.Request.field_newflags()));
		ClientRoll = uint8(uint8(Op.Request.field_clientroll()));
		View = uint32(Op.Request.field_view());
		{
			improbable::unreal::UnrealObjectRef ObjectRef = Op.Request.field_clientmovementbase();
			check(ObjectRef != SpatialConstants::UNRESOLVED_OBJECT_REF);
			if (ObjectRef == SpatialConstants::NULL_OBJECT_REF)
			{
				ClientMovementBase = nullptr;
			}
			else
			{
				FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(ObjectRef);
				if (NetGUID.IsValid())
				{
					UObject* Object_Raw = PackageMap->GetObjectFromNetGUID(NetGUID, true);
					checkf(Object_Raw, TEXT("An object ref %s should map to a valid object."), *ObjectRefToString(ObjectRef));
					ClientMovementBase = dynamic_cast<UPrimitiveComponent*>(Object_Raw);
					checkf(ClientMovementBase, TEXT("Object ref %s maps to object %s with the wrong class."), *ObjectRefToString(ObjectRef), *Object_Raw->GetFullName());
				}
				else
				{
					UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ServerMoveDualHybridRootMotion_OnCommandRequest: ClientMovementBase %s is not resolved on this worker."),
						*Interop->GetSpatialOS()->GetWorkerId(),
						*ObjectRefToString(ObjectRef));
					return {ObjectRef};
				}
			}
		}
		ClientBaseBoneName = FName((Op.Request.field_clientbasebonename()).data());
		ClientMovementMode = uint8(uint8(Op.Request.field_clientmovementmode()));

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ServerMoveDualHybridRootMotion, target: %s %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			*ObjectRefToString(TargetObjectRef));
		TargetObject->ServerMoveDualHybridRootMotion_Implementation(TimeStamp0, InAccel0, PendingFlags, View0, TimeStamp, InAccel, ClientLoc, NewFlags, ClientRoll, View, ClientMovementBase, ClientBaseBoneName, ClientMovementMode);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealCharacterServerRPCs::Commands::Servermovedualhybridrootmotion>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse_Internal(Receiver);
}

void USpatialTypeBinding_Character::ServerMoveDual_OnCommandRequest(const worker::CommandRequestOp<improbable::unreal::UnrealCharacterServerRPCs::Commands::Servermovedual>& Op)
{
	auto Receiver = [this, Op]() mutable -> FRPCCommandResponseResult
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ServerMoveDual_OnCommandRequest: Target object %s is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ObjectRefToString(TargetObjectRef));
			return {TargetObjectRef};
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		UCharacterMovementComponent* TargetObject = Cast<UCharacterMovementComponent>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ServerMoveDual_OnCommandRequest: Object Ref %s (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*ObjectRefToString(TargetObjectRef),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ServerMoveDual_OnCommandRequest: Object Ref %s (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*ObjectRefToString(TargetObjectRef),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Declare parameters.
		float TimeStamp0;
		FVector_NetQuantize10 InAccel0;
		uint8 PendingFlags;
		uint32 View0;
		float TimeStamp;
		FVector_NetQuantize10 InAccel;
		FVector_NetQuantize100 ClientLoc;
		uint8 NewFlags;
		uint8 ClientRoll;
		uint32 View;
		UPrimitiveComponent* ClientMovementBase;
		FName ClientBaseBoneName;
		uint8 ClientMovementMode;

		// Extract from request data.
		TimeStamp0 = Op.Request.field_timestamp0();
		{
			auto& Vector = Op.Request.field_inaccel0();
			InAccel0.X = Vector.x();
			InAccel0.Y = Vector.y();
			InAccel0.Z = Vector.z();
		}
		PendingFlags = uint8(uint8(Op.Request.field_pendingflags()));
		View0 = uint32(Op.Request.field_view0());
		TimeStamp = Op.Request.field_timestamp();
		{
			auto& Vector = Op.Request.field_inaccel();
			InAccel.X = Vector.x();
			InAccel.Y = Vector.y();
			InAccel.Z = Vector.z();
		}
		{
			auto& Vector = Op.Request.field_clientloc();
			ClientLoc.X = Vector.x();
			ClientLoc.Y = Vector.y();
			ClientLoc.Z = Vector.z();
		}
		NewFlags = uint8(uint8(Op.Request.field_newflags()));
		ClientRoll = uint8(uint8(Op.Request.field_clientroll()));
		View = uint32(Op.Request.field_view());
		{
			improbable::unreal::UnrealObjectRef ObjectRef = Op.Request.field_clientmovementbase();
			check(ObjectRef != SpatialConstants::UNRESOLVED_OBJECT_REF);
			if (ObjectRef == SpatialConstants::NULL_OBJECT_REF)
			{
				ClientMovementBase = nullptr;
			}
			else
			{
				FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(ObjectRef);
				if (NetGUID.IsValid())
				{
					UObject* Object_Raw = PackageMap->GetObjectFromNetGUID(NetGUID, true);
					checkf(Object_Raw, TEXT("An object ref %s should map to a valid object."), *ObjectRefToString(ObjectRef));
					ClientMovementBase = dynamic_cast<UPrimitiveComponent*>(Object_Raw);
					checkf(ClientMovementBase, TEXT("Object ref %s maps to object %s with the wrong class."), *ObjectRefToString(ObjectRef), *Object_Raw->GetFullName());
				}
				else
				{
					UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ServerMoveDual_OnCommandRequest: ClientMovementBase %s is not resolved on this worker."),
						*Interop->GetSpatialOS()->GetWorkerId(),
						*ObjectRefToString(ObjectRef));
					return {ObjectRef};
				}
			}
		}
		ClientBaseBoneName = FName((Op.Request.field_clientbasebonename()).data());
		ClientMovementMode = uint8(uint8(Op.Request.field_clientmovementmode()));

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ServerMoveDual, target: %s %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			*ObjectRefToString(TargetObjectRef));
		TargetObject->ServerMoveDual_Implementation(TimeStamp0, InAccel0, PendingFlags, View0, TimeStamp, InAccel, ClientLoc, NewFlags, ClientRoll, View, ClientMovementBase, ClientBaseBoneName, ClientMovementMode);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealCharacterServerRPCs::Commands::Servermovedual>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse_Internal(Receiver);
}

void USpatialTypeBinding_Character::ServerMove_OnCommandRequest(const worker::CommandRequestOp<improbable::unreal::UnrealCharacterServerRPCs::Commands::Servermove>& Op)
{
	auto Receiver = [this, Op]() mutable -> FRPCCommandResponseResult
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ServerMove_OnCommandRequest: Target object %s is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ObjectRefToString(TargetObjectRef));
			return {TargetObjectRef};
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		UCharacterMovementComponent* TargetObject = Cast<UCharacterMovementComponent>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ServerMove_OnCommandRequest: Object Ref %s (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*ObjectRefToString(TargetObjectRef),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ServerMove_OnCommandRequest: Object Ref %s (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*ObjectRefToString(TargetObjectRef),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Declare parameters.
		float TimeStamp;
		FVector_NetQuantize10 InAccel;
		FVector_NetQuantize100 ClientLoc;
		uint8 CompressedMoveFlags;
		uint8 ClientRoll;
		uint32 View;
		UPrimitiveComponent* ClientMovementBase;
		FName ClientBaseBoneName;
		uint8 ClientMovementMode;

		// Extract from request data.
		TimeStamp = Op.Request.field_timestamp();
		{
			auto& Vector = Op.Request.field_inaccel();
			InAccel.X = Vector.x();
			InAccel.Y = Vector.y();
			InAccel.Z = Vector.z();
		}
		{
			auto& Vector = Op.Request.field_clientloc();
			ClientLoc.X = Vector.x();
			ClientLoc.Y = Vector.y();
			ClientLoc.Z = Vector.z();
		}
		CompressedMoveFlags = uint8(uint8(Op.Request.field_compressedmoveflags()));
		ClientRoll = uint8(uint8(Op.Request.field_clientroll()));
		View = uint32(Op.Request.field_view());
		{
			improbable::unreal::UnrealObjectRef ObjectRef = Op.Request.field_clientmovementbase();
			check(ObjectRef != SpatialConstants::UNRESOLVED_OBJECT_REF);
			if (ObjectRef == SpatialConstants::NULL_OBJECT_REF)
			{
				ClientMovementBase = nullptr;
			}
			else
			{
				FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(ObjectRef);
				if (NetGUID.IsValid())
				{
					UObject* Object_Raw = PackageMap->GetObjectFromNetGUID(NetGUID, true);
					checkf(Object_Raw, TEXT("An object ref %s should map to a valid object."), *ObjectRefToString(ObjectRef));
					ClientMovementBase = dynamic_cast<UPrimitiveComponent*>(Object_Raw);
					checkf(ClientMovementBase, TEXT("Object ref %s maps to object %s with the wrong class."), *ObjectRefToString(ObjectRef), *Object_Raw->GetFullName());
				}
				else
				{
					UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ServerMove_OnCommandRequest: ClientMovementBase %s is not resolved on this worker."),
						*Interop->GetSpatialOS()->GetWorkerId(),
						*ObjectRefToString(ObjectRef));
					return {ObjectRef};
				}
			}
		}
		ClientBaseBoneName = FName((Op.Request.field_clientbasebonename()).data());
		ClientMovementMode = uint8(uint8(Op.Request.field_clientmovementmode()));

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ServerMove, target: %s %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			*ObjectRefToString(TargetObjectRef));
		TargetObject->ServerMove_Implementation(TimeStamp, InAccel, ClientLoc, CompressedMoveFlags, ClientRoll, View, ClientMovementBase, ClientBaseBoneName, ClientMovementMode);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealCharacterServerRPCs::Commands::Servermove>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse_Internal(Receiver);
}

void USpatialTypeBinding_Character::RootMotionDebugClientPrintOnScreen_OnCommandResponse(const worker::CommandResponseOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Rootmotiondebugclientprintonscreen>& Op)
{
	Interop->HandleCommandResponse_Internal(TEXT("RootMotionDebugClientPrintOnScreen"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_Character::ClientCheatWalk_OnCommandResponse(const worker::CommandResponseOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientcheatwalk>& Op)
{
	Interop->HandleCommandResponse_Internal(TEXT("ClientCheatWalk"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_Character::ClientCheatGhost_OnCommandResponse(const worker::CommandResponseOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientcheatghost>& Op)
{
	Interop->HandleCommandResponse_Internal(TEXT("ClientCheatGhost"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_Character::ClientCheatFly_OnCommandResponse(const worker::CommandResponseOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientcheatfly>& Op)
{
	Interop->HandleCommandResponse_Internal(TEXT("ClientCheatFly"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_Character::ClientVeryShortAdjustPosition_OnCommandResponse(const worker::CommandResponseOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientveryshortadjustposition>& Op)
{
	Interop->HandleCommandResponse_Internal(TEXT("ClientVeryShortAdjustPosition"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_Character::ClientAdjustRootMotionSourcePosition_OnCommandResponse(const worker::CommandResponseOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientadjustrootmotionsourceposition>& Op)
{
	Interop->HandleCommandResponse_Internal(TEXT("ClientAdjustRootMotionSourcePosition"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_Character::ClientAdjustRootMotionPosition_OnCommandResponse(const worker::CommandResponseOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientadjustrootmotionposition>& Op)
{
	Interop->HandleCommandResponse_Internal(TEXT("ClientAdjustRootMotionPosition"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_Character::ClientAdjustPosition_OnCommandResponse(const worker::CommandResponseOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientadjustposition>& Op)
{
	Interop->HandleCommandResponse_Internal(TEXT("ClientAdjustPosition"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_Character::ClientAckGoodMove_OnCommandResponse(const worker::CommandResponseOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientackgoodmove>& Op)
{
	Interop->HandleCommandResponse_Internal(TEXT("ClientAckGoodMove"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_Character::ServerMoveOld_OnCommandResponse(const worker::CommandResponseOp<improbable::unreal::UnrealCharacterServerRPCs::Commands::Servermoveold>& Op)
{
	Interop->HandleCommandResponse_Internal(TEXT("ServerMoveOld"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_Character::ServerMoveDualHybridRootMotion_OnCommandResponse(const worker::CommandResponseOp<improbable::unreal::UnrealCharacterServerRPCs::Commands::Servermovedualhybridrootmotion>& Op)
{
	Interop->HandleCommandResponse_Internal(TEXT("ServerMoveDualHybridRootMotion"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_Character::ServerMoveDual_OnCommandResponse(const worker::CommandResponseOp<improbable::unreal::UnrealCharacterServerRPCs::Commands::Servermovedual>& Op)
{
	Interop->HandleCommandResponse_Internal(TEXT("ServerMoveDual"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_Character::ServerMove_OnCommandResponse(const worker::CommandResponseOp<improbable::unreal::UnrealCharacterServerRPCs::Commands::Servermove>& Op)
{
	Interop->HandleCommandResponse_Internal(TEXT("ServerMove"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}
