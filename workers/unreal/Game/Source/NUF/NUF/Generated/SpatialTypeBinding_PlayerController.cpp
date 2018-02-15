// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// Note that this file has been generated automatically

#include "SpatialTypeBinding_PlayerController.h"
#include "Engine.h"
#include "SpatialOS.h"
#include "EntityBuilder.h"

#include "../SpatialActorChannel.h"
#include "../SpatialPackageMapClient.h"
#include "../SpatialNetDriver.h"
#include "../SpatialConstants.h"
#include "../SpatialInterop.h"

const FRepHandlePropertyMap& USpatialTypeBinding_PlayerController::GetHandlePropertyMap()
{
	static FRepHandlePropertyMap HandleToPropertyMap;
	if (HandleToPropertyMap.Num() == 0)
	{
		UClass* Class = FindObject<UClass>(ANY_PACKAGE, TEXT("PlayerController"));
		HandleToPropertyMap.Add(18, FRepHandleData{nullptr, Class->FindPropertyByName("TargetViewRotation"), COND_OwnerOnly});
		HandleToPropertyMap.Add(19, FRepHandleData{nullptr, Class->FindPropertyByName("SpawnLocation"), COND_OwnerOnly});
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
		HandleToPropertyMap.Add(16, FRepHandleData{nullptr, Class->FindPropertyByName("Pawn"), COND_None});
		HandleToPropertyMap.Add(17, FRepHandleData{nullptr, Class->FindPropertyByName("PlayerState"), COND_None});
	}
	return HandleToPropertyMap;
}

void USpatialTypeBinding_PlayerController::Init(USpatialInterop* InInterop, USpatialPackageMapClient* InPackageMap)
{
	Super::Init(InInterop, InPackageMap);

	RPCToSenderMap.Emplace("OnServerStartedVisualLogger", &USpatialTypeBinding_PlayerController::OnServerStartedVisualLogger_Sender);
	RPCToSenderMap.Emplace("ClientWasKicked", &USpatialTypeBinding_PlayerController::ClientWasKicked_Sender);
	RPCToSenderMap.Emplace("ClientVoiceHandshakeComplete", &USpatialTypeBinding_PlayerController::ClientVoiceHandshakeComplete_Sender);
	RPCToSenderMap.Emplace("ClientUpdateLevelStreamingStatus", &USpatialTypeBinding_PlayerController::ClientUpdateLevelStreamingStatus_Sender);
	RPCToSenderMap.Emplace("ClientUnmutePlayer", &USpatialTypeBinding_PlayerController::ClientUnmutePlayer_Sender);
	RPCToSenderMap.Emplace("ClientTravelInternal", &USpatialTypeBinding_PlayerController::ClientTravelInternal_Sender);
	RPCToSenderMap.Emplace("ClientTeamMessage", &USpatialTypeBinding_PlayerController::ClientTeamMessage_Sender);
	RPCToSenderMap.Emplace("ClientStopForceFeedback", &USpatialTypeBinding_PlayerController::ClientStopForceFeedback_Sender);
	RPCToSenderMap.Emplace("ClientStopCameraShake", &USpatialTypeBinding_PlayerController::ClientStopCameraShake_Sender);
	RPCToSenderMap.Emplace("ClientStopCameraAnim", &USpatialTypeBinding_PlayerController::ClientStopCameraAnim_Sender);
	RPCToSenderMap.Emplace("ClientStartOnlineSession", &USpatialTypeBinding_PlayerController::ClientStartOnlineSession_Sender);
	RPCToSenderMap.Emplace("ClientSpawnCameraLensEffect", &USpatialTypeBinding_PlayerController::ClientSpawnCameraLensEffect_Sender);
	RPCToSenderMap.Emplace("ClientSetViewTarget", &USpatialTypeBinding_PlayerController::ClientSetViewTarget_Sender);
	RPCToSenderMap.Emplace("ClientSetSpectatorWaiting", &USpatialTypeBinding_PlayerController::ClientSetSpectatorWaiting_Sender);
	RPCToSenderMap.Emplace("ClientSetHUD", &USpatialTypeBinding_PlayerController::ClientSetHUD_Sender);
	RPCToSenderMap.Emplace("ClientSetForceMipLevelsToBeResident", &USpatialTypeBinding_PlayerController::ClientSetForceMipLevelsToBeResident_Sender);
	RPCToSenderMap.Emplace("ClientSetCinematicMode", &USpatialTypeBinding_PlayerController::ClientSetCinematicMode_Sender);
	RPCToSenderMap.Emplace("ClientSetCameraMode", &USpatialTypeBinding_PlayerController::ClientSetCameraMode_Sender);
	RPCToSenderMap.Emplace("ClientSetCameraFade", &USpatialTypeBinding_PlayerController::ClientSetCameraFade_Sender);
	RPCToSenderMap.Emplace("ClientSetBlockOnAsyncLoading", &USpatialTypeBinding_PlayerController::ClientSetBlockOnAsyncLoading_Sender);
	RPCToSenderMap.Emplace("ClientReturnToMainMenu", &USpatialTypeBinding_PlayerController::ClientReturnToMainMenu_Sender);
	RPCToSenderMap.Emplace("ClientRetryClientRestart", &USpatialTypeBinding_PlayerController::ClientRetryClientRestart_Sender);
	RPCToSenderMap.Emplace("ClientRestart", &USpatialTypeBinding_PlayerController::ClientRestart_Sender);
	RPCToSenderMap.Emplace("ClientReset", &USpatialTypeBinding_PlayerController::ClientReset_Sender);
	RPCToSenderMap.Emplace("ClientRepObjRef", &USpatialTypeBinding_PlayerController::ClientRepObjRef_Sender);
	RPCToSenderMap.Emplace("ClientReceiveLocalizedMessage", &USpatialTypeBinding_PlayerController::ClientReceiveLocalizedMessage_Sender);
	RPCToSenderMap.Emplace("ClientPrestreamTextures", &USpatialTypeBinding_PlayerController::ClientPrestreamTextures_Sender);
	RPCToSenderMap.Emplace("ClientPrepareMapChange", &USpatialTypeBinding_PlayerController::ClientPrepareMapChange_Sender);
	RPCToSenderMap.Emplace("ClientPlaySoundAtLocation", &USpatialTypeBinding_PlayerController::ClientPlaySoundAtLocation_Sender);
	RPCToSenderMap.Emplace("ClientPlaySound", &USpatialTypeBinding_PlayerController::ClientPlaySound_Sender);
	RPCToSenderMap.Emplace("ClientPlayForceFeedback", &USpatialTypeBinding_PlayerController::ClientPlayForceFeedback_Sender);
	RPCToSenderMap.Emplace("ClientPlayCameraShake", &USpatialTypeBinding_PlayerController::ClientPlayCameraShake_Sender);
	RPCToSenderMap.Emplace("ClientPlayCameraAnim", &USpatialTypeBinding_PlayerController::ClientPlayCameraAnim_Sender);
	RPCToSenderMap.Emplace("ClientMutePlayer", &USpatialTypeBinding_PlayerController::ClientMutePlayer_Sender);
	RPCToSenderMap.Emplace("ClientMessage", &USpatialTypeBinding_PlayerController::ClientMessage_Sender);
	RPCToSenderMap.Emplace("ClientIgnoreMoveInput", &USpatialTypeBinding_PlayerController::ClientIgnoreMoveInput_Sender);
	RPCToSenderMap.Emplace("ClientIgnoreLookInput", &USpatialTypeBinding_PlayerController::ClientIgnoreLookInput_Sender);
	RPCToSenderMap.Emplace("ClientGotoState", &USpatialTypeBinding_PlayerController::ClientGotoState_Sender);
	RPCToSenderMap.Emplace("ClientGameEnded", &USpatialTypeBinding_PlayerController::ClientGameEnded_Sender);
	RPCToSenderMap.Emplace("ClientForceGarbageCollection", &USpatialTypeBinding_PlayerController::ClientForceGarbageCollection_Sender);
	RPCToSenderMap.Emplace("ClientFlushLevelStreaming", &USpatialTypeBinding_PlayerController::ClientFlushLevelStreaming_Sender);
	RPCToSenderMap.Emplace("ClientEndOnlineSession", &USpatialTypeBinding_PlayerController::ClientEndOnlineSession_Sender);
	RPCToSenderMap.Emplace("ClientEnableNetworkVoice", &USpatialTypeBinding_PlayerController::ClientEnableNetworkVoice_Sender);
	RPCToSenderMap.Emplace("ClientCommitMapChange", &USpatialTypeBinding_PlayerController::ClientCommitMapChange_Sender);
	RPCToSenderMap.Emplace("ClientClearCameraLensEffects", &USpatialTypeBinding_PlayerController::ClientClearCameraLensEffects_Sender);
	RPCToSenderMap.Emplace("ClientCapBandwidth", &USpatialTypeBinding_PlayerController::ClientCapBandwidth_Sender);
	RPCToSenderMap.Emplace("ClientCancelPendingMapChange", &USpatialTypeBinding_PlayerController::ClientCancelPendingMapChange_Sender);
	RPCToSenderMap.Emplace("ClientAddTextureStreamingLoc", &USpatialTypeBinding_PlayerController::ClientAddTextureStreamingLoc_Sender);
	RPCToSenderMap.Emplace("ClientSetRotation", &USpatialTypeBinding_PlayerController::ClientSetRotation_Sender);
	RPCToSenderMap.Emplace("ClientSetLocation", &USpatialTypeBinding_PlayerController::ClientSetLocation_Sender);
	RPCToSenderMap.Emplace("ServerViewSelf", &USpatialTypeBinding_PlayerController::ServerViewSelf_Sender);
	RPCToSenderMap.Emplace("ServerViewPrevPlayer", &USpatialTypeBinding_PlayerController::ServerViewPrevPlayer_Sender);
	RPCToSenderMap.Emplace("ServerViewNextPlayer", &USpatialTypeBinding_PlayerController::ServerViewNextPlayer_Sender);
	RPCToSenderMap.Emplace("ServerVerifyViewTarget", &USpatialTypeBinding_PlayerController::ServerVerifyViewTarget_Sender);
	RPCToSenderMap.Emplace("ServerUpdateLevelVisibility", &USpatialTypeBinding_PlayerController::ServerUpdateLevelVisibility_Sender);
	RPCToSenderMap.Emplace("ServerUpdateCamera", &USpatialTypeBinding_PlayerController::ServerUpdateCamera_Sender);
	RPCToSenderMap.Emplace("ServerUnmutePlayer", &USpatialTypeBinding_PlayerController::ServerUnmutePlayer_Sender);
	RPCToSenderMap.Emplace("ServerToggleAILogging", &USpatialTypeBinding_PlayerController::ServerToggleAILogging_Sender);
	RPCToSenderMap.Emplace("ServerShortTimeout", &USpatialTypeBinding_PlayerController::ServerShortTimeout_Sender);
	RPCToSenderMap.Emplace("ServerSetSpectatorWaiting", &USpatialTypeBinding_PlayerController::ServerSetSpectatorWaiting_Sender);
	RPCToSenderMap.Emplace("ServerSetSpectatorLocation", &USpatialTypeBinding_PlayerController::ServerSetSpectatorLocation_Sender);
	RPCToSenderMap.Emplace("ServerRestartPlayer", &USpatialTypeBinding_PlayerController::ServerRestartPlayer_Sender);
	RPCToSenderMap.Emplace("ServerPause", &USpatialTypeBinding_PlayerController::ServerPause_Sender);
	RPCToSenderMap.Emplace("ServerNotifyLoadedWorld", &USpatialTypeBinding_PlayerController::ServerNotifyLoadedWorld_Sender);
	RPCToSenderMap.Emplace("ServerMutePlayer", &USpatialTypeBinding_PlayerController::ServerMutePlayer_Sender);
	RPCToSenderMap.Emplace("ServerCheckClientPossessionReliable", &USpatialTypeBinding_PlayerController::ServerCheckClientPossessionReliable_Sender);
	RPCToSenderMap.Emplace("ServerCheckClientPossession", &USpatialTypeBinding_PlayerController::ServerCheckClientPossession_Sender);
	RPCToSenderMap.Emplace("ServerChangeName", &USpatialTypeBinding_PlayerController::ServerChangeName_Sender);
	RPCToSenderMap.Emplace("ServerCamera", &USpatialTypeBinding_PlayerController::ServerCamera_Sender);
	RPCToSenderMap.Emplace("ServerAcknowledgePossession", &USpatialTypeBinding_PlayerController::ServerAcknowledgePossession_Sender);
}

void USpatialTypeBinding_PlayerController::BindToView()
{
	TSharedPtr<worker::View> View = Interop->GetSpatialOS()->GetView().Pin();
	SingleClientAddCallback = View->OnAddComponent<improbable::unreal::UnrealPlayerControllerSingleClientReplicatedData>([this](
		const worker::AddComponentOp<improbable::unreal::UnrealPlayerControllerSingleClientReplicatedData>& Op)
	{
		auto Update = improbable::unreal::UnrealPlayerControllerSingleClientReplicatedData::Update::FromInitialData(Op.Data);
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
	SingleClientUpdateCallback = View->OnComponentUpdate<improbable::unreal::UnrealPlayerControllerSingleClientReplicatedData>([this](
		const worker::ComponentUpdateOp<improbable::unreal::UnrealPlayerControllerSingleClientReplicatedData>& Op)
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
	MultiClientAddCallback = View->OnAddComponent<improbable::unreal::UnrealPlayerControllerMultiClientReplicatedData>([this](
		const worker::AddComponentOp<improbable::unreal::UnrealPlayerControllerMultiClientReplicatedData>& Op)
	{
		auto Update = improbable::unreal::UnrealPlayerControllerMultiClientReplicatedData::Update::FromInitialData(Op.Data);
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
	MultiClientUpdateCallback = View->OnComponentUpdate<improbable::unreal::UnrealPlayerControllerMultiClientReplicatedData>([this](
		const worker::ComponentUpdateOp<improbable::unreal::UnrealPlayerControllerMultiClientReplicatedData>& Op)
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
	using ClientRPCCommandTypes = improbable::unreal::UnrealPlayerControllerClientRPCs::Commands;
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Onserverstartedvisuallogger>(std::bind(&USpatialTypeBinding_PlayerController::OnServerStartedVisualLogger_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientwaskicked>(std::bind(&USpatialTypeBinding_PlayerController::ClientWasKicked_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientvoicehandshakecomplete>(std::bind(&USpatialTypeBinding_PlayerController::ClientVoiceHandshakeComplete_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientupdatelevelstreamingstatus>(std::bind(&USpatialTypeBinding_PlayerController::ClientUpdateLevelStreamingStatus_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientunmuteplayer>(std::bind(&USpatialTypeBinding_PlayerController::ClientUnmutePlayer_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clienttravelinternal>(std::bind(&USpatialTypeBinding_PlayerController::ClientTravelInternal_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientteammessage>(std::bind(&USpatialTypeBinding_PlayerController::ClientTeamMessage_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientstopforcefeedback>(std::bind(&USpatialTypeBinding_PlayerController::ClientStopForceFeedback_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientstopcamerashake>(std::bind(&USpatialTypeBinding_PlayerController::ClientStopCameraShake_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientstopcameraanim>(std::bind(&USpatialTypeBinding_PlayerController::ClientStopCameraAnim_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientstartonlinesession>(std::bind(&USpatialTypeBinding_PlayerController::ClientStartOnlineSession_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientspawncameralenseffect>(std::bind(&USpatialTypeBinding_PlayerController::ClientSpawnCameraLensEffect_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientsetviewtarget>(std::bind(&USpatialTypeBinding_PlayerController::ClientSetViewTarget_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientsetspectatorwaiting>(std::bind(&USpatialTypeBinding_PlayerController::ClientSetSpectatorWaiting_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientsethud>(std::bind(&USpatialTypeBinding_PlayerController::ClientSetHUD_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientsetforcemiplevelstoberesident>(std::bind(&USpatialTypeBinding_PlayerController::ClientSetForceMipLevelsToBeResident_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientsetcinematicmode>(std::bind(&USpatialTypeBinding_PlayerController::ClientSetCinematicMode_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientsetcameramode>(std::bind(&USpatialTypeBinding_PlayerController::ClientSetCameraMode_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientsetcamerafade>(std::bind(&USpatialTypeBinding_PlayerController::ClientSetCameraFade_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientsetblockonasyncloading>(std::bind(&USpatialTypeBinding_PlayerController::ClientSetBlockOnAsyncLoading_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientreturntomainmenu>(std::bind(&USpatialTypeBinding_PlayerController::ClientReturnToMainMenu_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientretryclientrestart>(std::bind(&USpatialTypeBinding_PlayerController::ClientRetryClientRestart_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientrestart>(std::bind(&USpatialTypeBinding_PlayerController::ClientRestart_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientreset>(std::bind(&USpatialTypeBinding_PlayerController::ClientReset_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientrepobjref>(std::bind(&USpatialTypeBinding_PlayerController::ClientRepObjRef_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientreceivelocalizedmessage>(std::bind(&USpatialTypeBinding_PlayerController::ClientReceiveLocalizedMessage_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientprestreamtextures>(std::bind(&USpatialTypeBinding_PlayerController::ClientPrestreamTextures_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientpreparemapchange>(std::bind(&USpatialTypeBinding_PlayerController::ClientPrepareMapChange_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientplaysoundatlocation>(std::bind(&USpatialTypeBinding_PlayerController::ClientPlaySoundAtLocation_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientplaysound>(std::bind(&USpatialTypeBinding_PlayerController::ClientPlaySound_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientplayforcefeedback>(std::bind(&USpatialTypeBinding_PlayerController::ClientPlayForceFeedback_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientplaycamerashake>(std::bind(&USpatialTypeBinding_PlayerController::ClientPlayCameraShake_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientplaycameraanim>(std::bind(&USpatialTypeBinding_PlayerController::ClientPlayCameraAnim_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientmuteplayer>(std::bind(&USpatialTypeBinding_PlayerController::ClientMutePlayer_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientmessage>(std::bind(&USpatialTypeBinding_PlayerController::ClientMessage_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientignoremoveinput>(std::bind(&USpatialTypeBinding_PlayerController::ClientIgnoreMoveInput_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientignorelookinput>(std::bind(&USpatialTypeBinding_PlayerController::ClientIgnoreLookInput_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientgotostate>(std::bind(&USpatialTypeBinding_PlayerController::ClientGotoState_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientgameended>(std::bind(&USpatialTypeBinding_PlayerController::ClientGameEnded_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientforcegarbagecollection>(std::bind(&USpatialTypeBinding_PlayerController::ClientForceGarbageCollection_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientflushlevelstreaming>(std::bind(&USpatialTypeBinding_PlayerController::ClientFlushLevelStreaming_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientendonlinesession>(std::bind(&USpatialTypeBinding_PlayerController::ClientEndOnlineSession_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientenablenetworkvoice>(std::bind(&USpatialTypeBinding_PlayerController::ClientEnableNetworkVoice_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientcommitmapchange>(std::bind(&USpatialTypeBinding_PlayerController::ClientCommitMapChange_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientclearcameralenseffects>(std::bind(&USpatialTypeBinding_PlayerController::ClientClearCameraLensEffects_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientcapbandwidth>(std::bind(&USpatialTypeBinding_PlayerController::ClientCapBandwidth_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientcancelpendingmapchange>(std::bind(&USpatialTypeBinding_PlayerController::ClientCancelPendingMapChange_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientaddtexturestreamingloc>(std::bind(&USpatialTypeBinding_PlayerController::ClientAddTextureStreamingLoc_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientsetrotation>(std::bind(&USpatialTypeBinding_PlayerController::ClientSetRotation_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientsetlocation>(std::bind(&USpatialTypeBinding_PlayerController::ClientSetLocation_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Onserverstartedvisuallogger>(std::bind(&USpatialTypeBinding_PlayerController::OnServerStartedVisualLogger_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientwaskicked>(std::bind(&USpatialTypeBinding_PlayerController::ClientWasKicked_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientvoicehandshakecomplete>(std::bind(&USpatialTypeBinding_PlayerController::ClientVoiceHandshakeComplete_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientupdatelevelstreamingstatus>(std::bind(&USpatialTypeBinding_PlayerController::ClientUpdateLevelStreamingStatus_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientunmuteplayer>(std::bind(&USpatialTypeBinding_PlayerController::ClientUnmutePlayer_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clienttravelinternal>(std::bind(&USpatialTypeBinding_PlayerController::ClientTravelInternal_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientteammessage>(std::bind(&USpatialTypeBinding_PlayerController::ClientTeamMessage_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientstopforcefeedback>(std::bind(&USpatialTypeBinding_PlayerController::ClientStopForceFeedback_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientstopcamerashake>(std::bind(&USpatialTypeBinding_PlayerController::ClientStopCameraShake_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientstopcameraanim>(std::bind(&USpatialTypeBinding_PlayerController::ClientStopCameraAnim_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientstartonlinesession>(std::bind(&USpatialTypeBinding_PlayerController::ClientStartOnlineSession_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientspawncameralenseffect>(std::bind(&USpatialTypeBinding_PlayerController::ClientSpawnCameraLensEffect_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientsetviewtarget>(std::bind(&USpatialTypeBinding_PlayerController::ClientSetViewTarget_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientsetspectatorwaiting>(std::bind(&USpatialTypeBinding_PlayerController::ClientSetSpectatorWaiting_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientsethud>(std::bind(&USpatialTypeBinding_PlayerController::ClientSetHUD_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientsetforcemiplevelstoberesident>(std::bind(&USpatialTypeBinding_PlayerController::ClientSetForceMipLevelsToBeResident_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientsetcinematicmode>(std::bind(&USpatialTypeBinding_PlayerController::ClientSetCinematicMode_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientsetcameramode>(std::bind(&USpatialTypeBinding_PlayerController::ClientSetCameraMode_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientsetcamerafade>(std::bind(&USpatialTypeBinding_PlayerController::ClientSetCameraFade_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientsetblockonasyncloading>(std::bind(&USpatialTypeBinding_PlayerController::ClientSetBlockOnAsyncLoading_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientreturntomainmenu>(std::bind(&USpatialTypeBinding_PlayerController::ClientReturnToMainMenu_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientretryclientrestart>(std::bind(&USpatialTypeBinding_PlayerController::ClientRetryClientRestart_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientrestart>(std::bind(&USpatialTypeBinding_PlayerController::ClientRestart_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientreset>(std::bind(&USpatialTypeBinding_PlayerController::ClientReset_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientrepobjref>(std::bind(&USpatialTypeBinding_PlayerController::ClientRepObjRef_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientreceivelocalizedmessage>(std::bind(&USpatialTypeBinding_PlayerController::ClientReceiveLocalizedMessage_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientprestreamtextures>(std::bind(&USpatialTypeBinding_PlayerController::ClientPrestreamTextures_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientpreparemapchange>(std::bind(&USpatialTypeBinding_PlayerController::ClientPrepareMapChange_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientplaysoundatlocation>(std::bind(&USpatialTypeBinding_PlayerController::ClientPlaySoundAtLocation_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientplaysound>(std::bind(&USpatialTypeBinding_PlayerController::ClientPlaySound_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientplayforcefeedback>(std::bind(&USpatialTypeBinding_PlayerController::ClientPlayForceFeedback_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientplaycamerashake>(std::bind(&USpatialTypeBinding_PlayerController::ClientPlayCameraShake_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientplaycameraanim>(std::bind(&USpatialTypeBinding_PlayerController::ClientPlayCameraAnim_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientmuteplayer>(std::bind(&USpatialTypeBinding_PlayerController::ClientMutePlayer_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientmessage>(std::bind(&USpatialTypeBinding_PlayerController::ClientMessage_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientignoremoveinput>(std::bind(&USpatialTypeBinding_PlayerController::ClientIgnoreMoveInput_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientignorelookinput>(std::bind(&USpatialTypeBinding_PlayerController::ClientIgnoreLookInput_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientgotostate>(std::bind(&USpatialTypeBinding_PlayerController::ClientGotoState_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientgameended>(std::bind(&USpatialTypeBinding_PlayerController::ClientGameEnded_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientforcegarbagecollection>(std::bind(&USpatialTypeBinding_PlayerController::ClientForceGarbageCollection_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientflushlevelstreaming>(std::bind(&USpatialTypeBinding_PlayerController::ClientFlushLevelStreaming_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientendonlinesession>(std::bind(&USpatialTypeBinding_PlayerController::ClientEndOnlineSession_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientenablenetworkvoice>(std::bind(&USpatialTypeBinding_PlayerController::ClientEnableNetworkVoice_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientcommitmapchange>(std::bind(&USpatialTypeBinding_PlayerController::ClientCommitMapChange_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientclearcameralenseffects>(std::bind(&USpatialTypeBinding_PlayerController::ClientClearCameraLensEffects_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientcapbandwidth>(std::bind(&USpatialTypeBinding_PlayerController::ClientCapBandwidth_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientcancelpendingmapchange>(std::bind(&USpatialTypeBinding_PlayerController::ClientCancelPendingMapChange_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientaddtexturestreamingloc>(std::bind(&USpatialTypeBinding_PlayerController::ClientAddTextureStreamingLoc_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientsetrotation>(std::bind(&USpatialTypeBinding_PlayerController::ClientSetRotation_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ClientRPCCommandTypes::Clientsetlocation>(std::bind(&USpatialTypeBinding_PlayerController::ClientSetLocation_Sender_Response, this, std::placeholders::_1)));
	using ServerRPCCommandTypes = improbable::unreal::UnrealPlayerControllerServerRPCs::Commands;
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Serverviewself>(std::bind(&USpatialTypeBinding_PlayerController::ServerViewSelf_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Serverviewprevplayer>(std::bind(&USpatialTypeBinding_PlayerController::ServerViewPrevPlayer_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Serverviewnextplayer>(std::bind(&USpatialTypeBinding_PlayerController::ServerViewNextPlayer_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Serververifyviewtarget>(std::bind(&USpatialTypeBinding_PlayerController::ServerVerifyViewTarget_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Serverupdatelevelvisibility>(std::bind(&USpatialTypeBinding_PlayerController::ServerUpdateLevelVisibility_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Serverupdatecamera>(std::bind(&USpatialTypeBinding_PlayerController::ServerUpdateCamera_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Serverunmuteplayer>(std::bind(&USpatialTypeBinding_PlayerController::ServerUnmutePlayer_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Servertoggleailogging>(std::bind(&USpatialTypeBinding_PlayerController::ServerToggleAILogging_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Servershorttimeout>(std::bind(&USpatialTypeBinding_PlayerController::ServerShortTimeout_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Serversetspectatorwaiting>(std::bind(&USpatialTypeBinding_PlayerController::ServerSetSpectatorWaiting_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Serversetspectatorlocation>(std::bind(&USpatialTypeBinding_PlayerController::ServerSetSpectatorLocation_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Serverrestartplayer>(std::bind(&USpatialTypeBinding_PlayerController::ServerRestartPlayer_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Serverpause>(std::bind(&USpatialTypeBinding_PlayerController::ServerPause_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Servernotifyloadedworld>(std::bind(&USpatialTypeBinding_PlayerController::ServerNotifyLoadedWorld_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Servermuteplayer>(std::bind(&USpatialTypeBinding_PlayerController::ServerMutePlayer_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Servercheckclientpossessionreliable>(std::bind(&USpatialTypeBinding_PlayerController::ServerCheckClientPossessionReliable_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Servercheckclientpossession>(std::bind(&USpatialTypeBinding_PlayerController::ServerCheckClientPossession_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Serverchangename>(std::bind(&USpatialTypeBinding_PlayerController::ServerChangeName_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Servercamera>(std::bind(&USpatialTypeBinding_PlayerController::ServerCamera_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Serveracknowledgepossession>(std::bind(&USpatialTypeBinding_PlayerController::ServerAcknowledgePossession_Receiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ServerRPCCommandTypes::Serverviewself>(std::bind(&USpatialTypeBinding_PlayerController::ServerViewSelf_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ServerRPCCommandTypes::Serverviewprevplayer>(std::bind(&USpatialTypeBinding_PlayerController::ServerViewPrevPlayer_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ServerRPCCommandTypes::Serverviewnextplayer>(std::bind(&USpatialTypeBinding_PlayerController::ServerViewNextPlayer_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ServerRPCCommandTypes::Serververifyviewtarget>(std::bind(&USpatialTypeBinding_PlayerController::ServerVerifyViewTarget_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ServerRPCCommandTypes::Serverupdatelevelvisibility>(std::bind(&USpatialTypeBinding_PlayerController::ServerUpdateLevelVisibility_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ServerRPCCommandTypes::Serverupdatecamera>(std::bind(&USpatialTypeBinding_PlayerController::ServerUpdateCamera_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ServerRPCCommandTypes::Serverunmuteplayer>(std::bind(&USpatialTypeBinding_PlayerController::ServerUnmutePlayer_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ServerRPCCommandTypes::Servertoggleailogging>(std::bind(&USpatialTypeBinding_PlayerController::ServerToggleAILogging_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ServerRPCCommandTypes::Servershorttimeout>(std::bind(&USpatialTypeBinding_PlayerController::ServerShortTimeout_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ServerRPCCommandTypes::Serversetspectatorwaiting>(std::bind(&USpatialTypeBinding_PlayerController::ServerSetSpectatorWaiting_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ServerRPCCommandTypes::Serversetspectatorlocation>(std::bind(&USpatialTypeBinding_PlayerController::ServerSetSpectatorLocation_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ServerRPCCommandTypes::Serverrestartplayer>(std::bind(&USpatialTypeBinding_PlayerController::ServerRestartPlayer_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ServerRPCCommandTypes::Serverpause>(std::bind(&USpatialTypeBinding_PlayerController::ServerPause_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ServerRPCCommandTypes::Servernotifyloadedworld>(std::bind(&USpatialTypeBinding_PlayerController::ServerNotifyLoadedWorld_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ServerRPCCommandTypes::Servermuteplayer>(std::bind(&USpatialTypeBinding_PlayerController::ServerMutePlayer_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ServerRPCCommandTypes::Servercheckclientpossessionreliable>(std::bind(&USpatialTypeBinding_PlayerController::ServerCheckClientPossessionReliable_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ServerRPCCommandTypes::Servercheckclientpossession>(std::bind(&USpatialTypeBinding_PlayerController::ServerCheckClientPossession_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ServerRPCCommandTypes::Serverchangename>(std::bind(&USpatialTypeBinding_PlayerController::ServerChangeName_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ServerRPCCommandTypes::Servercamera>(std::bind(&USpatialTypeBinding_PlayerController::ServerCamera_Sender_Response, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandResponse<ServerRPCCommandTypes::Serveracknowledgepossession>(std::bind(&USpatialTypeBinding_PlayerController::ServerAcknowledgePossession_Sender_Response, this, std::placeholders::_1)));
}

void USpatialTypeBinding_PlayerController::UnbindFromView()
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

worker::ComponentId USpatialTypeBinding_PlayerController::GetReplicatedGroupComponentId(EReplicatedPropertyGroup Group) const
{
	switch (Group)
	{
	case GROUP_SingleClient:
		return improbable::unreal::UnrealPlayerControllerSingleClientReplicatedData::ComponentId;
	case GROUP_MultiClient:
		return improbable::unreal::UnrealPlayerControllerMultiClientReplicatedData::ComponentId;
	default:
		checkNoEntry();
		return 0;
	}
}

worker::Entity USpatialTypeBinding_PlayerController::CreateActorEntity(const FString& ClientWorkerId, const FVector& Position, const FString& Metadata, const FPropertyChangeState& InitialChanges, USpatialActorChannel* Channel) const
{
	// Setup initial data.
	improbable::unreal::UnrealPlayerControllerSingleClientReplicatedData::Data SingleClientData;
	improbable::unreal::UnrealPlayerControllerSingleClientReplicatedData::Update SingleClientUpdate;
	bool bSingleClientUpdateChanged = false;
	improbable::unreal::UnrealPlayerControllerMultiClientReplicatedData::Data MultiClientData;
	improbable::unreal::UnrealPlayerControllerMultiClientReplicatedData::Update MultiClientUpdate;
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

	const improbable::Coordinates SpatialPosition = USpatialOSConversionFunctionLibrary::UnrealCoordinatesToSpatialOsCoordinatesCast(Position);
	return improbable::unreal::FEntityBuilder::Begin()
		.AddPositionComponent(improbable::Position::Data{SpatialPosition}, WorkersOnly)
		.AddMetadataComponent(improbable::Metadata::Data{TCHAR_TO_UTF8(*Metadata)})
		.SetPersistence(true)
		.SetReadAcl(AnyUnrealWorkerOrOwningClient)
		.AddComponent<improbable::unreal::UnrealPlayerControllerSingleClientReplicatedData>(SingleClientData, WorkersOnly)
		.AddComponent<improbable::unreal::UnrealPlayerControllerMultiClientReplicatedData>(MultiClientData, WorkersOnly)
		.AddComponent<improbable::unreal::UnrealPlayerControllerCompleteData>(improbable::unreal::UnrealPlayerControllerCompleteData::Data{}, WorkersOnly)
		.AddComponent<improbable::unreal::UnrealPlayerControllerClientRPCs>(improbable::unreal::UnrealPlayerControllerClientRPCs::Data{}, OwningClientOnly)
		.AddComponent<improbable::unreal::UnrealPlayerControllerServerRPCs>(improbable::unreal::UnrealPlayerControllerServerRPCs::Data{}, WorkersOnly)
		.Build();
}

void USpatialTypeBinding_PlayerController::SendComponentUpdates(const FPropertyChangeState& Changes, USpatialActorChannel* Channel, const worker::EntityId& EntityId) const
{
	// Build SpatialOS updates.
	improbable::unreal::UnrealPlayerControllerSingleClientReplicatedData::Update SingleClientUpdate;
	bool bSingleClientUpdateChanged = false;
	improbable::unreal::UnrealPlayerControllerMultiClientReplicatedData::Update MultiClientUpdate;
	bool bMultiClientUpdateChanged = false;
	BuildSpatialComponentUpdate(Changes, Channel, SingleClientUpdate, bSingleClientUpdateChanged, MultiClientUpdate, bMultiClientUpdateChanged);

	// Send SpatialOS updates if anything changed.
	TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
	if (bSingleClientUpdateChanged)
	{
		Connection->SendComponentUpdate<improbable::unreal::UnrealPlayerControllerSingleClientReplicatedData>(EntityId, SingleClientUpdate);
	}
	if (bMultiClientUpdateChanged)
	{
		Connection->SendComponentUpdate<improbable::unreal::UnrealPlayerControllerMultiClientReplicatedData>(EntityId, MultiClientUpdate);
	}
}

void USpatialTypeBinding_PlayerController::SendRPCCommand(UObject* TargetObject, const UFunction* const Function, FFrame* const Frame)
{
	TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
	auto SenderFuncIterator = RPCToSenderMap.Find(Function->GetFName());
	checkf(*SenderFuncIterator, TEXT("Sender for %s has not been registered with RPCToSenderMap."), *Function->GetFName().ToString());
	(this->*(*SenderFuncIterator))(Connection.Get(), Frame, TargetObject);
}

void USpatialTypeBinding_PlayerController::ApplyQueuedStateToChannel(USpatialActorChannel* ActorChannel)
{
	improbable::unreal::UnrealPlayerControllerSingleClientReplicatedData::Data* SingleClientData = PendingSingleClientData.Find(ActorChannel->GetEntityId());
	if (SingleClientData)
	{
		auto Update = improbable::unreal::UnrealPlayerControllerSingleClientReplicatedData::Update::FromInitialData(*SingleClientData);
		PendingSingleClientData.Remove(ActorChannel->GetEntityId());
		ClientReceiveUpdate_SingleClient(ActorChannel, Update);
	}
	improbable::unreal::UnrealPlayerControllerMultiClientReplicatedData::Data* MultiClientData = PendingMultiClientData.Find(ActorChannel->GetEntityId());
	if (MultiClientData)
	{
		auto Update = improbable::unreal::UnrealPlayerControllerMultiClientReplicatedData::Update::FromInitialData(*MultiClientData);
		PendingMultiClientData.Remove(ActorChannel->GetEntityId());
		ClientReceiveUpdate_MultiClient(ActorChannel, Update);
	}
}

void USpatialTypeBinding_PlayerController::BuildSpatialComponentUpdate(
	const FPropertyChangeState& Changes,
	USpatialActorChannel* Channel,
	improbable::unreal::UnrealPlayerControllerSingleClientReplicatedData::Update& SingleClientUpdate,
	bool& bSingleClientUpdateChanged,
	improbable::unreal::UnrealPlayerControllerMultiClientReplicatedData::Update& MultiClientUpdate,
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

void USpatialTypeBinding_PlayerController::ServerSendUpdate_SingleClient(
	const uint8* RESTRICT Data,
	int32 Handle,
	UProperty* Property,
	USpatialActorChannel* Channel,
	improbable::unreal::UnrealPlayerControllerSingleClientReplicatedData::Update& OutUpdate) const
{
	switch (Handle)
	{
		case 18: // field_targetviewrotation
		{
			FRotator Value = *(reinterpret_cast<FRotator const*>(Data));

			OutUpdate.set_field_targetviewrotation(improbable::unreal::UnrealFRotator(Value.Yaw, Value.Pitch, Value.Roll));
			break;
		}
		case 19: // field_spawnlocation
		{
			FVector Value = *(reinterpret_cast<FVector const*>(Data));

			OutUpdate.set_field_spawnlocation(improbable::Vector3f(Value.X, Value.Y, Value.Z));
			break;
		}
	default:
		checkf(false, TEXT("Unknown replication handle %d encountered when creating a SpatialOS update."));
		break;
	}
}

void USpatialTypeBinding_PlayerController::ServerSendUpdate_MultiClient(
	const uint8* RESTRICT Data,
	int32 Handle,
	UProperty* Property,
	USpatialActorChannel* Channel,
	improbable::unreal::UnrealPlayerControllerMultiClientReplicatedData::Update& OutUpdate) const
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
					Interop->QueueOutgoingObjectUpdate(Value, Channel, 5);
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
					Interop->QueueOutgoingObjectUpdate(Value, Channel, 7);
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
					Interop->QueueOutgoingObjectUpdate(Value, Channel, 12);
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
					Interop->QueueOutgoingObjectUpdate(Value, Channel, 15);
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
		case 16: // field_pawn
		{
			APawn* Value = *(reinterpret_cast<APawn* const*>(Data));

			if (Value != nullptr)
			{
				FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(Value);
				improbable::unreal::UnrealObjectRef ObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
				if (ObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
				{
					Interop->QueueOutgoingObjectUpdate(Value, Channel, 16);
				}
				else
				{
					OutUpdate.set_field_pawn(ObjectRef);
				}
			}
			else
			{
				OutUpdate.set_field_pawn(SpatialConstants::NULL_OBJECT_REF);
			}
			break;
		}
		case 17: // field_playerstate
		{
			APlayerState* Value = *(reinterpret_cast<APlayerState* const*>(Data));

			if (Value != nullptr)
			{
				FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(Value);
				improbable::unreal::UnrealObjectRef ObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
				if (ObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
				{
					Interop->QueueOutgoingObjectUpdate(Value, Channel, 17);
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
	default:
		checkf(false, TEXT("Unknown replication handle %d encountered when creating a SpatialOS update."));
		break;
	}
}

void USpatialTypeBinding_PlayerController::ClientReceiveUpdate_SingleClient(
	USpatialActorChannel* ActorChannel,
	const improbable::unreal::UnrealPlayerControllerSingleClientReplicatedData::Update& Update) const
{
	FBunchPayloadWriter OutputWriter(PackageMap);

	auto& HandleToPropertyMap = GetHandlePropertyMap();
	const bool bAutonomousProxy = ActorChannel->IsClientAutonomousProxy(improbable::unreal::UnrealPlayerControllerClientRPCs::ComponentId);
	ConditionMapFilter ConditionMap(ActorChannel, bAutonomousProxy);
	if (!Update.field_targetviewrotation().empty())
	{
		// field_targetviewrotation
		uint32 Handle = 18;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			FRotator Value;

			{
				auto& Rotator = (*Update.field_targetviewrotation().data());
				Value.Yaw = Rotator.yaw();
				Value.Pitch = Rotator.pitch();
				Value.Roll = Rotator.roll();
			}

			OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%lld), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data.Property->GetName(),
				Handle);
		}
	}
	if (!Update.field_spawnlocation().empty())
	{
		// field_spawnlocation
		uint32 Handle = 19;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			FVector Value;

			{
				auto& Vector = (*Update.field_spawnlocation().data());
				Value.X = Vector.x();
				Value.Y = Vector.y();
				Value.Z = Vector.z();
			}

			OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%lld), property %s (handle %d)"),
				*Interop->GetSpatialOS()->GetWorkerId(),
				*ActorChannel->Actor->GetName(),
				ActorChannel->GetEntityId(),
				*Data.Property->GetName(),
				Handle);
		}
	}
	Interop->ReceiveSpatialUpdate(ActorChannel, OutputWriter.GetNetBitWriter());
}

void USpatialTypeBinding_PlayerController::ClientReceiveUpdate_MultiClient(
	USpatialActorChannel* ActorChannel,
	const improbable::unreal::UnrealPlayerControllerMultiClientReplicatedData::Update& Update) const
{
	FBunchPayloadWriter OutputWriter(PackageMap);

	auto& HandleToPropertyMap = GetHandlePropertyMap();
	const bool bAutonomousProxy = ActorChannel->IsClientAutonomousProxy(improbable::unreal::UnrealPlayerControllerClientRPCs::ComponentId);
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
			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%lld), property %s (handle %d)"),
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
			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%lld), property %s (handle %d)"),
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
			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%lld), property %s (handle %d)"),
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
			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%lld), property %s (handle %d)"),
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
						UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received unresolved object property. Value: (entity: %llu, offset: %u). actor %s (%lld), property %s (handle %d)"),
							*Interop->GetSpatialOS()->GetWorkerId(),
							ObjectRef.entity(),
							ObjectRef.offset(),
							*ActorChannel->Actor->GetName(),
							ActorChannel->GetEntityId(),
							*Data.Property->GetName(),
							Handle);
						bWriteObjectProperty = false;
						Interop->QueueIncomingObjectUpdate(ObjectRef, ActorChannel, Cast<UObjectPropertyBase>(Data.Property), Handle);
					}
				}
			}

			if (bWriteObjectProperty)
			{
				OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
				UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%lld), property %s (handle %d)"),
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
			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%lld), property %s (handle %d)"),
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
						UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received unresolved object property. Value: (entity: %llu, offset: %u). actor %s (%lld), property %s (handle %d)"),
							*Interop->GetSpatialOS()->GetWorkerId(),
							ObjectRef.entity(),
							ObjectRef.offset(),
							*ActorChannel->Actor->GetName(),
							ActorChannel->GetEntityId(),
							*Data.Property->GetName(),
							Handle);
						bWriteObjectProperty = false;
						Interop->QueueIncomingObjectUpdate(ObjectRef, ActorChannel, Cast<UObjectPropertyBase>(Data.Property), Handle);
					}
				}
			}

			if (bWriteObjectProperty)
			{
				OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
				UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%lld), property %s (handle %d)"),
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
			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%lld), property %s (handle %d)"),
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
			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%lld), property %s (handle %d)"),
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
			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%lld), property %s (handle %d)"),
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
			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%lld), property %s (handle %d)"),
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
						UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received unresolved object property. Value: (entity: %llu, offset: %u). actor %s (%lld), property %s (handle %d)"),
							*Interop->GetSpatialOS()->GetWorkerId(),
							ObjectRef.entity(),
							ObjectRef.offset(),
							*ActorChannel->Actor->GetName(),
							ActorChannel->GetEntityId(),
							*Data.Property->GetName(),
							Handle);
						bWriteObjectProperty = false;
						Interop->QueueIncomingObjectUpdate(ObjectRef, ActorChannel, Cast<UObjectPropertyBase>(Data.Property), Handle);
					}
				}
			}

			if (bWriteObjectProperty)
			{
				OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
				UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%lld), property %s (handle %d)"),
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
			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%lld), property %s (handle %d)"),
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
			UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%lld), property %s (handle %d)"),
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
						UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received unresolved object property. Value: (entity: %llu, offset: %u). actor %s (%lld), property %s (handle %d)"),
							*Interop->GetSpatialOS()->GetWorkerId(),
							ObjectRef.entity(),
							ObjectRef.offset(),
							*ActorChannel->Actor->GetName(),
							ActorChannel->GetEntityId(),
							*Data.Property->GetName(),
							Handle);
						bWriteObjectProperty = false;
						Interop->QueueIncomingObjectUpdate(ObjectRef, ActorChannel, Cast<UObjectPropertyBase>(Data.Property), Handle);
					}
				}
			}

			if (bWriteObjectProperty)
			{
				OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
				UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%lld), property %s (handle %d)"),
					*Interop->GetSpatialOS()->GetWorkerId(),
					*ActorChannel->Actor->GetName(),
					ActorChannel->GetEntityId(),
					*Data.Property->GetName(),
					Handle);
			}
		}
	}
	if (!Update.field_pawn().empty())
	{
		// field_pawn
		uint32 Handle = 16;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			bool bWriteObjectProperty = true;
			APawn* Value;

			{
				improbable::unreal::UnrealObjectRef ObjectRef = (*Update.field_pawn().data());
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
						UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received unresolved object property. Value: (entity: %llu, offset: %u). actor %s (%lld), property %s (handle %d)"),
							*Interop->GetSpatialOS()->GetWorkerId(),
							ObjectRef.entity(),
							ObjectRef.offset(),
							*ActorChannel->Actor->GetName(),
							ActorChannel->GetEntityId(),
							*Data.Property->GetName(),
							Handle);
						bWriteObjectProperty = false;
						Interop->QueueIncomingObjectUpdate(ObjectRef, ActorChannel, Cast<UObjectPropertyBase>(Data.Property), Handle);
					}
				}
			}

			if (bWriteObjectProperty)
			{
				OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
				UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%lld), property %s (handle %d)"),
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
		uint32 Handle = 17;
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
						UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received unresolved object property. Value: (entity: %llu, offset: %u). actor %s (%lld), property %s (handle %d)"),
							*Interop->GetSpatialOS()->GetWorkerId(),
							ObjectRef.entity(),
							ObjectRef.offset(),
							*ActorChannel->Actor->GetName(),
							ActorChannel->GetEntityId(),
							*Data.Property->GetName(),
							Handle);
						bWriteObjectProperty = false;
						Interop->QueueIncomingObjectUpdate(ObjectRef, ActorChannel, Cast<UObjectPropertyBase>(Data.Property), Handle);
					}
				}
			}

			if (bWriteObjectProperty)
			{
				OutputWriter.SerializeProperty(Handle, Data.Property, &Value);
				UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received property update. actor %s (%lld), property %s (handle %d)"),
					*Interop->GetSpatialOS()->GetWorkerId(),
					*ActorChannel->Actor->GetName(),
					ActorChannel->GetEntityId(),
					*Data.Property->GetName(),
					Handle);
			}
		}
	}
	Interop->ReceiveSpatialUpdate(ActorChannel, OutputWriter.GetNetBitWriter());
}

void USpatialTypeBinding_PlayerController::OnServerStartedVisualLogger_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_UBOOL(bIsLogging);

	auto Sender = [this, Connection, TargetObject, bIsLogging]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC OnServerStartedVisualLogger queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealOnServerStartedVisualLoggerRequest Request;
		Request.set_field_bislogging(bIsLogging);

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: OnServerStartedVisualLogger, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Onserverstartedvisuallogger>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ClientWasKicked_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UTextProperty, KickReason);

	auto Sender = [this, Connection, TargetObject, KickReason]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientWasKicked queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientWasKickedRequest Request;
		// UNSUPPORTED

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientWasKicked, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientwaskicked>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ClientVoiceHandshakeComplete_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	auto Sender = [this, Connection, TargetObject]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientVoiceHandshakeComplete queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientVoiceHandshakeCompleteRequest Request;

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientVoiceHandshakeComplete, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientvoicehandshakecomplete>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ClientUpdateLevelStreamingStatus_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UNameProperty, PackageName);
	P_GET_UBOOL(bNewShouldBeLoaded);
	P_GET_UBOOL(bNewShouldBeVisible);
	P_GET_UBOOL(bNewShouldBlockOnLoad);
	P_GET_PROPERTY(UIntProperty, LODIndex);

	auto Sender = [this, Connection, TargetObject, PackageName, bNewShouldBeLoaded, bNewShouldBeVisible, bNewShouldBlockOnLoad, LODIndex]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientUpdateLevelStreamingStatus queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientUpdateLevelStreamingStatusRequest Request;
		Request.set_field_packagename(TCHAR_TO_UTF8(*PackageName.ToString()));
		Request.set_field_bnewshouldbeloaded(bNewShouldBeLoaded);
		Request.set_field_bnewshouldbevisible(bNewShouldBeVisible);
		Request.set_field_bnewshouldblockonload(bNewShouldBlockOnLoad);
		Request.set_field_lodindex(LODIndex);

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientUpdateLevelStreamingStatus, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientupdatelevelstreamingstatus>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ClientUnmutePlayer_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_STRUCT(FUniqueNetIdRepl, PlayerId)

	auto Sender = [this, Connection, TargetObject, PlayerId]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientUnmutePlayer queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientUnmutePlayerRequest Request;
		{
			TArray<uint8> ValueData;
			FMemoryWriter ValueDataWriter(ValueData);
			bool Success;
			PlayerId.NetSerialize(ValueDataWriter, nullptr, Success);
			Request.set_field_playerid(std::string((char*)ValueData.GetData(), ValueData.Num()));
		}

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientUnmutePlayer, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientunmuteplayer>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ClientTravelInternal_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UStrProperty, URL);
	P_GET_PROPERTY(UByteProperty, TravelType);
	P_GET_UBOOL(bSeamless);
	P_GET_STRUCT(FGuid, MapPackageGuid)

	auto Sender = [this, Connection, TargetObject, URL, TravelType, bSeamless, MapPackageGuid]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientTravelInternal queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientTravelInternalRequest Request;
		Request.set_field_url(TCHAR_TO_UTF8(*URL));
		Request.set_field_traveltype(uint32_t(TravelType));
		Request.set_field_bseamless(bSeamless);
		Request.set_field_mappackageguid_a(MapPackageGuid.A);
		Request.set_field_mappackageguid_b(MapPackageGuid.B);
		Request.set_field_mappackageguid_c(MapPackageGuid.C);
		Request.set_field_mappackageguid_d(MapPackageGuid.D);

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientTravelInternal, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clienttravelinternal>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ClientTeamMessage_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(APlayerState, SenderPlayerState);
	P_GET_PROPERTY(UStrProperty, S);
	P_GET_PROPERTY(UNameProperty, Type);
	P_GET_PROPERTY(UFloatProperty, MsgLifeTime);

	auto Sender = [this, Connection, TargetObject, SenderPlayerState, S, Type, MsgLifeTime]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientTeamMessage queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientTeamMessageRequest Request;
		if (SenderPlayerState != nullptr)
		{
			FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(SenderPlayerState);
			improbable::unreal::UnrealObjectRef ObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
			if (ObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
			{
				UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientTeamMessage queued. SenderPlayerState is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
				return FRPCRequestResult{SenderPlayerState};
			}
			else
			{
				Request.set_field_senderplayerstate(ObjectRef);
			}
		}
		else
		{
			Request.set_field_senderplayerstate(SpatialConstants::NULL_OBJECT_REF);
		}
		Request.set_field_s(TCHAR_TO_UTF8(*S));
		Request.set_field_type(TCHAR_TO_UTF8(*Type.ToString()));
		Request.set_field_msglifetime(MsgLifeTime);

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientTeamMessage, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientteammessage>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ClientStopForceFeedback_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(UForceFeedbackEffect, ForceFeedbackEffect);
	P_GET_PROPERTY(UNameProperty, Tag);

	auto Sender = [this, Connection, TargetObject, ForceFeedbackEffect, Tag]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientStopForceFeedback queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientStopForceFeedbackRequest Request;
		if (ForceFeedbackEffect != nullptr)
		{
			FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(ForceFeedbackEffect);
			improbable::unreal::UnrealObjectRef ObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
			if (ObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
			{
				UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientStopForceFeedback queued. ForceFeedbackEffect is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
				return FRPCRequestResult{ForceFeedbackEffect};
			}
			else
			{
				Request.set_field_forcefeedbackeffect(ObjectRef);
			}
		}
		else
		{
			Request.set_field_forcefeedbackeffect(SpatialConstants::NULL_OBJECT_REF);
		}
		Request.set_field_tag(TCHAR_TO_UTF8(*Tag.ToString()));

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientStopForceFeedback, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstopforcefeedback>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ClientStopCameraShake_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(UClass, Shake);
	P_GET_UBOOL(bImmediately);

	auto Sender = [this, Connection, TargetObject, Shake, bImmediately]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientStopCameraShake queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientStopCameraShakeRequest Request;
		// UNSUPPORTED UClass
		Request.set_field_bimmediately(bImmediately);

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientStopCameraShake, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstopcamerashake>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ClientStopCameraAnim_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(UCameraAnim, AnimToStop);

	auto Sender = [this, Connection, TargetObject, AnimToStop]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientStopCameraAnim queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientStopCameraAnimRequest Request;
		if (AnimToStop != nullptr)
		{
			FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(AnimToStop);
			improbable::unreal::UnrealObjectRef ObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
			if (ObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
			{
				UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientStopCameraAnim queued. AnimToStop is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
				return FRPCRequestResult{AnimToStop};
			}
			else
			{
				Request.set_field_animtostop(ObjectRef);
			}
		}
		else
		{
			Request.set_field_animtostop(SpatialConstants::NULL_OBJECT_REF);
		}

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientStopCameraAnim, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstopcameraanim>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ClientStartOnlineSession_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	auto Sender = [this, Connection, TargetObject]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientStartOnlineSession queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientStartOnlineSessionRequest Request;

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientStartOnlineSession, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstartonlinesession>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ClientSpawnCameraLensEffect_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(UClass, LensEffectEmitterClass);

	auto Sender = [this, Connection, TargetObject, LensEffectEmitterClass]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientSpawnCameraLensEffect queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientSpawnCameraLensEffectRequest Request;
		// UNSUPPORTED UClass

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientSpawnCameraLensEffect, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientspawncameralenseffect>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ false);
}

void USpatialTypeBinding_PlayerController::ClientSetViewTarget_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(AActor, A);
	P_GET_STRUCT(FViewTargetTransitionParams, TransitionParams)

	auto Sender = [this, Connection, TargetObject, A, TransitionParams]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientSetViewTarget queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientSetViewTargetRequest Request;
		if (A != nullptr)
		{
			FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(A);
			improbable::unreal::UnrealObjectRef ObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
			if (ObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
			{
				UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientSetViewTarget queued. A is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
				return FRPCRequestResult{A};
			}
			else
			{
				Request.set_field_a(ObjectRef);
			}
		}
		else
		{
			Request.set_field_a(SpatialConstants::NULL_OBJECT_REF);
		}
		Request.set_field_transitionparams_blendtime(TransitionParams.BlendTime);
		Request.set_field_transitionparams_blendfunction(uint32_t(TransitionParams.BlendFunction));
		Request.set_field_transitionparams_blendexp(TransitionParams.BlendExp);
		Request.set_field_transitionparams_blockoutgoing(TransitionParams.bLockOutgoing);

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientSetViewTarget, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetviewtarget>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ClientSetSpectatorWaiting_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_UBOOL(bWaiting);

	auto Sender = [this, Connection, TargetObject, bWaiting]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientSetSpectatorWaiting queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientSetSpectatorWaitingRequest Request;
		Request.set_field_bwaiting(bWaiting);

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientSetSpectatorWaiting, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetspectatorwaiting>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ClientSetHUD_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(UClass, NewHUDClass);

	auto Sender = [this, Connection, TargetObject, NewHUDClass]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientSetHUD queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientSetHUDRequest Request;
		// UNSUPPORTED UClass

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientSetHUD, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsethud>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ClientSetForceMipLevelsToBeResident_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(UMaterialInterface, Material);
	P_GET_PROPERTY(UFloatProperty, ForceDuration);
	P_GET_PROPERTY(UIntProperty, CinematicTextureGroups);

	auto Sender = [this, Connection, TargetObject, Material, ForceDuration, CinematicTextureGroups]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientSetForceMipLevelsToBeResident queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientSetForceMipLevelsToBeResidentRequest Request;
		if (Material != nullptr)
		{
			FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(Material);
			improbable::unreal::UnrealObjectRef ObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
			if (ObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
			{
				UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientSetForceMipLevelsToBeResident queued. Material is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
				return FRPCRequestResult{Material};
			}
			else
			{
				Request.set_field_material(ObjectRef);
			}
		}
		else
		{
			Request.set_field_material(SpatialConstants::NULL_OBJECT_REF);
		}
		Request.set_field_forceduration(ForceDuration);
		Request.set_field_cinematictexturegroups(CinematicTextureGroups);

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientSetForceMipLevelsToBeResident, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetforcemiplevelstoberesident>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ClientSetCinematicMode_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_UBOOL(bInCinematicMode);
	P_GET_UBOOL(bAffectsMovement);
	P_GET_UBOOL(bAffectsTurning);
	P_GET_UBOOL(bAffectsHUD);

	auto Sender = [this, Connection, TargetObject, bInCinematicMode, bAffectsMovement, bAffectsTurning, bAffectsHUD]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientSetCinematicMode queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientSetCinematicModeRequest Request;
		Request.set_field_bincinematicmode(bInCinematicMode);
		Request.set_field_baffectsmovement(bAffectsMovement);
		Request.set_field_baffectsturning(bAffectsTurning);
		Request.set_field_baffectshud(bAffectsHUD);

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientSetCinematicMode, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetcinematicmode>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ClientSetCameraMode_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UNameProperty, NewCamMode);

	auto Sender = [this, Connection, TargetObject, NewCamMode]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientSetCameraMode queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientSetCameraModeRequest Request;
		Request.set_field_newcammode(TCHAR_TO_UTF8(*NewCamMode.ToString()));

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientSetCameraMode, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetcameramode>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ClientSetCameraFade_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_UBOOL(bEnableFading);
	P_GET_STRUCT(FColor, FadeColor)
	P_GET_STRUCT(FVector2D, FadeAlpha)
	P_GET_PROPERTY(UFloatProperty, FadeTime);
	P_GET_UBOOL(bFadeAudio);

	auto Sender = [this, Connection, TargetObject, bEnableFading, FadeColor, FadeAlpha, FadeTime, bFadeAudio]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientSetCameraFade queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientSetCameraFadeRequest Request;
		Request.set_field_benablefading(bEnableFading);
		Request.set_field_fadecolor_b(uint32_t(FadeColor.B));
		Request.set_field_fadecolor_g(uint32_t(FadeColor.G));
		Request.set_field_fadecolor_r(uint32_t(FadeColor.R));
		Request.set_field_fadecolor_a(uint32_t(FadeColor.A));
		Request.set_field_fadealpha_x(FadeAlpha.X);
		Request.set_field_fadealpha_y(FadeAlpha.Y);
		Request.set_field_fadetime(FadeTime);
		Request.set_field_bfadeaudio(bFadeAudio);

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientSetCameraFade, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetcamerafade>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ClientSetBlockOnAsyncLoading_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	auto Sender = [this, Connection, TargetObject]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientSetBlockOnAsyncLoading queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientSetBlockOnAsyncLoadingRequest Request;

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientSetBlockOnAsyncLoading, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetblockonasyncloading>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ClientReturnToMainMenu_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UStrProperty, ReturnReason);

	auto Sender = [this, Connection, TargetObject, ReturnReason]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientReturnToMainMenu queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientReturnToMainMenuRequest Request;
		Request.set_field_returnreason(TCHAR_TO_UTF8(*ReturnReason));

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientReturnToMainMenu, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientreturntomainmenu>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ClientRetryClientRestart_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(APawn, NewPawn);

	auto Sender = [this, Connection, TargetObject, NewPawn]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientRetryClientRestart queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientRetryClientRestartRequest Request;
		if (NewPawn != nullptr)
		{
			FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(NewPawn);
			improbable::unreal::UnrealObjectRef ObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
			if (ObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
			{
				UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientRetryClientRestart queued. NewPawn is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
				return FRPCRequestResult{NewPawn};
			}
			else
			{
				Request.set_field_newpawn(ObjectRef);
			}
		}
		else
		{
			Request.set_field_newpawn(SpatialConstants::NULL_OBJECT_REF);
		}

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientRetryClientRestart, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientretryclientrestart>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ClientRestart_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(APawn, NewPawn);

	auto Sender = [this, Connection, TargetObject, NewPawn]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientRestart queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientRestartRequest Request;
		if (NewPawn != nullptr)
		{
			FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(NewPawn);
			improbable::unreal::UnrealObjectRef ObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
			if (ObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
			{
				UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientRestart queued. NewPawn is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
				return FRPCRequestResult{NewPawn};
			}
			else
			{
				Request.set_field_newpawn(ObjectRef);
			}
		}
		else
		{
			Request.set_field_newpawn(SpatialConstants::NULL_OBJECT_REF);
		}

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientRestart, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientrestart>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ClientReset_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	auto Sender = [this, Connection, TargetObject]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientReset queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientResetRequest Request;

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientReset, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientreset>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ClientRepObjRef_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(UObject, Object);

	auto Sender = [this, Connection, TargetObject, Object]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientRepObjRef queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientRepObjRefRequest Request;
		if (Object != nullptr)
		{
			FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(Object);
			improbable::unreal::UnrealObjectRef ObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
			if (ObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
			{
				UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientRepObjRef queued. Object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
				return FRPCRequestResult{Object};
			}
			else
			{
				Request.set_field_object(ObjectRef);
			}
		}
		else
		{
			Request.set_field_object(SpatialConstants::NULL_OBJECT_REF);
		}

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientRepObjRef, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientrepobjref>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ClientReceiveLocalizedMessage_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(UClass, Message);
	P_GET_PROPERTY(UIntProperty, Switch);
	P_GET_OBJECT(APlayerState, RelatedPlayerState_1);
	P_GET_OBJECT(APlayerState, RelatedPlayerState_2);
	P_GET_OBJECT(UObject, OptionalObject);

	auto Sender = [this, Connection, TargetObject, Message, Switch, RelatedPlayerState_1, RelatedPlayerState_2, OptionalObject]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientReceiveLocalizedMessage queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientReceiveLocalizedMessageRequest Request;
		// UNSUPPORTED UClass
		Request.set_field_switch(Switch);
		if (RelatedPlayerState_1 != nullptr)
		{
			FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(RelatedPlayerState_1);
			improbable::unreal::UnrealObjectRef ObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
			if (ObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
			{
				UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientReceiveLocalizedMessage queued. RelatedPlayerState_1 is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
				return FRPCRequestResult{RelatedPlayerState_1};
			}
			else
			{
				Request.set_field_relatedplayerstate_1(ObjectRef);
			}
		}
		else
		{
			Request.set_field_relatedplayerstate_1(SpatialConstants::NULL_OBJECT_REF);
		}
		if (RelatedPlayerState_2 != nullptr)
		{
			FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(RelatedPlayerState_2);
			improbable::unreal::UnrealObjectRef ObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
			if (ObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
			{
				UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientReceiveLocalizedMessage queued. RelatedPlayerState_2 is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
				return FRPCRequestResult{RelatedPlayerState_2};
			}
			else
			{
				Request.set_field_relatedplayerstate_2(ObjectRef);
			}
		}
		else
		{
			Request.set_field_relatedplayerstate_2(SpatialConstants::NULL_OBJECT_REF);
		}
		if (OptionalObject != nullptr)
		{
			FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(OptionalObject);
			improbable::unreal::UnrealObjectRef ObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
			if (ObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
			{
				UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientReceiveLocalizedMessage queued. OptionalObject is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
				return FRPCRequestResult{OptionalObject};
			}
			else
			{
				Request.set_field_optionalobject(ObjectRef);
			}
		}
		else
		{
			Request.set_field_optionalobject(SpatialConstants::NULL_OBJECT_REF);
		}

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientReceiveLocalizedMessage, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientreceivelocalizedmessage>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ClientPrestreamTextures_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(AActor, ForcedActor);
	P_GET_PROPERTY(UFloatProperty, ForceDuration);
	P_GET_UBOOL(bEnableStreaming);
	P_GET_PROPERTY(UIntProperty, CinematicTextureGroups);

	auto Sender = [this, Connection, TargetObject, ForcedActor, ForceDuration, bEnableStreaming, CinematicTextureGroups]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientPrestreamTextures queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientPrestreamTexturesRequest Request;
		if (ForcedActor != nullptr)
		{
			FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(ForcedActor);
			improbable::unreal::UnrealObjectRef ObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
			if (ObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
			{
				UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientPrestreamTextures queued. ForcedActor is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
				return FRPCRequestResult{ForcedActor};
			}
			else
			{
				Request.set_field_forcedactor(ObjectRef);
			}
		}
		else
		{
			Request.set_field_forcedactor(SpatialConstants::NULL_OBJECT_REF);
		}
		Request.set_field_forceduration(ForceDuration);
		Request.set_field_benablestreaming(bEnableStreaming);
		Request.set_field_cinematictexturegroups(CinematicTextureGroups);

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientPrestreamTextures, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientprestreamtextures>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ClientPrepareMapChange_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UNameProperty, LevelName);
	P_GET_UBOOL(bFirst);
	P_GET_UBOOL(bLast);

	auto Sender = [this, Connection, TargetObject, LevelName, bFirst, bLast]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientPrepareMapChange queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientPrepareMapChangeRequest Request;
		Request.set_field_levelname(TCHAR_TO_UTF8(*LevelName.ToString()));
		Request.set_field_bfirst(bFirst);
		Request.set_field_blast(bLast);

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientPrepareMapChange, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientpreparemapchange>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ClientPlaySoundAtLocation_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(USoundBase, Sound);
	P_GET_STRUCT(FVector, Location)
	P_GET_PROPERTY(UFloatProperty, VolumeMultiplier);
	P_GET_PROPERTY(UFloatProperty, PitchMultiplier);

	auto Sender = [this, Connection, TargetObject, Sound, Location, VolumeMultiplier, PitchMultiplier]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientPlaySoundAtLocation queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientPlaySoundAtLocationRequest Request;
		if (Sound != nullptr)
		{
			FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(Sound);
			improbable::unreal::UnrealObjectRef ObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
			if (ObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
			{
				UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientPlaySoundAtLocation queued. Sound is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
				return FRPCRequestResult{Sound};
			}
			else
			{
				Request.set_field_sound(ObjectRef);
			}
		}
		else
		{
			Request.set_field_sound(SpatialConstants::NULL_OBJECT_REF);
		}
		Request.set_field_location(improbable::Vector3f(Location.X, Location.Y, Location.Z));
		Request.set_field_volumemultiplier(VolumeMultiplier);
		Request.set_field_pitchmultiplier(PitchMultiplier);

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientPlaySoundAtLocation, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaysoundatlocation>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ false);
}

void USpatialTypeBinding_PlayerController::ClientPlaySound_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(USoundBase, Sound);
	P_GET_PROPERTY(UFloatProperty, VolumeMultiplier);
	P_GET_PROPERTY(UFloatProperty, PitchMultiplier);

	auto Sender = [this, Connection, TargetObject, Sound, VolumeMultiplier, PitchMultiplier]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientPlaySound queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientPlaySoundRequest Request;
		if (Sound != nullptr)
		{
			FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(Sound);
			improbable::unreal::UnrealObjectRef ObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
			if (ObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
			{
				UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientPlaySound queued. Sound is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
				return FRPCRequestResult{Sound};
			}
			else
			{
				Request.set_field_sound(ObjectRef);
			}
		}
		else
		{
			Request.set_field_sound(SpatialConstants::NULL_OBJECT_REF);
		}
		Request.set_field_volumemultiplier(VolumeMultiplier);
		Request.set_field_pitchmultiplier(PitchMultiplier);

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientPlaySound, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaysound>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ false);
}

void USpatialTypeBinding_PlayerController::ClientPlayForceFeedback_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(UForceFeedbackEffect, ForceFeedbackEffect);
	P_GET_UBOOL(bLooping);
	P_GET_PROPERTY(UNameProperty, Tag);

	auto Sender = [this, Connection, TargetObject, ForceFeedbackEffect, bLooping, Tag]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientPlayForceFeedback queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientPlayForceFeedbackRequest Request;
		if (ForceFeedbackEffect != nullptr)
		{
			FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(ForceFeedbackEffect);
			improbable::unreal::UnrealObjectRef ObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
			if (ObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
			{
				UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientPlayForceFeedback queued. ForceFeedbackEffect is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
				return FRPCRequestResult{ForceFeedbackEffect};
			}
			else
			{
				Request.set_field_forcefeedbackeffect(ObjectRef);
			}
		}
		else
		{
			Request.set_field_forcefeedbackeffect(SpatialConstants::NULL_OBJECT_REF);
		}
		Request.set_field_blooping(bLooping);
		Request.set_field_tag(TCHAR_TO_UTF8(*Tag.ToString()));

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientPlayForceFeedback, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplayforcefeedback>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ false);
}

void USpatialTypeBinding_PlayerController::ClientPlayCameraShake_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(UClass, Shake);
	P_GET_PROPERTY(UFloatProperty, Scale);
	P_GET_PROPERTY(UByteProperty, PlaySpace);
	P_GET_STRUCT(FRotator, UserPlaySpaceRot)

	auto Sender = [this, Connection, TargetObject, Shake, Scale, PlaySpace, UserPlaySpaceRot]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientPlayCameraShake queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientPlayCameraShakeRequest Request;
		// UNSUPPORTED UClass
		Request.set_field_scale(Scale);
		Request.set_field_playspace(uint32_t(PlaySpace));
		Request.set_field_userplayspacerot(improbable::unreal::UnrealFRotator(UserPlaySpaceRot.Yaw, UserPlaySpaceRot.Pitch, UserPlaySpaceRot.Roll));

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientPlayCameraShake, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaycamerashake>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ false);
}

void USpatialTypeBinding_PlayerController::ClientPlayCameraAnim_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(UCameraAnim, AnimToPlay);
	P_GET_PROPERTY(UFloatProperty, Scale);
	P_GET_PROPERTY(UFloatProperty, Rate);
	P_GET_PROPERTY(UFloatProperty, BlendInTime);
	P_GET_PROPERTY(UFloatProperty, BlendOutTime);
	P_GET_UBOOL(bLoop);
	P_GET_UBOOL(bRandomStartTime);
	P_GET_PROPERTY(UByteProperty, Space);
	P_GET_STRUCT(FRotator, CustomPlaySpace)

	auto Sender = [this, Connection, TargetObject, AnimToPlay, Scale, Rate, BlendInTime, BlendOutTime, bLoop, bRandomStartTime, Space, CustomPlaySpace]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientPlayCameraAnim queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientPlayCameraAnimRequest Request;
		if (AnimToPlay != nullptr)
		{
			FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(AnimToPlay);
			improbable::unreal::UnrealObjectRef ObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
			if (ObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
			{
				UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientPlayCameraAnim queued. AnimToPlay is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
				return FRPCRequestResult{AnimToPlay};
			}
			else
			{
				Request.set_field_animtoplay(ObjectRef);
			}
		}
		else
		{
			Request.set_field_animtoplay(SpatialConstants::NULL_OBJECT_REF);
		}
		Request.set_field_scale(Scale);
		Request.set_field_rate(Rate);
		Request.set_field_blendintime(BlendInTime);
		Request.set_field_blendouttime(BlendOutTime);
		Request.set_field_bloop(bLoop);
		Request.set_field_brandomstarttime(bRandomStartTime);
		Request.set_field_space(uint32_t(Space));
		Request.set_field_customplayspace(improbable::unreal::UnrealFRotator(CustomPlaySpace.Yaw, CustomPlaySpace.Pitch, CustomPlaySpace.Roll));

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientPlayCameraAnim, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaycameraanim>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ false);
}

void USpatialTypeBinding_PlayerController::ClientMutePlayer_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_STRUCT(FUniqueNetIdRepl, PlayerId)

	auto Sender = [this, Connection, TargetObject, PlayerId]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientMutePlayer queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientMutePlayerRequest Request;
		{
			TArray<uint8> ValueData;
			FMemoryWriter ValueDataWriter(ValueData);
			bool Success;
			PlayerId.NetSerialize(ValueDataWriter, nullptr, Success);
			Request.set_field_playerid(std::string((char*)ValueData.GetData(), ValueData.Num()));
		}

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientMutePlayer, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientmuteplayer>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ClientMessage_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UStrProperty, S);
	P_GET_PROPERTY(UNameProperty, Type);
	P_GET_PROPERTY(UFloatProperty, MsgLifeTime);

	auto Sender = [this, Connection, TargetObject, S, Type, MsgLifeTime]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientMessage queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientMessageRequest Request;
		Request.set_field_s(TCHAR_TO_UTF8(*S));
		Request.set_field_type(TCHAR_TO_UTF8(*Type.ToString()));
		Request.set_field_msglifetime(MsgLifeTime);

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientMessage, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientmessage>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ClientIgnoreMoveInput_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_UBOOL(bIgnore);

	auto Sender = [this, Connection, TargetObject, bIgnore]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientIgnoreMoveInput queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientIgnoreMoveInputRequest Request;
		Request.set_field_bignore(bIgnore);

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientIgnoreMoveInput, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientignoremoveinput>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ClientIgnoreLookInput_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_UBOOL(bIgnore);

	auto Sender = [this, Connection, TargetObject, bIgnore]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientIgnoreLookInput queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientIgnoreLookInputRequest Request;
		Request.set_field_bignore(bIgnore);

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientIgnoreLookInput, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientignorelookinput>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ClientGotoState_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UNameProperty, NewState);

	auto Sender = [this, Connection, TargetObject, NewState]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientGotoState queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientGotoStateRequest Request;
		Request.set_field_newstate(TCHAR_TO_UTF8(*NewState.ToString()));

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientGotoState, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientgotostate>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ClientGameEnded_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(AActor, EndGameFocus);
	P_GET_UBOOL(bIsWinner);

	auto Sender = [this, Connection, TargetObject, EndGameFocus, bIsWinner]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientGameEnded queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientGameEndedRequest Request;
		if (EndGameFocus != nullptr)
		{
			FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(EndGameFocus);
			improbable::unreal::UnrealObjectRef ObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
			if (ObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
			{
				UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientGameEnded queued. EndGameFocus is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
				return FRPCRequestResult{EndGameFocus};
			}
			else
			{
				Request.set_field_endgamefocus(ObjectRef);
			}
		}
		else
		{
			Request.set_field_endgamefocus(SpatialConstants::NULL_OBJECT_REF);
		}
		Request.set_field_biswinner(bIsWinner);

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientGameEnded, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientgameended>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ClientForceGarbageCollection_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	auto Sender = [this, Connection, TargetObject]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientForceGarbageCollection queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientForceGarbageCollectionRequest Request;

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientForceGarbageCollection, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientforcegarbagecollection>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ClientFlushLevelStreaming_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	auto Sender = [this, Connection, TargetObject]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientFlushLevelStreaming queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientFlushLevelStreamingRequest Request;

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientFlushLevelStreaming, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientflushlevelstreaming>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ClientEndOnlineSession_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	auto Sender = [this, Connection, TargetObject]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientEndOnlineSession queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientEndOnlineSessionRequest Request;

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientEndOnlineSession, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientendonlinesession>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ClientEnableNetworkVoice_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_UBOOL(bEnable);

	auto Sender = [this, Connection, TargetObject, bEnable]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientEnableNetworkVoice queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientEnableNetworkVoiceRequest Request;
		Request.set_field_benable(bEnable);

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientEnableNetworkVoice, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientenablenetworkvoice>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ClientCommitMapChange_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	auto Sender = [this, Connection, TargetObject]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientCommitMapChange queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientCommitMapChangeRequest Request;

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientCommitMapChange, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientcommitmapchange>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ClientClearCameraLensEffects_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	auto Sender = [this, Connection, TargetObject]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientClearCameraLensEffects queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientClearCameraLensEffectsRequest Request;

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientClearCameraLensEffects, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientclearcameralenseffects>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ClientCapBandwidth_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UIntProperty, Cap);

	auto Sender = [this, Connection, TargetObject, Cap]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientCapBandwidth queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientCapBandwidthRequest Request;
		Request.set_field_cap(Cap);

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientCapBandwidth, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientcapbandwidth>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ClientCancelPendingMapChange_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	auto Sender = [this, Connection, TargetObject]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientCancelPendingMapChange queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientCancelPendingMapChangeRequest Request;

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientCancelPendingMapChange, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientcancelpendingmapchange>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ClientAddTextureStreamingLoc_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_STRUCT(FVector, InLoc)
	P_GET_PROPERTY(UFloatProperty, Duration);
	P_GET_UBOOL(bOverrideLocation);

	auto Sender = [this, Connection, TargetObject, InLoc, Duration, bOverrideLocation]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientAddTextureStreamingLoc queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientAddTextureStreamingLocRequest Request;
		Request.set_field_inloc(improbable::Vector3f(InLoc.X, InLoc.Y, InLoc.Z));
		Request.set_field_duration(Duration);
		Request.set_field_boverridelocation(bOverrideLocation);

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientAddTextureStreamingLoc, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientaddtexturestreamingloc>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ClientSetRotation_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_STRUCT(FRotator, NewRotation)
	P_GET_UBOOL(bResetCamera);

	auto Sender = [this, Connection, TargetObject, NewRotation, bResetCamera]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientSetRotation queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientSetRotationRequest Request;
		Request.set_field_newrotation(improbable::unreal::UnrealFRotator(NewRotation.Yaw, NewRotation.Pitch, NewRotation.Roll));
		Request.set_field_bresetcamera(bResetCamera);

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientSetRotation, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetrotation>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ClientSetLocation_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_STRUCT(FVector, NewLocation)
	P_GET_STRUCT(FRotator, NewRotation)

	auto Sender = [this, Connection, TargetObject, NewLocation, NewRotation]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ClientSetLocation queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealClientSetLocationRequest Request;
		Request.set_field_newlocation(improbable::Vector3f(NewLocation.X, NewLocation.Y, NewLocation.Z));
		Request.set_field_newrotation(improbable::unreal::UnrealFRotator(NewRotation.Yaw, NewRotation.Pitch, NewRotation.Roll));

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ClientSetLocation, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetlocation>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ServerViewSelf_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_STRUCT(FViewTargetTransitionParams, TransitionParams)

	auto Sender = [this, Connection, TargetObject, TransitionParams]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ServerViewSelf queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealServerViewSelfRequest Request;
		Request.set_field_transitionparams_blendtime(TransitionParams.BlendTime);
		Request.set_field_transitionparams_blendfunction(uint32_t(TransitionParams.BlendFunction));
		Request.set_field_transitionparams_blendexp(TransitionParams.BlendExp);
		Request.set_field_transitionparams_blockoutgoing(TransitionParams.bLockOutgoing);

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ServerViewSelf, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverviewself>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ false);
}

void USpatialTypeBinding_PlayerController::ServerViewPrevPlayer_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	auto Sender = [this, Connection, TargetObject]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ServerViewPrevPlayer queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealServerViewPrevPlayerRequest Request;

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ServerViewPrevPlayer, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverviewprevplayer>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ false);
}

void USpatialTypeBinding_PlayerController::ServerViewNextPlayer_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	auto Sender = [this, Connection, TargetObject]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ServerViewNextPlayer queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealServerViewNextPlayerRequest Request;

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ServerViewNextPlayer, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverviewnextplayer>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ false);
}

void USpatialTypeBinding_PlayerController::ServerVerifyViewTarget_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	auto Sender = [this, Connection, TargetObject]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ServerVerifyViewTarget queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealServerVerifyViewTargetRequest Request;

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ServerVerifyViewTarget, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serververifyviewtarget>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ServerUpdateLevelVisibility_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UNameProperty, PackageName);
	P_GET_UBOOL(bIsVisible);

	auto Sender = [this, Connection, TargetObject, PackageName, bIsVisible]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ServerUpdateLevelVisibility queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealServerUpdateLevelVisibilityRequest Request;
		Request.set_field_packagename(TCHAR_TO_UTF8(*PackageName.ToString()));
		Request.set_field_bisvisible(bIsVisible);

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ServerUpdateLevelVisibility, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverupdatelevelvisibility>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ServerUpdateCamera_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_STRUCT(FVector_NetQuantize, CamLoc)
	P_GET_PROPERTY(UIntProperty, CamPitchAndYaw);

	auto Sender = [this, Connection, TargetObject, CamLoc, CamPitchAndYaw]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ServerUpdateCamera queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealServerUpdateCameraRequest Request;
		Request.set_field_camloc(improbable::Vector3f(CamLoc.X, CamLoc.Y, CamLoc.Z));
		Request.set_field_campitchandyaw(CamPitchAndYaw);

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ServerUpdateCamera, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverupdatecamera>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ false);
}

void USpatialTypeBinding_PlayerController::ServerUnmutePlayer_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_STRUCT(FUniqueNetIdRepl, PlayerId)

	auto Sender = [this, Connection, TargetObject, PlayerId]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ServerUnmutePlayer queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealServerUnmutePlayerRequest Request;
		{
			TArray<uint8> ValueData;
			FMemoryWriter ValueDataWriter(ValueData);
			bool Success;
			PlayerId.NetSerialize(ValueDataWriter, nullptr, Success);
			Request.set_field_playerid(std::string((char*)ValueData.GetData(), ValueData.Num()));
		}

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ServerUnmutePlayer, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverunmuteplayer>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ServerToggleAILogging_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	auto Sender = [this, Connection, TargetObject]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ServerToggleAILogging queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealServerToggleAILoggingRequest Request;

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ServerToggleAILogging, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servertoggleailogging>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ServerShortTimeout_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	auto Sender = [this, Connection, TargetObject]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ServerShortTimeout queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealServerShortTimeoutRequest Request;

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ServerShortTimeout, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servershorttimeout>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ServerSetSpectatorWaiting_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_UBOOL(bWaiting);

	auto Sender = [this, Connection, TargetObject, bWaiting]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ServerSetSpectatorWaiting queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealServerSetSpectatorWaitingRequest Request;
		Request.set_field_bwaiting(bWaiting);

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ServerSetSpectatorWaiting, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serversetspectatorwaiting>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ServerSetSpectatorLocation_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_STRUCT(FVector, NewLoc)
	P_GET_STRUCT(FRotator, NewRot)

	auto Sender = [this, Connection, TargetObject, NewLoc, NewRot]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ServerSetSpectatorLocation queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealServerSetSpectatorLocationRequest Request;
		Request.set_field_newloc(improbable::Vector3f(NewLoc.X, NewLoc.Y, NewLoc.Z));
		Request.set_field_newrot(improbable::unreal::UnrealFRotator(NewRot.Yaw, NewRot.Pitch, NewRot.Roll));

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ServerSetSpectatorLocation, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serversetspectatorlocation>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ false);
}

void USpatialTypeBinding_PlayerController::ServerRestartPlayer_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	auto Sender = [this, Connection, TargetObject]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ServerRestartPlayer queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealServerRestartPlayerRequest Request;

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ServerRestartPlayer, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverrestartplayer>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ServerPause_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	auto Sender = [this, Connection, TargetObject]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ServerPause queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealServerPauseRequest Request;

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ServerPause, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverpause>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ServerNotifyLoadedWorld_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UNameProperty, WorldPackageName);

	auto Sender = [this, Connection, TargetObject, WorldPackageName]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ServerNotifyLoadedWorld queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealServerNotifyLoadedWorldRequest Request;
		Request.set_field_worldpackagename(TCHAR_TO_UTF8(*WorldPackageName.ToString()));

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ServerNotifyLoadedWorld, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servernotifyloadedworld>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ServerMutePlayer_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_STRUCT(FUniqueNetIdRepl, PlayerId)

	auto Sender = [this, Connection, TargetObject, PlayerId]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ServerMutePlayer queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealServerMutePlayerRequest Request;
		{
			TArray<uint8> ValueData;
			FMemoryWriter ValueDataWriter(ValueData);
			bool Success;
			PlayerId.NetSerialize(ValueDataWriter, nullptr, Success);
			Request.set_field_playerid(std::string((char*)ValueData.GetData(), ValueData.Num()));
		}

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ServerMutePlayer, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servermuteplayer>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ServerCheckClientPossessionReliable_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	auto Sender = [this, Connection, TargetObject]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ServerCheckClientPossessionReliable queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealServerCheckClientPossessionReliableRequest Request;

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ServerCheckClientPossessionReliable, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servercheckclientpossessionreliable>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ServerCheckClientPossession_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	auto Sender = [this, Connection, TargetObject]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ServerCheckClientPossession queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealServerCheckClientPossessionRequest Request;

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ServerCheckClientPossession, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servercheckclientpossession>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ false);
}

void USpatialTypeBinding_PlayerController::ServerChangeName_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UStrProperty, S);

	auto Sender = [this, Connection, TargetObject, S]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ServerChangeName queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealServerChangeNameRequest Request;
		Request.set_field_s(TCHAR_TO_UTF8(*S));

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ServerChangeName, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverchangename>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ServerCamera_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UNameProperty, NewMode);

	auto Sender = [this, Connection, TargetObject, NewMode]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ServerCamera queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealServerCameraRequest Request;
		Request.set_field_newmode(TCHAR_TO_UTF8(*NewMode.ToString()));

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ServerCamera, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servercamera>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::ServerAcknowledgePossession_Sender(worker::Connection* const Connection, struct FFrame* const RPCFrame, UObject* TargetObject)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(APawn, P);

	auto Sender = [this, Connection, TargetObject, P]() mutable -> FRPCRequestResult
	{
		// Resolve TargetObject.
		improbable::unreal::UnrealObjectRef TargetObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(PackageMap->GetNetGUIDFromObject(TargetObject));
		if (TargetObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ServerAcknowledgePossession queued. Target object is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
			return FRPCRequestResult{TargetObject};
		}

		// Build request.
		improbable::unreal::UnrealServerAcknowledgePossessionRequest Request;
		if (P != nullptr)
		{
			FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(P);
			improbable::unreal::UnrealObjectRef ObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
			if (ObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
			{
				UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: RPC ServerAcknowledgePossession queued. P is unresolved."), *Interop->GetSpatialOS()->GetWorkerId());
				return FRPCRequestResult{P};
			}
			else
			{
				Request.set_field_p(ObjectRef);
			}
		}
		else
		{
			Request.set_field_p(SpatialConstants::NULL_OBJECT_REF);
		}

		// Send command request.
		Request.set_target_subobject_offset(TargetObjectRef.offset());
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Sending RPC: ServerAcknowledgePossession, target: %s (entity ID %lld, offset: %d)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		auto RequestId = Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serveracknowledgepossession>(TargetObjectRef.entity(), Request, 0);
		return FRPCRequestResult{RequestId.Id};
	};
	Interop->SendCommandRequest(Sender, /*bReliable*/ true);
}

void USpatialTypeBinding_PlayerController::OnServerStartedVisualLogger_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Onserverstartedvisuallogger>& Op)
{
	Interop->HandleCommandResponse(TEXT("OnServerStartedVisualLogger"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ClientWasKicked_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientwaskicked>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientWasKicked"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ClientVoiceHandshakeComplete_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientvoicehandshakecomplete>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientVoiceHandshakeComplete"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ClientUpdateLevelStreamingStatus_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientupdatelevelstreamingstatus>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientUpdateLevelStreamingStatus"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ClientUnmutePlayer_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientunmuteplayer>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientUnmutePlayer"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ClientTravelInternal_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clienttravelinternal>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientTravelInternal"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ClientTeamMessage_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientteammessage>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientTeamMessage"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ClientStopForceFeedback_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstopforcefeedback>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientStopForceFeedback"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ClientStopCameraShake_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstopcamerashake>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientStopCameraShake"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ClientStopCameraAnim_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstopcameraanim>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientStopCameraAnim"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ClientStartOnlineSession_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstartonlinesession>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientStartOnlineSession"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ClientSpawnCameraLensEffect_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientspawncameralenseffect>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientSpawnCameraLensEffect"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ClientSetViewTarget_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetviewtarget>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientSetViewTarget"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ClientSetSpectatorWaiting_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetspectatorwaiting>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientSetSpectatorWaiting"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ClientSetHUD_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsethud>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientSetHUD"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ClientSetForceMipLevelsToBeResident_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetforcemiplevelstoberesident>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientSetForceMipLevelsToBeResident"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ClientSetCinematicMode_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetcinematicmode>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientSetCinematicMode"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ClientSetCameraMode_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetcameramode>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientSetCameraMode"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ClientSetCameraFade_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetcamerafade>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientSetCameraFade"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ClientSetBlockOnAsyncLoading_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetblockonasyncloading>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientSetBlockOnAsyncLoading"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ClientReturnToMainMenu_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientreturntomainmenu>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientReturnToMainMenu"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ClientRetryClientRestart_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientretryclientrestart>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientRetryClientRestart"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ClientRestart_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientrestart>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientRestart"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ClientReset_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientreset>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientReset"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ClientRepObjRef_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientrepobjref>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientRepObjRef"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ClientReceiveLocalizedMessage_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientreceivelocalizedmessage>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientReceiveLocalizedMessage"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ClientPrestreamTextures_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientprestreamtextures>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientPrestreamTextures"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ClientPrepareMapChange_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientpreparemapchange>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientPrepareMapChange"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ClientPlaySoundAtLocation_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaysoundatlocation>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientPlaySoundAtLocation"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ClientPlaySound_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaysound>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientPlaySound"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ClientPlayForceFeedback_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplayforcefeedback>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientPlayForceFeedback"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ClientPlayCameraShake_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaycamerashake>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientPlayCameraShake"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ClientPlayCameraAnim_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaycameraanim>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientPlayCameraAnim"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ClientMutePlayer_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientmuteplayer>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientMutePlayer"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ClientMessage_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientmessage>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientMessage"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ClientIgnoreMoveInput_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientignoremoveinput>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientIgnoreMoveInput"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ClientIgnoreLookInput_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientignorelookinput>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientIgnoreLookInput"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ClientGotoState_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientgotostate>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientGotoState"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ClientGameEnded_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientgameended>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientGameEnded"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ClientForceGarbageCollection_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientforcegarbagecollection>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientForceGarbageCollection"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ClientFlushLevelStreaming_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientflushlevelstreaming>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientFlushLevelStreaming"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ClientEndOnlineSession_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientendonlinesession>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientEndOnlineSession"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ClientEnableNetworkVoice_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientenablenetworkvoice>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientEnableNetworkVoice"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ClientCommitMapChange_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientcommitmapchange>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientCommitMapChange"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ClientClearCameraLensEffects_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientclearcameralenseffects>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientClearCameraLensEffects"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ClientCapBandwidth_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientcapbandwidth>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientCapBandwidth"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ClientCancelPendingMapChange_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientcancelpendingmapchange>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientCancelPendingMapChange"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ClientAddTextureStreamingLoc_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientaddtexturestreamingloc>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientAddTextureStreamingLoc"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ClientSetRotation_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetrotation>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientSetRotation"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ClientSetLocation_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetlocation>& Op)
{
	Interop->HandleCommandResponse(TEXT("ClientSetLocation"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ServerViewSelf_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverviewself>& Op)
{
	Interop->HandleCommandResponse(TEXT("ServerViewSelf"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ServerViewPrevPlayer_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverviewprevplayer>& Op)
{
	Interop->HandleCommandResponse(TEXT("ServerViewPrevPlayer"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ServerViewNextPlayer_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverviewnextplayer>& Op)
{
	Interop->HandleCommandResponse(TEXT("ServerViewNextPlayer"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ServerVerifyViewTarget_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serververifyviewtarget>& Op)
{
	Interop->HandleCommandResponse(TEXT("ServerVerifyViewTarget"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ServerUpdateLevelVisibility_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverupdatelevelvisibility>& Op)
{
	Interop->HandleCommandResponse(TEXT("ServerUpdateLevelVisibility"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ServerUpdateCamera_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverupdatecamera>& Op)
{
	Interop->HandleCommandResponse(TEXT("ServerUpdateCamera"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ServerUnmutePlayer_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverunmuteplayer>& Op)
{
	Interop->HandleCommandResponse(TEXT("ServerUnmutePlayer"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ServerToggleAILogging_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servertoggleailogging>& Op)
{
	Interop->HandleCommandResponse(TEXT("ServerToggleAILogging"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ServerShortTimeout_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servershorttimeout>& Op)
{
	Interop->HandleCommandResponse(TEXT("ServerShortTimeout"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ServerSetSpectatorWaiting_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serversetspectatorwaiting>& Op)
{
	Interop->HandleCommandResponse(TEXT("ServerSetSpectatorWaiting"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ServerSetSpectatorLocation_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serversetspectatorlocation>& Op)
{
	Interop->HandleCommandResponse(TEXT("ServerSetSpectatorLocation"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ServerRestartPlayer_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverrestartplayer>& Op)
{
	Interop->HandleCommandResponse(TEXT("ServerRestartPlayer"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ServerPause_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverpause>& Op)
{
	Interop->HandleCommandResponse(TEXT("ServerPause"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ServerNotifyLoadedWorld_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servernotifyloadedworld>& Op)
{
	Interop->HandleCommandResponse(TEXT("ServerNotifyLoadedWorld"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ServerMutePlayer_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servermuteplayer>& Op)
{
	Interop->HandleCommandResponse(TEXT("ServerMutePlayer"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ServerCheckClientPossessionReliable_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servercheckclientpossessionreliable>& Op)
{
	Interop->HandleCommandResponse(TEXT("ServerCheckClientPossessionReliable"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ServerCheckClientPossession_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servercheckclientpossession>& Op)
{
	Interop->HandleCommandResponse(TEXT("ServerCheckClientPossession"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ServerChangeName_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverchangename>& Op)
{
	Interop->HandleCommandResponse(TEXT("ServerChangeName"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ServerCamera_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servercamera>& Op)
{
	Interop->HandleCommandResponse(TEXT("ServerCamera"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::ServerAcknowledgePossession_Sender_Response(const worker::CommandResponseOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serveracknowledgepossession>& Op)
{
	Interop->HandleCommandResponse(TEXT("ServerAcknowledgePossession"), Op.RequestId.Id, Op.EntityId, Op.StatusCode, FString(UTF8_TO_TCHAR(Op.Message.c_str())));
}

void USpatialTypeBinding_PlayerController::OnServerStartedVisualLogger_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Onserverstartedvisuallogger>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: OnServerStartedVisualLogger_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: OnServerStartedVisualLogger_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: OnServerStartedVisualLogger_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract bIsLogging
		bool bIsLogging;
		bIsLogging = Op.Request.field_bislogging();

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: OnServerStartedVisualLogger, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->OnServerStartedVisualLogger_Implementation(bIsLogging);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Onserverstartedvisuallogger>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ClientWasKicked_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientwaskicked>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientWasKicked_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientWasKicked_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientWasKicked_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract KickReason
		FText KickReason;
		// UNSUPPORTED

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientWasKicked, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ClientWasKicked_Implementation(KickReason);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientwaskicked>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ClientVoiceHandshakeComplete_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientvoicehandshakecomplete>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientVoiceHandshakeComplete_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientVoiceHandshakeComplete_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientVoiceHandshakeComplete_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientVoiceHandshakeComplete, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ClientVoiceHandshakeComplete_Implementation();

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientvoicehandshakecomplete>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ClientUpdateLevelStreamingStatus_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientupdatelevelstreamingstatus>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientUpdateLevelStreamingStatus_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientUpdateLevelStreamingStatus_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientUpdateLevelStreamingStatus_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract PackageName
		FName PackageName;
		PackageName = FName((Op.Request.field_packagename()).data());

		// Extract bNewShouldBeLoaded
		bool bNewShouldBeLoaded;
		bNewShouldBeLoaded = Op.Request.field_bnewshouldbeloaded();

		// Extract bNewShouldBeVisible
		bool bNewShouldBeVisible;
		bNewShouldBeVisible = Op.Request.field_bnewshouldbevisible();

		// Extract bNewShouldBlockOnLoad
		bool bNewShouldBlockOnLoad;
		bNewShouldBlockOnLoad = Op.Request.field_bnewshouldblockonload();

		// Extract LODIndex
		int32 LODIndex;
		LODIndex = Op.Request.field_lodindex();

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientUpdateLevelStreamingStatus, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ClientUpdateLevelStreamingStatus_Implementation(PackageName, bNewShouldBeLoaded, bNewShouldBeVisible, bNewShouldBlockOnLoad, LODIndex);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientupdatelevelstreamingstatus>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ClientUnmutePlayer_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientunmuteplayer>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientUnmutePlayer_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientUnmutePlayer_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientUnmutePlayer_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract PlayerId
		FUniqueNetIdRepl PlayerId;
		{
			auto& ValueDataStr = Op.Request.field_playerid();
			TArray<uint8> ValueData;
			ValueData.Append((uint8*)ValueDataStr.data(), ValueDataStr.size());
			FMemoryReader ValueDataReader(ValueData);
			bool bSuccess;
			PlayerId.NetSerialize(ValueDataReader, nullptr, bSuccess);
		}

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientUnmutePlayer, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ClientUnmutePlayer_Implementation(PlayerId);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientunmuteplayer>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ClientTravelInternal_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clienttravelinternal>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientTravelInternal_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientTravelInternal_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientTravelInternal_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract URL
		FString URL;
		URL = FString(UTF8_TO_TCHAR(Op.Request.field_url().c_str()));

		// Extract TravelType
		TEnumAsByte<ETravelType> TravelType;
		TravelType = TEnumAsByte<ETravelType>(uint8(Op.Request.field_traveltype()));

		// Extract bSeamless
		bool bSeamless;
		bSeamless = Op.Request.field_bseamless();

		// Extract MapPackageGuid
		FGuid MapPackageGuid;
		MapPackageGuid.A = Op.Request.field_mappackageguid_a();
		MapPackageGuid.B = Op.Request.field_mappackageguid_b();
		MapPackageGuid.C = Op.Request.field_mappackageguid_c();
		MapPackageGuid.D = Op.Request.field_mappackageguid_d();

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientTravelInternal, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ClientTravelInternal_Implementation(URL, TravelType, bSeamless, MapPackageGuid);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clienttravelinternal>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ClientTeamMessage_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientteammessage>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientTeamMessage_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientTeamMessage_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientTeamMessage_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract SenderPlayerState
		APlayerState* SenderPlayerState;
		{
			improbable::unreal::UnrealObjectRef ObjectRef = Op.Request.field_senderplayerstate();
			check(ObjectRef != SpatialConstants::UNRESOLVED_OBJECT_REF);
			if (ObjectRef == SpatialConstants::NULL_OBJECT_REF)
			{
				SenderPlayerState = nullptr;
			}
			else
			{
				FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(ObjectRef);
				if (NetGUID.IsValid())
				{
					SenderPlayerState = static_cast<APlayerState*>(PackageMap->GetObjectFromNetGUID(NetGUID, true));
				}
				else
				{
					UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientTeamMessage_Receiver: SenderPlayerState (entity id %lld, offset %d) is not resolved on this worker."),
						*Interop->GetSpatialOS()->GetWorkerId(),
						ObjectRef.entity(),
						ObjectRef.offset());
					return TOptional<improbable::unreal::UnrealObjectRef>(ObjectRef);
				}
			}
		}

		// Extract S
		FString S;
		S = FString(UTF8_TO_TCHAR(Op.Request.field_s().c_str()));

		// Extract Type
		FName Type;
		Type = FName((Op.Request.field_type()).data());

		// Extract MsgLifeTime
		float MsgLifeTime;
		MsgLifeTime = Op.Request.field_msglifetime();

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientTeamMessage, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ClientTeamMessage_Implementation(SenderPlayerState, S, Type, MsgLifeTime);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientteammessage>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ClientStopForceFeedback_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstopforcefeedback>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientStopForceFeedback_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientStopForceFeedback_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientStopForceFeedback_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract ForceFeedbackEffect
		UForceFeedbackEffect* ForceFeedbackEffect;
		{
			improbable::unreal::UnrealObjectRef ObjectRef = Op.Request.field_forcefeedbackeffect();
			check(ObjectRef != SpatialConstants::UNRESOLVED_OBJECT_REF);
			if (ObjectRef == SpatialConstants::NULL_OBJECT_REF)
			{
				ForceFeedbackEffect = nullptr;
			}
			else
			{
				FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(ObjectRef);
				if (NetGUID.IsValid())
				{
					ForceFeedbackEffect = static_cast<UForceFeedbackEffect*>(PackageMap->GetObjectFromNetGUID(NetGUID, true));
				}
				else
				{
					UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientStopForceFeedback_Receiver: ForceFeedbackEffect (entity id %lld, offset %d) is not resolved on this worker."),
						*Interop->GetSpatialOS()->GetWorkerId(),
						ObjectRef.entity(),
						ObjectRef.offset());
					return TOptional<improbable::unreal::UnrealObjectRef>(ObjectRef);
				}
			}
		}

		// Extract Tag
		FName Tag;
		Tag = FName((Op.Request.field_tag()).data());

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientStopForceFeedback, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ClientStopForceFeedback_Implementation(ForceFeedbackEffect, Tag);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstopforcefeedback>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ClientStopCameraShake_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstopcamerashake>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientStopCameraShake_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientStopCameraShake_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientStopCameraShake_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract Shake
		TSubclassOf<UCameraShake>  Shake;
		// UNSUPPORTED UClass

		// Extract bImmediately
		bool bImmediately;
		bImmediately = Op.Request.field_bimmediately();

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientStopCameraShake, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ClientStopCameraShake_Implementation(Shake, bImmediately);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstopcamerashake>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ClientStopCameraAnim_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstopcameraanim>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientStopCameraAnim_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientStopCameraAnim_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientStopCameraAnim_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract AnimToStop
		UCameraAnim* AnimToStop;
		{
			improbable::unreal::UnrealObjectRef ObjectRef = Op.Request.field_animtostop();
			check(ObjectRef != SpatialConstants::UNRESOLVED_OBJECT_REF);
			if (ObjectRef == SpatialConstants::NULL_OBJECT_REF)
			{
				AnimToStop = nullptr;
			}
			else
			{
				FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(ObjectRef);
				if (NetGUID.IsValid())
				{
					AnimToStop = static_cast<UCameraAnim*>(PackageMap->GetObjectFromNetGUID(NetGUID, true));
				}
				else
				{
					UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientStopCameraAnim_Receiver: AnimToStop (entity id %lld, offset %d) is not resolved on this worker."),
						*Interop->GetSpatialOS()->GetWorkerId(),
						ObjectRef.entity(),
						ObjectRef.offset());
					return TOptional<improbable::unreal::UnrealObjectRef>(ObjectRef);
				}
			}
		}

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientStopCameraAnim, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ClientStopCameraAnim_Implementation(AnimToStop);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstopcameraanim>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ClientStartOnlineSession_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstartonlinesession>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientStartOnlineSession_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientStartOnlineSession_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientStartOnlineSession_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientStartOnlineSession, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ClientStartOnlineSession_Implementation();

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstartonlinesession>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ClientSpawnCameraLensEffect_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientspawncameralenseffect>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientSpawnCameraLensEffect_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientSpawnCameraLensEffect_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientSpawnCameraLensEffect_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract LensEffectEmitterClass
		TSubclassOf<AEmitterCameraLensEffectBase>  LensEffectEmitterClass;
		// UNSUPPORTED UClass

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientSpawnCameraLensEffect, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ClientSpawnCameraLensEffect_Implementation(LensEffectEmitterClass);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientspawncameralenseffect>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ClientSetViewTarget_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetviewtarget>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientSetViewTarget_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientSetViewTarget_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientSetViewTarget_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract A
		AActor* A;
		{
			improbable::unreal::UnrealObjectRef ObjectRef = Op.Request.field_a();
			check(ObjectRef != SpatialConstants::UNRESOLVED_OBJECT_REF);
			if (ObjectRef == SpatialConstants::NULL_OBJECT_REF)
			{
				A = nullptr;
			}
			else
			{
				FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(ObjectRef);
				if (NetGUID.IsValid())
				{
					A = static_cast<AActor*>(PackageMap->GetObjectFromNetGUID(NetGUID, true));
				}
				else
				{
					UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientSetViewTarget_Receiver: A (entity id %lld, offset %d) is not resolved on this worker."),
						*Interop->GetSpatialOS()->GetWorkerId(),
						ObjectRef.entity(),
						ObjectRef.offset());
					return TOptional<improbable::unreal::UnrealObjectRef>(ObjectRef);
				}
			}
		}

		// Extract TransitionParams
		FViewTargetTransitionParams TransitionParams;
		TransitionParams.BlendTime = Op.Request.field_transitionparams_blendtime();
		TransitionParams.BlendFunction = TEnumAsByte<EViewTargetBlendFunction>(uint8(Op.Request.field_transitionparams_blendfunction()));
		TransitionParams.BlendExp = Op.Request.field_transitionparams_blendexp();
		TransitionParams.bLockOutgoing = Op.Request.field_transitionparams_blockoutgoing();
		// Because Unreal will look for a specific bit in the serialization function below, we simply set all bits.
		// We will soon move away from using bunches when receiving Spatial updates.
		if (TransitionParams.bLockOutgoing)
		{
			TransitionParams.bLockOutgoing = 0xFF;
		}

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientSetViewTarget, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ClientSetViewTarget_Implementation(A, TransitionParams);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetviewtarget>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ClientSetSpectatorWaiting_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetspectatorwaiting>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientSetSpectatorWaiting_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientSetSpectatorWaiting_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientSetSpectatorWaiting_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract bWaiting
		bool bWaiting;
		bWaiting = Op.Request.field_bwaiting();

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientSetSpectatorWaiting, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ClientSetSpectatorWaiting_Implementation(bWaiting);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetspectatorwaiting>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ClientSetHUD_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsethud>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientSetHUD_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientSetHUD_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientSetHUD_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract NewHUDClass
		TSubclassOf<AHUD>  NewHUDClass;
		// UNSUPPORTED UClass

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientSetHUD, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ClientSetHUD_Implementation(NewHUDClass);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsethud>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ClientSetForceMipLevelsToBeResident_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetforcemiplevelstoberesident>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientSetForceMipLevelsToBeResident_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientSetForceMipLevelsToBeResident_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientSetForceMipLevelsToBeResident_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract Material
		UMaterialInterface* Material;
		{
			improbable::unreal::UnrealObjectRef ObjectRef = Op.Request.field_material();
			check(ObjectRef != SpatialConstants::UNRESOLVED_OBJECT_REF);
			if (ObjectRef == SpatialConstants::NULL_OBJECT_REF)
			{
				Material = nullptr;
			}
			else
			{
				FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(ObjectRef);
				if (NetGUID.IsValid())
				{
					Material = static_cast<UMaterialInterface*>(PackageMap->GetObjectFromNetGUID(NetGUID, true));
				}
				else
				{
					UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientSetForceMipLevelsToBeResident_Receiver: Material (entity id %lld, offset %d) is not resolved on this worker."),
						*Interop->GetSpatialOS()->GetWorkerId(),
						ObjectRef.entity(),
						ObjectRef.offset());
					return TOptional<improbable::unreal::UnrealObjectRef>(ObjectRef);
				}
			}
		}

		// Extract ForceDuration
		float ForceDuration;
		ForceDuration = Op.Request.field_forceduration();

		// Extract CinematicTextureGroups
		int32 CinematicTextureGroups;
		CinematicTextureGroups = Op.Request.field_cinematictexturegroups();

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientSetForceMipLevelsToBeResident, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ClientSetForceMipLevelsToBeResident_Implementation(Material, ForceDuration, CinematicTextureGroups);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetforcemiplevelstoberesident>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ClientSetCinematicMode_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetcinematicmode>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientSetCinematicMode_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientSetCinematicMode_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientSetCinematicMode_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract bInCinematicMode
		bool bInCinematicMode;
		bInCinematicMode = Op.Request.field_bincinematicmode();

		// Extract bAffectsMovement
		bool bAffectsMovement;
		bAffectsMovement = Op.Request.field_baffectsmovement();

		// Extract bAffectsTurning
		bool bAffectsTurning;
		bAffectsTurning = Op.Request.field_baffectsturning();

		// Extract bAffectsHUD
		bool bAffectsHUD;
		bAffectsHUD = Op.Request.field_baffectshud();

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientSetCinematicMode, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ClientSetCinematicMode_Implementation(bInCinematicMode, bAffectsMovement, bAffectsTurning, bAffectsHUD);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetcinematicmode>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ClientSetCameraMode_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetcameramode>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientSetCameraMode_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientSetCameraMode_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientSetCameraMode_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract NewCamMode
		FName NewCamMode;
		NewCamMode = FName((Op.Request.field_newcammode()).data());

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientSetCameraMode, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ClientSetCameraMode_Implementation(NewCamMode);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetcameramode>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ClientSetCameraFade_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetcamerafade>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientSetCameraFade_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientSetCameraFade_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientSetCameraFade_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract bEnableFading
		bool bEnableFading;
		bEnableFading = Op.Request.field_benablefading();

		// Extract FadeColor
		FColor FadeColor;
		FadeColor.B = uint8(uint8(Op.Request.field_fadecolor_b()));
		FadeColor.G = uint8(uint8(Op.Request.field_fadecolor_g()));
		FadeColor.R = uint8(uint8(Op.Request.field_fadecolor_r()));
		FadeColor.A = uint8(uint8(Op.Request.field_fadecolor_a()));

		// Extract FadeAlpha
		FVector2D FadeAlpha;
		FadeAlpha.X = Op.Request.field_fadealpha_x();
		FadeAlpha.Y = Op.Request.field_fadealpha_y();

		// Extract FadeTime
		float FadeTime;
		FadeTime = Op.Request.field_fadetime();

		// Extract bFadeAudio
		bool bFadeAudio;
		bFadeAudio = Op.Request.field_bfadeaudio();

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientSetCameraFade, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ClientSetCameraFade_Implementation(bEnableFading, FadeColor, FadeAlpha, FadeTime, bFadeAudio);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetcamerafade>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ClientSetBlockOnAsyncLoading_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetblockonasyncloading>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientSetBlockOnAsyncLoading_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientSetBlockOnAsyncLoading_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientSetBlockOnAsyncLoading_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientSetBlockOnAsyncLoading, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ClientSetBlockOnAsyncLoading_Implementation();

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetblockonasyncloading>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ClientReturnToMainMenu_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientreturntomainmenu>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientReturnToMainMenu_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientReturnToMainMenu_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientReturnToMainMenu_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract ReturnReason
		FString ReturnReason;
		ReturnReason = FString(UTF8_TO_TCHAR(Op.Request.field_returnreason().c_str()));

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientReturnToMainMenu, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ClientReturnToMainMenu_Implementation(ReturnReason);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientreturntomainmenu>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ClientRetryClientRestart_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientretryclientrestart>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientRetryClientRestart_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientRetryClientRestart_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientRetryClientRestart_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract NewPawn
		APawn* NewPawn;
		{
			improbable::unreal::UnrealObjectRef ObjectRef = Op.Request.field_newpawn();
			check(ObjectRef != SpatialConstants::UNRESOLVED_OBJECT_REF);
			if (ObjectRef == SpatialConstants::NULL_OBJECT_REF)
			{
				NewPawn = nullptr;
			}
			else
			{
				FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(ObjectRef);
				if (NetGUID.IsValid())
				{
					NewPawn = static_cast<APawn*>(PackageMap->GetObjectFromNetGUID(NetGUID, true));
				}
				else
				{
					UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientRetryClientRestart_Receiver: NewPawn (entity id %lld, offset %d) is not resolved on this worker."),
						*Interop->GetSpatialOS()->GetWorkerId(),
						ObjectRef.entity(),
						ObjectRef.offset());
					return TOptional<improbable::unreal::UnrealObjectRef>(ObjectRef);
				}
			}
		}

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientRetryClientRestart, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ClientRetryClientRestart_Implementation(NewPawn);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientretryclientrestart>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ClientRestart_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientrestart>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientRestart_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientRestart_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientRestart_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract NewPawn
		APawn* NewPawn;
		{
			improbable::unreal::UnrealObjectRef ObjectRef = Op.Request.field_newpawn();
			check(ObjectRef != SpatialConstants::UNRESOLVED_OBJECT_REF);
			if (ObjectRef == SpatialConstants::NULL_OBJECT_REF)
			{
				NewPawn = nullptr;
			}
			else
			{
				FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(ObjectRef);
				if (NetGUID.IsValid())
				{
					NewPawn = static_cast<APawn*>(PackageMap->GetObjectFromNetGUID(NetGUID, true));
				}
				else
				{
					UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientRestart_Receiver: NewPawn (entity id %lld, offset %d) is not resolved on this worker."),
						*Interop->GetSpatialOS()->GetWorkerId(),
						ObjectRef.entity(),
						ObjectRef.offset());
					return TOptional<improbable::unreal::UnrealObjectRef>(ObjectRef);
				}
			}
		}

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientRestart, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ClientRestart_Implementation(NewPawn);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientrestart>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ClientReset_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientreset>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientReset_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientReset_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientReset_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientReset, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ClientReset_Implementation();

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientreset>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ClientRepObjRef_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientrepobjref>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientRepObjRef_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientRepObjRef_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientRepObjRef_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract Object
		UObject* Object;
		{
			improbable::unreal::UnrealObjectRef ObjectRef = Op.Request.field_object();
			check(ObjectRef != SpatialConstants::UNRESOLVED_OBJECT_REF);
			if (ObjectRef == SpatialConstants::NULL_OBJECT_REF)
			{
				Object = nullptr;
			}
			else
			{
				FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(ObjectRef);
				if (NetGUID.IsValid())
				{
					Object = static_cast<UObject*>(PackageMap->GetObjectFromNetGUID(NetGUID, true));
				}
				else
				{
					UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientRepObjRef_Receiver: Object (entity id %lld, offset %d) is not resolved on this worker."),
						*Interop->GetSpatialOS()->GetWorkerId(),
						ObjectRef.entity(),
						ObjectRef.offset());
					return TOptional<improbable::unreal::UnrealObjectRef>(ObjectRef);
				}
			}
		}

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientRepObjRef, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ClientRepObjRef_Implementation(Object);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientrepobjref>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ClientReceiveLocalizedMessage_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientreceivelocalizedmessage>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientReceiveLocalizedMessage_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientReceiveLocalizedMessage_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientReceiveLocalizedMessage_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract Message
		TSubclassOf<ULocalMessage>  Message;
		// UNSUPPORTED UClass

		// Extract Switch
		int32 Switch;
		Switch = Op.Request.field_switch();

		// Extract RelatedPlayerState_1
		APlayerState* RelatedPlayerState_1;
		{
			improbable::unreal::UnrealObjectRef ObjectRef = Op.Request.field_relatedplayerstate_1();
			check(ObjectRef != SpatialConstants::UNRESOLVED_OBJECT_REF);
			if (ObjectRef == SpatialConstants::NULL_OBJECT_REF)
			{
				RelatedPlayerState_1 = nullptr;
			}
			else
			{
				FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(ObjectRef);
				if (NetGUID.IsValid())
				{
					RelatedPlayerState_1 = static_cast<APlayerState*>(PackageMap->GetObjectFromNetGUID(NetGUID, true));
				}
				else
				{
					UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientReceiveLocalizedMessage_Receiver: RelatedPlayerState_1 (entity id %lld, offset %d) is not resolved on this worker."),
						*Interop->GetSpatialOS()->GetWorkerId(),
						ObjectRef.entity(),
						ObjectRef.offset());
					return TOptional<improbable::unreal::UnrealObjectRef>(ObjectRef);
				}
			}
		}

		// Extract RelatedPlayerState_2
		APlayerState* RelatedPlayerState_2;
		{
			improbable::unreal::UnrealObjectRef ObjectRef = Op.Request.field_relatedplayerstate_2();
			check(ObjectRef != SpatialConstants::UNRESOLVED_OBJECT_REF);
			if (ObjectRef == SpatialConstants::NULL_OBJECT_REF)
			{
				RelatedPlayerState_2 = nullptr;
			}
			else
			{
				FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(ObjectRef);
				if (NetGUID.IsValid())
				{
					RelatedPlayerState_2 = static_cast<APlayerState*>(PackageMap->GetObjectFromNetGUID(NetGUID, true));
				}
				else
				{
					UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientReceiveLocalizedMessage_Receiver: RelatedPlayerState_2 (entity id %lld, offset %d) is not resolved on this worker."),
						*Interop->GetSpatialOS()->GetWorkerId(),
						ObjectRef.entity(),
						ObjectRef.offset());
					return TOptional<improbable::unreal::UnrealObjectRef>(ObjectRef);
				}
			}
		}

		// Extract OptionalObject
		UObject* OptionalObject;
		{
			improbable::unreal::UnrealObjectRef ObjectRef = Op.Request.field_optionalobject();
			check(ObjectRef != SpatialConstants::UNRESOLVED_OBJECT_REF);
			if (ObjectRef == SpatialConstants::NULL_OBJECT_REF)
			{
				OptionalObject = nullptr;
			}
			else
			{
				FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(ObjectRef);
				if (NetGUID.IsValid())
				{
					OptionalObject = static_cast<UObject*>(PackageMap->GetObjectFromNetGUID(NetGUID, true));
				}
				else
				{
					UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientReceiveLocalizedMessage_Receiver: OptionalObject (entity id %lld, offset %d) is not resolved on this worker."),
						*Interop->GetSpatialOS()->GetWorkerId(),
						ObjectRef.entity(),
						ObjectRef.offset());
					return TOptional<improbable::unreal::UnrealObjectRef>(ObjectRef);
				}
			}
		}

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientReceiveLocalizedMessage, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ClientReceiveLocalizedMessage_Implementation(Message, Switch, RelatedPlayerState_1, RelatedPlayerState_2, OptionalObject);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientreceivelocalizedmessage>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ClientPrestreamTextures_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientprestreamtextures>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientPrestreamTextures_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientPrestreamTextures_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientPrestreamTextures_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract ForcedActor
		AActor* ForcedActor;
		{
			improbable::unreal::UnrealObjectRef ObjectRef = Op.Request.field_forcedactor();
			check(ObjectRef != SpatialConstants::UNRESOLVED_OBJECT_REF);
			if (ObjectRef == SpatialConstants::NULL_OBJECT_REF)
			{
				ForcedActor = nullptr;
			}
			else
			{
				FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(ObjectRef);
				if (NetGUID.IsValid())
				{
					ForcedActor = static_cast<AActor*>(PackageMap->GetObjectFromNetGUID(NetGUID, true));
				}
				else
				{
					UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientPrestreamTextures_Receiver: ForcedActor (entity id %lld, offset %d) is not resolved on this worker."),
						*Interop->GetSpatialOS()->GetWorkerId(),
						ObjectRef.entity(),
						ObjectRef.offset());
					return TOptional<improbable::unreal::UnrealObjectRef>(ObjectRef);
				}
			}
		}

		// Extract ForceDuration
		float ForceDuration;
		ForceDuration = Op.Request.field_forceduration();

		// Extract bEnableStreaming
		bool bEnableStreaming;
		bEnableStreaming = Op.Request.field_benablestreaming();

		// Extract CinematicTextureGroups
		int32 CinematicTextureGroups;
		CinematicTextureGroups = Op.Request.field_cinematictexturegroups();

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientPrestreamTextures, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ClientPrestreamTextures_Implementation(ForcedActor, ForceDuration, bEnableStreaming, CinematicTextureGroups);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientprestreamtextures>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ClientPrepareMapChange_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientpreparemapchange>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientPrepareMapChange_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientPrepareMapChange_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientPrepareMapChange_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract LevelName
		FName LevelName;
		LevelName = FName((Op.Request.field_levelname()).data());

		// Extract bFirst
		bool bFirst;
		bFirst = Op.Request.field_bfirst();

		// Extract bLast
		bool bLast;
		bLast = Op.Request.field_blast();

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientPrepareMapChange, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ClientPrepareMapChange_Implementation(LevelName, bFirst, bLast);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientpreparemapchange>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ClientPlaySoundAtLocation_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaysoundatlocation>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientPlaySoundAtLocation_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientPlaySoundAtLocation_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientPlaySoundAtLocation_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract Sound
		USoundBase* Sound;
		{
			improbable::unreal::UnrealObjectRef ObjectRef = Op.Request.field_sound();
			check(ObjectRef != SpatialConstants::UNRESOLVED_OBJECT_REF);
			if (ObjectRef == SpatialConstants::NULL_OBJECT_REF)
			{
				Sound = nullptr;
			}
			else
			{
				FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(ObjectRef);
				if (NetGUID.IsValid())
				{
					Sound = static_cast<USoundBase*>(PackageMap->GetObjectFromNetGUID(NetGUID, true));
				}
				else
				{
					UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientPlaySoundAtLocation_Receiver: Sound (entity id %lld, offset %d) is not resolved on this worker."),
						*Interop->GetSpatialOS()->GetWorkerId(),
						ObjectRef.entity(),
						ObjectRef.offset());
					return TOptional<improbable::unreal::UnrealObjectRef>(ObjectRef);
				}
			}
		}

		// Extract Location
		FVector Location;
		{
			auto& Vector = Op.Request.field_location();
			Location.X = Vector.x();
			Location.Y = Vector.y();
			Location.Z = Vector.z();
		}

		// Extract VolumeMultiplier
		float VolumeMultiplier;
		VolumeMultiplier = Op.Request.field_volumemultiplier();

		// Extract PitchMultiplier
		float PitchMultiplier;
		PitchMultiplier = Op.Request.field_pitchmultiplier();

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientPlaySoundAtLocation, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ClientPlaySoundAtLocation_Implementation(Sound, Location, VolumeMultiplier, PitchMultiplier);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaysoundatlocation>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ClientPlaySound_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaysound>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientPlaySound_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientPlaySound_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientPlaySound_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract Sound
		USoundBase* Sound;
		{
			improbable::unreal::UnrealObjectRef ObjectRef = Op.Request.field_sound();
			check(ObjectRef != SpatialConstants::UNRESOLVED_OBJECT_REF);
			if (ObjectRef == SpatialConstants::NULL_OBJECT_REF)
			{
				Sound = nullptr;
			}
			else
			{
				FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(ObjectRef);
				if (NetGUID.IsValid())
				{
					Sound = static_cast<USoundBase*>(PackageMap->GetObjectFromNetGUID(NetGUID, true));
				}
				else
				{
					UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientPlaySound_Receiver: Sound (entity id %lld, offset %d) is not resolved on this worker."),
						*Interop->GetSpatialOS()->GetWorkerId(),
						ObjectRef.entity(),
						ObjectRef.offset());
					return TOptional<improbable::unreal::UnrealObjectRef>(ObjectRef);
				}
			}
		}

		// Extract VolumeMultiplier
		float VolumeMultiplier;
		VolumeMultiplier = Op.Request.field_volumemultiplier();

		// Extract PitchMultiplier
		float PitchMultiplier;
		PitchMultiplier = Op.Request.field_pitchmultiplier();

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientPlaySound, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ClientPlaySound_Implementation(Sound, VolumeMultiplier, PitchMultiplier);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaysound>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ClientPlayForceFeedback_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplayforcefeedback>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientPlayForceFeedback_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientPlayForceFeedback_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientPlayForceFeedback_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract ForceFeedbackEffect
		UForceFeedbackEffect* ForceFeedbackEffect;
		{
			improbable::unreal::UnrealObjectRef ObjectRef = Op.Request.field_forcefeedbackeffect();
			check(ObjectRef != SpatialConstants::UNRESOLVED_OBJECT_REF);
			if (ObjectRef == SpatialConstants::NULL_OBJECT_REF)
			{
				ForceFeedbackEffect = nullptr;
			}
			else
			{
				FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(ObjectRef);
				if (NetGUID.IsValid())
				{
					ForceFeedbackEffect = static_cast<UForceFeedbackEffect*>(PackageMap->GetObjectFromNetGUID(NetGUID, true));
				}
				else
				{
					UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientPlayForceFeedback_Receiver: ForceFeedbackEffect (entity id %lld, offset %d) is not resolved on this worker."),
						*Interop->GetSpatialOS()->GetWorkerId(),
						ObjectRef.entity(),
						ObjectRef.offset());
					return TOptional<improbable::unreal::UnrealObjectRef>(ObjectRef);
				}
			}
		}

		// Extract bLooping
		bool bLooping;
		bLooping = Op.Request.field_blooping();

		// Extract Tag
		FName Tag;
		Tag = FName((Op.Request.field_tag()).data());

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientPlayForceFeedback, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ClientPlayForceFeedback_Implementation(ForceFeedbackEffect, bLooping, Tag);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplayforcefeedback>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ClientPlayCameraShake_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaycamerashake>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientPlayCameraShake_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientPlayCameraShake_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientPlayCameraShake_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract Shake
		TSubclassOf<UCameraShake>  Shake;
		// UNSUPPORTED UClass

		// Extract Scale
		float Scale;
		Scale = Op.Request.field_scale();

		// Extract PlaySpace
		TEnumAsByte<ECameraAnimPlaySpace::Type> PlaySpace;
		PlaySpace = TEnumAsByte<ECameraAnimPlaySpace::Type>(uint8(Op.Request.field_playspace()));

		// Extract UserPlaySpaceRot
		FRotator UserPlaySpaceRot;
		{
			auto& Rotator = Op.Request.field_userplayspacerot();
			UserPlaySpaceRot.Yaw = Rotator.yaw();
			UserPlaySpaceRot.Pitch = Rotator.pitch();
			UserPlaySpaceRot.Roll = Rotator.roll();
		}

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientPlayCameraShake, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ClientPlayCameraShake_Implementation(Shake, Scale, PlaySpace, UserPlaySpaceRot);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaycamerashake>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ClientPlayCameraAnim_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaycameraanim>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientPlayCameraAnim_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientPlayCameraAnim_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientPlayCameraAnim_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract AnimToPlay
		UCameraAnim* AnimToPlay;
		{
			improbable::unreal::UnrealObjectRef ObjectRef = Op.Request.field_animtoplay();
			check(ObjectRef != SpatialConstants::UNRESOLVED_OBJECT_REF);
			if (ObjectRef == SpatialConstants::NULL_OBJECT_REF)
			{
				AnimToPlay = nullptr;
			}
			else
			{
				FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(ObjectRef);
				if (NetGUID.IsValid())
				{
					AnimToPlay = static_cast<UCameraAnim*>(PackageMap->GetObjectFromNetGUID(NetGUID, true));
				}
				else
				{
					UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientPlayCameraAnim_Receiver: AnimToPlay (entity id %lld, offset %d) is not resolved on this worker."),
						*Interop->GetSpatialOS()->GetWorkerId(),
						ObjectRef.entity(),
						ObjectRef.offset());
					return TOptional<improbable::unreal::UnrealObjectRef>(ObjectRef);
				}
			}
		}

		// Extract Scale
		float Scale;
		Scale = Op.Request.field_scale();

		// Extract Rate
		float Rate;
		Rate = Op.Request.field_rate();

		// Extract BlendInTime
		float BlendInTime;
		BlendInTime = Op.Request.field_blendintime();

		// Extract BlendOutTime
		float BlendOutTime;
		BlendOutTime = Op.Request.field_blendouttime();

		// Extract bLoop
		bool bLoop;
		bLoop = Op.Request.field_bloop();

		// Extract bRandomStartTime
		bool bRandomStartTime;
		bRandomStartTime = Op.Request.field_brandomstarttime();

		// Extract Space
		TEnumAsByte<ECameraAnimPlaySpace::Type> Space;
		Space = TEnumAsByte<ECameraAnimPlaySpace::Type>(uint8(Op.Request.field_space()));

		// Extract CustomPlaySpace
		FRotator CustomPlaySpace;
		{
			auto& Rotator = Op.Request.field_customplayspace();
			CustomPlaySpace.Yaw = Rotator.yaw();
			CustomPlaySpace.Pitch = Rotator.pitch();
			CustomPlaySpace.Roll = Rotator.roll();
		}

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientPlayCameraAnim, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ClientPlayCameraAnim_Implementation(AnimToPlay, Scale, Rate, BlendInTime, BlendOutTime, bLoop, bRandomStartTime, Space, CustomPlaySpace);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaycameraanim>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ClientMutePlayer_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientmuteplayer>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientMutePlayer_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientMutePlayer_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientMutePlayer_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract PlayerId
		FUniqueNetIdRepl PlayerId;
		{
			auto& ValueDataStr = Op.Request.field_playerid();
			TArray<uint8> ValueData;
			ValueData.Append((uint8*)ValueDataStr.data(), ValueDataStr.size());
			FMemoryReader ValueDataReader(ValueData);
			bool bSuccess;
			PlayerId.NetSerialize(ValueDataReader, nullptr, bSuccess);
		}

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientMutePlayer, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ClientMutePlayer_Implementation(PlayerId);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientmuteplayer>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ClientMessage_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientmessage>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientMessage_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientMessage_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientMessage_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract S
		FString S;
		S = FString(UTF8_TO_TCHAR(Op.Request.field_s().c_str()));

		// Extract Type
		FName Type;
		Type = FName((Op.Request.field_type()).data());

		// Extract MsgLifeTime
		float MsgLifeTime;
		MsgLifeTime = Op.Request.field_msglifetime();

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientMessage, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ClientMessage_Implementation(S, Type, MsgLifeTime);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientmessage>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ClientIgnoreMoveInput_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientignoremoveinput>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientIgnoreMoveInput_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientIgnoreMoveInput_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientIgnoreMoveInput_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract bIgnore
		bool bIgnore;
		bIgnore = Op.Request.field_bignore();

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientIgnoreMoveInput, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ClientIgnoreMoveInput_Implementation(bIgnore);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientignoremoveinput>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ClientIgnoreLookInput_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientignorelookinput>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientIgnoreLookInput_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientIgnoreLookInput_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientIgnoreLookInput_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract bIgnore
		bool bIgnore;
		bIgnore = Op.Request.field_bignore();

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientIgnoreLookInput, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ClientIgnoreLookInput_Implementation(bIgnore);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientignorelookinput>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ClientGotoState_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientgotostate>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientGotoState_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientGotoState_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientGotoState_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract NewState
		FName NewState;
		NewState = FName((Op.Request.field_newstate()).data());

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientGotoState, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ClientGotoState_Implementation(NewState);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientgotostate>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ClientGameEnded_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientgameended>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientGameEnded_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientGameEnded_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientGameEnded_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract EndGameFocus
		AActor* EndGameFocus;
		{
			improbable::unreal::UnrealObjectRef ObjectRef = Op.Request.field_endgamefocus();
			check(ObjectRef != SpatialConstants::UNRESOLVED_OBJECT_REF);
			if (ObjectRef == SpatialConstants::NULL_OBJECT_REF)
			{
				EndGameFocus = nullptr;
			}
			else
			{
				FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(ObjectRef);
				if (NetGUID.IsValid())
				{
					EndGameFocus = static_cast<AActor*>(PackageMap->GetObjectFromNetGUID(NetGUID, true));
				}
				else
				{
					UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientGameEnded_Receiver: EndGameFocus (entity id %lld, offset %d) is not resolved on this worker."),
						*Interop->GetSpatialOS()->GetWorkerId(),
						ObjectRef.entity(),
						ObjectRef.offset());
					return TOptional<improbable::unreal::UnrealObjectRef>(ObjectRef);
				}
			}
		}

		// Extract bIsWinner
		bool bIsWinner;
		bIsWinner = Op.Request.field_biswinner();

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientGameEnded, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ClientGameEnded_Implementation(EndGameFocus, bIsWinner);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientgameended>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ClientForceGarbageCollection_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientforcegarbagecollection>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientForceGarbageCollection_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientForceGarbageCollection_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientForceGarbageCollection_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientForceGarbageCollection, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ClientForceGarbageCollection_Implementation();

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientforcegarbagecollection>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ClientFlushLevelStreaming_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientflushlevelstreaming>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientFlushLevelStreaming_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientFlushLevelStreaming_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientFlushLevelStreaming_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientFlushLevelStreaming, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ClientFlushLevelStreaming_Implementation();

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientflushlevelstreaming>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ClientEndOnlineSession_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientendonlinesession>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientEndOnlineSession_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientEndOnlineSession_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientEndOnlineSession_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientEndOnlineSession, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ClientEndOnlineSession_Implementation();

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientendonlinesession>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ClientEnableNetworkVoice_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientenablenetworkvoice>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientEnableNetworkVoice_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientEnableNetworkVoice_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientEnableNetworkVoice_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract bEnable
		bool bEnable;
		bEnable = Op.Request.field_benable();

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientEnableNetworkVoice, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ClientEnableNetworkVoice_Implementation(bEnable);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientenablenetworkvoice>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ClientCommitMapChange_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientcommitmapchange>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientCommitMapChange_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientCommitMapChange_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientCommitMapChange_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientCommitMapChange, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ClientCommitMapChange_Implementation();

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientcommitmapchange>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ClientClearCameraLensEffects_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientclearcameralenseffects>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientClearCameraLensEffects_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientClearCameraLensEffects_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientClearCameraLensEffects_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientClearCameraLensEffects, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ClientClearCameraLensEffects_Implementation();

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientclearcameralenseffects>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ClientCapBandwidth_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientcapbandwidth>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientCapBandwidth_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientCapBandwidth_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientCapBandwidth_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract Cap
		int32 Cap;
		Cap = Op.Request.field_cap();

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientCapBandwidth, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ClientCapBandwidth_Implementation(Cap);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientcapbandwidth>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ClientCancelPendingMapChange_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientcancelpendingmapchange>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientCancelPendingMapChange_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientCancelPendingMapChange_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientCancelPendingMapChange_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientCancelPendingMapChange, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ClientCancelPendingMapChange_Implementation();

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientcancelpendingmapchange>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ClientAddTextureStreamingLoc_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientaddtexturestreamingloc>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientAddTextureStreamingLoc_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientAddTextureStreamingLoc_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientAddTextureStreamingLoc_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract InLoc
		FVector InLoc;
		{
			auto& Vector = Op.Request.field_inloc();
			InLoc.X = Vector.x();
			InLoc.Y = Vector.y();
			InLoc.Z = Vector.z();
		}

		// Extract Duration
		float Duration;
		Duration = Op.Request.field_duration();

		// Extract bOverrideLocation
		bool bOverrideLocation;
		bOverrideLocation = Op.Request.field_boverridelocation();

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientAddTextureStreamingLoc, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ClientAddTextureStreamingLoc_Implementation(InLoc, Duration, bOverrideLocation);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientaddtexturestreamingloc>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ClientSetRotation_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetrotation>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientSetRotation_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientSetRotation_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientSetRotation_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract NewRotation
		FRotator NewRotation;
		{
			auto& Rotator = Op.Request.field_newrotation();
			NewRotation.Yaw = Rotator.yaw();
			NewRotation.Pitch = Rotator.pitch();
			NewRotation.Roll = Rotator.roll();
		}

		// Extract bResetCamera
		bool bResetCamera;
		bResetCamera = Op.Request.field_bresetcamera();

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientSetRotation, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ClientSetRotation_Implementation(NewRotation, bResetCamera);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetrotation>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ClientSetLocation_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetlocation>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ClientSetLocation_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ClientSetLocation_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ClientSetLocation_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract NewLocation
		FVector NewLocation;
		{
			auto& Vector = Op.Request.field_newlocation();
			NewLocation.X = Vector.x();
			NewLocation.Y = Vector.y();
			NewLocation.Z = Vector.z();
		}

		// Extract NewRotation
		FRotator NewRotation;
		{
			auto& Rotator = Op.Request.field_newrotation();
			NewRotation.Yaw = Rotator.yaw();
			NewRotation.Pitch = Rotator.pitch();
			NewRotation.Roll = Rotator.roll();
		}

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ClientSetLocation, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ClientSetLocation_Implementation(NewLocation, NewRotation);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetlocation>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ServerViewSelf_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverviewself>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ServerViewSelf_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ServerViewSelf_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ServerViewSelf_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract TransitionParams
		FViewTargetTransitionParams TransitionParams;
		TransitionParams.BlendTime = Op.Request.field_transitionparams_blendtime();
		TransitionParams.BlendFunction = TEnumAsByte<EViewTargetBlendFunction>(uint8(Op.Request.field_transitionparams_blendfunction()));
		TransitionParams.BlendExp = Op.Request.field_transitionparams_blendexp();
		TransitionParams.bLockOutgoing = Op.Request.field_transitionparams_blockoutgoing();
		// Because Unreal will look for a specific bit in the serialization function below, we simply set all bits.
		// We will soon move away from using bunches when receiving Spatial updates.
		if (TransitionParams.bLockOutgoing)
		{
			TransitionParams.bLockOutgoing = 0xFF;
		}

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ServerViewSelf, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ServerViewSelf_Implementation(TransitionParams);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverviewself>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ServerViewPrevPlayer_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverviewprevplayer>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ServerViewPrevPlayer_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ServerViewPrevPlayer_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ServerViewPrevPlayer_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ServerViewPrevPlayer, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ServerViewPrevPlayer_Implementation();

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverviewprevplayer>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ServerViewNextPlayer_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverviewnextplayer>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ServerViewNextPlayer_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ServerViewNextPlayer_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ServerViewNextPlayer_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ServerViewNextPlayer, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ServerViewNextPlayer_Implementation();

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverviewnextplayer>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ServerVerifyViewTarget_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serververifyviewtarget>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ServerVerifyViewTarget_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ServerVerifyViewTarget_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ServerVerifyViewTarget_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ServerVerifyViewTarget, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ServerVerifyViewTarget_Implementation();

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serververifyviewtarget>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ServerUpdateLevelVisibility_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverupdatelevelvisibility>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ServerUpdateLevelVisibility_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ServerUpdateLevelVisibility_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ServerUpdateLevelVisibility_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract PackageName
		FName PackageName;
		PackageName = FName((Op.Request.field_packagename()).data());

		// Extract bIsVisible
		bool bIsVisible;
		bIsVisible = Op.Request.field_bisvisible();

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ServerUpdateLevelVisibility, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ServerUpdateLevelVisibility_Implementation(PackageName, bIsVisible);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverupdatelevelvisibility>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ServerUpdateCamera_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverupdatecamera>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ServerUpdateCamera_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ServerUpdateCamera_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ServerUpdateCamera_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract CamLoc
		FVector_NetQuantize CamLoc;
		{
			auto& Vector = Op.Request.field_camloc();
			CamLoc.X = Vector.x();
			CamLoc.Y = Vector.y();
			CamLoc.Z = Vector.z();
		}

		// Extract CamPitchAndYaw
		int32 CamPitchAndYaw;
		CamPitchAndYaw = Op.Request.field_campitchandyaw();

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ServerUpdateCamera, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ServerUpdateCamera_Implementation(CamLoc, CamPitchAndYaw);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverupdatecamera>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ServerUnmutePlayer_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverunmuteplayer>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ServerUnmutePlayer_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ServerUnmutePlayer_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ServerUnmutePlayer_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract PlayerId
		FUniqueNetIdRepl PlayerId;
		{
			auto& ValueDataStr = Op.Request.field_playerid();
			TArray<uint8> ValueData;
			ValueData.Append((uint8*)ValueDataStr.data(), ValueDataStr.size());
			FMemoryReader ValueDataReader(ValueData);
			bool bSuccess;
			PlayerId.NetSerialize(ValueDataReader, nullptr, bSuccess);
		}

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ServerUnmutePlayer, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ServerUnmutePlayer_Implementation(PlayerId);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverunmuteplayer>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ServerToggleAILogging_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servertoggleailogging>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ServerToggleAILogging_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ServerToggleAILogging_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ServerToggleAILogging_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ServerToggleAILogging, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ServerToggleAILogging_Implementation();

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servertoggleailogging>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ServerShortTimeout_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servershorttimeout>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ServerShortTimeout_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ServerShortTimeout_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ServerShortTimeout_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ServerShortTimeout, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ServerShortTimeout_Implementation();

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servershorttimeout>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ServerSetSpectatorWaiting_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serversetspectatorwaiting>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ServerSetSpectatorWaiting_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ServerSetSpectatorWaiting_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ServerSetSpectatorWaiting_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract bWaiting
		bool bWaiting;
		bWaiting = Op.Request.field_bwaiting();

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ServerSetSpectatorWaiting, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ServerSetSpectatorWaiting_Implementation(bWaiting);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serversetspectatorwaiting>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ServerSetSpectatorLocation_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serversetspectatorlocation>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ServerSetSpectatorLocation_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ServerSetSpectatorLocation_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ServerSetSpectatorLocation_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract NewLoc
		FVector NewLoc;
		{
			auto& Vector = Op.Request.field_newloc();
			NewLoc.X = Vector.x();
			NewLoc.Y = Vector.y();
			NewLoc.Z = Vector.z();
		}

		// Extract NewRot
		FRotator NewRot;
		{
			auto& Rotator = Op.Request.field_newrot();
			NewRot.Yaw = Rotator.yaw();
			NewRot.Pitch = Rotator.pitch();
			NewRot.Roll = Rotator.roll();
		}

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ServerSetSpectatorLocation, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ServerSetSpectatorLocation_Implementation(NewLoc, NewRot);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serversetspectatorlocation>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ServerRestartPlayer_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverrestartplayer>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ServerRestartPlayer_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ServerRestartPlayer_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ServerRestartPlayer_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ServerRestartPlayer, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ServerRestartPlayer_Implementation();

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverrestartplayer>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ServerPause_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverpause>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ServerPause_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ServerPause_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ServerPause_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ServerPause, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ServerPause_Implementation();

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverpause>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ServerNotifyLoadedWorld_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servernotifyloadedworld>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ServerNotifyLoadedWorld_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ServerNotifyLoadedWorld_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ServerNotifyLoadedWorld_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract WorldPackageName
		FName WorldPackageName;
		WorldPackageName = FName((Op.Request.field_worldpackagename()).data());

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ServerNotifyLoadedWorld, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ServerNotifyLoadedWorld_Implementation(WorldPackageName);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servernotifyloadedworld>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ServerMutePlayer_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servermuteplayer>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ServerMutePlayer_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ServerMutePlayer_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ServerMutePlayer_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract PlayerId
		FUniqueNetIdRepl PlayerId;
		{
			auto& ValueDataStr = Op.Request.field_playerid();
			TArray<uint8> ValueData;
			ValueData.Append((uint8*)ValueDataStr.data(), ValueDataStr.size());
			FMemoryReader ValueDataReader(ValueData);
			bool bSuccess;
			PlayerId.NetSerialize(ValueDataReader, nullptr, bSuccess);
		}

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ServerMutePlayer, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ServerMutePlayer_Implementation(PlayerId);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servermuteplayer>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ServerCheckClientPossessionReliable_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servercheckclientpossessionreliable>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ServerCheckClientPossessionReliable_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ServerCheckClientPossessionReliable_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ServerCheckClientPossessionReliable_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ServerCheckClientPossessionReliable, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ServerCheckClientPossessionReliable_Implementation();

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servercheckclientpossessionreliable>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ServerCheckClientPossession_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servercheckclientpossession>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ServerCheckClientPossession_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ServerCheckClientPossession_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ServerCheckClientPossession_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ServerCheckClientPossession, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ServerCheckClientPossession_Implementation();

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servercheckclientpossession>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ServerChangeName_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverchangename>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ServerChangeName_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ServerChangeName_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ServerChangeName_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract S
		FString S;
		S = FString(UTF8_TO_TCHAR(Op.Request.field_s().c_str()));

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ServerChangeName, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ServerChangeName_Implementation(S);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverchangename>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ServerCamera_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servercamera>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ServerCamera_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ServerCamera_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ServerCamera_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract NewMode
		FName NewMode;
		NewMode = FName((Op.Request.field_newmode()).data());

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ServerCamera, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ServerCamera_Implementation(NewMode);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servercamera>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}

void USpatialTypeBinding_PlayerController::ServerAcknowledgePossession_Receiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serveracknowledgepossession>& Op)
{
	auto Receiver = [this, Op]() mutable -> TOptional<improbable::unreal::UnrealObjectRef>
	{
		improbable::unreal::UnrealObjectRef TargetObjectRef{Op.EntityId, Op.Request.target_subobject_offset()};
		FNetworkGUID TargetNetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(TargetObjectRef);
		if (!TargetNetGUID.IsValid())
		{
			UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ServerAcknowledgePossession_Receiver: Target object (entity id %lld, offset %d) is not resolved on this worker."),
				*Interop->GetSpatialOS()->GetWorkerId(),
				TargetObjectRef.entity(),
				TargetObjectRef.offset());
			return TOptional<improbable::unreal::UnrealObjectRef>(TargetObjectRef);
		}
		UObject* TargetObjectUntyped = PackageMap->GetObjectFromNetGUID(TargetNetGUID, false);
		APlayerController* TargetObject = Cast<APlayerController>(TargetObjectUntyped);
		checkf(TargetObjectUntyped, TEXT("%s: ServerAcknowledgePossession_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) does not correspond to a UObject."),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString());
		checkf(TargetObject, TEXT("%s: ServerAcknowledgePossession_Receiver: Object Ref (entity: %llu, offset: %u) (NetGUID %s) is the wrong type. Name: %s"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset(),
			*TargetNetGUID.ToString(),
			*TargetObjectUntyped->GetName());

		// Extract P
		APawn* P;
		{
			improbable::unreal::UnrealObjectRef ObjectRef = Op.Request.field_p();
			check(ObjectRef != SpatialConstants::UNRESOLVED_OBJECT_REF);
			if (ObjectRef == SpatialConstants::NULL_OBJECT_REF)
			{
				P = nullptr;
			}
			else
			{
				FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(ObjectRef);
				if (NetGUID.IsValid())
				{
					P = static_cast<APawn*>(PackageMap->GetObjectFromNetGUID(NetGUID, true));
				}
				else
				{
					UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: ServerAcknowledgePossession_Receiver: P (entity id %lld, offset %d) is not resolved on this worker."),
						*Interop->GetSpatialOS()->GetWorkerId(),
						ObjectRef.entity(),
						ObjectRef.offset());
					return TOptional<improbable::unreal::UnrealObjectRef>(ObjectRef);
				}
			}
		}

		// Call implementation.
		UE_LOG(LogSpatialOSInterop, Verbose, TEXT("%s: Received RPC: ServerAcknowledgePossession, target: %s (entity: %llu, offset: %u)"),
			*Interop->GetSpatialOS()->GetWorkerId(),
			*TargetObject->GetName(),
			TargetObjectRef.entity(),
			TargetObjectRef.offset());
		TargetObject->ServerAcknowledgePossession_Implementation(P);

		// Send command response.
		TSharedPtr<worker::Connection> Connection = Interop->GetSpatialOS()->GetConnection().Pin();
		Connection->SendCommandResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serveracknowledgepossession>(Op.RequestId, {});
		return {};
	};
	Interop->SendCommandResponse(Receiver);
}
