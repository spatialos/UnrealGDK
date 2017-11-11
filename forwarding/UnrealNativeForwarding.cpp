{
  switch (cmdIndex) {
    case 0:
      {
        auto& Value = *Property->ContainerPtrToValuePtr<uint8>(this);
        NativeComponent->FieldBhidden = Value != 0;
        break;
      }
    case 1:
      {
        auto& Value = *Property->ContainerPtrToValuePtr<uint8>(this);
        NativeComponent->FieldBreplicatemovement = Value != 0;
        break;
      }
    case 2:
      {
        auto& Value = *Property->ContainerPtrToValuePtr<uint8>(this);
        NativeComponent->FieldBtearoff = Value != 0;
        break;
      }
    case 3:
      {
        auto& Value = *Property->ContainerPtrToValuePtr<TEnumAsByte<ENetRole>>(this);
        NativeComponent->FieldRemoterole = int(Value);
        break;
      }
    case 4:
      {
        auto& Value = *Property->ContainerPtrToValuePtr<AActor*>(this);
        // WEAK OBJECT REPLICATION - NativeComponent->FieldOwner = Value;
        break;
      }
    case 5:
      {
        auto& Value = *Property->ContainerPtrToValuePtr<FRepMovement>(this);
        TArray<uint8> Data;
        FMemoryWriter Writer(Data);
        bool Success;
        Value.NetSerialize(Writer, nullptr, Success);
        NativeComponent->FieldReplicatedmovement = FBase64::Encode(Data);
        break;
      }
    case 6:
      {
        auto& Value = *Property->ContainerPtrToValuePtr<AActor*>(this);
        // WEAK OBJECT REPLICATION - NativeComponent->FieldAttachmentreplicationAttachparent = Value;
        break;
      }
    case 7:
      {
        auto& Value = *Property->ContainerPtrToValuePtr<FVector_NetQuantize100>(this);
        NativeComponent->FieldAttachmentreplicationLocationoffset = Value;
        break;
      }
    case 8:
      {
        auto& Value = *Property->ContainerPtrToValuePtr<FVector_NetQuantize100>(this);
        NativeComponent->FieldAttachmentreplicationRelativescale3d = Value;
        break;
      }
    case 9:
      {
        auto& Value = *Property->ContainerPtrToValuePtr<FRotator>(this);
        auto& Rotator = NativeComponent->FieldAttachmentreplicationRotationoffset;
        Rotator->SetPitch(Value.Pitch);
        Rotator->SetYaw(Value.Yaw);
        Rotator->SetRoll(Value.Roll);
        break;
      }
    case 10:
      {
        auto& Value = *Property->ContainerPtrToValuePtr<FName>(this);
        NativeComponent->FieldAttachmentreplicationAttachsocket = Value.ToString();
        break;
      }
    case 11:
      {
        auto& Value = *Property->ContainerPtrToValuePtr<USceneComponent*>(this);
        // WEAK OBJECT REPLICATION - NativeComponent->FieldAttachmentreplicationAttachcomponent = Value;
        break;
      }
    case 12:
      {
        auto& Value = *Property->ContainerPtrToValuePtr<TEnumAsByte<ENetRole>>(this);
        NativeComponent->FieldRole = int(Value);
        break;
      }
    case 13:
      {
        auto& Value = *Property->ContainerPtrToValuePtr<uint8>(this);
        NativeComponent->FieldBcanbedamaged = Value != 0;
        break;
      }
    case 14:
      {
        auto& Value = *Property->ContainerPtrToValuePtr<APawn*>(this);
        // WEAK OBJECT REPLICATION - NativeComponent->FieldInstigator = Value;
        break;
      }
    case 15:
      {
        auto& Value = *Property->ContainerPtrToValuePtr<APlayerState*>(this);
        // WEAK OBJECT REPLICATION - NativeComponent->FieldPlayerstate = Value;
        break;
      }
    case 16:
      {
        auto& Value = *Property->ContainerPtrToValuePtr<uint8>(this);
        NativeComponent->FieldRemoteviewpitch = int(Value);
        break;
      }
    case 17:
      {
        auto& Value = *Property->ContainerPtrToValuePtr<AController*>(this);
        // WEAK OBJECT REPLICATION - NativeComponent->FieldController = Value;
        break;
      }
    case 18:
      {
        auto& Value = *Property->ContainerPtrToValuePtr<UPrimitiveComponent*>(this);
        // WEAK OBJECT REPLICATION - NativeComponent->FieldReplicatedbasedmovementMovementbase = Value;
        break;
      }
    case 19:
      {
        auto& Value = *Property->ContainerPtrToValuePtr<FName>(this);
        NativeComponent->FieldReplicatedbasedmovementBonename = Value.ToString();
        break;
      }
    case 20:
      {
        auto& Value = *Property->ContainerPtrToValuePtr<FVector_NetQuantize100>(this);
        NativeComponent->FieldReplicatedbasedmovementLocation = Value;
        break;
      }
    case 21:
      {
        auto& Value = *Property->ContainerPtrToValuePtr<FRotator>(this);
        auto& Rotator = NativeComponent->FieldReplicatedbasedmovementRotation;
        Rotator->SetPitch(Value.Pitch);
        Rotator->SetYaw(Value.Yaw);
        Rotator->SetRoll(Value.Roll);
        break;
      }
    case 22:
      {
        auto& Value = *Property->ContainerPtrToValuePtr<bool>(this);
        NativeComponent->FieldReplicatedbasedmovementBserverhasbasecomponent = Value != 0;
        break;
      }
    case 23:
      {
        auto& Value = *Property->ContainerPtrToValuePtr<bool>(this);
        NativeComponent->FieldReplicatedbasedmovementBrelativerotation = Value != 0;
        break;
      }
    case 24:
      {
        auto& Value = *Property->ContainerPtrToValuePtr<bool>(this);
        NativeComponent->FieldReplicatedbasedmovementBserverhasvelocity = Value != 0;
        break;
      }
    case 25:
      {
        auto& Value = *Property->ContainerPtrToValuePtr<float>(this);
        NativeComponent->FieldAnimrootmotiontranslationscale = Value;
        break;
      }
    case 26:
      {
        auto& Value = *Property->ContainerPtrToValuePtr<float>(this);
        NativeComponent->FieldReplicatedserverlasttransformupdatetimestamp = Value;
        break;
      }
    case 27:
      {
        auto& Value = *Property->ContainerPtrToValuePtr<uint8>(this);
        NativeComponent->FieldReplicatedmovementmode = int(Value);
        break;
      }
    case 28:
      {
        auto& Value = *Property->ContainerPtrToValuePtr<uint8>(this);
        NativeComponent->FieldBiscrouched = Value != 0;
        break;
      }
    case 29:
      {
        auto& Value = *Property->ContainerPtrToValuePtr<float>(this);
        NativeComponent->FieldJumpmaxholdtime = Value;
        break;
      }
    case 30:
      {
        auto& Value = *Property->ContainerPtrToValuePtr<int32>(this);
        NativeComponent->FieldJumpmaxcount = Value;
        break;
      }
    case 31:
      {
        auto& Value = *Property->ContainerPtrToValuePtr<bool>(this);
        NativeComponent->FieldReprootmotionBisactive = Value != 0;
        break;
      }
    case 32:
      {
        auto& Value = *Property->ContainerPtrToValuePtr<UAnimMontage*>(this);
        // WEAK OBJECT REPLICATION - NativeComponent->FieldReprootmotionAnimmontage = Value;
        break;
      }
    case 33:
      {
        auto& Value = *Property->ContainerPtrToValuePtr<float>(this);
        NativeComponent->FieldReprootmotionPosition = Value;
        break;
      }
    case 34:
      {
        auto& Value = *Property->ContainerPtrToValuePtr<FVector_NetQuantize100>(this);
        NativeComponent->FieldReprootmotionLocation = Value;
        break;
      }
    case 35:
      {
        auto& Value = *Property->ContainerPtrToValuePtr<FRotator>(this);
        auto& Rotator = NativeComponent->FieldReprootmotionRotation;
        Rotator->SetPitch(Value.Pitch);
        Rotator->SetYaw(Value.Yaw);
        Rotator->SetRoll(Value.Roll);
        break;
      }
    case 36:
      {
        auto& Value = *Property->ContainerPtrToValuePtr<UPrimitiveComponent*>(this);
        // WEAK OBJECT REPLICATION - NativeComponent->FieldReprootmotionMovementbase = Value;
        break;
      }
    case 37:
      {
        auto& Value = *Property->ContainerPtrToValuePtr<FName>(this);
        NativeComponent->FieldReprootmotionMovementbasebonename = Value.ToString();
        break;
      }
    case 38:
      {
        auto& Value = *Property->ContainerPtrToValuePtr<bool>(this);
        NativeComponent->FieldReprootmotionBrelativeposition = Value != 0;
        break;
      }
    case 39:
      {
        auto& Value = *Property->ContainerPtrToValuePtr<bool>(this);
        NativeComponent->FieldReprootmotionBrelativerotation = Value != 0;
        break;
      }
    case 40:
      {
        auto& Value = *Property->ContainerPtrToValuePtr<FRootMotionSourceGroup>(this);
        {
          NativeComponent->FieldReprootmotionAuthoritativerootmotionBhasadditivesources = Value.bHasAdditiveSources != 0;
        }
        {
          NativeComponent->FieldReprootmotionAuthoritativerootmotionBhasoverridesources = Value.bHasOverrideSources != 0;
        }
        {
          NativeComponent->FieldReprootmotionAuthoritativerootmotionLastpreadditivevelocity = Value.LastPreAdditiveVelocity;
        }
        {
          NativeComponent->FieldReprootmotionAuthoritativerootmotionBisadditivevelocityapplied = Value.bIsAdditiveVelocityApplied != 0;
        }
        {
          {
            NativeComponent->FieldReprootmotionAuthoritativerootmotionLastaccumulatedsettingsFlags = int(Value.LastAccumulatedSettings.Flags);
          }
        }
        break;
      }
    case 41:
      {
        auto& Value = *Property->ContainerPtrToValuePtr<FVector_NetQuantize10>(this);
        NativeComponent->FieldReprootmotionAcceleration = Value;
        break;
      }
    case 42:
      {
        auto& Value = *Property->ContainerPtrToValuePtr<FVector_NetQuantize10>(this);
        NativeComponent->FieldReprootmotionLinearvelocity = Value;
        break;
      }
  }
}
