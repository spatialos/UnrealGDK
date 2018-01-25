// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// Note that this file has been generated automatically

#include "SpatialTypeBinding_PlayerController.h"
#include "SpatialOS.h"
#include "Engine.h"
#include "SpatialActorChannel.h"
#include "EntityBuilder.h"
// TODO(David): Remove this once RPCs are merged, as we will no longer need a placeholder component.
#include "improbable/player/player.h"
#include "SpatialPackageMapClient.h"
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
			FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromObject(Value);

			improbable::unreal::UnrealObjectRef UObjectRef = SpatialPMC->GetUnrealObjectRefFromNetGUID(NetGUID);
			Update.set_field_owner(UObjectRef);
			break;
		}
		case 6: // field_replicatedmovement
		{
			FRepMovement Value;
			Value = *(reinterpret_cast<const FRepMovement*>(Data));

			TArray<uint8> ValueData;
			FMemoryWriter ValueDataWriter(ValueData);
			bool Success;
			Value.NetSerialize(ValueDataWriter, nullptr, Success);
			Update.set_field_replicatedmovement(std::string((char*)ValueData.GetData(), ValueData.Num()));
			break;
		}
		case 7: // field_attachmentreplication_attachparent
		{
			AActor* Value;
			Value = *(reinterpret_cast<AActor* const*>(Data));
			FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromObject(Value);

			improbable::unreal::UnrealObjectRef UObjectRef = SpatialPMC->GetUnrealObjectRefFromNetGUID(NetGUID);
			Update.set_field_attachmentreplication_attachparent(UObjectRef);
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
			FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromObject(Value);

			improbable::unreal::UnrealObjectRef UObjectRef = SpatialPMC->GetUnrealObjectRefFromNetGUID(NetGUID);
			Update.set_field_attachmentreplication_attachcomponent(UObjectRef);
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
			FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromObject(Value);

			improbable::unreal::UnrealObjectRef UObjectRef = SpatialPMC->GetUnrealObjectRefFromNetGUID(NetGUID);
			Update.set_field_instigator(UObjectRef);
			break;
		}
		case 16: // field_pawn
		{
			APawn* Value;
			Value = *(reinterpret_cast<APawn* const*>(Data));
			FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromObject(Value);

			improbable::unreal::UnrealObjectRef UObjectRef = SpatialPMC->GetUnrealObjectRefFromNetGUID(NetGUID);
			Update.set_field_pawn(UObjectRef);
			break;
		}
		case 17: // field_playerstate
		{
			APlayerState* Value;
			Value = *(reinterpret_cast<APlayerState* const*>(Data));
			FNetworkGUID NetGUID = SpatialPMC->GetNetGUIDFromObject(Value);

			improbable::unreal::UnrealObjectRef UObjectRef = SpatialPMC->GetUnrealObjectRefFromNetGUID(NetGUID);
			Update.set_field_playerstate(UObjectRef);
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
}

void USpatialTypeBinding_PlayerController::UnbindFromView()
{
	TSharedPtr<worker::View> View = UpdateInterop->GetSpatialOS()->GetView().Pin();
	View->Remove(SingleClientCallback);
	View->Remove(MultiClientCallback);
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
		.AddComponent<improbable::player::PlayerControlClient>(improbable::player::PlayerControlClient::Data{}, UnrealClientWritePermission)
		.AddComponent<improbable::unreal::UnrealPlayerControllerSingleClientReplicatedData>(SingleClientData, UnrealWorkerWritePermission)
		.AddComponent<improbable::unreal::UnrealPlayerControllerMultiClientReplicatedData>(MultiClientData, UnrealWorkerWritePermission)
		.AddComponent<improbable::unreal::UnrealPlayerControllerCompleteData>(improbable::unreal::UnrealPlayerControllerCompleteData::Data{}, UnrealWorkerWritePermission)
		.Build();
}
