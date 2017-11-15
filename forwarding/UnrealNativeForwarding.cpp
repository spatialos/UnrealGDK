void ApplyUpdateToSpatial_Character(AActor* Actor, int CmdIndex, UProperty* ParentProperty, UProperty* Property, UUnrealACharacterReplicatedDataComponent* ReplicatedData)
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
        // WEAK OBJECT REPLICATION - ReplicatedData->FieldController = Value;
        break;
      }
    case 18:
      {
        auto& Value = *ParentProperty->ContainerPtrToValuePtr<FBasedMovementInfo>(Container);
        // WEAK OBJECT REPLICATION - ReplicatedData->FieldReplicatedbasedmovementMovementbase = Value.MovementBase;
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
