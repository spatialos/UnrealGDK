// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// Note that this file has been generated automatically
#pragma once

#include <improbable/worker.h>
#include <improbable/view.h>
#include <unreal/generated/UnrealPlayerController.h>
#include <unreal/core_types.h>
#include "SpatialHandlePropertyMap.h"
#include "SpatialTypeBinding.h"
#include "SpatialTypeBinding_PlayerController.generated.h"

UCLASS()
class USpatialTypeBinding_PlayerController : public USpatialTypeBinding
{
	GENERATED_BODY()

public:
	static const FRepHandlePropertyMap& GetHandlePropertyMap();
	
	void Init(USpatialUpdateInterop* InUpdateInterop, USpatialPackageMapClient* InPackageMap) override;
	void BindToView() override;
	void UnbindFromView() override;
	worker::ComponentId GetReplicatedGroupComponentId(EReplicatedPropertyGroup Group) const override;
	
	worker::Entity CreateActorEntity(const FVector& Position, const FString& Metadata, const FPropertyChangeState& InitialChanges, USpatialActorChannel* Channel) const override;
	void SendComponentUpdates(const FPropertyChangeState& Changes, USpatialActorChannel* Channel, const worker::EntityId& EntityId) const override;
	void SendRPCCommand(const UFunction* const Function, FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target) override;
	
	void ApplyQueuedStateToChannel(USpatialActorChannel* ActorChannel) override;

private:
	worker::Dispatcher::CallbackKey SingleClientAddCallback;
	worker::Dispatcher::CallbackKey SingleClientUpdateCallback;
	worker::Dispatcher::CallbackKey MultiClientAddCallback;
	worker::Dispatcher::CallbackKey MultiClientUpdateCallback;

	// Pending updates.
	TMap<worker::EntityId, improbable::unreal::UnrealPlayerControllerSingleClientReplicatedData::Data> PendingSingleClientData;
	TMap<worker::EntityId, improbable::unreal::UnrealPlayerControllerMultiClientReplicatedData::Data> PendingMultiClientData;

	// RPC sender and receiver callbacks.
	using FRPCSender = void (USpatialTypeBinding_PlayerController::*)(worker::Connection* const, struct FFrame* const, USpatialActorChannel*, const worker::EntityId&);
	TMap<FName, FRPCSender> RPCToSenderMap;
	TArray<worker::Dispatcher::CallbackKey> RPCReceiverCallbacks;

	// Component update helper functions.
	void BuildSpatialComponentUpdate(
		const FPropertyChangeState& Changes,
		USpatialActorChannel* Channel,
		improbable::unreal::UnrealPlayerControllerSingleClientReplicatedData::Update& SingleClientUpdate,
		bool& bSingleClientUpdateChanged,
		improbable::unreal::UnrealPlayerControllerMultiClientReplicatedData::Update& MultiClientUpdate,
		bool& bMultiClientUpdateChanged) const;
	void ApplyUpdateToSpatial_SingleClient(
		const uint8* RESTRICT Data,
		int32 Handle,
		UProperty* Property,
		USpatialActorChannel* Channel,
		improbable::unreal::UnrealPlayerControllerSingleClientReplicatedData::Update& OutUpdate) const;
	void ApplyUpdateToSpatial_MultiClient(
		const uint8* RESTRICT Data,
		int32 Handle,
		UProperty* Property,
		USpatialActorChannel* Channel,
		improbable::unreal::UnrealPlayerControllerMultiClientReplicatedData::Update& OutUpdate) const;
	void ReceiveUpdateFromSpatial_SingleClient(
		USpatialActorChannel* ActorChannel,
		const improbable::unreal::UnrealPlayerControllerSingleClientReplicatedData::Update& Update) const;
	void ReceiveUpdateFromSpatial_MultiClient(
		USpatialActorChannel* ActorChannel,
		const improbable::unreal::UnrealPlayerControllerMultiClientReplicatedData::Update& Update) const;

	// RPC sender functions.
	void OnServerStartedVisualLogger_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ClientWasKicked_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ClientVoiceHandshakeComplete_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ClientUpdateLevelStreamingStatus_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ClientUnmutePlayer_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ClientTravelInternal_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ClientTeamMessage_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ClientStopForceFeedback_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ClientStopCameraShake_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ClientStopCameraAnim_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ClientStartOnlineSession_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ClientSpawnCameraLensEffect_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ClientSetViewTarget_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ClientSetSpectatorWaiting_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ClientSetHUD_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ClientSetForceMipLevelsToBeResident_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ClientSetCinematicMode_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ClientSetCameraMode_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ClientSetCameraFade_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ClientSetBlockOnAsyncLoading_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ClientReturnToMainMenu_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ClientRetryClientRestart_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ClientRestart_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ClientReset_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ClientRepObjRef_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ClientReceiveLocalizedMessage_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ClientPrestreamTextures_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ClientPrepareMapChange_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ClientPlaySoundAtLocation_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ClientPlaySound_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ClientPlayForceFeedback_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ClientPlayCameraShake_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ClientPlayCameraAnim_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ClientMutePlayer_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ClientMessage_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ClientIgnoreMoveInput_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ClientIgnoreLookInput_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ClientGotoState_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ClientGameEnded_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ClientForceGarbageCollection_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ClientFlushLevelStreaming_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ClientEndOnlineSession_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ClientEnableNetworkVoice_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ClientCommitMapChange_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ClientClearCameraLensEffects_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ClientCapBandwidth_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ClientCancelPendingMapChange_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ClientAddTextureStreamingLoc_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ClientSetRotation_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ClientSetLocation_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ServerViewSelf_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ServerViewPrevPlayer_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ServerViewNextPlayer_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ServerVerifyViewTarget_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ServerUpdateLevelVisibility_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ServerUpdateCamera_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ServerUnmutePlayer_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ServerToggleAILogging_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ServerShortTimeout_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ServerSetSpectatorWaiting_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ServerSetSpectatorLocation_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ServerRestartPlayer_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ServerPause_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ServerNotifyLoadedWorld_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ServerMutePlayer_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ServerCheckClientPossessionReliable_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ServerCheckClientPossession_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ServerChangeName_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ServerCamera_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);
	void ServerAcknowledgePossession_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, USpatialActorChannel* Channel, const worker::EntityId& Target);

	// RPC receiver functions.
	void OnServerStartedVisualLogger_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Onserverstartedvisuallogger>& Op);
	void ClientWasKicked_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientwaskicked>& Op);
	void ClientVoiceHandshakeComplete_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientvoicehandshakecomplete>& Op);
	void ClientUpdateLevelStreamingStatus_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientupdatelevelstreamingstatus>& Op);
	void ClientUnmutePlayer_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientunmuteplayer>& Op);
	void ClientTravelInternal_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clienttravelinternal>& Op);
	void ClientTeamMessage_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientteammessage>& Op);
	void ClientStopForceFeedback_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstopforcefeedback>& Op);
	void ClientStopCameraShake_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstopcamerashake>& Op);
	void ClientStopCameraAnim_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstopcameraanim>& Op);
	void ClientStartOnlineSession_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstartonlinesession>& Op);
	void ClientSpawnCameraLensEffect_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientspawncameralenseffect>& Op);
	void ClientSetViewTarget_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetviewtarget>& Op);
	void ClientSetSpectatorWaiting_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetspectatorwaiting>& Op);
	void ClientSetHUD_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsethud>& Op);
	void ClientSetForceMipLevelsToBeResident_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetforcemiplevelstoberesident>& Op);
	void ClientSetCinematicMode_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetcinematicmode>& Op);
	void ClientSetCameraMode_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetcameramode>& Op);
	void ClientSetCameraFade_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetcamerafade>& Op);
	void ClientSetBlockOnAsyncLoading_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetblockonasyncloading>& Op);
	void ClientReturnToMainMenu_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientreturntomainmenu>& Op);
	void ClientRetryClientRestart_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientretryclientrestart>& Op);
	void ClientRestart_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientrestart>& Op);
	void ClientReset_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientreset>& Op);
	void ClientRepObjRef_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientrepobjref>& Op);
	void ClientReceiveLocalizedMessage_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientreceivelocalizedmessage>& Op);
	void ClientPrestreamTextures_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientprestreamtextures>& Op);
	void ClientPrepareMapChange_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientpreparemapchange>& Op);
	void ClientPlaySoundAtLocation_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaysoundatlocation>& Op);
	void ClientPlaySound_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaysound>& Op);
	void ClientPlayForceFeedback_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplayforcefeedback>& Op);
	void ClientPlayCameraShake_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaycamerashake>& Op);
	void ClientPlayCameraAnim_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaycameraanim>& Op);
	void ClientMutePlayer_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientmuteplayer>& Op);
	void ClientMessage_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientmessage>& Op);
	void ClientIgnoreMoveInput_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientignoremoveinput>& Op);
	void ClientIgnoreLookInput_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientignorelookinput>& Op);
	void ClientGotoState_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientgotostate>& Op);
	void ClientGameEnded_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientgameended>& Op);
	void ClientForceGarbageCollection_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientforcegarbagecollection>& Op);
	void ClientFlushLevelStreaming_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientflushlevelstreaming>& Op);
	void ClientEndOnlineSession_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientendonlinesession>& Op);
	void ClientEnableNetworkVoice_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientenablenetworkvoice>& Op);
	void ClientCommitMapChange_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientcommitmapchange>& Op);
	void ClientClearCameraLensEffects_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientclearcameralenseffects>& Op);
	void ClientCapBandwidth_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientcapbandwidth>& Op);
	void ClientCancelPendingMapChange_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientcancelpendingmapchange>& Op);
	void ClientAddTextureStreamingLoc_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientaddtexturestreamingloc>& Op);
	void ClientSetRotation_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetrotation>& Op);
	void ClientSetLocation_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetlocation>& Op);
	void ServerViewSelf_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverviewself>& Op);
	void ServerViewPrevPlayer_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverviewprevplayer>& Op);
	void ServerViewNextPlayer_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverviewnextplayer>& Op);
	void ServerVerifyViewTarget_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serververifyviewtarget>& Op);
	void ServerUpdateLevelVisibility_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverupdatelevelvisibility>& Op);
	void ServerUpdateCamera_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverupdatecamera>& Op);
	void ServerUnmutePlayer_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverunmuteplayer>& Op);
	void ServerToggleAILogging_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servertoggleailogging>& Op);
	void ServerShortTimeout_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servershorttimeout>& Op);
	void ServerSetSpectatorWaiting_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serversetspectatorwaiting>& Op);
	void ServerSetSpectatorLocation_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serversetspectatorlocation>& Op);
	void ServerRestartPlayer_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverrestartplayer>& Op);
	void ServerPause_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverpause>& Op);
	void ServerNotifyLoadedWorld_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servernotifyloadedworld>& Op);
	void ServerMutePlayer_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servermuteplayer>& Op);
	void ServerCheckClientPossessionReliable_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servercheckclientpossessionreliable>& Op);
	void ServerCheckClientPossession_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servercheckclientpossession>& Op);
	void ServerChangeName_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverchangename>& Op);
	void ServerCamera_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servercamera>& Op);
	void ServerAcknowledgePossession_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serveracknowledgepossession>& Op);
};
