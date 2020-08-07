// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CoreMinimal.h"

#include "Schema/UnrealObjectRef.h"
#include "Tests/AutomationCommon.h"
#include "Tests/TestDefinitions.h"
#include "UObject/SoftObjectPtr.h"

#define UNREALOBJECTREF_TEST(TestName) GDK_TEST(Core, FUnrealObjectRef, TestName)

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

// TODO : [UNR-2691] Add tests involving the PackageMapClient, with entity Id and actual assets to generate the path to/from (needs a
// NetDriver right now).
