// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialVirtualWorkerTranslator.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/SpatialStaticComponentView.h"
#include "Net/UnrealNetwork.h"
#include "Schema/StandardLibrary.h"
#include "SpatialConstants.h"

DEFINE_LOG_CATEGORY(LogSpatialVirtualWorkerTranslator);

using namespace SpatialGDK;

ASpatialVirtualWorkerTranslator::ASpatialVirtualWorkerTranslator(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	NetDriver(nullptr)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	bReplicates = true;
	bAlwaysRelevant = true;

	NetUpdateFrequency = 100.f;
}

void ASpatialVirtualWorkerTranslator::Init(USpatialNetDriver* InNetDriver)
{
	NetDriver = InNetDriver;
}

void ASpatialVirtualWorkerTranslator::BeginPlay()
{
	Super::BeginPlay();

	// These collections contain static data that is accessible on all server workers via accessor methods
	// This data should likely live somewhere else, but for the purposes of the prototype it's here
	// Zones
	// VirtualWorkers
	// TODO - replace with real data from the editor
	Zones.Add(TEXT("Zone_A"));
	Zones.Add(TEXT("Zone_B"));
	Zones.Add(TEXT("Zone_C"));
	Zones.Add(TEXT("Zone_D"));

	VirtualWorkers.Add("VW_A");
	VirtualWorkers.Add("VW_B");
	VirtualWorkers.Add("VW_C");
	VirtualWorkers.Add("VW_D");

	if (HasAuthority())
	{
		for (const FString& VirtualWorker : VirtualWorkers)
		{
			UnassignedVirtualWorkers.Enqueue(VirtualWorker);
		}

		VirtualWorkerAssignment.AddDefaulted(VirtualWorkers.Num());
	}
}

void ASpatialVirtualWorkerTranslator::Tick(float DeltaSeconds)
{
	ProcessQueuedAclAssignmentRequests();
}
void ASpatialVirtualWorkerTranslator::AuthorityChanged(const Worker_AuthorityChangeOp& AuthOp)
{
	// We have gained or lost authority over the ACL component for some entity
	// If we have authority, the responsibility here is to set the EntityACL write auth to match the worker requested via the virtual worker component
	if (AuthOp.component_id == SpatialConstants::ENTITY_ACL_COMPONENT_ID &&
		AuthOp.authority == WORKER_AUTHORITY_AUTHORITATIVE)
	{
		QueueAclAssignmentRequest(AuthOp.entity_id);
	}
}

void ASpatialVirtualWorkerTranslator::OnWorkerComponentReceived(const Worker_ComponentData& Data)
{
	// TODO - this will possibly miss worker components that arrive before an instance of this class has authority
	// TODO - handle workers disconnecting
	if (HasAuthority())
	{
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		FString WorkerId = GetStringFromSchema(ComponentObject, SpatialConstants::WORKER_ID_ID);
		FString WorkerType = GetStringFromSchema(ComponentObject, SpatialConstants::WORKER_TYPE_ID);

		if (WorkerType.Equals(SpatialConstants::DefaultServerWorkerType.ToString()))
		{
			// We've received information about a new Worker, consider it and possibly assign it as one of our virtual workers
			if (!UnassignedVirtualWorkers.IsEmpty())
			{
				AssignWorker(WorkerId);
			}
		}
	}
}

void ASpatialVirtualWorkerTranslator::AssignWorker(const FString& WorkerId)
{
	check(!UnassignedVirtualWorkers.IsEmpty());

	FString VirtualWorkerId;
	int32 VirtualWorkerIndex;

	UnassignedVirtualWorkers.Dequeue(VirtualWorkerId);
	VirtualWorkers.Find(VirtualWorkerId, VirtualWorkerIndex);
	VirtualWorkerAssignment[VirtualWorkerIndex] = WorkerId;
}

void ASpatialVirtualWorkerTranslator::OnComponentAdded(const Worker_AddComponentOp& Op)
{
	if (Op.data.component_id == SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID)
	{
		// TODO - Set Entity's ACL component to correct worker id based on requested virtual worker
		//Worker_EntityId EntityId = Op.entity_id;
		//AuthorityIntent* MyAuthorityIntent = NetDriver->StaticComponentView->GetComponentData<SpatialGDK::AuthorityIntent>(EntityId);
	}
	else if (Op.data.component_id == SpatialConstants::WORKER_COMPONENT_ID)
	{
		OnWorkerComponentReceived(Op.data);
	}
}

void ASpatialVirtualWorkerTranslator::OnComponentUpdated(const Worker_ComponentUpdateOp& Op)
{
	if (Op.update.component_id == SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID)
	{
		// TODO - Set Entity's ACL component to correct worker id based on requested virtual worker
		//Worker_EntityId EntityId = Op.entity_id;
		//AuthorityIntent* MyAuthorityIntent = NetDriver->StaticComponentView->GetComponentData<SpatialGDK::AuthorityIntent>(EntityId);
	}
}

void ASpatialVirtualWorkerTranslator::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpatialVirtualWorkerTranslator, VirtualWorkerAssignment);
}

void ASpatialVirtualWorkerTranslator::QueueAclAssignmentRequest(const Worker_EntityId EntityId)
{
	AclWriteAuthAssignmentRequests.Add(EntityId);
}

// TODO: should probably move this to SpatialSender
void ASpatialVirtualWorkerTranslator::SetAclWriteAuthority(const Worker_EntityId EntityId, const FString& WorkerId)
{
	check(NetDriver->StaticComponentView->HasAuthority(EntityId, SpatialConstants::ENTITY_ACL_COMPONENT_ID));

	EntityAcl* EntityACL = NetDriver->StaticComponentView->GetComponentData<EntityAcl>(EntityId);
	check(EntityACL);

	WorkerAttributeSet OwningWorkerAttribute = { WorkerId };

	TArray<Worker_ComponentId> ComponentIds;
	EntityACL->ComponentWriteAcl.GetKeys(ComponentIds);

	for (int i = 0; i < ComponentIds.Num(); ++i)
	{
		if (ComponentIds[i] != SpatialConstants::ENTITY_ACL_COMPONENT_ID)
		{
			WorkerRequirementSet* wrs = EntityACL->ComponentWriteAcl.Find(ComponentIds[i]);
			check(wrs->Num() == 1);
			wrs->Empty();
			wrs->Add(OwningWorkerAttribute);
		}
	}

	UE_LOG(LogSpatialVirtualWorkerTranslator, Log, TEXT("Setting Acl WriteAuth for entity %lld to workerid: %s"), EntityId, *WorkerId);

	Worker_ComponentUpdate Update = EntityACL->CreateEntityAclUpdate();
	NetDriver->Connection->SendComponentUpdate(EntityId, &Update);
}

void ASpatialVirtualWorkerTranslator::ProcessQueuedAclAssignmentRequests()
{
	TArray<Worker_EntityId> CompletedRequests;
	CompletedRequests.Reserve(AclWriteAuthAssignmentRequests.Num());

	// TODO: add a mechanism to detect when these requests stay in the queue longer than expected
	for (const Worker_EntityId& EntityId : AclWriteAuthAssignmentRequests)
	{
		AuthorityIntent* MyAuthorityIntentComponent = NetDriver->StaticComponentView->GetComponentData<SpatialGDK::AuthorityIntent>(EntityId);

		// TODO - if some entities won't have the component we should detect that before queueing the request.
		// Need to be certain it is invalid to get here before receiving the AuthIntentComponent for an entity, then we can check() on it.
		if (MyAuthorityIntentComponent == nullptr)
		{
			//UE_LOG(LogSpatialVirtualWorkerTranslator, Warning, TEXT("Detected entity without AuthIntent component"));
			continue;
		}

		const FString& VirtualWorkerId = MyAuthorityIntentComponent->VirtualWorkerId;
		check(!VirtualWorkerId.IsEmpty());

		int32 VirtualWorkerIndex;
		VirtualWorkers.Find(VirtualWorkerId, VirtualWorkerIndex);
		check(VirtualWorkerIndex != -1);

		const FString& OwningWorkerId = VirtualWorkerAssignment[VirtualWorkerIndex];
		if (OwningWorkerId.IsEmpty())
		{
			// A virtual worker -> physical worker mapping may not be established yet.
			// We'll retry on the next Tick().
			continue;
		}

		SetAclWriteAuthority(EntityId, OwningWorkerId);
		CompletedRequests.Add(EntityId);
	}

	AclWriteAuthAssignmentRequests.RemoveAll([&](const Worker_EntityId EntityId) { return CompletedRequests.Contains(EntityId); });
}

void ASpatialVirtualWorkerTranslator::OnRep_VirtualWorkerAssignment()
{
	OnWorkerAssignmentChanged.ExecuteIfBound(VirtualWorkerAssignment);
}
