#include "SpatialShadowActor_Character.h"
#include "CoreMinimal.h"
#include "Misc/Base64.h"

ASpatialShadowActor_Character::ASpatialShadowActor_Character()
{
	ReplicatedData = CreateDefaultSubobject<UUnrealACharacterReplicatedDataComponent>(TEXT("UnrealACharacterReplicatedDataComponent"));
	CompleteData = CreateDefaultSubobject<UUnrealACharacterCompleteDataComponent>(TEXT("UnrealACharacterCompleteDataComponent"));

	UClass* Class = ACharacter::StaticClass();
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

void ASpatialShadowActor_Character::ApplyUpdateToSpatial(FArchive& Reader, int32 Handle, UProperty* Property)
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

void ASpatialShadowActor_Character::ReceiveUpdateFromSpatial(AActor* Actor, UUnrealACharacterReplicatedDataComponentUpdate* Update)
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

void ASpatialShadowActor_Character::ReplicateChanges(float DeltaTime)
{
	ReplicatedData->ReplicateChanges(DeltaTime);
}

const TMap<int32, RepHandleData>& ASpatialShadowActor_Character::GetHandlePropertyMap() const
{
	return HandleToPropertyMap;
}
