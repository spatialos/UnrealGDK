#include "SpatialInteropCharacter.h"
#include "CoreMinimal.h"
#include "SpatialPackageMapClient.h"
#include "Misc/Base64.h"

TMap<int, TPair<UProperty*, UProperty*>> CreateCmdIndexToPropertyMap_Character()
{
	UClass* Class = ACharacter::StaticClass();
	TMap<int, TPair<UProperty*, UProperty*>> Properties;
	Properties[0] = MakeTuple<UProperty*, UProperty*>(nullptr, Class->FindPropertyByName("bHidden"));
	Properties[1] = MakeTuple<UProperty*, UProperty*>(nullptr, Class->FindPropertyByName("bReplicateMovement"));
	Properties[2] = MakeTuple<UProperty*, UProperty*>(nullptr, Class->FindPropertyByName("bTearOff"));
	Properties[3] = MakeTuple<UProperty*, UProperty*>(nullptr, Class->FindPropertyByName("RemoteRole"));
	Properties[4] = MakeTuple<UProperty*, UProperty*>(nullptr, Class->FindPropertyByName("Owner"));
	Properties[5] = MakeTuple<UProperty*, UProperty*>(nullptr, Class->FindPropertyByName("ReplicatedMovement"));
	Properties[6] = MakeTuple<UProperty*, UProperty*>(Class->FindPropertyByName("AttachmentReplication"), nullptr);
	Properties[6].Value = Cast<UStructProperty>(Properties[6].Key)->Struct->FindPropertyByName("AttachParent");
	Properties[7] = MakeTuple<UProperty*, UProperty*>(Class->FindPropertyByName("AttachmentReplication"), nullptr);
	Properties[7].Value = Cast<UStructProperty>(Properties[7].Key)->Struct->FindPropertyByName("LocationOffset");
	Properties[8] = MakeTuple<UProperty*, UProperty*>(Class->FindPropertyByName("AttachmentReplication"), nullptr);
	Properties[8].Value = Cast<UStructProperty>(Properties[8].Key)->Struct->FindPropertyByName("RelativeScale3D");
	Properties[9] = MakeTuple<UProperty*, UProperty*>(Class->FindPropertyByName("AttachmentReplication"), nullptr);
	Properties[9].Value = Cast<UStructProperty>(Properties[9].Key)->Struct->FindPropertyByName("RotationOffset");
	Properties[10] = MakeTuple<UProperty*, UProperty*>(Class->FindPropertyByName("AttachmentReplication"), nullptr);
	Properties[10].Value = Cast<UStructProperty>(Properties[10].Key)->Struct->FindPropertyByName("AttachSocket");
	Properties[11] = MakeTuple<UProperty*, UProperty*>(Class->FindPropertyByName("AttachmentReplication"), nullptr);
	Properties[11].Value = Cast<UStructProperty>(Properties[11].Key)->Struct->FindPropertyByName("AttachComponent");
	Properties[12] = MakeTuple<UProperty*, UProperty*>(nullptr, Class->FindPropertyByName("Role"));
	Properties[13] = MakeTuple<UProperty*, UProperty*>(nullptr, Class->FindPropertyByName("bCanBeDamaged"));
	Properties[14] = MakeTuple<UProperty*, UProperty*>(nullptr, Class->FindPropertyByName("Instigator"));
	Properties[15] = MakeTuple<UProperty*, UProperty*>(nullptr, Class->FindPropertyByName("PlayerState"));
	Properties[16] = MakeTuple<UProperty*, UProperty*>(nullptr, Class->FindPropertyByName("RemoteViewPitch"));
	Properties[17] = MakeTuple<UProperty*, UProperty*>(nullptr, Class->FindPropertyByName("Controller"));
	Properties[18] = MakeTuple<UProperty*, UProperty*>(Class->FindPropertyByName("ReplicatedBasedMovement"), nullptr);
	Properties[18].Value = Cast<UStructProperty>(Properties[18].Key)->Struct->FindPropertyByName("MovementBase");
	Properties[19] = MakeTuple<UProperty*, UProperty*>(Class->FindPropertyByName("ReplicatedBasedMovement"), nullptr);
	Properties[19].Value = Cast<UStructProperty>(Properties[19].Key)->Struct->FindPropertyByName("BoneName");
	Properties[20] = MakeTuple<UProperty*, UProperty*>(Class->FindPropertyByName("ReplicatedBasedMovement"), nullptr);
	Properties[20].Value = Cast<UStructProperty>(Properties[20].Key)->Struct->FindPropertyByName("Location");
	Properties[21] = MakeTuple<UProperty*, UProperty*>(Class->FindPropertyByName("ReplicatedBasedMovement"), nullptr);
	Properties[21].Value = Cast<UStructProperty>(Properties[21].Key)->Struct->FindPropertyByName("Rotation");
	Properties[22] = MakeTuple<UProperty*, UProperty*>(Class->FindPropertyByName("ReplicatedBasedMovement"), nullptr);
	Properties[22].Value = Cast<UStructProperty>(Properties[22].Key)->Struct->FindPropertyByName("bServerHasBaseComponent");
	Properties[23] = MakeTuple<UProperty*, UProperty*>(Class->FindPropertyByName("ReplicatedBasedMovement"), nullptr);
	Properties[23].Value = Cast<UStructProperty>(Properties[23].Key)->Struct->FindPropertyByName("bRelativeRotation");
	Properties[24] = MakeTuple<UProperty*, UProperty*>(Class->FindPropertyByName("ReplicatedBasedMovement"), nullptr);
	Properties[24].Value = Cast<UStructProperty>(Properties[24].Key)->Struct->FindPropertyByName("bServerHasVelocity");
	Properties[25] = MakeTuple<UProperty*, UProperty*>(nullptr, Class->FindPropertyByName("AnimRootMotionTranslationScale"));
	Properties[26] = MakeTuple<UProperty*, UProperty*>(nullptr, Class->FindPropertyByName("ReplicatedServerLastTransformUpdateTimeStamp"));
	Properties[27] = MakeTuple<UProperty*, UProperty*>(nullptr, Class->FindPropertyByName("ReplicatedMovementMode"));
	Properties[28] = MakeTuple<UProperty*, UProperty*>(nullptr, Class->FindPropertyByName("bIsCrouched"));
	Properties[29] = MakeTuple<UProperty*, UProperty*>(nullptr, Class->FindPropertyByName("JumpMaxHoldTime"));
	Properties[30] = MakeTuple<UProperty*, UProperty*>(nullptr, Class->FindPropertyByName("JumpMaxCount"));
	Properties[31] = MakeTuple<UProperty*, UProperty*>(Class->FindPropertyByName("RepRootMotion"), nullptr);
	Properties[31].Value = Cast<UStructProperty>(Properties[31].Key)->Struct->FindPropertyByName("bIsActive");
	Properties[32] = MakeTuple<UProperty*, UProperty*>(Class->FindPropertyByName("RepRootMotion"), nullptr);
	Properties[32].Value = Cast<UStructProperty>(Properties[32].Key)->Struct->FindPropertyByName("AnimMontage");
	Properties[33] = MakeTuple<UProperty*, UProperty*>(Class->FindPropertyByName("RepRootMotion"), nullptr);
	Properties[33].Value = Cast<UStructProperty>(Properties[33].Key)->Struct->FindPropertyByName("Position");
	Properties[34] = MakeTuple<UProperty*, UProperty*>(Class->FindPropertyByName("RepRootMotion"), nullptr);
	Properties[34].Value = Cast<UStructProperty>(Properties[34].Key)->Struct->FindPropertyByName("Location");
	Properties[35] = MakeTuple<UProperty*, UProperty*>(Class->FindPropertyByName("RepRootMotion"), nullptr);
	Properties[35].Value = Cast<UStructProperty>(Properties[35].Key)->Struct->FindPropertyByName("Rotation");
	Properties[36] = MakeTuple<UProperty*, UProperty*>(Class->FindPropertyByName("RepRootMotion"), nullptr);
	Properties[36].Value = Cast<UStructProperty>(Properties[36].Key)->Struct->FindPropertyByName("MovementBase");
	Properties[37] = MakeTuple<UProperty*, UProperty*>(Class->FindPropertyByName("RepRootMotion"), nullptr);
	Properties[37].Value = Cast<UStructProperty>(Properties[37].Key)->Struct->FindPropertyByName("MovementBaseBoneName");
	Properties[38] = MakeTuple<UProperty*, UProperty*>(Class->FindPropertyByName("RepRootMotion"), nullptr);
	Properties[38].Value = Cast<UStructProperty>(Properties[38].Key)->Struct->FindPropertyByName("bRelativePosition");
	Properties[39] = MakeTuple<UProperty*, UProperty*>(Class->FindPropertyByName("RepRootMotion"), nullptr);
	Properties[39].Value = Cast<UStructProperty>(Properties[39].Key)->Struct->FindPropertyByName("bRelativeRotation");
	Properties[40] = MakeTuple<UProperty*, UProperty*>(Class->FindPropertyByName("RepRootMotion"), nullptr);
	Properties[40].Value = Cast<UStructProperty>(Properties[40].Key)->Struct->FindPropertyByName("AuthoritativeRootMotion");
	Properties[41] = MakeTuple<UProperty*, UProperty*>(Class->FindPropertyByName("RepRootMotion"), nullptr);
	Properties[41].Value = Cast<UStructProperty>(Properties[41].Key)->Struct->FindPropertyByName("Acceleration");
	Properties[42] = MakeTuple<UProperty*, UProperty*>(Class->FindPropertyByName("RepRootMotion"), nullptr);
	Properties[42].Value = Cast<UStructProperty>(Properties[42].Key)->Struct->FindPropertyByName("LinearVelocity");
	return Properties;
}

void ApplyUpdateToSpatial_Character(AActor* Actor, int CmdIndex, UProperty* ParentProperty, UProperty* Property, UUnrealACharacterReplicatedDataComponent* ReplicatedData, USpatialPackageMapClient* PackageMap)
{
	UObject* Container = Actor;
	switch (CmdIndex)
	{
		case 0:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<uint8>(Container);
			ReplicatedData->FieldBhidden = Value != 0;
			break;
		}
		case 1:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<uint8>(Container);
			ReplicatedData->FieldBreplicatemovement = Value != 0;
			break;
		}
		case 2:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<uint8>(Container);
			ReplicatedData->FieldBtearoff = Value != 0;
			break;
		}
		case 3:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<TEnumAsByte<ENetRole>>(Container);
			ReplicatedData->FieldRemoterole = int(Value);
			break;
		}
		case 4:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<AActor*>(Container);
			// WEAK OBJECT REPLICATION - ReplicatedData->FieldOwner = Value;
			break;
		}
		case 5:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<FRepMovement>(Container);
			TArray<uint8> Data;
			FMemoryWriter Writer(Data);
			bool Success;
			Value.NetSerialize(Writer, nullptr, Success);
			ReplicatedData->FieldReplicatedmovement = FBase64::Encode(Data);
			break;
		}
		case 6:
		{
			auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepAttachment>(Container);
			// WEAK OBJECT REPLICATION - ReplicatedData->FieldAttachmentreplicationAttachparent = Value.AttachParent;
			break;
		}
		case 7:
		{
			auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepAttachment>(Container);
			ReplicatedData->FieldAttachmentreplicationLocationoffset = Value.LocationOffset;
			break;
		}
		case 8:
		{
			auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepAttachment>(Container);
			ReplicatedData->FieldAttachmentreplicationRelativescale3d = Value.RelativeScale3D;
			break;
		}
		case 9:
		{
			auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepAttachment>(Container);
			auto& Rotator = ReplicatedData->FieldAttachmentreplicationRotationoffset;
			Rotator->SetPitch(Value.RotationOffset.Pitch);
			Rotator->SetYaw(Value.RotationOffset.Yaw);
			Rotator->SetRoll(Value.RotationOffset.Roll);
			break;
		}
		case 10:
		{
			auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepAttachment>(Container);
			ReplicatedData->FieldAttachmentreplicationAttachsocket = Value.AttachSocket.ToString();
			break;
		}
		case 11:
		{
			auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepAttachment>(Container);
			// WEAK OBJECT REPLICATION - ReplicatedData->FieldAttachmentreplicationAttachcomponent = Value.AttachComponent;
			break;
		}
		case 12:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<TEnumAsByte<ENetRole>>(Container);
			ReplicatedData->FieldRole = int(Value);
			break;
		}
		case 13:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<uint8>(Container);
			ReplicatedData->FieldBcanbedamaged = Value != 0;
			break;
		}
		case 14:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<APawn*>(Container);
			// WEAK OBJECT REPLICATION - ReplicatedData->FieldInstigator = Value;
			break;
		}
		case 15:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<APlayerState*>(Container);
			// WEAK OBJECT REPLICATION - ReplicatedData->FieldPlayerstate = Value;
			break;
		}
		case 16:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<uint8>(Container);
			ReplicatedData->FieldRemoteviewpitch = int(Value);
			break;
		}
		case 17:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<AController*>(Container);
			//auto UObjectRef = NewObject<UUnrealObjectRef>();
			//UObjectRef->SetEntity(FEntityId((int64(PackageMap->GetNetGUIDFromObject(Value).Value))));// Value
			//ReplicatedData->FieldController = UObjectRef;
			break;
		}
		case 18:
		{
			auto& Value = *ParentProperty->ContainerPtrToValuePtr<FBasedMovementInfo>(Container);
			auto UObjectRef = NewObject<UUnrealObjectRef>();
			UObjectRef->SetEntity(FEntityId((int64(PackageMap->GetNetGUIDFromObject(Value.MovementBase).Value))));// Value
			ReplicatedData->FieldReplicatedbasedmovementMovementbase = UObjectRef;
			break;
		}
		case 19:
		{
			auto& Value = *ParentProperty->ContainerPtrToValuePtr<FBasedMovementInfo>(Container);
			ReplicatedData->FieldReplicatedbasedmovementBonename = Value.BoneName.ToString();
			break;
		}
		case 20:
		{
			auto& Value = *ParentProperty->ContainerPtrToValuePtr<FBasedMovementInfo>(Container);
			ReplicatedData->FieldReplicatedbasedmovementLocation = Value.Location;
			break;
		}
		case 21:
		{
			auto& Value = *ParentProperty->ContainerPtrToValuePtr<FBasedMovementInfo>(Container);
			auto& Rotator = ReplicatedData->FieldReplicatedbasedmovementRotation;
			Rotator->SetPitch(Value.Rotation.Pitch);
			Rotator->SetYaw(Value.Rotation.Yaw);
			Rotator->SetRoll(Value.Rotation.Roll);
			break;
		}
		case 22:
		{
			auto& Value = *ParentProperty->ContainerPtrToValuePtr<FBasedMovementInfo>(Container);
			ReplicatedData->FieldReplicatedbasedmovementBserverhasbasecomponent = Value.bServerHasBaseComponent != 0;
			break;
		}
		case 23:
		{
			auto& Value = *ParentProperty->ContainerPtrToValuePtr<FBasedMovementInfo>(Container);
			ReplicatedData->FieldReplicatedbasedmovementBrelativerotation = Value.bRelativeRotation != 0;
			break;
		}
		case 24:
		{
			auto& Value = *ParentProperty->ContainerPtrToValuePtr<FBasedMovementInfo>(Container);
			ReplicatedData->FieldReplicatedbasedmovementBserverhasvelocity = Value.bServerHasVelocity != 0;
			break;
		}
		case 25:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<float>(Container);
			ReplicatedData->FieldAnimrootmotiontranslationscale = Value;
			break;
		}
		case 26:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<float>(Container);
			ReplicatedData->FieldReplicatedserverlasttransformupdatetimestamp = Value;
			break;
		}
		case 27:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<uint8>(Container);
			ReplicatedData->FieldReplicatedmovementmode = int(Value);
			break;
		}
		case 28:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<uint8>(Container);
			ReplicatedData->FieldBiscrouched = Value != 0;
			break;
		}
		case 29:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<float>(Container);
			ReplicatedData->FieldJumpmaxholdtime = Value;
			break;
		}
		case 30:
		{
			auto& Value = *Property->ContainerPtrToValuePtr<int32>(Container);
			ReplicatedData->FieldJumpmaxcount = Value;
			break;
		}
		case 31:
		{
			auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
			ReplicatedData->FieldReprootmotionBisactive = Value.bIsActive != 0;
			break;
		}
		case 32:
		{
			auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
			// WEAK OBJECT REPLICATION - ReplicatedData->FieldReprootmotionAnimmontage = Value.AnimMontage;
			break;
		}
		case 33:
		{
			auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
			ReplicatedData->FieldReprootmotionPosition = Value.Position;
			break;
		}
		case 34:
		{
			auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
			ReplicatedData->FieldReprootmotionLocation = Value.Location;
			break;
		}
		case 35:
		{
			auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
			auto& Rotator = ReplicatedData->FieldReprootmotionRotation;
			Rotator->SetPitch(Value.Rotation.Pitch);
			Rotator->SetYaw(Value.Rotation.Yaw);
			Rotator->SetRoll(Value.Rotation.Roll);
			break;
		}
		case 36:
		{
			auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
			// WEAK OBJECT REPLICATION - ReplicatedData->FieldReprootmotionMovementbase = Value.MovementBase;
			break;
		}
		case 37:
		{
			auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
			ReplicatedData->FieldReprootmotionMovementbasebonename = Value.MovementBaseBoneName.ToString();
			break;
		}
		case 38:
		{
			auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
			ReplicatedData->FieldReprootmotionBrelativeposition = Value.bRelativePosition != 0;
			break;
		}
		case 39:
		{
			auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
			ReplicatedData->FieldReprootmotionBrelativerotation = Value.bRelativeRotation != 0;
			break;
		}
		case 40:
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
		case 41:
		{
			auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
			ReplicatedData->FieldReprootmotionAcceleration = Value.Acceleration;
			break;
		}
		case 42:
		{
			auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
			ReplicatedData->FieldReprootmotionLinearvelocity = Value.LinearVelocity;
			break;
		}
	}
}
void ReceiveUpdateFromSpatial_Character(AActor* Actor, TMap<int, TPair<UProperty*, UProperty*>>& CmdIndexToPropertyMap, UUnrealACharacterReplicatedDataComponentUpdate* Update)
{
	UObject* Container = Actor;
	if (Update->HasFieldBhidden())
	{
		UProperty* ParentProperty = CmdIndexToPropertyMap[0].Key;
		UProperty* Property = CmdIndexToPropertyMap[0].Value;
		auto& Value = *Property->ContainerPtrToValuePtr<uint8>(Container);
	}
	if (Update->HasFieldBreplicatemovement())
	{
		UProperty* ParentProperty = CmdIndexToPropertyMap[1].Key;
		UProperty* Property = CmdIndexToPropertyMap[1].Value;
		auto& Value = *Property->ContainerPtrToValuePtr<uint8>(Container);
	}
	if (Update->HasFieldBtearoff())
	{
		UProperty* ParentProperty = CmdIndexToPropertyMap[2].Key;
		UProperty* Property = CmdIndexToPropertyMap[2].Value;
		auto& Value = *Property->ContainerPtrToValuePtr<uint8>(Container);
	}
	if (Update->HasFieldRemoterole())
	{
		UProperty* ParentProperty = CmdIndexToPropertyMap[3].Key;
		UProperty* Property = CmdIndexToPropertyMap[3].Value;
		auto& Value = *Property->ContainerPtrToValuePtr<TEnumAsByte<ENetRole>>(Container);
	}
	if (Update->HasFieldOwner())
	{
		UProperty* ParentProperty = CmdIndexToPropertyMap[4].Key;
		UProperty* Property = CmdIndexToPropertyMap[4].Value;
		auto& Value = *Property->ContainerPtrToValuePtr<AActor*>(Container);
	}
	if (Update->HasFieldReplicatedmovement())
	{
		UProperty* ParentProperty = CmdIndexToPropertyMap[5].Key;
		UProperty* Property = CmdIndexToPropertyMap[5].Value;
		auto& Value = *Property->ContainerPtrToValuePtr<FRepMovement>(Container);
	}
	if (Update->HasFieldAttachmentreplicationAttachparent())
	{
		UProperty* ParentProperty = CmdIndexToPropertyMap[6].Key;
		UProperty* Property = CmdIndexToPropertyMap[6].Value;
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepAttachment>(Container);
	}
	if (Update->HasFieldAttachmentreplicationLocationoffset())
	{
		UProperty* ParentProperty = CmdIndexToPropertyMap[7].Key;
		UProperty* Property = CmdIndexToPropertyMap[7].Value;
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepAttachment>(Container);
	}
	if (Update->HasFieldAttachmentreplicationRelativescale3d())
	{
		UProperty* ParentProperty = CmdIndexToPropertyMap[8].Key;
		UProperty* Property = CmdIndexToPropertyMap[8].Value;
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepAttachment>(Container);
	}
	if (Update->HasFieldAttachmentreplicationRotationoffset())
	{
		UProperty* ParentProperty = CmdIndexToPropertyMap[9].Key;
		UProperty* Property = CmdIndexToPropertyMap[9].Value;
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepAttachment>(Container);
	}
	if (Update->HasFieldAttachmentreplicationAttachsocket())
	{
		UProperty* ParentProperty = CmdIndexToPropertyMap[10].Key;
		UProperty* Property = CmdIndexToPropertyMap[10].Value;
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepAttachment>(Container);
	}
	if (Update->HasFieldAttachmentreplicationAttachcomponent())
	{
		UProperty* ParentProperty = CmdIndexToPropertyMap[11].Key;
		UProperty* Property = CmdIndexToPropertyMap[11].Value;
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepAttachment>(Container);
	}
	if (Update->HasFieldRole())
	{
		UProperty* ParentProperty = CmdIndexToPropertyMap[12].Key;
		UProperty* Property = CmdIndexToPropertyMap[12].Value;
		auto& Value = *Property->ContainerPtrToValuePtr<TEnumAsByte<ENetRole>>(Container);
	}
	if (Update->HasFieldBcanbedamaged())
	{
		UProperty* ParentProperty = CmdIndexToPropertyMap[13].Key;
		UProperty* Property = CmdIndexToPropertyMap[13].Value;
		auto& Value = *Property->ContainerPtrToValuePtr<uint8>(Container);
	}
	if (Update->HasFieldInstigator())
	{
		UProperty* ParentProperty = CmdIndexToPropertyMap[14].Key;
		UProperty* Property = CmdIndexToPropertyMap[14].Value;
		auto& Value = *Property->ContainerPtrToValuePtr<APawn*>(Container);
	}
	if (Update->HasFieldPlayerstate())
	{
		UProperty* ParentProperty = CmdIndexToPropertyMap[15].Key;
		UProperty* Property = CmdIndexToPropertyMap[15].Value;
		auto& Value = *Property->ContainerPtrToValuePtr<APlayerState*>(Container);
	}
	if (Update->HasFieldRemoteviewpitch())
	{
		UProperty* ParentProperty = CmdIndexToPropertyMap[16].Key;
		UProperty* Property = CmdIndexToPropertyMap[16].Value;
		auto& Value = *Property->ContainerPtrToValuePtr<uint8>(Container);
	}
	if (Update->HasFieldController())
	{
		UProperty* ParentProperty = CmdIndexToPropertyMap[17].Key;
		UProperty* Property = CmdIndexToPropertyMap[17].Value;
		auto& Value = *Property->ContainerPtrToValuePtr<AController*>(Container);
	}
	if (Update->HasFieldReplicatedbasedmovementMovementbase())
	{
		UProperty* ParentProperty = CmdIndexToPropertyMap[18].Key;
		UProperty* Property = CmdIndexToPropertyMap[18].Value;
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FBasedMovementInfo>(Container);
	}
	if (Update->HasFieldReplicatedbasedmovementBonename())
	{
		UProperty* ParentProperty = CmdIndexToPropertyMap[19].Key;
		UProperty* Property = CmdIndexToPropertyMap[19].Value;
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FBasedMovementInfo>(Container);
	}
	if (Update->HasFieldReplicatedbasedmovementLocation())
	{
		UProperty* ParentProperty = CmdIndexToPropertyMap[20].Key;
		UProperty* Property = CmdIndexToPropertyMap[20].Value;
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FBasedMovementInfo>(Container);
	}
	if (Update->HasFieldReplicatedbasedmovementRotation())
	{
		UProperty* ParentProperty = CmdIndexToPropertyMap[21].Key;
		UProperty* Property = CmdIndexToPropertyMap[21].Value;
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FBasedMovementInfo>(Container);
	}
	if (Update->HasFieldReplicatedbasedmovementBserverhasbasecomponent())
	{
		UProperty* ParentProperty = CmdIndexToPropertyMap[22].Key;
		UProperty* Property = CmdIndexToPropertyMap[22].Value;
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FBasedMovementInfo>(Container);
	}
	if (Update->HasFieldReplicatedbasedmovementBrelativerotation())
	{
		UProperty* ParentProperty = CmdIndexToPropertyMap[23].Key;
		UProperty* Property = CmdIndexToPropertyMap[23].Value;
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FBasedMovementInfo>(Container);
	}
	if (Update->HasFieldReplicatedbasedmovementBserverhasvelocity())
	{
		UProperty* ParentProperty = CmdIndexToPropertyMap[24].Key;
		UProperty* Property = CmdIndexToPropertyMap[24].Value;
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FBasedMovementInfo>(Container);
	}
	if (Update->HasFieldAnimrootmotiontranslationscale())
	{
		UProperty* ParentProperty = CmdIndexToPropertyMap[25].Key;
		UProperty* Property = CmdIndexToPropertyMap[25].Value;
		auto& Value = *Property->ContainerPtrToValuePtr<float>(Container);
	}
	if (Update->HasFieldReplicatedserverlasttransformupdatetimestamp())
	{
		UProperty* ParentProperty = CmdIndexToPropertyMap[26].Key;
		UProperty* Property = CmdIndexToPropertyMap[26].Value;
		auto& Value = *Property->ContainerPtrToValuePtr<float>(Container);
	}
	if (Update->HasFieldReplicatedmovementmode())
	{
		UProperty* ParentProperty = CmdIndexToPropertyMap[27].Key;
		UProperty* Property = CmdIndexToPropertyMap[27].Value;
		auto& Value = *Property->ContainerPtrToValuePtr<uint8>(Container);
	}
	if (Update->HasFieldBiscrouched())
	{
		UProperty* ParentProperty = CmdIndexToPropertyMap[28].Key;
		UProperty* Property = CmdIndexToPropertyMap[28].Value;
		auto& Value = *Property->ContainerPtrToValuePtr<uint8>(Container);
	}
	if (Update->HasFieldJumpmaxholdtime())
	{
		UProperty* ParentProperty = CmdIndexToPropertyMap[29].Key;
		UProperty* Property = CmdIndexToPropertyMap[29].Value;
		auto& Value = *Property->ContainerPtrToValuePtr<float>(Container);
	}
	if (Update->HasFieldJumpmaxcount())
	{
		UProperty* ParentProperty = CmdIndexToPropertyMap[30].Key;
		UProperty* Property = CmdIndexToPropertyMap[30].Value;
		auto& Value = *Property->ContainerPtrToValuePtr<int32>(Container);
	}
	if (Update->HasFieldReprootmotionBisactive())
	{
		UProperty* ParentProperty = CmdIndexToPropertyMap[31].Key;
		UProperty* Property = CmdIndexToPropertyMap[31].Value;
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
	}
	if (Update->HasFieldReprootmotionAnimmontage())
	{
		UProperty* ParentProperty = CmdIndexToPropertyMap[32].Key;
		UProperty* Property = CmdIndexToPropertyMap[32].Value;
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
	}
	if (Update->HasFieldReprootmotionPosition())
	{
		UProperty* ParentProperty = CmdIndexToPropertyMap[33].Key;
		UProperty* Property = CmdIndexToPropertyMap[33].Value;
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
	}
	if (Update->HasFieldReprootmotionLocation())
	{
		UProperty* ParentProperty = CmdIndexToPropertyMap[34].Key;
		UProperty* Property = CmdIndexToPropertyMap[34].Value;
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
	}
	if (Update->HasFieldReprootmotionRotation())
	{
		UProperty* ParentProperty = CmdIndexToPropertyMap[35].Key;
		UProperty* Property = CmdIndexToPropertyMap[35].Value;
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
	}
	if (Update->HasFieldReprootmotionMovementbase())
	{
		UProperty* ParentProperty = CmdIndexToPropertyMap[36].Key;
		UProperty* Property = CmdIndexToPropertyMap[36].Value;
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
	}
	if (Update->HasFieldReprootmotionMovementbasebonename())
	{
		UProperty* ParentProperty = CmdIndexToPropertyMap[37].Key;
		UProperty* Property = CmdIndexToPropertyMap[37].Value;
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
	}
	if (Update->HasFieldReprootmotionBrelativeposition())
	{
		UProperty* ParentProperty = CmdIndexToPropertyMap[38].Key;
		UProperty* Property = CmdIndexToPropertyMap[38].Value;
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
	}
	if (Update->HasFieldReprootmotionBrelativerotation())
	{
		UProperty* ParentProperty = CmdIndexToPropertyMap[39].Key;
		UProperty* Property = CmdIndexToPropertyMap[39].Value;
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
	}
	if (Update->HasFieldReprootmotionAuthoritativerootmotion())
	{
		UProperty* ParentProperty = CmdIndexToPropertyMap[40].Key;
		UProperty* Property = CmdIndexToPropertyMap[40].Value;
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
	}
	if (Update->HasFieldReprootmotionAcceleration())
	{
		UProperty* ParentProperty = CmdIndexToPropertyMap[41].Key;
		UProperty* Property = CmdIndexToPropertyMap[41].Value;
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
	}
	if (Update->HasFieldReprootmotionLinearvelocity())
	{
		UProperty* ParentProperty = CmdIndexToPropertyMap[42].Key;
		UProperty* Property = CmdIndexToPropertyMap[42].Value;
		auto& Value = *ParentProperty->ContainerPtrToValuePtr<FRepRootMotionMontage>(Container);
	}
}
