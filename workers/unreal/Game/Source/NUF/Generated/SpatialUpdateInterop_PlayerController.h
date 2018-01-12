// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// Note that this file has been generated automatically

#pragma once

#include <improbable/worker.h>
#include <improbable/view.h>
#include <unreal/generated/UnrealPlayerController.h>
#include <unreal/core_types.h>
#include "SpatialHandlePropertyMap.h"
#include "SpatialUpdateInterop.h"

const FRepHandlePropertyMap& GetHandlePropertyMap_PlayerController();

class FSpatialTypeBinding_PlayerController : public FSpatialTypeBinding
{
public:
	void Init(USpatialUpdateInterop* InUpdateInterop, UPackageMap* InPackageMap) override;
	void BindToView() override;
	void UnbindFromView() override;
	worker::ComponentId GetReplicatedGroupComponentId(EReplicatedPropertyGroup Group) const override;
	void SendComponentUpdates(FOutBunch* BunchPtr, const worker::EntityId& EntityId) const override;
	void SendRPCCommand(UFunction* Function, FFrame* RPCFrame, worker::EntityId Target) override;
private:
	TMap<FName, FRPCSender> RPCToSenderMap;
	worker::Dispatcher::CallbackKey SingleClientCallback;
	worker::Dispatcher::CallbackKey MultiClientCallback;
	TArray<worker::Dispatcher::CallbackKey> RPCReceiverCallbacks;

	void OnServerStartedVisualLoggerReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Onserverstartedvisuallogger>& Op);
	void ClientWasKickedReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientwaskicked>& Op);
	void ClientVoiceHandshakeCompleteReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientvoicehandshakecomplete>& Op);
	void ClientUpdateLevelStreamingStatusReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientupdatelevelstreamingstatus>& Op);
	void ClientUnmutePlayerReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientunmuteplayer>& Op);
	void ClientTravelInternalReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clienttravelinternal>& Op);
	void ClientTeamMessageReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientteammessage>& Op);
	void ClientStopForceFeedbackReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstopforcefeedback>& Op);
	void ClientStopCameraShakeReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstopcamerashake>& Op);
	void ClientStopCameraAnimReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstopcameraanim>& Op);
	void ClientStartOnlineSessionReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstartonlinesession>& Op);
	void ClientSpawnCameraLensEffectReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientspawncameralenseffect>& Op);
	void ClientSetViewTargetReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetviewtarget>& Op);
	void ClientSetSpectatorWaitingReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetspectatorwaiting>& Op);
	void ClientSetHUDReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsethud>& Op);
	void ClientSetForceMipLevelsToBeResidentReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetforcemiplevelstoberesident>& Op);
	void ClientSetCinematicModeReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetcinematicmode>& Op);
	void ClientSetCameraModeReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetcameramode>& Op);
	void ClientSetCameraFadeReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetcamerafade>& Op);
	void ClientSetBlockOnAsyncLoadingReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetblockonasyncloading>& Op);
	void ClientReturnToMainMenuReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientreturntomainmenu>& Op);
	void ClientRetryClientRestartReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientretryclientrestart>& Op);
	void ClientRestartReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientrestart>& Op);
	void ClientResetReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientreset>& Op);
	void ClientRepObjRefReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientrepobjref>& Op);
	void ClientReceiveLocalizedMessageReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientreceivelocalizedmessage>& Op);
	void ClientPrestreamTexturesReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientprestreamtextures>& Op);
	void ClientPrepareMapChangeReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientpreparemapchange>& Op);
	void ClientPlaySoundAtLocationReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaysoundatlocation>& Op);
	void ClientPlaySoundReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaysound>& Op);
	void ClientPlayForceFeedbackReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplayforcefeedback>& Op);
	void ClientPlayCameraShakeReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaycamerashake>& Op);
	void ClientPlayCameraAnimReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaycameraanim>& Op);
	void ClientMutePlayerReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientmuteplayer>& Op);
	void ClientMessageReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientmessage>& Op);
	void ClientIgnoreMoveInputReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientignoremoveinput>& Op);
	void ClientIgnoreLookInputReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientignorelookinput>& Op);
	void ClientGotoStateReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientgotostate>& Op);
	void ClientGameEndedReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientgameended>& Op);
	void ClientForceGarbageCollectionReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientforcegarbagecollection>& Op);
	void ClientFlushLevelStreamingReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientflushlevelstreaming>& Op);
	void ClientEndOnlineSessionReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientendonlinesession>& Op);
	void ClientEnableNetworkVoiceReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientenablenetworkvoice>& Op);
	void ClientCommitMapChangeReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientcommitmapchange>& Op);
	void ClientClearCameraLensEffectsReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientclearcameralenseffects>& Op);
	void ClientCapBandwidthReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientcapbandwidth>& Op);
	void ClientCancelPendingMapChangeReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientcancelpendingmapchange>& Op);
	void ClientAddTextureStreamingLocReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientaddtexturestreamingloc>& Op);
	void ClientSetRotationReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetrotation>& Op);
	void ClientSetLocationReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetlocation>& Op);
	void ServerViewSelfReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverviewself>& Op);
	void ServerViewPrevPlayerReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverviewprevplayer>& Op);
	void ServerViewNextPlayerReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverviewnextplayer>& Op);
	void ServerVerifyViewTargetReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serververifyviewtarget>& Op);
	void ServerUpdateLevelVisibilityReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverupdatelevelvisibility>& Op);
	void ServerUpdateCameraReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverupdatecamera>& Op);
	void ServerUnmutePlayerReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverunmuteplayer>& Op);
	void ServerToggleAILoggingReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servertoggleailogging>& Op);
	void ServerShortTimeoutReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servershorttimeout>& Op);
	void ServerSetSpectatorWaitingReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serversetspectatorwaiting>& Op);
	void ServerSetSpectatorLocationReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serversetspectatorlocation>& Op);
	void ServerRestartPlayerReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverrestartplayer>& Op);
	void ServerPauseReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverpause>& Op);
	void ServerNotifyLoadedWorldReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servernotifyloadedworld>& Op);
	void ServerMutePlayerReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servermuteplayer>& Op);
	void ServerCheckClientPossessionReliableReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servercheckclientpossessionreliable>& Op);
	void ServerCheckClientPossessionReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servercheckclientpossession>& Op);
	void ServerChangeNameReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverchangename>& Op);
	void ServerCameraReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servercamera>& Op);
	void ServerAcknowledgePossessionReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serveracknowledgepossession>& Op);
};
