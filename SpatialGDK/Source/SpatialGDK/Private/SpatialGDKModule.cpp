// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKModule.h"

// clang-format off
#include "SpatialConstants.h"
#include "SpatialConstants.cxx"
// clang-format on

#include <improbable/c_trace.h>
#include <improbable/c_worker.h>

#define LOCTEXT_NAMESPACE "FSpatialGDKModule"

DEFINE_LOG_CATEGORY(LogSpatialGDKModule);

IMPLEMENT_MODULE(FSpatialGDKModule, SpatialGDK)

size_t TotalAllocate;
size_t TotalDeallocate;

struct user_data
{
	const char* call_point;
};

struct CustomAllocator
{
	static void* Custom_AllocateFunction(size_t size, void* state)
	{
		TotalAllocate += size;
		return malloc(size);
	}
	static void Custom_DeallocateFunction(void* pointer, size_t size, void* state)
	{
		TotalDeallocate += size;
		free(pointer);
	}
};

void FSpatialGDKModule::ShowAllocateStatus()
{
	UE_LOG(LogSpatialGDKModule, Warning, TEXT("Worker SDK allocate %u bytes, free %u bytes, current %u bytes."),
		   (unsigned int)TotalAllocate, (unsigned int)TotalDeallocate, (unsigned int)(TotalAllocate - TotalDeallocate));
}

void FSpatialGDKModule::StartupModule()
{
	struct user_data user_data;
	user_data.call_point = "worker api";
	Worker_AllocateFunction* AllocateFunc = &CustomAllocator::Custom_AllocateFunction;
	Worker_DeallocateFunction* DeallocateFunc = &CustomAllocator::Custom_DeallocateFunction;
	Worker_Alpha_SetAllocator(AllocateFunc, DeallocateFunc, &user_data);
}

void FSpatialGDKModule::ShutdownModule()
{
	UE_LOG(LogSpatialGDKModule, Warning, TEXT("Spatial GDK Module shuting down."));
}

#undef LOCTEXT_NAMESPACE
