// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialActorChannel.h"
#include "SpatialNetDriver.h"
#include "SpatialPackageMapClient.h"
#include "EntityRegistry.h"
#include "Net/DataBunch.h"
#include <improbable/worker.h>
#include <improbable/standard_library.h>
#include <improbable/player/player.h>
#include "Commander.h"
#include "EntityBuilder.h"
#include "EntityTemplate.h"
#include "SpatialNetConnection.h"
#include "SpatialOS.h"
#include "SpatialUpdateInterop.h"

#include "Utils/BunchReader.h"
#include "Generated/SpatialUpdateInterop_Character.h"

// TODO(David): Required until we sever the unreal connection.
#include "Generated/SpatialUpdateInterop_Character.h"

using namespace improbable;

USpatialActorChannel::USpatialActorChannel(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{
	ChType = CHTYPE_Actor;
	bCoreActor = true;
	bSendingInitialBunch = false;
	ActorEntityId = worker::EntityId{};
}

void USpatialActorChannel::Init(UNetConnection* connection, int32 channelIndex, bool bOpenedLocally)
{
	Super::Init(connection, channelIndex, bOpenedLocally);

	USpatialNetDriver* Driver = Cast<USpatialNetDriver>(connection->Driver);
	WorkerView = Driver->GetSpatialOS()->GetView();
	WorkerConnection = Driver->GetSpatialOS()->GetConnection();

	Callbacks.Reset(new unreal::callbacks::FScopedViewCallbacks(WorkerView));

	TSharedPtr<worker::View> PinnedView = WorkerView.Pin();
	if (PinnedView.IsValid())
	{
		Callbacks->Add(PinnedView->OnReserveEntityIdResponse(std::bind(&USpatialActorChannel::OnReserveEntityIdResponse, this, std::placeholders::_1)));
		Callbacks->Add(PinnedView->OnCreateEntityResponse(std::bind(&USpatialActorChannel::OnCreateEntityResponse, this, std::placeholders::_1)));
	}
}

void USpatialActorChannel::SetClosingFlag()
{
	Super::SetClosingFlag();
}

void USpatialActorChannel::Close()
{
	Super::Close();
}

void USpatialActorChannel::ReceivedBunch(FInBunch &Bunch)
{
	// If not a client, just call normal behaviour.
	if (!Connection->Driver->ServerConnection)
	{
		UActorChannel::ReceivedBunch(Bunch);
		return;
	}

	// Parse the bunch, and let through all non-replicated stuff, and replicated stuff that matches a few properties.
	// TODO(david): Remove this when we sever the Unreal connection.
	auto& PropertyMap = GetHandlePropertyMap_Character();
	FBunchReader BunchReader(&Bunch);
	FBunchReader::RepDataHandler RepDataHandler = [](FNetBitReader& Reader, UPackageMap* PackageMap, int32 Handle, UProperty* Property) -> bool
	{
		// TODO: We can't parse UObjects or FNames here as we have no package map.
		if (Property->IsA(UObjectPropertyBase::StaticClass()) || Property->IsA(UNameProperty::StaticClass()))
		{
			UE_LOG(LogTemp, Warning, TEXT("<- Allowing through UObject/FName update."));
			return false;
		}

		// Skip bytes.
		// Property data size: RepLayout.cpp:237
		TArray<uint8> PropertyData;
		PropertyData.AddZeroed(Property->ElementSize);
		Property->InitializeValue(PropertyData.GetData());
		Property->NetSerializeItem(Reader, PackageMap, PropertyData.GetData());
		return true;
	};
	BunchReader.Parse(Connection->Driver->IsServer(), PropertyMap, RepDataHandler);
	if (BunchReader.HasError())
	{
		//UE_LOG(LogTemp, Warning, TEXT("<- Allowing through non actor bunch."));
		UActorChannel::ReceivedBunch(Bunch);
	}
	else if (!BunchReader.HasRepLayout() || !BunchReader.IsActor())
	{
		//UE_LOG(LogTemp, Warning, TEXT("<- Allowing through non replicated bunch."));
		UActorChannel::ReceivedBunch(Bunch);
	}
}

void USpatialActorChannel::ReceivedNak(int32 PacketId)
{
	Super::ReceivedNak(PacketId);
}

void USpatialActorChannel::Tick()
{
	Super::Tick();
}

bool USpatialActorChannel::CanStopTicking() const
{
	return Super::CanStopTicking();
}

void USpatialActorChannel::AppendExportBunches(TArray<FOutBunch *> & outExportBunches)
{
	Super::AppendExportBunches(outExportBunches);
}

void USpatialActorChannel::AppendMustBeMappedGuids(FOutBunch * bunch)
{
	Super::AppendMustBeMappedGuids(bunch);
}

FPacketIdRange USpatialActorChannel::SendBunch(FOutBunch * BunchPtr, bool bMerge)
{
	// Run default actor channel code.
	// TODO(David) I believe that ReturnValue will always contain a PacketId which has the same First and Last value
	// if we don't break up bunches, which in our case we wont as there's no need to at this layer.
	auto ReturnValue = UActorChannel::SendBunch(BunchPtr, bMerge);

	// Pass bunch to update interop.
	USpatialUpdateInterop* UpdateInterop = Cast<USpatialNetDriver>(Connection->Driver)->GetSpatialUpdateInterop();
	if (!UpdateInterop)
	{
		UE_LOG(LogTemp, Error, TEXT("Update Interop object not set up. This should never happen"));
	}
	UpdateInterop->SendSpatialUpdate(this, BunchPtr);
	return ReturnValue;
}

void USpatialActorChannel::StartBecomingDormant()
{
	UActorChannel::StartBecomingDormant();
}

void USpatialActorChannel::BecomeDormant()
{
	UActorChannel::BecomeDormant();
}

bool USpatialActorChannel::CleanUp(const bool bForDestroy)
{
	USpatialNetConnection* SpatialConnection = Cast<USpatialNetConnection>(Connection);
	if (SpatialConnection && SpatialConnection->Driver->IsServer()
		&& SpatialConnection->bFakeSpatialClient)
	{
		TSharedPtr<worker::Connection> PinnedConnection = WorkerConnection.Pin();
		if (PinnedConnection.IsValid())
		{
			PinnedConnection->SendDeleteEntityRequest(ActorEntityId, 0);
		}
	}
	return UActorChannel::CleanUp(bForDestroy);
}

bool USpatialActorChannel::ReplicateActor()
{
// filter everything else temporarily to make it easier to debug for now
	if (!Actor->IsA(APlayerController::StaticClass()))
		return false;
	if ((OpenPacketId.First == INDEX_NONE || Connection->bResendAllDataSinceOpen) && OpenedLocally)
	{
		bSendingInitialBunch = true;
	}
	
	bool bResult = Super::ReplicateActor();
	bSendingInitialBunch = false;
	
	return bResult;
}

void USpatialActorChannel::SetChannelActor(AActor* InActor)
{
	Super::SetChannelActor(InActor);

	if (!bCoreActor)
		return;

	USpatialNetConnection* SpatialConnection = Cast<USpatialNetConnection>(Connection);

	if (SpatialConnection && SpatialConnection->Driver->IsServer()
		&& SpatialConnection->bFakeSpatialClient)
	{
		// Create a Spatial entity that corresponds to this actor.
		TSharedPtr<worker::Connection> PinnedConnection = WorkerConnection.Pin();
		if (PinnedConnection.IsValid())
		{
			ReserveEntityIdRequestId = PinnedConnection->SendReserveEntityIdRequest(0);
		}
	}
	else
	{
		USpatialNetDriver* Driver = Cast<USpatialNetDriver>(Connection->Driver);
		check(Driver);
		check(Driver->GetEntityRegistry());
		ActorEntityId = Driver->GetEntityRegistry()->GetEntityIdFromActor(InActor).ToSpatialEntityId();
	}
}

void USpatialActorChannel::OnReserveEntityIdResponse(const worker::ReserveEntityIdResponseOp& Op)
{
	// just filter incorrect callbacks for now
	if (Op.RequestId == ReserveEntityIdRequestId)
	{
		if (!(Op.StatusCode == worker::StatusCode::kSuccess))
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to reserve entity id"));
			return;
		}
		else
		{
			TSharedPtr<worker::Connection> PinnedConnection = WorkerConnection.Pin();
			if (PinnedConnection.IsValid())
			{
				USpatialNetDriver* Driver = Cast<USpatialNetDriver>(Connection->Driver);
				checkf(Driver->GetSpatialUpdateInterop(), TEXT("Spatial Update Interop is not initialised"));
				const FSpatialTypeBinding* Binding = Driver->GetSpatialUpdateInterop()->GetTypeBindingByClass(Actor->GetClass());

				FStringAssetReference ActorClassRef(Actor->GetClass());
				FString PathStr = ActorClassRef.ToString();

				UE_LOG(LogTemp, Log, TEXT("Creating entity for actor with path: %s on ActorChannel: %s"), *PathStr, *GetName());

				if (Binding)
				{
					auto Entity = Binding->CreateActorEntity(Actor->GetActorLocation(), PathStr);
					CreateEntityRequestId = PinnedConnection->SendCreateEntityRequest(Entity, Op.EntityId, 0);
				}
				else
				{
					WorkerAttributeSet UnrealWorkerAttributeSet{{worker::List<std::string>{"UnrealWorker"}}};
					WorkerAttributeSet UnrealClientAttributeSet{{worker::List<std::string>{"UnrealClient"}}};

					// UnrealWorker write authority, any worker read authority
					WorkerRequirementSet UnrealWorkerWritePermission{{UnrealWorkerAttributeSet}};
					WorkerRequirementSet UnrealClientWritePermission{{UnrealClientAttributeSet}};
					WorkerRequirementSet AnyWorkerReadRequirement{{UnrealWorkerAttributeSet, UnrealClientAttributeSet}};

					auto Entity = unreal::FEntityBuilder::Begin()
						.AddPositionComponent(USpatialOSConversionFunctionLibrary::UnrealCoordinatesToSpatialOsCoordinatesCast(Actor->GetActorLocation()), UnrealWorkerWritePermission)
						.AddMetadataComponent(Metadata::Data{TCHAR_TO_UTF8(*PathStr)})
						.SetPersistence(true)
						.SetReadAcl(AnyWorkerReadRequirement)
						.AddComponent<improbable::player::PlayerControlClient>(improbable::player::PlayerControlClientData{}, UnrealClientWritePermission)
						.Build();

					CreateEntityRequestId = PinnedConnection->SendCreateEntityRequest(Entity, Op.EntityId, 0);
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Failed to obtain reference to SpatialOS connection!"));
				return;
			}
		}
	}
}

void USpatialActorChannel::OnCreateEntityResponse(const worker::CreateEntityResponseOp& Op)
{
	if (Op.RequestId != CreateEntityRequestId)
	{
		return;
	}

	if (!(Op.StatusCode == worker::StatusCode::kSuccess))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to create entity!"));
		return;
	}	

	USpatialNetDriver* Driver = Cast<USpatialNetDriver>(Connection->Driver);
	if (Driver->ClientConnections.Num() > 0)
	{
		// should provide a better way of getting hold of the SpatialOS client connection 
		USpatialPackageMapClient* PMC = Cast<USpatialPackageMapClient>(Driver->ClientConnections[0]->PackageMap);
		if (PMC)
		{
			worker::EntityId SpatialEntityId = Op.EntityId.value_or(0);
			FEntityId EntityId(SpatialEntityId);

			UE_LOG(LogTemp, Warning, TEXT("Received create entity response op for %d"), EntityId.ToSpatialEntityId());
			// once we know the entity was successfully spawned, add the local actor 
			// to the package map and to the EntityRegistry
			PMC->ResolveEntityActor(GetActor(), EntityId);
			Driver->GetEntityRegistry()->AddToRegistry(EntityId, GetActor());
			ActorEntityId = SpatialEntityId;
		}
	}
}	

