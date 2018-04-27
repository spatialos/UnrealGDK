// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorInteropCodeGenerator.h"

#include "SchemaGenerator.h"
#include "TypeBindingGenerator.h"
#include "TypeStructure.h"
#include "Utils/CodeWriter.h"
#include "Utils/ComponentIdGenerator.h"

#include "Misc/FileHelper.h"

DEFINE_LOG_CATEGORY(LogSpatialGDKInteropCodeGenerator);

namespace
{
int GenerateCompleteSchemaFromClass(const FString& SchemaPath, const FString& ForwardingCodePath,
									int ComponentId, UClass* Class,
									const TArray<TArray<FName>>& MigratableProperties)
{
	FCodeWriter OutputSchema;
	FCodeWriter OutputHeader;
	FCodeWriter OutputSource;

	FString SchemaFilename =
		FString::Printf(TEXT("Unreal%s"), *UnrealNameToSchemaTypeName(Class->GetName()));
	FString TypeBindingFilename = FString::Printf(TEXT("SpatialTypeBinding_%s"), *Class->GetName());

	TSharedPtr<FUnrealType> TypeInfo = CreateUnrealTypeInfo(Class, MigratableProperties);

	// Generate schema.
	int NumComponents =
		GenerateTypeBindingSchema(OutputSchema, ComponentId, Class, TypeInfo, SchemaPath);
	OutputSchema.WriteToFile(FString::Printf(TEXT("%s%s.schema"), *SchemaPath, *SchemaFilename));

	// Generate forwarding code.
	GenerateTypeBindingHeader(OutputHeader, SchemaFilename, TypeBindingFilename, Class, TypeInfo);
	GenerateTypeBindingSource(OutputSource, SchemaFilename, TypeBindingFilename, Class, TypeInfo);
	OutputHeader.WriteToFile(
		FString::Printf(TEXT("%s%s.h"), *ForwardingCodePath, *TypeBindingFilename));
	OutputSource.WriteToFile(
		FString::Printf(TEXT("%s%s.cpp"), *ForwardingCodePath, *TypeBindingFilename));

	return NumComponents;
}

void GenerateTypeBindingList(const FString& ForwardingCodePath, const TArray<FString>& Classes)
{
	FCodeWriter OutputListHeader;
	FCodeWriter OutputListSource;

	// Header.
	OutputListHeader.Print(R"""(
		// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
		// Note that this file has been generated automatically
		#pragma once

		TArray<UClass*> GetGeneratedTypeBindings();)""");

	// Implementation.
	OutputListSource.Print(R"""(
		// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
		// Note that this file has been generated automatically

		#include "SpatialTypeBindingList.h")""");
	OutputListSource.PrintNewLine();
	for (auto& ClassName : Classes)
	{
		OutputListSource.Printf("#include \"SpatialTypeBinding_%s.h\"", *ClassName);
	}
	OutputListSource.PrintNewLine();

	// GetGeneratedTypeBindings.
	OutputListSource.BeginFunction({"TArray<UClass*>", "GetGeneratedTypeBindings()"});
	OutputListSource.Print("return {");
	OutputListSource.Indent();
	for (int i = 0; i < Classes.Num(); ++i)
	{
		OutputListSource.Printf(TEXT("USpatialTypeBinding_%s::StaticClass()%s"), *Classes[i],
								i < (Classes.Num() - 1) ? TEXT(",") : TEXT(""));
	}
	OutputListSource.Outdent();
	OutputListSource.Print("};");
	OutputListSource.End();

	// Write to files.
	OutputListHeader.WriteToFile(
		FString::Printf(TEXT("%sSpatialTypeBindingList.h"), *ForwardingCodePath));
	OutputListSource.WriteToFile(
		FString::Printf(TEXT("%sSpatialTypeBindingList.cpp"), *ForwardingCodePath));
}

bool CheckClassNameListValidity(const TArray<FString>& Classes)
{
	for (int i = 0; i < Classes.Num() - 1; ++i)
	{
		const FString& ClassA = Classes[i];
		const FString SchemaTypeA = UnrealNameToSchemaTypeName(ClassA);

		for (int j = i + 1; j < Classes.Num(); ++j)
		{
			const FString& ClassB = Classes[j];
			const FString SchemaTypeB = UnrealNameToSchemaTypeName(ClassB);

			if (SchemaTypeA.Equals(SchemaTypeB))
			{
				UE_LOG(LogSpatialGDKInteropCodeGenerator, Error,
					   TEXT("Class name collision after removing underscores: '%s' and "
							"'%s' - "
							"schema not "
							"generated"),
					   *ClassA, *ClassB);
				return false;
			}
		}
	}

	return true;
}
}  // ::

void SpatialGDKGenerateInteropCode()
{
	FString CombinedSchemaPath =
		FPaths::Combine(*FPaths::GetPath(FPaths::GetProjectFilePath()),
						TEXT("../../../generatedschema/improbable/unreal/"));
	FString CombinedForwardingCodePath =
		FPaths::Combine(*FPaths::GetPath(FPaths::GetProjectFilePath()),
						TEXT("../../../workers/unreal/Game/Source/SampleGame/Generated/"));
	FString AbsoluteCombinedSchemaPath = FPaths::ConvertRelativePathToFull(CombinedSchemaPath);
	FString AbsoluteCombinedForwardingCodePath =
		FPaths::ConvertRelativePathToFull(CombinedForwardingCodePath);

	UE_LOG(LogSpatialGDKInteropCodeGenerator, Display,
		   TEXT("Schema path %s - Forwarding code path %s"), *AbsoluteCombinedSchemaPath,
		   *AbsoluteCombinedForwardingCodePath);

	// Hard coded class information.
	TArray<FString> Classes = {"PlayerController", "PlayerState", "Character", "WheeledVehicle"};
	TMap<FString, TArray<TArray<FName>>> MigratableProperties;
	MigratableProperties.Add("PlayerController", {{"AcknowledgedPawn"}});
	MigratableProperties.Add("Character", {{"CharacterMovement", "GroundMovementMode"},
										   {"CharacterMovement", "MovementMode"},
										   {"CharacterMovement", "CustomMovementMode"}});

	if (!CheckClassNameListValidity(Classes))
	{
		return;
	}

	if (FPaths::CollapseRelativeDirectories(AbsoluteCombinedSchemaPath) &&
		FPaths::CollapseRelativeDirectories(AbsoluteCombinedForwardingCodePath))
	{
		// Component IDs 100000 to 100009 reserved for other SpatialGDK components.
		int ComponentId = 100010;
		for (auto& ClassName : Classes)
		{
			UClass* Class = FindObject<UClass>(ANY_PACKAGE, *ClassName);
			TArray<TArray<FName>> ClassMigratableProperties =
				MigratableProperties.FindRef(ClassName);
			ComponentId +=
				GenerateCompleteSchemaFromClass(CombinedSchemaPath, CombinedForwardingCodePath,
												ComponentId, Class, ClassMigratableProperties);
		}
	}
	else
	{
		UE_LOG(LogSpatialGDKInteropCodeGenerator, Error,
			   TEXT("Path was invalid - schema not generated"));
	}
}
