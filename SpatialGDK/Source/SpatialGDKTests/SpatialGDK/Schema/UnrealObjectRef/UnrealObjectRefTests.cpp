// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CoreMinimal.h"

#include "Tests/TestDefinitions.h"
#include "Tests/AutomationCommon.h"
#include "Schema/UnrealObjectRef.h"
#include "SoftObjectPtr.h"
#include "Package.h"
#include "LinkerLoad.h"
#include "SpatialPackageMapClient.h"
#include "SpatialNetBitReader.h"
#include "SpatialNetBitWriter.h"

#define UNREALOBJECTREF_TEST(TestName) \
	GDK_TEST(Core, FUnrealObjectRef, TestName)

UNREALOBJECTREF_TEST(GIVEN_a_softpointer_WHEN_making_an_object_ref_from_it_THEN_we_can_recover_it)
{
	FString PackagePath = "/Game/TestAsset/DummyAsset";
	FString ObjectName = "DummyObject";
	FSoftObjectPath SoftPath(PackagePath + "." + ObjectName);
	FSoftObjectPtr DummySoftReference(SoftPath);

	FUnrealObjectRef SoftObjectRef = FUnrealObjectRef::FromSoftObjectPtr(DummySoftReference);

	TestTrue("Got a stably named reference", SoftObjectRef.Path.IsSet() && SoftObjectRef.Path.IsSet());

	FSoftObjectPtr OutPtr;

	FUnrealObjectRef::ToSoftObjectPtr(SoftObjectRef, OutPtr);

	TestTrue("Can serialize a SoftObjectPointer", DummySoftReference == OutPtr);

	return true;
}

// Todo : Add tests involving the PackageMapClient (needs a NetDriver).
// Todo : Add test using entityId (needs the PackageMapClient)
// Todo : Add test using actual assets to generate the path to/from (needs the PackageMapClient)
