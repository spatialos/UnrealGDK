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

	worker::Entity CreateActorEntity(const FString& ClientWorkerId, const FVector& Position, const FString& Metadata, const FPropertyChangeState& InitialChanges, USpatialActorChannel* Channel) const override;
	void SendComponentUpdates(const FPropertyChangeState& Changes, USpatialActorChannel* Channel, const worker::EntityId& EntityId) const override;
	void SendRPCCommand(UObject* TargetObject, const UFunction* const Function, FFrame* const Frame) override;

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
	using FRPCSender = void (USpatialTypeBinding_PlayerController::*)(worker::Connection* const, struct FFrame* const, UObject*);
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
	void OnServerStartedVisualLogger_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientWasKicked_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientVoiceHandshakeComplete_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientUpdateLevelStreamingStatus_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientUnmutePlayer_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientTravelInternal_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientTeamMessage_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientStopForceFeedback_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientStopCameraShake_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientStopCameraAnim_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientStartOnlineSession_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientSpawnCameraLensEffect_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientSetViewTarget_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientSetSpectatorWaiting_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientSetHUD_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientSetForceMipLevelsToBeResident_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientSetCinematicMode_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientSetCameraMode_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientSetCameraFade_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientSetBlockOnAsyncLoading_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientReturnToMainMenu_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientRetryClientRestart_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientRestart_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientReset_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientRepObjRef_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientReceiveLocalizedMessage_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientPrestreamTextures_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientPrepareMapChange_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientPlaySoundAtLocation_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientPlaySound_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientPlayForceFeedback_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientPlayCameraShake_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientPlayCameraAnim_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientMutePlayer_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientMessage_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientIgnoreMoveInput_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientIgnoreLookInput_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientGotoState_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientGameEnded_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientForceGarbageCollection_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientFlushLevelStreaming_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientEndOnlineSession_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientEnableNetworkVoice_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientCommitMapChange_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientClearCameraLensEffects_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientCapBandwidth_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientCancelPendingMapChange_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientAddTextureStreamingLoc_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientSetRotation_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ClientSetLocation_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ServerViewSelf_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ServerViewPrevPlayer_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ServerViewNextPlayer_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ServerVerifyViewTarget_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ServerUpdateLevelVisibility_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ServerUpdateCamera_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ServerUnmutePlayer_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ServerToggleAILogging_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ServerShortTimeout_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ServerSetSpectatorWaiting_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ServerSetSpectatorLocation_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ServerRestartPlayer_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ServerPause_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ServerNotifyLoadedWorld_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ServerMutePlayer_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ServerCheckClientPossessionReliable_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ServerCheckClientPossession_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ServerChangeName_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ServerCamera_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);
	void ServerAcknowledgePossession_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject);

	// RPC sender response functions.
	void OnServerStartedVisualLogger_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Onserverstartedvisuallogger>& Op);
	void ClientWasKicked_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientwaskicked>& Op);
	void ClientVoiceHandshakeComplete_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientvoicehandshakecomplete>& Op);
	void ClientUpdateLevelStreamingStatus_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientupdatelevelstreamingstatus>& Op);
	void ClientUnmutePlayer_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientunmuteplayer>& Op);
	void ClientTravelInternal_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clienttravelinternal>& Op);
	void ClientTeamMessage_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientteammessage>& Op);
	void ClientStopForceFeedback_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstopforcefeedback>& Op);
	void ClientStopCameraShake_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstopcamerashake>& Op);
	void ClientStopCameraAnim_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstopcameraanim>& Op);
	void ClientStartOnlineSession_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstartonlinesession>& Op);
	void ClientSpawnCameraLensEffect_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientspawncameralenseffect>& Op);
	void ClientSetViewTarget_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetviewtarget>& Op);
	void ClientSetSpectatorWaiting_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetspectatorwaiting>& Op);
	void ClientSetHUD_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsethud>& Op);
	void ClientSetForceMipLevelsToBeResident_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetforcemiplevelstoberesident>& Op);
	void ClientSetCinematicMode_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetcinematicmode>& Op);
	void ClientSetCameraMode_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetcameramode>& Op);
	void ClientSetCameraFade_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetcamerafade>& Op);
	void ClientSetBlockOnAsyncLoading_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetblockonasyncloading>& Op);
	void ClientReturnToMainMenu_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientreturntomainmenu>& Op);
	void ClientRetryClientRestart_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientretryclientrestart>& Op);
	void ClientRestart_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientrestart>& Op);
	void ClientReset_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientreset>& Op);
	void ClientRepObjRef_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientrepobjref>& Op);
	void ClientReceiveLocalizedMessage_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientreceivelocalizedmessage>& Op);
	void ClientPrestreamTextures_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientprestreamtextures>& Op);
	void ClientPrepareMapChange_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientpreparemapchange>& Op);
	void ClientPlaySoundAtLocation_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaysoundatlocation>& Op);
	void ClientPlaySound_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaysound>& Op);
	void ClientPlayForceFeedback_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplayforcefeedback>& Op);
	void ClientPlayCameraShake_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaycamerashake>& Op);
	void ClientPlayCameraAnim_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaycameraanim>& Op);
	void ClientMutePlayer_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientmuteplayer>& Op);
	void ClientMessage_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientmessage>& Op);
	void ClientIgnoreMoveInput_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientignoremoveinput>& Op);
	void ClientIgnoreLookInput_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientignorelookinput>& Op);
	void ClientGotoState_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientgotostate>& Op);
	void ClientGameEnded_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientgameended>& Op);
	void ClientForceGarbageCollection_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientforcegarbagecollection>& Op);
	void ClientFlushLevelStreaming_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientflushlevelstreaming>& Op);
	void ClientEndOnlineSession_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientendonlinesession>& Op);
	void ClientEnableNetworkVoice_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientenablenetworkvoice>& Op);
	void ClientCommitMapChange_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientcommitmapchange>& Op);
	void ClientClearCameraLensEffects_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientclearcameralenseffects>& Op);
	void ClientCapBandwidth_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientcapbandwidth>& Op);
	void ClientCancelPendingMapChange_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientcancelpendingmapchange>& Op);
	void ClientAddTextureStreamingLoc_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientaddtexturestreamingloc>& Op);
	void ClientSetRotation_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetrotation>& Op);
	void ClientSetLocation_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetlocation>& Op);
	void ServerViewSelf_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverviewself>& Op);
	void ServerViewPrevPlayer_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverviewprevplayer>& Op);
	void ServerViewNextPlayer_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverviewnextplayer>& Op);
	void ServerVerifyViewTarget_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serververifyviewtarget>& Op);
	void ServerUpdateLevelVisibility_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverupdatelevelvisibility>& Op);
	void ServerUpdateCamera_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverupdatecamera>& Op);
	void ServerUnmutePlayer_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverunmuteplayer>& Op);
	void ServerToggleAILogging_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servertoggleailogging>& Op);
	void ServerShortTimeout_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servershorttimeout>& Op);
	void ServerSetSpectatorWaiting_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serversetspectatorwaiting>& Op);
	void ServerSetSpectatorLocation_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serversetspectatorlocation>& Op);
	void ServerRestartPlayer_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverrestartplayer>& Op);
	void ServerPause_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverpause>& Op);
	void ServerNotifyLoadedWorld_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servernotifyloadedworld>& Op);
	void ServerMutePlayer_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servermuteplayer>& Op);
	void ServerCheckClientPossessionReliable_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servercheckclientpossessionreliable>& Op);
	void ServerCheckClientPossession_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servercheckclientpossession>& Op);
	void ServerChangeName_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverchangename>& Op);
	void ServerCamera_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servercamera>& Op);
	void ServerAcknowledgePossession_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serveracknowledgepossession>& Op);

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
