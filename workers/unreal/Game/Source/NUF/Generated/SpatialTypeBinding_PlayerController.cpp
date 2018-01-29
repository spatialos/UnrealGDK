// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// Note that this file has been generated automatically

#include "SpatialTypeBinding_PlayerController.h"
#include "SpatialOS.h"
#include "Engine.h"
#include "SpatialActorChannel.h"
#include "EntityBuilder.h"
#include "SpatialPackageMapClient.h"
#include "Utils/BunchReader.h"
#include "SpatialNetDriver.h"
#include "SpatialUpdateInterop.h"

namespace {

void ApplyUpdateToSpatial_SingleClient_PlayerController(const uint8* RESTRICT Data, int32 Handle, UProperty* Property, UPackageMap* PackageMap, improbable::unreal::UnrealPlayerControllerSingleClientReplicatedData::Update& Update)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	check(SpatialPMC);
	switch (Handle)
	{
		case 18: // field_targetviewrotation
		{
			FRotator Value;
			Value = *(reinterpret_cast<const FRotator*>(Data));

			Update.set_field_targetviewrotation(improbable::unreal::UnrealFRotator(Value.Yaw, Value.Pitch, Value.Roll));
			break;
		}
		case 19: // field_spawnlocation
		{
			FVector Value;
			Value = *(reinterpret_cast<const FVector*>(Data));

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
	auto& HandleToPropertyMap = USpatialTypeBinding_PlayerController::GetHandlePropertyMap();
	USpatialActorChannel* ActorChannel = UpdateInterop->GetClientActorChannel(Op.EntityId);
	if (!ActorChannel)
	{
		return;
	}
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	check(SpatialPMC);
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

void ApplyUpdateToSpatial_MultiClient_PlayerController(const uint8* RESTRICT Data, int32 Handle, UProperty* Property, UPackageMap* PackageMap, improbable::unreal::UnrealPlayerControllerMultiClientReplicatedData::Update& Update)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	check(SpatialPMC);
	switch (Handle)
	{
		case 1: // field_bhidden
		{
			uint8 Value;
			Value = *(reinterpret_cast<const uint8*>(Data));

			Update.set_field_bhidden(Value != 0);
			break;
		}
		case 2: // field_breplicatemovement
		{
			uint8 Value;
			Value = *(reinterpret_cast<const uint8*>(Data));

			Update.set_field_breplicatemovement(Value != 0);
			break;
		}
		case 3: // field_btearoff
		{
			uint8 Value;
			Value = *(reinterpret_cast<const uint8*>(Data));

			Update.set_field_btearoff(Value != 0);
			break;
		}
		case 4: // field_remoterole
		{
			TEnumAsByte<ENetRole> Value;
			Value = *(reinterpret_cast<const TEnumAsByte<ENetRole>*>(Data));

			Update.set_field_remoterole(uint32_t(Value));
			break;
		}
		case 5: // field_owner
		{
			AActor* Value;
			Value = *(reinterpret_cast<AActor* const*>(Data));

			{
				FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromObject(Value);
				improbable::unreal::UnrealObjectRef UObjectRef = SpatialPMC->GetUnrealObjectRefFromNetGUID(NetGUID);
				Update.set_field_owner(UObjectRef);
			}
			break;
		}
		case 6: // field_replicatedmovement
		{
			FRepMovement Value;
			Value = *(reinterpret_cast<const FRepMovement*>(Data));

			{
				TArray<uint8> ValueData;
				FMemoryWriter ValueDataWriter(ValueData);
				bool Success;
				Value.NetSerialize(ValueDataWriter, nullptr, Success);
				Update.set_field_replicatedmovement(std::string((char*)ValueData.GetData(), ValueData.Num()));
			}
			break;
		}
		case 7: // field_attachmentreplication_attachparent
		{
			AActor* Value;
			Value = *(reinterpret_cast<AActor* const*>(Data));

			{
				FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromObject(Value);
				improbable::unreal::UnrealObjectRef UObjectRef = SpatialPMC->GetUnrealObjectRefFromNetGUID(NetGUID);
				Update.set_field_attachmentreplication_attachparent(UObjectRef);
			}
			break;
		}
		case 8: // field_attachmentreplication_locationoffset
		{
			FVector_NetQuantize100 Value;
			Value = *(reinterpret_cast<const FVector_NetQuantize100*>(Data));

			Update.set_field_attachmentreplication_locationoffset(improbable::Vector3f(Value.X, Value.Y, Value.Z));
			break;
		}
		case 9: // field_attachmentreplication_relativescale3d
		{
			FVector_NetQuantize100 Value;
			Value = *(reinterpret_cast<const FVector_NetQuantize100*>(Data));

			Update.set_field_attachmentreplication_relativescale3d(improbable::Vector3f(Value.X, Value.Y, Value.Z));
			break;
		}
		case 10: // field_attachmentreplication_rotationoffset
		{
			FRotator Value;
			Value = *(reinterpret_cast<const FRotator*>(Data));

			Update.set_field_attachmentreplication_rotationoffset(improbable::unreal::UnrealFRotator(Value.Yaw, Value.Pitch, Value.Roll));
			break;
		}
		case 11: // field_attachmentreplication_attachsocket
		{
			FName Value;
			Value = *(reinterpret_cast<const FName*>(Data));

			Update.set_field_attachmentreplication_attachsocket(TCHAR_TO_UTF8(*Value.ToString()));
			break;
		}
		case 12: // field_attachmentreplication_attachcomponent
		{
			USceneComponent* Value;
			Value = *(reinterpret_cast<USceneComponent* const*>(Data));

			{
				FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromObject(Value);
				improbable::unreal::UnrealObjectRef UObjectRef = SpatialPMC->GetUnrealObjectRefFromNetGUID(NetGUID);
				Update.set_field_attachmentreplication_attachcomponent(UObjectRef);
			}
			break;
		}
		case 13: // field_role
		{
			TEnumAsByte<ENetRole> Value;
			Value = *(reinterpret_cast<const TEnumAsByte<ENetRole>*>(Data));

			Update.set_field_role(uint32_t(Value));
			break;
		}
		case 14: // field_bcanbedamaged
		{
			uint8 Value;
			Value = *(reinterpret_cast<const uint8*>(Data));

			Update.set_field_bcanbedamaged(Value != 0);
			break;
		}
		case 15: // field_instigator
		{
			APawn* Value;
			Value = *(reinterpret_cast<APawn* const*>(Data));

			{
				FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromObject(Value);
				improbable::unreal::UnrealObjectRef UObjectRef = SpatialPMC->GetUnrealObjectRefFromNetGUID(NetGUID);
				Update.set_field_instigator(UObjectRef);
			}
			break;
		}
		case 16: // field_pawn
		{
			APawn* Value;
			Value = *(reinterpret_cast<APawn* const*>(Data));

			{
				FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromObject(Value);
				improbable::unreal::UnrealObjectRef UObjectRef = SpatialPMC->GetUnrealObjectRefFromNetGUID(NetGUID);
				Update.set_field_pawn(UObjectRef);
			}
			break;
		}
		case 17: // field_playerstate
		{
			APlayerState* Value;
			Value = *(reinterpret_cast<APlayerState* const*>(Data));

			{
				FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromObject(Value);
				improbable::unreal::UnrealObjectRef UObjectRef = SpatialPMC->GetUnrealObjectRefFromNetGUID(NetGUID);
				Update.set_field_playerstate(UObjectRef);
			}
			break;
		}
	default:
		checkf(false, TEXT("Unknown replication handle %d encountered when creating a SpatialOS update."));
		break;
	}
}

void ReceiveUpdateFromSpatial_MultiClient_PlayerController(USpatialUpdateInterop* UpdateInterop, UPackageMap* PackageMap, const worker::ComponentUpdateOp<improbable::unreal::UnrealPlayerControllerMultiClientReplicatedData>& Op)
{
	FNetBitWriter OutputWriter(nullptr, 0); 
	auto& HandleToPropertyMap = USpatialTypeBinding_PlayerController::GetHandlePropertyMap();
	USpatialActorChannel* ActorChannel = UpdateInterop->GetClientActorChannel(Op.EntityId);
	if (!ActorChannel)
	{
		return;
	}
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	check(SpatialPMC);
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

			{
				improbable::unreal::UnrealObjectRef TargetObject = *(Op.Update.field_owner().data());
				FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromUnrealObjectRef(TargetObject);
				Value = static_cast<AActor*>(SpatialPMC->GetObjectFromNetGUID(NetGUID, true));
			}

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

			{
				improbable::unreal::UnrealObjectRef TargetObject = *(Op.Update.field_attachmentreplication_attachparent().data());
				FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromUnrealObjectRef(TargetObject);
				Value = static_cast<AActor*>(SpatialPMC->GetObjectFromNetGUID(NetGUID, true));
			}

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

			{
				improbable::unreal::UnrealObjectRef TargetObject = *(Op.Update.field_attachmentreplication_attachcomponent().data());
				FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromUnrealObjectRef(TargetObject);
				Value = static_cast<USceneComponent*>(SpatialPMC->GetObjectFromNetGUID(NetGUID, true));
			}

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

			{
				improbable::unreal::UnrealObjectRef TargetObject = *(Op.Update.field_instigator().data());
				FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromUnrealObjectRef(TargetObject);
				Value = static_cast<APawn*>(SpatialPMC->GetObjectFromNetGUID(NetGUID, true));
			}

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

			{
				improbable::unreal::UnrealObjectRef TargetObject = *(Op.Update.field_pawn().data());
				FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromUnrealObjectRef(TargetObject);
				Value = static_cast<APawn*>(SpatialPMC->GetObjectFromNetGUID(NetGUID, true));
			}

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

			{
				improbable::unreal::UnrealObjectRef TargetObject = *(Op.Update.field_playerstate().data());
				FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromUnrealObjectRef(TargetObject);
				Value = static_cast<APlayerState*>(SpatialPMC->GetObjectFromNetGUID(NetGUID, true));
			}

			Data.Property->NetSerializeItem(OutputWriter, PackageMap, &Value);
			UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
		}
	}
	UpdateInterop->ReceiveSpatialUpdate(ActorChannel, OutputWriter);
}

// RPC sender functions

void OnServerStartedVisualLoggerSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	FFrame& Stack = *RPCFrame;
	P_GET_UBOOL(bIsLogging);

	improbable::unreal::UnrealOnServerStartedVisualLoggerRequest Request;
	Request.set_field_bislogging(bIsLogging != 0);

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Onserverstartedvisuallogger>(Target, Request, 0);
}

void ClientWasKickedSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UTextProperty, KickReason);

	improbable::unreal::UnrealClientWasKickedRequest Request;
	// UNSUPPORTED

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientwaskicked>(Target, Request, 0);
}

void ClientVoiceHandshakeCompleteSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	improbable::unreal::UnrealClientVoiceHandshakeCompleteRequest Request;

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientvoicehandshakecomplete>(Target, Request, 0);
}

void ClientUpdateLevelStreamingStatusSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
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

void ClientUnmutePlayerSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
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

void ClientTravelInternalSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
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

void ClientTeamMessageSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(APlayerState, SenderPlayerState);
	P_GET_PROPERTY(UStrProperty, S);
	P_GET_PROPERTY(UNameProperty, Type);
	P_GET_PROPERTY(UFloatProperty, MsgLifeTime);

	improbable::unreal::UnrealClientTeamMessageRequest Request;
	{
		FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromObject(SenderPlayerState);
		improbable::unreal::UnrealObjectRef UObjectRef = SpatialPMC->GetUnrealObjectRefFromNetGUID(NetGUID);
		Request.set_field_senderplayerstate(UObjectRef);
	}
	Request.set_field_s(TCHAR_TO_UTF8(*S));
	Request.set_field_type(TCHAR_TO_UTF8(*Type.ToString()));
	Request.set_field_msglifetime(MsgLifeTime);

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientteammessage>(Target, Request, 0);
}

void ClientStopForceFeedbackSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(UForceFeedbackEffect, ForceFeedbackEffect);
	P_GET_PROPERTY(UNameProperty, Tag);

	improbable::unreal::UnrealClientStopForceFeedbackRequest Request;
	{
		FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromObject(ForceFeedbackEffect);
		improbable::unreal::UnrealObjectRef UObjectRef = SpatialPMC->GetUnrealObjectRefFromNetGUID(NetGUID);
		Request.set_field_forcefeedbackeffect(UObjectRef);
	}
	Request.set_field_tag(TCHAR_TO_UTF8(*Tag.ToString()));

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstopforcefeedback>(Target, Request, 0);
}

void ClientStopCameraShakeSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(UClass, Shake);
	P_GET_UBOOL(bImmediately);

	improbable::unreal::UnrealClientStopCameraShakeRequest Request;
	//Not yet implemented UClass properties
	Request.set_field_bimmediately(bImmediately != 0);

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstopcamerashake>(Target, Request, 0);
}

void ClientStopCameraAnimSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(UCameraAnim, AnimToStop);

	improbable::unreal::UnrealClientStopCameraAnimRequest Request;
	{
		FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromObject(AnimToStop);
		improbable::unreal::UnrealObjectRef UObjectRef = SpatialPMC->GetUnrealObjectRefFromNetGUID(NetGUID);
		Request.set_field_animtostop(UObjectRef);
	}

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstopcameraanim>(Target, Request, 0);
}

void ClientStartOnlineSessionSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	improbable::unreal::UnrealClientStartOnlineSessionRequest Request;

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstartonlinesession>(Target, Request, 0);
}

void ClientSpawnCameraLensEffectSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(UClass, LensEffectEmitterClass);

	improbable::unreal::UnrealClientSpawnCameraLensEffectRequest Request;
	//Not yet implemented UClass properties

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientspawncameralenseffect>(Target, Request, 0);
}

void ClientSetViewTargetSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(AActor, A);
	P_GET_STRUCT(FViewTargetTransitionParams, TransitionParams)

	improbable::unreal::UnrealClientSetViewTargetRequest Request;
	{
		FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromObject(A);
		improbable::unreal::UnrealObjectRef UObjectRef = SpatialPMC->GetUnrealObjectRefFromNetGUID(NetGUID);
		Request.set_field_a(UObjectRef);
	}
	Request.set_field_transitionparams_blendtime(TransitionParams.BlendTime);
	Request.set_field_transitionparams_blendfunction(uint32_t(TransitionParams.BlendFunction));
	Request.set_field_transitionparams_blendexp(TransitionParams.BlendExp);
	Request.set_field_transitionparams_blockoutgoing(TransitionParams.bLockOutgoing != 0);

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetviewtarget>(Target, Request, 0);
}

void ClientSetSpectatorWaitingSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	FFrame& Stack = *RPCFrame;
	P_GET_UBOOL(bWaiting);

	improbable::unreal::UnrealClientSetSpectatorWaitingRequest Request;
	Request.set_field_bwaiting(bWaiting != 0);

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetspectatorwaiting>(Target, Request, 0);
}

void ClientSetHUDSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(UClass, NewHUDClass);

	improbable::unreal::UnrealClientSetHUDRequest Request;
	//Not yet implemented UClass properties

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsethud>(Target, Request, 0);
}

void ClientSetForceMipLevelsToBeResidentSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(UMaterialInterface, Material);
	P_GET_PROPERTY(UFloatProperty, ForceDuration);
	P_GET_PROPERTY(UIntProperty, CinematicTextureGroups);

	improbable::unreal::UnrealClientSetForceMipLevelsToBeResidentRequest Request;
	{
		FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromObject(Material);
		improbable::unreal::UnrealObjectRef UObjectRef = SpatialPMC->GetUnrealObjectRefFromNetGUID(NetGUID);
		Request.set_field_material(UObjectRef);
	}
	Request.set_field_forceduration(ForceDuration);
	Request.set_field_cinematictexturegroups(CinematicTextureGroups);

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetforcemiplevelstoberesident>(Target, Request, 0);
}

void ClientSetCinematicModeSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
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

void ClientSetCameraModeSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UNameProperty, NewCamMode);

	improbable::unreal::UnrealClientSetCameraModeRequest Request;
	Request.set_field_newcammode(TCHAR_TO_UTF8(*NewCamMode.ToString()));

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetcameramode>(Target, Request, 0);
}

void ClientSetCameraFadeSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
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

void ClientSetBlockOnAsyncLoadingSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	improbable::unreal::UnrealClientSetBlockOnAsyncLoadingRequest Request;

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetblockonasyncloading>(Target, Request, 0);
}

void ClientReturnToMainMenuSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UStrProperty, ReturnReason);

	improbable::unreal::UnrealClientReturnToMainMenuRequest Request;
	Request.set_field_returnreason(TCHAR_TO_UTF8(*ReturnReason));

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientreturntomainmenu>(Target, Request, 0);
}

void ClientRetryClientRestartSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(APawn, NewPawn);

	improbable::unreal::UnrealClientRetryClientRestartRequest Request;
	{
		FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromObject(NewPawn);
		improbable::unreal::UnrealObjectRef UObjectRef = SpatialPMC->GetUnrealObjectRefFromNetGUID(NetGUID);
		Request.set_field_newpawn(UObjectRef);
	}

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientretryclientrestart>(Target, Request, 0);
}

void ClientRestartSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(APawn, NewPawn);

	improbable::unreal::UnrealClientRestartRequest Request;
	{
		FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromObject(NewPawn);
		improbable::unreal::UnrealObjectRef UObjectRef = SpatialPMC->GetUnrealObjectRefFromNetGUID(NetGUID);
		Request.set_field_newpawn(UObjectRef);
	}

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientrestart>(Target, Request, 0);
}

void ClientResetSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	improbable::unreal::UnrealClientResetRequest Request;

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientreset>(Target, Request, 0);
}

void ClientRepObjRefSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(UObject, Object);

	improbable::unreal::UnrealClientRepObjRefRequest Request;
	{
		FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromObject(Object);
		improbable::unreal::UnrealObjectRef UObjectRef = SpatialPMC->GetUnrealObjectRefFromNetGUID(NetGUID);
		Request.set_field_object(UObjectRef);
	}

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientrepobjref>(Target, Request, 0);
}

void ClientReceiveLocalizedMessageSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(UClass, Message);
	P_GET_PROPERTY(UIntProperty, Switch);
	P_GET_OBJECT(APlayerState, RelatedPlayerState_1);
	P_GET_OBJECT(APlayerState, RelatedPlayerState_2);
	P_GET_OBJECT(UObject, OptionalObject);

	improbable::unreal::UnrealClientReceiveLocalizedMessageRequest Request;
	//Not yet implemented UClass properties
	Request.set_field_switch(Switch);
	{
		FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromObject(RelatedPlayerState_1);
		improbable::unreal::UnrealObjectRef UObjectRef = SpatialPMC->GetUnrealObjectRefFromNetGUID(NetGUID);
		Request.set_field_relatedplayerstate_1(UObjectRef);
	}
	{
		FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromObject(RelatedPlayerState_2);
		improbable::unreal::UnrealObjectRef UObjectRef = SpatialPMC->GetUnrealObjectRefFromNetGUID(NetGUID);
		Request.set_field_relatedplayerstate_2(UObjectRef);
	}
	{
		FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromObject(OptionalObject);
		improbable::unreal::UnrealObjectRef UObjectRef = SpatialPMC->GetUnrealObjectRefFromNetGUID(NetGUID);
		Request.set_field_optionalobject(UObjectRef);
	}

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientreceivelocalizedmessage>(Target, Request, 0);
}

void ClientPrestreamTexturesSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(AActor, ForcedActor);
	P_GET_PROPERTY(UFloatProperty, ForceDuration);
	P_GET_UBOOL(bEnableStreaming);
	P_GET_PROPERTY(UIntProperty, CinematicTextureGroups);

	improbable::unreal::UnrealClientPrestreamTexturesRequest Request;
	{
		FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromObject(ForcedActor);
		improbable::unreal::UnrealObjectRef UObjectRef = SpatialPMC->GetUnrealObjectRefFromNetGUID(NetGUID);
		Request.set_field_forcedactor(UObjectRef);
	}
	Request.set_field_forceduration(ForceDuration);
	Request.set_field_benablestreaming(bEnableStreaming != 0);
	Request.set_field_cinematictexturegroups(CinematicTextureGroups);

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientprestreamtextures>(Target, Request, 0);
}

void ClientPrepareMapChangeSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
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

void ClientPlaySoundAtLocationSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(USoundBase, Sound);
	P_GET_STRUCT(FVector, Location)
	P_GET_PROPERTY(UFloatProperty, VolumeMultiplier);
	P_GET_PROPERTY(UFloatProperty, PitchMultiplier);

	improbable::unreal::UnrealClientPlaySoundAtLocationRequest Request;
	{
		FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromObject(Sound);
		improbable::unreal::UnrealObjectRef UObjectRef = SpatialPMC->GetUnrealObjectRefFromNetGUID(NetGUID);
		Request.set_field_sound(UObjectRef);
	}
	Request.set_field_location(improbable::Vector3f(Location.X, Location.Y, Location.Z));
	Request.set_field_volumemultiplier(VolumeMultiplier);
	Request.set_field_pitchmultiplier(PitchMultiplier);

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaysoundatlocation>(Target, Request, 0);
}

void ClientPlaySoundSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(USoundBase, Sound);
	P_GET_PROPERTY(UFloatProperty, VolumeMultiplier);
	P_GET_PROPERTY(UFloatProperty, PitchMultiplier);

	improbable::unreal::UnrealClientPlaySoundRequest Request;
	{
		FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromObject(Sound);
		improbable::unreal::UnrealObjectRef UObjectRef = SpatialPMC->GetUnrealObjectRefFromNetGUID(NetGUID);
		Request.set_field_sound(UObjectRef);
	}
	Request.set_field_volumemultiplier(VolumeMultiplier);
	Request.set_field_pitchmultiplier(PitchMultiplier);

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaysound>(Target, Request, 0);
}

void ClientPlayForceFeedbackSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(UForceFeedbackEffect, ForceFeedbackEffect);
	P_GET_UBOOL(bLooping);
	P_GET_PROPERTY(UNameProperty, Tag);

	improbable::unreal::UnrealClientPlayForceFeedbackRequest Request;
	{
		FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromObject(ForceFeedbackEffect);
		improbable::unreal::UnrealObjectRef UObjectRef = SpatialPMC->GetUnrealObjectRefFromNetGUID(NetGUID);
		Request.set_field_forcefeedbackeffect(UObjectRef);
	}
	Request.set_field_blooping(bLooping != 0);
	Request.set_field_tag(TCHAR_TO_UTF8(*Tag.ToString()));

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplayforcefeedback>(Target, Request, 0);
}

void ClientPlayCameraShakeSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(UClass, Shake);
	P_GET_PROPERTY(UFloatProperty, Scale);
	P_GET_PROPERTY(UByteProperty, PlaySpace);
	P_GET_STRUCT(FRotator, UserPlaySpaceRot)

	improbable::unreal::UnrealClientPlayCameraShakeRequest Request;
	//Not yet implemented UClass properties
	Request.set_field_scale(Scale);
	Request.set_field_playspace(uint32_t(PlaySpace));
	Request.set_field_userplayspacerot(improbable::unreal::UnrealFRotator(UserPlaySpaceRot.Yaw, UserPlaySpaceRot.Pitch, UserPlaySpaceRot.Roll));

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaycamerashake>(Target, Request, 0);
}

void ClientPlayCameraAnimSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
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
	{
		FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromObject(AnimToPlay);
		improbable::unreal::UnrealObjectRef UObjectRef = SpatialPMC->GetUnrealObjectRefFromNetGUID(NetGUID);
		Request.set_field_animtoplay(UObjectRef);
	}
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

void ClientMutePlayerSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
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

void ClientMessageSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
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

void ClientIgnoreMoveInputSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	FFrame& Stack = *RPCFrame;
	P_GET_UBOOL(bIgnore);

	improbable::unreal::UnrealClientIgnoreMoveInputRequest Request;
	Request.set_field_bignore(bIgnore != 0);

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientignoremoveinput>(Target, Request, 0);
}

void ClientIgnoreLookInputSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	FFrame& Stack = *RPCFrame;
	P_GET_UBOOL(bIgnore);

	improbable::unreal::UnrealClientIgnoreLookInputRequest Request;
	Request.set_field_bignore(bIgnore != 0);

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientignorelookinput>(Target, Request, 0);
}

void ClientGotoStateSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UNameProperty, NewState);

	improbable::unreal::UnrealClientGotoStateRequest Request;
	Request.set_field_newstate(TCHAR_TO_UTF8(*NewState.ToString()));

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientgotostate>(Target, Request, 0);
}

void ClientGameEndedSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(AActor, EndGameFocus);
	P_GET_UBOOL(bIsWinner);

	improbable::unreal::UnrealClientGameEndedRequest Request;
	{
		FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromObject(EndGameFocus);
		improbable::unreal::UnrealObjectRef UObjectRef = SpatialPMC->GetUnrealObjectRefFromNetGUID(NetGUID);
		Request.set_field_endgamefocus(UObjectRef);
	}
	Request.set_field_biswinner(bIsWinner != 0);

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientgameended>(Target, Request, 0);
}

void ClientForceGarbageCollectionSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	improbable::unreal::UnrealClientForceGarbageCollectionRequest Request;

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientforcegarbagecollection>(Target, Request, 0);
}

void ClientFlushLevelStreamingSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	improbable::unreal::UnrealClientFlushLevelStreamingRequest Request;

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientflushlevelstreaming>(Target, Request, 0);
}

void ClientEndOnlineSessionSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	improbable::unreal::UnrealClientEndOnlineSessionRequest Request;

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientendonlinesession>(Target, Request, 0);
}

void ClientEnableNetworkVoiceSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	FFrame& Stack = *RPCFrame;
	P_GET_UBOOL(bEnable);

	improbable::unreal::UnrealClientEnableNetworkVoiceRequest Request;
	Request.set_field_benable(bEnable != 0);

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientenablenetworkvoice>(Target, Request, 0);
}

void ClientCommitMapChangeSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	improbable::unreal::UnrealClientCommitMapChangeRequest Request;

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientcommitmapchange>(Target, Request, 0);
}

void ClientClearCameraLensEffectsSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	improbable::unreal::UnrealClientClearCameraLensEffectsRequest Request;

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientclearcameralenseffects>(Target, Request, 0);
}

void ClientCapBandwidthSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UIntProperty, Cap);

	improbable::unreal::UnrealClientCapBandwidthRequest Request;
	Request.set_field_cap(Cap);

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientcapbandwidth>(Target, Request, 0);
}

void ClientCancelPendingMapChangeSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	improbable::unreal::UnrealClientCancelPendingMapChangeRequest Request;

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientcancelpendingmapchange>(Target, Request, 0);
}

void ClientAddTextureStreamingLocSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
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

void ClientSetRotationSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	FFrame& Stack = *RPCFrame;
	P_GET_STRUCT(FRotator, NewRotation)
	P_GET_UBOOL(bResetCamera);

	improbable::unreal::UnrealClientSetRotationRequest Request;
	Request.set_field_newrotation(improbable::unreal::UnrealFRotator(NewRotation.Yaw, NewRotation.Pitch, NewRotation.Roll));
	Request.set_field_bresetcamera(bResetCamera != 0);

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetrotation>(Target, Request, 0);
}

void ClientSetLocationSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	FFrame& Stack = *RPCFrame;
	P_GET_STRUCT(FVector, NewLocation)
	P_GET_STRUCT(FRotator, NewRotation)

	improbable::unreal::UnrealClientSetLocationRequest Request;
	Request.set_field_newlocation(improbable::Vector3f(NewLocation.X, NewLocation.Y, NewLocation.Z));
	Request.set_field_newrotation(improbable::unreal::UnrealFRotator(NewRotation.Yaw, NewRotation.Pitch, NewRotation.Roll));

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetlocation>(Target, Request, 0);
}

void ServerViewSelfSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
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

void ServerViewPrevPlayerSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	improbable::unreal::UnrealServerViewPrevPlayerRequest Request;

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverviewprevplayer>(Target, Request, 0);
}

void ServerViewNextPlayerSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	improbable::unreal::UnrealServerViewNextPlayerRequest Request;

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverviewnextplayer>(Target, Request, 0);
}

void ServerVerifyViewTargetSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	improbable::unreal::UnrealServerVerifyViewTargetRequest Request;

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serververifyviewtarget>(Target, Request, 0);
}

void ServerUpdateLevelVisibilitySender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UNameProperty, PackageName);
	P_GET_UBOOL(bIsVisible);

	improbable::unreal::UnrealServerUpdateLevelVisibilityRequest Request;
	Request.set_field_packagename(TCHAR_TO_UTF8(*PackageName.ToString()));
	Request.set_field_bisvisible(bIsVisible != 0);

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverupdatelevelvisibility>(Target, Request, 0);
}

void ServerUpdateCameraSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	FFrame& Stack = *RPCFrame;
	P_GET_STRUCT(FVector_NetQuantize, CamLoc)
	P_GET_PROPERTY(UIntProperty, CamPitchAndYaw);

	improbable::unreal::UnrealServerUpdateCameraRequest Request;
	Request.set_field_camloc(improbable::Vector3f(CamLoc.X, CamLoc.Y, CamLoc.Z));
	Request.set_field_campitchandyaw(CamPitchAndYaw);

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverupdatecamera>(Target, Request, 0);
}

void ServerUnmutePlayerSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
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

void ServerToggleAILoggingSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	improbable::unreal::UnrealServerToggleAILoggingRequest Request;

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servertoggleailogging>(Target, Request, 0);
}

void ServerShortTimeoutSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	improbable::unreal::UnrealServerShortTimeoutRequest Request;

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servershorttimeout>(Target, Request, 0);
}

void ServerSetSpectatorWaitingSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	FFrame& Stack = *RPCFrame;
	P_GET_UBOOL(bWaiting);

	improbable::unreal::UnrealServerSetSpectatorWaitingRequest Request;
	Request.set_field_bwaiting(bWaiting != 0);

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serversetspectatorwaiting>(Target, Request, 0);
}

void ServerSetSpectatorLocationSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	FFrame& Stack = *RPCFrame;
	P_GET_STRUCT(FVector, NewLoc)
	P_GET_STRUCT(FRotator, NewRot)

	improbable::unreal::UnrealServerSetSpectatorLocationRequest Request;
	Request.set_field_newloc(improbable::Vector3f(NewLoc.X, NewLoc.Y, NewLoc.Z));
	Request.set_field_newrot(improbable::unreal::UnrealFRotator(NewRot.Yaw, NewRot.Pitch, NewRot.Roll));

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serversetspectatorlocation>(Target, Request, 0);
}

void ServerRestartPlayerSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	improbable::unreal::UnrealServerRestartPlayerRequest Request;

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverrestartplayer>(Target, Request, 0);
}

void ServerPauseSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	improbable::unreal::UnrealServerPauseRequest Request;

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverpause>(Target, Request, 0);
}

void ServerNotifyLoadedWorldSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UNameProperty, WorldPackageName);

	improbable::unreal::UnrealServerNotifyLoadedWorldRequest Request;
	Request.set_field_worldpackagename(TCHAR_TO_UTF8(*WorldPackageName.ToString()));

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servernotifyloadedworld>(Target, Request, 0);
}

void ServerMutePlayerSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
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

void ServerCheckClientPossessionReliableSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	improbable::unreal::UnrealServerCheckClientPossessionReliableRequest Request;

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servercheckclientpossessionreliable>(Target, Request, 0);
}

void ServerCheckClientPossessionSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	improbable::unreal::UnrealServerCheckClientPossessionRequest Request;

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servercheckclientpossession>(Target, Request, 0);
}

void ServerChangeNameSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UStrProperty, S);

	improbable::unreal::UnrealServerChangeNameRequest Request;
	Request.set_field_s(TCHAR_TO_UTF8(*S));

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverchangename>(Target, Request, 0);
}

void ServerCameraSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	FFrame& Stack = *RPCFrame;
	P_GET_PROPERTY(UNameProperty, NewMode);

	improbable::unreal::UnrealServerCameraRequest Request;
	Request.set_field_newmode(TCHAR_TO_UTF8(*NewMode.ToString()));

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servercamera>(Target, Request, 0);
}

void ServerAcknowledgePossessionSender(worker::Connection* const Connection, struct FFrame* const RPCFrame, const worker::EntityId& Target, USpatialPackageMapClient* SpatialPMC)
{
	FFrame& Stack = *RPCFrame;
	P_GET_OBJECT(APawn, P);

	improbable::unreal::UnrealServerAcknowledgePossessionRequest Request;
	{
		FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromObject(P);
		improbable::unreal::UnrealObjectRef UObjectRef = SpatialPMC->GetUnrealObjectRefFromNetGUID(NetGUID);
		Request.set_field_p(UObjectRef);
	}

	Connection->SendCommandRequest<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serveracknowledgepossession>(Target, Request, 0);
}

void BuildSpatialComponentUpdate(const FPropertyChangeState& Changes,
		improbable::unreal::UnrealPlayerControllerSingleClientReplicatedData::Update& SingleClientUpdate,
		bool& bSingleClientUpdateChanged,
		improbable::unreal::UnrealPlayerControllerMultiClientReplicatedData::Update& MultiClientUpdate,
		bool& bMultiClientUpdateChanged,
		UPackageMap* PackageMap)
{
	// Build up SpatialOS component updates.
	auto& PropertyMap = USpatialTypeBinding_PlayerController::GetHandlePropertyMap();
	FChangelistIterator ChangelistIterator(Changes.Changed, 0);
	FRepHandleIterator HandleIterator(ChangelistIterator, Changes.Cmds, Changes.BaseHandleToCmdIndex, 0, 1, 0, Changes.Cmds.Num() - 1);
	while (HandleIterator.NextHandle())
	{
		const FRepLayoutCmd& Cmd = Changes.Cmds[HandleIterator.CmdIndex];
		const uint8* Data = Changes.SourceData + HandleIterator.ArrayOffset + Cmd.Offset;
		auto& PropertyMapData = PropertyMap[HandleIterator.Handle];
		UE_LOG(LogTemp, Log, TEXT("-> Handle: %d Property %s"), HandleIterator.Handle, *Cmd.Property->GetName());
		switch (GetGroupFromCondition(PropertyMapData.Condition))
		{
		case GROUP_SingleClient:
			ApplyUpdateToSpatial_SingleClient_PlayerController(Data, HandleIterator.Handle, Cmd.Property, PackageMap, SingleClientUpdate);
			bSingleClientUpdateChanged = true;
			break;
		case GROUP_MultiClient:
			ApplyUpdateToSpatial_MultiClient_PlayerController(Data, HandleIterator.Handle, Cmd.Property, PackageMap, MultiClientUpdate);
			bMultiClientUpdateChanged = true;
			break;
		}
	}
}
} // ::

const FRepHandlePropertyMap& USpatialTypeBinding_PlayerController::GetHandlePropertyMap()
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

void USpatialTypeBinding_PlayerController::Init(USpatialUpdateInterop* InUpdateInterop, UPackageMap* InPackageMap)
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

void USpatialTypeBinding_PlayerController::BindToView()
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
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Onserverstartedvisuallogger>(std::bind(&USpatialTypeBinding_PlayerController::OnServerStartedVisualLoggerReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientwaskicked>(std::bind(&USpatialTypeBinding_PlayerController::ClientWasKickedReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientvoicehandshakecomplete>(std::bind(&USpatialTypeBinding_PlayerController::ClientVoiceHandshakeCompleteReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientupdatelevelstreamingstatus>(std::bind(&USpatialTypeBinding_PlayerController::ClientUpdateLevelStreamingStatusReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientunmuteplayer>(std::bind(&USpatialTypeBinding_PlayerController::ClientUnmutePlayerReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clienttravelinternal>(std::bind(&USpatialTypeBinding_PlayerController::ClientTravelInternalReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientteammessage>(std::bind(&USpatialTypeBinding_PlayerController::ClientTeamMessageReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientstopforcefeedback>(std::bind(&USpatialTypeBinding_PlayerController::ClientStopForceFeedbackReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientstopcamerashake>(std::bind(&USpatialTypeBinding_PlayerController::ClientStopCameraShakeReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientstopcameraanim>(std::bind(&USpatialTypeBinding_PlayerController::ClientStopCameraAnimReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientstartonlinesession>(std::bind(&USpatialTypeBinding_PlayerController::ClientStartOnlineSessionReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientspawncameralenseffect>(std::bind(&USpatialTypeBinding_PlayerController::ClientSpawnCameraLensEffectReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientsetviewtarget>(std::bind(&USpatialTypeBinding_PlayerController::ClientSetViewTargetReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientsetspectatorwaiting>(std::bind(&USpatialTypeBinding_PlayerController::ClientSetSpectatorWaitingReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientsethud>(std::bind(&USpatialTypeBinding_PlayerController::ClientSetHUDReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientsetforcemiplevelstoberesident>(std::bind(&USpatialTypeBinding_PlayerController::ClientSetForceMipLevelsToBeResidentReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientsetcinematicmode>(std::bind(&USpatialTypeBinding_PlayerController::ClientSetCinematicModeReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientsetcameramode>(std::bind(&USpatialTypeBinding_PlayerController::ClientSetCameraModeReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientsetcamerafade>(std::bind(&USpatialTypeBinding_PlayerController::ClientSetCameraFadeReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientsetblockonasyncloading>(std::bind(&USpatialTypeBinding_PlayerController::ClientSetBlockOnAsyncLoadingReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientreturntomainmenu>(std::bind(&USpatialTypeBinding_PlayerController::ClientReturnToMainMenuReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientretryclientrestart>(std::bind(&USpatialTypeBinding_PlayerController::ClientRetryClientRestartReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientrestart>(std::bind(&USpatialTypeBinding_PlayerController::ClientRestartReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientreset>(std::bind(&USpatialTypeBinding_PlayerController::ClientResetReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientrepobjref>(std::bind(&USpatialTypeBinding_PlayerController::ClientRepObjRefReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientreceivelocalizedmessage>(std::bind(&USpatialTypeBinding_PlayerController::ClientReceiveLocalizedMessageReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientprestreamtextures>(std::bind(&USpatialTypeBinding_PlayerController::ClientPrestreamTexturesReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientpreparemapchange>(std::bind(&USpatialTypeBinding_PlayerController::ClientPrepareMapChangeReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientplaysoundatlocation>(std::bind(&USpatialTypeBinding_PlayerController::ClientPlaySoundAtLocationReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientplaysound>(std::bind(&USpatialTypeBinding_PlayerController::ClientPlaySoundReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientplayforcefeedback>(std::bind(&USpatialTypeBinding_PlayerController::ClientPlayForceFeedbackReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientplaycamerashake>(std::bind(&USpatialTypeBinding_PlayerController::ClientPlayCameraShakeReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientplaycameraanim>(std::bind(&USpatialTypeBinding_PlayerController::ClientPlayCameraAnimReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientmuteplayer>(std::bind(&USpatialTypeBinding_PlayerController::ClientMutePlayerReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientmessage>(std::bind(&USpatialTypeBinding_PlayerController::ClientMessageReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientignoremoveinput>(std::bind(&USpatialTypeBinding_PlayerController::ClientIgnoreMoveInputReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientignorelookinput>(std::bind(&USpatialTypeBinding_PlayerController::ClientIgnoreLookInputReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientgotostate>(std::bind(&USpatialTypeBinding_PlayerController::ClientGotoStateReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientgameended>(std::bind(&USpatialTypeBinding_PlayerController::ClientGameEndedReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientforcegarbagecollection>(std::bind(&USpatialTypeBinding_PlayerController::ClientForceGarbageCollectionReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientflushlevelstreaming>(std::bind(&USpatialTypeBinding_PlayerController::ClientFlushLevelStreamingReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientendonlinesession>(std::bind(&USpatialTypeBinding_PlayerController::ClientEndOnlineSessionReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientenablenetworkvoice>(std::bind(&USpatialTypeBinding_PlayerController::ClientEnableNetworkVoiceReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientcommitmapchange>(std::bind(&USpatialTypeBinding_PlayerController::ClientCommitMapChangeReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientclearcameralenseffects>(std::bind(&USpatialTypeBinding_PlayerController::ClientClearCameraLensEffectsReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientcapbandwidth>(std::bind(&USpatialTypeBinding_PlayerController::ClientCapBandwidthReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientcancelpendingmapchange>(std::bind(&USpatialTypeBinding_PlayerController::ClientCancelPendingMapChangeReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientaddtexturestreamingloc>(std::bind(&USpatialTypeBinding_PlayerController::ClientAddTextureStreamingLocReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientsetrotation>(std::bind(&USpatialTypeBinding_PlayerController::ClientSetRotationReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ClientRPCCommandTypes::Clientsetlocation>(std::bind(&USpatialTypeBinding_PlayerController::ClientSetLocationReceiver, this, std::placeholders::_1)));
	using ServerRPCCommandTypes = improbable::unreal::UnrealPlayerControllerServerRPCs::Commands;
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Serverviewself>(std::bind(&USpatialTypeBinding_PlayerController::ServerViewSelfReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Serverviewprevplayer>(std::bind(&USpatialTypeBinding_PlayerController::ServerViewPrevPlayerReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Serverviewnextplayer>(std::bind(&USpatialTypeBinding_PlayerController::ServerViewNextPlayerReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Serververifyviewtarget>(std::bind(&USpatialTypeBinding_PlayerController::ServerVerifyViewTargetReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Serverupdatelevelvisibility>(std::bind(&USpatialTypeBinding_PlayerController::ServerUpdateLevelVisibilityReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Serverupdatecamera>(std::bind(&USpatialTypeBinding_PlayerController::ServerUpdateCameraReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Serverunmuteplayer>(std::bind(&USpatialTypeBinding_PlayerController::ServerUnmutePlayerReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Servertoggleailogging>(std::bind(&USpatialTypeBinding_PlayerController::ServerToggleAILoggingReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Servershorttimeout>(std::bind(&USpatialTypeBinding_PlayerController::ServerShortTimeoutReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Serversetspectatorwaiting>(std::bind(&USpatialTypeBinding_PlayerController::ServerSetSpectatorWaitingReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Serversetspectatorlocation>(std::bind(&USpatialTypeBinding_PlayerController::ServerSetSpectatorLocationReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Serverrestartplayer>(std::bind(&USpatialTypeBinding_PlayerController::ServerRestartPlayerReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Serverpause>(std::bind(&USpatialTypeBinding_PlayerController::ServerPauseReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Servernotifyloadedworld>(std::bind(&USpatialTypeBinding_PlayerController::ServerNotifyLoadedWorldReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Servermuteplayer>(std::bind(&USpatialTypeBinding_PlayerController::ServerMutePlayerReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Servercheckclientpossessionreliable>(std::bind(&USpatialTypeBinding_PlayerController::ServerCheckClientPossessionReliableReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Servercheckclientpossession>(std::bind(&USpatialTypeBinding_PlayerController::ServerCheckClientPossessionReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Serverchangename>(std::bind(&USpatialTypeBinding_PlayerController::ServerChangeNameReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Servercamera>(std::bind(&USpatialTypeBinding_PlayerController::ServerCameraReceiver, this, std::placeholders::_1)));
	RPCReceiverCallbacks.AddUnique(View->OnCommandRequest<ServerRPCCommandTypes::Serveracknowledgepossession>(std::bind(&USpatialTypeBinding_PlayerController::ServerAcknowledgePossessionReceiver, this, std::placeholders::_1)));
}

void USpatialTypeBinding_PlayerController::UnbindFromView()
{
	TSharedPtr<worker::View> View = UpdateInterop->GetSpatialOS()->GetView().Pin();
	View->Remove(SingleClientCallback);
	View->Remove(MultiClientCallback);
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

void USpatialTypeBinding_PlayerController::SendComponentUpdates(const FPropertyChangeState& Changes, const worker::EntityId& EntityId) const
{
	// Build SpatialOS updates.
	improbable::unreal::UnrealPlayerControllerSingleClientReplicatedData::Update SingleClientUpdate;
	bool SingleClientUpdateChanged = false;
	improbable::unreal::UnrealPlayerControllerMultiClientReplicatedData::Update MultiClientUpdate;
	bool MultiClientUpdateChanged = false;
	BuildSpatialComponentUpdate(Changes,
		SingleClientUpdate, SingleClientUpdateChanged,
		MultiClientUpdate, MultiClientUpdateChanged,
		PackageMap);

	// Send SpatialOS updates if anything changed.
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

worker::Entity USpatialTypeBinding_PlayerController::CreateActorEntity(const FVector& Position, const FString& Metadata, const FPropertyChangeState& InitialChanges) const
{
	// Setup initial data.
	improbable::unreal::UnrealPlayerControllerSingleClientReplicatedData::Data SingleClientData;
	improbable::unreal::UnrealPlayerControllerSingleClientReplicatedData::Update SingleClientUpdate;
	bool bSingleClientUpdateChanged = false;
	improbable::unreal::UnrealPlayerControllerMultiClientReplicatedData::Data MultiClientData;
	improbable::unreal::UnrealPlayerControllerMultiClientReplicatedData::Update MultiClientUpdate;
	bool bMultiClientUpdateChanged = false;
	BuildSpatialComponentUpdate(InitialChanges,
		SingleClientUpdate, bSingleClientUpdateChanged,
		MultiClientUpdate, bMultiClientUpdateChanged,
		PackageMap);
	SingleClientUpdate.ApplyTo(SingleClientData);
	MultiClientUpdate.ApplyTo(MultiClientData);
	
	// Create entity.
	const improbable::Coordinates SpatialPosition = USpatialOSConversionFunctionLibrary::UnrealCoordinatesToSpatialOsCoordinatesCast(Position);
	improbable::WorkerAttributeSet UnrealWorkerAttributeSet{worker::List<std::string>{"UnrealWorker"}};
	improbable::WorkerAttributeSet UnrealClientAttributeSet{worker::List<std::string>{"UnrealClient"}};
	improbable::WorkerRequirementSet UnrealWorkerWritePermission{{UnrealWorkerAttributeSet}};
	improbable::WorkerRequirementSet UnrealClientWritePermission{{UnrealClientAttributeSet}};
	improbable::WorkerRequirementSet AnyWorkerReadPermission{{UnrealClientAttributeSet, UnrealWorkerAttributeSet}};
	
	return improbable::unreal::FEntityBuilder::Begin()
		.AddPositionComponent(improbable::Position::Data{SpatialPosition}, UnrealWorkerWritePermission)
		.AddMetadataComponent(improbable::Metadata::Data{TCHAR_TO_UTF8(*Metadata)})
		.SetPersistence(true)
		.SetReadAcl(AnyWorkerReadPermission)
		.AddComponent<improbable::unreal::UnrealPlayerControllerSingleClientReplicatedData>(SingleClientData, UnrealWorkerWritePermission)
		.AddComponent<improbable::unreal::UnrealPlayerControllerMultiClientReplicatedData>(MultiClientData, UnrealWorkerWritePermission)
		.AddComponent<improbable::unreal::UnrealPlayerControllerCompleteData>(improbable::unreal::UnrealPlayerControllerCompleteData::Data{}, UnrealWorkerWritePermission)
		.AddComponent<improbable::unreal::UnrealPlayerControllerClientRPCs>(improbable::unreal::UnrealPlayerControllerClientRPCs::Data{}, UnrealWorkerWritePermission)
		.AddComponent<improbable::unreal::UnrealPlayerControllerServerRPCs>(improbable::unreal::UnrealPlayerControllerServerRPCs::Data{}, UnrealClientWritePermission)
		.Build();
}

void USpatialTypeBinding_PlayerController::SendRPCCommand(const UFunction* const Function, FFrame* const RPCFrame, const worker::EntityId& Target) const
{
	TSharedPtr<worker::Connection> Connection = UpdateInterop->GetSpatialOS()->GetConnection().Pin();
	auto Func = RPCToSenderMap.Find(Function->GetFName());
	checkf(*Func, TEXT("Sender for %s has not been registered with RPCToSenderMap."), *(Function->GetFName().ToString()));
	(*Func)(Connection.Get(), RPCFrame, Target, Cast<USpatialPackageMapClient>(PackageMap));
}

void USpatialTypeBinding_PlayerController::OnServerStartedVisualLoggerReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Onserverstartedvisuallogger>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("OnServerStartedVisualLoggerReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("OnServerStartedVisualLoggerReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Extract bIsLogging
	bool bIsLogging;
	bIsLogging = Op.Request.field_bislogging();

	// Call implementation and send command response.
	TargetObject->OnServerStartedVisualLogger_Implementation(bIsLogging);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Onserverstartedvisuallogger>(Op);
}

void USpatialTypeBinding_PlayerController::ClientWasKickedReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientwaskicked>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ClientWasKickedReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ClientWasKickedReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Extract KickReason
	FText KickReason;
	// UNSUPPORTED

	// Call implementation and send command response.
	TargetObject->ClientWasKicked_Implementation(KickReason);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientwaskicked>(Op);
}

void USpatialTypeBinding_PlayerController::ClientVoiceHandshakeCompleteReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientvoicehandshakecomplete>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ClientVoiceHandshakeCompleteReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ClientVoiceHandshakeCompleteReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Call implementation and send command response.
	TargetObject->ClientVoiceHandshakeComplete_Implementation();
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientvoicehandshakecomplete>(Op);
}

void USpatialTypeBinding_PlayerController::ClientUpdateLevelStreamingStatusReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientupdatelevelstreamingstatus>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ClientUpdateLevelStreamingStatusReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ClientUpdateLevelStreamingStatusReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

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

	// Call implementation and send command response.
	TargetObject->ClientUpdateLevelStreamingStatus_Implementation(PackageName, bNewShouldBeLoaded, bNewShouldBeVisible, bNewShouldBlockOnLoad, LODIndex);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientupdatelevelstreamingstatus>(Op);
}

void USpatialTypeBinding_PlayerController::ClientUnmutePlayerReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientunmuteplayer>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ClientUnmutePlayerReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ClientUnmutePlayerReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

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

	// Call implementation and send command response.
	TargetObject->ClientUnmutePlayer_Implementation(PlayerId);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientunmuteplayer>(Op);
}

void USpatialTypeBinding_PlayerController::ClientTravelInternalReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clienttravelinternal>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ClientTravelInternalReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ClientTravelInternalReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

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

	// Call implementation and send command response.
	TargetObject->ClientTravelInternal_Implementation(URL, TravelType, bSeamless, MapPackageGuid);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clienttravelinternal>(Op);
}

void USpatialTypeBinding_PlayerController::ClientTeamMessageReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientteammessage>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ClientTeamMessageReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ClientTeamMessageReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Extract SenderPlayerState
	APlayerState* SenderPlayerState;
	{
		improbable::unreal::UnrealObjectRef TargetObject = Op.Request.field_senderplayerstate();
		FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromUnrealObjectRef(TargetObject);
		SenderPlayerState = static_cast<APlayerState*>(SpatialPMC->GetObjectFromNetGUID(NetGUID, true));
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

	// Call implementation and send command response.
	TargetObject->ClientTeamMessage_Implementation(SenderPlayerState, S, Type, MsgLifeTime);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientteammessage>(Op);
}

void USpatialTypeBinding_PlayerController::ClientStopForceFeedbackReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstopforcefeedback>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ClientStopForceFeedbackReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ClientStopForceFeedbackReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Extract ForceFeedbackEffect
	UForceFeedbackEffect* ForceFeedbackEffect;
	{
		improbable::unreal::UnrealObjectRef TargetObject = Op.Request.field_forcefeedbackeffect();
		FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromUnrealObjectRef(TargetObject);
		ForceFeedbackEffect = static_cast<UForceFeedbackEffect*>(SpatialPMC->GetObjectFromNetGUID(NetGUID, true));
	}

	// Extract Tag
	FName Tag;
	Tag = FName((Op.Request.field_tag()).data());

	// Call implementation and send command response.
	TargetObject->ClientStopForceFeedback_Implementation(ForceFeedbackEffect, Tag);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstopforcefeedback>(Op);
}

void USpatialTypeBinding_PlayerController::ClientStopCameraShakeReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstopcamerashake>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ClientStopCameraShakeReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ClientStopCameraShakeReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Extract Shake
	TSubclassOf<UCameraShake>  Shake;
	//Not yet implemented UClass properties

	// Extract bImmediately
	bool bImmediately;
	bImmediately = Op.Request.field_bimmediately();

	// Call implementation and send command response.
	TargetObject->ClientStopCameraShake_Implementation(Shake, bImmediately);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstopcamerashake>(Op);
}

void USpatialTypeBinding_PlayerController::ClientStopCameraAnimReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstopcameraanim>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ClientStopCameraAnimReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ClientStopCameraAnimReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Extract AnimToStop
	UCameraAnim* AnimToStop;
	{
		improbable::unreal::UnrealObjectRef TargetObject = Op.Request.field_animtostop();
		FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromUnrealObjectRef(TargetObject);
		AnimToStop = static_cast<UCameraAnim*>(SpatialPMC->GetObjectFromNetGUID(NetGUID, true));
	}

	// Call implementation and send command response.
	TargetObject->ClientStopCameraAnim_Implementation(AnimToStop);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstopcameraanim>(Op);
}

void USpatialTypeBinding_PlayerController::ClientStartOnlineSessionReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstartonlinesession>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ClientStartOnlineSessionReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ClientStartOnlineSessionReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Call implementation and send command response.
	TargetObject->ClientStartOnlineSession_Implementation();
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientstartonlinesession>(Op);
}

void USpatialTypeBinding_PlayerController::ClientSpawnCameraLensEffectReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientspawncameralenseffect>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ClientSpawnCameraLensEffectReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ClientSpawnCameraLensEffectReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Extract LensEffectEmitterClass
	TSubclassOf<AEmitterCameraLensEffectBase>  LensEffectEmitterClass;
	//Not yet implemented UClass properties

	// Call implementation and send command response.
	TargetObject->ClientSpawnCameraLensEffect_Implementation(LensEffectEmitterClass);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientspawncameralenseffect>(Op);
}

void USpatialTypeBinding_PlayerController::ClientSetViewTargetReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetviewtarget>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ClientSetViewTargetReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ClientSetViewTargetReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Extract A
	AActor* A;
	{
		improbable::unreal::UnrealObjectRef TargetObject = Op.Request.field_a();
		FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromUnrealObjectRef(TargetObject);
		A = static_cast<AActor*>(SpatialPMC->GetObjectFromNetGUID(NetGUID, true));
	}

	// Extract TransitionParams
	FViewTargetTransitionParams TransitionParams;
	TransitionParams.BlendTime = Op.Request.field_transitionparams_blendtime();
	TransitionParams.BlendFunction = TEnumAsByte<EViewTargetBlendFunction>(uint8(Op.Request.field_transitionparams_blendfunction()));
	TransitionParams.BlendExp = Op.Request.field_transitionparams_blendexp();
	TransitionParams.bLockOutgoing = Op.Request.field_transitionparams_blockoutgoing();

	// Call implementation and send command response.
	TargetObject->ClientSetViewTarget_Implementation(A, TransitionParams);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetviewtarget>(Op);
}

void USpatialTypeBinding_PlayerController::ClientSetSpectatorWaitingReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetspectatorwaiting>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ClientSetSpectatorWaitingReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ClientSetSpectatorWaitingReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Extract bWaiting
	bool bWaiting;
	bWaiting = Op.Request.field_bwaiting();

	// Call implementation and send command response.
	TargetObject->ClientSetSpectatorWaiting_Implementation(bWaiting);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetspectatorwaiting>(Op);
}

void USpatialTypeBinding_PlayerController::ClientSetHUDReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsethud>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ClientSetHUDReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ClientSetHUDReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Extract NewHUDClass
	TSubclassOf<AHUD>  NewHUDClass;
	//Not yet implemented UClass properties

	// Call implementation and send command response.
	TargetObject->ClientSetHUD_Implementation(NewHUDClass);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsethud>(Op);
}

void USpatialTypeBinding_PlayerController::ClientSetForceMipLevelsToBeResidentReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetforcemiplevelstoberesident>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ClientSetForceMipLevelsToBeResidentReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ClientSetForceMipLevelsToBeResidentReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Extract Material
	UMaterialInterface* Material;
	{
		improbable::unreal::UnrealObjectRef TargetObject = Op.Request.field_material();
		FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromUnrealObjectRef(TargetObject);
		Material = static_cast<UMaterialInterface*>(SpatialPMC->GetObjectFromNetGUID(NetGUID, true));
	}

	// Extract ForceDuration
	float ForceDuration;
	ForceDuration = Op.Request.field_forceduration();

	// Extract CinematicTextureGroups
	int32 CinematicTextureGroups;
	CinematicTextureGroups = Op.Request.field_cinematictexturegroups();

	// Call implementation and send command response.
	TargetObject->ClientSetForceMipLevelsToBeResident_Implementation(Material, ForceDuration, CinematicTextureGroups);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetforcemiplevelstoberesident>(Op);
}

void USpatialTypeBinding_PlayerController::ClientSetCinematicModeReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetcinematicmode>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ClientSetCinematicModeReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ClientSetCinematicModeReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

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

	// Call implementation and send command response.
	TargetObject->ClientSetCinematicMode_Implementation(bInCinematicMode, bAffectsMovement, bAffectsTurning, bAffectsHUD);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetcinematicmode>(Op);
}

void USpatialTypeBinding_PlayerController::ClientSetCameraModeReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetcameramode>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ClientSetCameraModeReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ClientSetCameraModeReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Extract NewCamMode
	FName NewCamMode;
	NewCamMode = FName((Op.Request.field_newcammode()).data());

	// Call implementation and send command response.
	TargetObject->ClientSetCameraMode_Implementation(NewCamMode);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetcameramode>(Op);
}

void USpatialTypeBinding_PlayerController::ClientSetCameraFadeReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetcamerafade>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ClientSetCameraFadeReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ClientSetCameraFadeReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

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

	// Call implementation and send command response.
	TargetObject->ClientSetCameraFade_Implementation(bEnableFading, FadeColor, FadeAlpha, FadeTime, bFadeAudio);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetcamerafade>(Op);
}

void USpatialTypeBinding_PlayerController::ClientSetBlockOnAsyncLoadingReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetblockonasyncloading>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ClientSetBlockOnAsyncLoadingReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ClientSetBlockOnAsyncLoadingReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Call implementation and send command response.
	TargetObject->ClientSetBlockOnAsyncLoading_Implementation();
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetblockonasyncloading>(Op);
}

void USpatialTypeBinding_PlayerController::ClientReturnToMainMenuReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientreturntomainmenu>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ClientReturnToMainMenuReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ClientReturnToMainMenuReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Extract ReturnReason
	FString ReturnReason;
	ReturnReason = FString(UTF8_TO_TCHAR(Op.Request.field_returnreason().c_str()));

	// Call implementation and send command response.
	TargetObject->ClientReturnToMainMenu_Implementation(ReturnReason);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientreturntomainmenu>(Op);
}

void USpatialTypeBinding_PlayerController::ClientRetryClientRestartReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientretryclientrestart>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ClientRetryClientRestartReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ClientRetryClientRestartReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Extract NewPawn
	APawn* NewPawn;
	{
		improbable::unreal::UnrealObjectRef TargetObject = Op.Request.field_newpawn();
		FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromUnrealObjectRef(TargetObject);
		NewPawn = static_cast<APawn*>(SpatialPMC->GetObjectFromNetGUID(NetGUID, true));
	}

	// Call implementation and send command response.
	TargetObject->ClientRetryClientRestart_Implementation(NewPawn);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientretryclientrestart>(Op);
}

void USpatialTypeBinding_PlayerController::ClientRestartReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientrestart>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ClientRestartReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ClientRestartReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Extract NewPawn
	APawn* NewPawn;
	{
		improbable::unreal::UnrealObjectRef TargetObject = Op.Request.field_newpawn();
		FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromUnrealObjectRef(TargetObject);
		NewPawn = static_cast<APawn*>(SpatialPMC->GetObjectFromNetGUID(NetGUID, true));
	}

	// Call implementation and send command response.
	TargetObject->ClientRestart_Implementation(NewPawn);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientrestart>(Op);
}

void USpatialTypeBinding_PlayerController::ClientResetReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientreset>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ClientResetReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ClientResetReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Call implementation and send command response.
	TargetObject->ClientReset_Implementation();
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientreset>(Op);
}

void USpatialTypeBinding_PlayerController::ClientRepObjRefReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientrepobjref>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ClientRepObjRefReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ClientRepObjRefReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Extract Object
	UObject* Object;
	{
		improbable::unreal::UnrealObjectRef TargetObject = Op.Request.field_object();
		FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromUnrealObjectRef(TargetObject);
		Object = static_cast<UObject*>(SpatialPMC->GetObjectFromNetGUID(NetGUID, true));
	}

	// Call implementation and send command response.
	TargetObject->ClientRepObjRef_Implementation(Object);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientrepobjref>(Op);
}

void USpatialTypeBinding_PlayerController::ClientReceiveLocalizedMessageReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientreceivelocalizedmessage>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ClientReceiveLocalizedMessageReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ClientReceiveLocalizedMessageReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Extract Message
	TSubclassOf<ULocalMessage>  Message;
	//Not yet implemented UClass properties

	// Extract Switch
	int32 Switch;
	Switch = Op.Request.field_switch();

	// Extract RelatedPlayerState_1
	APlayerState* RelatedPlayerState_1;
	{
		improbable::unreal::UnrealObjectRef TargetObject = Op.Request.field_relatedplayerstate_1();
		FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromUnrealObjectRef(TargetObject);
		RelatedPlayerState_1 = static_cast<APlayerState*>(SpatialPMC->GetObjectFromNetGUID(NetGUID, true));
	}

	// Extract RelatedPlayerState_2
	APlayerState* RelatedPlayerState_2;
	{
		improbable::unreal::UnrealObjectRef TargetObject = Op.Request.field_relatedplayerstate_2();
		FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromUnrealObjectRef(TargetObject);
		RelatedPlayerState_2 = static_cast<APlayerState*>(SpatialPMC->GetObjectFromNetGUID(NetGUID, true));
	}

	// Extract OptionalObject
	UObject* OptionalObject;
	{
		improbable::unreal::UnrealObjectRef TargetObject = Op.Request.field_optionalobject();
		FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromUnrealObjectRef(TargetObject);
		OptionalObject = static_cast<UObject*>(SpatialPMC->GetObjectFromNetGUID(NetGUID, true));
	}

	// Call implementation and send command response.
	TargetObject->ClientReceiveLocalizedMessage_Implementation(Message, Switch, RelatedPlayerState_1, RelatedPlayerState_2, OptionalObject);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientreceivelocalizedmessage>(Op);
}

void USpatialTypeBinding_PlayerController::ClientPrestreamTexturesReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientprestreamtextures>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ClientPrestreamTexturesReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ClientPrestreamTexturesReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Extract ForcedActor
	AActor* ForcedActor;
	{
		improbable::unreal::UnrealObjectRef TargetObject = Op.Request.field_forcedactor();
		FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromUnrealObjectRef(TargetObject);
		ForcedActor = static_cast<AActor*>(SpatialPMC->GetObjectFromNetGUID(NetGUID, true));
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

	// Call implementation and send command response.
	TargetObject->ClientPrestreamTextures_Implementation(ForcedActor, ForceDuration, bEnableStreaming, CinematicTextureGroups);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientprestreamtextures>(Op);
}

void USpatialTypeBinding_PlayerController::ClientPrepareMapChangeReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientpreparemapchange>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ClientPrepareMapChangeReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ClientPrepareMapChangeReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Extract LevelName
	FName LevelName;
	LevelName = FName((Op.Request.field_levelname()).data());

	// Extract bFirst
	bool bFirst;
	bFirst = Op.Request.field_bfirst();

	// Extract bLast
	bool bLast;
	bLast = Op.Request.field_blast();

	// Call implementation and send command response.
	TargetObject->ClientPrepareMapChange_Implementation(LevelName, bFirst, bLast);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientpreparemapchange>(Op);
}

void USpatialTypeBinding_PlayerController::ClientPlaySoundAtLocationReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaysoundatlocation>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ClientPlaySoundAtLocationReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ClientPlaySoundAtLocationReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Extract Sound
	USoundBase* Sound;
	{
		improbable::unreal::UnrealObjectRef TargetObject = Op.Request.field_sound();
		FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromUnrealObjectRef(TargetObject);
		Sound = static_cast<USoundBase*>(SpatialPMC->GetObjectFromNetGUID(NetGUID, true));
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

	// Call implementation and send command response.
	TargetObject->ClientPlaySoundAtLocation_Implementation(Sound, Location, VolumeMultiplier, PitchMultiplier);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaysoundatlocation>(Op);
}

void USpatialTypeBinding_PlayerController::ClientPlaySoundReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaysound>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ClientPlaySoundReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ClientPlaySoundReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Extract Sound
	USoundBase* Sound;
	{
		improbable::unreal::UnrealObjectRef TargetObject = Op.Request.field_sound();
		FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromUnrealObjectRef(TargetObject);
		Sound = static_cast<USoundBase*>(SpatialPMC->GetObjectFromNetGUID(NetGUID, true));
	}

	// Extract VolumeMultiplier
	float VolumeMultiplier;
	VolumeMultiplier = Op.Request.field_volumemultiplier();

	// Extract PitchMultiplier
	float PitchMultiplier;
	PitchMultiplier = Op.Request.field_pitchmultiplier();

	// Call implementation and send command response.
	TargetObject->ClientPlaySound_Implementation(Sound, VolumeMultiplier, PitchMultiplier);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaysound>(Op);
}

void USpatialTypeBinding_PlayerController::ClientPlayForceFeedbackReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplayforcefeedback>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ClientPlayForceFeedbackReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ClientPlayForceFeedbackReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Extract ForceFeedbackEffect
	UForceFeedbackEffect* ForceFeedbackEffect;
	{
		improbable::unreal::UnrealObjectRef TargetObject = Op.Request.field_forcefeedbackeffect();
		FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromUnrealObjectRef(TargetObject);
		ForceFeedbackEffect = static_cast<UForceFeedbackEffect*>(SpatialPMC->GetObjectFromNetGUID(NetGUID, true));
	}

	// Extract bLooping
	bool bLooping;
	bLooping = Op.Request.field_blooping();

	// Extract Tag
	FName Tag;
	Tag = FName((Op.Request.field_tag()).data());

	// Call implementation and send command response.
	TargetObject->ClientPlayForceFeedback_Implementation(ForceFeedbackEffect, bLooping, Tag);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplayforcefeedback>(Op);
}

void USpatialTypeBinding_PlayerController::ClientPlayCameraShakeReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaycamerashake>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ClientPlayCameraShakeReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ClientPlayCameraShakeReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Extract Shake
	TSubclassOf<UCameraShake>  Shake;
	//Not yet implemented UClass properties

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

	// Call implementation and send command response.
	TargetObject->ClientPlayCameraShake_Implementation(Shake, Scale, PlaySpace, UserPlaySpaceRot);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaycamerashake>(Op);
}

void USpatialTypeBinding_PlayerController::ClientPlayCameraAnimReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaycameraanim>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ClientPlayCameraAnimReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ClientPlayCameraAnimReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Extract AnimToPlay
	UCameraAnim* AnimToPlay;
	{
		improbable::unreal::UnrealObjectRef TargetObject = Op.Request.field_animtoplay();
		FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromUnrealObjectRef(TargetObject);
		AnimToPlay = static_cast<UCameraAnim*>(SpatialPMC->GetObjectFromNetGUID(NetGUID, true));
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

	// Call implementation and send command response.
	TargetObject->ClientPlayCameraAnim_Implementation(AnimToPlay, Scale, Rate, BlendInTime, BlendOutTime, bLoop, bRandomStartTime, Space, CustomPlaySpace);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientplaycameraanim>(Op);
}

void USpatialTypeBinding_PlayerController::ClientMutePlayerReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientmuteplayer>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ClientMutePlayerReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ClientMutePlayerReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

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

	// Call implementation and send command response.
	TargetObject->ClientMutePlayer_Implementation(PlayerId);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientmuteplayer>(Op);
}

void USpatialTypeBinding_PlayerController::ClientMessageReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientmessage>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ClientMessageReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ClientMessageReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Extract S
	FString S;
	S = FString(UTF8_TO_TCHAR(Op.Request.field_s().c_str()));

	// Extract Type
	FName Type;
	Type = FName((Op.Request.field_type()).data());

	// Extract MsgLifeTime
	float MsgLifeTime;
	MsgLifeTime = Op.Request.field_msglifetime();

	// Call implementation and send command response.
	TargetObject->ClientMessage_Implementation(S, Type, MsgLifeTime);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientmessage>(Op);
}

void USpatialTypeBinding_PlayerController::ClientIgnoreMoveInputReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientignoremoveinput>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ClientIgnoreMoveInputReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ClientIgnoreMoveInputReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Extract bIgnore
	bool bIgnore;
	bIgnore = Op.Request.field_bignore();

	// Call implementation and send command response.
	TargetObject->ClientIgnoreMoveInput_Implementation(bIgnore);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientignoremoveinput>(Op);
}

void USpatialTypeBinding_PlayerController::ClientIgnoreLookInputReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientignorelookinput>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ClientIgnoreLookInputReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ClientIgnoreLookInputReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Extract bIgnore
	bool bIgnore;
	bIgnore = Op.Request.field_bignore();

	// Call implementation and send command response.
	TargetObject->ClientIgnoreLookInput_Implementation(bIgnore);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientignorelookinput>(Op);
}

void USpatialTypeBinding_PlayerController::ClientGotoStateReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientgotostate>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ClientGotoStateReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ClientGotoStateReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Extract NewState
	FName NewState;
	NewState = FName((Op.Request.field_newstate()).data());

	// Call implementation and send command response.
	TargetObject->ClientGotoState_Implementation(NewState);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientgotostate>(Op);
}

void USpatialTypeBinding_PlayerController::ClientGameEndedReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientgameended>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ClientGameEndedReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ClientGameEndedReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Extract EndGameFocus
	AActor* EndGameFocus;
	{
		improbable::unreal::UnrealObjectRef TargetObject = Op.Request.field_endgamefocus();
		FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromUnrealObjectRef(TargetObject);
		EndGameFocus = static_cast<AActor*>(SpatialPMC->GetObjectFromNetGUID(NetGUID, true));
	}

	// Extract bIsWinner
	bool bIsWinner;
	bIsWinner = Op.Request.field_biswinner();

	// Call implementation and send command response.
	TargetObject->ClientGameEnded_Implementation(EndGameFocus, bIsWinner);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientgameended>(Op);
}

void USpatialTypeBinding_PlayerController::ClientForceGarbageCollectionReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientforcegarbagecollection>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ClientForceGarbageCollectionReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ClientForceGarbageCollectionReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Call implementation and send command response.
	TargetObject->ClientForceGarbageCollection_Implementation();
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientforcegarbagecollection>(Op);
}

void USpatialTypeBinding_PlayerController::ClientFlushLevelStreamingReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientflushlevelstreaming>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ClientFlushLevelStreamingReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ClientFlushLevelStreamingReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Call implementation and send command response.
	TargetObject->ClientFlushLevelStreaming_Implementation();
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientflushlevelstreaming>(Op);
}

void USpatialTypeBinding_PlayerController::ClientEndOnlineSessionReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientendonlinesession>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ClientEndOnlineSessionReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ClientEndOnlineSessionReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Call implementation and send command response.
	TargetObject->ClientEndOnlineSession_Implementation();
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientendonlinesession>(Op);
}

void USpatialTypeBinding_PlayerController::ClientEnableNetworkVoiceReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientenablenetworkvoice>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ClientEnableNetworkVoiceReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ClientEnableNetworkVoiceReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Extract bEnable
	bool bEnable;
	bEnable = Op.Request.field_benable();

	// Call implementation and send command response.
	TargetObject->ClientEnableNetworkVoice_Implementation(bEnable);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientenablenetworkvoice>(Op);
}

void USpatialTypeBinding_PlayerController::ClientCommitMapChangeReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientcommitmapchange>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ClientCommitMapChangeReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ClientCommitMapChangeReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Call implementation and send command response.
	TargetObject->ClientCommitMapChange_Implementation();
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientcommitmapchange>(Op);
}

void USpatialTypeBinding_PlayerController::ClientClearCameraLensEffectsReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientclearcameralenseffects>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ClientClearCameraLensEffectsReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ClientClearCameraLensEffectsReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Call implementation and send command response.
	TargetObject->ClientClearCameraLensEffects_Implementation();
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientclearcameralenseffects>(Op);
}

void USpatialTypeBinding_PlayerController::ClientCapBandwidthReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientcapbandwidth>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ClientCapBandwidthReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ClientCapBandwidthReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Extract Cap
	int32 Cap;
	Cap = Op.Request.field_cap();

	// Call implementation and send command response.
	TargetObject->ClientCapBandwidth_Implementation(Cap);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientcapbandwidth>(Op);
}

void USpatialTypeBinding_PlayerController::ClientCancelPendingMapChangeReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientcancelpendingmapchange>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ClientCancelPendingMapChangeReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ClientCancelPendingMapChangeReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Call implementation and send command response.
	TargetObject->ClientCancelPendingMapChange_Implementation();
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientcancelpendingmapchange>(Op);
}

void USpatialTypeBinding_PlayerController::ClientAddTextureStreamingLocReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientaddtexturestreamingloc>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ClientAddTextureStreamingLocReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ClientAddTextureStreamingLocReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

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

	// Call implementation and send command response.
	TargetObject->ClientAddTextureStreamingLoc_Implementation(InLoc, Duration, bOverrideLocation);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientaddtexturestreamingloc>(Op);
}

void USpatialTypeBinding_PlayerController::ClientSetRotationReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetrotation>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ClientSetRotationReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ClientSetRotationReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

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

	// Call implementation and send command response.
	TargetObject->ClientSetRotation_Implementation(NewRotation, bResetCamera);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetrotation>(Op);
}

void USpatialTypeBinding_PlayerController::ClientSetLocationReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetlocation>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ClientSetLocationReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ClientSetLocationReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

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

	// Call implementation and send command response.
	TargetObject->ClientSetLocation_Implementation(NewLocation, NewRotation);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerClientRPCs::Commands::Clientsetlocation>(Op);
}

void USpatialTypeBinding_PlayerController::ServerViewSelfReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverviewself>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ServerViewSelfReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ServerViewSelfReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Extract TransitionParams
	FViewTargetTransitionParams TransitionParams;
	TransitionParams.BlendTime = Op.Request.field_transitionparams_blendtime();
	TransitionParams.BlendFunction = TEnumAsByte<EViewTargetBlendFunction>(uint8(Op.Request.field_transitionparams_blendfunction()));
	TransitionParams.BlendExp = Op.Request.field_transitionparams_blendexp();
	TransitionParams.bLockOutgoing = Op.Request.field_transitionparams_blockoutgoing();

	// Call implementation and send command response.
	TargetObject->ServerViewSelf_Implementation(TransitionParams);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverviewself>(Op);
}

void USpatialTypeBinding_PlayerController::ServerViewPrevPlayerReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverviewprevplayer>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ServerViewPrevPlayerReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ServerViewPrevPlayerReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Call implementation and send command response.
	TargetObject->ServerViewPrevPlayer_Implementation();
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverviewprevplayer>(Op);
}

void USpatialTypeBinding_PlayerController::ServerViewNextPlayerReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverviewnextplayer>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ServerViewNextPlayerReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ServerViewNextPlayerReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Call implementation and send command response.
	TargetObject->ServerViewNextPlayer_Implementation();
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverviewnextplayer>(Op);
}

void USpatialTypeBinding_PlayerController::ServerVerifyViewTargetReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serververifyviewtarget>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ServerVerifyViewTargetReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ServerVerifyViewTargetReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Call implementation and send command response.
	TargetObject->ServerVerifyViewTarget_Implementation();
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serververifyviewtarget>(Op);
}

void USpatialTypeBinding_PlayerController::ServerUpdateLevelVisibilityReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverupdatelevelvisibility>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ServerUpdateLevelVisibilityReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ServerUpdateLevelVisibilityReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Extract PackageName
	FName PackageName;
	PackageName = FName((Op.Request.field_packagename()).data());

	// Extract bIsVisible
	bool bIsVisible;
	bIsVisible = Op.Request.field_bisvisible();

	// Call implementation and send command response.
	TargetObject->ServerUpdateLevelVisibility_Implementation(PackageName, bIsVisible);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverupdatelevelvisibility>(Op);
}

void USpatialTypeBinding_PlayerController::ServerUpdateCameraReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverupdatecamera>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ServerUpdateCameraReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ServerUpdateCameraReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

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

	// Call implementation and send command response.
	TargetObject->ServerUpdateCamera_Implementation(CamLoc, CamPitchAndYaw);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverupdatecamera>(Op);
}

void USpatialTypeBinding_PlayerController::ServerUnmutePlayerReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverunmuteplayer>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ServerUnmutePlayerReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ServerUnmutePlayerReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

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

	// Call implementation and send command response.
	TargetObject->ServerUnmutePlayer_Implementation(PlayerId);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverunmuteplayer>(Op);
}

void USpatialTypeBinding_PlayerController::ServerToggleAILoggingReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servertoggleailogging>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ServerToggleAILoggingReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ServerToggleAILoggingReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Call implementation and send command response.
	TargetObject->ServerToggleAILogging_Implementation();
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servertoggleailogging>(Op);
}

void USpatialTypeBinding_PlayerController::ServerShortTimeoutReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servershorttimeout>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ServerShortTimeoutReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ServerShortTimeoutReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Call implementation and send command response.
	TargetObject->ServerShortTimeout_Implementation();
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servershorttimeout>(Op);
}

void USpatialTypeBinding_PlayerController::ServerSetSpectatorWaitingReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serversetspectatorwaiting>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ServerSetSpectatorWaitingReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ServerSetSpectatorWaitingReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Extract bWaiting
	bool bWaiting;
	bWaiting = Op.Request.field_bwaiting();

	// Call implementation and send command response.
	TargetObject->ServerSetSpectatorWaiting_Implementation(bWaiting);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serversetspectatorwaiting>(Op);
}

void USpatialTypeBinding_PlayerController::ServerSetSpectatorLocationReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serversetspectatorlocation>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ServerSetSpectatorLocationReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ServerSetSpectatorLocationReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

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

	// Call implementation and send command response.
	TargetObject->ServerSetSpectatorLocation_Implementation(NewLoc, NewRot);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serversetspectatorlocation>(Op);
}

void USpatialTypeBinding_PlayerController::ServerRestartPlayerReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverrestartplayer>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ServerRestartPlayerReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ServerRestartPlayerReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Call implementation and send command response.
	TargetObject->ServerRestartPlayer_Implementation();
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverrestartplayer>(Op);
}

void USpatialTypeBinding_PlayerController::ServerPauseReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverpause>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ServerPauseReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ServerPauseReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Call implementation and send command response.
	TargetObject->ServerPause_Implementation();
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverpause>(Op);
}

void USpatialTypeBinding_PlayerController::ServerNotifyLoadedWorldReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servernotifyloadedworld>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ServerNotifyLoadedWorldReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ServerNotifyLoadedWorldReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Extract WorldPackageName
	FName WorldPackageName;
	WorldPackageName = FName((Op.Request.field_worldpackagename()).data());

	// Call implementation and send command response.
	TargetObject->ServerNotifyLoadedWorld_Implementation(WorldPackageName);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servernotifyloadedworld>(Op);
}

void USpatialTypeBinding_PlayerController::ServerMutePlayerReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servermuteplayer>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ServerMutePlayerReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ServerMutePlayerReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

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

	// Call implementation and send command response.
	TargetObject->ServerMutePlayer_Implementation(PlayerId);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servermuteplayer>(Op);
}

void USpatialTypeBinding_PlayerController::ServerCheckClientPossessionReliableReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servercheckclientpossessionreliable>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ServerCheckClientPossessionReliableReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ServerCheckClientPossessionReliableReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Call implementation and send command response.
	TargetObject->ServerCheckClientPossessionReliable_Implementation();
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servercheckclientpossessionreliable>(Op);
}

void USpatialTypeBinding_PlayerController::ServerCheckClientPossessionReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servercheckclientpossession>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ServerCheckClientPossessionReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ServerCheckClientPossessionReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Call implementation and send command response.
	TargetObject->ServerCheckClientPossession_Implementation();
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servercheckclientpossession>(Op);
}

void USpatialTypeBinding_PlayerController::ServerChangeNameReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverchangename>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ServerChangeNameReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ServerChangeNameReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Extract S
	FString S;
	S = FString(UTF8_TO_TCHAR(Op.Request.field_s().c_str()));

	// Call implementation and send command response.
	TargetObject->ServerChangeName_Implementation(S);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serverchangename>(Op);
}

void USpatialTypeBinding_PlayerController::ServerCameraReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servercamera>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ServerCameraReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ServerCameraReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Extract NewMode
	FName NewMode;
	NewMode = FName((Op.Request.field_newmode()).data());

	// Call implementation and send command response.
	TargetObject->ServerCamera_Implementation(NewMode);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Servercamera>(Op);
}

void USpatialTypeBinding_PlayerController::ServerAcknowledgePossessionReceiver(const worker::CommandRequestOp<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serveracknowledgepossession>& Op)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	FNetworkGUID TargetNetGUID = SpatialPMC->GetNetGUIDFromEntityId(Op.EntityId);
	if (!TargetNetGUID.IsValid())
	{
		UE_LOG(LogSpatialUpdateInterop, Warning, TEXT("ServerAcknowledgePossessionReceiver: Entity ID %lld does not have a valid NetGUID."), Op.EntityId);
		return;
	}
	APlayerController* TargetObject = Cast<APlayerController>(SpatialPMC->GetObjectFromNetGUID(TargetNetGUID, false));
	checkf(TargetObject, TEXT("ServerAcknowledgePossessionReceiver: Entity ID %lld (NetGUID %s) does not correspond to a UObject."), Op.EntityId, *TargetNetGUID.ToString());

	// Extract P
	APawn* P;
	{
		improbable::unreal::UnrealObjectRef TargetObject = Op.Request.field_p();
		FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromUnrealObjectRef(TargetObject);
		P = static_cast<APawn*>(SpatialPMC->GetObjectFromNetGUID(NetGUID, true));
	}

	// Call implementation and send command response.
	TargetObject->ServerAcknowledgePossession_Implementation(P);
	SendRPCResponse<improbable::unreal::UnrealPlayerControllerServerRPCs::Commands::Serveracknowledgepossession>(Op);
}
