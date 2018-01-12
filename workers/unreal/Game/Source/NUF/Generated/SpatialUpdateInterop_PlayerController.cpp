// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// Note that this file has been generated automatically

#include "SpatialUpdateInterop_PlayerController.h"
#include "SpatialOS.h"
#include "Engine.h"
#include "SpatialActorChannel.h"
#include "SpatialPackageMapClient.h"
#include "Utils/BunchReader.h"

namespace {

void ApplyUpdateToSpatial_SingleClient_PlayerController(FArchive& Reader, int32 Handle, UProperty* Property, UPackageMap* PackageMap, improbable::unreal::UnrealPlayerControllerSingleClientReplicatedData::Update& Update)
{
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	
	check(SpatialPMC);
	
	switch (Handle)
	{
		case 18: // field_targetviewrotation
		{
			FRotator Value;
			check(Property->ElementSize == sizeof(Value));
			//HACK:
			// Doing this temporarily just to get to properties after RemoteRole without corrupting the archive.
			// This needs to be solved at a more fundamental level.
			uint32 NumBits = 0;
			Reader.SerializeIntPacked(NumBits);
			//END-HACK
			Property->NetSerializeItem(Reader, PackageMap, &Value);

			Update.set_field_targetviewrotation(improbable::unreal::UnrealFRotator(Value.Yaw, Value.Pitch, Value.Roll));
			break;
		}
		case 19: // field_spawnlocation
		{
			FVector Value;
			check(Property->ElementSize == sizeof(Value));
			//HACK:
			// Doing this temporarily just to get to properties after RemoteRole without corrupting the archive.
			// This needs to be solved at a more fundamental level.
			uint32 NumBits = 0;
			Reader.SerializeIntPacked(NumBits);
			//END-HACK
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
	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(PackageMap);
	
	check(SpatialPMC);
	
	switch (Handle)
	{
		case 1: // field_bhidden
		{
			uint8 Value;
			check(Property->ElementSize == sizeof(Value));
			//HACK:
			// Doing this temporarily just to get to properties after RemoteRole without corrupting the archive.
			// This needs to be solved at a more fundamental level.
			uint32 NumBits = 0;
			Reader.SerializeIntPacked(NumBits);
			//END-HACK
			Property->NetSerializeItem(Reader, PackageMap, &Value);

			Update.set_field_bhidden(Value != 0);
			break;
		}
		case 2: // field_breplicatemovement
		{
			uint8 Value;
			check(Property->ElementSize == sizeof(Value));
			//HACK:
			// Doing this temporarily just to get to properties after RemoteRole without corrupting the archive.
			// This needs to be solved at a more fundamental level.
			uint32 NumBits = 0;
			Reader.SerializeIntPacked(NumBits);
			//END-HACK
			Property->NetSerializeItem(Reader, PackageMap, &Value);

			Update.set_field_breplicatemovement(Value != 0);
			break;
		}
		case 3: // field_btearoff
		{
			uint8 Value;
			check(Property->ElementSize == sizeof(Value));
			//HACK:
			// Doing this temporarily just to get to properties after RemoteRole without corrupting the archive.
			// This needs to be solved at a more fundamental level.
			uint32 NumBits = 0;
			Reader.SerializeIntPacked(NumBits);
			//END-HACK
			Property->NetSerializeItem(Reader, PackageMap, &Value);

			Update.set_field_btearoff(Value != 0);
			break;
		}
		case 4: // field_remoterole
		{
			TEnumAsByte<ENetRole> Value;
			check(Property->ElementSize == sizeof(Value));
			//HACK:
			// Doing this temporarily just to get to properties after RemoteRole without corrupting the archive.
			// This needs to be solved at a more fundamental level.
			uint32 NumBits = 0;
			Reader.SerializeIntPacked(NumBits);
			//END-HACK
			Property->NetSerializeItem(Reader, PackageMap, &Value);

			Update.set_field_remoterole(uint32_t(Value));
			break;
		}
		case 5: // field_owner
		{
			AActor* Value = nullptr;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, PackageMap, &Value);

			FNetworkGUID NetGUID;
			// Note that NetGUID is not connected to anything right now, so the serialization won't work. We'll connect in the non-bunch branch.
			improbable::unreal::UnrealObjectRef UObjectRef = SpatialPMC->GetUnrealObjectRefFromNetGUID(NetGUID);
			Value(UObjectRef);
			break;
		}
		case 6: // field_replicatedmovement
		{
			FRepMovement Value;
			check(Property->ElementSize == sizeof(Value));
			//HACK:
			// Doing this temporarily just to get to properties after RemoteRole without corrupting the archive.
			// This needs to be solved at a more fundamental level.
			uint32 NumBits = 0;
			Reader.SerializeIntPacked(NumBits);
			//END-HACK
			Property->NetSerializeItem(Reader, PackageMap, &Value);

			TArray<uint8> ValueData;
			FMemoryWriter ValueDataWriter(ValueData);
			bool Success;
			Value.NetSerialize(ValueDataWriter, nullptr, Success);
			Update.set_field_replicatedmovement(std::string((char*)ValueData.GetData(), ValueData.Num()));
			break;
		}
		case 7: // field_attachmentreplication_attachparent
		{
			AActor* Value = nullptr;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, PackageMap, &Value);

			FNetworkGUID NetGUID;
			// Note that NetGUID is not connected to anything right now, so the serialization won't work. We'll connect in the non-bunch branch.
			improbable::unreal::UnrealObjectRef UObjectRef = SpatialPMC->GetUnrealObjectRefFromNetGUID(NetGUID);
			Value(UObjectRef);
			break;
		}
		case 8: // field_attachmentreplication_locationoffset
		{
			FVector_NetQuantize100 Value;
			check(Property->ElementSize == sizeof(Value));
			//HACK:
			// Doing this temporarily just to get to properties after RemoteRole without corrupting the archive.
			// This needs to be solved at a more fundamental level.
			uint32 NumBits = 0;
			Reader.SerializeIntPacked(NumBits);
			//END-HACK
			Property->NetSerializeItem(Reader, PackageMap, &Value);

			Update.set_field_attachmentreplication_locationoffset(improbable::Vector3f(Value.X, Value.Y, Value.Z));
			break;
		}
		case 9: // field_attachmentreplication_relativescale3d
		{
			FVector_NetQuantize100 Value;
			check(Property->ElementSize == sizeof(Value));
			//HACK:
			// Doing this temporarily just to get to properties after RemoteRole without corrupting the archive.
			// This needs to be solved at a more fundamental level.
			uint32 NumBits = 0;
			Reader.SerializeIntPacked(NumBits);
			//END-HACK
			Property->NetSerializeItem(Reader, PackageMap, &Value);

			Update.set_field_attachmentreplication_relativescale3d(improbable::Vector3f(Value.X, Value.Y, Value.Z));
			break;
		}
		case 10: // field_attachmentreplication_rotationoffset
		{
			FRotator Value;
			check(Property->ElementSize == sizeof(Value));
			//HACK:
			// Doing this temporarily just to get to properties after RemoteRole without corrupting the archive.
			// This needs to be solved at a more fundamental level.
			uint32 NumBits = 0;
			Reader.SerializeIntPacked(NumBits);
			//END-HACK
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
		case 12: // field_attachmentreplication_attachcomponent
		{
			USceneComponent* Value = nullptr;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, PackageMap, &Value);

			FNetworkGUID NetGUID;
			// Note that NetGUID is not connected to anything right now, so the serialization won't work. We'll connect in the non-bunch branch.
			improbable::unreal::UnrealObjectRef UObjectRef = SpatialPMC->GetUnrealObjectRefFromNetGUID(NetGUID);
			Value(UObjectRef);
			break;
		}
		case 13: // field_role
		{
			TEnumAsByte<ENetRole> Value;
			check(Property->ElementSize == sizeof(Value));
			//HACK:
			// Doing this temporarily just to get to properties after RemoteRole without corrupting the archive.
			// This needs to be solved at a more fundamental level.
			uint32 NumBits = 0;
			Reader.SerializeIntPacked(NumBits);
			//END-HACK
			Property->NetSerializeItem(Reader, PackageMap, &Value);

			Update.set_field_role(uint32_t(Value));
			break;
		}
		case 14: // field_bcanbedamaged
		{
			uint8 Value;
			check(Property->ElementSize == sizeof(Value));
			//HACK:
			// Doing this temporarily just to get to properties after RemoteRole without corrupting the archive.
			// This needs to be solved at a more fundamental level.
			uint32 NumBits = 0;
			Reader.SerializeIntPacked(NumBits);
			//END-HACK
			Property->NetSerializeItem(Reader, PackageMap, &Value);

			Update.set_field_bcanbedamaged(Value != 0);
			break;
		}
		case 15: // field_instigator
		{
			APawn* Value = nullptr;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, PackageMap, &Value);

			FNetworkGUID NetGUID;
			// Note that NetGUID is not connected to anything right now, so the serialization won't work. We'll connect in the non-bunch branch.
			improbable::unreal::UnrealObjectRef UObjectRef = SpatialPMC->GetUnrealObjectRefFromNetGUID(NetGUID);
			Value(UObjectRef);
			break;
		}
		case 16: // field_pawn
		{
			APawn* Value = nullptr;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, PackageMap, &Value);

			FNetworkGUID NetGUID;
			// Note that NetGUID is not connected to anything right now, so the serialization won't work. We'll connect in the non-bunch branch.
			improbable::unreal::UnrealObjectRef UObjectRef = SpatialPMC->GetUnrealObjectRefFromNetGUID(NetGUID);
			Value(UObjectRef);
			break;
		}
		case 17: // field_playerstate
		{
			APlayerState* Value = nullptr;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, PackageMap, &Value);

			FNetworkGUID NetGUID;
			// Note that NetGUID is not connected to anything right now, so the serialization won't work. We'll connect in the non-bunch branch.
			improbable::unreal::UnrealObjectRef UObjectRef = SpatialPMC->GetUnrealObjectRefFromNetGUID(NetGUID);
			Value(UObjectRef);
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
} // ::

const RepHandlePropertyMap& GetHandlePropertyMap_PlayerController()
{
	static RepHandlePropertyMap* HandleToPropertyMapData = nullptr;
	if (HandleToPropertyMapData == nullptr)
	{
		UClass* Class = FindObject<UClass>(ANY_PACKAGE, TEXT("PlayerController"));
		HandleToPropertyMapData = new RepHandlePropertyMap();
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

void FSpatialTypeBinding_PlayerController::SendComponentUpdates(FInBunch* BunchPtr, const worker::EntityId& EntityId) const
{
	// Build SpatialOS updates.
	improbable::unreal::UnrealPlayerControllerSingleClientReplicatedData::Update SingleClientUpdate;
	bool SingleClientUpdateChanged = false;
	improbable::unreal::UnrealPlayerControllerMultiClientReplicatedData::Update MultiClientUpdate;
	bool MultiClientUpdateChanged = false;

	// Read bunch and build up SpatialOS component updates.
	auto& PropertyMap = GetHandlePropertyMap_PlayerController();
	FBunchReader BunchReader(BunchPtr);
	FBunchReader::RepDataHandler RepDataHandler = [&](FNetBitReader& Reader, UPackageMap* PackageMap, int32 Handle, UProperty* Property) -> bool
	{
		// TODO: We can't parse UObjects or FNames here as we have no package map.
		
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
	BunchReader.Parse(true, PropertyMap, RepDataHandler);

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
