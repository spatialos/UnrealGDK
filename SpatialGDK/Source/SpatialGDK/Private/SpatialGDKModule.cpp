// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKModule.h"

// clang-format off
#include "SpatialConstants.h"
#include "SpatialConstants.cxx"
// clang-format on

#define LOCTEXT_NAMESPACE "FSpatialGDKModule"

DEFINE_LOG_CATEGORY(LogSpatialGDKModule);

IMPLEMENT_MODULE(FSpatialGDKModule, SpatialGDK)

size_t CustomAllocateSize;
size_t CustomFreeSize;
float ByteToMegabyte = 0.000001;

struct user_data
{
	const char* call_point;
};

void* my_allocate(size_t size, void* state)
{
	CustomAllocateSize += size;
	return malloc(size);
}

void my_free(void* ptr, size_t size, void* state)
{
	CustomFreeSize += size;
	free(ptr);
}

void FSpatialGDKModule::PrintCustomAllocateInfo()
{
	UE_LOG(LogSpatialGDKModule, Log, TEXT("Custom Allocate %f MB, Free %f MB, Current size %f MB, Delta size %f MB"),
		   CustomAllocateSize * ByteToMegabyte, CustomFreeSize * ByteToMegabyte, (CustomAllocateSize - CustomFreeSize) * ByteToMegabyte,
		   ((CustomAllocateSize - CustomFreeSize) - LastSize) * ByteToMegabyte);

	LastSize = CustomAllocateSize - CustomFreeSize;
}

void FSpatialGDKModule::StartupModule()
{
	LastSize = 0;
	
	user_data u_data;
	u_data.call_point = "worker api";
	//Worker_AllocateFunction custom_allocate = &FSpatialGDKModule::my_allocate;
	//Worker_DeallocateFunction custom_deallocate = &FSpatialGDKModule::my_free;
	Worker_Alpha_SetAllocator(&my_allocate, &my_free, &u_data);
}

void FSpatialGDKModule::ShutdownModule() {}


#undef LOCTEXT_NAMESPACE
