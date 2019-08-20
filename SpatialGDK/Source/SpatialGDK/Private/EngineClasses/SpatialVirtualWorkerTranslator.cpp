// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "EngineClasses/SpatialGameInstance.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/SpatialStaticComponentView.h"
#include "Interop/SpatialReceiver.h"
#include "Net/UnrealNetwork.h"
#include "Schema/StandardLibrary.h"
#include "SpatialConstants.h"

DEFINE_LOG_CATEGORY(LogSpatialVirtualWorkerTranslator);

using namespace SpatialGDK;

ASpatialVirtualWorkerTranslator::ASpatialVirtualWorkerTranslator(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, NetDriver(nullptr)
	, bWorkerEntityQueryInFlight(false)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	bReplicates = true;
	bAlwaysRelevant = true;

	NetUpdateFrequency = 100.f;

	TranslatorEntityId = SpatialConstants::INITIAL_VIRTUAL_WORKER_TRANSLATOR_ENTITY_ID;
}

void ASpatialVirtualWorkerTranslator::Init(USpatialNetDriver* InNetDriver)
{
	NetDriver = InNetDriver;
}

void ASpatialVirtualWorkerTranslator::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogSpatialVirtualWorkerTranslator, Log, TEXT("(%s role %d) BeginPlay"), *Cast<USpatialGameInstance>(GetWorld()->GetGameInstance())->GetSpatialWorkerLabel(), (int)Role);

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
}

void ASpatialVirtualWorkerTranslator::Tick(float DeltaSeconds)
{
	ProcessQueuedAclAssignmentRequests();
}

void ASpatialVirtualWorkerTranslator::AuthorityChanged(const Worker_AuthorityChangeOp& AuthOp)
{
	const bool bAuthoritative = AuthOp.authority == WORKER_AUTHORITY_AUTHORITATIVE;

	if (AuthOp.component_id == SpatialConstants::VIRTUAL_WORKER_MANAGER_COMPONENT_ID)
	{	
		UE_LOG(LogSpatialVirtualWorkerTranslator, Log, TEXT("(%s role %d) Authority over the VirtualWorkerTranslator has changed. This worker %s authority."), *Cast<USpatialGameInstance>(GetWorld()->GetGameInstance())->GetSpatialWorkerLabel(), (int)Role, bAuthoritative ? TEXT("now has") : TEXT("does not have"));

		// Only the VirtualWorkerTranslator has this component, so we can use it to get our real EntityId
		TranslatorEntityId = AuthOp.entity_id;

		if (bAuthoritative)
		{
			for (const FString& VirtualWorker : VirtualWorkers)
			{
				UnassignedVirtualWorkers.Enqueue(VirtualWorker);
			}

			VirtualWorkerAssignment.AddDefaulted(VirtualWorkers.Num());

			QueryForWorkerEntities();
		}
	}
	else if (AuthOp.component_id == SpatialConstants::ENTITY_ACL_COMPONENT_ID &&
		bAuthoritative)
	{
		// We have gained authority over the ACL component for some entity
		// If we have authority, the responsibility here is to set the EntityACL write auth to match the worker requested via the virtual worker component
		QueueAclAssignmentRequest(AuthOp.entity_id);
	}
}

void ASpatialVirtualWorkerTranslator::AssignWorker(const FString& WorkerId)
{
	check(!UnassignedVirtualWorkers.IsEmpty());
	check(NetDriver->StaticComponentView->HasAuthority(TranslatorEntityId, SpatialConstants::VIRTUAL_WORKER_MANAGER_COMPONENT_ID));

	FString VirtualWorkerId;
	int32 VirtualWorkerIndex;

	UnassignedVirtualWorkers.Dequeue(VirtualWorkerId);
	VirtualWorkers.Find(VirtualWorkerId, VirtualWorkerIndex);
	VirtualWorkerAssignment[VirtualWorkerIndex] = WorkerId;

	UE_LOG(LogSpatialVirtualWorkerTranslator, Log, TEXT("(%s role %d) Assigned VirtualWorker %s to simulate on Worker %s"), *Cast<USpatialGameInstance>(GetWorld()->GetGameInstance())->GetSpatialWorkerLabel(), (int)Role, *VirtualWorkerId, *VirtualWorkerAssignment[VirtualWorkerIndex]);
}

// TODO: call this from appropriate location
void ASpatialVirtualWorkerTranslator::UnassignWorker(const FString& WorkerId)
{
	for (int i = 0; i < VirtualWorkerAssignment.Num(); ++i)
	{
		if (VirtualWorkerAssignment[i].Equals(WorkerId))
		{
			VirtualWorkerAssignment[i] = TEXT("");
			UnassignedVirtualWorkers.Enqueue(VirtualWorkers[i]);

			UE_LOG(LogSpatialVirtualWorkerTranslator, Log, TEXT("(%s) Unassigned VirtualWorker %s from simulating on Worker %s"), *Cast<USpatialGameInstance>(GetWorld()->GetGameInstance())->GetSpatialWorkerLabel(), *VirtualWorkers[i], *WorkerId);
			break;
		}
	}
}

void ASpatialVirtualWorkerTranslator::OnComponentAdded(const Worker_AddComponentOp& Op)
{
	if (Op.data.component_id == SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID)
	{
		// TODO - Set Entity's ACL component to correct worker id based on requested virtual worker
		//Worker_EntityId EntityId = Op.entity_id;
		//AuthorityIntent* MyAuthorityIntent = NetDriver->StaticComponentView->GetComponentData<SpatialGDK::AuthorityIntent>(EntityId);
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

void ASpatialVirtualWorkerTranslator::OnComponentRemoved(const Worker_RemoveComponentOp& Op)
{
	if (Op.component_id == SpatialConstants::WORKER_COMPONENT_ID)
	{
		//OnWorkerComponentRemoved(Op.data);
	}
}

void ASpatialVirtualWorkerTranslator::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpatialVirtualWorkerTranslator, VirtualWorkerAssignment);
}

void ASpatialVirtualWorkerTranslator::QueueAclAssignmentRequest(const Worker_EntityId EntityId)
{
	UE_LOG(LogSpatialVirtualWorkerTranslator, Log, TEXT("(%s role %d) Queueing ACL assignment request for %lld"), *Cast<USpatialGameInstance>(GetWorld()->GetGameInstance())->GetSpatialWorkerLabel(), (int)Role, EntityId);
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

	UE_LOG(LogSpatialVirtualWorkerTranslator, Log, TEXT("(%s role %d) Setting Acl WriteAuth for entity %lld to workerid: %s"), *Cast<USpatialGameInstance>(GetWorld()->GetGameInstance())->GetSpatialWorkerLabel(), (int)Role, EntityId, *WorkerId);

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
	UE_LOG(LogSpatialVirtualWorkerTranslator, Log, TEXT("(%s role %d) OnRep_VirtualWorkerAssignment"), *Cast<USpatialGameInstance>(GetWorld()->GetGameInstance())->GetSpatialWorkerLabel(), (int)Role);
	for (int i = 0; i < VirtualWorkerAssignment.Num(); ++i)
	{
		UE_LOG(LogSpatialVirtualWorkerTranslator, Log, TEXT("(%s role %d) assignment: %s"), *Cast<USpatialGameInstance>(GetWorld()->GetGameInstance())->GetSpatialWorkerLabel(), (int)Role, *VirtualWorkerAssignment[i]);
	}
	OnWorkerAssignmentChanged.ExecuteIfBound(VirtualWorkerAssignment);
}

void ASpatialVirtualWorkerTranslator::ConstructVirtualWorkerMappingFromQueryResponse(const Worker_EntityQueryResponseOp& Op)
{
	for (uint32_t i = 0; i < Op.results[0].component_count; i++)
	{
		Worker_ComponentData Data = Op.results[0].components[i];
		if (Data.component_id == SpatialConstants::WORKER_COMPONENT_ID)
		{
			Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

			const FString& WorkerType = GetStringFromSchema(ComponentObject, SpatialConstants::WORKER_TYPE_ID);

			if (WorkerType.Equals(SpatialConstants::DefaultServerWorkerType.ToString()) &&
				!UnassignedVirtualWorkers.IsEmpty())
			{
				AssignWorker(GetStringFromSchema(ComponentObject, SpatialConstants::WORKER_ID_ID));
			}
		}
	}
}

void ASpatialVirtualWorkerTranslator::QueryForWorkerEntities()
{
	UE_LOG(LogSpatialVirtualWorkerTranslator, Log, TEXT("(%s role %d) Sending query for WorkerEntities"), *Cast<USpatialGameInstance>(GetWorld()->GetGameInstance())->GetSpatialWorkerLabel(), (int)Role);

	checkf(!bWorkerEntityQueryInFlight, TEXT("(%s role %d) Trying to query for worker entities while a previous query is still in flight!"), *Cast<USpatialGameInstance>(GetWorld()->GetGameInstance())->GetSpatialWorkerLabel(), (int)Role);

	Worker_ComponentConstraint WorkerEntityComponentConstraint{};
	WorkerEntityComponentConstraint.component_id = SpatialConstants::WORKER_COMPONENT_ID;

	Worker_Constraint WorkerEntityConstraint{};
	WorkerEntityConstraint.constraint_type = WORKER_CONSTRAINT_TYPE_COMPONENT;
	WorkerEntityConstraint.component_constraint = WorkerEntityComponentConstraint;

	Worker_EntityQuery WorkerEntityQuery{};
	WorkerEntityQuery.constraint = WorkerEntityConstraint;
	WorkerEntityQuery.result_type = WORKER_RESULT_TYPE_SNAPSHOT;

	Worker_RequestId RequestID;
	RequestID = NetDriver->Connection->SendEntityQueryRequest(&WorkerEntityQuery);

	EntityQueryDelegate WorkerEntityQueryDelegate;
	WorkerEntityQueryDelegate.BindLambda([this](const Worker_EntityQueryResponseOp& Op)
	{
		if (!NetDriver->StaticComponentView->HasAuthority(TranslatorEntityId, SpatialConstants::VIRTUAL_WORKER_MANAGER_COMPONENT_ID))
		{
			UE_LOG(LogSpatialVirtualWorkerTranslator, Log, TEXT("(%s role %d) Received response to WorkerEntityQuery, but don't have authority over VIRTUAL_WORKER_MANAGER_COMPONENT.  Aborting processing."), *Cast<USpatialGameInstance>(GetWorld()->GetGameInstance())->GetSpatialWorkerLabel(), (int)Role);
			return;
		}

		if (Op.status_code != WORKER_STATUS_CODE_SUCCESS)
		{
			UE_LOG(LogSpatialVirtualWorkerTranslator, Log, TEXT("(%s role %d) Could not find Worker Entities via entity query: %s"), *Cast<USpatialGameInstance>(GetWorld()->GetGameInstance())->GetSpatialWorkerLabel(), (int)Role, UTF8_TO_TCHAR(Op.message));
		}
		else if (Op.result_count == 0)
		{
			UE_LOG(LogSpatialVirtualWorkerTranslator, Log, TEXT("(%s role %d) Worker Entity query shows that Worker Entities do not yet exist in the world."), *Cast<USpatialGameInstance>(GetWorld()->GetGameInstance())->GetSpatialWorkerLabel(), (int)Role);
		}
		else
		{
			UE_LOG(LogSpatialVirtualWorkerTranslator, Log, TEXT("(%s role %d) Processing Worker Entity query response"), *Cast<USpatialGameInstance>(GetWorld()->GetGameInstance())->GetSpatialWorkerLabel(), (int)Role);
			ConstructVirtualWorkerMappingFromQueryResponse(Op);
		}

		bWorkerEntityQueryInFlight = false;
	});

	bWorkerEntityQueryInFlight = true;

	NetDriver->Receiver->AddEntityQueryDelegate(RequestID, WorkerEntityQueryDelegate);
}
