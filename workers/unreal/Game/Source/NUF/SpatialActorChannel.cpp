// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialActorChannel.h"
#include "SpatialNetDriver.h"
#include "Engine/NetConnection.h"
#include "SpatialPackageMapClient.h"
#include "EntityRegistry.h"
#include "Net/DataBunch.h"
#include <improbable/worker.h>
#include <improbable/standard_library.h>
#include "Commander.h"
#include "EntityTemplate.h"
#include "EntityBuilder.h"
#include "Generated/SpatialShadowActor_Character.h"

using namespace improbable;

// We assume that #define ENABLE_PROPERTY_CHECKSUMS exists in RepLayout.cpp:88 here.
#define ENABLE_PROPERTY_CHECKSUMS

USpatialActorChannel::USpatialActorChannel(const FObjectInitializer & objectInitializer /*= FObjectInitializer::Get()*/)
	: Super(objectInitializer)
{
	ChannelClasses[CHTYPE_Actor] = GetClass();
	ChType = CHTYPE_Actor;
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

void USpatialActorChannel::ReceivedBunch(FInBunch & bunch)
{
	Super::ReceivedBunch(bunch);
}

void USpatialActorChannel::ReceivedNak(int32 packetId)
{
	Super::ReceivedNak(packetId);
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

// TODO: Epic should expose this.
struct FExportFlags
{
	union
	{
		struct
		{
			uint8 bHasPath : 1;
			uint8 bNoLoad : 1;
			uint8 bHasNetworkChecksum : 1;
		};

		uint8	Value;
	};

	FExportFlags()
	{
		Value = 0;
	}
};

static void ParseHeader_Subobject(FNetBitReader& Bunch)
{
	FNetworkGUID NetGUID;
	FExportFlags ExportFlags;
	Bunch << NetGUID;
	Bunch << ExportFlags.Value;
	if (ExportFlags.bHasPath)
	{
		ParseHeader_Subobject(Bunch);

		FString PathName;
		Bunch << PathName;
		uint32 NetworkChecksum;
		if (ExportFlags.bHasNetworkChecksum)
		{
			Bunch << NetworkChecksum;
		}
	}
}

static bool ParseHeader(FNetBitReader& Bunch, bool bIsServer, bool& bHasRepLayout, bool& bIsActor)
{
	bHasRepLayout = Bunch.ReadBit() != 0;
	bIsActor = Bunch.ReadBit() != 0;
	if (!bIsActor)
	{
		// For now, just give up.
		return false;

		ParseHeader_Subobject(Bunch);
		if (!bIsServer)
		{
			// bStablyNamed.
			Bunch.ReadBit();
		}
	}
	// If we have enough space to read another byte, then we should continue.
	return (Bunch.GetPosBits() + 8 <= Bunch.GetNumBits());
}

FPacketIdRange USpatialActorChannel::SendBunch(FOutBunch * BunchPtr, bool bMerge)
{
	FNetBitReader BunchReader(nullptr, BunchPtr->GetData(), BunchPtr->GetNumBits());

	// Run default actor channel code.
	auto ReturnValue = UActorChannel::SendBunch(BunchPtr, bMerge);

	// Read bunch.
	bool bHasRepLayout;
	bool bIsActor;
	if (!ParseHeader(BunchReader, Connection->Driver->IsServer(), bHasRepLayout, bIsActor))
	{
		return ReturnValue;
	}

	// Get payload data.
	uint32 PayloadBitCount;
	BunchReader.SerializeIntPacked(PayloadBitCount);
	FNetBitReader Reader;
	Reader.SetData(BunchReader, PayloadBitCount);

	if (!bHasRepLayout)
	{
		return ReturnValue;
	}

	UE_LOG(LogTemp, Log, TEXT("-> Header: HasRepLayout %d IsActor %d PayloadBitCount %d"), (int)bHasRepLayout, (int)bIsActor, (int)PayloadBitCount);

	// Get SpatialNetDriver.
	USpatialNetDriver* Driver = Cast<USpatialNetDriver>(Connection->Driver);
	if (!Driver->ShadowActorPipelineBlock)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpatialOS is not connected yet."));
		return ReturnValue;
	}

	// Get entity ID.
	// TODO: Replace with EntityId from registry corresponding to this replicated actor.
	FEntityId EntityId = 2;//EntityRegistry->GetEntityIdFromActor(ActorChannel->Actor);
	if (EntityId == FEntityId())
	{
		return ReturnValue;
	}

	// Get shadow actor.
	ASpatialShadowActor_Character* ShadowActor = Cast<ASpatialShadowActor_Character>(Driver->ShadowActorPipelineBlock->GetShadowActor(EntityId));
	if (!ShadowActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("Actor channel has no corresponding shadow actor. That means the entity hasn't been checked out yet."));
		return ReturnValue;
	}

	// Parse payload.
#ifdef ENABLE_PROPERTY_CHECKSUMS
	bool bDoChecksum = Reader.ReadBit() != 0;
#else
	bool bDoChecksum = false;
#endif
	while (Reader.GetBitsLeft() > 0)
	{
		uint32 Handle;
		Reader.SerializeIntPacked(Handle);
		if (bDoChecksum)
		{
			// Skip checksum bits.
			uint32 Checksum;
			Reader << Checksum;
		}

		// Ignore null terminator handle.
		if (Handle == 0)
		{
			break;
		}

		// Get property and apply update to SpatialOS.
		auto PropertyMap = ShadowActor->GetHandlePropertyMap();
		UProperty* Property = PropertyMap[Handle].Property;
		UE_LOG(LogTemp, Log, TEXT("-> Handle: %d Property %s"), Handle, *Property->GetName());
		ShadowActor->ApplyUpdateToSpatial(Reader, Handle, Property);

		// TODO: Currently, object references are not handled properly in ApplyUpdateToSpatial.
		if (Property->IsA(UObjectPropertyBase::StaticClass()))
		{
			UE_LOG(LogTemp, Warning, TEXT("-> Skipping object reference."));
			break;
		}

		// Skip checksum bits.
		if (bDoChecksum)
		{
			// Skip.
			uint32 Checksum;
			Reader << Checksum;
			if (!Property->IsA(UObjectPropertyBase::StaticClass()))
			{
				Reader << Checksum;
			}
		}
	}

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
	return UActorChannel::CleanUp(bForDestroy);
}

bool USpatialActorChannel::ReplicateActor()
{	
	return Super::ReplicateActor();
}

void USpatialActorChannel::SetChannelActor(AActor* InActor)
{
	Super::SetChannelActor(InActor);

	// Create a Spatial entity that corresponds to this actor.
	TSharedPtr<worker::Connection> PinnedConnection = WorkerConnection.Pin();
	if (PinnedConnection.IsValid())
	{
		ReserveEntityIdRequestId = PinnedConnection->SendReserveEntityIdRequest(0);
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
				FVector Loc = GetActor()->GetActorLocation();
				FStringAssetReference ActorClassRef(GetActor()->GetClass());
				FString PathStr = ActorClassRef.ToString();

				UE_LOG(LogTemp, Log, TEXT("Creating entity for actor with path: %s on ActorChannel: %s"), *PathStr, *GetName());

				WorkerAttributeSet UnrealWorkerAttributeSet{ { worker::List<std::string>{"UnrealWorker"} } };
				WorkerAttributeSet UnrealClientAttributeSet{ { worker::List<std::string>{"UnrealClient"} } };

				// UnrealWorker write authority, any worker read authority
				WorkerRequirementSet UnrealWorkerWritePermission{ { UnrealWorkerAttributeSet } };
				WorkerRequirementSet AnyWorkerReadRequirement{ { UnrealWorkerAttributeSet, UnrealClientAttributeSet } };

				auto Entity = unreal::FEntityBuilder::Begin()
					.AddPositionComponent(USpatialOSConversionFunctionLibrary::UnrealCoordinatesToSpatialOsCoordinatesCast(Loc), UnrealWorkerWritePermission)
					.AddMetadataComponent(Metadata::Data{ TCHAR_TO_UTF8(*PathStr) })
					.SetPersistence(true)
					.SetReadAcl(AnyWorkerReadRequirement)
					.Build();

				CreateEntityRequestId = PinnedConnection->SendCreateEntityRequest(Entity, Op.EntityId, 0);
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
		//todo-giray: Need a less hacky way of finding the right create request, as there will be more than one in flight.
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
			FEntityId EntityId(Op.EntityId.value_or(0));

			UE_LOG(LogTemp, Warning, TEXT("Received create entity response op for %d"), EntityId.ToSpatialEntityId());
			// once we know the entity was successfully spawned, add the local actor 
			// to the package map and to the EntityRegistry
			PMC->ResolveEntityActor(GetActor(), EntityId);
			Driver->GetEntityRegistry()->AddToRegistry(EntityId, GetActor());
		}
	}
}	

