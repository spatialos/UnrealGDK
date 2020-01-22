// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "Interop/SpatialStaticComponentView.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "SpatialConstants.h"
#include "Utils/SchemaUtils.h"

DEFINE_LOG_CATEGORY(LogSpatialVirtualWorkerTranslator);

SpatialVirtualWorkerTranslator::SpatialVirtualWorkerTranslator()
	: bIsReady(false)
	, LocalPhysicalWorkerName(SpatialConstants::TRANSLATOR_UNSET_PHYSICAL_NAME)
	, LocalVirtualWorkerId(SpatialConstants::INVALID_VIRTUAL_WORKER_ID)
{}

void SpatialVirtualWorkerTranslator::Init(UAbstractLBStrategy* InLoadBalanceStrategy,
	USpatialStaticComponentView* InStaticComponentView,
	PhysicalWorkerName InPhysicalWorkerName)
{
	LoadBalanceStrategy = InLoadBalanceStrategy;

	check(InStaticComponentView != nullptr);
	StaticComponentView = InStaticComponentView;

	LocalPhysicalWorkerName = InPhysicalWorkerName;
}

const PhysicalWorkerName* SpatialVirtualWorkerTranslator::GetPhysicalWorkerForVirtualWorker(VirtualWorkerId Id) const
{
	return VirtualToPhysicalWorkerMapping.Find(Id);
}

void SpatialVirtualWorkerTranslator::ApplyVirtualWorkerManagerData(Schema_Object* ComponentObject)
{
    UE_LOG(LogSpatialVirtualWorkerTranslator, Log, TEXT("(%s) ApplyVirtualWorkerManagerData"), *LocalPhysicalWorkerName);

	// The translation schema is a list of Mappings, where each entry has a virtual and physical worker ID. 
	ApplyMappingFromSchema(ComponentObject);

	for (const auto& Entry : VirtualToPhysicalWorkerMapping)
	{
		UE_LOG(LogSpatialVirtualWorkerTranslator, Log, TEXT("Translator assignment: %d - %s"), Entry.Key, *(Entry.Value));
	}
}

// Check to see if this worker's physical worker name is in the mapping. If it isn't, it's possibly an old mapping.
// This is needed to give good behaviour across restarts. It's not very efficient, but it should happen only a few times
// after a PiE restart.
bool SpatialVirtualWorkerTranslator::IsValidMapping(Schema_Object* Object) const
{
	int32 TranslationCount = (int32)Schema_GetObjectCount(Object, SpatialConstants::VIRTUAL_WORKER_TRANSLATION_MAPPING_ID);

	for (int32 i = 0; i < TranslationCount; i++)
	{
		// Get each entry of the list and then unpack the virtual and physical IDs from the entry.
		Schema_Object* MappingObject = Schema_IndexObject(Object, SpatialConstants::VIRTUAL_WORKER_TRANSLATION_MAPPING_ID, i);
		if (SpatialGDK::GetStringFromSchema(MappingObject, SpatialConstants::MAPPING_PHYSICAL_WORKER_NAME) == LocalPhysicalWorkerName)
		{
			return true;
		}
	}

	return false;
}

// The translation schema is a list of Mappings, where each entry has a virtual and physical worker ID.
// This method should only be called on workers who are not authoritative over the mapping and also when
// a worker first becomes authoritative for the mapping.
void SpatialVirtualWorkerTranslator::ApplyMappingFromSchema(Schema_Object* Object)
{
	if (!IsValidMapping(Object))
	{
		UE_LOG(LogSpatialVirtualWorkerTranslator, Log, TEXT("(%s) Received invalid mapping, likely due to PiE restart, will wait for a valid version."), *LocalPhysicalWorkerName);
		return;
	}

	// Resize the map to accept the new data.
	VirtualToPhysicalWorkerMapping.Empty();
	int32 TranslationCount = (int32)Schema_GetObjectCount(Object, SpatialConstants::VIRTUAL_WORKER_TRANSLATION_MAPPING_ID);
	VirtualToPhysicalWorkerMapping.Reserve(TranslationCount);

	for (int32 i = 0; i < TranslationCount; i++)
	{
		// Get each entry of the list and then unpack the virtual and physical IDs from the entry.
		Schema_Object* MappingObject = Schema_IndexObject(Object, SpatialConstants::VIRTUAL_WORKER_TRANSLATION_MAPPING_ID, i);
		VirtualWorkerId VirtualWorkerId = Schema_GetUint32(MappingObject, SpatialConstants::MAPPING_VIRTUAL_WORKER_ID);
		PhysicalWorkerName PhysicalWorkerName = SpatialGDK::GetStringFromSchema(MappingObject, SpatialConstants::MAPPING_PHYSICAL_WORKER_NAME);

		// Insert each into the provided map.
		UpdateMapping(VirtualWorkerId, PhysicalWorkerName);
	}
}

void SpatialVirtualWorkerTranslator::UpdateMapping(VirtualWorkerId Id, PhysicalWorkerName Name)
{
	VirtualToPhysicalWorkerMapping.Add(Id, Name);

	if (LocalVirtualWorkerId == SpatialConstants::INVALID_VIRTUAL_WORKER_ID && Name == LocalPhysicalWorkerName)
	{
		LocalVirtualWorkerId = Id;
		bIsReady = true;

		// Tell the strategy about the local virtual worker id. This is an "if" and not a "check" to allow unit tests which don't
		// provide a strategy.
		if (LoadBalanceStrategy.IsValid())
		{
			LoadBalanceStrategy->SetLocalVirtualWorkerId(LocalVirtualWorkerId);
		}

		UE_LOG(LogSpatialVirtualWorkerTranslator, Log, TEXT("VirtualWorkerTranslator is now ready for loadbalancing."));
	}
}
