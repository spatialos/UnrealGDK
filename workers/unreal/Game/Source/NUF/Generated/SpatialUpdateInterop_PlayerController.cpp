// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// Note that this file has been generated automatically

#include "SpatialUpdateInterop_PlayerController.h"
#include "SpatialOS.h"
#include "Engine.h"
#include "SpatialActorChannel.h"
#include "Utils/BunchReader.h"
#include "SpatialNetDriver.h"
#include "SpatialUpdateInterop.h"

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

			{
				auto& Rotator = *(Op.Update.field_targetviewrotation().data());
				Value.Yaw = Rotator.yaw();
				Value.Pitch = Rotator.pitch();
				Value.Roll = Rotator.roll();
			}

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

			{
				auto& Vector = *(Op.Update.field_spawnlocation().data());
				Value.X = Vector.x();
				Value.Y = Vector.y();
				Value.Z = Vector.z();
			}

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

			{
				TArray<uint8> ValueData;
				FMemoryWriter ValueDataWriter(ValueData);
				bool Success;
				Value.NetSerialize(ValueDataWriter, nullptr, Success);
				Update.set_field_replicatedmovement(std::string((char*)ValueData.GetData(), ValueData.Num()));
			}
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

			{
				auto& ValueDataStr = *(Op.Update.field_replicatedmovement().data());
				TArray<uint8> ValueData;
				ValueData.Append((uint8*)ValueDataStr.data(), ValueDataStr.size());
				FMemoryReader ValueDataReader(ValueData);
				bool bSuccess;
				Value.NetSerialize(ValueDataReader, nullptr, bSuccess);
			}

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

			{
				auto& Vector = *(Op.Update.field_attachmentreplication_locationoffset().data());
				Value.X = Vector.x();
				Value.Y = Vector.y();
				Value.Z = Vector.z();
			}

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

			{
				auto& Vector = *(Op.Update.field_attachmentreplication_relativescale3d().data());
				Value.X = Vector.x();
				Value.Y = Vector.y();
				Value.Z = Vector.z();
			}

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

			{
				auto& Rotator = *(Op.Update.field_attachmentreplication_rotationoffset().data());
				Value.Yaw = Rotator.yaw();
				Value.Pitch = Rotator.pitch();
				Value.Roll = Rotator.roll();
			}

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

// RPC sender functions
void OnServerStartedVisualLoggerSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_UBOOL(bIsLogging);

	improbable::unreal::UnrealOnServerStartedVisualLoggerRequest Request;
	Request.set_field_bislogging(bIsLogging != 0);

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Onserverstartedvisuallogger>(Target, Request, 0);
}
void ClientWasKickedSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UTextProperty, KickReason);

	improbable::unreal::UnrealClientWasKickedRequest Request;
	// UNSUPPORTED

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientwaskicked>(Target, Request, 0);
}
void ClientVoiceHandshakeCompleteSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	improbable::unreal::UnrealClientVoiceHandshakeCompleteRequest Request;

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientvoicehandshakecomplete>(Target, Request, 0);
}
void ClientUpdateLevelStreamingStatusSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UNameProperty, PackageName);
	P_GET_UBOOL(bNewShouldBeLoaded);
	P_GET_UBOOL(bNewShouldBeVisible);
	P_GET_UBOOL(bNewShouldBlockOnLoad);
	P_GET_PROPERTY(UIntProperty, LODIndex);

	improbable::unreal::UnrealClientUpdateLevelStreamingStatusRequest Request;
	Request.set_field_packagename(TCHAR_TO_UTF8(*PackageName.ToString()));
	Request.set_field_bnewshouldbeloaded(bNewShouldBeLoaded != 0);
	Request.set_field_bnewshouldbevisible(bNewShouldBeVisible != 0);
	Request.set_field_bnewshouldblockonload(bNewShouldBlockOnLoad != 0);
	Request.set_field_lodindex(LODIndex);

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientupdatelevelstreamingstatus>(Target, Request, 0);
}
void ClientUnmutePlayerSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_STRUCT(FUniqueNetIdRepl, PlayerId)

	improbable::unreal::UnrealClientUnmutePlayerRequest Request;
	{
		TArray<uint8> ValueData;
		FMemoryWriter ValueDataWriter(ValueData);
		bool Success;
		PlayerId.NetSerialize(ValueDataWriter, nullptr, Success);
		Request.set_field_playerid(std::string((char*)ValueData.GetData(), ValueData.Num()));
	}

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientunmuteplayer>(Target, Request, 0);
}
void ClientTravelInternalSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UStrProperty, URL);
	P_GET_PROPERTY(UByteProperty, TravelType);
	P_GET_UBOOL(bSeamless);
	P_GET_STRUCT(FGuid, MapPackageGuid)

	improbable::unreal::UnrealClientTravelInternalRequest Request;
	Request.set_field_url(TCHAR_TO_UTF8(*URL));
	Request.set_field_traveltype(uint32_t(TravelType));
	Request.set_field_bseamless(bSeamless != 0);
	Request.set_field_mappackageguid_a(MapPackageGuid.A);
	Request.set_field_mappackageguid_b(MapPackageGuid.B);
	Request.set_field_mappackageguid_c(MapPackageGuid.C);
	Request.set_field_mappackageguid_d(MapPackageGuid.D);

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clienttravelinternal>(Target, Request, 0);
}
void ClientTeamMessageSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(APlayerState, SenderPlayerState);
	P_GET_PROPERTY(UStrProperty, S);
	P_GET_PROPERTY(UNameProperty, Type);
	P_GET_PROPERTY(UFloatProperty, MsgLifeTime);

	improbable::unreal::UnrealClientTeamMessageRequest Request;
	// WEAK OBJECT REPLICATION - Request.set_field_senderplayerstate = SenderPlayerState;
	Request.set_field_s(TCHAR_TO_UTF8(*S));
	Request.set_field_type(TCHAR_TO_UTF8(*Type.ToString()));
	Request.set_field_msglifetime(MsgLifeTime);

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientteammessage>(Target, Request, 0);
}
void ClientStopForceFeedbackSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(UForceFeedbackEffect, ForceFeedbackEffect);
	P_GET_PROPERTY(UNameProperty, Tag);

	improbable::unreal::UnrealClientStopForceFeedbackRequest Request;
	// WEAK OBJECT REPLICATION - Request.set_field_forcefeedbackeffect = ForceFeedbackEffect;
	Request.set_field_tag(TCHAR_TO_UTF8(*Tag.ToString()));

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstopforcefeedback>(Target, Request, 0);
}
void ClientStopCameraShakeSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(UClass, Shake);
	P_GET_UBOOL(bImmediately);

	improbable::unreal::UnrealClientStopCameraShakeRequest Request;
	// WEAK OBJECT REPLICATION - Request.set_field_shake = Shake;
	Request.set_field_bimmediately(bImmediately != 0);

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstopcamerashake>(Target, Request, 0);
}
void ClientStopCameraAnimSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(UCameraAnim, AnimToStop);

	improbable::unreal::UnrealClientStopCameraAnimRequest Request;
	// WEAK OBJECT REPLICATION - Request.set_field_animtostop = AnimToStop;

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstopcameraanim>(Target, Request, 0);
}
void ClientStartOnlineSessionSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	improbable::unreal::UnrealClientStartOnlineSessionRequest Request;

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstartonlinesession>(Target, Request, 0);
}
void ClientSpawnCameraLensEffectSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(UClass, LensEffectEmitterClass);

	improbable::unreal::UnrealClientSpawnCameraLensEffectRequest Request;
	// WEAK OBJECT REPLICATION - Request.set_field_lenseffectemitterclass = LensEffectEmitterClass;

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientspawncameralenseffect>(Target, Request, 0);
}
void ClientSetViewTargetSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(AActor, A);
	P_GET_STRUCT(FViewTargetTransitionParams, TransitionParams)

	improbable::unreal::UnrealClientSetViewTargetRequest Request;
	// WEAK OBJECT REPLICATION - Request.set_field_a = A;
	Request.set_field_transitionparams_blendtime(TransitionParams.BlendTime);
	Request.set_field_transitionparams_blendfunction(uint32_t(TransitionParams.BlendFunction));
	Request.set_field_transitionparams_blendexp(TransitionParams.BlendExp);
	Request.set_field_transitionparams_blockoutgoing(TransitionParams.bLockOutgoing != 0);

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetviewtarget>(Target, Request, 0);
}
void ClientSetSpectatorWaitingSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_UBOOL(bWaiting);

	improbable::unreal::UnrealClientSetSpectatorWaitingRequest Request;
	Request.set_field_bwaiting(bWaiting != 0);

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetspectatorwaiting>(Target, Request, 0);
}
void ClientSetHUDSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(UClass, NewHUDClass);

	improbable::unreal::UnrealClientSetHUDRequest Request;
	// WEAK OBJECT REPLICATION - Request.set_field_newhudclass = NewHUDClass;

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsethud>(Target, Request, 0);
}
void ClientSetForceMipLevelsToBeResidentSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(UMaterialInterface, Material);
	P_GET_PROPERTY(UFloatProperty, ForceDuration);
	P_GET_PROPERTY(UIntProperty, CinematicTextureGroups);

	improbable::unreal::UnrealClientSetForceMipLevelsToBeResidentRequest Request;
	// WEAK OBJECT REPLICATION - Request.set_field_material = Material;
	Request.set_field_forceduration(ForceDuration);
	Request.set_field_cinematictexturegroups(CinematicTextureGroups);

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetforcemiplevelstoberesident>(Target, Request, 0);
}
void ClientSetCinematicModeSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_UBOOL(bInCinematicMode);
	P_GET_UBOOL(bAffectsMovement);
	P_GET_UBOOL(bAffectsTurning);
	P_GET_UBOOL(bAffectsHUD);

	improbable::unreal::UnrealClientSetCinematicModeRequest Request;
	Request.set_field_bincinematicmode(bInCinematicMode != 0);
	Request.set_field_baffectsmovement(bAffectsMovement != 0);
	Request.set_field_baffectsturning(bAffectsTurning != 0);
	Request.set_field_baffectshud(bAffectsHUD != 0);

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetcinematicmode>(Target, Request, 0);
}
void ClientSetCameraModeSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UNameProperty, NewCamMode);

	improbable::unreal::UnrealClientSetCameraModeRequest Request;
	Request.set_field_newcammode(TCHAR_TO_UTF8(*NewCamMode.ToString()));

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetcameramode>(Target, Request, 0);
}
void ClientSetCameraFadeSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_UBOOL(bEnableFading);
	P_GET_STRUCT(FColor, FadeColor)
	P_GET_STRUCT(FVector2D, FadeAlpha)
	P_GET_PROPERTY(UFloatProperty, FadeTime);
	P_GET_UBOOL(bFadeAudio);

	improbable::unreal::UnrealClientSetCameraFadeRequest Request;
	Request.set_field_benablefading(bEnableFading != 0);
	Request.set_field_fadecolor_b(uint32_t(FadeColor.B));
	Request.set_field_fadecolor_g(uint32_t(FadeColor.G));
	Request.set_field_fadecolor_r(uint32_t(FadeColor.R));
	Request.set_field_fadecolor_a(uint32_t(FadeColor.A));
	Request.set_field_fadealpha_x(FadeAlpha.X);
	Request.set_field_fadealpha_y(FadeAlpha.Y);
	Request.set_field_fadetime(FadeTime);
	Request.set_field_bfadeaudio(bFadeAudio != 0);

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetcamerafade>(Target, Request, 0);
}
void ClientSetBlockOnAsyncLoadingSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	improbable::unreal::UnrealClientSetBlockOnAsyncLoadingRequest Request;

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetblockonasyncloading>(Target, Request, 0);
}
void ClientReturnToMainMenuSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UStrProperty, ReturnReason);

	improbable::unreal::UnrealClientReturnToMainMenuRequest Request;
	Request.set_field_returnreason(TCHAR_TO_UTF8(*ReturnReason));

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientreturntomainmenu>(Target, Request, 0);
}
void ClientRetryClientRestartSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(APawn, NewPawn);

	improbable::unreal::UnrealClientRetryClientRestartRequest Request;
	// WEAK OBJECT REPLICATION - Request.set_field_newpawn = NewPawn;

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientretryclientrestart>(Target, Request, 0);
}
void ClientRestartSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(APawn, NewPawn);

	improbable::unreal::UnrealClientRestartRequest Request;
	// WEAK OBJECT REPLICATION - Request.set_field_newpawn = NewPawn;

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientrestart>(Target, Request, 0);
}
void ClientResetSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	improbable::unreal::UnrealClientResetRequest Request;

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientreset>(Target, Request, 0);
}
void ClientRepObjRefSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(UObject, Object);

	improbable::unreal::UnrealClientRepObjRefRequest Request;
	// WEAK OBJECT REPLICATION - Request.set_field_object = Object;

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientrepobjref>(Target, Request, 0);
}
void ClientReceiveLocalizedMessageSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(UClass, Message);
	P_GET_PROPERTY(UIntProperty, Switch);
	P_GET_OBJECT(APlayerState, RelatedPlayerState_1);
	P_GET_OBJECT(APlayerState, RelatedPlayerState_2);
	P_GET_OBJECT(UObject, OptionalObject);

	improbable::unreal::UnrealClientReceiveLocalizedMessageRequest Request;
	// WEAK OBJECT REPLICATION - Request.set_field_message = Message;
	Request.set_field_switch(Switch);
	// WEAK OBJECT REPLICATION - Request.set_field_relatedplayerstate_1 = RelatedPlayerState_1;
	// WEAK OBJECT REPLICATION - Request.set_field_relatedplayerstate_2 = RelatedPlayerState_2;
	// WEAK OBJECT REPLICATION - Request.set_field_optionalobject = OptionalObject;

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientreceivelocalizedmessage>(Target, Request, 0);
}
void ClientPrestreamTexturesSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(AActor, ForcedActor);
	P_GET_PROPERTY(UFloatProperty, ForceDuration);
	P_GET_UBOOL(bEnableStreaming);
	P_GET_PROPERTY(UIntProperty, CinematicTextureGroups);

	improbable::unreal::UnrealClientPrestreamTexturesRequest Request;
	// WEAK OBJECT REPLICATION - Request.set_field_forcedactor = ForcedActor;
	Request.set_field_forceduration(ForceDuration);
	Request.set_field_benablestreaming(bEnableStreaming != 0);
	Request.set_field_cinematictexturegroups(CinematicTextureGroups);

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientprestreamtextures>(Target, Request, 0);
}
void ClientPrepareMapChangeSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UNameProperty, LevelName);
	P_GET_UBOOL(bFirst);
	P_GET_UBOOL(bLast);

	improbable::unreal::UnrealClientPrepareMapChangeRequest Request;
	Request.set_field_levelname(TCHAR_TO_UTF8(*LevelName.ToString()));
	Request.set_field_bfirst(bFirst != 0);
	Request.set_field_blast(bLast != 0);

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientpreparemapchange>(Target, Request, 0);
}
void ClientPlaySoundAtLocationSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(USoundBase, Sound);
	P_GET_STRUCT(FVector, Location)
	P_GET_PROPERTY(UFloatProperty, VolumeMultiplier);
	P_GET_PROPERTY(UFloatProperty, PitchMultiplier);

	improbable::unreal::UnrealClientPlaySoundAtLocationRequest Request;
	// WEAK OBJECT REPLICATION - Request.set_field_sound = Sound;
	Request.set_field_location(improbable::Vector3f(Location.X, Location.Y, Location.Z));
	Request.set_field_volumemultiplier(VolumeMultiplier);
	Request.set_field_pitchmultiplier(PitchMultiplier);

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaysoundatlocation>(Target, Request, 0);
}
void ClientPlaySoundSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(USoundBase, Sound);
	P_GET_PROPERTY(UFloatProperty, VolumeMultiplier);
	P_GET_PROPERTY(UFloatProperty, PitchMultiplier);

	improbable::unreal::UnrealClientPlaySoundRequest Request;
	// WEAK OBJECT REPLICATION - Request.set_field_sound = Sound;
	Request.set_field_volumemultiplier(VolumeMultiplier);
	Request.set_field_pitchmultiplier(PitchMultiplier);

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaysound>(Target, Request, 0);
}
void ClientPlayForceFeedbackSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(UForceFeedbackEffect, ForceFeedbackEffect);
	P_GET_UBOOL(bLooping);
	P_GET_PROPERTY(UNameProperty, Tag);

	improbable::unreal::UnrealClientPlayForceFeedbackRequest Request;
	// WEAK OBJECT REPLICATION - Request.set_field_forcefeedbackeffect = ForceFeedbackEffect;
	Request.set_field_blooping(bLooping != 0);
	Request.set_field_tag(TCHAR_TO_UTF8(*Tag.ToString()));

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplayforcefeedback>(Target, Request, 0);
}
void ClientPlayCameraShakeSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(UClass, Shake);
	P_GET_PROPERTY(UFloatProperty, Scale);
	P_GET_PROPERTY(UByteProperty, PlaySpace);
	P_GET_STRUCT(FRotator, UserPlaySpaceRot)

	improbable::unreal::UnrealClientPlayCameraShakeRequest Request;
	// WEAK OBJECT REPLICATION - Request.set_field_shake = Shake;
	Request.set_field_scale(Scale);
	Request.set_field_playspace(uint32_t(PlaySpace));
	Request.set_field_userplayspacerot(improbable::unreal::UnrealFRotator(UserPlaySpaceRot.Yaw, UserPlaySpaceRot.Pitch, UserPlaySpaceRot.Roll));

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaycamerashake>(Target, Request, 0);
}
void ClientPlayCameraAnimSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
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

	improbable::unreal::UnrealClientPlayCameraAnimRequest Request;
	// WEAK OBJECT REPLICATION - Request.set_field_animtoplay = AnimToPlay;
	Request.set_field_scale(Scale);
	Request.set_field_rate(Rate);
	Request.set_field_blendintime(BlendInTime);
	Request.set_field_blendouttime(BlendOutTime);
	Request.set_field_bloop(bLoop != 0);
	Request.set_field_brandomstarttime(bRandomStartTime != 0);
	Request.set_field_space(uint32_t(Space));
	Request.set_field_customplayspace(improbable::unreal::UnrealFRotator(CustomPlaySpace.Yaw, CustomPlaySpace.Pitch, CustomPlaySpace.Roll));

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaycameraanim>(Target, Request, 0);
}
void ClientMutePlayerSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_STRUCT(FUniqueNetIdRepl, PlayerId)

	improbable::unreal::UnrealClientMutePlayerRequest Request;
	{
		TArray<uint8> ValueData;
		FMemoryWriter ValueDataWriter(ValueData);
		bool Success;
		PlayerId.NetSerialize(ValueDataWriter, nullptr, Success);
		Request.set_field_playerid(std::string((char*)ValueData.GetData(), ValueData.Num()));
	}

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientmuteplayer>(Target, Request, 0);
}
void ClientMessageSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UStrProperty, S);
	P_GET_PROPERTY(UNameProperty, Type);
	P_GET_PROPERTY(UFloatProperty, MsgLifeTime);

	improbable::unreal::UnrealClientMessageRequest Request;
	Request.set_field_s(TCHAR_TO_UTF8(*S));
	Request.set_field_type(TCHAR_TO_UTF8(*Type.ToString()));
	Request.set_field_msglifetime(MsgLifeTime);

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientmessage>(Target, Request, 0);
}
void ClientIgnoreMoveInputSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_UBOOL(bIgnore);

	improbable::unreal::UnrealClientIgnoreMoveInputRequest Request;
	Request.set_field_bignore(bIgnore != 0);

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientignoremoveinput>(Target, Request, 0);
}
void ClientIgnoreLookInputSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_UBOOL(bIgnore);

	improbable::unreal::UnrealClientIgnoreLookInputRequest Request;
	Request.set_field_bignore(bIgnore != 0);

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientignorelookinput>(Target, Request, 0);
}
void ClientGotoStateSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UNameProperty, NewState);

	improbable::unreal::UnrealClientGotoStateRequest Request;
	Request.set_field_newstate(TCHAR_TO_UTF8(*NewState.ToString()));

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientgotostate>(Target, Request, 0);
}
void ClientGameEndedSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(AActor, EndGameFocus);
	P_GET_UBOOL(bIsWinner);

	improbable::unreal::UnrealClientGameEndedRequest Request;
	// WEAK OBJECT REPLICATION - Request.set_field_endgamefocus = EndGameFocus;
	Request.set_field_biswinner(bIsWinner != 0);

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientgameended>(Target, Request, 0);
}
void ClientForceGarbageCollectionSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	improbable::unreal::UnrealClientForceGarbageCollectionRequest Request;

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientforcegarbagecollection>(Target, Request, 0);
}
void ClientFlushLevelStreamingSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	improbable::unreal::UnrealClientFlushLevelStreamingRequest Request;

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientflushlevelstreaming>(Target, Request, 0);
}
void ClientEndOnlineSessionSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	improbable::unreal::UnrealClientEndOnlineSessionRequest Request;

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientendonlinesession>(Target, Request, 0);
}
void ClientEnableNetworkVoiceSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_UBOOL(bEnable);

	improbable::unreal::UnrealClientEnableNetworkVoiceRequest Request;
	Request.set_field_benable(bEnable != 0);

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientenablenetworkvoice>(Target, Request, 0);
}
void ClientCommitMapChangeSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	improbable::unreal::UnrealClientCommitMapChangeRequest Request;

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientcommitmapchange>(Target, Request, 0);
}
void ClientClearCameraLensEffectsSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	improbable::unreal::UnrealClientClearCameraLensEffectsRequest Request;

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientclearcameralenseffects>(Target, Request, 0);
}
void ClientCapBandwidthSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UIntProperty, Cap);

	improbable::unreal::UnrealClientCapBandwidthRequest Request;
	Request.set_field_cap(Cap);

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientcapbandwidth>(Target, Request, 0);
}
void ClientCancelPendingMapChangeSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	improbable::unreal::UnrealClientCancelPendingMapChangeRequest Request;

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientcancelpendingmapchange>(Target, Request, 0);
}
void ClientAddTextureStreamingLocSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_STRUCT(FVector, InLoc)
	P_GET_PROPERTY(UFloatProperty, Duration);
	P_GET_UBOOL(bOverrideLocation);

	improbable::unreal::UnrealClientAddTextureStreamingLocRequest Request;
	Request.set_field_inloc(improbable::Vector3f(InLoc.X, InLoc.Y, InLoc.Z));
	Request.set_field_duration(Duration);
	Request.set_field_boverridelocation(bOverrideLocation != 0);

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientaddtexturestreamingloc>(Target, Request, 0);
}
void ClientSetRotationSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_STRUCT(FRotator, NewRotation)
	P_GET_UBOOL(bResetCamera);

	improbable::unreal::UnrealClientSetRotationRequest Request;
	Request.set_field_newrotation(improbable::unreal::UnrealFRotator(NewRotation.Yaw, NewRotation.Pitch, NewRotation.Roll));
	Request.set_field_bresetcamera(bResetCamera != 0);

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetrotation>(Target, Request, 0);
}
void ClientSetLocationSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_STRUCT(FVector, NewLocation)
	P_GET_STRUCT(FRotator, NewRotation)

	improbable::unreal::UnrealClientSetLocationRequest Request;
	Request.set_field_newlocation(improbable::Vector3f(NewLocation.X, NewLocation.Y, NewLocation.Z));
	Request.set_field_newrotation(improbable::unreal::UnrealFRotator(NewRotation.Yaw, NewRotation.Pitch, NewRotation.Roll));

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetlocation>(Target, Request, 0);
}
void ServerViewSelfSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_STRUCT(FViewTargetTransitionParams, TransitionParams)

	improbable::unreal::UnrealServerViewSelfRequest Request;
	Request.set_field_transitionparams_blendtime(TransitionParams.BlendTime);
	Request.set_field_transitionparams_blendfunction(uint32_t(TransitionParams.BlendFunction));
	Request.set_field_transitionparams_blendexp(TransitionParams.BlendExp);
	Request.set_field_transitionparams_blockoutgoing(TransitionParams.bLockOutgoing != 0);

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverviewself>(Target, Request, 0);
}
void ServerViewPrevPlayerSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	improbable::unreal::UnrealServerViewPrevPlayerRequest Request;

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverviewprevplayer>(Target, Request, 0);
}
void ServerViewNextPlayerSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	improbable::unreal::UnrealServerViewNextPlayerRequest Request;

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverviewnextplayer>(Target, Request, 0);
}
void ServerVerifyViewTargetSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	improbable::unreal::UnrealServerVerifyViewTargetRequest Request;

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serververifyviewtarget>(Target, Request, 0);
}
void ServerUpdateLevelVisibilitySender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UNameProperty, PackageName);
	P_GET_UBOOL(bIsVisible);

	improbable::unreal::UnrealServerUpdateLevelVisibilityRequest Request;
	Request.set_field_packagename(TCHAR_TO_UTF8(*PackageName.ToString()));
	Request.set_field_bisvisible(bIsVisible != 0);

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverupdatelevelvisibility>(Target, Request, 0);
}
void ServerUpdateCameraSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_STRUCT(FVector_NetQuantize, CamLoc)
	P_GET_PROPERTY(UIntProperty, CamPitchAndYaw);

	improbable::unreal::UnrealServerUpdateCameraRequest Request;
	Request.set_field_camloc(improbable::Vector3f(CamLoc.X, CamLoc.Y, CamLoc.Z));
	Request.set_field_campitchandyaw(CamPitchAndYaw);

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverupdatecamera>(Target, Request, 0);
}
void ServerUnmutePlayerSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_STRUCT(FUniqueNetIdRepl, PlayerId)

	improbable::unreal::UnrealServerUnmutePlayerRequest Request;
	{
		TArray<uint8> ValueData;
		FMemoryWriter ValueDataWriter(ValueData);
		bool Success;
		PlayerId.NetSerialize(ValueDataWriter, nullptr, Success);
		Request.set_field_playerid(std::string((char*)ValueData.GetData(), ValueData.Num()));
	}

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverunmuteplayer>(Target, Request, 0);
}
void ServerToggleAILoggingSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	improbable::unreal::UnrealServerToggleAILoggingRequest Request;

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servertoggleailogging>(Target, Request, 0);
}
void ServerShortTimeoutSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	improbable::unreal::UnrealServerShortTimeoutRequest Request;

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servershorttimeout>(Target, Request, 0);
}
void ServerSetSpectatorWaitingSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_UBOOL(bWaiting);

	improbable::unreal::UnrealServerSetSpectatorWaitingRequest Request;
	Request.set_field_bwaiting(bWaiting != 0);

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serversetspectatorwaiting>(Target, Request, 0);
}
void ServerSetSpectatorLocationSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_STRUCT(FVector, NewLoc)
	P_GET_STRUCT(FRotator, NewRot)

	improbable::unreal::UnrealServerSetSpectatorLocationRequest Request;
	Request.set_field_newloc(improbable::Vector3f(NewLoc.X, NewLoc.Y, NewLoc.Z));
	Request.set_field_newrot(improbable::unreal::UnrealFRotator(NewRot.Yaw, NewRot.Pitch, NewRot.Roll));

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serversetspectatorlocation>(Target, Request, 0);
}
void ServerRestartPlayerSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	improbable::unreal::UnrealServerRestartPlayerRequest Request;

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverrestartplayer>(Target, Request, 0);
}
void ServerPauseSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	improbable::unreal::UnrealServerPauseRequest Request;

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverpause>(Target, Request, 0);
}
void ServerNotifyLoadedWorldSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UNameProperty, WorldPackageName);

	improbable::unreal::UnrealServerNotifyLoadedWorldRequest Request;
	Request.set_field_worldpackagename(TCHAR_TO_UTF8(*WorldPackageName.ToString()));

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servernotifyloadedworld>(Target, Request, 0);
}
void ServerMutePlayerSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_STRUCT(FUniqueNetIdRepl, PlayerId)

	improbable::unreal::UnrealServerMutePlayerRequest Request;
	{
		TArray<uint8> ValueData;
		FMemoryWriter ValueDataWriter(ValueData);
		bool Success;
		PlayerId.NetSerialize(ValueDataWriter, nullptr, Success);
		Request.set_field_playerid(std::string((char*)ValueData.GetData(), ValueData.Num()));
	}

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servermuteplayer>(Target, Request, 0);
}
void ServerCheckClientPossessionReliableSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	improbable::unreal::UnrealServerCheckClientPossessionReliableRequest Request;

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servercheckclientpossessionreliable>(Target, Request, 0);
}
void ServerCheckClientPossessionSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	improbable::unreal::UnrealServerCheckClientPossessionRequest Request;

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servercheckclientpossession>(Target, Request, 0);
}
void ServerChangeNameSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UStrProperty, S);

	improbable::unreal::UnrealServerChangeNameRequest Request;
	Request.set_field_s(TCHAR_TO_UTF8(*S));

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverchangename>(Target, Request, 0);
}
void ServerCameraSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UNameProperty, NewMode);

	improbable::unreal::UnrealServerCameraRequest Request;
	Request.set_field_newmode(TCHAR_TO_UTF8(*NewMode.ToString()));

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servercamera>(Target, Request, 0);
}
void ServerAcknowledgePossessionSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(APawn, P);

	improbable::unreal::UnrealServerAcknowledgePossessionRequest Request;
	// WEAK OBJECT REPLICATION - Request.set_field_p = P;

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serveracknowledgepossession>(Target, Request, 0);
}
} // ::

void FSpatialTypeBinding_PlayerController::Init(USpatialUpdateInterop* InUpdateInterop, UPackageMap* InPackageMap)
{
	UpdateInterop = InUpdateInterop;
	PackageMap = InPackageMap;
	RPCToSenderMap.Emplace("OnServerStartedVisualLogger", &OnServerStartedVisualLoggerSender);
	RPCToSenderMap.Emplace("ClientWasKicked", &ClientWasKickedSender);
	RPCToSenderMap.Emplace("ClientVoiceHandshakeComplete", &ClientVoiceHandshakeCompleteSender);
	RPCToSenderMap.Emplace("ClientUpdateLevelStreamingStatus", &ClientUpdateLevelStreamingStatusSender);
	RPCToSenderMap.Emplace("ClientUnmutePlayer", &ClientUnmutePlayerSender);
	RPCToSenderMap.Emplace("ClientTravelInternal", &ClientTravelInternalSender);
	RPCToSenderMap.Emplace("ClientTeamMessage", &ClientTeamMessageSender);
	RPCToSenderMap.Emplace("ClientStopForceFeedback", &ClientStopForceFeedbackSender);
	RPCToSenderMap.Emplace("ClientStopCameraShake", &ClientStopCameraShakeSender);
	RPCToSenderMap.Emplace("ClientStopCameraAnim", &ClientStopCameraAnimSender);
	RPCToSenderMap.Emplace("ClientStartOnlineSession", &ClientStartOnlineSessionSender);
	RPCToSenderMap.Emplace("ClientSpawnCameraLensEffect", &ClientSpawnCameraLensEffectSender);
	RPCToSenderMap.Emplace("ClientSetViewTarget", &ClientSetViewTargetSender);
	RPCToSenderMap.Emplace("ClientSetSpectatorWaiting", &ClientSetSpectatorWaitingSender);
	RPCToSenderMap.Emplace("ClientSetHUD", &ClientSetHUDSender);
	RPCToSenderMap.Emplace("ClientSetForceMipLevelsToBeResident", &ClientSetForceMipLevelsToBeResidentSender);
	RPCToSenderMap.Emplace("ClientSetCinematicMode", &ClientSetCinematicModeSender);
	RPCToSenderMap.Emplace("ClientSetCameraMode", &ClientSetCameraModeSender);
	RPCToSenderMap.Emplace("ClientSetCameraFade", &ClientSetCameraFadeSender);
	RPCToSenderMap.Emplace("ClientSetBlockOnAsyncLoading", &ClientSetBlockOnAsyncLoadingSender);
	RPCToSenderMap.Emplace("ClientReturnToMainMenu", &ClientReturnToMainMenuSender);
	RPCToSenderMap.Emplace("ClientRetryClientRestart", &ClientRetryClientRestartSender);
	RPCToSenderMap.Emplace("ClientRestart", &ClientRestartSender);
	RPCToSenderMap.Emplace("ClientReset", &ClientResetSender);
	RPCToSenderMap.Emplace("ClientRepObjRef", &ClientRepObjRefSender);
	RPCToSenderMap.Emplace("ClientReceiveLocalizedMessage", &ClientReceiveLocalizedMessageSender);
	RPCToSenderMap.Emplace("ClientPrestreamTextures", &ClientPrestreamTexturesSender);
	RPCToSenderMap.Emplace("ClientPrepareMapChange", &ClientPrepareMapChangeSender);
	RPCToSenderMap.Emplace("ClientPlaySoundAtLocation", &ClientPlaySoundAtLocationSender);
	RPCToSenderMap.Emplace("ClientPlaySound", &ClientPlaySoundSender);
	RPCToSenderMap.Emplace("ClientPlayForceFeedback", &ClientPlayForceFeedbackSender);
	RPCToSenderMap.Emplace("ClientPlayCameraShake", &ClientPlayCameraShakeSender);
	RPCToSenderMap.Emplace("ClientPlayCameraAnim", &ClientPlayCameraAnimSender);
	RPCToSenderMap.Emplace("ClientMutePlayer", &ClientMutePlayerSender);
	RPCToSenderMap.Emplace("ClientMessage", &ClientMessageSender);
	RPCToSenderMap.Emplace("ClientIgnoreMoveInput", &ClientIgnoreMoveInputSender);
	RPCToSenderMap.Emplace("ClientIgnoreLookInput", &ClientIgnoreLookInputSender);
	RPCToSenderMap.Emplace("ClientGotoState", &ClientGotoStateSender);
	RPCToSenderMap.Emplace("ClientGameEnded", &ClientGameEndedSender);
	RPCToSenderMap.Emplace("ClientForceGarbageCollection", &ClientForceGarbageCollectionSender);
	RPCToSenderMap.Emplace("ClientFlushLevelStreaming", &ClientFlushLevelStreamingSender);
	RPCToSenderMap.Emplace("ClientEndOnlineSession", &ClientEndOnlineSessionSender);
	RPCToSenderMap.Emplace("ClientEnableNetworkVoice", &ClientEnableNetworkVoiceSender);
	RPCToSenderMap.Emplace("ClientCommitMapChange", &ClientCommitMapChangeSender);
	RPCToSenderMap.Emplace("ClientClearCameraLensEffects", &ClientClearCameraLensEffectsSender);
	RPCToSenderMap.Emplace("ClientCapBandwidth", &ClientCapBandwidthSender);
	RPCToSenderMap.Emplace("ClientCancelPendingMapChange", &ClientCancelPendingMapChangeSender);
	RPCToSenderMap.Emplace("ClientAddTextureStreamingLoc", &ClientAddTextureStreamingLocSender);
	RPCToSenderMap.Emplace("ClientSetRotation", &ClientSetRotationSender);
	RPCToSenderMap.Emplace("ClientSetLocation", &ClientSetLocationSender);
	RPCToSenderMap.Emplace("ServerViewSelf", &ServerViewSelfSender);
	RPCToSenderMap.Emplace("ServerViewPrevPlayer", &ServerViewPrevPlayerSender);
	RPCToSenderMap.Emplace("ServerViewNextPlayer", &ServerViewNextPlayerSender);
	RPCToSenderMap.Emplace("ServerVerifyViewTarget", &ServerVerifyViewTargetSender);
	RPCToSenderMap.Emplace("ServerUpdateLevelVisibility", &ServerUpdateLevelVisibilitySender);
	RPCToSenderMap.Emplace("ServerUpdateCamera", &ServerUpdateCameraSender);
	RPCToSenderMap.Emplace("ServerUnmutePlayer", &ServerUnmutePlayerSender);
	RPCToSenderMap.Emplace("ServerToggleAILogging", &ServerToggleAILoggingSender);
	RPCToSenderMap.Emplace("ServerShortTimeout", &ServerShortTimeoutSender);
	RPCToSenderMap.Emplace("ServerSetSpectatorWaiting", &ServerSetSpectatorWaitingSender);
	RPCToSenderMap.Emplace("ServerSetSpectatorLocation", &ServerSetSpectatorLocationSender);
	RPCToSenderMap.Emplace("ServerRestartPlayer", &ServerRestartPlayerSender);
	RPCToSenderMap.Emplace("ServerPause", &ServerPauseSender);
	RPCToSenderMap.Emplace("ServerNotifyLoadedWorld", &ServerNotifyLoadedWorldSender);
	RPCToSenderMap.Emplace("ServerMutePlayer", &ServerMutePlayerSender);
	RPCToSenderMap.Emplace("ServerCheckClientPossessionReliable", &ServerCheckClientPossessionReliableSender);
	RPCToSenderMap.Emplace("ServerCheckClientPossession", &ServerCheckClientPossessionSender);
	RPCToSenderMap.Emplace("ServerChangeName", &ServerChangeNameSender);
	RPCToSenderMap.Emplace("ServerCamera", &ServerCameraSender);
	RPCToSenderMap.Emplace("ServerAcknowledgePossession", &ServerAcknowledgePossessionSender);
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
	using ClientRPCCommandTypes = improbable::unreal::UnrealPlayerControllerClientRPCs::Commands;
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Onserverstartedvisuallogger>(std::bind(&FSpatialTypeBinding_PlayerController::OnServerStartedVisualLoggerReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientwaskicked>(std::bind(&FSpatialTypeBinding_PlayerController::ClientWasKickedReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientvoicehandshakecomplete>(std::bind(&FSpatialTypeBinding_PlayerController::ClientVoiceHandshakeCompleteReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientupdatelevelstreamingstatus>(std::bind(&FSpatialTypeBinding_PlayerController::ClientUpdateLevelStreamingStatusReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientunmuteplayer>(std::bind(&FSpatialTypeBinding_PlayerController::ClientUnmutePlayerReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clienttravelinternal>(std::bind(&FSpatialTypeBinding_PlayerController::ClientTravelInternalReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientteammessage>(std::bind(&FSpatialTypeBinding_PlayerController::ClientTeamMessageReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientstopforcefeedback>(std::bind(&FSpatialTypeBinding_PlayerController::ClientStopForceFeedbackReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientstopcamerashake>(std::bind(&FSpatialTypeBinding_PlayerController::ClientStopCameraShakeReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientstopcameraanim>(std::bind(&FSpatialTypeBinding_PlayerController::ClientStopCameraAnimReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientstartonlinesession>(std::bind(&FSpatialTypeBinding_PlayerController::ClientStartOnlineSessionReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientspawncameralenseffect>(std::bind(&FSpatialTypeBinding_PlayerController::ClientSpawnCameraLensEffectReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientsetviewtarget>(std::bind(&FSpatialTypeBinding_PlayerController::ClientSetViewTargetReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientsetspectatorwaiting>(std::bind(&FSpatialTypeBinding_PlayerController::ClientSetSpectatorWaitingReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientsethud>(std::bind(&FSpatialTypeBinding_PlayerController::ClientSetHUDReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientsetforcemiplevelstoberesident>(std::bind(&FSpatialTypeBinding_PlayerController::ClientSetForceMipLevelsToBeResidentReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientsetcinematicmode>(std::bind(&FSpatialTypeBinding_PlayerController::ClientSetCinematicModeReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientsetcameramode>(std::bind(&FSpatialTypeBinding_PlayerController::ClientSetCameraModeReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientsetcamerafade>(std::bind(&FSpatialTypeBinding_PlayerController::ClientSetCameraFadeReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientsetblockonasyncloading>(std::bind(&FSpatialTypeBinding_PlayerController::ClientSetBlockOnAsyncLoadingReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientreturntomainmenu>(std::bind(&FSpatialTypeBinding_PlayerController::ClientReturnToMainMenuReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientretryclientrestart>(std::bind(&FSpatialTypeBinding_PlayerController::ClientRetryClientRestartReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientrestart>(std::bind(&FSpatialTypeBinding_PlayerController::ClientRestartReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientreset>(std::bind(&FSpatialTypeBinding_PlayerController::ClientResetReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientrepobjref>(std::bind(&FSpatialTypeBinding_PlayerController::ClientRepObjRefReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientreceivelocalizedmessage>(std::bind(&FSpatialTypeBinding_PlayerController::ClientReceiveLocalizedMessageReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientprestreamtextures>(std::bind(&FSpatialTypeBinding_PlayerController::ClientPrestreamTexturesReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientpreparemapchange>(std::bind(&FSpatialTypeBinding_PlayerController::ClientPrepareMapChangeReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientplaysoundatlocation>(std::bind(&FSpatialTypeBinding_PlayerController::ClientPlaySoundAtLocationReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientplaysound>(std::bind(&FSpatialTypeBinding_PlayerController::ClientPlaySoundReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientplayforcefeedback>(std::bind(&FSpatialTypeBinding_PlayerController::ClientPlayForceFeedbackReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientplaycamerashake>(std::bind(&FSpatialTypeBinding_PlayerController::ClientPlayCameraShakeReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientplaycameraanim>(std::bind(&FSpatialTypeBinding_PlayerController::ClientPlayCameraAnimReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientmuteplayer>(std::bind(&FSpatialTypeBinding_PlayerController::ClientMutePlayerReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientmessage>(std::bind(&FSpatialTypeBinding_PlayerController::ClientMessageReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientignoremoveinput>(std::bind(&FSpatialTypeBinding_PlayerController::ClientIgnoreMoveInputReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientignorelookinput>(std::bind(&FSpatialTypeBinding_PlayerController::ClientIgnoreLookInputReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientgotostate>(std::bind(&FSpatialTypeBinding_PlayerController::ClientGotoStateReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientgameended>(std::bind(&FSpatialTypeBinding_PlayerController::ClientGameEndedReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientforcegarbagecollection>(std::bind(&FSpatialTypeBinding_PlayerController::ClientForceGarbageCollectionReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientflushlevelstreaming>(std::bind(&FSpatialTypeBinding_PlayerController::ClientFlushLevelStreamingReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientendonlinesession>(std::bind(&FSpatialTypeBinding_PlayerController::ClientEndOnlineSessionReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientenablenetworkvoice>(std::bind(&FSpatialTypeBinding_PlayerController::ClientEnableNetworkVoiceReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientcommitmapchange>(std::bind(&FSpatialTypeBinding_PlayerController::ClientCommitMapChangeReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientclearcameralenseffects>(std::bind(&FSpatialTypeBinding_PlayerController::ClientClearCameraLensEffectsReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientcapbandwidth>(std::bind(&FSpatialTypeBinding_PlayerController::ClientCapBandwidthReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientcancelpendingmapchange>(std::bind(&FSpatialTypeBinding_PlayerController::ClientCancelPendingMapChangeReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientaddtexturestreamingloc>(std::bind(&FSpatialTypeBinding_PlayerController::ClientAddTextureStreamingLocReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientsetrotation>(std::bind(&FSpatialTypeBinding_PlayerController::ClientSetRotationReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientsetlocation>(std::bind(&FSpatialTypeBinding_PlayerController::ClientSetLocationReceiver, this, std::placeholders::_1)));
	using ServerRPCCommandTypes = improbable::unreal::UnrealPlayerControllerServerRPCs::Commands;
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Serverviewself>(std::bind(&FSpatialTypeBinding_PlayerController::ServerViewSelfReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Serverviewprevplayer>(std::bind(&FSpatialTypeBinding_PlayerController::ServerViewPrevPlayerReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Serverviewnextplayer>(std::bind(&FSpatialTypeBinding_PlayerController::ServerViewNextPlayerReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Serververifyviewtarget>(std::bind(&FSpatialTypeBinding_PlayerController::ServerVerifyViewTargetReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Serverupdatelevelvisibility>(std::bind(&FSpatialTypeBinding_PlayerController::ServerUpdateLevelVisibilityReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Serverupdatecamera>(std::bind(&FSpatialTypeBinding_PlayerController::ServerUpdateCameraReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Serverunmuteplayer>(std::bind(&FSpatialTypeBinding_PlayerController::ServerUnmutePlayerReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Servertoggleailogging>(std::bind(&FSpatialTypeBinding_PlayerController::ServerToggleAILoggingReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Servershorttimeout>(std::bind(&FSpatialTypeBinding_PlayerController::ServerShortTimeoutReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Serversetspectatorwaiting>(std::bind(&FSpatialTypeBinding_PlayerController::ServerSetSpectatorWaitingReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Serversetspectatorlocation>(std::bind(&FSpatialTypeBinding_PlayerController::ServerSetSpectatorLocationReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Serverrestartplayer>(std::bind(&FSpatialTypeBinding_PlayerController::ServerRestartPlayerReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Serverpause>(std::bind(&FSpatialTypeBinding_PlayerController::ServerPauseReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Servernotifyloadedworld>(std::bind(&FSpatialTypeBinding_PlayerController::ServerNotifyLoadedWorldReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Servermuteplayer>(std::bind(&FSpatialTypeBinding_PlayerController::ServerMutePlayerReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Servercheckclientpossessionreliable>(std::bind(&FSpatialTypeBinding_PlayerController::ServerCheckClientPossessionReliableReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Servercheckclientpossession>(std::bind(&FSpatialTypeBinding_PlayerController::ServerCheckClientPossessionReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Serverchangename>(std::bind(&FSpatialTypeBinding_PlayerController::ServerChangeNameReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Servercamera>(std::bind(&FSpatialTypeBinding_PlayerController::ServerCameraReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Serveracknowledgepossession>(std::bind(&FSpatialTypeBinding_PlayerController::ServerAcknowledgePossessionReceiver, this, std::placeholders::_1)));
}

void FSpatialTypeBinding_PlayerController::UnbindFromView()
{
	TSharedPtr<worker::View> View = UpdateInterop->GetSpatialOS()->GetView().Pin();
	View->Remove(SingleClientCallback);
	View->Remove(MultiClientCallback);
	for (auto& Callback : RPCReceiverCallbacks)
	{
	View->Remove(Callback);
	}
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

void FSpatialTypeBinding_PlayerController::SendRPCCommand(const UFunction* const Function, FFrame* const RPCFrame, const worker::EntityId& Target)
{
	TSharedPtr<worker::Connection> Connection = UpdateInterop->GetSpatialOS()->GetConnection().Pin();
	auto Func = RPCToSenderMap.Find(FName(*Function->GetName()));
	checkf(*Func, TEXT(""))
	(*Func)(Connection.Get(), RPCFrame, Target);
}
void FSpatialTypeBinding_PlayerController::OnServerStartedVisualLoggerReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Onserverstartedvisuallogger>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	bool bIsLogging;
	bIsLogging = Op.Request.field_bislogging();

	TargetObject->OnServerStartedVisualLogger_Implementation(bIsLogging);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Onserverstartedvisuallogger>(Op);
}
void FSpatialTypeBinding_PlayerController::ClientWasKickedReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientwaskicked>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	FText KickReason;
	// UNSUPPORTED

	TargetObject->ClientWasKicked_Implementation(KickReason);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientwaskicked>(Op);
}
void FSpatialTypeBinding_PlayerController::ClientVoiceHandshakeCompleteReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientvoicehandshakecomplete>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));

	TargetObject->ClientVoiceHandshakeComplete_Implementation();

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientvoicehandshakecomplete>(Op);
}
void FSpatialTypeBinding_PlayerController::ClientUpdateLevelStreamingStatusReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientupdatelevelstreamingstatus>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	FName PackageName;
	PackageName = FName((Op.Request.field_packagename()).data());
	bool bNewShouldBeLoaded;
	bNewShouldBeLoaded = Op.Request.field_bnewshouldbeloaded();
	bool bNewShouldBeVisible;
	bNewShouldBeVisible = Op.Request.field_bnewshouldbevisible();
	bool bNewShouldBlockOnLoad;
	bNewShouldBlockOnLoad = Op.Request.field_bnewshouldblockonload();
	int32 LODIndex;
	LODIndex = Op.Request.field_lodindex();

	TargetObject->ClientUpdateLevelStreamingStatus_Implementation(PackageName, bNewShouldBeLoaded, bNewShouldBeVisible, bNewShouldBlockOnLoad, LODIndex);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientupdatelevelstreamingstatus>(Op);
}
void FSpatialTypeBinding_PlayerController::ClientUnmutePlayerReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientunmuteplayer>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	FUniqueNetIdRepl PlayerId;
	{
		auto& ValueDataStr = Op.Request.field_playerid();
		TArray<uint8> ValueData;
		ValueData.Append((uint8*)ValueDataStr.data(), ValueDataStr.size());
		FMemoryReader ValueDataReader(ValueData);
		bool bSuccess;
		PlayerId.NetSerialize(ValueDataReader, nullptr, bSuccess);
	}

	TargetObject->ClientUnmutePlayer_Implementation(PlayerId);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientunmuteplayer>(Op);
}
void FSpatialTypeBinding_PlayerController::ClientTravelInternalReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clienttravelinternal>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	FString URL;
	URL = FString(UTF8_TO_TCHAR(Op.Request.field_url().c_str()));
	TEnumAsByte<ETravelType> TravelType;
	// Byte properties are weird, because they can also be an enum in the form TEnumAsByte<...>.
	// Therefore, the code generator needs to cast to either TEnumAsByte<...> or uint8. However,
	// as TEnumAsByte<...> only has a uint8 constructor, we need to cast the SpatialOS value into
	// uint8 first, which causes "uint8(uint8(...))" to be generated for non enum bytes.
	TravelType = TEnumAsByte<ETravelType>(uint8(Op.Request.field_traveltype()));
	bool bSeamless;
	bSeamless = Op.Request.field_bseamless();
	FGuid MapPackageGuid;
	MapPackageGuid.A = Op.Request.field_mappackageguid_a();
	MapPackageGuid.B = Op.Request.field_mappackageguid_b();
	MapPackageGuid.C = Op.Request.field_mappackageguid_c();
	MapPackageGuid.D = Op.Request.field_mappackageguid_d();

	TargetObject->ClientTravelInternal_Implementation(URL, TravelType, bSeamless, MapPackageGuid);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clienttravelinternal>(Op);
}
void FSpatialTypeBinding_PlayerController::ClientTeamMessageReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientteammessage>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	APlayerState* SenderPlayerState = nullptr;
	// UNSUPPORTED ObjectProperty - SenderPlayerState Op.Request.field_senderplayerstate();
	FString S;
	S = FString(UTF8_TO_TCHAR(Op.Request.field_s().c_str()));
	FName Type;
	Type = FName((Op.Request.field_type()).data());
	float MsgLifeTime;
	MsgLifeTime = Op.Request.field_msglifetime();

	TargetObject->ClientTeamMessage_Implementation(SenderPlayerState, S, Type, MsgLifeTime);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientteammessage>(Op);
}
void FSpatialTypeBinding_PlayerController::ClientStopForceFeedbackReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstopforcefeedback>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	UForceFeedbackEffect* ForceFeedbackEffect = nullptr;
	// UNSUPPORTED ObjectProperty - ForceFeedbackEffect Op.Request.field_forcefeedbackeffect();
	FName Tag;
	Tag = FName((Op.Request.field_tag()).data());

	TargetObject->ClientStopForceFeedback_Implementation(ForceFeedbackEffect, Tag);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstopforcefeedback>(Op);
}
void FSpatialTypeBinding_PlayerController::ClientStopCameraShakeReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstopcamerashake>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	TSubclassOf<UCameraShake>  Shake = nullptr;
	// UNSUPPORTED ObjectProperty - Shake Op.Request.field_shake();
	bool bImmediately;
	bImmediately = Op.Request.field_bimmediately();

	TargetObject->ClientStopCameraShake_Implementation(Shake, bImmediately);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstopcamerashake>(Op);
}
void FSpatialTypeBinding_PlayerController::ClientStopCameraAnimReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstopcameraanim>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	UCameraAnim* AnimToStop = nullptr;
	// UNSUPPORTED ObjectProperty - AnimToStop Op.Request.field_animtostop();

	TargetObject->ClientStopCameraAnim_Implementation(AnimToStop);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstopcameraanim>(Op);
}
void FSpatialTypeBinding_PlayerController::ClientStartOnlineSessionReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstartonlinesession>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));

	TargetObject->ClientStartOnlineSession_Implementation();

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstartonlinesession>(Op);
}
void FSpatialTypeBinding_PlayerController::ClientSpawnCameraLensEffectReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientspawncameralenseffect>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	TSubclassOf<AEmitterCameraLensEffectBase>  LensEffectEmitterClass = nullptr;
	// UNSUPPORTED ObjectProperty - LensEffectEmitterClass Op.Request.field_lenseffectemitterclass();

	TargetObject->ClientSpawnCameraLensEffect_Implementation(LensEffectEmitterClass);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientspawncameralenseffect>(Op);
}
void FSpatialTypeBinding_PlayerController::ClientSetViewTargetReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetviewtarget>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	AActor* A = nullptr;
	// UNSUPPORTED ObjectProperty - A Op.Request.field_a();
	FViewTargetTransitionParams TransitionParams;
	TransitionParams.BlendTime = Op.Request.field_transitionparams_blendtime();
	// Byte properties are weird, because they can also be an enum in the form TEnumAsByte<...>.
	// Therefore, the code generator needs to cast to either TEnumAsByte<...> or uint8. However,
	// as TEnumAsByte<...> only has a uint8 constructor, we need to cast the SpatialOS value into
	// uint8 first, which causes "uint8(uint8(...))" to be generated for non enum bytes.
	TransitionParams.BlendFunction = TEnumAsByte<EViewTargetBlendFunction>(uint8(Op.Request.field_transitionparams_blendfunction()));
	TransitionParams.BlendExp = Op.Request.field_transitionparams_blendexp();
	TransitionParams.bLockOutgoing = Op.Request.field_transitionparams_blockoutgoing();

	TargetObject->ClientSetViewTarget_Implementation(A, TransitionParams);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetviewtarget>(Op);
}
void FSpatialTypeBinding_PlayerController::ClientSetSpectatorWaitingReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetspectatorwaiting>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	bool bWaiting;
	bWaiting = Op.Request.field_bwaiting();

	TargetObject->ClientSetSpectatorWaiting_Implementation(bWaiting);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetspectatorwaiting>(Op);
}
void FSpatialTypeBinding_PlayerController::ClientSetHUDReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsethud>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	TSubclassOf<AHUD>  NewHUDClass = nullptr;
	// UNSUPPORTED ObjectProperty - NewHUDClass Op.Request.field_newhudclass();

	TargetObject->ClientSetHUD_Implementation(NewHUDClass);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsethud>(Op);
}
void FSpatialTypeBinding_PlayerController::ClientSetForceMipLevelsToBeResidentReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetforcemiplevelstoberesident>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	UMaterialInterface* Material = nullptr;
	// UNSUPPORTED ObjectProperty - Material Op.Request.field_material();
	float ForceDuration;
	ForceDuration = Op.Request.field_forceduration();
	int32 CinematicTextureGroups;
	CinematicTextureGroups = Op.Request.field_cinematictexturegroups();

	TargetObject->ClientSetForceMipLevelsToBeResident_Implementation(Material, ForceDuration, CinematicTextureGroups);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetforcemiplevelstoberesident>(Op);
}
void FSpatialTypeBinding_PlayerController::ClientSetCinematicModeReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetcinematicmode>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	bool bInCinematicMode;
	bInCinematicMode = Op.Request.field_bincinematicmode();
	bool bAffectsMovement;
	bAffectsMovement = Op.Request.field_baffectsmovement();
	bool bAffectsTurning;
	bAffectsTurning = Op.Request.field_baffectsturning();
	bool bAffectsHUD;
	bAffectsHUD = Op.Request.field_baffectshud();

	TargetObject->ClientSetCinematicMode_Implementation(bInCinematicMode, bAffectsMovement, bAffectsTurning, bAffectsHUD);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetcinematicmode>(Op);
}
void FSpatialTypeBinding_PlayerController::ClientSetCameraModeReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetcameramode>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	FName NewCamMode;
	NewCamMode = FName((Op.Request.field_newcammode()).data());

	TargetObject->ClientSetCameraMode_Implementation(NewCamMode);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetcameramode>(Op);
}
void FSpatialTypeBinding_PlayerController::ClientSetCameraFadeReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetcamerafade>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	bool bEnableFading;
	bEnableFading = Op.Request.field_benablefading();
	FColor FadeColor;
	// Byte properties are weird, because they can also be an enum in the form TEnumAsByte<...>.
	// Therefore, the code generator needs to cast to either TEnumAsByte<...> or uint8. However,
	// as TEnumAsByte<...> only has a uint8 constructor, we need to cast the SpatialOS value into
	// uint8 first, which causes "uint8(uint8(...))" to be generated for non enum bytes.
	FadeColor.B = uint8(uint8(Op.Request.field_fadecolor_b()));
	// Byte properties are weird, because they can also be an enum in the form TEnumAsByte<...>.
	// Therefore, the code generator needs to cast to either TEnumAsByte<...> or uint8. However,
	// as TEnumAsByte<...> only has a uint8 constructor, we need to cast the SpatialOS value into
	// uint8 first, which causes "uint8(uint8(...))" to be generated for non enum bytes.
	FadeColor.G = uint8(uint8(Op.Request.field_fadecolor_g()));
	// Byte properties are weird, because they can also be an enum in the form TEnumAsByte<...>.
	// Therefore, the code generator needs to cast to either TEnumAsByte<...> or uint8. However,
	// as TEnumAsByte<...> only has a uint8 constructor, we need to cast the SpatialOS value into
	// uint8 first, which causes "uint8(uint8(...))" to be generated for non enum bytes.
	FadeColor.R = uint8(uint8(Op.Request.field_fadecolor_r()));
	// Byte properties are weird, because they can also be an enum in the form TEnumAsByte<...>.
	// Therefore, the code generator needs to cast to either TEnumAsByte<...> or uint8. However,
	// as TEnumAsByte<...> only has a uint8 constructor, we need to cast the SpatialOS value into
	// uint8 first, which causes "uint8(uint8(...))" to be generated for non enum bytes.
	FadeColor.A = uint8(uint8(Op.Request.field_fadecolor_a()));
	FVector2D FadeAlpha;
	FadeAlpha.X = Op.Request.field_fadealpha_x();
	FadeAlpha.Y = Op.Request.field_fadealpha_y();
	float FadeTime;
	FadeTime = Op.Request.field_fadetime();
	bool bFadeAudio;
	bFadeAudio = Op.Request.field_bfadeaudio();

	TargetObject->ClientSetCameraFade_Implementation(bEnableFading, FadeColor, FadeAlpha, FadeTime, bFadeAudio);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetcamerafade>(Op);
}
void FSpatialTypeBinding_PlayerController::ClientSetBlockOnAsyncLoadingReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetblockonasyncloading>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));

	TargetObject->ClientSetBlockOnAsyncLoading_Implementation();

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetblockonasyncloading>(Op);
}
void FSpatialTypeBinding_PlayerController::ClientReturnToMainMenuReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientreturntomainmenu>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	FString ReturnReason;
	ReturnReason = FString(UTF8_TO_TCHAR(Op.Request.field_returnreason().c_str()));

	TargetObject->ClientReturnToMainMenu_Implementation(ReturnReason);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientreturntomainmenu>(Op);
}
void FSpatialTypeBinding_PlayerController::ClientRetryClientRestartReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientretryclientrestart>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	APawn* NewPawn = nullptr;
	// UNSUPPORTED ObjectProperty - NewPawn Op.Request.field_newpawn();

	TargetObject->ClientRetryClientRestart_Implementation(NewPawn);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientretryclientrestart>(Op);
}
void FSpatialTypeBinding_PlayerController::ClientRestartReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientrestart>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	APawn* NewPawn = nullptr;
	// UNSUPPORTED ObjectProperty - NewPawn Op.Request.field_newpawn();

	TargetObject->ClientRestart_Implementation(NewPawn);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientrestart>(Op);
}
void FSpatialTypeBinding_PlayerController::ClientResetReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientreset>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));

	TargetObject->ClientReset_Implementation();

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientreset>(Op);
}
void FSpatialTypeBinding_PlayerController::ClientRepObjRefReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientrepobjref>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	UObject* Object = nullptr;
	// UNSUPPORTED ObjectProperty - Object Op.Request.field_object();

	TargetObject->ClientRepObjRef_Implementation(Object);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientrepobjref>(Op);
}
void FSpatialTypeBinding_PlayerController::ClientReceiveLocalizedMessageReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientreceivelocalizedmessage>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	TSubclassOf<ULocalMessage>  Message = nullptr;
	// UNSUPPORTED ObjectProperty - Message Op.Request.field_message();
	int32 Switch;
	Switch = Op.Request.field_switch();
	APlayerState* RelatedPlayerState_1 = nullptr;
	// UNSUPPORTED ObjectProperty - RelatedPlayerState_1 Op.Request.field_relatedplayerstate_1();
	APlayerState* RelatedPlayerState_2 = nullptr;
	// UNSUPPORTED ObjectProperty - RelatedPlayerState_2 Op.Request.field_relatedplayerstate_2();
	UObject* OptionalObject = nullptr;
	// UNSUPPORTED ObjectProperty - OptionalObject Op.Request.field_optionalobject();

	TargetObject->ClientReceiveLocalizedMessage_Implementation(Message, Switch, RelatedPlayerState_1, RelatedPlayerState_2, OptionalObject);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientreceivelocalizedmessage>(Op);
}
void FSpatialTypeBinding_PlayerController::ClientPrestreamTexturesReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientprestreamtextures>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	AActor* ForcedActor = nullptr;
	// UNSUPPORTED ObjectProperty - ForcedActor Op.Request.field_forcedactor();
	float ForceDuration;
	ForceDuration = Op.Request.field_forceduration();
	bool bEnableStreaming;
	bEnableStreaming = Op.Request.field_benablestreaming();
	int32 CinematicTextureGroups;
	CinematicTextureGroups = Op.Request.field_cinematictexturegroups();

	TargetObject->ClientPrestreamTextures_Implementation(ForcedActor, ForceDuration, bEnableStreaming, CinematicTextureGroups);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientprestreamtextures>(Op);
}
void FSpatialTypeBinding_PlayerController::ClientPrepareMapChangeReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientpreparemapchange>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	FName LevelName;
	LevelName = FName((Op.Request.field_levelname()).data());
	bool bFirst;
	bFirst = Op.Request.field_bfirst();
	bool bLast;
	bLast = Op.Request.field_blast();

	TargetObject->ClientPrepareMapChange_Implementation(LevelName, bFirst, bLast);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientpreparemapchange>(Op);
}
void FSpatialTypeBinding_PlayerController::ClientPlaySoundAtLocationReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaysoundatlocation>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	USoundBase* Sound = nullptr;
	// UNSUPPORTED ObjectProperty - Sound Op.Request.field_sound();
	FVector Location;
	{
		auto& Vector = Op.Request.field_location();
		Location.X = Vector.x();
		Location.Y = Vector.y();
		Location.Z = Vector.z();
	}
	float VolumeMultiplier;
	VolumeMultiplier = Op.Request.field_volumemultiplier();
	float PitchMultiplier;
	PitchMultiplier = Op.Request.field_pitchmultiplier();

	TargetObject->ClientPlaySoundAtLocation_Implementation(Sound, Location, VolumeMultiplier, PitchMultiplier);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaysoundatlocation>(Op);
}
void FSpatialTypeBinding_PlayerController::ClientPlaySoundReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaysound>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	USoundBase* Sound = nullptr;
	// UNSUPPORTED ObjectProperty - Sound Op.Request.field_sound();
	float VolumeMultiplier;
	VolumeMultiplier = Op.Request.field_volumemultiplier();
	float PitchMultiplier;
	PitchMultiplier = Op.Request.field_pitchmultiplier();

	TargetObject->ClientPlaySound_Implementation(Sound, VolumeMultiplier, PitchMultiplier);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaysound>(Op);
}
void FSpatialTypeBinding_PlayerController::ClientPlayForceFeedbackReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplayforcefeedback>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	UForceFeedbackEffect* ForceFeedbackEffect = nullptr;
	// UNSUPPORTED ObjectProperty - ForceFeedbackEffect Op.Request.field_forcefeedbackeffect();
	bool bLooping;
	bLooping = Op.Request.field_blooping();
	FName Tag;
	Tag = FName((Op.Request.field_tag()).data());

	TargetObject->ClientPlayForceFeedback_Implementation(ForceFeedbackEffect, bLooping, Tag);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplayforcefeedback>(Op);
}
void FSpatialTypeBinding_PlayerController::ClientPlayCameraShakeReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaycamerashake>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	TSubclassOf<UCameraShake>  Shake = nullptr;
	// UNSUPPORTED ObjectProperty - Shake Op.Request.field_shake();
	float Scale;
	Scale = Op.Request.field_scale();
	TEnumAsByte<ECameraAnimPlaySpace::Type> PlaySpace;
	// Byte properties are weird, because they can also be an enum in the form TEnumAsByte<...>.
	// Therefore, the code generator needs to cast to either TEnumAsByte<...> or uint8. However,
	// as TEnumAsByte<...> only has a uint8 constructor, we need to cast the SpatialOS value into
	// uint8 first, which causes "uint8(uint8(...))" to be generated for non enum bytes.
	PlaySpace = TEnumAsByte<ECameraAnimPlaySpace::Type>(uint8(Op.Request.field_playspace()));
	FRotator UserPlaySpaceRot;
	{
		auto& Rotator = Op.Request.field_userplayspacerot();
		UserPlaySpaceRot.Yaw = Rotator.yaw();
		UserPlaySpaceRot.Pitch = Rotator.pitch();
		UserPlaySpaceRot.Roll = Rotator.roll();
	}

	TargetObject->ClientPlayCameraShake_Implementation(Shake, Scale, PlaySpace, UserPlaySpaceRot);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaycamerashake>(Op);
}
void FSpatialTypeBinding_PlayerController::ClientPlayCameraAnimReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaycameraanim>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	UCameraAnim* AnimToPlay = nullptr;
	// UNSUPPORTED ObjectProperty - AnimToPlay Op.Request.field_animtoplay();
	float Scale;
	Scale = Op.Request.field_scale();
	float Rate;
	Rate = Op.Request.field_rate();
	float BlendInTime;
	BlendInTime = Op.Request.field_blendintime();
	float BlendOutTime;
	BlendOutTime = Op.Request.field_blendouttime();
	bool bLoop;
	bLoop = Op.Request.field_bloop();
	bool bRandomStartTime;
	bRandomStartTime = Op.Request.field_brandomstarttime();
	TEnumAsByte<ECameraAnimPlaySpace::Type> Space;
	// Byte properties are weird, because they can also be an enum in the form TEnumAsByte<...>.
	// Therefore, the code generator needs to cast to either TEnumAsByte<...> or uint8. However,
	// as TEnumAsByte<...> only has a uint8 constructor, we need to cast the SpatialOS value into
	// uint8 first, which causes "uint8(uint8(...))" to be generated for non enum bytes.
	Space = TEnumAsByte<ECameraAnimPlaySpace::Type>(uint8(Op.Request.field_space()));
	FRotator CustomPlaySpace;
	{
		auto& Rotator = Op.Request.field_customplayspace();
		CustomPlaySpace.Yaw = Rotator.yaw();
		CustomPlaySpace.Pitch = Rotator.pitch();
		CustomPlaySpace.Roll = Rotator.roll();
	}

	TargetObject->ClientPlayCameraAnim_Implementation(AnimToPlay, Scale, Rate, BlendInTime, BlendOutTime, bLoop, bRandomStartTime, Space, CustomPlaySpace);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaycameraanim>(Op);
}
void FSpatialTypeBinding_PlayerController::ClientMutePlayerReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientmuteplayer>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	FUniqueNetIdRepl PlayerId;
	{
		auto& ValueDataStr = Op.Request.field_playerid();
		TArray<uint8> ValueData;
		ValueData.Append((uint8*)ValueDataStr.data(), ValueDataStr.size());
		FMemoryReader ValueDataReader(ValueData);
		bool bSuccess;
		PlayerId.NetSerialize(ValueDataReader, nullptr, bSuccess);
	}

	TargetObject->ClientMutePlayer_Implementation(PlayerId);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientmuteplayer>(Op);
}
void FSpatialTypeBinding_PlayerController::ClientMessageReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientmessage>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	FString S;
	S = FString(UTF8_TO_TCHAR(Op.Request.field_s().c_str()));
	FName Type;
	Type = FName((Op.Request.field_type()).data());
	float MsgLifeTime;
	MsgLifeTime = Op.Request.field_msglifetime();

	TargetObject->ClientMessage_Implementation(S, Type, MsgLifeTime);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientmessage>(Op);
}
void FSpatialTypeBinding_PlayerController::ClientIgnoreMoveInputReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientignoremoveinput>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	bool bIgnore;
	bIgnore = Op.Request.field_bignore();

	TargetObject->ClientIgnoreMoveInput_Implementation(bIgnore);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientignoremoveinput>(Op);
}
void FSpatialTypeBinding_PlayerController::ClientIgnoreLookInputReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientignorelookinput>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	bool bIgnore;
	bIgnore = Op.Request.field_bignore();

	TargetObject->ClientIgnoreLookInput_Implementation(bIgnore);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientignorelookinput>(Op);
}
void FSpatialTypeBinding_PlayerController::ClientGotoStateReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientgotostate>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	FName NewState;
	NewState = FName((Op.Request.field_newstate()).data());

	TargetObject->ClientGotoState_Implementation(NewState);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientgotostate>(Op);
}
void FSpatialTypeBinding_PlayerController::ClientGameEndedReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientgameended>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	AActor* EndGameFocus = nullptr;
	// UNSUPPORTED ObjectProperty - EndGameFocus Op.Request.field_endgamefocus();
	bool bIsWinner;
	bIsWinner = Op.Request.field_biswinner();

	TargetObject->ClientGameEnded_Implementation(EndGameFocus, bIsWinner);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientgameended>(Op);
}
void FSpatialTypeBinding_PlayerController::ClientForceGarbageCollectionReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientforcegarbagecollection>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));

	TargetObject->ClientForceGarbageCollection_Implementation();

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientforcegarbagecollection>(Op);
}
void FSpatialTypeBinding_PlayerController::ClientFlushLevelStreamingReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientflushlevelstreaming>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));

	TargetObject->ClientFlushLevelStreaming_Implementation();

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientflushlevelstreaming>(Op);
}
void FSpatialTypeBinding_PlayerController::ClientEndOnlineSessionReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientendonlinesession>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));

	TargetObject->ClientEndOnlineSession_Implementation();

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientendonlinesession>(Op);
}
void FSpatialTypeBinding_PlayerController::ClientEnableNetworkVoiceReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientenablenetworkvoice>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	bool bEnable;
	bEnable = Op.Request.field_benable();

	TargetObject->ClientEnableNetworkVoice_Implementation(bEnable);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientenablenetworkvoice>(Op);
}
void FSpatialTypeBinding_PlayerController::ClientCommitMapChangeReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientcommitmapchange>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));

	TargetObject->ClientCommitMapChange_Implementation();

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientcommitmapchange>(Op);
}
void FSpatialTypeBinding_PlayerController::ClientClearCameraLensEffectsReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientclearcameralenseffects>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));

	TargetObject->ClientClearCameraLensEffects_Implementation();

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientclearcameralenseffects>(Op);
}
void FSpatialTypeBinding_PlayerController::ClientCapBandwidthReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientcapbandwidth>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	int32 Cap;
	Cap = Op.Request.field_cap();

	TargetObject->ClientCapBandwidth_Implementation(Cap);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientcapbandwidth>(Op);
}
void FSpatialTypeBinding_PlayerController::ClientCancelPendingMapChangeReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientcancelpendingmapchange>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));

	TargetObject->ClientCancelPendingMapChange_Implementation();

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientcancelpendingmapchange>(Op);
}
void FSpatialTypeBinding_PlayerController::ClientAddTextureStreamingLocReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientaddtexturestreamingloc>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	FVector InLoc;
	{
		auto& Vector = Op.Request.field_inloc();
		InLoc.X = Vector.x();
		InLoc.Y = Vector.y();
		InLoc.Z = Vector.z();
	}
	float Duration;
	Duration = Op.Request.field_duration();
	bool bOverrideLocation;
	bOverrideLocation = Op.Request.field_boverridelocation();

	TargetObject->ClientAddTextureStreamingLoc_Implementation(InLoc, Duration, bOverrideLocation);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientaddtexturestreamingloc>(Op);
}
void FSpatialTypeBinding_PlayerController::ClientSetRotationReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetrotation>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	FRotator NewRotation;
	{
		auto& Rotator = Op.Request.field_newrotation();
		NewRotation.Yaw = Rotator.yaw();
		NewRotation.Pitch = Rotator.pitch();
		NewRotation.Roll = Rotator.roll();
	}
	bool bResetCamera;
	bResetCamera = Op.Request.field_bresetcamera();

	TargetObject->ClientSetRotation_Implementation(NewRotation, bResetCamera);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetrotation>(Op);
}
void FSpatialTypeBinding_PlayerController::ClientSetLocationReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetlocation>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	FVector NewLocation;
	{
		auto& Vector = Op.Request.field_newlocation();
		NewLocation.X = Vector.x();
		NewLocation.Y = Vector.y();
		NewLocation.Z = Vector.z();
	}
	FRotator NewRotation;
	{
		auto& Rotator = Op.Request.field_newrotation();
		NewRotation.Yaw = Rotator.yaw();
		NewRotation.Pitch = Rotator.pitch();
		NewRotation.Roll = Rotator.roll();
	}

	TargetObject->ClientSetLocation_Implementation(NewLocation, NewRotation);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetlocation>(Op);
}
void FSpatialTypeBinding_PlayerController::ServerViewSelfReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverviewself>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	FViewTargetTransitionParams TransitionParams;
	TransitionParams.BlendTime = Op.Request.field_transitionparams_blendtime();
	// Byte properties are weird, because they can also be an enum in the form TEnumAsByte<...>.
	// Therefore, the code generator needs to cast to either TEnumAsByte<...> or uint8. However,
	// as TEnumAsByte<...> only has a uint8 constructor, we need to cast the SpatialOS value into
	// uint8 first, which causes "uint8(uint8(...))" to be generated for non enum bytes.
	TransitionParams.BlendFunction = TEnumAsByte<EViewTargetBlendFunction>(uint8(Op.Request.field_transitionparams_blendfunction()));
	TransitionParams.BlendExp = Op.Request.field_transitionparams_blendexp();
	TransitionParams.bLockOutgoing = Op.Request.field_transitionparams_blockoutgoing();

	TargetObject->ServerViewSelf_Implementation(TransitionParams);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverviewself>(Op);
}
void FSpatialTypeBinding_PlayerController::ServerViewPrevPlayerReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverviewprevplayer>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));

	TargetObject->ServerViewPrevPlayer_Implementation();

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverviewprevplayer>(Op);
}
void FSpatialTypeBinding_PlayerController::ServerViewNextPlayerReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverviewnextplayer>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));

	TargetObject->ServerViewNextPlayer_Implementation();

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverviewnextplayer>(Op);
}
void FSpatialTypeBinding_PlayerController::ServerVerifyViewTargetReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serververifyviewtarget>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));

	TargetObject->ServerVerifyViewTarget_Implementation();

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serververifyviewtarget>(Op);
}
void FSpatialTypeBinding_PlayerController::ServerUpdateLevelVisibilityReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverupdatelevelvisibility>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	FName PackageName;
	PackageName = FName((Op.Request.field_packagename()).data());
	bool bIsVisible;
	bIsVisible = Op.Request.field_bisvisible();

	TargetObject->ServerUpdateLevelVisibility_Implementation(PackageName, bIsVisible);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverupdatelevelvisibility>(Op);
}
void FSpatialTypeBinding_PlayerController::ServerUpdateCameraReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverupdatecamera>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	FVector_NetQuantize CamLoc;
	{
		auto& Vector = Op.Request.field_camloc();
		CamLoc.X = Vector.x();
		CamLoc.Y = Vector.y();
		CamLoc.Z = Vector.z();
	}
	int32 CamPitchAndYaw;
	CamPitchAndYaw = Op.Request.field_campitchandyaw();

	TargetObject->ServerUpdateCamera_Implementation(CamLoc, CamPitchAndYaw);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverupdatecamera>(Op);
}
void FSpatialTypeBinding_PlayerController::ServerUnmutePlayerReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverunmuteplayer>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	FUniqueNetIdRepl PlayerId;
	{
		auto& ValueDataStr = Op.Request.field_playerid();
		TArray<uint8> ValueData;
		ValueData.Append((uint8*)ValueDataStr.data(), ValueDataStr.size());
		FMemoryReader ValueDataReader(ValueData);
		bool bSuccess;
		PlayerId.NetSerialize(ValueDataReader, nullptr, bSuccess);
	}

	TargetObject->ServerUnmutePlayer_Implementation(PlayerId);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverunmuteplayer>(Op);
}
void FSpatialTypeBinding_PlayerController::ServerToggleAILoggingReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servertoggleailogging>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));

	TargetObject->ServerToggleAILogging_Implementation();

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servertoggleailogging>(Op);
}
void FSpatialTypeBinding_PlayerController::ServerShortTimeoutReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servershorttimeout>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));

	TargetObject->ServerShortTimeout_Implementation();

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servershorttimeout>(Op);
}
void FSpatialTypeBinding_PlayerController::ServerSetSpectatorWaitingReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serversetspectatorwaiting>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	bool bWaiting;
	bWaiting = Op.Request.field_bwaiting();

	TargetObject->ServerSetSpectatorWaiting_Implementation(bWaiting);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serversetspectatorwaiting>(Op);
}
void FSpatialTypeBinding_PlayerController::ServerSetSpectatorLocationReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serversetspectatorlocation>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	FVector NewLoc;
	{
		auto& Vector = Op.Request.field_newloc();
		NewLoc.X = Vector.x();
		NewLoc.Y = Vector.y();
		NewLoc.Z = Vector.z();
	}
	FRotator NewRot;
	{
		auto& Rotator = Op.Request.field_newrot();
		NewRot.Yaw = Rotator.yaw();
		NewRot.Pitch = Rotator.pitch();
		NewRot.Roll = Rotator.roll();
	}

	TargetObject->ServerSetSpectatorLocation_Implementation(NewLoc, NewRot);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serversetspectatorlocation>(Op);
}
void FSpatialTypeBinding_PlayerController::ServerRestartPlayerReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverrestartplayer>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));

	TargetObject->ServerRestartPlayer_Implementation();

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverrestartplayer>(Op);
}
void FSpatialTypeBinding_PlayerController::ServerPauseReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverpause>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));

	TargetObject->ServerPause_Implementation();

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverpause>(Op);
}
void FSpatialTypeBinding_PlayerController::ServerNotifyLoadedWorldReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servernotifyloadedworld>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	FName WorldPackageName;
	WorldPackageName = FName((Op.Request.field_worldpackagename()).data());

	TargetObject->ServerNotifyLoadedWorld_Implementation(WorldPackageName);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servernotifyloadedworld>(Op);
}
void FSpatialTypeBinding_PlayerController::ServerMutePlayerReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servermuteplayer>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	FUniqueNetIdRepl PlayerId;
	{
		auto& ValueDataStr = Op.Request.field_playerid();
		TArray<uint8> ValueData;
		ValueData.Append((uint8*)ValueDataStr.data(), ValueDataStr.size());
		FMemoryReader ValueDataReader(ValueData);
		bool bSuccess;
		PlayerId.NetSerialize(ValueDataReader, nullptr, bSuccess);
	}

	TargetObject->ServerMutePlayer_Implementation(PlayerId);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servermuteplayer>(Op);
}
void FSpatialTypeBinding_PlayerController::ServerCheckClientPossessionReliableReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servercheckclientpossessionreliable>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));

	TargetObject->ServerCheckClientPossessionReliable_Implementation();

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servercheckclientpossessionreliable>(Op);
}
void FSpatialTypeBinding_PlayerController::ServerCheckClientPossessionReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servercheckclientpossession>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));

	TargetObject->ServerCheckClientPossession_Implementation();

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servercheckclientpossession>(Op);
}
void FSpatialTypeBinding_PlayerController::ServerChangeNameReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverchangename>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	FString S;
	S = FString(UTF8_TO_TCHAR(Op.Request.field_s().c_str()));

	TargetObject->ServerChangeName_Implementation(S);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverchangename>(Op);
}
void FSpatialTypeBinding_PlayerController::ServerCameraReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servercamera>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	FName NewMode;
	NewMode = FName((Op.Request.field_newmode()).data());

	TargetObject->ServerCamera_Implementation(NewMode);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servercamera>(Op);
}
void FSpatialTypeBinding_PlayerController::ServerAcknowledgePossessionReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serveracknowledgepossession>& Op)
{
	// This is just hardcoded to a known entity for now. Once the PackageMap stuff is in, we need to get the correct object from that
	APlayerController* TargetObject = Cast<APlayerController>(UpdateInterop->GetNetDriver()->GuidCache.Get()->GetObjectFromNetGUID(Op.EntityId, false));
	APawn* P = nullptr;
	// UNSUPPORTED ObjectProperty - P Op.Request.field_p();

	TargetObject->ServerAcknowledgePossession_Implementation(P);

	SendRPCResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serveracknowledgepossession>(Op);
}
