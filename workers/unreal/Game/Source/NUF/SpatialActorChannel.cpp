// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialActorChannel.h"
#include "SpatialNetDriver.h"
#include "SpatialPackageMapClient.h"
#include "EntityRegistry.h"
#include "Engine/DemoNetDriver.h"
#include "Net/DataBunch.h"
#include "Net/NetworkProfiler.h"
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

using namespace improbable;

USpatialActorChannel::USpatialActorChannel(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{
	ChType = CHTYPE_Actor;
	bCoreActor = true;
	bSendingInitialBunch = false;
	ActorEntityId = worker::EntityId{};
}

void USpatialActorChannel::Init(UNetConnection* Connection, int32 ChannelIndex, bool bOpenedLocally)
{
	Super::Init(Connection, ChannelIndex, bOpenedLocally);

	USpatialNetDriver* Driver = Cast<USpatialNetDriver>(Connection->Driver);
	check(Driver);

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

<<<<<<< HEAD
=======
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
	auto& PropertyMap = FSpatialTypeBinding_Character::GetHandlePropertyMap();
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

void USpatialActorChannel::StartBecomingDormant()
{
	UActorChannel::StartBecomingDormant();
}

void USpatialActorChannel::BecomeDormant()
{
	UActorChannel::BecomeDormant();
}

>>>>>>> Renamed SpatialUpdateInterop files to SpatialTypeBinding. Made
bool USpatialActorChannel::CleanUp(const bool bForDestroy)
{
	USpatialNetConnection* SpatialConnection = Cast<USpatialNetConnection>(Connection);
	if (SpatialConnection && SpatialConnection->Driver->IsServer()
		&& SpatialConnection->bReliableSpatialConnection)
	{
		TSharedPtr<worker::Connection> PinnedConnection = WorkerConnection.Pin();
		if (PinnedConnection.IsValid())
		{
			PinnedConnection->SendDeleteEntityRequest(ActorEntityId, 0);
		}
	}
	return UActorChannel::CleanUp(bForDestroy);
}

//This is a bookkeeping function that is similar to the one in RepLayout.cpp, modified for our needs (e.g. no NaKs)
// We can't use the one in RepLayout.cpp because it's private and it cannot account for our approach.
void UpdateChangelistHistory(FRepState * RepState)
{
	check(RepState->HistoryEnd >= RepState->HistoryStart);

	const int32 HistoryCount = RepState->HistoryEnd - RepState->HistoryStart;
	check(HistoryCount < FRepState::MAX_CHANGE_HISTORY);

	for (int32 i = RepState->HistoryStart; i < RepState->HistoryEnd; i++)
	{
		const int32 HistoryIndex = i % FRepState::MAX_CHANGE_HISTORY;

		FRepChangedHistory & HistoryItem = RepState->ChangeHistory[HistoryIndex];

		check(HistoryItem.Changed.Num() > 0);		// All active history items should contain a change list

		HistoryItem.Changed.Empty();
		HistoryItem.OutPacketIdRange = FPacketIdRange();
		RepState->HistoryStart++;
	}

	// Remove any tiling in the history markers to keep them from wrapping over time
	const int32 NewHistoryCount = RepState->HistoryEnd - RepState->HistoryStart;

	check(NewHistoryCount <= FRepState::MAX_CHANGE_HISTORY);

	RepState->HistoryStart = RepState->HistoryStart % FRepState::MAX_CHANGE_HISTORY;
	RepState->HistoryEnd = RepState->HistoryStart + NewHistoryCount;
}

bool USpatialActorChannel::ReplicateActor()
{
	check(Actor);
	check(!Closing);
	check(Connection);
	check(Connection->PackageMap);

	const UWorld* const ActorWorld = Actor->GetWorld();

	// The package map shouldn't have any carry over guids
	if (CastChecked<UPackageMapClient>(Connection->PackageMap)->GetMustBeMappedGuidsInLastBunch().Num() != 0)
	{
		UE_LOG(LogNet, Warning, TEXT("ReplicateActor: PackageMap->GetMustBeMappedGuidsInLastBunch().Num() != 0: %i"), CastChecked<UPackageMapClient>(Connection->PackageMap)->GetMustBeMappedGuidsInLastBunch().Num());
	}

	// Time how long it takes to replicate this particular actor
	STAT(FScopeCycleCounterUObject FunctionScope(Actor));

	// Create an outgoing bunch, and skip this actor if the channel is saturated.
	FOutBunch Bunch(this, 0);
	if (Bunch.IsError())
	{
		return false;
	}

	bIsReplicatingActor = true;
	FReplicationFlags RepFlags;

	// Send initial stuff.
	if (OpenPacketId.First == INDEX_NONE)
	{
		RepFlags.bNetInitial = true;
		Bunch.bClose = Actor->bNetTemporary;
		Bunch.bReliable = true; // Net temporary sends need to be reliable as well to force them to retry
	}

	//Here, Unreal would have determined if this connection belongs to this actor's Outer.
	//We don't have this concept when it comes to connections, our ownership-based logic is in the interop layer.
	//Setting this to true, but should not matter in the end.
	RepFlags.bNetOwner = true;

	// If initial, send init data.
	if (RepFlags.bNetInitial && OpenedLocally)
	{
		Connection->PackageMap->SerializeNewActor(Bunch, this, Actor);
	
		Actor->OnSerializeNewActor(Bunch);
	}

	RepFlags.bNetSimulated = (Actor->GetRemoteRole() == ROLE_SimulatedProxy);
	RepFlags.bRepPhysics = Actor->ReplicatedMovement.bRepPhysics;
	RepFlags.bReplay = ActorWorld && (ActorWorld->DemoNetDriver == Connection->GetDriver());
	RepFlags.bNetInitial = RepFlags.bNetInitial;

	UE_LOG(LogNetTraffic, Log, TEXT("Replicate %s, bNetInitial: %d, bNetOwner: %d"), *Actor->GetName(), RepFlags.bNetInitial, RepFlags.bNetOwner);

	FMemMark	MemMark(FMemStack::Get());	// The calls to ReplicateProperties will allocate memory on FMemStack::Get(), and use it in ::PostSendBunch. we free it below

											// ----------------------------------------------------------
											// Replicate Actor and Component properties and RPCs
											// ----------------------------------------------------------

	bool WroteSomethingImportant = false;

	ActorReplicator->ChangelistMgr->Update(Actor, Connection->Driver->ReplicationFrame, ActorReplicator->RepState->LastCompareIndex, RepFlags, bForceCompareProperties);

	const int32 PossibleNewHistoryIndex = ActorReplicator->RepState->HistoryEnd % FRepState::MAX_CHANGE_HISTORY;

	FRepChangedHistory& PossibleNewHistoryItem = ActorReplicator->RepState->ChangeHistory[PossibleNewHistoryIndex];

	TArray<uint16>& Changed = PossibleNewHistoryItem.Changed;

	FRepChangelistState* ChangelistState = ActorReplicator->ChangelistMgr->GetRepChangelistState();
	// Gather all change lists that are new since we last looked, and merge them all together into a single CL
	for (int32 i = ActorReplicator->RepState->LastChangelistIndex; i < ChangelistState->HistoryEnd; i++)
	{
		const int32 HistoryIndex = i % FRepChangelistState::MAX_CHANGE_HISTORY;

		FRepChangedHistory& HistoryItem = ChangelistState->ChangeHistory[HistoryIndex];

		TArray<uint16> Temp = Changed;
		ActorReplicator->RepLayout->MergeChangeList((uint8*)Actor, HistoryItem.Changed, Temp, Changed);
	}

	const bool bCompareIndexSame = ActorReplicator->RepState->LastCompareIndex == ChangelistState->CompareIndex;
	ActorReplicator->RepState->LastCompareIndex = ChangelistState->CompareIndex;

	// We can early out if we know for sure there are no new changelists to send
	if (bCompareIndexSame || ActorReplicator->RepState->LastChangelistIndex == ChangelistState->HistoryEnd)
	{
		UpdateChangelistHistory(ActorReplicator->RepState);
		return false;
	}

	//todo-giray: We currently don't take replication of custom delta properties into account here because it doesn't use changelists.
	// see ActorReplicator->ReplicateCustomDeltaProperties().

	if (Changed.Num() > 0)
	{
		USpatialUpdateInterop* UpdateInterop = Cast<USpatialNetDriver>(Connection->Driver)->GetSpatialUpdateInterop();
		check(UpdateInterop);
		UpdateInterop->SendSpatialUpdate(this, Changed);
		WroteSomethingImportant = true;
		ActorReplicator->RepState->HistoryEnd++;
		UpdateChangelistHistory(ActorReplicator->RepState);
	}

	ActorReplicator->RepState->LastChangelistIndex = ChangelistState->HistoryEnd;
	//todo-giray: The rest of this function is taken from Unreal's own implementation. It is mostly redundant in our case,
	// but keeping it here for now to give us a chance to investigate if we need to write our own implementation for any of
	// any code block below.

	/*
	// The Actor
	WroteSomethingImportant |= ActorReplicator->ReplicateProperties(Bunch, RepFlags);

	//todo-giray: Implement subobject replication
	// The SubObjects
	WroteSomethingImportant |= Actor->ReplicateSubobjects(this, &Bunch, &RepFlags);
*/

	// Look for deleted subobjects
	for (auto RepComp = ReplicationMap.CreateIterator(); RepComp; ++RepComp)
	{
		if (!RepComp.Key().IsValid())
		{
			// Write a deletion content header:
			WriteContentBlockForSubObjectDelete(Bunch, RepComp.Value()->ObjectNetGUID);

			WroteSomethingImportant = true;
			Bunch.bReliable = true;

			RepComp.Value()->CleanUp();
			RepComp.RemoveCurrent();
		}
	}

	// -----------------------------
	// Send if necessary
	// -----------------------------
	bool SentBunch = false;
	if (WroteSomethingImportant)
	{
		FPacketIdRange PacketRange = SendBunch(&Bunch, 1);

		for (auto RepComp = ReplicationMap.CreateIterator(); RepComp; ++RepComp)
		{
			RepComp.Value()->PostSendBunch(PacketRange, Bunch.bReliable);
		}
		SentBunch = true;
	}

	// If we evaluated everything, mark LastUpdateTime, even if nothing changed.
	LastUpdateTime = Connection->Driver->Time;

	MemMark.Pop();

	bIsReplicatingActor = false;

	bForceCompareProperties = false;		// Only do this once per frame when set

	return WroteSomethingImportant;
}

void USpatialActorChannel::SetChannelActor(AActor* InActor)
{
	Super::SetChannelActor(InActor);

	if (!bCoreActor)
		return;

	USpatialNetConnection* SpatialConnection = Cast<USpatialNetConnection>(Connection);

	if (SpatialConnection && SpatialConnection->Driver->IsServer()
		&& SpatialConnection->bReliableSpatialConnection)
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
		//Callbacks->Add(PinnedView->OnReserveEntityIdResponse(std::bind(&USpatialActorChannel::OnReserveEntityIdResponse, this, std::placeholders::_1)));
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
						.AddMetadataComponent(Metadata::Data{ TCHAR_TO_UTF8(*PathStr) })
						.SetPersistence(true)
						.SetReadAcl(AnyWorkerReadRequirement)
						// For now, just a dummy component we add to every such entity to make sure client has write access to at least one component.
						//todo-giray: Remove once we're using proper (generated) entity templates here.
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
	USpatialNetConnection* SpatialConnection = Driver->GetSpatialOSNetConnection();
	// This can be true only on the server
	if (SpatialConnection)
	{
		USpatialPackageMapClient* PMC = Cast<USpatialPackageMapClient>(SpatialConnection->PackageMap);
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

