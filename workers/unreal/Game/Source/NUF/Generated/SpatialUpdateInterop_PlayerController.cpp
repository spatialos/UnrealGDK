// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// Note that this file has been generated automatically

#include "SpatialUpdateInterop_PlayerController.h"
#include "SpatialOS.h"
#include "Engine.h"
#include "SpatialActorChannel.h"
#include "Utils/BunchReader.h"

namespace {

void ApplyUpdateToSpatial_SingleClient_PlayerController(FArchive& Reader, int32 Handle, UProperty* Property, UPackageMap* PackageMap, improbable::unreal::UnrealPlayerControllerSingleClientReplicatedData::Update& Update)
{
	switch (Handle)
	{
		case 18: // field_targetviewrotation
		{
			FRotator Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, PackageMap, &Value);

			Update.set_field_targetviewrotation(improbable::unreal::UnrealFRotator(Value.Yaw, Value.Pitch, Value.Roll));
			break;
		}
		case 19: // field_spawnlocation
		{
			FVector Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, PackageMap, &Value);

			Update.set_field_spawnlocation(improbable::Vector3f(Value.X, Value.Y, Value.Z));
			break;
		}
	default:
		checkf(false, TEXT("Unknown replication handle %d encountered when creating a SpatialOS update."));
		break;
	}
}

void ReceiveUpdateFromSpatial_SingleClient_PlayerController(USpatialUpdateInterop* UpdateInterop, UPackageMap* PackageMap, const worker::ComponentUpdateOp<improbable::unreal::UnrealPlayerControllerSingleClientReplicatedData>& Op)
{
	FNetBitWriter OutputWriter(nullptr, 0); 
	auto& HandleToPropertyMap = GetHandlePropertyMap_PlayerController();
	USpatialActorChannel* ActorChannel = UpdateInterop->GetClientActorChannel(Op.EntityId);
	if (!ActorChannel)
	{
		return;
	}
	ConditionMapFilter ConditionMap(ActorChannel);
	if (!Op.Update.field_targetviewrotation().empty())
	{
		// field_targetviewrotation
		uint32 Handle = 18;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			OutputWriter.SerializeIntPacked(Handle);

			FRotator Value;
			check(Data.Property->ElementSize == sizeof(Value));

			auto& Rotator = *(Op.Update.field_targetviewrotation().data());
			Value.Yaw = Rotator.yaw();
			Value.Pitch = Rotator.pitch();
			Value.Roll = Rotator.roll();

			Data.Property->NetSerializeItem(OutputWriter, PackageMap, &Value);
			UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
		}
	}
	if (!Op.Update.field_spawnlocation().empty())
	{
		// field_spawnlocation
		uint32 Handle = 19;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			OutputWriter.SerializeIntPacked(Handle);

			FVector Value;
			check(Data.Property->ElementSize == sizeof(Value));

			auto& Vector = *(Op.Update.field_spawnlocation().data());
			Value.X = Vector.x();
			Value.Y = Vector.y();
			Value.Z = Vector.z();

			Data.Property->NetSerializeItem(OutputWriter, PackageMap, &Value);
			UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
		}
	}
	UpdateInterop->ReceiveSpatialUpdate(ActorChannel, OutputWriter);
}

void ApplyUpdateToSpatial_MultiClient_PlayerController(FArchive& Reader, int32 Handle, UProperty* Property, UPackageMap* PackageMap, improbable::unreal::UnrealPlayerControllerMultiClientReplicatedData::Update& Update)
{
	switch (Handle)
	{
		case 1: // field_bhidden
		{
			uint8 Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, PackageMap, &Value);

			Update.set_field_bhidden(Value != 0);
			break;
		}
		case 2: // field_breplicatemovement
		{
			uint8 Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, PackageMap, &Value);

			Update.set_field_breplicatemovement(Value != 0);
			break;
		}
		case 3: // field_btearoff
		{
			uint8 Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, PackageMap, &Value);

			Update.set_field_btearoff(Value != 0);
			break;
		}
		case 4: // field_remoterole
		{
			TEnumAsByte<ENetRole> Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, PackageMap, &Value);

			Update.set_field_remoterole(uint32_t(Value));
			break;
		}
		// case 5: - Owner is an object reference, skipping.
		case 6: // field_replicatedmovement
		{
			FRepMovement Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, PackageMap, &Value);

			TArray<uint8> ValueData;
			FMemoryWriter ValueDataWriter(ValueData);
			bool Success;
			Value.NetSerialize(ValueDataWriter, nullptr, Success);
			Update.set_field_replicatedmovement(std::string((char*)ValueData.GetData(), ValueData.Num()));
			break;
		}
		// case 7: - AttachParent is an object reference, skipping.
		case 8: // field_attachmentreplication_locationoffset
		{
			FVector_NetQuantize100 Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, PackageMap, &Value);

			Update.set_field_attachmentreplication_locationoffset(improbable::Vector3f(Value.X, Value.Y, Value.Z));
			break;
		}
		case 9: // field_attachmentreplication_relativescale3d
		{
			FVector_NetQuantize100 Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, PackageMap, &Value);

			Update.set_field_attachmentreplication_relativescale3d(improbable::Vector3f(Value.X, Value.Y, Value.Z));
			break;
		}
		case 10: // field_attachmentreplication_rotationoffset
		{
			FRotator Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, PackageMap, &Value);

			Update.set_field_attachmentreplication_rotationoffset(improbable::unreal::UnrealFRotator(Value.Yaw, Value.Pitch, Value.Roll));
			break;
		}
		case 11: // field_attachmentreplication_attachsocket
		{
			FName Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, PackageMap, &Value);

			Update.set_field_attachmentreplication_attachsocket(TCHAR_TO_UTF8(*Value.ToString()));
			break;
		}
		// case 12: - AttachComponent is an object reference, skipping.
		case 13: // field_role
		{
			TEnumAsByte<ENetRole> Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, PackageMap, &Value);

			Update.set_field_role(uint32_t(Value));
			break;
		}
		case 14: // field_bcanbedamaged
		{
			uint8 Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, PackageMap, &Value);

			Update.set_field_bcanbedamaged(Value != 0);
			break;
		}
		// case 15: - Instigator is an object reference, skipping.
		// case 16: - Pawn is an object reference, skipping.
		// case 17: - PlayerState is an object reference, skipping.
	default:
		checkf(false, TEXT("Unknown replication handle %d encountered when creating a SpatialOS update."));
		break;
	}
}

void ReceiveUpdateFromSpatial_MultiClient_PlayerController(USpatialUpdateInterop* UpdateInterop, UPackageMap* PackageMap, const worker::ComponentUpdateOp<improbable::unreal::UnrealPlayerControllerMultiClientReplicatedData>& Op)
{
	FNetBitWriter OutputWriter(nullptr, 0); 
	auto& HandleToPropertyMap = GetHandlePropertyMap_PlayerController();
	USpatialActorChannel* ActorChannel = UpdateInterop->GetClientActorChannel(Op.EntityId);
	if (!ActorChannel)
	{
		return;
	}
	ConditionMapFilter ConditionMap(ActorChannel);
	if (!Op.Update.field_bhidden().empty())
	{
		// field_bhidden
		uint32 Handle = 1;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			OutputWriter.SerializeIntPacked(Handle);

			uint8 Value;
			check(Data.Property->ElementSize == sizeof(Value));

			Value = *(Op.Update.field_bhidden().data());

			Data.Property->NetSerializeItem(OutputWriter, PackageMap, &Value);
			UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
		}
	}
	if (!Op.Update.field_breplicatemovement().empty())
	{
		// field_breplicatemovement
		uint32 Handle = 2;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			OutputWriter.SerializeIntPacked(Handle);

			uint8 Value;
			check(Data.Property->ElementSize == sizeof(Value));

			Value = *(Op.Update.field_breplicatemovement().data());

			Data.Property->NetSerializeItem(OutputWriter, PackageMap, &Value);
			UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
		}
	}
	if (!Op.Update.field_btearoff().empty())
	{
		// field_btearoff
		uint32 Handle = 3;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			OutputWriter.SerializeIntPacked(Handle);

			uint8 Value;
			check(Data.Property->ElementSize == sizeof(Value));

			Value = *(Op.Update.field_btearoff().data());

			Data.Property->NetSerializeItem(OutputWriter, PackageMap, &Value);
			UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
		}
	}
	if (!Op.Update.field_remoterole().empty())
	{
		// field_remoterole
		uint32 Handle = 4;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			OutputWriter.SerializeIntPacked(Handle);

			TEnumAsByte<ENetRole> Value;
			check(Data.Property->ElementSize == sizeof(Value));

			// Byte properties are weird, because they can also be an enum in the form TEnumAsByte<...>.
			// Therefore, the code generator needs to cast to either TEnumAsByte<...> or uint8. However,
			// as TEnumAsByte<...> only has a uint8 constructor, we need to cast the SpatialOS value into
			// uint8 first, which causes "uint8(uint8(...))" to be generated for non enum bytes.
			Value = TEnumAsByte<ENetRole>(uint8(*(Op.Update.field_remoterole().data())));

			Data.Property->NetSerializeItem(OutputWriter, PackageMap, &Value);
			UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
		}
	}
	if (!Op.Update.field_owner().empty())
	{
		// field_owner
		uint32 Handle = 5;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			OutputWriter.SerializeIntPacked(Handle);

			AActor* Value;
			check(Data.Property->ElementSize == sizeof(Value));

			// UNSUPPORTED ObjectProperty - Value *(Op.Update.field_owner().data());

			Data.Property->NetSerializeItem(OutputWriter, PackageMap, &Value);
			UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
		}
	}
	if (!Op.Update.field_replicatedmovement().empty())
	{
		// field_replicatedmovement
		uint32 Handle = 6;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			OutputWriter.SerializeIntPacked(Handle);

			FRepMovement Value;
			check(Data.Property->ElementSize == sizeof(Value));

			auto& ValueDataStr = *(Op.Update.field_replicatedmovement().data());
			TArray<uint8> ValueData;
			ValueData.Append((uint8*)ValueDataStr.data(), ValueDataStr.size());
			FMemoryReader ValueDataReader(ValueData);
			bool bSuccess;
			Value.NetSerialize(ValueDataReader, nullptr, bSuccess);

			Data.Property->NetSerializeItem(OutputWriter, PackageMap, &Value);
			UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
		}
	}
	if (!Op.Update.field_attachmentreplication_attachparent().empty())
	{
		// field_attachmentreplication_attachparent
		uint32 Handle = 7;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			OutputWriter.SerializeIntPacked(Handle);

			AActor* Value;
			check(Data.Property->ElementSize == sizeof(Value));

			// UNSUPPORTED ObjectProperty - Value *(Op.Update.field_attachmentreplication_attachparent().data());

			Data.Property->NetSerializeItem(OutputWriter, PackageMap, &Value);
			UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
		}
	}
	if (!Op.Update.field_attachmentreplication_locationoffset().empty())
	{
		// field_attachmentreplication_locationoffset
		uint32 Handle = 8;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			OutputWriter.SerializeIntPacked(Handle);

			FVector_NetQuantize100 Value;
			check(Data.Property->ElementSize == sizeof(Value));

			auto& Vector = *(Op.Update.field_attachmentreplication_locationoffset().data());
			Value.X = Vector.x();
			Value.Y = Vector.y();
			Value.Z = Vector.z();

			Data.Property->NetSerializeItem(OutputWriter, PackageMap, &Value);
			UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
		}
	}
	if (!Op.Update.field_attachmentreplication_relativescale3d().empty())
	{
		// field_attachmentreplication_relativescale3d
		uint32 Handle = 9;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			OutputWriter.SerializeIntPacked(Handle);

			FVector_NetQuantize100 Value;
			check(Data.Property->ElementSize == sizeof(Value));

			auto& Vector = *(Op.Update.field_attachmentreplication_relativescale3d().data());
			Value.X = Vector.x();
			Value.Y = Vector.y();
			Value.Z = Vector.z();

			Data.Property->NetSerializeItem(OutputWriter, PackageMap, &Value);
			UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
		}
	}
	if (!Op.Update.field_attachmentreplication_rotationoffset().empty())
	{
		// field_attachmentreplication_rotationoffset
		uint32 Handle = 10;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			OutputWriter.SerializeIntPacked(Handle);

			FRotator Value;
			check(Data.Property->ElementSize == sizeof(Value));

			auto& Rotator = *(Op.Update.field_attachmentreplication_rotationoffset().data());
			Value.Yaw = Rotator.yaw();
			Value.Pitch = Rotator.pitch();
			Value.Roll = Rotator.roll();

			Data.Property->NetSerializeItem(OutputWriter, PackageMap, &Value);
			UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
		}
	}
	if (!Op.Update.field_attachmentreplication_attachsocket().empty())
	{
		// field_attachmentreplication_attachsocket
		uint32 Handle = 11;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			OutputWriter.SerializeIntPacked(Handle);

			FName Value;
			check(Data.Property->ElementSize == sizeof(Value));

			Value = FName((*(Op.Update.field_attachmentreplication_attachsocket().data())).data());

			Data.Property->NetSerializeItem(OutputWriter, PackageMap, &Value);
			UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
		}
	}
	if (!Op.Update.field_attachmentreplication_attachcomponent().empty())
	{
		// field_attachmentreplication_attachcomponent
		uint32 Handle = 12;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			OutputWriter.SerializeIntPacked(Handle);

			USceneComponent* Value;
			check(Data.Property->ElementSize == sizeof(Value));

			// UNSUPPORTED ObjectProperty - Value *(Op.Update.field_attachmentreplication_attachcomponent().data());

			Data.Property->NetSerializeItem(OutputWriter, PackageMap, &Value);
			UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
		}
	}
	if (!Op.Update.field_role().empty())
	{
		// field_role
		uint32 Handle = 13;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			OutputWriter.SerializeIntPacked(Handle);

			TEnumAsByte<ENetRole> Value;
			check(Data.Property->ElementSize == sizeof(Value));

			// Byte properties are weird, because they can also be an enum in the form TEnumAsByte<...>.
			// Therefore, the code generator needs to cast to either TEnumAsByte<...> or uint8. However,
			// as TEnumAsByte<...> only has a uint8 constructor, we need to cast the SpatialOS value into
			// uint8 first, which causes "uint8(uint8(...))" to be generated for non enum bytes.
			Value = TEnumAsByte<ENetRole>(uint8(*(Op.Update.field_role().data())));

			Data.Property->NetSerializeItem(OutputWriter, PackageMap, &Value);
			UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
		}
	}
	if (!Op.Update.field_bcanbedamaged().empty())
	{
		// field_bcanbedamaged
		uint32 Handle = 14;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			OutputWriter.SerializeIntPacked(Handle);

			uint8 Value;
			check(Data.Property->ElementSize == sizeof(Value));

			Value = *(Op.Update.field_bcanbedamaged().data());

			Data.Property->NetSerializeItem(OutputWriter, PackageMap, &Value);
			UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
		}
	}
	if (!Op.Update.field_instigator().empty())
	{
		// field_instigator
		uint32 Handle = 15;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			OutputWriter.SerializeIntPacked(Handle);

			APawn* Value;
			check(Data.Property->ElementSize == sizeof(Value));

			// UNSUPPORTED ObjectProperty - Value *(Op.Update.field_instigator().data());

			Data.Property->NetSerializeItem(OutputWriter, PackageMap, &Value);
			UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
		}
	}
	if (!Op.Update.field_pawn().empty())
	{
		// field_pawn
		uint32 Handle = 16;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			OutputWriter.SerializeIntPacked(Handle);

			APawn* Value;
			check(Data.Property->ElementSize == sizeof(Value));

			// UNSUPPORTED ObjectProperty - Value *(Op.Update.field_pawn().data());

			Data.Property->NetSerializeItem(OutputWriter, PackageMap, &Value);
			UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
		}
	}
	if (!Op.Update.field_playerstate().empty())
	{
		// field_playerstate
		uint32 Handle = 17;
		const FRepHandleData& Data = HandleToPropertyMap[Handle];
		if (ConditionMap.IsRelevant(Data.Condition))
		{
			OutputWriter.SerializeIntPacked(Handle);

			APlayerState* Value;
			check(Data.Property->ElementSize == sizeof(Value));

			// UNSUPPORTED ObjectProperty - Value *(Op.Update.field_playerstate().data());

			Data.Property->NetSerializeItem(OutputWriter, PackageMap, &Value);
			UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
		}
	}
	UpdateInterop->ReceiveSpatialUpdate(ActorChannel, OutputWriter);
}

// RPC handler functions
void OnServerStartedVisualLoggerHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_UBOOL(bIsLogging);
}
void ClientWasKickedHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UTextProperty, KickReason);
}
void ClientVoiceHandshakeCompleteHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
{
	FFrame& Stack = *RPCFrame;
}
void ClientUpdateLevelStreamingStatusHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UNameProperty, PackageName);
	P_GET_UBOOL(bNewShouldBeLoaded);
	P_GET_UBOOL(bNewShouldBeVisible);
	P_GET_UBOOL(bNewShouldBlockOnLoad);
	P_GET_PROPERTY(UIntProperty, LODIndex);
}
void ClientUnmutePlayerHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_STRUCT(FUniqueNetIdRepl, PlayerId)
}
void ClientTravelInternalHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UStrProperty, URL);
	P_GET_PROPERTY(UByteProperty, TravelType);
	P_GET_UBOOL(bSeamless);
	P_GET_STRUCT(FGuid, MapPackageGuid)
}
void ClientTeamMessageHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(APlayerState, SenderPlayerState);
	P_GET_PROPERTY(UStrProperty, S);
	P_GET_PROPERTY(UNameProperty, Type);
	P_GET_PROPERTY(UFloatProperty, MsgLifeTime);
}
void ClientStopForceFeedbackHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(UForceFeedbackEffect, ForceFeedbackEffect);
	P_GET_PROPERTY(UNameProperty, Tag);
}
void ClientStopCameraShakeHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(UClass, Shake);
	P_GET_UBOOL(bImmediately);
}
void ClientStopCameraAnimHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(UCameraAnim, AnimToStop);
}
void ClientStartOnlineSessionHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
{
	FFrame& Stack = *RPCFrame;
}
void ClientSpawnCameraLensEffectHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(UClass, LensEffectEmitterClass);
}
void ClientSetViewTargetHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(AActor, A);
	P_GET_STRUCT(FViewTargetTransitionParams, TransitionParams)
}
void ClientSetSpectatorWaitingHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_UBOOL(bWaiting);
}
void ClientSetHUDHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(UClass, NewHUDClass);
}
void ClientSetForceMipLevelsToBeResidentHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(UMaterialInterface, Material);
	P_GET_PROPERTY(UFloatProperty, ForceDuration);
	P_GET_PROPERTY(UIntProperty, CinematicTextureGroups);
}
void ClientSetCinematicModeHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_UBOOL(bInCinematicMode);
	P_GET_UBOOL(bAffectsMovement);
	P_GET_UBOOL(bAffectsTurning);
	P_GET_UBOOL(bAffectsHUD);
}
void ClientSetCameraModeHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UNameProperty, NewCamMode);
}
void ClientSetCameraFadeHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_UBOOL(bEnableFading);
	P_GET_STRUCT(FColor, FadeColor)
	P_GET_STRUCT(FVector2D, FadeAlpha)
	P_GET_PROPERTY(UFloatProperty, FadeTime);
	P_GET_UBOOL(bFadeAudio);
}
void ClientSetBlockOnAsyncLoadingHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
{
	FFrame& Stack = *RPCFrame;
}
void ClientReturnToMainMenuHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UStrProperty, ReturnReason);
}
void ClientRetryClientRestartHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(APawn, NewPawn);
}
void ClientRestartHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(APawn, NewPawn);
}
void ClientResetHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
{
	FFrame& Stack = *RPCFrame;
}
void ClientRepObjRefHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(UObject, Object);
}
void ClientReceiveLocalizedMessageHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(UClass, Message);
	P_GET_PROPERTY(UIntProperty, Switch);
	P_GET_OBJECT(APlayerState, RelatedPlayerState_1);
	P_GET_OBJECT(APlayerState, RelatedPlayerState_2);
	P_GET_OBJECT(UObject, OptionalObject);
}
void ClientPrestreamTexturesHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(AActor, ForcedActor);
	P_GET_PROPERTY(UFloatProperty, ForceDuration);
	P_GET_UBOOL(bEnableStreaming);
	P_GET_PROPERTY(UIntProperty, CinematicTextureGroups);
}
void ClientPrepareMapChangeHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UNameProperty, LevelName);
	P_GET_UBOOL(bFirst);
	P_GET_UBOOL(bLast);
}
void ClientPlaySoundAtLocationHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(USoundBase, Sound);
	P_GET_STRUCT(FVector, Location)
	P_GET_PROPERTY(UFloatProperty, VolumeMultiplier);
	P_GET_PROPERTY(UFloatProperty, PitchMultiplier);
}
void ClientPlaySoundHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(USoundBase, Sound);
	P_GET_PROPERTY(UFloatProperty, VolumeMultiplier);
	P_GET_PROPERTY(UFloatProperty, PitchMultiplier);
}
void ClientPlayForceFeedbackHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(UForceFeedbackEffect, ForceFeedbackEffect);
	P_GET_UBOOL(bLooping);
	P_GET_PROPERTY(UNameProperty, Tag);
}
void ClientPlayCameraShakeHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(UClass, Shake);
	P_GET_PROPERTY(UFloatProperty, Scale);
	P_GET_PROPERTY(UByteProperty, PlaySpace);
	P_GET_STRUCT(FRotator, UserPlaySpaceRot)
}
void ClientPlayCameraAnimHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
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
}
void ClientMutePlayerHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_STRUCT(FUniqueNetIdRepl, PlayerId)
}
void ClientMessageHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UStrProperty, S);
	P_GET_PROPERTY(UNameProperty, Type);
	P_GET_PROPERTY(UFloatProperty, MsgLifeTime);
}
void ClientIgnoreMoveInputHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_UBOOL(bIgnore);
}
void ClientIgnoreLookInputHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_UBOOL(bIgnore);
}
void ClientGotoStateHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UNameProperty, NewState);
}
void ClientGameEndedHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(AActor, EndGameFocus);
	P_GET_UBOOL(bIsWinner);
}
void ClientForceGarbageCollectionHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
{
	FFrame& Stack = *RPCFrame;
}
void ClientFlushLevelStreamingHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
{
	FFrame& Stack = *RPCFrame;
}
void ClientEndOnlineSessionHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
{
	FFrame& Stack = *RPCFrame;
}
void ClientEnableNetworkVoiceHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_UBOOL(bEnable);
}
void ClientCommitMapChangeHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
{
	FFrame& Stack = *RPCFrame;
}
void ClientClearCameraLensEffectsHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
{
	FFrame& Stack = *RPCFrame;
}
void ClientCapBandwidthHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UIntProperty, Cap);
}
void ClientCancelPendingMapChangeHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
{
	FFrame& Stack = *RPCFrame;
}
void ClientAddTextureStreamingLocHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_STRUCT(FVector, InLoc)
	P_GET_PROPERTY(UFloatProperty, Duration);
	P_GET_UBOOL(bOverrideLocation);
}
void ClientSetRotationHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_STRUCT(FRotator, NewRotation)
	P_GET_UBOOL(bResetCamera);
}
void ClientSetLocationHandler(worker::Connection* Connection, struct FFrame* RPCFrame, worker::EntityId Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_STRUCT(FVector, NewLocation)
	P_GET_STRUCT(FRotator, NewRotation)
}
} // ::

void FSpatialTypeBinding_PlayerController::Init(USpatialUpdateInterop* UpdateInterop, UPackageMap* PackageMap)
{
	RPCToHandlerMap.Emplace("OnServerStartedVisualLogger", &OnServerStartedVisualLoggerHandler);
	RPCToHandlerMap.Emplace("ClientWasKicked", &ClientWasKickedHandler);
	RPCToHandlerMap.Emplace("ClientVoiceHandshakeComplete", &ClientVoiceHandshakeCompleteHandler);
	RPCToHandlerMap.Emplace("ClientUpdateLevelStreamingStatus", &ClientUpdateLevelStreamingStatusHandler);
	RPCToHandlerMap.Emplace("ClientUnmutePlayer", &ClientUnmutePlayerHandler);
	RPCToHandlerMap.Emplace("ClientTravelInternal", &ClientTravelInternalHandler);
	RPCToHandlerMap.Emplace("ClientTeamMessage", &ClientTeamMessageHandler);
	RPCToHandlerMap.Emplace("ClientStopForceFeedback", &ClientStopForceFeedbackHandler);
	RPCToHandlerMap.Emplace("ClientStopCameraShake", &ClientStopCameraShakeHandler);
	RPCToHandlerMap.Emplace("ClientStopCameraAnim", &ClientStopCameraAnimHandler);
	RPCToHandlerMap.Emplace("ClientStartOnlineSession", &ClientStartOnlineSessionHandler);
	RPCToHandlerMap.Emplace("ClientSpawnCameraLensEffect", &ClientSpawnCameraLensEffectHandler);
	RPCToHandlerMap.Emplace("ClientSetViewTarget", &ClientSetViewTargetHandler);
	RPCToHandlerMap.Emplace("ClientSetSpectatorWaiting", &ClientSetSpectatorWaitingHandler);
	RPCToHandlerMap.Emplace("ClientSetHUD", &ClientSetHUDHandler);
	RPCToHandlerMap.Emplace("ClientSetForceMipLevelsToBeResident", &ClientSetForceMipLevelsToBeResidentHandler);
	RPCToHandlerMap.Emplace("ClientSetCinematicMode", &ClientSetCinematicModeHandler);
	RPCToHandlerMap.Emplace("ClientSetCameraMode", &ClientSetCameraModeHandler);
	RPCToHandlerMap.Emplace("ClientSetCameraFade", &ClientSetCameraFadeHandler);
	RPCToHandlerMap.Emplace("ClientSetBlockOnAsyncLoading", &ClientSetBlockOnAsyncLoadingHandler);
	RPCToHandlerMap.Emplace("ClientReturnToMainMenu", &ClientReturnToMainMenuHandler);
	RPCToHandlerMap.Emplace("ClientRetryClientRestart", &ClientRetryClientRestartHandler);
	RPCToHandlerMap.Emplace("ClientRestart", &ClientRestartHandler);
	RPCToHandlerMap.Emplace("ClientReset", &ClientResetHandler);
	RPCToHandlerMap.Emplace("ClientRepObjRef", &ClientRepObjRefHandler);
	RPCToHandlerMap.Emplace("ClientReceiveLocalizedMessage", &ClientReceiveLocalizedMessageHandler);
	RPCToHandlerMap.Emplace("ClientPrestreamTextures", &ClientPrestreamTexturesHandler);
	RPCToHandlerMap.Emplace("ClientPrepareMapChange", &ClientPrepareMapChangeHandler);
	RPCToHandlerMap.Emplace("ClientPlaySoundAtLocation", &ClientPlaySoundAtLocationHandler);
	RPCToHandlerMap.Emplace("ClientPlaySound", &ClientPlaySoundHandler);
	RPCToHandlerMap.Emplace("ClientPlayForceFeedback", &ClientPlayForceFeedbackHandler);
	RPCToHandlerMap.Emplace("ClientPlayCameraShake", &ClientPlayCameraShakeHandler);
	RPCToHandlerMap.Emplace("ClientPlayCameraAnim", &ClientPlayCameraAnimHandler);
	RPCToHandlerMap.Emplace("ClientMutePlayer", &ClientMutePlayerHandler);
	RPCToHandlerMap.Emplace("ClientMessage", &ClientMessageHandler);
	RPCToHandlerMap.Emplace("ClientIgnoreMoveInput", &ClientIgnoreMoveInputHandler);
	RPCToHandlerMap.Emplace("ClientIgnoreLookInput", &ClientIgnoreLookInputHandler);
	RPCToHandlerMap.Emplace("ClientGotoState", &ClientGotoStateHandler);
	RPCToHandlerMap.Emplace("ClientGameEnded", &ClientGameEndedHandler);
	RPCToHandlerMap.Emplace("ClientForceGarbageCollection", &ClientForceGarbageCollectionHandler);
	RPCToHandlerMap.Emplace("ClientFlushLevelStreaming", &ClientFlushLevelStreamingHandler);
	RPCToHandlerMap.Emplace("ClientEndOnlineSession", &ClientEndOnlineSessionHandler);
	RPCToHandlerMap.Emplace("ClientEnableNetworkVoice", &ClientEnableNetworkVoiceHandler);
	RPCToHandlerMap.Emplace("ClientCommitMapChange", &ClientCommitMapChangeHandler);
	RPCToHandlerMap.Emplace("ClientClearCameraLensEffects", &ClientClearCameraLensEffectsHandler);
	RPCToHandlerMap.Emplace("ClientCapBandwidth", &ClientCapBandwidthHandler);
	RPCToHandlerMap.Emplace("ClientCancelPendingMapChange", &ClientCancelPendingMapChangeHandler);
	RPCToHandlerMap.Emplace("ClientAddTextureStreamingLoc", &ClientAddTextureStreamingLocHandler);
	RPCToHandlerMap.Emplace("ClientSetRotation", &ClientSetRotationHandler);
	RPCToHandlerMap.Emplace("ClientSetLocation", &ClientSetLocationHandler);
}

const FRepHandlePropertyMap& GetHandlePropertyMap_PlayerController()
{
	static FRepHandlePropertyMap* HandleToPropertyMapData = nullptr;
	if (HandleToPropertyMapData == nullptr)
	{
		UClass* Class = FindObject<UClass>(ANY_PACKAGE, TEXT("PlayerController"));
		HandleToPropertyMapData = new FRepHandlePropertyMap();
		auto& HandleToPropertyMap = *HandleToPropertyMapData;
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
	return *HandleToPropertyMapData;
}

void FSpatialTypeBinding_PlayerController::BindToView()
{
	TSharedPtr<worker::View> View = UpdateInterop->GetSpatialOS()->GetView().Pin();
	SingleClientCallback = View->OnComponentUpdate<improbable::unreal::UnrealPlayerControllerSingleClientReplicatedData>([this](
		const worker::ComponentUpdateOp<improbable::unreal::UnrealPlayerControllerSingleClientReplicatedData>& Op)
	{
		ReceiveUpdateFromSpatial_SingleClient_PlayerController(UpdateInterop, PackageMap, Op);
	});
	MultiClientCallback = View->OnComponentUpdate<improbable::unreal::UnrealPlayerControllerMultiClientReplicatedData>([this](
		const worker::ComponentUpdateOp<improbable::unreal::UnrealPlayerControllerMultiClientReplicatedData>& Op)
	{
		ReceiveUpdateFromSpatial_MultiClient_PlayerController(UpdateInterop, PackageMap, Op);
	});
}

void FSpatialTypeBinding_PlayerController::UnbindFromView()
{
	TSharedPtr<worker::View> View = UpdateInterop->GetSpatialOS()->GetView().Pin();
	View->Remove(SingleClientCallback);
	View->Remove(MultiClientCallback);
}

worker::ComponentId FSpatialTypeBinding_PlayerController::GetReplicatedGroupComponentId(EReplicatedPropertyGroup Group) const
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

void FSpatialTypeBinding_PlayerController::SendComponentUpdates(FOutBunch* BunchPtr, const worker::EntityId& EntityId) const
{
	// Build SpatialOS updates.
	improbable::unreal::UnrealPlayerControllerSingleClientReplicatedData::Update SingleClientUpdate;
	bool SingleClientUpdateChanged = false;
	improbable::unreal::UnrealPlayerControllerMultiClientReplicatedData::Update MultiClientUpdate;
	bool MultiClientUpdateChanged = false;

	// Read bunch and build up SpatialOS component updates.
	auto& PropertyMap = GetHandlePropertyMap_PlayerController();
	FBunchReader BunchReader(BunchPtr->GetData(), BunchPtr->GetNumBits());
	FBunchReader::RepDataHandler RepDataHandler = [&](FNetBitReader& Reader, UPackageMap* PackageMap, int32 Handle, UProperty* Property) -> bool
	{
		// TODO: We can't parse UObjects or FNames here as we have no package map.
		if (Property->IsA(UObjectPropertyBase::StaticClass()) || Property->IsA(UNameProperty::StaticClass()))
		{
		return false;
		}
		
		// Filter Role (13) and RemoteRole (4)
		if (Handle == 13 || Handle == 4)
		{
		return false;
		}
		
		auto& Data = PropertyMap[Handle];
		UE_LOG(LogTemp, Log, TEXT("-> Handle: %d Property %s"), Handle, *Property->GetName());
		
		switch (GetGroupFromCondition(Data.Condition))
		{
		case GROUP_SingleClient:
			ApplyUpdateToSpatial_SingleClient_PlayerController(Reader, Handle, Property, PackageMap, SingleClientUpdate);
			SingleClientUpdateChanged = true;
			break;
		case GROUP_MultiClient:
			ApplyUpdateToSpatial_MultiClient_PlayerController(Reader, Handle, Property, PackageMap, MultiClientUpdate);
			MultiClientUpdateChanged = true;
			break;
		}
		return true;
	};
	BunchReader.Parse(true, PackageMap, PropertyMap, RepDataHandler);

	// Send SpatialOS update.
	TSharedPtr<worker::Connection> Connection = UpdateInterop->GetSpatialOS()->GetConnection().Pin();
	if (SingleClientUpdateChanged)
	{
		Connection->SendComponentUpdate<improbable::unreal::UnrealPlayerControllerSingleClientReplicatedData>(EntityId, SingleClientUpdate);
	}
	if (MultiClientUpdateChanged)
	{
		Connection->SendComponentUpdate<improbable::unreal::UnrealPlayerControllerMultiClientReplicatedData>(EntityId, MultiClientUpdate);
	}
}

void FSpatialTypeBinding_PlayerController::SendRPCCommand(UFunction* Function, FFrame* RPCFrame, worker::EntityId Target) const
{
	TSharedPtr<worker::Connection> Connection = UpdateInterop->GetSpatialOS()->GetConnection().Pin();
	RPCToHandlerMap[Function->GetName()](Connection.Get(), RPCFrame, Target);
}
