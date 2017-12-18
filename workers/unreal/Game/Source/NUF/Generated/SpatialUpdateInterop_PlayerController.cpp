// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// Note that this file has been generated automatically

#include "SpatialUpdateInterop_PlayerController.h"
#include "Engine.h"
#include "SpatialActorChannel.h"

const RepHandlePropertyMap& GetHandlePropertyMap_PlayerController()
{
	UClass* Class = FindObject<UClass>(ANY_PACKAGE, TEXT("PlayerController"));
	static RepHandlePropertyMap* HandleToPropertyMapData = nullptr;
	if (HandleToPropertyMapData == nullptr)
	{
		HandleToPropertyMapData = new RepHandlePropertyMap();
		auto& HandleToPropertyMap = *HandleToPropertyMapData;
		HandleToPropertyMap.Add(18, RepHandleData{nullptr, Class->FindPropertyByName("TargetViewRotation"), 1244});
		HandleToPropertyMap.Add(19, RepHandleData{nullptr, Class->FindPropertyByName("SpawnLocation"), 1896});
		HandleToPropertyMap.Add(1, RepHandleData{nullptr, Class->FindPropertyByName("bHidden"), 148});
		HandleToPropertyMap.Add(2, RepHandleData{nullptr, Class->FindPropertyByName("bReplicateMovement"), 148});
		HandleToPropertyMap.Add(3, RepHandleData{nullptr, Class->FindPropertyByName("bTearOff"), 148});
		HandleToPropertyMap.Add(4, RepHandleData{nullptr, Class->FindPropertyByName("RemoteRole"), 164});
		HandleToPropertyMap.Add(5, RepHandleData{nullptr, Class->FindPropertyByName("Owner"), 168});
		HandleToPropertyMap.Add(6, RepHandleData{nullptr, Class->FindPropertyByName("ReplicatedMovement"), 176});
		HandleToPropertyMap.Add(7, RepHandleData{Class->FindPropertyByName("AttachmentReplication"), nullptr, 232});
		HandleToPropertyMap[7].Property = Cast<UStructProperty>(HandleToPropertyMap[7].Parent)->Struct->FindPropertyByName("AttachParent");
		HandleToPropertyMap.Add(8, RepHandleData{Class->FindPropertyByName("AttachmentReplication"), nullptr, 240});
		HandleToPropertyMap[8].Property = Cast<UStructProperty>(HandleToPropertyMap[8].Parent)->Struct->FindPropertyByName("LocationOffset");
		HandleToPropertyMap.Add(9, RepHandleData{Class->FindPropertyByName("AttachmentReplication"), nullptr, 252});
		HandleToPropertyMap[9].Property = Cast<UStructProperty>(HandleToPropertyMap[9].Parent)->Struct->FindPropertyByName("RelativeScale3D");
		HandleToPropertyMap.Add(10, RepHandleData{Class->FindPropertyByName("AttachmentReplication"), nullptr, 264});
		HandleToPropertyMap[10].Property = Cast<UStructProperty>(HandleToPropertyMap[10].Parent)->Struct->FindPropertyByName("RotationOffset");
		HandleToPropertyMap.Add(11, RepHandleData{Class->FindPropertyByName("AttachmentReplication"), nullptr, 276});
		HandleToPropertyMap[11].Property = Cast<UStructProperty>(HandleToPropertyMap[11].Parent)->Struct->FindPropertyByName("AttachSocket");
		HandleToPropertyMap.Add(12, RepHandleData{Class->FindPropertyByName("AttachmentReplication"), nullptr, 288});
		HandleToPropertyMap[12].Property = Cast<UStructProperty>(HandleToPropertyMap[12].Parent)->Struct->FindPropertyByName("AttachComponent");
		HandleToPropertyMap.Add(13, RepHandleData{nullptr, Class->FindPropertyByName("Role"), 296});
		HandleToPropertyMap.Add(14, RepHandleData{nullptr, Class->FindPropertyByName("bCanBeDamaged"), 344});
		HandleToPropertyMap.Add(15, RepHandleData{nullptr, Class->FindPropertyByName("Instigator"), 352});
		HandleToPropertyMap.Add(16, RepHandleData{nullptr, Class->FindPropertyByName("Pawn"), 1080});
		HandleToPropertyMap.Add(17, RepHandleData{nullptr, Class->FindPropertyByName("PlayerState"), 1104});
	}
	return *HandleToPropertyMapData;
}

void ApplyUpdateToSpatial_SingleClient_PlayerController(FArchive& Reader, int32 Handle, UProperty* Property, improbable::unreal::UnrealPlayerControllerSingleClientReplicatedData::Update& Update)
{
	switch (Handle)
	{
		case 18: // field_targetviewrotation
		{
			FRotator Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);

			Update.set_field_targetviewrotation(improbable::unreal::UnrealFRotator(Value.Yaw, Value.Pitch, Value.Roll));
			break;
		}
		case 19: // field_spawnlocation
		{
			FVector Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);

			Update.set_field_spawnlocation(improbable::Vector3f(Value.X, Value.Y, Value.Z));
			break;
		}
	default:
		break;
	}
}

void ReceiveUpdateFromSpatial_SingleClient_PlayerController(USpatialActorChannel* ActorChannel, const improbable::unreal::UnrealPlayerControllerSingleClientReplicatedData::Update& Update)
{
	FNetBitWriter OutputWriter(nullptr, 0); 
	auto& HandleToPropertyMap = GetHandlePropertyMap_PlayerController();
	ConditionMapFilter ConditionMap(ActorChannel);
	if (!Update.field_targetviewrotation().empty())
	{
		// field_targetviewrotation
		uint32 Handle = 18;
		if (ConditionMap.IsRelevant(COND_OwnerOnly))
		{
			OutputWriter.SerializeIntPacked(Handle);
			const RepHandleData& Data = HandleToPropertyMap[Handle];

			FRotator Value;
			check(Data.Property->ElementSize == sizeof(Value));

			auto& Rotator = *(Update.field_targetviewrotation().data());
			Value.Yaw = Rotator.yaw();
			Value.Pitch = Rotator.pitch();
			Value.Roll = Rotator.roll();

			Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
			UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
		}
	}
	if (!Update.field_spawnlocation().empty())
	{
		// field_spawnlocation
		uint32 Handle = 19;
		if (ConditionMap.IsRelevant(COND_OwnerOnly))
		{
			OutputWriter.SerializeIntPacked(Handle);
			const RepHandleData& Data = HandleToPropertyMap[Handle];

			FVector Value;
			check(Data.Property->ElementSize == sizeof(Value));

			auto& Vector = *(Update.field_spawnlocation().data());
			Value.X = Vector.x();
			Value.Y = Vector.y();
			Value.Z = Vector.z();

			Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
			UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
		}
	}
	ActorChannel->SpatialReceivePropertyUpdate(OutputWriter);
}

void ApplyUpdateToSpatial_MultiClient_PlayerController(FArchive& Reader, int32 Handle, UProperty* Property, improbable::unreal::UnrealPlayerControllerMultiClientReplicatedData::Update& Update)
{
	switch (Handle)
	{
		case 1: // field_bhidden
		{
			uint8 Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);

			Update.set_field_bhidden(Value != 0);
			break;
		}
		case 2: // field_breplicatemovement
		{
			uint8 Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);

			Update.set_field_breplicatemovement(Value != 0);
			break;
		}
		case 3: // field_btearoff
		{
			uint8 Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);

			Update.set_field_btearoff(Value != 0);
			break;
		}
		case 4: // field_remoterole
		{
			TEnumAsByte<ENetRole> Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);

			Update.set_field_remoterole(uint32_t(Value));
			break;
		}
		// case 5: - Owner is an object reference, skipping.
		case 6: // field_replicatedmovement
		{
			FRepMovement Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);

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
			Property->NetSerializeItem(Reader, nullptr, &Value);

			Update.set_field_attachmentreplication_locationoffset(improbable::Vector3f(Value.X, Value.Y, Value.Z));
			break;
		}
		case 9: // field_attachmentreplication_relativescale3d
		{
			FVector_NetQuantize100 Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);

			Update.set_field_attachmentreplication_relativescale3d(improbable::Vector3f(Value.X, Value.Y, Value.Z));
			break;
		}
		case 10: // field_attachmentreplication_rotationoffset
		{
			FRotator Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);

			Update.set_field_attachmentreplication_rotationoffset(improbable::unreal::UnrealFRotator(Value.Yaw, Value.Pitch, Value.Roll));
			break;
		}
		case 11: // field_attachmentreplication_attachsocket
		{
			FName Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);

			Update.set_field_attachmentreplication_attachsocket(TCHAR_TO_UTF8(*Value.ToString()));
			break;
		}
		// case 12: - AttachComponent is an object reference, skipping.
		case 13: // field_role
		{
			TEnumAsByte<ENetRole> Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);

			Update.set_field_role(uint32_t(Value));
			break;
		}
		case 14: // field_bcanbedamaged
		{
			uint8 Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);

			Update.set_field_bcanbedamaged(Value != 0);
			break;
		}
		// case 15: - Instigator is an object reference, skipping.
		// case 16: - Pawn is an object reference, skipping.
		// case 17: - PlayerState is an object reference, skipping.
	default:
		break;
	}
}

void ReceiveUpdateFromSpatial_MultiClient_PlayerController(USpatialActorChannel* ActorChannel, const improbable::unreal::UnrealPlayerControllerMultiClientReplicatedData::Update& Update)
{
	FNetBitWriter OutputWriter(nullptr, 0); 
	auto& HandleToPropertyMap = GetHandlePropertyMap_PlayerController();
	ConditionMapFilter ConditionMap(ActorChannel);
	if (!Update.field_bhidden().empty())
	{
		// field_bhidden
		uint32 Handle = 1;
		if (ConditionMap.IsRelevant(COND_None))
		{
			OutputWriter.SerializeIntPacked(Handle);
			const RepHandleData& Data = HandleToPropertyMap[Handle];

			uint8 Value;
			check(Data.Property->ElementSize == sizeof(Value));

			Value = *(Update.field_bhidden().data());

			Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
			UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
		}
	}
	if (!Update.field_breplicatemovement().empty())
	{
		// field_breplicatemovement
		uint32 Handle = 2;
		if (ConditionMap.IsRelevant(COND_None))
		{
			OutputWriter.SerializeIntPacked(Handle);
			const RepHandleData& Data = HandleToPropertyMap[Handle];

			uint8 Value;
			check(Data.Property->ElementSize == sizeof(Value));

			Value = *(Update.field_breplicatemovement().data());

			Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
			UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
		}
	}
	if (!Update.field_btearoff().empty())
	{
		// field_btearoff
		uint32 Handle = 3;
		if (ConditionMap.IsRelevant(COND_None))
		{
			OutputWriter.SerializeIntPacked(Handle);
			const RepHandleData& Data = HandleToPropertyMap[Handle];

			uint8 Value;
			check(Data.Property->ElementSize == sizeof(Value));

			Value = *(Update.field_btearoff().data());

			Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
			UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
		}
	}
	if (!Update.field_remoterole().empty())
	{
		// field_remoterole
		uint32 Handle = 4;
		if (ConditionMap.IsRelevant(COND_None))
		{
			OutputWriter.SerializeIntPacked(Handle);
			const RepHandleData& Data = HandleToPropertyMap[Handle];

			TEnumAsByte<ENetRole> Value;
			check(Data.Property->ElementSize == sizeof(Value));

			// Byte properties are weird, because they can also be an enum in the form TEnumAsByte<...>.
			// Therefore, the code generator needs to cast to either TEnumAsByte<...> or uint8. However,
			// as TEnumAsByte<...> only has a uint8 constructor, we need to cast the SpatialOS value into
			// uint8 first, which causes "uint8(uint8(...))" to be generated for non enum bytes.
			Value = TEnumAsByte<ENetRole>(uint8(*(Update.field_remoterole().data())));

			Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
			UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
		}
	}
	if (!Update.field_owner().empty())
	{
		// field_owner
		uint32 Handle = 5;
		if (ConditionMap.IsRelevant(COND_None))
		{
			OutputWriter.SerializeIntPacked(Handle);
			const RepHandleData& Data = HandleToPropertyMap[Handle];

			AActor* Value;
			check(Data.Property->ElementSize == sizeof(Value));

			// UNSUPPORTED ObjectProperty - Value *(Update.field_owner().data());

			Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
			UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
		}
	}
	if (!Update.field_replicatedmovement().empty())
	{
		// field_replicatedmovement
		uint32 Handle = 6;
		if (ConditionMap.IsRelevant(COND_SimulatedOrPhysics))
		{
			OutputWriter.SerializeIntPacked(Handle);
			const RepHandleData& Data = HandleToPropertyMap[Handle];

			FRepMovement Value;
			check(Data.Property->ElementSize == sizeof(Value));

			auto& ValueDataStr = *(Update.field_replicatedmovement().data());
			TArray<uint8> ValueData;
			ValueData.Append((uint8*)ValueDataStr.data(), ValueDataStr.size());
			FMemoryReader ValueDataReader(ValueData);
			bool bSuccess;
			Value.NetSerialize(ValueDataReader, nullptr, bSuccess);

			Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
			UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
		}
	}
	if (!Update.field_attachmentreplication_attachparent().empty())
	{
		// field_attachmentreplication_attachparent
		uint32 Handle = 7;
		if (ConditionMap.IsRelevant(COND_Custom))
		{
			OutputWriter.SerializeIntPacked(Handle);
			const RepHandleData& Data = HandleToPropertyMap[Handle];

			AActor* Value;
			check(Data.Property->ElementSize == sizeof(Value));

			// UNSUPPORTED ObjectProperty - Value *(Update.field_attachmentreplication_attachparent().data());

			Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
			UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
		}
	}
	if (!Update.field_attachmentreplication_locationoffset().empty())
	{
		// field_attachmentreplication_locationoffset
		uint32 Handle = 8;
		if (ConditionMap.IsRelevant(COND_Custom))
		{
			OutputWriter.SerializeIntPacked(Handle);
			const RepHandleData& Data = HandleToPropertyMap[Handle];

			FVector_NetQuantize100 Value;
			check(Data.Property->ElementSize == sizeof(Value));

			auto& Vector = *(Update.field_attachmentreplication_locationoffset().data());
			Value.X = Vector.x();
			Value.Y = Vector.y();
			Value.Z = Vector.z();

			Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
			UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
		}
	}
	if (!Update.field_attachmentreplication_relativescale3d().empty())
	{
		// field_attachmentreplication_relativescale3d
		uint32 Handle = 9;
		if (ConditionMap.IsRelevant(COND_Custom))
		{
			OutputWriter.SerializeIntPacked(Handle);
			const RepHandleData& Data = HandleToPropertyMap[Handle];

			FVector_NetQuantize100 Value;
			check(Data.Property->ElementSize == sizeof(Value));

			auto& Vector = *(Update.field_attachmentreplication_relativescale3d().data());
			Value.X = Vector.x();
			Value.Y = Vector.y();
			Value.Z = Vector.z();

			Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
			UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
		}
	}
	if (!Update.field_attachmentreplication_rotationoffset().empty())
	{
		// field_attachmentreplication_rotationoffset
		uint32 Handle = 10;
		if (ConditionMap.IsRelevant(COND_Custom))
		{
			OutputWriter.SerializeIntPacked(Handle);
			const RepHandleData& Data = HandleToPropertyMap[Handle];

			FRotator Value;
			check(Data.Property->ElementSize == sizeof(Value));

			auto& Rotator = *(Update.field_attachmentreplication_rotationoffset().data());
			Value.Yaw = Rotator.yaw();
			Value.Pitch = Rotator.pitch();
			Value.Roll = Rotator.roll();

			Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
			UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
		}
	}
	if (!Update.field_attachmentreplication_attachsocket().empty())
	{
		// field_attachmentreplication_attachsocket
		uint32 Handle = 11;
		if (ConditionMap.IsRelevant(COND_Custom))
		{
			OutputWriter.SerializeIntPacked(Handle);
			const RepHandleData& Data = HandleToPropertyMap[Handle];

			FName Value;
			check(Data.Property->ElementSize == sizeof(Value));

			Value = FName((*(Update.field_attachmentreplication_attachsocket().data())).data());

			Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
			UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
		}
	}
	if (!Update.field_attachmentreplication_attachcomponent().empty())
	{
		// field_attachmentreplication_attachcomponent
		uint32 Handle = 12;
		if (ConditionMap.IsRelevant(COND_Custom))
		{
			OutputWriter.SerializeIntPacked(Handle);
			const RepHandleData& Data = HandleToPropertyMap[Handle];

			USceneComponent* Value;
			check(Data.Property->ElementSize == sizeof(Value));

			// UNSUPPORTED ObjectProperty - Value *(Update.field_attachmentreplication_attachcomponent().data());

			Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
			UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
		}
	}
	if (!Update.field_role().empty())
	{
		// field_role
		uint32 Handle = 13;
		if (ConditionMap.IsRelevant(COND_None))
		{
			OutputWriter.SerializeIntPacked(Handle);
			const RepHandleData& Data = HandleToPropertyMap[Handle];

			TEnumAsByte<ENetRole> Value;
			check(Data.Property->ElementSize == sizeof(Value));

			// Byte properties are weird, because they can also be an enum in the form TEnumAsByte<...>.
			// Therefore, the code generator needs to cast to either TEnumAsByte<...> or uint8. However,
			// as TEnumAsByte<...> only has a uint8 constructor, we need to cast the SpatialOS value into
			// uint8 first, which causes "uint8(uint8(...))" to be generated for non enum bytes.
			Value = TEnumAsByte<ENetRole>(uint8(*(Update.field_role().data())));

			Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
			UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
		}
	}
	if (!Update.field_bcanbedamaged().empty())
	{
		// field_bcanbedamaged
		uint32 Handle = 14;
		if (ConditionMap.IsRelevant(COND_None))
		{
			OutputWriter.SerializeIntPacked(Handle);
			const RepHandleData& Data = HandleToPropertyMap[Handle];

			uint8 Value;
			check(Data.Property->ElementSize == sizeof(Value));

			Value = *(Update.field_bcanbedamaged().data());

			Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
			UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
		}
	}
	if (!Update.field_instigator().empty())
	{
		// field_instigator
		uint32 Handle = 15;
		if (ConditionMap.IsRelevant(COND_None))
		{
			OutputWriter.SerializeIntPacked(Handle);
			const RepHandleData& Data = HandleToPropertyMap[Handle];

			APawn* Value;
			check(Data.Property->ElementSize == sizeof(Value));

			// UNSUPPORTED ObjectProperty - Value *(Update.field_instigator().data());

			Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
			UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
		}
	}
	if (!Update.field_pawn().empty())
	{
		// field_pawn
		uint32 Handle = 16;
		if (ConditionMap.IsRelevant(COND_None))
		{
			OutputWriter.SerializeIntPacked(Handle);
			const RepHandleData& Data = HandleToPropertyMap[Handle];

			APawn* Value;
			check(Data.Property->ElementSize == sizeof(Value));

			// UNSUPPORTED ObjectProperty - Value *(Update.field_pawn().data());

			Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
			UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
		}
	}
	if (!Update.field_playerstate().empty())
	{
		// field_playerstate
		uint32 Handle = 17;
		if (ConditionMap.IsRelevant(COND_None))
		{
			OutputWriter.SerializeIntPacked(Handle);
			const RepHandleData& Data = HandleToPropertyMap[Handle];

			APlayerState* Value;
			check(Data.Property->ElementSize == sizeof(Value));

			// UNSUPPORTED ObjectProperty - Value *(Update.field_playerstate().data());

			Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
			UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
		}
	}
	ActorChannel->SpatialReceivePropertyUpdate(OutputWriter);
}
