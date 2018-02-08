// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// Note that this file has been generated automatically

#include "SpatialTypeBinding_Character.h"
#include "SpatialOS.h"
#include "Engine.h"
#include "SpatialActorChannel.h"
#include "EntityBuilder.h"
#include "SpatialPackageMapClient.h"
#include "SpatialNetDriver.h"
#include "SpatialConstants.h"
#include "SpatialInterop.h"

const FRepHandlePropertyMap& USpatialTypeBinding_Character::GetHandlePropertyMap()
{
	static FRepHandlePropertyMap HandleToPropertyMap;
	if (HandleToPropertyMap.Num() == 0)
	{
		UClass* Class = FindObject<UClass>(ANY_PACKAGE, TEXT("Character"));
		HandleToPropertyMap.Add(1, FRepHandleData{nullptr, Class->FindPropertyByName("bHidden"), COND_None});
		HandleToPropertyMap.Add(2, FRepHandleData{nullptr, Class->FindPropertyByName("bReplicateMovement"), COND_None});
		HandleToPropertyMap.Add(3, FRepHandleData{nullptr, Class->FindPropertyByName("bTearOff"), COND_None});
		HandleToPropertyMap.Add(4, FRepHandleData{nullptr, Class->FindPropertyByName("RemoteRole"), COND_None});
		HandleToPropertyMap.Add(5, FRepHandleData{nullptr, Class->FindPropertyByName("Owner"), COND_None});
		HandleToPropertyMap.Add(6, FRepHandleData{nullptr, Class->FindPropertyByName("ReplicatedMovement"), COND_SimulatedOrPhysicsNoReplay});
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
		HandleToPropertyMap.Add(16, FRepHandleData{nullptr, Class->FindPropertyByName("PlayerState"), COND_None});
		HandleToPropertyMap.Add(17, FRepHandleData{nullptr, Class->FindPropertyByName("RemoteViewPitch"), COND_SkipOwner});
		HandleToPropertyMap.Add(18, FRepHandleData{nullptr, Class->FindPropertyByName("Controller"), COND_None});
		HandleToPropertyMap.Add(19, FRepHandleData{Class->FindPropertyByName("ReplicatedBasedMovement"), nullptr, COND_SimulatedOnly});
		HandleToPropertyMap[19].Property = Cast<UStructProperty>(HandleToPropertyMap[19].Parent)->Struct->FindPropertyByName("MovementBase");
		HandleToPropertyMap.Add(20, FRepHandleData{Class->FindPropertyByName("ReplicatedBasedMovement"), nullptr, COND_SimulatedOnly});
		HandleToPropertyMap[20].Property = Cast<UStructProperty>(HandleToPropertyMap[20].Parent)->Struct->FindPropertyByName("BoneName");
		HandleToPropertyMap.Add(21, FRepHandleData{Class->FindPropertyByName("ReplicatedBasedMovement"), nullptr, COND_SimulatedOnly});
		HandleToPropertyMap[21].Property = Cast<UStructProperty>(HandleToPropertyMap[21].Parent)->Struct->FindPropertyByName("Location");
		HandleToPropertyMap.Add(22, FRepHandleData{Class->FindPropertyByName("ReplicatedBasedMovement"), nullptr, COND_SimulatedOnly});
		HandleToPropertyMap[22].Property = Cast<UStructProperty>(HandleToPropertyMap[22].Parent)->Struct->FindPropertyByName("Rotation");
		HandleToPropertyMap.Add(23, FRepHandleData{Class->FindPropertyByName("ReplicatedBasedMovement"), nullptr, COND_SimulatedOnly});
		HandleToPropertyMap[23].Property = Cast<UStructProperty>(HandleToPropertyMap[23].Parent)->Struct->FindPropertyByName("bServerHasBaseComponent");
		HandleToPropertyMap.Add(24, FRepHandleData{Class->FindPropertyByName("ReplicatedBasedMovement"), nullptr, COND_SimulatedOnly});
		HandleToPropertyMap[24].Property = Cast<UStructProperty>(HandleToPropertyMap[24].Parent)->Struct->FindPropertyByName("bRelativeRotation");
		HandleToPropertyMap.Add(25, FRepHandleData{Class->FindPropertyByName("ReplicatedBasedMovement"), nullptr, COND_SimulatedOnly});
		HandleToPropertyMap[25].Property = Cast<UStructProperty>(HandleToPropertyMap[25].Parent)->Struct->FindPropertyByName("bServerHasVelocity");
		HandleToPropertyMap.Add(26, FRepHandleData{nullptr, Class->FindPropertyByName("AnimRootMotionTranslationScale"), COND_SimulatedOnly});
		HandleToPropertyMap.Add(27, FRepHandleData{nullptr, Class->FindPropertyByName("ReplicatedServerLastTransformUpdateTimeStamp"), COND_SimulatedOnlyNoReplay});
		HandleToPropertyMap.Add(28, FRepHandleData{nullptr, Class->FindPropertyByName("ReplicatedMovementMode"), COND_SimulatedOnly});
		HandleToPropertyMap.Add(29, FRepHandleData{nullptr, Class->FindPropertyByName("bIsCrouched"), COND_SimulatedOnly});
		HandleToPropertyMap.Add(30, FRepHandleData{nullptr, Class->FindPropertyByName("JumpMaxHoldTime"), COND_None});
		HandleToPropertyMap.Add(31, FRepHandleData{nullptr, Class->FindPropertyByName("JumpMaxCount"), COND_None});
		HandleToPropertyMap.Add(32, FRepHandleData{Class->FindPropertyByName("RepRootMotion"), nullptr, COND_SimulatedOnlyNoReplay});
		HandleToPropertyMap[32].Property = Cast<UStructProperty>(HandleToPropertyMap[32].Parent)->Struct->FindPropertyByName("bIsActive");
		HandleToPropertyMap.Add(33, FRepHandleData{Class->FindPropertyByName("RepRootMotion"), nullptr, COND_SimulatedOnlyNoReplay});
		HandleToPropertyMap[33].Property = Cast<UStructProperty>(HandleToPropertyMap[33].Parent)->Struct->FindPropertyByName("AnimMontage");
		HandleToPropertyMap.Add(34, FRepHandleData{Class->FindPropertyByName("RepRootMotion"), nullptr, COND_SimulatedOnlyNoReplay});
		HandleToPropertyMap[34].Property = Cast<UStructProperty>(HandleToPropertyMap[34].Parent)->Struct->FindPropertyByName("Position");
		HandleToPropertyMap.Add(35, FRepHandleData{Class->FindPropertyByName("RepRootMotion"), nullptr, COND_SimulatedOnlyNoReplay});
		HandleToPropertyMap[35].Property = Cast<UStructProperty>(HandleToPropertyMap[35].Parent)->Struct->FindPropertyByName("Location");
		HandleToPropertyMap.Add(36, FRepHandleData{Class->FindPropertyByName("RepRootMotion"), nullptr, COND_SimulatedOnlyNoReplay});
		HandleToPropertyMap[36].Property = Cast<UStructProperty>(HandleToPropertyMap[36].Parent)->Struct->FindPropertyByName("Rotation");
		HandleToPropertyMap.Add(37, FRepHandleData{Class->FindPropertyByName("RepRootMotion"), nullptr, COND_SimulatedOnlyNoReplay});
		HandleToPropertyMap[37].Property = Cast<UStructProperty>(HandleToPropertyMap[37].Parent)->Struct->FindPropertyByName("MovementBase");
		HandleToPropertyMap.Add(38, FRepHandleData{Class->FindPropertyByName("RepRootMotion"), nullptr, COND_SimulatedOnlyNoReplay});
		HandleToPropertyMap[38].Property = Cast<UStructProperty>(HandleToPropertyMap[38].Parent)->Struct->FindPropertyByName("MovementBaseBoneName");
		HandleToPropertyMap.Add(39, FRepHandleData{Class->FindPropertyByName("RepRootMotion"), nullptr, COND_SimulatedOnlyNoReplay});
		HandleToPropertyMap[39].Property = Cast<UStructProperty>(HandleToPropertyMap[39].Parent)->Struct->FindPropertyByName("bRelativePosition");
		HandleToPropertyMap.Add(40, FRepHandleData{Class->FindPropertyByName("RepRootMotion"), nullptr, COND_SimulatedOnlyNoReplay});
		HandleToPropertyMap[40].Property = Cast<UStructProperty>(HandleToPropertyMap[40].Parent)->Struct->FindPropertyByName("bRelativeRotation");
		HandleToPropertyMap.Add(41, FRepHandleData{Class->FindPropertyByName("RepRootMotion"), nullptr, COND_SimulatedOnlyNoReplay});
		HandleToPropertyMap[41].Property = Cast<UStructProperty>(HandleToPropertyMap[41].Parent)->Struct->FindPropertyByName("AuthoritativeRootMotion");
		HandleToPropertyMap.Add(42, FRepHandleData{Class->FindPropertyByName("RepRootMotion"), nullptr, COND_SimulatedOnlyNoReplay});
		HandleToPropertyMap[42].Property = Cast<UStructProperty>(HandleToPropertyMap[42].Parent)->Struct->FindPropertyByName("Acceleration");
		HandleToPropertyMap.Add(43, FRepHandleData{Class->FindPropertyByName("RepRootMotion"), nullptr, COND_SimulatedOnlyNoReplay});
		HandleToPropertyMap[43].Property = Cast<UStructProperty>(HandleToPropertyMap[43].Parent)->Struct->FindPropertyByName("LinearVelocity");
	}
	return HandleToPropertyMap;
}

void USpatialTypeBinding_Character::Init(USpatialInterop* InInterop, USpatialPackageMapClient* InPackageMap)
{
	Super::Init(InInterop, InPackageMap);

	RPCToSenderMap.Emplace("ClientCheatWalk", &USpatialTypeBinding_Character::ClientCheatWalk_Sender);
	RPCToSenderMap.Emplace("ClientCheatGhost", &USpatialTypeBinding_Character::ClientCheatGhost_Sender);
	RPCToSenderMap.Emplace("ClientCheatFly", &USpatialTypeBinding_Character::ClientCheatFly_Sender);
	RPCToSenderMap.Emplace("ClientVeryShortAdjustPosition", &USpatialTypeBinding_Character::ClientVeryShortAdjustPosition_Sender);
	RPCToSenderMap.Emplace("ClientAdjustRootMotionSourcePosition", &USpatialTypeBinding_Character::ClientAdjustRootMotionSourcePosition_Sender);
	RPCToSenderMap.Emplace("ClientAdjustRootMotionPosition", &USpatialTypeBinding_Character::ClientAdjustRootMotionPosition_Sender);
	RPCToSenderMap.Emplace("ClientAdjustPosition", &USpatialTypeBinding_Character::ClientAdjustPosition_Sender);
	RPCToSenderMap.Emplace("ClientAckGoodMove", &USpatialTypeBinding_Character::ClientAckGoodMove_Sender);
	RPCToSenderMap.Emplace("ServerMoveOld", &USpatialTypeBinding_Character::ServerMoveOld_Sender);
	RPCToSenderMap.Emplace("ServerMoveDualHybridRootMotion", &USpatialTypeBinding_Character::ServerMoveDualHybridRootMotion_Sender);
	RPCToSenderMap.Emplace("ServerMoveDual", &USpatialTypeBinding_Character::ServerMoveDual_Sender);
	RPCToSenderMap.Emplace("ServerMove", &USpatialTypeBinding_Character::ServerMove_Sender);
}

void USpatialTypeBinding_Character::BindToView()
{
	TSharedPtr<worker::View> View = Interop->GetSpatialOS()->GetView().Pin();
	SingleClientAddCallback = View->OnAddComponent<improbable::unreal::UnrealCharacterSingleClientReplicatedData>([this](
		const worker::AddComponentOp<improbable::unreal::UnrealCharacterSingleClientReplicatedData>& Op)
	{
		auto Update = improbable::unreal::UnrealCharacterSingleClientReplicatedData::Update::FromInitialData(Op.Data);
		USpatialActorChannel* ActorChannel = Interop->GetClientActorChannel(Op.EntityId);
		if (ActorChannel)
		{
			ClientReceiveUpdate_SingleClient(ActorChannel, Update);
		}
		else
		{
			PendingSingleClientData.Add(Op.EntityId, Op.Data);
		}
	});
	SingleClientUpdateCallback = View->OnComponentUpdate<improbable::unreal::UnrealCharacterSingleClientReplicatedData>([this](
		const worker::ComponentUpdateOp<improbable::unreal::UnrealCharacterSingleClientReplicatedData>& Op)
	{
		USpatialActorChannel* ActorChannel = Interop->GetClientActorChannel(Op.EntityId);
		if (ActorChannel)
		{
			ClientReceiveUpdate_SingleClient(ActorChannel, Op.Update);
		}
		else
		{
			Op.Update.ApplyTo(PendingSingleClientData.FindOrAdd(Op.EntityId));
		}
	});
	MultiClientAddCallback = View->OnAddComponent<improbable::unreal::UnrealCharacterMultiClientReplicatedData>([this](
		const worker::AddComponentOp<improbable::unreal::UnrealCharacterMultiClientReplicatedData>& Op)
	{
		auto Update = improbable::unreal::UnrealCharacterMultiClientReplicatedData::Update::FromInitialData(Op.Data);
		USpatialActorChannel* ActorChannel = Interop->GetClientActorChannel(Op.EntityId);
		if (ActorChannel)
		{
			ClientReceiveUpdate_MultiClient(ActorChannel, Update);
		}
		else
		{
			PendingMultiClientData.Add(Op.EntityId, Op.Data);
		}
	});
	MultiClientUpdateCallback = View->OnComponentUpdate<improbable::unreal::UnrealCharacterMultiClientReplicatedData>([this](
		const worker::ComponentUpdateOp<improbable::unreal::UnrealCharacterMultiClientReplicatedData>& Op)
	{
		USpatialActorChannel* ActorChannel = Interop->GetClientActorChannel(Op.EntityId);
		if (ActorChannel)
		{
			ClientReceiveUpdate_MultiClient(ActorChannel, Op.Update);
		}
		else
		{
			Op.Update.ApplyTo(PendingMultiClientData.FindOrAdd(Op.EntityId));
		}
	});
	using ClientRPCCommandTypes = improbable::unreal::UnrealCharacterClientRPCs::Commands;
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientcheatwalk>(std::bind(&USpatialTypeBinding_Character::ClientCheatWalk_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientcheatghost>(std::bind(&USpatialTypeBinding_Character::ClientCheatGhost_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientcheatfly>(std::bind(&USpatialTypeBinding_Character::ClientCheatFly_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientveryshortadjustposition>(std::bind(&USpatialTypeBinding_Character::ClientVeryShortAdjustPosition_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientadjustrootmotionsourceposition>(std::bind(&USpatialTypeBinding_Character::ClientAdjustRootMotionSourcePosition_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientadjustrootmotionposition>(std::bind(&USpatialTypeBinding_Character::ClientAdjustRootMotionPosition_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientadjustposition>(std::bind(&USpatialTypeBinding_Character::ClientAdjustPosition_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientackgoodmove>(std::bind(&USpatialTypeBinding_Character::ClientAckGoodMove_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientcheatwalk>(std::bind(&USpatialTypeBinding_Character::ClientCheatWalk_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientcheatghost>(std::bind(&USpatialTypeBinding_Character::ClientCheatGhost_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientcheatfly>(std::bind(&USpatialTypeBinding_Character::ClientCheatFly_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientveryshortadjustposition>(std::bind(&USpatialTypeBinding_Character::ClientVeryShortAdjustPosition_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientadjustrootmotionsourceposition>(std::bind(&USpatialTypeBinding_Character::ClientAdjustRootMotionSourcePosition_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientadjustrootmotionposition>(std::bind(&USpatialTypeBinding_Character::ClientAdjustRootMotionPosition_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientadjustposition>(std::bind(&USpatialTypeBinding_Character::ClientAdjustPosition_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientackgoodmove>(std::bind(&USpatialTypeBinding_Character::ClientAckGoodMove_Sender_Response, this, std::placeholders::_1)));
	using ServerRPCCommandTypes = improbable::unreal::UnrealCharacterServerRPCs::Commands;
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Servermoveold>(std::bind(&USpatialTypeBinding_Character::ServerMoveOld_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Servermovedualhybridrootmotion>(std::bind(&USpatialTypeBinding_Character::ServerMoveDualHybridRootMotion_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Servermovedual>(std::bind(&USpatialTypeBinding_Character::ServerMoveDual_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Servermove>(std::bind(&USpatialTypeBinding_Character::ServerMove_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ServerRPCCommandTypes::Servermoveold>(std::bind(&USpatialTypeBinding_Character::ServerMoveOld_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ServerRPCCommandTypes::Servermovedualhybridrootmotion>(std::bind(&USpatialTypeBinding_Character::ServerMoveDualHybridRootMotion_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ServerRPCCommandTypes::Servermovedual>(std::bind(&USpatialTypeBinding_Character::ServerMoveDual_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ServerRPCCommandTypes::Servermove>(std::bind(&USpatialTypeBinding_Character::ServerMove_Sender_Response, this, std::placeholders::_1)));
}

void USpatialTypeBinding_Character::UnbindFromView()
{
	TSharedPtr<worker::View> View = Interop->GetSpatialOS()->GetView().Pin();
	View->Remove(SingleClientAddCallback);
	View->Remove(SingleClientUpdateCallback);
	View->Remove(MultiClientAddCallback);
	View->Remove(MultiClientUpdateCallback);
	for (auto& Callback : RPCReceiverCallbacks)
	{
		View->Remove(Callback);
	}
}

worker::ComponentId USpatialTypeBinding_Character::GetReplicatedGroupComponentId(EReplicatedPropertyGroup Group) const
{
	switch (Group)
	{
	case GROUP_SingleClient:
		return improbable::unreal::UnrealCharacterSingleClientReplicatedData::ComponentId;
	case GROUP_MultiClient:
		return improbable::unreal::UnrealCharacterMultiClientReplicatedData::ComponentId;
	default:
		checkNoEntry();
		return 0;
	}
}

worker::Entity USpatialTypeBinding_Character::CreateActorEntity(const FString& ClientWorkerId, const FVector& Position, const FString& Metadata, const FPropertyChangeState& InitialChanges, USpatialActorChannel* Channel) const
{
	// Setup initial data.
	improbable::unreal::UnrealCharacterSingleClientReplicatedData::Data SingleClientData;
	improbable::unreal::UnrealCharacterSingleClientReplicatedData::Update SingleClientUpdate;
	bool bSingleClientUpdateChanged = false;
	improbable::unreal::UnrealCharacterMultiClientReplicatedData::Data MultiClientData;
	improbable::unreal::UnrealCharacterMultiClientReplicatedData::Update MultiClientUpdate;
	bool bMultiClientUpdateChanged = false;
	BuildSpatialComponentUpdate(InitialChanges, Channel, SingleClientUpdate, bSingleClientUpdateChanged, MultiClientUpdate, bMultiClientUpdateChanged);
	SingleClientUpdate.ApplyTo(SingleClientData);
	MultiClientUpdate.ApplyTo(MultiClientData);

	// Create entity.
	std::string ClientWorkerIdString = TCHAR_TO_UTF8(*ClientWorkerId);

	improbable::WorkerAttributeSet WorkerAttribute{{worker::List<std::string>{"UnrealWorker"}}};
	improbable::WorkerAttributeSet ClientAttribute{{worker::List<std::string>{"UnrealClient"}}};
	improbable::WorkerAttributeSet OwnClientAttribute{{"workerId:" + ClientWorkerIdString}};

	improbable::WorkerRequirementSet WorkersOnly{{WorkerAttribute}};
	improbable::WorkerRequirementSet ClientsOnly{{ClientAttribute}};
	improbable::WorkerRequirementSet OwnClientOnly{{OwnClientAttribute}};
	improbable::WorkerRequirementSet AnyUnrealWorkerOrClient{{WorkerAttribute, ClientAttribute}};

	const improbable::Coordinates SpatialPosition = USpatialOSConversionFunctionLibrary::UnrealCoordinatesToSpatialOsCoordinatesCast(Position);
	return improbable::unreal::FEntityBuilder::Begin()
		.AddPositionComponent(improbable::Position::Data{SpatialPosition}, WorkersOnly)
		.AddMetadataComponent(improbable::Metadata::Data{TCHAR_TO_UTF8(*Metadata)})
		.SetPersistence(true)
		.SetReadAcl(AnyUnrealWorkerOrClient)
		.AddComponent<improbable::unreal::UnrealCharacterSingleClientReplicatedData>(SingleClientData, WorkersOnly)
		.AddComponent<improbable::unreal::UnrealCharacterMultiClientReplicatedData>(MultiClientData, WorkersOnly)
		.AddComponent<improbable::unreal::UnrealCharacterCompleteData>(improbable::unreal::UnrealCharacterCompleteData::Data{}, WorkersOnly)
		.AddComponent<improbable::unreal::UnrealCharacterClientRPCs>(improbable::unreal::UnrealCharacterClientRPCs::Data{}, OwnClientOnly)
		.AddComponent<improbable::unreal::UnrealCharacterServerRPCs>(improbable::unreal::UnrealCharacterServerRPCs::Data{}, WorkersOnly)
		.Build();
}

void USpatialTypeBinding_Character::SendComponentUpdates(const FPropertyChangeState& Changes, USpatialActorChannel* Channel, const worker::EntityId& EntityId) const
{
	// Build SpatialOS updates.
	improbable::unreal::UnrealCharacterSingleClientReplicatedData::Update SingleClientUpdate;
	bool bSingleClientUpdateChanged = false;
	improbable::unreal::UnrealCharacterMultiClientReplicatedData::Update MultiClientUpdate;
	bool bMultiClientUpdateChanged = false;
	BuildSpatialComponentUpdate(Changes, Channel, SingleClientUpdate, bSingleClientUpdateChanged, MultiClientUpdate, bMultiClientUpdateChanged);

	// Send SpatialOS updates if anything changed.
	TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
	if (bSingleClientUpdateChanged)
	{
		Connection->SendComponentUpdate<improbable::unreal::UnrealCharacterSingleClientReplicatedData>(EntityId, SingleClientUpdate);
	}
	if (bMultiClientUpdateChanged)
	{
		Connection->SendComponentUpdate<improbable::unreal::UnrealCharacterMultiClientReplicatedData>(EntityId, MultiClientUpdate);
	}
}

void USpatialTypeBinding_Character::SendRPCCommand(UObject* TargetObject, const UFunction* const Function, FFrame* const Frame)
{
	TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
	auto SenderFuncIterator = RPCToSenderMap.Find(Function->GetFName());
	checkf(*SenderFuncIterator, TEXT("Sender for %s has not been registered with RPCToSenderMap."), *Function->GetFName().ToString());
	(this->*(*SenderFuncIterator))(Connection.Get(), Frame, TargetObject);
}

void USpatialTypeBinding_Character::ApplyQueuedStateToChannel(USpatialActorChannel* ActorChannel)
{
	improbable::unreal::UnrealCharacterSingleClientReplicatedData::Data* SingleClientData = PendingSingleClientData.Find(ActorChannel->GetEntityId());
	if (SingleClientData)
	{
		auto Update = improbable::unreal::UnrealCharacterSingleClientReplicatedData::Update::FromInitialData(*SingleClientData);
		PendingSingleClientData.Remove(ActorChannel->GetEntityId());
		ClientReceiveUpdate_SingleClient(ActorChannel, Update);
	}
	improbable::unreal::UnrealCharacterMultiClientReplicatedData::Data* MultiClientData = PendingMultiClientData.Find(ActorChannel->GetEntityId());
	if (MultiClientData)
	{
		auto Update = improbable::unreal::UnrealCharacterMultiClientReplicatedData::Update::FromInitialData(*MultiClientData);
		PendingMultiClientData.Remove(ActorChannel->GetEntityId());
		ClientReceiveUpdate_MultiClient(ActorChannel, Update);
	}
}

void USpatialTypeBinding_Character::BuildSpatialComponentUpdate(
	const FPropertyChangeState& Changes,
	USpatialActorChannel* Channel,
	improbable::unreal::UnrealCharacterSingleClientReplicatedData::Update& SingleClientUpdate,
	bool& bSingleClientUpdateChanged,
	improbable::unreal::UnrealCharacterMultiClientReplicatedData::Update& MultiClientUpdate,
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
		UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Sending property update. actor %s (%llu), property %s (handle %d)"),
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

void USpatialTypeBinding_Character::ServerSendUpdate_SingleClient(
	const uint8* RESTRICT Data,
	int32 Handle,
	UProperty* Property,
	USpatialActorChannel* Channel,
	improbable::unreal::UnrealCharacterSingleClientReplicatedData::Update& OutUpdate) const
{
}

void USpatialTypeBinding_Character::ServerSendUpdate_MultiClient(
	const uint8* RESTRICT Data,
	int32 Handle,
	UProperty* Property,
	USpatialActorChannel* Channel,
	improbable::unreal::UnrealCharacterMultiClientReplicatedData::Update& OutUpdate) const
{
	switch (Handle)
	{
		case 1: // field_bhidden
		{
			uint8 Value = *(reinterpret_cast<uint8 const*>(Data));

			OutUpdate.set_field_bhidden(Value != 0);
			break;
		}
		case 2: // field_breplicatemovement
		{
			uint8 Value = *(reinterpret_cast<uint8 const*>(Data));

			OutUpdate.set_field_breplicatemovement(Value != 0);
			break;
		}
		case 3: // field_btearoff
		{
			uint8 Value = *(reinterpret_cast<uint8 const*>(Data));

			OutUpdate.set_field_btearoff(Value != 0);
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
					Interop->AddPendingOutgoingObjectRefUpdate(Value, Channel, 5);
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
					Interop->AddPendingOutgoingObjectRefUpdate(Value, Channel, 7);
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
					Interop->AddPendingOutgoingObjectRefUpdate(Value, Channel, 12);
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
			uint8 Value = *(reinterpret_cast<uint8 const*>(Data));

			OutUpdate.set_field_bcanbedamaged(Value != 0);
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
					Interop->AddPendingOutgoingObjectRefUpdate(Value, Channel, 15);
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
					Interop->AddPendingOutgoingObjectRefUpdate(Value, Channel, 16);
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
					Interop->AddPendingOutgoingObjectRefUpdate(Value, Channel, 18);
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
					Interop->AddPendingOutgoingObjectRefUpdate(Value, Channel, 19);
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
			bool Value = *(reinterpret_cast<bool const*>(Data));

			OutUpdate.set_field_replicatedbasedmovement_bserverhasbasecomponent(Value != 0);
			break;
		}
		case 24: // field_replicatedbasedmovement_brelativerotation
		{
			bool Value = *(reinterpret_cast<bool const*>(Data));

			OutUpdate.set_field_replicatedbasedmovement_brelativerotation(Value != 0);
			break;
		}
		case 25: // field_replicatedbasedmovement_bserverhasvelocity
		{
			bool Value = *(reinterpret_cast<bool const*>(Data));

			OutUpdate.set_field_replicatedbasedmovement_bserverhasvelocity(Value != 0);
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
			uint8 Value = *(reinterpret_cast<uint8 const*>(Data));

			OutUpdate.set_field_biscrouched(Value != 0);
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
			bool Value = *(reinterpret_cast<bool const*>(Data));

			OutUpdate.set_field_reprootmotion_bisactive(Value != 0);
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
					Interop->AddPendingOutgoingObjectRefUpdate(Value, Channel, 33);
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
					Interop->AddPendingOutgoingObjectRefUpdate(Value, Channel, 37);
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
			bool Value = *(reinterpret_cast<bool const*>(Data));

			OutUpdate.set_field_reprootmotion_brelativeposition(Value != 0);
			break;
		}
		case 40: // field_reprootmotion_brelativerotation
		{
			bool Value = *(reinterpret_cast<bool const*>(Data));

			OutUpdate.set_field_reprootmotion_brelativerotation(Value != 0);
			break;
		}
		case 41: // field_reprootmotion_authoritativerootmotion
		{
			FRootMotionSourceGroup Value = *(reinterpret_cast<FRootMotionSourceGroup const*>(Data));

			OutUpdate.set_field_reprootmotion_authoritativerootmotion_bhasadditivesources(Value.bHasAdditiveSources != 0);
			OutUpdate.set_field_reprootmotion_authoritativerootmotion_bhasoverridesources(Value.bHasOverrideSources != 0);
			OutUpdate.set_field_reprootmotion_authoritativerootmotion_lastpreadditivevelocity(improbable::Vector3f(Value.LastPreAdditiveVelocity.X, Value.LastPreAdditiveVelocity.Y, Value.LastPreAdditiveVelocity.Z));
			OutUpdate.set_field_reprootmotion_authoritativerootmotion_bisadditivevelocityapplied(Value.bIsAdditiveVelocityApplied != 0);
			OutUpdate.set_field_reprootmotion_authoritativerootmotion_lastaccumulatedsettings_flags(uint32_t(Value.LastAccumulatedSettings.Flags));
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

void USpatialTypeBinding_Character::ClientReceiveUpdate_SingleClient(
	USpatialActorChannel* ActorChannel,
	const improbable::unreal::UnrealCharacterSingleClientReplicatedData::Update& Update) const
{
	FBunchPayloadWriter OutputWriter(PackageMap);

	auto& HandleToPropertyMap = GetHandlePropertyMap();
	const bool bAutonomousProxy = ActorChannel->IsClientAutonomousProxy(improbable::unreal::UnrealCharacterClientRPCs::ComponentId);
	ConditionMapFilter ConditionMap(ActorChannel, bAutonomousProxy);
	Interop->ReceiveSpatialUpdate(ActorChannel, OutputWriter.GetNetBitWriter());
}

void USpatialTypeBinding_Character::ClientReceiveUpdate_MultiClient(
	USpatialActorChannel* ActorChannel,
	const improbable::unreal::UnrealCharacterMultiClientReplicatedData::Update& Update) const
{
	FBunchPayloadWriter OutputWriter(PackageMap);

	auto& HandleToPropertyMap = GetHandlePropertyMap();
	const bool bAutonomousProxy = ActorChannel->IsClientAutonomousProxy(improbable::unreal::UnrealCharacterClientRPCs::ComponentId);
	ConditionMapFilter ConditionMap(ActorChannel, bAutonomousProxy);
	if (!Update.field_bhidden().empty())
	{
		// field_bhidden
		uint32 Handle = 1;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			uint8 Value;

			Value = (*Update.field_bhidden().data());
			// Because Unreal will look for a specific bit in the serialization function below, we simply set all bits.
			// We will soon move away from using bunches when receiving Spatial updates.
			if (Value)
			{
				Value = 0xFF;
			}

			OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data.Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_breplicatemovement().empty())
	{
		// field_breplicatemovement
		uint32 Handle = 2;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			uint8 Value;

			Value = (*Update.field_breplicatemovement().data());
			// Because Unreal will look for a specific bit in the serialization function below, we simply set all bits.
			// We will soon move away from using bunches when receiving Spatial updates.
			if (Value)
			{
				Value = 0xFF;
			}

			OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data.Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_btearoff().empty())
	{
		// field_btearoff
		uint32 Handle = 3;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			uint8 Value;

			Value = (*Update.field_btearoff().data());
			// Because Unreal will look for a specific bit in the serialization function below, we simply set all bits.
			// We will soon move away from using bunches when receiving Spatial updates.
			if (Value)
			{
				Value = 0xFF;
			}

			OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data.Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_remoterole().empty())
	{
		// field_remoterole
		uint32 Handle = 4;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			TEnumAsByte<ENetRole> Value;

			Value = TEnumAsByte<ENetRole>(uint8((*Update.field_remoterole().data())));

			// Downgrade role from AutonomousProxy to SimulatedProxy if we aren't authoritative over
			// the server RPCs component.
			if (Value == ROLE_AutonomousProxy && !bAutonomousProxy)
			{
				Value = ROLE_SimulatedProxy;
			}

			OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data.Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_owner().empty())
	{
		// field_owner
		uint32 Handle = 5;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
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
						Value = static_cast<AActor*>(PackageMap->GetObjectFromNetGUID(NetGUID, true));
					}
					else
					{
						// TODO(David): Deal with an unresolved object ref on the client.
						UE_LOG(LogSpatialOSInterop, Warning, TEXT("%s: Received unresolved object property. Setting to nullptr (but this is probably incorrect). actor %s (%llu), property %s (handle %d)"),
							*Interop->GetSpatialOS()->GetWorkerId(),
							*ActorChannel->Actor->GetName(),
							ActorChannel->GetEntityId(),
							*Data.Property->GetName(),
							Handle);
						bWriteObjectProperty = false;
						Interop->AddPendingIncomingObjectRefUpdate(ObjectRef, ActorChannel, Cast<UObjectPropertyBase>(Data.Property), Handle);
					}
				}
			}

			if (bWriteObjectProperty)
			{
				OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
				UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
					*Interop->GetSpatialOS()->GetWorkerId(),
					*ActorChannel->Actor->GetName(),
					ActorChannel->GetEntityId(),
					*Data.Property->GetName(),
					Handle);
			}
		}
	}
	if (!Update.field_replicatedmovement().empty())
	{
		// field_replicatedmovement
		uint32 Handle = 6;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
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

			OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data.Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_attachmentreplication_attachparent().empty())
	{
		// field_attachmentreplication_attachparent
		uint32 Handle = 7;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
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
						Value = static_cast<AActor*>(PackageMap->GetObjectFromNetGUID(NetGUID, true));
					}
					else
					{
						// TODO(David): Deal with an unresolved object ref on the client.
						UE_LOG(LogSpatialOSInterop, Warning, TEXT("%s: Received unresolved object property. Setting to nullptr (but this is probably incorrect). actor %s (%llu), property %s (handle %d)"),
							*Interop->GetSpatialOS()->GetWorkerId(),
							*ActorChannel->Actor->GetName(),
							ActorChannel->GetEntityId(),
							*Data.Property->GetName(),
							Handle);
						bWriteObjectProperty = false;
						Interop->AddPendingIncomingObjectRefUpdate(ObjectRef, ActorChannel, Cast<UObjectPropertyBase>(Data.Property), Handle);
					}
				}
			}

			if (bWriteObjectProperty)
			{
				OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
				UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
					*Interop->GetSpatialOS()->GetWorkerId(),
					*ActorChannel->Actor->GetName(),
					ActorChannel->GetEntityId(),
					*Data.Property->GetName(),
					Handle);
			}
		}
	}
	if (!Update.field_attachmentreplication_locationoffset().empty())
	{
		// field_attachmentreplication_locationoffset
		uint32 Handle = 8;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			FVector_NetQuantize100 Value;

			{
				auto& Vector = (*Update.field_attachmentreplication_locationoffset().data());
				Value.X = Vector.x();
				Value.Y = Vector.y();
				Value.Z = Vector.z();
			}

			OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data.Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_attachmentreplication_relativescale3d().empty())
	{
		// field_attachmentreplication_relativescale3d
		uint32 Handle = 9;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			FVector_NetQuantize100 Value;

			{
				auto& Vector = (*Update.field_attachmentreplication_relativescale3d().data());
				Value.X = Vector.x();
				Value.Y = Vector.y();
				Value.Z = Vector.z();
			}

			OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data.Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_attachmentreplication_rotationoffset().empty())
	{
		// field_attachmentreplication_rotationoffset
		uint32 Handle = 10;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			FRotator Value;

			{
				auto& Rotator = (*Update.field_attachmentreplication_rotationoffset().data());
				Value.Yaw = Rotator.yaw();
				Value.Pitch = Rotator.pitch();
				Value.Roll = Rotator.roll();
			}

			OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data.Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_attachmentreplication_attachsocket().empty())
	{
		// field_attachmentreplication_attachsocket
		uint32 Handle = 11;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			FName Value;

			Value = FName(((*Update.field_attachmentreplication_attachsocket().data())).data());

			OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data.Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_attachmentreplication_attachcomponent().empty())
	{
		// field_attachmentreplication_attachcomponent
		uint32 Handle = 12;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
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
						Value = static_cast<USceneComponent*>(PackageMap->GetObjectFromNetGUID(NetGUID, true));
					}
					else
					{
						// TODO(David): Deal with an unresolved object ref on the client.
						UE_LOG(LogSpatialOSInterop, Warning, TEXT("%s: Received unresolved object property. Setting to nullptr (but this is probably incorrect). actor %s (%llu), property %s (handle %d)"),
							*Interop->GetSpatialOS()->GetWorkerId(),
							*ActorChannel->Actor->GetName(),
							ActorChannel->GetEntityId(),
							*Data.Property->GetName(),
							Handle);
						bWriteObjectProperty = false;
						Interop->AddPendingIncomingObjectRefUpdate(ObjectRef, ActorChannel, Cast<UObjectPropertyBase>(Data.Property), Handle);
					}
				}
			}

			if (bWriteObjectProperty)
			{
				OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
				UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
					*Interop->GetSpatialOS()->GetWorkerId(),
					*ActorChannel->Actor->GetName(),
					ActorChannel->GetEntityId(),
					*Data.Property->GetName(),
					Handle);
			}
		}
	}
	if (!Update.field_role().empty())
	{
		// field_role
		uint32 Handle = 13;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			TEnumAsByte<ENetRole> Value;

			Value = TEnumAsByte<ENetRole>(uint8((*Update.field_role().data())));

			OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data.Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_bcanbedamaged().empty())
	{
		// field_bcanbedamaged
		uint32 Handle = 14;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			uint8 Value;

			Value = (*Update.field_bcanbedamaged().data());
			// Because Unreal will look for a specific bit in the serialization function below, we simply set all bits.
			// We will soon move away from using bunches when receiving Spatial updates.
			if (Value)
			{
				Value = 0xFF;
			}

			OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data.Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_instigator().empty())
	{
		// field_instigator
		uint32 Handle = 15;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
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
						Value = static_cast<APawn*>(PackageMap->GetObjectFromNetGUID(NetGUID, true));
					}
					else
					{
						// TODO(David): Deal with an unresolved object ref on the client.
						UE_LOG(LogSpatialOSInterop, Warning, TEXT("%s: Received unresolved object property. Setting to nullptr (but this is probably incorrect). actor %s (%llu), property %s (handle %d)"),
							*Interop->GetSpatialOS()->GetWorkerId(),
							*ActorChannel->Actor->GetName(),
							ActorChannel->GetEntityId(),
							*Data.Property->GetName(),
							Handle);
						bWriteObjectProperty = false;
						Interop->AddPendingIncomingObjectRefUpdate(ObjectRef, ActorChannel, Cast<UObjectPropertyBase>(Data.Property), Handle);
					}
				}
			}

			if (bWriteObjectProperty)
			{
				OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
				UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
					*Interop->GetSpatialOS()->GetWorkerId(),
					*ActorChannel->Actor->GetName(),
					ActorChannel->GetEntityId(),
					*Data.Property->GetName(),
					Handle);
			}
		}
	}
	if (!Update.field_playerstate().empty())
	{
		// field_playerstate
		uint32 Handle = 16;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			bool bWriteObjectProperty = true;
			APlayerState* Value;

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
						Value = static_cast<APlayerState*>(PackageMap->GetObjectFromNetGUID(NetGUID, true));
					}
					else
					{
						// TODO(David): Deal with an unresolved object ref on the client.
						UE_LOG(LogSpatialOSInterop, Warning, TEXT("%s: Received unresolved object property. Setting to nullptr (but this is probably incorrect). actor %s (%llu), property %s (handle %d)"),
							*Interop->GetSpatialOS()->GetWorkerId(),
							*ActorChannel->Actor->GetName(),
							ActorChannel->GetEntityId(),
							*Data.Property->GetName(),
							Handle);
						bWriteObjectProperty = false;
						Interop->AddPendingIncomingObjectRefUpdate(ObjectRef, ActorChannel, Cast<UObjectPropertyBase>(Data.Property), Handle);
					}
				}
			}

			if (bWriteObjectProperty)
			{
				OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
				UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
					*Interop->GetSpatialOS()->GetWorkerId(),
					*ActorChannel->Actor->GetName(),
					ActorChannel->GetEntityId(),
					*Data.Property->GetName(),
					Handle);
			}
		}
	}
	if (!Update.field_remoteviewpitch().empty())
	{
		// field_remoteviewpitch
		uint32 Handle = 17;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			uint8 Value;

			Value = uint8(uint8((*Update.field_remoteviewpitch().data())));

			OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data.Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_controller().empty())
	{
		// field_controller
		uint32 Handle = 18;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			bool bWriteObjectProperty = true;
			AController* Value;

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
						Value = static_cast<AController*>(PackageMap->GetObjectFromNetGUID(NetGUID, true));
					}
					else
					{
						// TODO(David): Deal with an unresolved object ref on the client.
						UE_LOG(LogSpatialOSInterop, Warning, TEXT("%s: Received unresolved object property. Setting to nullptr (but this is probably incorrect). actor %s (%llu), property %s (handle %d)"),
							*Interop->GetSpatialOS()->GetWorkerId(),
							*ActorChannel->Actor->GetName(),
							ActorChannel->GetEntityId(),
							*Data.Property->GetName(),
							Handle);
						bWriteObjectProperty = false;
						Interop->AddPendingIncomingObjectRefUpdate(ObjectRef, ActorChannel, Cast<UObjectPropertyBase>(Data.Property), Handle);
					}
				}
			}

			if (bWriteObjectProperty)
			{
				OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
				UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
					*Interop->GetSpatialOS()->GetWorkerId(),
					*ActorChannel->Actor->GetName(),
					ActorChannel->GetEntityId(),
					*Data.Property->GetName(),
					Handle);
			}
		}
	}
	if (!Update.field_replicatedbasedmovement_movementbase().empty())
	{
		// field_replicatedbasedmovement_movementbase
		uint32 Handle = 19;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			bool bWriteObjectProperty = true;
			UPrimitiveComponent* Value;

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
						Value = static_cast<UPrimitiveComponent*>(PackageMap->GetObjectFromNetGUID(NetGUID, true));
					}
					else
					{
						// TODO(David): Deal with an unresolved object ref on the client.
						UE_LOG(LogSpatialOSInterop, Warning, TEXT("%s: Received unresolved object property. Setting to nullptr (but this is probably incorrect). actor %s (%llu), property %s (handle %d)"),
							*Interop->GetSpatialOS()->GetWorkerId(),
							*ActorChannel->Actor->GetName(),
							ActorChannel->GetEntityId(),
							*Data.Property->GetName(),
							Handle);
						bWriteObjectProperty = false;
						Interop->AddPendingIncomingObjectRefUpdate(ObjectRef, ActorChannel, Cast<UObjectPropertyBase>(Data.Property), Handle);
					}
				}
			}

			if (bWriteObjectProperty)
			{
				OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
				UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
					*Interop->GetSpatialOS()->GetWorkerId(),
					*ActorChannel->Actor->GetName(),
					ActorChannel->GetEntityId(),
					*Data.Property->GetName(),
					Handle);
			}
		}
	}
	if (!Update.field_replicatedbasedmovement_bonename().empty())
	{
		// field_replicatedbasedmovement_bonename
		uint32 Handle = 20;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			FName Value;

			Value = FName(((*Update.field_replicatedbasedmovement_bonename().data())).data());

			OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data.Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_replicatedbasedmovement_location().empty())
	{
		// field_replicatedbasedmovement_location
		uint32 Handle = 21;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			FVector_NetQuantize100 Value;

			{
				auto& Vector = (*Update.field_replicatedbasedmovement_location().data());
				Value.X = Vector.x();
				Value.Y = Vector.y();
				Value.Z = Vector.z();
			}

			OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data.Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_replicatedbasedmovement_rotation().empty())
	{
		// field_replicatedbasedmovement_rotation
		uint32 Handle = 22;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			FRotator Value;

			{
				auto& Rotator = (*Update.field_replicatedbasedmovement_rotation().data());
				Value.Yaw = Rotator.yaw();
				Value.Pitch = Rotator.pitch();
				Value.Roll = Rotator.roll();
			}

			OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data.Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_replicatedbasedmovement_bserverhasbasecomponent().empty())
	{
		// field_replicatedbasedmovement_bserverhasbasecomponent
		uint32 Handle = 23;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			bool Value;

			Value = (*Update.field_replicatedbasedmovement_bserverhasbasecomponent().data());

			OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data.Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_replicatedbasedmovement_brelativerotation().empty())
	{
		// field_replicatedbasedmovement_brelativerotation
		uint32 Handle = 24;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			bool Value;

			Value = (*Update.field_replicatedbasedmovement_brelativerotation().data());

			OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data.Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_replicatedbasedmovement_bserverhasvelocity().empty())
	{
		// field_replicatedbasedmovement_bserverhasvelocity
		uint32 Handle = 25;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			bool Value;

			Value = (*Update.field_replicatedbasedmovement_bserverhasvelocity().data());

			OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data.Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_animrootmotiontranslationscale().empty())
	{
		// field_animrootmotiontranslationscale
		uint32 Handle = 26;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			float Value;

			Value = (*Update.field_animrootmotiontranslationscale().data());

			OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data.Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_replicatedserverlasttransformupdatetimestamp().empty())
	{
		// field_replicatedserverlasttransformupdatetimestamp
		uint32 Handle = 27;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			float Value;

			Value = (*Update.field_replicatedserverlasttransformupdatetimestamp().data());

			OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data.Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_replicatedmovementmode().empty())
	{
		// field_replicatedmovementmode
		uint32 Handle = 28;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			uint8 Value;

			Value = uint8(uint8((*Update.field_replicatedmovementmode().data())));

			OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data.Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_biscrouched().empty())
	{
		// field_biscrouched
		uint32 Handle = 29;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			uint8 Value;

			Value = (*Update.field_biscrouched().data());
			// Because Unreal will look for a specific bit in the serialization function below, we simply set all bits.
			// We will soon move away from using bunches when receiving Spatial updates.
			if (Value)
			{
				Value = 0xFF;
			}

			OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data.Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_jumpmaxholdtime().empty())
	{
		// field_jumpmaxholdtime
		uint32 Handle = 30;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			float Value;

			Value = (*Update.field_jumpmaxholdtime().data());

			OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data.Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_jumpmaxcount().empty())
	{
		// field_jumpmaxcount
		uint32 Handle = 31;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			int32 Value;

			Value = (*Update.field_jumpmaxcount().data());

			OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data.Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_reprootmotion_bisactive().empty())
	{
		// field_reprootmotion_bisactive
		uint32 Handle = 32;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			bool Value;

			Value = (*Update.field_reprootmotion_bisactive().data());

			OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data.Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_reprootmotion_animmontage().empty())
	{
		// field_reprootmotion_animmontage
		uint32 Handle = 33;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			bool bWriteObjectProperty = true;
			UAnimMontage* Value;

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
						Value = static_cast<UAnimMontage*>(PackageMap->GetObjectFromNetGUID(NetGUID, true));
					}
					else
					{
						// TODO(David): Deal with an unresolved object ref on the client.
						UE_LOG(LogSpatialOSInterop, Warning, TEXT("%s: Received unresolved object property. Setting to nullptr (but this is probably incorrect). actor %s (%llu), property %s (handle %d)"),
							*Interop->GetSpatialOS()->GetWorkerId(),
							*ActorChannel->Actor->GetName(),
							ActorChannel->GetEntityId(),
							*Data.Property->GetName(),
							Handle);
						bWriteObjectProperty = false;
						Interop->AddPendingIncomingObjectRefUpdate(ObjectRef, ActorChannel, Cast<UObjectPropertyBase>(Data.Property), Handle);
					}
				}
			}

			if (bWriteObjectProperty)
			{
				OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
				UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
					*Interop->GetSpatialOS()->GetWorkerId(),
					*ActorChannel->Actor->GetName(),
					ActorChannel->GetEntityId(),
					*Data.Property->GetName(),
					Handle);
			}
		}
	}
	if (!Update.field_reprootmotion_position().empty())
	{
		// field_reprootmotion_position
		uint32 Handle = 34;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			float Value;

			Value = (*Update.field_reprootmotion_position().data());

			OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data.Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_reprootmotion_location().empty())
	{
		// field_reprootmotion_location
		uint32 Handle = 35;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			FVector_NetQuantize100 Value;

			{
				auto& Vector = (*Update.field_reprootmotion_location().data());
				Value.X = Vector.x();
				Value.Y = Vector.y();
				Value.Z = Vector.z();
			}

			OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data.Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_reprootmotion_rotation().empty())
	{
		// field_reprootmotion_rotation
		uint32 Handle = 36;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			FRotator Value;

			{
				auto& Rotator = (*Update.field_reprootmotion_rotation().data());
				Value.Yaw = Rotator.yaw();
				Value.Pitch = Rotator.pitch();
				Value.Roll = Rotator.roll();
			}

			OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data.Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_reprootmotion_movementbase().empty())
	{
		// field_reprootmotion_movementbase
		uint32 Handle = 37;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			bool bWriteObjectProperty = true;
			UPrimitiveComponent* Value;

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
						Value = static_cast<UPrimitiveComponent*>(PackageMap->GetObjectFromNetGUID(NetGUID, true));
					}
					else
					{
						// TODO(David): Deal with an unresolved object ref on the client.
						UE_LOG(LogSpatialOSInterop, Warning, TEXT("%s: Received unresolved object property. Setting to nullptr (but this is probably incorrect). actor %s (%llu), property %s (handle %d)"),
							*Interop->GetSpatialOS()->GetWorkerId(),
							*ActorChannel->Actor->GetName(),
							ActorChannel->GetEntityId(),
							*Data.Property->GetName(),
							Handle);
						bWriteObjectProperty = false;
						Interop->AddPendingIncomingObjectRefUpdate(ObjectRef, ActorChannel, Cast<UObjectPropertyBase>(Data.Property), Handle);
					}
				}
			}

			if (bWriteObjectProperty)
			{
				OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
				UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
					*Interop->GetSpatialOS()->GetWorkerId(),
					*ActorChannel->Actor->GetName(),
					ActorChannel->GetEntityId(),
					*Data.Property->GetName(),
					Handle);
			}
		}
	}
	if (!Update.field_reprootmotion_movementbasebonename().empty())
	{
		// field_reprootmotion_movementbasebonename
		uint32 Handle = 38;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			FName Value;

			Value = FName(((*Update.field_reprootmotion_movementbasebonename().data())).data());

			OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data.Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_reprootmotion_brelativeposition().empty())
	{
		// field_reprootmotion_brelativeposition
		uint32 Handle = 39;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			bool Value;

			Value = (*Update.field_reprootmotion_brelativeposition().data());

			OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data.Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_reprootmotion_brelativerotation().empty())
	{
		// field_reprootmotion_brelativerotation
		uint32 Handle = 40;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			bool Value;

			Value = (*Update.field_reprootmotion_brelativerotation().data());

			OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data.Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_reprootmotion_authoritativerootmotion_bhasadditivesources().empty())
	{
		// field_reprootmotion_authoritativerootmotion
		uint32 Handle = 41;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			FRootMotionSourceGroup Value;

			Value.bHasAdditiveSources = (*Update.field_reprootmotion_authoritativerootmotion_bhasadditivesources().data());
			Value.bHasOverrideSources = (*Update.field_reprootmotion_authoritativerootmotion_bhasoverridesources().data());
			{
				auto& Vector = (*Update.field_reprootmotion_authoritativerootmotion_lastpreadditivevelocity().data());
				Value.LastPreAdditiveVelocity.X = Vector.x();
				Value.LastPreAdditiveVelocity.Y = Vector.y();
				Value.LastPreAdditiveVelocity.Z = Vector.z();
			}
			Value.bIsAdditiveVelocityApplied = (*Update.field_reprootmotion_authoritativerootmotion_bisadditivevelocityapplied().data());
			Value.LastAccumulatedSettings.Flags = uint8(uint8((*Update.field_reprootmotion_authoritativerootmotion_lastaccumulatedsettings_flags().data())));

			OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data.Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_reprootmotion_acceleration().empty())
	{
		// field_reprootmotion_acceleration
		uint32 Handle = 42;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			FVector_NetQuantize10 Value;

			{
				auto& Vector = (*Update.field_reprootmotion_acceleration().data());
				Value.X = Vector.x();
				Value.Y = Vector.y();
				Value.Z = Vector.z();
			}

			OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data.Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_reprootmotion_linearvelocity().empty())
	{
		// field_reprootmotion_linearvelocity
		uint32 Handle = 43;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			FVector_NetQuantize10 Value;

			{
				auto& Vector = (*Update.field_reprootmotion_linearvelocity().data());
				Value.X = Vector.x();
				Value.Y = Vector.y();
				Value.Z = Vector.z();
			}

			OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received property update. actor %s (%llu), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data.Property->GetName(),
				Handle);
		}
	}
	Interop->ReceiveSpatialUpdate(ActorChannel, OutputWriter.GetNetBitWriter());
}

void USpatialTypeBinding_Character::ClientCheatWalk_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	auto Sender = [this, Connection, TargetObject]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("RPC ClientCheatWalk queued. Target object is unresolved."));
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientCheatWalkRequest Request;

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Sending RPC: ClientCheatWalk, target: %s (entity ID %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientcheatwalk>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender);
}

void USpatialTypeBinding_Character::ClientCheatGhost_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	auto Sender = [this, Connection, TargetObject]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("RPC ClientCheatGhost queued. Target object is unresolved."));
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientCheatGhostRequest Request;

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Sending RPC: ClientCheatGhost, target: %s (entity ID %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientcheatghost>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender);
}

void USpatialTypeBinding_Character::ClientCheatFly_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	auto Sender = [this, Connection, TargetObject]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("RPC ClientCheatFly queued. Target object is unresolved."));
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientCheatFlyRequest Request;

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Sending RPC: ClientCheatFly, target: %s (entity ID %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientcheatfly>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender);
}

void USpatialTypeBinding_Character::ClientVeryShortAdjustPosition_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UFloatProperty, TimeStamp);
	P_GET_STRUCT(FVector, NewLoc)
	P_GET_OBJECT(UPrimitiveComponent, NewBase);
	P_GET_PROPERTY(UNameProperty, NewBaseBoneName);
	P_GET_UBOOL(bHasBase);
	P_GET_UBOOL(bBaseRelativePosition);
	P_GET_PROPERTY(UByteProperty, ServerMovementMode);

	auto Sender = [this, Connection, TargetObject, TimeStamp, NewLoc, NewBase, NewBaseBoneName, bHasBase, bBaseRelativePosition, ServerMovementMode]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("RPC ClientVeryShortAdjustPosition queued. Target object is unresolved."));
			return FRPCRequestResult{TargetObject};
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
				UE_LOG(LogSpatialOSInterop, Log, TEXT("RPC queued. NewBase is unresolved."));
				return FRPCRequestResult{NewBase};
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
		Request.set_field_bhasbase(bHasBase != 0);
		Request.set_field_bbaserelativeposition(bBaseRelativePosition != 0);
		Request.set_field_servermovementmode(uint32_t(ServerMovementMode));

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Sending RPC: ClientVeryShortAdjustPosition, target: %s (entity ID %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientveryshortadjustposition>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender);
}

void USpatialTypeBinding_Character::ClientAdjustRootMotionSourcePosition_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
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

	auto Sender = [this, Connection, TargetObject, TimeStamp, ServerRootMotion, bHasAnimRootMotion, ServerMontageTrackPosition, ServerLoc, ServerRotation, ServerVelZ, ServerBase, ServerBoneName, bHasBase, bBaseRelativePosition, ServerMovementMode]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("RPC ClientAdjustRootMotionSourcePosition queued. Target object is unresolved."));
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientAdjustRootMotionSourcePositionRequest Request;
		Request.set_field_timestamp(TimeStamp);
		Request.set_field_serverrootmotion_bhasadditivesources(ServerRootMotion.bHasAdditiveSources != 0);
		Request.set_field_serverrootmotion_bhasoverridesources(ServerRootMotion.bHasOverrideSources != 0);
		Request.set_field_serverrootmotion_lastpreadditivevelocity(improbable::Vector3f(ServerRootMotion.LastPreAdditiveVelocity.X, ServerRootMotion.LastPreAdditiveVelocity.Y, ServerRootMotion.LastPreAdditiveVelocity.Z));
		Request.set_field_serverrootmotion_bisadditivevelocityapplied(ServerRootMotion.bIsAdditiveVelocityApplied != 0);
		Request.set_field_serverrootmotion_lastaccumulatedsettings_flags(uint32_t(ServerRootMotion.LastAccumulatedSettings.Flags));
		Request.set_field_bhasanimrootmotion(bHasAnimRootMotion != 0);
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
				UE_LOG(LogSpatialOSInterop, Log, TEXT("RPC queued. ServerBase is unresolved."));
				return FRPCRequestResult{ServerBase};
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
		Request.set_field_bhasbase(bHasBase != 0);
		Request.set_field_bbaserelativeposition(bBaseRelativePosition != 0);
		Request.set_field_servermovementmode(uint32_t(ServerMovementMode));

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Sending RPC: ClientAdjustRootMotionSourcePosition, target: %s (entity ID %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientadjustrootmotionsourceposition>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender);
}

void USpatialTypeBinding_Character::ClientAdjustRootMotionPosition_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
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

	auto Sender = [this, Connection, TargetObject, TimeStamp, ServerMontageTrackPosition, ServerLoc, ServerRotation, ServerVelZ, ServerBase, ServerBoneName, bHasBase, bBaseRelativePosition, ServerMovementMode]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("RPC ClientAdjustRootMotionPosition queued. Target object is unresolved."));
			return FRPCRequestResult{TargetObject};
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
				UE_LOG(LogSpatialOSInterop, Log, TEXT("RPC queued. ServerBase is unresolved."));
				return FRPCRequestResult{ServerBase};
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
		Request.set_field_bhasbase(bHasBase != 0);
		Request.set_field_bbaserelativeposition(bBaseRelativePosition != 0);
		Request.set_field_servermovementmode(uint32_t(ServerMovementMode));

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Sending RPC: ClientAdjustRootMotionPosition, target: %s (entity ID %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientadjustrootmotionposition>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender);
}

void USpatialTypeBinding_Character::ClientAdjustPosition_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
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

	auto Sender = [this, Connection, TargetObject, TimeStamp, NewLoc, NewVel, NewBase, NewBaseBoneName, bHasBase, bBaseRelativePosition, ServerMovementMode]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("RPC ClientAdjustPosition queued. Target object is unresolved."));
			return FRPCRequestResult{TargetObject};
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
				UE_LOG(LogSpatialOSInterop, Log, TEXT("RPC queued. NewBase is unresolved."));
				return FRPCRequestResult{NewBase};
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
		Request.set_field_bhasbase(bHasBase != 0);
		Request.set_field_bbaserelativeposition(bBaseRelativePosition != 0);
		Request.set_field_servermovementmode(uint32_t(ServerMovementMode));

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Sending RPC: ClientAdjustPosition, target: %s (entity ID %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientadjustposition>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender);
}

void USpatialTypeBinding_Character::ClientAckGoodMove_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UFloatProperty, TimeStamp);

	auto Sender = [this, Connection, TargetObject, TimeStamp]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("RPC ClientAckGoodMove queued. Target object is unresolved."));
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientAckGoodMoveRequest Request;
		Request.set_field_timestamp(TimeStamp);

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Sending RPC: ClientAckGoodMove, target: %s (entity ID %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientackgoodmove>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender);
}

void USpatialTypeBinding_Character::ServerMoveOld_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UFloatProperty, OldTimeStamp);
	P_GET_STRUCT(FVector_NetQuantize10, OldAccel)
	P_GET_PROPERTY(UByteProperty, OldMoveFlags);

	auto Sender = [this, Connection, TargetObject, OldTimeStamp, OldAccel, OldMoveFlags]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("RPC ServerMoveOld queued. Target object is unresolved."));
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealServerMoveOldRequest Request;
		Request.set_field_oldtimestamp(OldTimeStamp);
		Request.set_field_oldaccel(improbable::Vector3f(OldAccel.X, OldAccel.Y, OldAccel.Z));
		Request.set_field_oldmoveflags(uint32_t(OldMoveFlags));

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Sending RPC: ServerMoveOld, target: %s (entity ID %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealCharacterServerRPCs::Commands::Servermoveold>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender);
}

void USpatialTypeBinding_Character::ServerMoveDualHybridRootMotion_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
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

	auto Sender = [this, Connection, TargetObject, TimeStamp0, InAccel0, PendingFlags, View0, TimeStamp, InAccel, ClientLoc, NewFlags, ClientRoll, View, ClientMovementBase, ClientBaseBoneName, ClientMovementMode]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("RPC ServerMoveDualHybridRootMotion queued. Target object is unresolved."));
			return FRPCRequestResult{TargetObject};
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
				UE_LOG(LogSpatialOSInterop, Log, TEXT("RPC queued. ClientMovementBase is unresolved."));
				return FRPCRequestResult{ClientMovementBase};
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
		UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Sending RPC: ServerMoveDualHybridRootMotion, target: %s (entity ID %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealCharacterServerRPCs::Commands::Servermovedualhybridrootmotion>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender);
}

void USpatialTypeBinding_Character::ServerMoveDual_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
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

	auto Sender = [this, Connection, TargetObject, TimeStamp0, InAccel0, PendingFlags, View0, TimeStamp, InAccel, ClientLoc, NewFlags, ClientRoll, View, ClientMovementBase, ClientBaseBoneName, ClientMovementMode]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("RPC ServerMoveDual queued. Target object is unresolved."));
			return FRPCRequestResult{TargetObject};
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
				UE_LOG(LogSpatialOSInterop, Log, TEXT("RPC queued. ClientMovementBase is unresolved."));
				return FRPCRequestResult{ClientMovementBase};
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
		UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Sending RPC: ServerMoveDual, target: %s (entity ID %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealCharacterServerRPCs::Commands::Servermovedual>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender);
}

void USpatialTypeBinding_Character::ServerMove_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
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

	auto Sender = [this, Connection, TargetObject, TimeStamp, InAccel, ClientLoc, CompressedMoveFlags, ClientRoll, View, ClientMovementBase, ClientBaseBoneName, ClientMovementMode]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("RPC ServerMove queued. Target object is unresolved."));
			return FRPCRequestResult{TargetObject};
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
				UE_LOG(LogSpatialOSInterop, Log, TEXT("RPC queued. ClientMovementBase is unresolved."));
				return FRPCRequestResult{ClientMovementBase};
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
		UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Sending RPC: ServerMove, target: %s (entity ID %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealCharacterServerRPCs::Commands::Servermove>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender);
}

void USpatialTypeBinding_Character::ClientCheatWalk_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientcheatwalk>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientCheatWalk"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_Character::ClientCheatGhost_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientcheatghost>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientCheatGhost"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_Character::ClientCheatFly_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientcheatfly>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientCheatFly"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_Character::ClientVeryShortAdjustPosition_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientveryshortadjustposition>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientVeryShortAdjustPosition"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_Character::ClientAdjustRootMotionSourcePosition_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientadjustrootmotionsourceposition>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientAdjustRootMotionSourcePosition"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_Character::ClientAdjustRootMotionPosition_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientadjustrootmotionposition>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientAdjustRootMotionPosition"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_Character::ClientAdjustPosition_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientadjustposition>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientAdjustPosition"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_Character::ClientAckGoodMove_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientackgoodmove>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientAckGoodMove"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_Character::ServerMoveOld_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealCharacterServerRPCs::Commands::Servermoveold>& Op)
{
	Interop->HandleCommandResponse(TEXT("ServerMoveOld"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_Character::ServerMoveDualHybridRootMotion_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealCharacterServerRPCs::Commands::Servermovedualhybridrootmotion>& Op)
{
	Interop->HandleCommandResponse(TEXT("ServerMoveDualHybridRootMotion"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_Character::ServerMoveDual_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealCharacterServerRPCs::Commands::Servermovedual>& Op)
{
	Interop->HandleCommandResponse(TEXT("ServerMoveDual"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_Character::ServerMove_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealCharacterServerRPCs::Commands::Servermove>& Op)
{
	Interop->HandleCommandResponse(TEXT("ServerMove"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_Character::ClientCheatWalk_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientcheatwalk>& Op)
{
	improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
	FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialOSInterop, Warning, TEXT("%s: ClientCheatWalk_Receiver: Target object (entity id %llu, offset %u) is not resolved on this worker. Sending command failure."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		SendRPCResponse<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientcheatwalk>(Op, false, TEXT("Target object is unresolved on the target worker"));
		return;
	}
	ACharacter* TargetObject = Cast<ACharacter>(PackageMap->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("%s: ClientCheatWalk_Receiver: Entity ID %llu (NetGUID %s) does not correspond to a UObject."), *Interop->GetSpatialOS()->GetWorkerId(), TargetObjectRef.entity(), *TargetNetGUID.ToString());

	// Call implementation and send command response.
	UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Receiving RPC: ClientCheatWalk, target: %s (entity ID %llu, offset: %u)"),
		*Interop->GetSpatialOS()->GetWorkerId(),
		*TargetObject->GetName(),
		TargetObjectRef.entity(),
		TargetObjectRef.offset());
	TargetObject->ClientCheatWalk_Implementation();
	SendRPCResponse<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientcheatwalk>(Op, true, FString());
}

void USpatialTypeBinding_Character::ClientCheatGhost_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientcheatghost>& Op)
{
	improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
	FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialOSInterop, Warning, TEXT("%s: ClientCheatGhost_Receiver: Target object (entity id %llu, offset %u) is not resolved on this worker. Sending command failure."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		SendRPCResponse<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientcheatghost>(Op, false, TEXT("Target object is unresolved on the target worker"));
		return;
	}
	ACharacter* TargetObject = Cast<ACharacter>(PackageMap->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("%s: ClientCheatGhost_Receiver: Entity ID %llu (NetGUID %s) does not correspond to a UObject."), *Interop->GetSpatialOS()->GetWorkerId(), TargetObjectRef.entity(), *TargetNetGUID.ToString());

	// Call implementation and send command response.
	UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Receiving RPC: ClientCheatGhost, target: %s (entity ID %llu, offset: %u)"),
		*Interop->GetSpatialOS()->GetWorkerId(),
		*TargetObject->GetName(),
		TargetObjectRef.entity(),
		TargetObjectRef.offset());
	TargetObject->ClientCheatGhost_Implementation();
	SendRPCResponse<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientcheatghost>(Op, true, FString());
}

void USpatialTypeBinding_Character::ClientCheatFly_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientcheatfly>& Op)
{
	improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
	FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialOSInterop, Warning, TEXT("%s: ClientCheatFly_Receiver: Target object (entity id %llu, offset %u) is not resolved on this worker. Sending command failure."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		SendRPCResponse<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientcheatfly>(Op, false, TEXT("Target object is unresolved on the target worker"));
		return;
	}
	ACharacter* TargetObject = Cast<ACharacter>(PackageMap->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("%s: ClientCheatFly_Receiver: Entity ID %llu (NetGUID %s) does not correspond to a UObject."), *Interop->GetSpatialOS()->GetWorkerId(), TargetObjectRef.entity(), *TargetNetGUID.ToString());

	// Call implementation and send command response.
	UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Receiving RPC: ClientCheatFly, target: %s (entity ID %llu, offset: %u)"),
		*Interop->GetSpatialOS()->GetWorkerId(),
		*TargetObject->GetName(),
		TargetObjectRef.entity(),
		TargetObjectRef.offset());
	TargetObject->ClientCheatFly_Implementation();
	SendRPCResponse<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientcheatfly>(Op, true, FString());
}

void USpatialTypeBinding_Character::ClientVeryShortAdjustPosition_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientveryshortadjustposition>& Op)
{
	improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
	FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialOSInterop, Warning, TEXT("%s: ClientVeryShortAdjustPosition_Receiver: Target object (entity id %llu, offset %u) is not resolved on this worker. Sending command failure."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		SendRPCResponse<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientveryshortadjustposition>(Op, false, TEXT("Target object is unresolved on the target worker"));
		return;
	}
	UCharacterMovementComponent* TargetObject = Cast<UCharacterMovementComponent>(PackageMap->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("%s: ClientVeryShortAdjustPosition_Receiver: Entity ID %llu (NetGUID %s) does not correspond to a UObject."), *Interop->GetSpatialOS()->GetWorkerId(), TargetObjectRef.entity(), *TargetNetGUID.ToString());

	// Extract TimeStamp
	float TimeStamp;
	TimeStamp = Op.Request.field_timestamp();

	// Extract NewLoc
	FVector NewLoc;
	{
		auto& Vector = Op.Request.field_newloc();
		NewLoc.X = Vector.x();
		NewLoc.Y = Vector.y();
		NewLoc.Z = Vector.z();
	}

	// Extract NewBase
	UPrimitiveComponent* NewBase;
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
				NewBase = static_cast<UPrimitiveComponent*>(PackageMap->GetObjectFromNetGUID(NetGUID, true));
			}
			else
			{
				UE_LOG(LogSpatialOSInterop, Warning, TEXT("%s: ClientVeryShortAdjustPosition_Receiver: NewBase (entity id %llu, offset %u) is not resolved on this worker. Sending command failure."),
					*Interop->GetSpatialOS()->GetWorkerId(),
					ObjectRef.entity(),
					ObjectRef.offset());
				SendRPCResponse<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientveryshortadjustposition>(Op, false, TEXT("NewBase is unresolved on the target worker"));
				return;
			}
		}
	}

	// Extract NewBaseBoneName
	FName NewBaseBoneName;
	NewBaseBoneName = FName((Op.Request.field_newbasebonename()).data());

	// Extract bHasBase
	bool bHasBase;
	bHasBase = Op.Request.field_bhasbase();

	// Extract bBaseRelativePosition
	bool bBaseRelativePosition;
	bBaseRelativePosition = Op.Request.field_bbaserelativeposition();

	// Extract ServerMovementMode
	uint8 ServerMovementMode;
	ServerMovementMode = uint8(uint8(Op.Request.field_servermovementmode()));

	// Call implementation and send command response.
	UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Receiving RPC: ClientVeryShortAdjustPosition, target: %s (entity ID %llu, offset: %u)"),
		*Interop->GetSpatialOS()->GetWorkerId(),
		*TargetObject->GetName(),
		TargetObjectRef.entity(),
		TargetObjectRef.offset());
	TargetObject->ClientVeryShortAdjustPosition_Implementation(TimeStamp, NewLoc, NewBase, NewBaseBoneName, bHasBase, bBaseRelativePosition, ServerMovementMode);
	SendRPCResponse<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientveryshortadjustposition>(Op, true, FString());
}

void USpatialTypeBinding_Character::ClientAdjustRootMotionSourcePosition_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientadjustrootmotionsourceposition>& Op)
{
	improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
	FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialOSInterop, Warning, TEXT("%s: ClientAdjustRootMotionSourcePosition_Receiver: Target object (entity id %llu, offset %u) is not resolved on this worker. Sending command failure."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		SendRPCResponse<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientadjustrootmotionsourceposition>(Op, false, TEXT("Target object is unresolved on the target worker"));
		return;
	}
	UCharacterMovementComponent* TargetObject = Cast<UCharacterMovementComponent>(PackageMap->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("%s: ClientAdjustRootMotionSourcePosition_Receiver: Entity ID %llu (NetGUID %s) does not correspond to a UObject."), *Interop->GetSpatialOS()->GetWorkerId(), TargetObjectRef.entity(), *TargetNetGUID.ToString());

	// Extract TimeStamp
	float TimeStamp;
	TimeStamp = Op.Request.field_timestamp();

	// Extract ServerRootMotion
	FRootMotionSourceGroup ServerRootMotion;
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

	// Extract bHasAnimRootMotion
	bool bHasAnimRootMotion;
	bHasAnimRootMotion = Op.Request.field_bhasanimrootmotion();

	// Extract ServerMontageTrackPosition
	float ServerMontageTrackPosition;
	ServerMontageTrackPosition = Op.Request.field_servermontagetrackposition();

	// Extract ServerLoc
	FVector ServerLoc;
	{
		auto& Vector = Op.Request.field_serverloc();
		ServerLoc.X = Vector.x();
		ServerLoc.Y = Vector.y();
		ServerLoc.Z = Vector.z();
	}

	// Extract ServerRotation
	FVector_NetQuantizeNormal ServerRotation;
	{
		auto& Vector = Op.Request.field_serverrotation();
		ServerRotation.X = Vector.x();
		ServerRotation.Y = Vector.y();
		ServerRotation.Z = Vector.z();
	}

	// Extract ServerVelZ
	float ServerVelZ;
	ServerVelZ = Op.Request.field_servervelz();

	// Extract ServerBase
	UPrimitiveComponent* ServerBase;
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
				ServerBase = static_cast<UPrimitiveComponent*>(PackageMap->GetObjectFromNetGUID(NetGUID, true));
			}
			else
			{
				UE_LOG(LogSpatialOSInterop, Warning, TEXT("%s: ClientAdjustRootMotionSourcePosition_Receiver: ServerBase (entity id %llu, offset %u) is not resolved on this worker. Sending command failure."),
					*Interop->GetSpatialOS()->GetWorkerId(),
					ObjectRef.entity(),
					ObjectRef.offset());
				SendRPCResponse<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientadjustrootmotionsourceposition>(Op, false, TEXT("ServerBase is unresolved on the target worker"));
				return;
			}
		}
	}

	// Extract ServerBoneName
	FName ServerBoneName;
	ServerBoneName = FName((Op.Request.field_serverbonename()).data());

	// Extract bHasBase
	bool bHasBase;
	bHasBase = Op.Request.field_bhasbase();

	// Extract bBaseRelativePosition
	bool bBaseRelativePosition;
	bBaseRelativePosition = Op.Request.field_bbaserelativeposition();

	// Extract ServerMovementMode
	uint8 ServerMovementMode;
	ServerMovementMode = uint8(uint8(Op.Request.field_servermovementmode()));

	// Call implementation and send command response.
	UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Receiving RPC: ClientAdjustRootMotionSourcePosition, target: %s (entity ID %llu, offset: %u)"),
		*Interop->GetSpatialOS()->GetWorkerId(),
		*TargetObject->GetName(),
		TargetObjectRef.entity(),
		TargetObjectRef.offset());
	TargetObject->ClientAdjustRootMotionSourcePosition_Implementation(TimeStamp, ServerRootMotion, bHasAnimRootMotion, ServerMontageTrackPosition, ServerLoc, ServerRotation, ServerVelZ, ServerBase, ServerBoneName, bHasBase, bBaseRelativePosition, ServerMovementMode);
	SendRPCResponse<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientadjustrootmotionsourceposition>(Op, true, FString());
}

void USpatialTypeBinding_Character::ClientAdjustRootMotionPosition_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientadjustrootmotionposition>& Op)
{
	improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
	FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialOSInterop, Warning, TEXT("%s: ClientAdjustRootMotionPosition_Receiver: Target object (entity id %llu, offset %u) is not resolved on this worker. Sending command failure."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		SendRPCResponse<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientadjustrootmotionposition>(Op, false, TEXT("Target object is unresolved on the target worker"));
		return;
	}
	UCharacterMovementComponent* TargetObject = Cast<UCharacterMovementComponent>(PackageMap->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("%s: ClientAdjustRootMotionPosition_Receiver: Entity ID %llu (NetGUID %s) does not correspond to a UObject."), *Interop->GetSpatialOS()->GetWorkerId(), TargetObjectRef.entity(), *TargetNetGUID.ToString());

	// Extract TimeStamp
	float TimeStamp;
	TimeStamp = Op.Request.field_timestamp();

	// Extract ServerMontageTrackPosition
	float ServerMontageTrackPosition;
	ServerMontageTrackPosition = Op.Request.field_servermontagetrackposition();

	// Extract ServerLoc
	FVector ServerLoc;
	{
		auto& Vector = Op.Request.field_serverloc();
		ServerLoc.X = Vector.x();
		ServerLoc.Y = Vector.y();
		ServerLoc.Z = Vector.z();
	}

	// Extract ServerRotation
	FVector_NetQuantizeNormal ServerRotation;
	{
		auto& Vector = Op.Request.field_serverrotation();
		ServerRotation.X = Vector.x();
		ServerRotation.Y = Vector.y();
		ServerRotation.Z = Vector.z();
	}

	// Extract ServerVelZ
	float ServerVelZ;
	ServerVelZ = Op.Request.field_servervelz();

	// Extract ServerBase
	UPrimitiveComponent* ServerBase;
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
				ServerBase = static_cast<UPrimitiveComponent*>(PackageMap->GetObjectFromNetGUID(NetGUID, true));
			}
			else
			{
				UE_LOG(LogSpatialOSInterop, Warning, TEXT("%s: ClientAdjustRootMotionPosition_Receiver: ServerBase (entity id %llu, offset %u) is not resolved on this worker. Sending command failure."),
					*Interop->GetSpatialOS()->GetWorkerId(),
					ObjectRef.entity(),
					ObjectRef.offset());
				SendRPCResponse<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientadjustrootmotionposition>(Op, false, TEXT("ServerBase is unresolved on the target worker"));
				return;
			}
		}
	}

	// Extract ServerBoneName
	FName ServerBoneName;
	ServerBoneName = FName((Op.Request.field_serverbonename()).data());

	// Extract bHasBase
	bool bHasBase;
	bHasBase = Op.Request.field_bhasbase();

	// Extract bBaseRelativePosition
	bool bBaseRelativePosition;
	bBaseRelativePosition = Op.Request.field_bbaserelativeposition();

	// Extract ServerMovementMode
	uint8 ServerMovementMode;
	ServerMovementMode = uint8(uint8(Op.Request.field_servermovementmode()));

	// Call implementation and send command response.
	UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Receiving RPC: ClientAdjustRootMotionPosition, target: %s (entity ID %llu, offset: %u)"),
		*Interop->GetSpatialOS()->GetWorkerId(),
		*TargetObject->GetName(),
		TargetObjectRef.entity(),
		TargetObjectRef.offset());
	TargetObject->ClientAdjustRootMotionPosition_Implementation(TimeStamp, ServerMontageTrackPosition, ServerLoc, ServerRotation, ServerVelZ, ServerBase, ServerBoneName, bHasBase, bBaseRelativePosition, ServerMovementMode);
	SendRPCResponse<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientadjustrootmotionposition>(Op, true, FString());
}

void USpatialTypeBinding_Character::ClientAdjustPosition_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientadjustposition>& Op)
{
	improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
	FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialOSInterop, Warning, TEXT("%s: ClientAdjustPosition_Receiver: Target object (entity id %llu, offset %u) is not resolved on this worker. Sending command failure."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		SendRPCResponse<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientadjustposition>(Op, false, TEXT("Target object is unresolved on the target worker"));
		return;
	}
	UCharacterMovementComponent* TargetObject = Cast<UCharacterMovementComponent>(PackageMap->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("%s: ClientAdjustPosition_Receiver: Entity ID %llu (NetGUID %s) does not correspond to a UObject."), *Interop->GetSpatialOS()->GetWorkerId(), TargetObjectRef.entity(), *TargetNetGUID.ToString());

	// Extract TimeStamp
	float TimeStamp;
	TimeStamp = Op.Request.field_timestamp();

	// Extract NewLoc
	FVector NewLoc;
	{
		auto& Vector = Op.Request.field_newloc();
		NewLoc.X = Vector.x();
		NewLoc.Y = Vector.y();
		NewLoc.Z = Vector.z();
	}

	// Extract NewVel
	FVector NewVel;
	{
		auto& Vector = Op.Request.field_newvel();
		NewVel.X = Vector.x();
		NewVel.Y = Vector.y();
		NewVel.Z = Vector.z();
	}

	// Extract NewBase
	UPrimitiveComponent* NewBase;
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
				NewBase = static_cast<UPrimitiveComponent*>(PackageMap->GetObjectFromNetGUID(NetGUID, true));
			}
			else
			{
				UE_LOG(LogSpatialOSInterop, Warning, TEXT("%s: ClientAdjustPosition_Receiver: NewBase (entity id %llu, offset %u) is not resolved on this worker. Sending command failure."),
					*Interop->GetSpatialOS()->GetWorkerId(),
					ObjectRef.entity(),
					ObjectRef.offset());
				SendRPCResponse<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientadjustposition>(Op, false, TEXT("NewBase is unresolved on the target worker"));
				return;
			}
		}
	}

	// Extract NewBaseBoneName
	FName NewBaseBoneName;
	NewBaseBoneName = FName((Op.Request.field_newbasebonename()).data());

	// Extract bHasBase
	bool bHasBase;
	bHasBase = Op.Request.field_bhasbase();

	// Extract bBaseRelativePosition
	bool bBaseRelativePosition;
	bBaseRelativePosition = Op.Request.field_bbaserelativeposition();

	// Extract ServerMovementMode
	uint8 ServerMovementMode;
	ServerMovementMode = uint8(uint8(Op.Request.field_servermovementmode()));

	// Call implementation and send command response.
	UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Receiving RPC: ClientAdjustPosition, target: %s (entity ID %llu, offset: %u)"),
		*Interop->GetSpatialOS()->GetWorkerId(),
		*TargetObject->GetName(),
		TargetObjectRef.entity(),
		TargetObjectRef.offset());
	TargetObject->ClientAdjustPosition_Implementation(TimeStamp, NewLoc, NewVel, NewBase, NewBaseBoneName, bHasBase, bBaseRelativePosition, ServerMovementMode);
	SendRPCResponse<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientadjustposition>(Op, true, FString());
}

void USpatialTypeBinding_Character::ClientAckGoodMove_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientackgoodmove>& Op)
{
	improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
	FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialOSInterop, Warning, TEXT("%s: ClientAckGoodMove_Receiver: Target object (entity id %llu, offset %u) is not resolved on this worker. Sending command failure."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		SendRPCResponse<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientackgoodmove>(Op, false, TEXT("Target object is unresolved on the target worker"));
		return;
	}
	UCharacterMovementComponent* TargetObject = Cast<UCharacterMovementComponent>(PackageMap->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("%s: ClientAckGoodMove_Receiver: Entity ID %llu (NetGUID %s) does not correspond to a UObject."), *Interop->GetSpatialOS()->GetWorkerId(), TargetObjectRef.entity(), *TargetNetGUID.ToString());

	// Extract TimeStamp
	float TimeStamp;
	TimeStamp = Op.Request.field_timestamp();

	// Call implementation and send command response.
	UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Receiving RPC: ClientAckGoodMove, target: %s (entity ID %llu, offset: %u)"),
		*Interop->GetSpatialOS()->GetWorkerId(),
		*TargetObject->GetName(),
		TargetObjectRef.entity(),
		TargetObjectRef.offset());
	TargetObject->ClientAckGoodMove_Implementation(TimeStamp);
	SendRPCResponse<improbable::unreal::UnrealCharacterClientRPCs::Commands::Clientackgoodmove>(Op, true, FString());
}

void USpatialTypeBinding_Character::ServerMoveOld_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealCharacterServerRPCs::Commands::Servermoveold>& Op)
{
	improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
	FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialOSInterop, Warning, TEXT("%s: ServerMoveOld_Receiver: Target object (entity id %llu, offset %u) is not resolved on this worker. Sending command failure."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		SendRPCResponse<improbable::unreal::UnrealCharacterServerRPCs::Commands::Servermoveold>(Op, false, TEXT("Target object is unresolved on the target worker"));
		return;
	}
	UCharacterMovementComponent* TargetObject = Cast<UCharacterMovementComponent>(PackageMap->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("%s: ServerMoveOld_Receiver: Entity ID %llu (NetGUID %s) does not correspond to a UObject."), *Interop->GetSpatialOS()->GetWorkerId(), TargetObjectRef.entity(), *TargetNetGUID.ToString());

	// Extract OldTimeStamp
	float OldTimeStamp;
	OldTimeStamp = Op.Request.field_oldtimestamp();

	// Extract OldAccel
	FVector_NetQuantize10 OldAccel;
	{
		auto& Vector = Op.Request.field_oldaccel();
		OldAccel.X = Vector.x();
		OldAccel.Y = Vector.y();
		OldAccel.Z = Vector.z();
	}

	// Extract OldMoveFlags
	uint8 OldMoveFlags;
	OldMoveFlags = uint8(uint8(Op.Request.field_oldmoveflags()));

	// Call implementation and send command response.
	UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Receiving RPC: ServerMoveOld, target: %s (entity ID %llu, offset: %u)"),
		*Interop->GetSpatialOS()->GetWorkerId(),
		*TargetObject->GetName(),
		TargetObjectRef.entity(),
		TargetObjectRef.offset());
	TargetObject->ServerMoveOld_Implementation(OldTimeStamp, OldAccel, OldMoveFlags);
	SendRPCResponse<improbable::unreal::UnrealCharacterServerRPCs::Commands::Servermoveold>(Op, true, FString());
}

void USpatialTypeBinding_Character::ServerMoveDualHybridRootMotion_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealCharacterServerRPCs::Commands::Servermovedualhybridrootmotion>& Op)
{
	improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
	FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialOSInterop, Warning, TEXT("%s: ServerMoveDualHybridRootMotion_Receiver: Target object (entity id %llu, offset %u) is not resolved on this worker. Sending command failure."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		SendRPCResponse<improbable::unreal::UnrealCharacterServerRPCs::Commands::Servermovedualhybridrootmotion>(Op, false, TEXT("Target object is unresolved on the target worker"));
		return;
	}
	UCharacterMovementComponent* TargetObject = Cast<UCharacterMovementComponent>(PackageMap->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("%s: ServerMoveDualHybridRootMotion_Receiver: Entity ID %llu (NetGUID %s) does not correspond to a UObject."), *Interop->GetSpatialOS()->GetWorkerId(), TargetObjectRef.entity(), *TargetNetGUID.ToString());

	// Extract TimeStamp0
	float TimeStamp0;
	TimeStamp0 = Op.Request.field_timestamp0();

	// Extract InAccel0
	FVector_NetQuantize10 InAccel0;
	{
		auto& Vector = Op.Request.field_inaccel0();
		InAccel0.X = Vector.x();
		InAccel0.Y = Vector.y();
		InAccel0.Z = Vector.z();
	}

	// Extract PendingFlags
	uint8 PendingFlags;
	PendingFlags = uint8(uint8(Op.Request.field_pendingflags()));

	// Extract View0
	uint32 View0;
	View0 = uint32(Op.Request.field_view0());

	// Extract TimeStamp
	float TimeStamp;
	TimeStamp = Op.Request.field_timestamp();

	// Extract InAccel
	FVector_NetQuantize10 InAccel;
	{
		auto& Vector = Op.Request.field_inaccel();
		InAccel.X = Vector.x();
		InAccel.Y = Vector.y();
		InAccel.Z = Vector.z();
	}

	// Extract ClientLoc
	FVector_NetQuantize100 ClientLoc;
	{
		auto& Vector = Op.Request.field_clientloc();
		ClientLoc.X = Vector.x();
		ClientLoc.Y = Vector.y();
		ClientLoc.Z = Vector.z();
	}

	// Extract NewFlags
	uint8 NewFlags;
	NewFlags = uint8(uint8(Op.Request.field_newflags()));

	// Extract ClientRoll
	uint8 ClientRoll;
	ClientRoll = uint8(uint8(Op.Request.field_clientroll()));

	// Extract View
	uint32 View;
	View = uint32(Op.Request.field_view());

	// Extract ClientMovementBase
	UPrimitiveComponent* ClientMovementBase;
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
				ClientMovementBase = static_cast<UPrimitiveComponent*>(PackageMap->GetObjectFromNetGUID(NetGUID, true));
			}
			else
			{
				UE_LOG(LogSpatialOSInterop, Warning, TEXT("%s: ServerMoveDualHybridRootMotion_Receiver: ClientMovementBase (entity id %llu, offset %u) is not resolved on this worker. Sending command failure."),
					*Interop->GetSpatialOS()->GetWorkerId(),
					ObjectRef.entity(),
					ObjectRef.offset());
				SendRPCResponse<improbable::unreal::UnrealCharacterServerRPCs::Commands::Servermovedualhybridrootmotion>(Op, false, TEXT("ClientMovementBase is unresolved on the target worker"));
				return;
			}
		}
	}

	// Extract ClientBaseBoneName
	FName ClientBaseBoneName;
	ClientBaseBoneName = FName((Op.Request.field_clientbasebonename()).data());

	// Extract ClientMovementMode
	uint8 ClientMovementMode;
	ClientMovementMode = uint8(uint8(Op.Request.field_clientmovementmode()));

	// Call implementation and send command response.
	UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Receiving RPC: ServerMoveDualHybridRootMotion, target: %s (entity ID %llu, offset: %u)"),
		*Interop->GetSpatialOS()->GetWorkerId(),
		*TargetObject->GetName(),
		TargetObjectRef.entity(),
		TargetObjectRef.offset());
	TargetObject->ServerMoveDualHybridRootMotion_Implementation(TimeStamp0, InAccel0, PendingFlags, View0, TimeStamp, InAccel, ClientLoc, NewFlags, ClientRoll, View, ClientMovementBase, ClientBaseBoneName, ClientMovementMode);
	SendRPCResponse<improbable::unreal::UnrealCharacterServerRPCs::Commands::Servermovedualhybridrootmotion>(Op, true, FString());
}

void USpatialTypeBinding_Character::ServerMoveDual_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealCharacterServerRPCs::Commands::Servermovedual>& Op)
{
	improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
	FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialOSInterop, Warning, TEXT("%s: ServerMoveDual_Receiver: Target object (entity id %llu, offset %u) is not resolved on this worker. Sending command failure."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		SendRPCResponse<improbable::unreal::UnrealCharacterServerRPCs::Commands::Servermovedual>(Op, false, TEXT("Target object is unresolved on the target worker"));
		return;
	}
	UCharacterMovementComponent* TargetObject = Cast<UCharacterMovementComponent>(PackageMap->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("%s: ServerMoveDual_Receiver: Entity ID %llu (NetGUID %s) does not correspond to a UObject."), *Interop->GetSpatialOS()->GetWorkerId(), TargetObjectRef.entity(), *TargetNetGUID.ToString());

	// Extract TimeStamp0
	float TimeStamp0;
	TimeStamp0 = Op.Request.field_timestamp0();

	// Extract InAccel0
	FVector_NetQuantize10 InAccel0;
	{
		auto& Vector = Op.Request.field_inaccel0();
		InAccel0.X = Vector.x();
		InAccel0.Y = Vector.y();
		InAccel0.Z = Vector.z();
	}

	// Extract PendingFlags
	uint8 PendingFlags;
	PendingFlags = uint8(uint8(Op.Request.field_pendingflags()));

	// Extract View0
	uint32 View0;
	View0 = uint32(Op.Request.field_view0());

	// Extract TimeStamp
	float TimeStamp;
	TimeStamp = Op.Request.field_timestamp();

	// Extract InAccel
	FVector_NetQuantize10 InAccel;
	{
		auto& Vector = Op.Request.field_inaccel();
		InAccel.X = Vector.x();
		InAccel.Y = Vector.y();
		InAccel.Z = Vector.z();
	}

	// Extract ClientLoc
	FVector_NetQuantize100 ClientLoc;
	{
		auto& Vector = Op.Request.field_clientloc();
		ClientLoc.X = Vector.x();
		ClientLoc.Y = Vector.y();
		ClientLoc.Z = Vector.z();
	}

	// Extract NewFlags
	uint8 NewFlags;
	NewFlags = uint8(uint8(Op.Request.field_newflags()));

	// Extract ClientRoll
	uint8 ClientRoll;
	ClientRoll = uint8(uint8(Op.Request.field_clientroll()));

	// Extract View
	uint32 View;
	View = uint32(Op.Request.field_view());

	// Extract ClientMovementBase
	UPrimitiveComponent* ClientMovementBase;
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
				ClientMovementBase = static_cast<UPrimitiveComponent*>(PackageMap->GetObjectFromNetGUID(NetGUID, true));
			}
			else
			{
				UE_LOG(LogSpatialOSInterop, Warning, TEXT("%s: ServerMoveDual_Receiver: ClientMovementBase (entity id %llu, offset %u) is not resolved on this worker. Sending command failure."),
					*Interop->GetSpatialOS()->GetWorkerId(),
					ObjectRef.entity(),
					ObjectRef.offset());
				SendRPCResponse<improbable::unreal::UnrealCharacterServerRPCs::Commands::Servermovedual>(Op, false, TEXT("ClientMovementBase is unresolved on the target worker"));
				return;
			}
		}
	}

	// Extract ClientBaseBoneName
	FName ClientBaseBoneName;
	ClientBaseBoneName = FName((Op.Request.field_clientbasebonename()).data());

	// Extract ClientMovementMode
	uint8 ClientMovementMode;
	ClientMovementMode = uint8(uint8(Op.Request.field_clientmovementmode()));

	// Call implementation and send command response.
	UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Receiving RPC: ServerMoveDual, target: %s (entity ID %llu, offset: %u)"),
		*Interop->GetSpatialOS()->GetWorkerId(),
		*TargetObject->GetName(),
		TargetObjectRef.entity(),
		TargetObjectRef.offset());
	TargetObject->ServerMoveDual_Implementation(TimeStamp0, InAccel0, PendingFlags, View0, TimeStamp, InAccel, ClientLoc, NewFlags, ClientRoll, View, ClientMovementBase, ClientBaseBoneName, ClientMovementMode);
	SendRPCResponse<improbable::unreal::UnrealCharacterServerRPCs::Commands::Servermovedual>(Op, true, FString());
}

void USpatialTypeBinding_Character::ServerMove_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealCharacterServerRPCs::Commands::Servermove>& Op)
{
	improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
	FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialOSInterop, Warning, TEXT("%s: ServerMove_Receiver: Target object (entity id %llu, offset %u) is not resolved on this worker. Sending command failure."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		SendRPCResponse<improbable::unreal::UnrealCharacterServerRPCs::Commands::Servermove>(Op, false, TEXT("Target object is unresolved on the target worker"));
		return;
	}
	UCharacterMovementComponent* TargetObject = Cast<UCharacterMovementComponent>(PackageMap->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("%s: ServerMove_Receiver: Entity ID %llu (NetGUID %s) does not correspond to a UObject."), *Interop->GetSpatialOS()->GetWorkerId(), TargetObjectRef.entity(), *TargetNetGUID.ToString());

	// Extract TimeStamp
	float TimeStamp;
	TimeStamp = Op.Request.field_timestamp();

	// Extract InAccel
	FVector_NetQuantize10 InAccel;
	{
		auto& Vector = Op.Request.field_inaccel();
		InAccel.X = Vector.x();
		InAccel.Y = Vector.y();
		InAccel.Z = Vector.z();
	}

	// Extract ClientLoc
	FVector_NetQuantize100 ClientLoc;
	{
		auto& Vector = Op.Request.field_clientloc();
		ClientLoc.X = Vector.x();
		ClientLoc.Y = Vector.y();
		ClientLoc.Z = Vector.z();
	}

	// Extract CompressedMoveFlags
	uint8 CompressedMoveFlags;
	CompressedMoveFlags = uint8(uint8(Op.Request.field_compressedmoveflags()));

	// Extract ClientRoll
	uint8 ClientRoll;
	ClientRoll = uint8(uint8(Op.Request.field_clientroll()));

	// Extract View
	uint32 View;
	View = uint32(Op.Request.field_view());

	// Extract ClientMovementBase
	UPrimitiveComponent* ClientMovementBase;
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
				ClientMovementBase = static_cast<UPrimitiveComponent*>(PackageMap->GetObjectFromNetGUID(NetGUID, true));
			}
			else
			{
				UE_LOG(LogSpatialOSInterop, Warning, TEXT("%s: ServerMove_Receiver: ClientMovementBase (entity id %llu, offset %u) is not resolved on this worker. Sending command failure."),
					*Interop->GetSpatialOS()->GetWorkerId(),
					ObjectRef.entity(),
					ObjectRef.offset());
				SendRPCResponse<improbable::unreal::UnrealCharacterServerRPCs::Commands::Servermove>(Op, false, TEXT("ClientMovementBase is unresolved on the target worker"));
				return;
			}
		}
	}

	// Extract ClientBaseBoneName
	FName ClientBaseBoneName;
	ClientBaseBoneName = FName((Op.Request.field_clientbasebonename()).data());

	// Extract ClientMovementMode
	uint8 ClientMovementMode;
	ClientMovementMode = uint8(uint8(Op.Request.field_clientmovementmode()));

	// Call implementation and send command response.
	UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Receiving RPC: ServerMove, target: %s (entity ID %llu, offset: %u)"),
		*Interop->GetSpatialOS()->GetWorkerId(),
		*TargetObject->GetName(),
		TargetObjectRef.entity(),
		TargetObjectRef.offset());
	TargetObject->ServerMove_Implementation(TimeStamp, InAccel, ClientLoc, CompressedMoveFlags, ClientRoll, View, ClientMovementBase, ClientBaseBoneName, ClientMovementMode);
	SendRPCResponse<improbable::unreal::UnrealCharacterServerRPCs::Commands::Servermove>(Op, true, FString());
}
