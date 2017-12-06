// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// Note that this file has been generated automatically

#include "SpatialUpdateInterop_Character.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#include "SpatialActorChannel.h"

const RepHandlePropertyMap& GetHandlePropertyMap_Character()
{
	UClass* Class = ACharacter::StaticClass();
	static RepHandlePropertyMap* HandleToPropertyMapData = nullptr;
	if (HandleToPropertyMapData == nullptr)
	{
		HandleToPropertyMapData = new RepHandlePropertyMap();
		auto& HandleToPropertyMap = *HandleToPropertyMapData;
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
		HandleToPropertyMap.Add(16, RepHandleData{nullptr, Class->FindPropertyByName("PlayerState"), 1104});
		HandleToPropertyMap.Add(17, RepHandleData{nullptr, Class->FindPropertyByName("RemoteViewPitch"), 1112});
		HandleToPropertyMap.Add(18, RepHandleData{nullptr, Class->FindPropertyByName("Controller"), 1128});
		HandleToPropertyMap.Add(19, RepHandleData{Class->FindPropertyByName("ReplicatedBasedMovement"), nullptr, 1248});
		HandleToPropertyMap[19].Property = Cast<UStructProperty>(HandleToPropertyMap[19].Parent)->Struct->FindPropertyByName("MovementBase");
		HandleToPropertyMap.Add(20, RepHandleData{Class->FindPropertyByName("ReplicatedBasedMovement"), nullptr, 1256});
		HandleToPropertyMap[20].Property = Cast<UStructProperty>(HandleToPropertyMap[20].Parent)->Struct->FindPropertyByName("BoneName");
		HandleToPropertyMap.Add(21, RepHandleData{Class->FindPropertyByName("ReplicatedBasedMovement"), nullptr, 1268});
		HandleToPropertyMap[21].Property = Cast<UStructProperty>(HandleToPropertyMap[21].Parent)->Struct->FindPropertyByName("Location");
		HandleToPropertyMap.Add(22, RepHandleData{Class->FindPropertyByName("ReplicatedBasedMovement"), nullptr, 1280});
		HandleToPropertyMap[22].Property = Cast<UStructProperty>(HandleToPropertyMap[22].Parent)->Struct->FindPropertyByName("Rotation");
		HandleToPropertyMap.Add(23, RepHandleData{Class->FindPropertyByName("ReplicatedBasedMovement"), nullptr, 1292});
		HandleToPropertyMap[23].Property = Cast<UStructProperty>(HandleToPropertyMap[23].Parent)->Struct->FindPropertyByName("bServerHasBaseComponent");
		HandleToPropertyMap.Add(24, RepHandleData{Class->FindPropertyByName("ReplicatedBasedMovement"), nullptr, 1293});
		HandleToPropertyMap[24].Property = Cast<UStructProperty>(HandleToPropertyMap[24].Parent)->Struct->FindPropertyByName("bRelativeRotation");
		HandleToPropertyMap.Add(25, RepHandleData{Class->FindPropertyByName("ReplicatedBasedMovement"), nullptr, 1294});
		HandleToPropertyMap[25].Property = Cast<UStructProperty>(HandleToPropertyMap[25].Parent)->Struct->FindPropertyByName("bServerHasVelocity");
		HandleToPropertyMap.Add(26, RepHandleData{nullptr, Class->FindPropertyByName("AnimRootMotionTranslationScale"), 1296});
		HandleToPropertyMap.Add(27, RepHandleData{nullptr, Class->FindPropertyByName("ReplicatedServerLastTransformUpdateTimeStamp"), 1328});
		HandleToPropertyMap.Add(28, RepHandleData{nullptr, Class->FindPropertyByName("ReplicatedMovementMode"), 1332});
		HandleToPropertyMap.Add(29, RepHandleData{nullptr, Class->FindPropertyByName("bIsCrouched"), 1340});
		HandleToPropertyMap.Add(30, RepHandleData{nullptr, Class->FindPropertyByName("JumpMaxHoldTime"), 1348});
		HandleToPropertyMap.Add(31, RepHandleData{nullptr, Class->FindPropertyByName("JumpMaxCount"), 1352});
		HandleToPropertyMap.Add(32, RepHandleData{Class->FindPropertyByName("RepRootMotion"), nullptr, 1776});
		HandleToPropertyMap[32].Property = Cast<UStructProperty>(HandleToPropertyMap[32].Parent)->Struct->FindPropertyByName("bIsActive");
		HandleToPropertyMap.Add(33, RepHandleData{Class->FindPropertyByName("RepRootMotion"), nullptr, 1784});
		HandleToPropertyMap[33].Property = Cast<UStructProperty>(HandleToPropertyMap[33].Parent)->Struct->FindPropertyByName("AnimMontage");
		HandleToPropertyMap.Add(34, RepHandleData{Class->FindPropertyByName("RepRootMotion"), nullptr, 1792});
		HandleToPropertyMap[34].Property = Cast<UStructProperty>(HandleToPropertyMap[34].Parent)->Struct->FindPropertyByName("Position");
		HandleToPropertyMap.Add(35, RepHandleData{Class->FindPropertyByName("RepRootMotion"), nullptr, 1796});
		HandleToPropertyMap[35].Property = Cast<UStructProperty>(HandleToPropertyMap[35].Parent)->Struct->FindPropertyByName("Location");
		HandleToPropertyMap.Add(36, RepHandleData{Class->FindPropertyByName("RepRootMotion"), nullptr, 1808});
		HandleToPropertyMap[36].Property = Cast<UStructProperty>(HandleToPropertyMap[36].Parent)->Struct->FindPropertyByName("Rotation");
		HandleToPropertyMap.Add(37, RepHandleData{Class->FindPropertyByName("RepRootMotion"), nullptr, 1824});
		HandleToPropertyMap[37].Property = Cast<UStructProperty>(HandleToPropertyMap[37].Parent)->Struct->FindPropertyByName("MovementBase");
		HandleToPropertyMap.Add(38, RepHandleData{Class->FindPropertyByName("RepRootMotion"), nullptr, 1832});
		HandleToPropertyMap[38].Property = Cast<UStructProperty>(HandleToPropertyMap[38].Parent)->Struct->FindPropertyByName("MovementBaseBoneName");
		HandleToPropertyMap.Add(39, RepHandleData{Class->FindPropertyByName("RepRootMotion"), nullptr, 1844});
		HandleToPropertyMap[39].Property = Cast<UStructProperty>(HandleToPropertyMap[39].Parent)->Struct->FindPropertyByName("bRelativePosition");
		HandleToPropertyMap.Add(40, RepHandleData{Class->FindPropertyByName("RepRootMotion"), nullptr, 1845});
		HandleToPropertyMap[40].Property = Cast<UStructProperty>(HandleToPropertyMap[40].Parent)->Struct->FindPropertyByName("bRelativeRotation");
		HandleToPropertyMap.Add(41, RepHandleData{Class->FindPropertyByName("RepRootMotion"), nullptr, 1848});
		HandleToPropertyMap[41].Property = Cast<UStructProperty>(HandleToPropertyMap[41].Parent)->Struct->FindPropertyByName("AuthoritativeRootMotion");
		HandleToPropertyMap.Add(42, RepHandleData{Class->FindPropertyByName("RepRootMotion"), nullptr, 2104});
		HandleToPropertyMap[42].Property = Cast<UStructProperty>(HandleToPropertyMap[42].Parent)->Struct->FindPropertyByName("Acceleration");
		HandleToPropertyMap.Add(43, RepHandleData{Class->FindPropertyByName("RepRootMotion"), nullptr, 2116});
		HandleToPropertyMap[43].Property = Cast<UStructProperty>(HandleToPropertyMap[43].Parent)->Struct->FindPropertyByName("LinearVelocity");
	}
	return *HandleToPropertyMapData;
}

void ApplyUpdateToSpatial_Character(FArchive& Reader, int32 Handle, UProperty* Property, improbable::unreal::UnrealCharacterReplicatedData::Update& Update)
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
		// case 16: - PlayerState is an object reference, skipping.
		case 17: // field_remoteviewpitch
		{
			uint8 Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);

			Update.set_field_remoteviewpitch(uint32_t(Value));
			break;
		}
		// case 18: - Controller is an object reference, skipping.
		// case 19: - MovementBase is an object reference, skipping.
		case 20: // field_replicatedbasedmovement_bonename
		{
			FName Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);

			Update.set_field_replicatedbasedmovement_bonename(TCHAR_TO_UTF8(*Value.ToString()));
			break;
		}
		case 21: // field_replicatedbasedmovement_location
		{
			FVector_NetQuantize100 Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);

			Update.set_field_replicatedbasedmovement_location(improbable::Vector3f(Value.X, Value.Y, Value.Z));
			break;
		}
		case 22: // field_replicatedbasedmovement_rotation
		{
			FRotator Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);

			Update.set_field_replicatedbasedmovement_rotation(improbable::unreal::UnrealFRotator(Value.Yaw, Value.Pitch, Value.Roll));
			break;
		}
		case 23: // field_replicatedbasedmovement_bserverhasbasecomponent
		{
			bool Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);

			Update.set_field_replicatedbasedmovement_bserverhasbasecomponent(Value != 0);
			break;
		}
		case 24: // field_replicatedbasedmovement_brelativerotation
		{
			bool Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);

			Update.set_field_replicatedbasedmovement_brelativerotation(Value != 0);
			break;
		}
		case 25: // field_replicatedbasedmovement_bserverhasvelocity
		{
			bool Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);

			Update.set_field_replicatedbasedmovement_bserverhasvelocity(Value != 0);
			break;
		}
		case 26: // field_animrootmotiontranslationscale
		{
			float Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);

			Update.set_field_animrootmotiontranslationscale(Value);
			break;
		}
		case 27: // field_replicatedserverlasttransformupdatetimestamp
		{
			float Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);

			Update.set_field_replicatedserverlasttransformupdatetimestamp(Value);
			break;
		}
		case 28: // field_replicatedmovementmode
		{
			uint8 Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);

			Update.set_field_replicatedmovementmode(uint32_t(Value));
			break;
		}
		case 29: // field_biscrouched
		{
			uint8 Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);

			Update.set_field_biscrouched(Value != 0);
			break;
		}
		case 30: // field_jumpmaxholdtime
		{
			float Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);

			Update.set_field_jumpmaxholdtime(Value);
			break;
		}
		case 31: // field_jumpmaxcount
		{
			int32 Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);

			Update.set_field_jumpmaxcount(Value);
			break;
		}
		case 32: // field_reprootmotion_bisactive
		{
			bool Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);

			Update.set_field_reprootmotion_bisactive(Value != 0);
			break;
		}
		// case 33: - AnimMontage is an object reference, skipping.
		case 34: // field_reprootmotion_position
		{
			float Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);

			Update.set_field_reprootmotion_position(Value);
			break;
		}
		case 35: // field_reprootmotion_location
		{
			FVector_NetQuantize100 Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);

			Update.set_field_reprootmotion_location(improbable::Vector3f(Value.X, Value.Y, Value.Z));
			break;
		}
		case 36: // field_reprootmotion_rotation
		{
			FRotator Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);

			Update.set_field_reprootmotion_rotation(improbable::unreal::UnrealFRotator(Value.Yaw, Value.Pitch, Value.Roll));
			break;
		}
		// case 37: - MovementBase is an object reference, skipping.
		case 38: // field_reprootmotion_movementbasebonename
		{
			FName Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);

			Update.set_field_reprootmotion_movementbasebonename(TCHAR_TO_UTF8(*Value.ToString()));
			break;
		}
		case 39: // field_reprootmotion_brelativeposition
		{
			bool Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);

			Update.set_field_reprootmotion_brelativeposition(Value != 0);
			break;
		}
		case 40: // field_reprootmotion_brelativerotation
		{
			bool Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);

			Update.set_field_reprootmotion_brelativerotation(Value != 0);
			break;
		}
		case 41: // field_reprootmotion_authoritativerootmotion
		{
			FRootMotionSourceGroup Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);

			{
				Update.set_field_reprootmotion_authoritativerootmotion_bhasadditivesources(Value.bHasAdditiveSources != 0);
			}
			{
				Update.set_field_reprootmotion_authoritativerootmotion_bhasoverridesources(Value.bHasOverrideSources != 0);
			}
			{
				Update.set_field_reprootmotion_authoritativerootmotion_lastpreadditivevelocity(improbable::Vector3f(Value.LastPreAdditiveVelocity.X, Value.LastPreAdditiveVelocity.Y, Value.LastPreAdditiveVelocity.Z));
			}
			{
				Update.set_field_reprootmotion_authoritativerootmotion_bisadditivevelocityapplied(Value.bIsAdditiveVelocityApplied != 0);
			}
			{
				{
					Update.set_field_reprootmotion_authoritativerootmotion_lastaccumulatedsettings_flags(uint32_t(Value.LastAccumulatedSettings.Flags));
				}
			}
			break;
		}
		case 42: // field_reprootmotion_acceleration
		{
			FVector_NetQuantize10 Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);

			Update.set_field_reprootmotion_acceleration(improbable::Vector3f(Value.X, Value.Y, Value.Z));
			break;
		}
		case 43: // field_reprootmotion_linearvelocity
		{
			FVector_NetQuantize10 Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);

			Update.set_field_reprootmotion_linearvelocity(improbable::Vector3f(Value.X, Value.Y, Value.Z));
			break;
		}
	}
}

void ReceiveUpdateFromSpatial_Character(USpatialActorChannel* ActorChannel, const improbable::unreal::UnrealCharacterReplicatedData::Update& Update)
{
	FNetBitWriter OutputWriter(nullptr, 0); 
	auto& HandleToPropertyMap = GetHandlePropertyMap_Character();
	if (!Update.field_bhidden().empty())
	{
		// field_bhidden
		uint32 Handle = 1;
		OutputWriter.SerializeIntPacked(Handle);
		const RepHandleData& Data = HandleToPropertyMap[Handle];

		uint8 Value;
		check(Data.Property->ElementSize == sizeof(Value));

		Value = *(Update.field_bhidden().data());

		Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
		UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
	}
	if (!Update.field_breplicatemovement().empty())
	{
		// field_breplicatemovement
		uint32 Handle = 2;
		OutputWriter.SerializeIntPacked(Handle);
		const RepHandleData& Data = HandleToPropertyMap[Handle];

		uint8 Value;
		check(Data.Property->ElementSize == sizeof(Value));

		Value = *(Update.field_breplicatemovement().data());

		Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
		UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
	}
	if (!Update.field_btearoff().empty())
	{
		// field_btearoff
		uint32 Handle = 3;
		OutputWriter.SerializeIntPacked(Handle);
		const RepHandleData& Data = HandleToPropertyMap[Handle];

		uint8 Value;
		check(Data.Property->ElementSize == sizeof(Value));

		Value = *(Update.field_btearoff().data());

		Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
		UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
	}
	if (!Update.field_remoterole().empty())
	{
		// field_remoterole
		uint32 Handle = 4;
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
	if (!Update.field_owner().empty())
	{
		// field_owner
		uint32 Handle = 5;
		OutputWriter.SerializeIntPacked(Handle);
		const RepHandleData& Data = HandleToPropertyMap[Handle];

		AActor* Value;
		check(Data.Property->ElementSize == sizeof(Value));

		// UNSUPPORTED ObjectProperty - Value *(Update.field_owner().data());

		Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
		UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
	}
	if (!Update.field_replicatedmovement().empty())
	{
		// field_replicatedmovement
		uint32 Handle = 6;
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
	if (!Update.field_attachmentreplication_attachparent().empty())
	{
		// field_attachmentreplication_attachparent
		uint32 Handle = 7;
		OutputWriter.SerializeIntPacked(Handle);
		const RepHandleData& Data = HandleToPropertyMap[Handle];

		AActor* Value;
		check(Data.Property->ElementSize == sizeof(Value));

		// UNSUPPORTED ObjectProperty - Value *(Update.field_attachmentreplication_attachparent().data());

		Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
		UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
	}
	if (!Update.field_attachmentreplication_locationoffset().empty())
	{
		// field_attachmentreplication_locationoffset
		uint32 Handle = 8;
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
	if (!Update.field_attachmentreplication_relativescale3d().empty())
	{
		// field_attachmentreplication_relativescale3d
		uint32 Handle = 9;
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
	if (!Update.field_attachmentreplication_rotationoffset().empty())
	{
		// field_attachmentreplication_rotationoffset
		uint32 Handle = 10;
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
	if (!Update.field_attachmentreplication_attachsocket().empty())
	{
		// field_attachmentreplication_attachsocket
		uint32 Handle = 11;
		OutputWriter.SerializeIntPacked(Handle);
		const RepHandleData& Data = HandleToPropertyMap[Handle];

		FName Value;
		check(Data.Property->ElementSize == sizeof(Value));

		Value = FName((*(Update.field_attachmentreplication_attachsocket().data())).data());

		Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
		UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
	}
	if (!Update.field_attachmentreplication_attachcomponent().empty())
	{
		// field_attachmentreplication_attachcomponent
		uint32 Handle = 12;
		OutputWriter.SerializeIntPacked(Handle);
		const RepHandleData& Data = HandleToPropertyMap[Handle];

		USceneComponent* Value;
		check(Data.Property->ElementSize == sizeof(Value));

		// UNSUPPORTED ObjectProperty - Value *(Update.field_attachmentreplication_attachcomponent().data());

		Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
		UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
	}
	if (!Update.field_role().empty())
	{
		// field_role
		uint32 Handle = 13;
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
	if (!Update.field_bcanbedamaged().empty())
	{
		// field_bcanbedamaged
		uint32 Handle = 14;
		OutputWriter.SerializeIntPacked(Handle);
		const RepHandleData& Data = HandleToPropertyMap[Handle];

		uint8 Value;
		check(Data.Property->ElementSize == sizeof(Value));

		Value = *(Update.field_bcanbedamaged().data());

		Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
		UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
	}
	if (!Update.field_instigator().empty())
	{
		// field_instigator
		uint32 Handle = 15;
		OutputWriter.SerializeIntPacked(Handle);
		const RepHandleData& Data = HandleToPropertyMap[Handle];

		APawn* Value;
		check(Data.Property->ElementSize == sizeof(Value));

		// UNSUPPORTED ObjectProperty - Value *(Update.field_instigator().data());

		Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
		UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
	}
	if (!Update.field_playerstate().empty())
	{
		// field_playerstate
		uint32 Handle = 16;
		OutputWriter.SerializeIntPacked(Handle);
		const RepHandleData& Data = HandleToPropertyMap[Handle];

		APlayerState* Value;
		check(Data.Property->ElementSize == sizeof(Value));

		// UNSUPPORTED ObjectProperty - Value *(Update.field_playerstate().data());

		Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
		UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
	}
	if (!Update.field_remoteviewpitch().empty())
	{
		// field_remoteviewpitch
		uint32 Handle = 17;
		OutputWriter.SerializeIntPacked(Handle);
		const RepHandleData& Data = HandleToPropertyMap[Handle];

		uint8 Value;
		check(Data.Property->ElementSize == sizeof(Value));

		// Byte properties are weird, because they can also be an enum in the form TEnumAsByte<...>.
		// Therefore, the code generator needs to cast to either TEnumAsByte<...> or uint8. However,
		// as TEnumAsByte<...> only has a uint8 constructor, we need to cast the SpatialOS value into
		// uint8 first, which causes "uint8(uint8(...))" to be generated for non enum bytes.
		Value = uint8(uint8(*(Update.field_remoteviewpitch().data())));

		Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
		UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
	}
	if (!Update.field_controller().empty())
	{
		// field_controller
		uint32 Handle = 18;
		OutputWriter.SerializeIntPacked(Handle);
		const RepHandleData& Data = HandleToPropertyMap[Handle];

		AController* Value;
		check(Data.Property->ElementSize == sizeof(Value));

		// UNSUPPORTED ObjectProperty - Value *(Update.field_controller().data());

		Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
		UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
	}
	if (!Update.field_replicatedbasedmovement_movementbase().empty())
	{
		// field_replicatedbasedmovement_movementbase
		uint32 Handle = 19;
		OutputWriter.SerializeIntPacked(Handle);
		const RepHandleData& Data = HandleToPropertyMap[Handle];

		UPrimitiveComponent* Value;
		check(Data.Property->ElementSize == sizeof(Value));

		// UNSUPPORTED ObjectProperty - Value *(Update.field_replicatedbasedmovement_movementbase().data());

		Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
		UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
	}
	if (!Update.field_replicatedbasedmovement_bonename().empty())
	{
		// field_replicatedbasedmovement_bonename
		uint32 Handle = 20;
		OutputWriter.SerializeIntPacked(Handle);
		const RepHandleData& Data = HandleToPropertyMap[Handle];

		FName Value;
		check(Data.Property->ElementSize == sizeof(Value));

		Value = FName((*(Update.field_replicatedbasedmovement_bonename().data())).data());

		Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
		UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
	}
	if (!Update.field_replicatedbasedmovement_location().empty())
	{
		// field_replicatedbasedmovement_location
		uint32 Handle = 21;
		OutputWriter.SerializeIntPacked(Handle);
		const RepHandleData& Data = HandleToPropertyMap[Handle];

		FVector_NetQuantize100 Value;
		check(Data.Property->ElementSize == sizeof(Value));

		auto& Vector = *(Update.field_replicatedbasedmovement_location().data());
		Value.X = Vector.x();
		Value.Y = Vector.y();
		Value.Z = Vector.z();

		Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
		UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
	}
	if (!Update.field_replicatedbasedmovement_rotation().empty())
	{
		// field_replicatedbasedmovement_rotation
		uint32 Handle = 22;
		OutputWriter.SerializeIntPacked(Handle);
		const RepHandleData& Data = HandleToPropertyMap[Handle];

		FRotator Value;
		check(Data.Property->ElementSize == sizeof(Value));

		auto& Rotator = *(Update.field_replicatedbasedmovement_rotation().data());
		Value.Yaw = Rotator.yaw();
		Value.Pitch = Rotator.pitch();
		Value.Roll = Rotator.roll();

		Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
		UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
	}
	if (!Update.field_replicatedbasedmovement_bserverhasbasecomponent().empty())
	{
		// field_replicatedbasedmovement_bserverhasbasecomponent
		uint32 Handle = 23;
		OutputWriter.SerializeIntPacked(Handle);
		const RepHandleData& Data = HandleToPropertyMap[Handle];

		bool Value;
		check(Data.Property->ElementSize == sizeof(Value));

		Value = *(Update.field_replicatedbasedmovement_bserverhasbasecomponent().data());

		Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
		UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
	}
	if (!Update.field_replicatedbasedmovement_brelativerotation().empty())
	{
		// field_replicatedbasedmovement_brelativerotation
		uint32 Handle = 24;
		OutputWriter.SerializeIntPacked(Handle);
		const RepHandleData& Data = HandleToPropertyMap[Handle];

		bool Value;
		check(Data.Property->ElementSize == sizeof(Value));

		Value = *(Update.field_replicatedbasedmovement_brelativerotation().data());

		Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
		UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
	}
	if (!Update.field_replicatedbasedmovement_bserverhasvelocity().empty())
	{
		// field_replicatedbasedmovement_bserverhasvelocity
		uint32 Handle = 25;
		OutputWriter.SerializeIntPacked(Handle);
		const RepHandleData& Data = HandleToPropertyMap[Handle];

		bool Value;
		check(Data.Property->ElementSize == sizeof(Value));

		Value = *(Update.field_replicatedbasedmovement_bserverhasvelocity().data());

		Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
		UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
	}
	if (!Update.field_animrootmotiontranslationscale().empty())
	{
		// field_animrootmotiontranslationscale
		uint32 Handle = 26;
		OutputWriter.SerializeIntPacked(Handle);
		const RepHandleData& Data = HandleToPropertyMap[Handle];

		float Value;
		check(Data.Property->ElementSize == sizeof(Value));

		Value = *(Update.field_animrootmotiontranslationscale().data());

		Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
		UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
	}
	if (!Update.field_replicatedserverlasttransformupdatetimestamp().empty())
	{
		// field_replicatedserverlasttransformupdatetimestamp
		uint32 Handle = 27;
		OutputWriter.SerializeIntPacked(Handle);
		const RepHandleData& Data = HandleToPropertyMap[Handle];

		float Value;
		check(Data.Property->ElementSize == sizeof(Value));

		Value = *(Update.field_replicatedserverlasttransformupdatetimestamp().data());

		Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
		UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
	}
	if (!Update.field_replicatedmovementmode().empty())
	{
		// field_replicatedmovementmode
		uint32 Handle = 28;
		OutputWriter.SerializeIntPacked(Handle);
		const RepHandleData& Data = HandleToPropertyMap[Handle];

		uint8 Value;
		check(Data.Property->ElementSize == sizeof(Value));

		// Byte properties are weird, because they can also be an enum in the form TEnumAsByte<...>.
		// Therefore, the code generator needs to cast to either TEnumAsByte<...> or uint8. However,
		// as TEnumAsByte<...> only has a uint8 constructor, we need to cast the SpatialOS value into
		// uint8 first, which causes "uint8(uint8(...))" to be generated for non enum bytes.
		Value = uint8(uint8(*(Update.field_replicatedmovementmode().data())));

		Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
		UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
	}
	if (!Update.field_biscrouched().empty())
	{
		// field_biscrouched
		uint32 Handle = 29;
		OutputWriter.SerializeIntPacked(Handle);
		const RepHandleData& Data = HandleToPropertyMap[Handle];

		uint8 Value;
		check(Data.Property->ElementSize == sizeof(Value));

		Value = *(Update.field_biscrouched().data());

		Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
		UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
	}
	if (!Update.field_jumpmaxholdtime().empty())
	{
		// field_jumpmaxholdtime
		uint32 Handle = 30;
		OutputWriter.SerializeIntPacked(Handle);
		const RepHandleData& Data = HandleToPropertyMap[Handle];

		float Value;
		check(Data.Property->ElementSize == sizeof(Value));

		Value = *(Update.field_jumpmaxholdtime().data());

		Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
		UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
	}
	if (!Update.field_jumpmaxcount().empty())
	{
		// field_jumpmaxcount
		uint32 Handle = 31;
		OutputWriter.SerializeIntPacked(Handle);
		const RepHandleData& Data = HandleToPropertyMap[Handle];

		int32 Value;
		check(Data.Property->ElementSize == sizeof(Value));

		Value = *(Update.field_jumpmaxcount().data());

		Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
		UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
	}
	if (!Update.field_reprootmotion_bisactive().empty())
	{
		// field_reprootmotion_bisactive
		uint32 Handle = 32;
		OutputWriter.SerializeIntPacked(Handle);
		const RepHandleData& Data = HandleToPropertyMap[Handle];

		bool Value;
		check(Data.Property->ElementSize == sizeof(Value));

		Value = *(Update.field_reprootmotion_bisactive().data());

		Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
		UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
	}
	if (!Update.field_reprootmotion_animmontage().empty())
	{
		// field_reprootmotion_animmontage
		uint32 Handle = 33;
		OutputWriter.SerializeIntPacked(Handle);
		const RepHandleData& Data = HandleToPropertyMap[Handle];

		UAnimMontage* Value;
		check(Data.Property->ElementSize == sizeof(Value));

		// UNSUPPORTED ObjectProperty - Value *(Update.field_reprootmotion_animmontage().data());

		Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
		UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
	}
	if (!Update.field_reprootmotion_position().empty())
	{
		// field_reprootmotion_position
		uint32 Handle = 34;
		OutputWriter.SerializeIntPacked(Handle);
		const RepHandleData& Data = HandleToPropertyMap[Handle];

		float Value;
		check(Data.Property->ElementSize == sizeof(Value));

		Value = *(Update.field_reprootmotion_position().data());

		Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
		UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
	}
	if (!Update.field_reprootmotion_location().empty())
	{
		// field_reprootmotion_location
		uint32 Handle = 35;
		OutputWriter.SerializeIntPacked(Handle);
		const RepHandleData& Data = HandleToPropertyMap[Handle];

		FVector_NetQuantize100 Value;
		check(Data.Property->ElementSize == sizeof(Value));

		auto& Vector = *(Update.field_reprootmotion_location().data());
		Value.X = Vector.x();
		Value.Y = Vector.y();
		Value.Z = Vector.z();

		Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
		UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
	}
	if (!Update.field_reprootmotion_rotation().empty())
	{
		// field_reprootmotion_rotation
		uint32 Handle = 36;
		OutputWriter.SerializeIntPacked(Handle);
		const RepHandleData& Data = HandleToPropertyMap[Handle];

		FRotator Value;
		check(Data.Property->ElementSize == sizeof(Value));

		auto& Rotator = *(Update.field_reprootmotion_rotation().data());
		Value.Yaw = Rotator.yaw();
		Value.Pitch = Rotator.pitch();
		Value.Roll = Rotator.roll();

		Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
		UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
	}
	if (!Update.field_reprootmotion_movementbase().empty())
	{
		// field_reprootmotion_movementbase
		uint32 Handle = 37;
		OutputWriter.SerializeIntPacked(Handle);
		const RepHandleData& Data = HandleToPropertyMap[Handle];

		UPrimitiveComponent* Value;
		check(Data.Property->ElementSize == sizeof(Value));

		// UNSUPPORTED ObjectProperty - Value *(Update.field_reprootmotion_movementbase().data());

		Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
		UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
	}
	if (!Update.field_reprootmotion_movementbasebonename().empty())
	{
		// field_reprootmotion_movementbasebonename
		uint32 Handle = 38;
		OutputWriter.SerializeIntPacked(Handle);
		const RepHandleData& Data = HandleToPropertyMap[Handle];

		FName Value;
		check(Data.Property->ElementSize == sizeof(Value));

		Value = FName((*(Update.field_reprootmotion_movementbasebonename().data())).data());

		Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
		UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
	}
	if (!Update.field_reprootmotion_brelativeposition().empty())
	{
		// field_reprootmotion_brelativeposition
		uint32 Handle = 39;
		OutputWriter.SerializeIntPacked(Handle);
		const RepHandleData& Data = HandleToPropertyMap[Handle];

		bool Value;
		check(Data.Property->ElementSize == sizeof(Value));

		Value = *(Update.field_reprootmotion_brelativeposition().data());

		Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
		UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
	}
	if (!Update.field_reprootmotion_brelativerotation().empty())
	{
		// field_reprootmotion_brelativerotation
		uint32 Handle = 40;
		OutputWriter.SerializeIntPacked(Handle);
		const RepHandleData& Data = HandleToPropertyMap[Handle];

		bool Value;
		check(Data.Property->ElementSize == sizeof(Value));

		Value = *(Update.field_reprootmotion_brelativerotation().data());

		Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
		UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
	}
	if (!Update.field_reprootmotion_authoritativerootmotion_bhasadditivesources().empty())
	{
		// field_reprootmotion_authoritativerootmotion
		uint32 Handle = 41;
		OutputWriter.SerializeIntPacked(Handle);
		const RepHandleData& Data = HandleToPropertyMap[Handle];

		FRootMotionSourceGroup Value;
		check(Data.Property->ElementSize == sizeof(Value));

		{
			Value.bHasAdditiveSources = *(Update.field_reprootmotion_authoritativerootmotion_bhasadditivesources().data());
		}
		{
			Value.bHasOverrideSources = *(Update.field_reprootmotion_authoritativerootmotion_bhasoverridesources().data());
		}
		{
			auto& Vector = *(Update.field_reprootmotion_authoritativerootmotion_lastpreadditivevelocity().data());
			Value.LastPreAdditiveVelocity.X = Vector.x();
			Value.LastPreAdditiveVelocity.Y = Vector.y();
			Value.LastPreAdditiveVelocity.Z = Vector.z();
		}
		{
			Value.bIsAdditiveVelocityApplied = *(Update.field_reprootmotion_authoritativerootmotion_bisadditivevelocityapplied().data());
		}
		{
			{
				// Byte properties are weird, because they can also be an enum in the form TEnumAsByte<...>.
				// Therefore, the code generator needs to cast to either TEnumAsByte<...> or uint8. However,
				// as TEnumAsByte<...> only has a uint8 constructor, we need to cast the SpatialOS value into
				// uint8 first, which causes "uint8(uint8(...))" to be generated for non enum bytes.
				Value.LastAccumulatedSettings.Flags = uint8(uint8(*(Update.field_reprootmotion_authoritativerootmotion_lastaccumulatedsettings_flags().data())));
			}
		}

		Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
		UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
	}
	if (!Update.field_reprootmotion_acceleration().empty())
	{
		// field_reprootmotion_acceleration
		uint32 Handle = 42;
		OutputWriter.SerializeIntPacked(Handle);
		const RepHandleData& Data = HandleToPropertyMap[Handle];

		FVector_NetQuantize10 Value;
		check(Data.Property->ElementSize == sizeof(Value));

		auto& Vector = *(Update.field_reprootmotion_acceleration().data());
		Value.X = Vector.x();
		Value.Y = Vector.y();
		Value.Z = Vector.z();

		Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
		UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
	}
	if (!Update.field_reprootmotion_linearvelocity().empty())
	{
		// field_reprootmotion_linearvelocity
		uint32 Handle = 43;
		OutputWriter.SerializeIntPacked(Handle);
		const RepHandleData& Data = HandleToPropertyMap[Handle];

		FVector_NetQuantize10 Value;
		check(Data.Property->ElementSize == sizeof(Value));

		auto& Vector = *(Update.field_reprootmotion_linearvelocity().data());
		Value.X = Vector.x();
		Value.Y = Vector.y();
		Value.Z = Vector.z();

		Data.Property->NetSerializeItem(OutputWriter, nullptr, &Value);
		UE_LOG(LogTemp, Log, TEXT("<- Handle: %d Property %s"), Handle, *Data.Property->GetName());
	}
	ActorChannel->SpatialReceivePropertyUpdate(OutputWriter);
}
