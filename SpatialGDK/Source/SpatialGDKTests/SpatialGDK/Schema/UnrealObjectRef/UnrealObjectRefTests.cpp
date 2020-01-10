// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CoreMinimal.h"

#include "Tests/TestDefinitions.h"
#include "Tests/AutomationCommon.h"
#include "Schema/UnrealObjectRef.h"
#include "SoftObjectPtr.h"

#define UNREALOBJECTREF_TEST(TestName) \
	GDK_TEST(Core, FUnrealObjectRef, TestName)

UNREALOBJECTREF_TEST(GIVEN_a_softpointer_WHEN_making_an_object_ref_from_it_THEN_we_can_recover_it)
{
	FString PackagePath = "/Game/TestAsset/DummyAsset";
	FString ObjectName = "DummyObject";
	FSoftObjectPath SoftPath(PackagePath + "." + ObjectName);
	FSoftObjectPtr DummySoftReference(SoftPath);

	FUnrealObjectRef SoftObjectRef = FUnrealObjectRef::FromSoftObjectPath(DummySoftReference.ToSoftObjectPath());

	TestTrue("Got a stably named reference", SoftObjectRef.Path.IsSet() && SoftObjectRef.Path.IsSet());

	FSoftObjectPtr OutPtr;
	OutPtr = FUnrealObjectRef::ToSoftObjectPath(SoftObjectRef);

	TestTrue("Can serialize a SoftObjectPointer", DummySoftReference == OutPtr);

	return true;
}

// [UNR-2691] Todo : Add tests involving the PackageMapClient (needs a NetDriver).
// [UNR-2691] Todo : Add test using entityId (needs the PackageMapClient)
// [UNR-2691] Todo : Add test using actual assets to generate the path to/from (needs the PackageMapClient)
