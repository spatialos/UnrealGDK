#include "SpatialInteropCharacter.h"
#include "CoreMinimal.h"
#include "Misc/Base64.h"

TMap<int, RepHandleData> CreateHandleToPropertyMap_Character()
{
	UClass* Class = ACharacter::StaticClass();
	TMap<int, RepHandleData> Properties;
	Properties.Add(1, RepHandleData{nullptr, Class->FindPropertyByName("bHidden"), 148});
	Properties.Add(2, RepHandleData{nullptr, Class->FindPropertyByName("bReplicateMovement"), 148});
	Properties.Add(3, RepHandleData{nullptr, Class->FindPropertyByName("bTearOff"), 148});
	Properties.Add(4, RepHandleData{nullptr, Class->FindPropertyByName("RemoteRole"), 164});
	Properties.Add(5, RepHandleData{nullptr, Class->FindPropertyByName("Owner"), 168});
	Properties.Add(6, RepHandleData{nullptr, Class->FindPropertyByName("ReplicatedMovement"), 176});
	Properties.Add(7, RepHandleData{Class->FindPropertyByName("AttachmentReplication"), nullptr, 232});
	Properties[7].Property = Cast<UStructProperty>(Properties[7].Parent)->Struct->FindPropertyByName("AttachParent");
	Properties.Add(8, RepHandleData{Class->FindPropertyByName("AttachmentReplication"), nullptr, 240});
	Properties[8].Property = Cast<UStructProperty>(Properties[8].Parent)->Struct->FindPropertyByName("LocationOffset");
	Properties.Add(9, RepHandleData{Class->FindPropertyByName("AttachmentReplication"), nullptr, 252});
	Properties[9].Property = Cast<UStructProperty>(Properties[9].Parent)->Struct->FindPropertyByName("RelativeScale3D");
	Properties.Add(10, RepHandleData{Class->FindPropertyByName("AttachmentReplication"), nullptr, 264});
	Properties[10].Property = Cast<UStructProperty>(Properties[10].Parent)->Struct->FindPropertyByName("RotationOffset");
	Properties.Add(11, RepHandleData{Class->FindPropertyByName("AttachmentReplication"), nullptr, 276});
	Properties[11].Property = Cast<UStructProperty>(Properties[11].Parent)->Struct->FindPropertyByName("AttachSocket");
	Properties.Add(12, RepHandleData{Class->FindPropertyByName("AttachmentReplication"), nullptr, 288});
	Properties[12].Property = Cast<UStructProperty>(Properties[12].Parent)->Struct->FindPropertyByName("AttachComponent");
	Properties.Add(13, RepHandleData{nullptr, Class->FindPropertyByName("Role"), 296});
	Properties.Add(14, RepHandleData{nullptr, Class->FindPropertyByName("bCanBeDamaged"), 344});
	Properties.Add(15, RepHandleData{nullptr, Class->FindPropertyByName("Instigator"), 352});
	Properties.Add(16, RepHandleData{nullptr, Class->FindPropertyByName("PlayerState"), 1104});
	Properties.Add(17, RepHandleData{nullptr, Class->FindPropertyByName("RemoteViewPitch"), 1112});
	Properties.Add(18, RepHandleData{nullptr, Class->FindPropertyByName("Controller"), 1128});
	Properties.Add(19, RepHandleData{Class->FindPropertyByName("ReplicatedBasedMovement"), nullptr, 1248});
	Properties[19].Property = Cast<UStructProperty>(Properties[19].Parent)->Struct->FindPropertyByName("MovementBase");
	Properties.Add(20, RepHandleData{Class->FindPropertyByName("ReplicatedBasedMovement"), nullptr, 1256});
	Properties[20].Property = Cast<UStructProperty>(Properties[20].Parent)->Struct->FindPropertyByName("BoneName");
	Properties.Add(21, RepHandleData{Class->FindPropertyByName("ReplicatedBasedMovement"), nullptr, 1268});
	Properties[21].Property = Cast<UStructProperty>(Properties[21].Parent)->Struct->FindPropertyByName("Location");
	Properties.Add(22, RepHandleData{Class->FindPropertyByName("ReplicatedBasedMovement"), nullptr, 1280});
	Properties[22].Property = Cast<UStructProperty>(Properties[22].Parent)->Struct->FindPropertyByName("Rotation");
	Properties.Add(23, RepHandleData{Class->FindPropertyByName("ReplicatedBasedMovement"), nullptr, 1292});
	Properties[23].Property = Cast<UStructProperty>(Properties[23].Parent)->Struct->FindPropertyByName("bServerHasBaseComponent");
	Properties.Add(24, RepHandleData{Class->FindPropertyByName("ReplicatedBasedMovement"), nullptr, 1293});
	Properties[24].Property = Cast<UStructProperty>(Properties[24].Parent)->Struct->FindPropertyByName("bRelativeRotation");
	Properties.Add(25, RepHandleData{Class->FindPropertyByName("ReplicatedBasedMovement"), nullptr, 1294});
	Properties[25].Property = Cast<UStructProperty>(Properties[25].Parent)->Struct->FindPropertyByName("bServerHasVelocity");
	Properties.Add(26, RepHandleData{nullptr, Class->FindPropertyByName("AnimRootMotionTranslationScale"), 1296});
	Properties.Add(27, RepHandleData{nullptr, Class->FindPropertyByName("ReplicatedServerLastTransformUpdateTimeStamp"), 1328});
	Properties.Add(28, RepHandleData{nullptr, Class->FindPropertyByName("ReplicatedMovementMode"), 1332});
	Properties.Add(29, RepHandleData{nullptr, Class->FindPropertyByName("bIsCrouched"), 1340});
	Properties.Add(30, RepHandleData{nullptr, Class->FindPropertyByName("JumpMaxHoldTime"), 1348});
	Properties.Add(31, RepHandleData{nullptr, Class->FindPropertyByName("JumpMaxCount"), 1352});
	Properties.Add(32, RepHandleData{Class->FindPropertyByName("RepRootMotion"), nullptr, 1776});
	Properties[32].Property = Cast<UStructProperty>(Properties[32].Parent)->Struct->FindPropertyByName("bIsActive");
	Properties.Add(33, RepHandleData{Class->FindPropertyByName("RepRootMotion"), nullptr, 1784});
	Properties[33].Property = Cast<UStructProperty>(Properties[33].Parent)->Struct->FindPropertyByName("AnimMontage");
	Properties.Add(34, RepHandleData{Class->FindPropertyByName("RepRootMotion"), nullptr, 1792});
	Properties[34].Property = Cast<UStructProperty>(Properties[34].Parent)->Struct->FindPropertyByName("Position");
	Properties.Add(35, RepHandleData{Class->FindPropertyByName("RepRootMotion"), nullptr, 1796});
	Properties[35].Property = Cast<UStructProperty>(Properties[35].Parent)->Struct->FindPropertyByName("Location");
	Properties.Add(36, RepHandleData{Class->FindPropertyByName("RepRootMotion"), nullptr, 1808});
	Properties[36].Property = Cast<UStructProperty>(Properties[36].Parent)->Struct->FindPropertyByName("Rotation");
	Properties.Add(37, RepHandleData{Class->FindPropertyByName("RepRootMotion"), nullptr, 1824});
	Properties[37].Property = Cast<UStructProperty>(Properties[37].Parent)->Struct->FindPropertyByName("MovementBase");
	Properties.Add(38, RepHandleData{Class->FindPropertyByName("RepRootMotion"), nullptr, 1832});
	Properties[38].Property = Cast<UStructProperty>(Properties[38].Parent)->Struct->FindPropertyByName("MovementBaseBoneName");
	Properties.Add(39, RepHandleData{Class->FindPropertyByName("RepRootMotion"), nullptr, 1844});
	Properties[39].Property = Cast<UStructProperty>(Properties[39].Parent)->Struct->FindPropertyByName("bRelativePosition");
	Properties.Add(40, RepHandleData{Class->FindPropertyByName("RepRootMotion"), nullptr, 1845});
	Properties[40].Property = Cast<UStructProperty>(Properties[40].Parent)->Struct->FindPropertyByName("bRelativeRotation");
	Properties.Add(41, RepHandleData{Class->FindPropertyByName("RepRootMotion"), nullptr, 1848});
	Properties[41].Property = Cast<UStructProperty>(Properties[41].Parent)->Struct->FindPropertyByName("AuthoritativeRootMotion");
	Properties.Add(42, RepHandleData{Class->FindPropertyByName("RepRootMotion"), nullptr, 2104});
	Properties[42].Property = Cast<UStructProperty>(Properties[42].Parent)->Struct->FindPropertyByName("Acceleration");
	Properties.Add(43, RepHandleData{Class->FindPropertyByName("RepRootMotion"), nullptr, 2116});
	Properties[43].Property = Cast<UStructProperty>(Properties[43].Parent)->Struct->FindPropertyByName("LinearVelocity");
	return Properties;
}

void ApplyUpdateToSpatial_Character(FArchive& Reader, int32 Handle, UProperty* Property, UUnrealACharacterReplicatedDataComponent* ReplicatedData)
{
	switch (Handle)
	{
		case 1:
		{
			uint8 Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);
			ReplicatedData->FieldBhidden = Value != 0;
			break;
		}
		case 2:
		{
			uint8 Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);
			ReplicatedData->FieldBreplicatemovement = Value != 0;
			break;
		}
		case 3:
		{
			uint8 Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);
			ReplicatedData->FieldBtearoff = Value != 0;
			break;
		}
		case 4:
		{
			TEnumAsByte<ENetRole> Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);
			ReplicatedData->FieldRemoterole = int(Value);
			break;
		}
		// case 5: - Owner is an object reference, skipping.
		case 6:
		{
			FRepMovement Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);
			TArray<uint8> Data;
			FMemoryWriter Writer(Data);
			bool Success;
			Value.NetSerialize(Writer, nullptr, Success);
			ReplicatedData->FieldReplicatedmovement = FBase64::Encode(Data);
			break;
		}
		// case 7: - AttachParent is an object reference, skipping.
		case 8:
		{
			FVector_NetQuantize100 Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);
			ReplicatedData->FieldAttachmentreplicationLocationoffset = Value;
			break;
		}
		case 9:
		{
			FVector_NetQuantize100 Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);
			ReplicatedData->FieldAttachmentreplicationRelativescale3d = Value;
			break;
		}
		case 10:
		{
			FRotator Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);
			auto& Rotator = ReplicatedData->FieldAttachmentreplicationRotationoffset;
			Rotator->SetPitch(Value.Pitch);
			Rotator->SetYaw(Value.Yaw);
			Rotator->SetRoll(Value.Roll);
			break;
		}
		case 11:
		{
			FName Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);
			ReplicatedData->FieldAttachmentreplicationAttachsocket = Value.ToString();
			break;
		}
		// case 12: - AttachComponent is an object reference, skipping.
		case 13:
		{
			TEnumAsByte<ENetRole> Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);
			ReplicatedData->FieldRole = int(Value);
			break;
		}
		case 14:
		{
			uint8 Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);
			ReplicatedData->FieldBcanbedamaged = Value != 0;
			break;
		}
		// case 15: - Instigator is an object reference, skipping.
		// case 16: - PlayerState is an object reference, skipping.
		case 17:
		{
			uint8 Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);
			ReplicatedData->FieldRemoteviewpitch = int(Value);
			break;
		}
		// case 18: - Controller is an object reference, skipping.
		// case 19: - MovementBase is an object reference, skipping.
		case 20:
		{
			FName Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);
			ReplicatedData->FieldReplicatedbasedmovementBonename = Value.ToString();
			break;
		}
		case 21:
		{
			FVector_NetQuantize100 Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);
			ReplicatedData->FieldReplicatedbasedmovementLocation = Value;
			break;
		}
		case 22:
		{
			FRotator Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);
			auto& Rotator = ReplicatedData->FieldReplicatedbasedmovementRotation;
			Rotator->SetPitch(Value.Pitch);
			Rotator->SetYaw(Value.Yaw);
			Rotator->SetRoll(Value.Roll);
			break;
		}
		case 23:
		{
			bool Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);
			ReplicatedData->FieldReplicatedbasedmovementBserverhasbasecomponent = Value != 0;
			break;
		}
		case 24:
		{
			bool Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);
			ReplicatedData->FieldReplicatedbasedmovementBrelativerotation = Value != 0;
			break;
		}
		case 25:
		{
			bool Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);
			ReplicatedData->FieldReplicatedbasedmovementBserverhasvelocity = Value != 0;
			break;
		}
		case 26:
		{
			float Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);
			ReplicatedData->FieldAnimrootmotiontranslationscale = Value;
			break;
		}
		case 27:
		{
			float Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);
			ReplicatedData->FieldReplicatedserverlasttransformupdatetimestamp = Value;
			break;
		}
		case 28:
		{
			uint8 Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);
			ReplicatedData->FieldReplicatedmovementmode = int(Value);
			break;
		}
		case 29:
		{
			uint8 Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);
			ReplicatedData->FieldBiscrouched = Value != 0;
			break;
		}
		case 30:
		{
			float Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);
			ReplicatedData->FieldJumpmaxholdtime = Value;
			break;
		}
		case 31:
		{
			int32 Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);
			ReplicatedData->FieldJumpmaxcount = Value;
			break;
		}
		case 32:
		{
			bool Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);
			ReplicatedData->FieldReprootmotionBisactive = Value != 0;
			break;
		}
		// case 33: - AnimMontage is an object reference, skipping.
		case 34:
		{
			float Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);
			ReplicatedData->FieldReprootmotionPosition = Value;
			break;
		}
		case 35:
		{
			FVector_NetQuantize100 Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);
			ReplicatedData->FieldReprootmotionLocation = Value;
			break;
		}
		case 36:
		{
			FRotator Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);
			auto& Rotator = ReplicatedData->FieldReprootmotionRotation;
			Rotator->SetPitch(Value.Pitch);
			Rotator->SetYaw(Value.Yaw);
			Rotator->SetRoll(Value.Roll);
			break;
		}
		// case 37: - MovementBase is an object reference, skipping.
		case 38:
		{
			FName Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);
			ReplicatedData->FieldReprootmotionMovementbasebonename = Value.ToString();
			break;
		}
		case 39:
		{
			bool Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);
			ReplicatedData->FieldReprootmotionBrelativeposition = Value != 0;
			break;
		}
		case 40:
		{
			bool Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);
			ReplicatedData->FieldReprootmotionBrelativerotation = Value != 0;
			break;
		}
		case 41:
		{
			FRootMotionSourceGroup Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);
			{
			}
			{
			}
			{
			}
			{
			}
			{
			}
			break;
		}
		case 42:
		{
			FVector_NetQuantize10 Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);
			ReplicatedData->FieldReprootmotionAcceleration = Value;
			break;
		}
		case 43:
		{
			FVector_NetQuantize10 Value;
			check(Property->ElementSize == sizeof(Value));
			Property->NetSerializeItem(Reader, nullptr, &Value);
			ReplicatedData->FieldReprootmotionLinearvelocity = Value;
			break;
		}
	}
}

void ApplyUpdateToSpatial_Old_Character(AActor* Actor, int32 Handle, UProperty* ParentProperty, UProperty* Property, UUnrealACharacterReplicatedDataComponent* ReplicatedData)
{
	UObject* Container = Actor;
	switch (Handle)
	{
		case 1:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<uint8>(Container);
			ReplicatedData->FieldBhidden = Value != 0;
			break;
		}
		case 2:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<uint8>(Container);
			ReplicatedData->FieldBreplicatemovement = Value != 0;
			break;
		}
		case 3:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<uint8>(Container);
			ReplicatedData->FieldBtearoff = Value != 0;
			break;
		}
		case 4:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<TEnumAsByte<ENetRole>>(Container);
			ReplicatedData->FieldRemoterole = int(Value);
			break;
		}
		case 5:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<AActor*>(Container);
			// WEAK OBJECT REPLICATION - ReplicatedData->FieldOwner = Value;
			break;
		}
		case 6:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<FRepMovement>(Container);
			TArray<uint8> Data;
			FMemoryWriter Writer(Data);
			bool Success;
			Value.NetSerialize(Writer, nullptr, Success);
			ReplicatedData->FieldReplicatedmovement = FBase64::Encode(Data);
			break;
		}
		case 7:
		{
			auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepAttachment>(Container);
			// WEAK OBJECT REPLICATION - ReplicatedData->FieldAttachmentreplicationAttachparent = Value.AttachParent;
			break;
		}
		case 8:
		{
			auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepAttachment>(Container);
			ReplicatedData->FieldAttachmentreplicationLocationoffset = Value.LocationOffset;
			break;
		}
		case 9:
		{
			auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepAttachment>(Container);
			ReplicatedData->FieldAttachmentreplicationRelativescale3d = Value.RelativeScale3D;
			break;
		}
		case 10:
		{
			auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepAttachment>(Container);
			auto& Rotator = ReplicatedData->FieldAttachmentreplicationRotationoffset;
			Rotator->SetPitch(Value.RotationOffset.Pitch);
			Rotator->SetYaw(Value.RotationOffset.Yaw);
			Rotator->SetRoll(Value.RotationOffset.Roll);
			break;
		}
		case 11:
		{
			auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepAttachment>(Container);
			ReplicatedData->FieldAttachmentreplicationAttachsocket = Value.AttachSocket.ToString();
			break;
		}
		case 12:
		{
			auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepAttachment>(Container);
			// WEAK OBJECT REPLICATION - ReplicatedData->FieldAttachmentreplicationAttachcomponent = Value.AttachComponent;
			break;
		}
		case 13:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<TEnumAsByte<ENetRole>>(Container);
			ReplicatedData->FieldRole = int(Value);
			break;
		}
		case 14:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<uint8>(Container);
			ReplicatedData->FieldBcanbedamaged = Value != 0;
			break;
		}
		case 15:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<APawn*>(Container);
			// WEAK OBJECT REPLICATION - ReplicatedData->FieldInstigator = Value;
			break;
		}
		case 16:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<APlayerState*>(Container);
			// WEAK OBJECT REPLICATION - ReplicatedData->FieldPlayerstate = Value;
			break;
		}
		case 17:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<uint8>(Container);
			ReplicatedData->FieldRemoteviewpitch = int(Value);
			break;
		}
		case 18:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<AController*>(Container);
			// WEAK OBJECT REPLICATION - ReplicatedData->FieldController = Value;
			break;
		}
		case 19:
		{
			auto& Value = *ParentProperty->ContainerPtrToValuePtr<FBasedMovementInfo>(Container);
			// WEAK OBJECT REPLICATION - ReplicatedData->FieldReplicatedbasedmovementMovementbase = Value.MovementBase;
			break;
		}
		case 20:
		{
			auto& Value = *ParentProperty->ContainerPtrToValuePtr<FBasedMovementInfo>(Container);
			ReplicatedData->FieldReplicatedbasedmovementBonename = Value.BoneName.ToString();
			break;
		}
		case 21:
		{
			auto& Value = *ParentProperty->ContainerPtrToValuePtr<FBasedMovementInfo>(Container);
			ReplicatedData->FieldReplicatedbasedmovementLocation = Value.Location;
			break;
		}
		case 22:
		{
			auto& Value = *ParentProperty->ContainerPtrToValuePtr<FBasedMovementInfo>(Container);
			auto& Rotator = ReplicatedData->FieldReplicatedbasedmovementRotation;
			Rotator->SetPitch(Value.Rotation.Pitch);
			Rotator->SetYaw(Value.Rotation.Yaw);
			Rotator->SetRoll(Value.Rotation.Roll);
			break;
		}
		case 23:
		{
			auto& Value = *ParentProperty->ContainerPtrToValuePtr<FBasedMovementInfo>(Container);
			ReplicatedData->FieldReplicatedbasedmovementBserverhasbasecomponent = Value.bServerHasBaseComponent != 0;
			break;
		}
		case 24:
		{
			auto& Value = *ParentProperty->ContainerPtrToValuePtr<FBasedMovementInfo>(Container);
			ReplicatedData->FieldReplicatedbasedmovementBrelativerotation = Value.bRelativeRotation != 0;
			break;
		}
		case 25:
		{
			auto& Value = *ParentProperty->ContainerPtrToValuePtr<FBasedMovementInfo>(Container);
			ReplicatedData->FieldReplicatedbasedmovementBserverhasvelocity = Value.bServerHasVelocity != 0;
			break;
		}
		case 26:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<float>(Container);
			ReplicatedData->FieldAnimrootmotiontranslationscale = Value;
			break;
		}
		case 27:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<float>(Container);
			ReplicatedData->FieldReplicatedserverlasttransformupdatetimestamp = Value;
			break;
		}
		case 28:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<uint8>(Container);
			ReplicatedData->FieldReplicatedmovementmode = int(Value);
			break;
		}
		case 29:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<uint8>(Container);
			ReplicatedData->FieldBiscrouched = Value != 0;
			break;
		}
		case 30:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<float>(Container);
			ReplicatedData->FieldJumpmaxholdtime = Value;
			break;
		}
		case 31:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<int32>(Container);
			ReplicatedData->FieldJumpmaxcount = Value;
			break;
		}
		case 32:
		{
			auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
			ReplicatedData->FieldReprootmotionBisactive = Value.bIsActive != 0;
			break;
		}
		case 33:
		{
			auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
			// WEAK OBJECT REPLICATION - ReplicatedData->FieldReprootmotionAnimmontage = Value.AnimMontage;
			break;
		}
		case 34:
		{
			auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
			ReplicatedData->FieldReprootmotionPosition = Value.Position;
			break;
		}
		case 35:
		{
			auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
			ReplicatedData->FieldReprootmotionLocation = Value.Location;
			break;
		}
		case 36:
		{
			auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
			auto& Rotator = ReplicatedData->FieldReprootmotionRotation;
			Rotator->SetPitch(Value.Rotation.Pitch);
			Rotator->SetYaw(Value.Rotation.Yaw);
			Rotator->SetRoll(Value.Rotation.Roll);
			break;
		}
		case 37:
		{
			auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
			// WEAK OBJECT REPLICATION - ReplicatedData->FieldReprootmotionMovementbase = Value.MovementBase;
			break;
		}
		case 38:
		{
			auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
			ReplicatedData->FieldReprootmotionMovementbasebonename = Value.MovementBaseBoneName.ToString();
			break;
		}
		case 39:
		{
			auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
			ReplicatedData->FieldReprootmotionBrelativeposition = Value.bRelativePosition != 0;
			break;
		}
		case 40:
		{
			auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
			ReplicatedData->FieldReprootmotionBrelativerotation = Value.bRelativeRotation != 0;
			break;
		}
		case 41:
		{
			auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
			{
			}
			{
			}
			{
			}
			{
			}
			{
			}
			break;
		}
		case 42:
		{
			auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
			ReplicatedData->FieldReprootmotionAcceleration = Value.Acceleration;
			break;
		}
		case 43:
		{
			auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
			ReplicatedData->FieldReprootmotionLinearvelocity = Value.LinearVelocity;
			break;
		}
	}
}

void ReceiveUpdateFromSpatial_Character(AActor* Actor, TMap<int, RepHandleData>& HandleToPropertyMap, UUnrealACharacterReplicatedDataComponentUpdate* Update)
{
	UObject* Container = Actor;
	if (Update->HasFieldBhidden())
	{
		RepHandleData& Data = HandleToPropertyMap[1];
		auto& Value = *Data.Property->ContainerPtrToValuePtr<uint8>(Container);
	}
	if (Update->HasFieldBreplicatemovement())
	{
		RepHandleData& Data = HandleToPropertyMap[2];
		auto& Value = *Data.Property->ContainerPtrToValuePtr<uint8>(Container);
	}
	if (Update->HasFieldBtearoff())
	{
		RepHandleData& Data = HandleToPropertyMap[3];
		auto& Value = *Data.Property->ContainerPtrToValuePtr<uint8>(Container);
	}
	if (Update->HasFieldRemoterole())
	{
		RepHandleData& Data = HandleToPropertyMap[4];
		auto& Value = *Data.Property->ContainerPtrToValuePtr<TEnumAsByte<ENetRole>>(Container);
	}
	if (Update->HasFieldOwner())
	{
		RepHandleData& Data = HandleToPropertyMap[5];
		auto& Value = *Data.Property->ContainerPtrToValuePtr<AActor*>(Container);
	}
	if (Update->HasFieldReplicatedmovement())
	{
		RepHandleData& Data = HandleToPropertyMap[6];
		auto& Value = *Data.Property->ContainerPtrToValuePtr<FRepMovement>(Container);
	}
	if (Update->HasFieldAttachmentreplicationAttachparent())
	{
		RepHandleData& Data = HandleToPropertyMap[7];
		auto& Value = *Data.Parent->ContainerPtrToValuePtr<FRepAttachment>(Container);
	}
	if (Update->HasFieldAttachmentreplicationLocationoffset())
	{
		RepHandleData& Data = HandleToPropertyMap[8];
		auto& Value = *Data.Parent->ContainerPtrToValuePtr<FRepAttachment>(Container);
	}
	if (Update->HasFieldAttachmentreplicationRelativescale3d())
	{
		RepHandleData& Data = HandleToPropertyMap[9];
		auto& Value = *Data.Parent->ContainerPtrToValuePtr<FRepAttachment>(Container);
	}
	if (Update->HasFieldAttachmentreplicationRotationoffset())
	{
		RepHandleData& Data = HandleToPropertyMap[10];
		auto& Value = *Data.Parent->ContainerPtrToValuePtr<FRepAttachment>(Container);
	}
	if (Update->HasFieldAttachmentreplicationAttachsocket())
	{
		RepHandleData& Data = HandleToPropertyMap[11];
		auto& Value = *Data.Parent->ContainerPtrToValuePtr<FRepAttachment>(Container);
	}
	if (Update->HasFieldAttachmentreplicationAttachcomponent())
	{
		RepHandleData& Data = HandleToPropertyMap[12];
		auto& Value = *Data.Parent->ContainerPtrToValuePtr<FRepAttachment>(Container);
	}
	if (Update->HasFieldRole())
	{
		RepHandleData& Data = HandleToPropertyMap[13];
		auto& Value = *Data.Property->ContainerPtrToValuePtr<TEnumAsByte<ENetRole>>(Container);
	}
	if (Update->HasFieldBcanbedamaged())
	{
		RepHandleData& Data = HandleToPropertyMap[14];
		auto& Value = *Data.Property->ContainerPtrToValuePtr<uint8>(Container);
	}
	if (Update->HasFieldInstigator())
	{
		RepHandleData& Data = HandleToPropertyMap[15];
		auto& Value = *Data.Property->ContainerPtrToValuePtr<APawn*>(Container);
	}
	if (Update->HasFieldPlayerstate())
	{
		RepHandleData& Data = HandleToPropertyMap[16];
		auto& Value = *Data.Property->ContainerPtrToValuePtr<APlayerState*>(Container);
	}
	if (Update->HasFieldRemoteviewpitch())
	{
		RepHandleData& Data = HandleToPropertyMap[17];
		auto& Value = *Data.Property->ContainerPtrToValuePtr<uint8>(Container);
	}
	if (Update->HasFieldController())
	{
		RepHandleData& Data = HandleToPropertyMap[18];
		auto& Value = *Data.Property->ContainerPtrToValuePtr<AController*>(Container);
	}
	if (Update->HasFieldReplicatedbasedmovementMovementbase())
	{
		RepHandleData& Data = HandleToPropertyMap[19];
		auto& Value = *Data.Parent->ContainerPtrToValuePtr<FBasedMovementInfo>(Container);
	}
	if (Update->HasFieldReplicatedbasedmovementBonename())
	{
		RepHandleData& Data = HandleToPropertyMap[20];
		auto& Value = *Data.Parent->ContainerPtrToValuePtr<FBasedMovementInfo>(Container);
	}
	if (Update->HasFieldReplicatedbasedmovementLocation())
	{
		RepHandleData& Data = HandleToPropertyMap[21];
		auto& Value = *Data.Parent->ContainerPtrToValuePtr<FBasedMovementInfo>(Container);
	}
	if (Update->HasFieldReplicatedbasedmovementRotation())
	{
		RepHandleData& Data = HandleToPropertyMap[22];
		auto& Value = *Data.Parent->ContainerPtrToValuePtr<FBasedMovementInfo>(Container);
	}
	if (Update->HasFieldReplicatedbasedmovementBserverhasbasecomponent())
	{
		RepHandleData& Data = HandleToPropertyMap[23];
		auto& Value = *Data.Parent->ContainerPtrToValuePtr<FBasedMovementInfo>(Container);
	}
	if (Update->HasFieldReplicatedbasedmovementBrelativerotation())
	{
		RepHandleData& Data = HandleToPropertyMap[24];
		auto& Value = *Data.Parent->ContainerPtrToValuePtr<FBasedMovementInfo>(Container);
	}
	if (Update->HasFieldReplicatedbasedmovementBserverhasvelocity())
	{
		RepHandleData& Data = HandleToPropertyMap[25];
		auto& Value = *Data.Parent->ContainerPtrToValuePtr<FBasedMovementInfo>(Container);
	}
	if (Update->HasFieldAnimrootmotiontranslationscale())
	{
		RepHandleData& Data = HandleToPropertyMap[26];
		auto& Value = *Data.Property->ContainerPtrToValuePtr<float>(Container);
	}
	if (Update->HasFieldReplicatedserverlasttransformupdatetimestamp())
	{
		RepHandleData& Data = HandleToPropertyMap[27];
		auto& Value = *Data.Property->ContainerPtrToValuePtr<float>(Container);
	}
	if (Update->HasFieldReplicatedmovementmode())
	{
		RepHandleData& Data = HandleToPropertyMap[28];
		auto& Value = *Data.Property->ContainerPtrToValuePtr<uint8>(Container);
	}
	if (Update->HasFieldBiscrouched())
	{
		RepHandleData& Data = HandleToPropertyMap[29];
		auto& Value = *Data.Property->ContainerPtrToValuePtr<uint8>(Container);
	}
	if (Update->HasFieldJumpmaxholdtime())
	{
		RepHandleData& Data = HandleToPropertyMap[30];
		auto& Value = *Data.Property->ContainerPtrToValuePtr<float>(Container);
	}
	if (Update->HasFieldJumpmaxcount())
	{
		RepHandleData& Data = HandleToPropertyMap[31];
		auto& Value = *Data.Property->ContainerPtrToValuePtr<int32>(Container);
	}
	if (Update->HasFieldReprootmotionBisactive())
	{
		RepHandleData& Data = HandleToPropertyMap[32];
		auto& Value = *Data.Parent->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
	}
	if (Update->HasFieldReprootmotionAnimmontage())
	{
		RepHandleData& Data = HandleToPropertyMap[33];
		auto& Value = *Data.Parent->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
	}
	if (Update->HasFieldReprootmotionPosition())
	{
		RepHandleData& Data = HandleToPropertyMap[34];
		auto& Value = *Data.Parent->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
	}
	if (Update->HasFieldReprootmotionLocation())
	{
		RepHandleData& Data = HandleToPropertyMap[35];
		auto& Value = *Data.Parent->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
	}
	if (Update->HasFieldReprootmotionRotation())
	{
		RepHandleData& Data = HandleToPropertyMap[36];
		auto& Value = *Data.Parent->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
	}
	if (Update->HasFieldReprootmotionMovementbase())
	{
		RepHandleData& Data = HandleToPropertyMap[37];
		auto& Value = *Data.Parent->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
	}
	if (Update->HasFieldReprootmotionMovementbasebonename())
	{
		RepHandleData& Data = HandleToPropertyMap[38];
		auto& Value = *Data.Parent->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
	}
	if (Update->HasFieldReprootmotionBrelativeposition())
	{
		RepHandleData& Data = HandleToPropertyMap[39];
		auto& Value = *Data.Parent->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
	}
	if (Update->HasFieldReprootmotionBrelativerotation())
	{
		RepHandleData& Data = HandleToPropertyMap[40];
		auto& Value = *Data.Parent->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
	}
	if (Update->HasFieldReprootmotionAuthoritativerootmotion())
	{
		RepHandleData& Data = HandleToPropertyMap[41];
		auto& Value = *Data.Parent->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
	}
	if (Update->HasFieldReprootmotionAcceleration())
	{
		RepHandleData& Data = HandleToPropertyMap[42];
		auto& Value = *Data.Parent->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
	}
	if (Update->HasFieldReprootmotionLinearvelocity())
	{
		RepHandleData& Data = HandleToPropertyMap[43];
		auto& Value = *Data.Parent->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
	}
}
