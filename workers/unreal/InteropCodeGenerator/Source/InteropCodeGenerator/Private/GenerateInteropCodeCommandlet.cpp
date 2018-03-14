// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GenerateInteropCodeCommandlet.h"

#include "Utils/CodeWriter.h"
#include "Utils/ComponentIdGenerator.h"
#include "TypeStructure.h"
#include "SchemaGenerator.h"
#include "TypeBindingGenerator.h"

#include "Misc/FileHelper.h"

namespace
{
int GenerateCompleteSchemaFromClass(const FString& SchemaPath, const FString& ForwardingCodePath, int ComponentId, UClass* Class)
{
	FCodeWriter OutputSchema;
	FCodeWriter OutputHeader;
	FCodeWriter OutputSource;

	FString SchemaFilename = FString::Printf(TEXT("Unreal%s"), *Class->GetName());
	FString TypeBindingFilename = FString::Printf(TEXT("SpatialTypeBinding_%s"), *Class->GetName());

	FPropertyLayout Layout = CreatePropertyLayout(Class);

	// Generate schema.
	int NumComponents = GenerateSchemaFromLayout(OutputSchema, ComponentId, Class, Layout);
	OutputSchema.WriteToFile(FString::Printf(TEXT("%s%s.schema"), *SchemaPath, *SchemaFilename));

	// Generate forwarding code.
	GenerateForwardingCodeFromLayout(OutputHeader, OutputSource, SchemaFilename, TypeBindingFilename, Class, Layout);
	OutputHeader.WriteToFile(FString::Printf(TEXT("%s%s.h"), *ForwardingCodePath, *TypeBindingFilename));
	OutputSource.WriteToFile(FString::Printf(TEXT("%s%s.cpp"), *ForwardingCodePath, *TypeBindingFilename));

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
	OutputListSource.Print();
	for (auto& ClassName : Classes)
	{
		OutputListSource.Printf("#include \"SpatialTypeBinding_%s.h\"", *ClassName);
	}
	OutputListSource.Print();
	OutputListSource.Print("TArray<UClass*> GetGeneratedTypeBindings()");
	OutputListSource.Print("{").Indent();
	OutputListSource.Print("return {");
	OutputListSource.Indent();
	for (int i = 0; i < Classes.Num(); ++i)
	{
		OutputListSource.Printf(TEXT("USpatialTypeBinding_%s::StaticClass()%s"), *Classes[i], i < (Classes.Num() - 1) ? TEXT(",") : TEXT(""));
	}
	OutputListSource.Outdent();
	OutputListSource.Print("};");
	OutputListSource.Outdent().Print("}");

	// Write to files.
	OutputListHeader.WriteToFile(FString::Printf(TEXT("%sSpatialTypeBindingList.h"), *ForwardingCodePath));
	OutputListSource.WriteToFile(FString::Printf(TEXT("%sSpatialTypeBindingList.cpp"), *ForwardingCodePath));
}
} // ::

int32 UGenerateInteropCodeCommandlet::Main(const FString& Params)
{
	FString CombinedSchemaPath = FPaths::Combine(*FPaths::GetPath(FPaths::GetProjectFilePath()), TEXT("../../../schema/improbable/unreal/generated/"));
	FString CombinedForwardingCodePath = FPaths::Combine(*FPaths::GetPath(FPaths::GetProjectFilePath()), TEXT("../../../workers/unreal/Game/Source/NUF/NUF/Generated/"));
	UE_LOG(LogTemp, Display, TEXT("Schema path %s - Forwarding code path %s"), *CombinedSchemaPath, *CombinedForwardingCodePath);

	TArray<FString> Classes = {TEXT("PlayerController"), TEXT("PlayerState"), TEXT("Character"), TEXT("WheeledVehicle")};
	if (FPaths::CollapseRelativeDirectories(CombinedSchemaPath) && FPaths::CollapseRelativeDirectories(CombinedForwardingCodePath))
	{
		// Component IDs 100000 to 100009 reserved for other NUF components.
		int ComponentId = 100010;
		for (auto& ClassName : Classes)
		{
			UClass* Class = FindObject<UClass>(ANY_PACKAGE, *ClassName);
			ComponentId += GenerateCompleteSchemaFromClass(CombinedSchemaPath, CombinedForwardingCodePath, ComponentId, Class);
		}
		GenerateTypeBindingList(CombinedForwardingCodePath, Classes);
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("Path was invalid - schema not generated"));
	}

	return 0;
}
