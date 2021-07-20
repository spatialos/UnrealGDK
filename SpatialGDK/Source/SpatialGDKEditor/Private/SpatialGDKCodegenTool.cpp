// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKCodegenTool.h"

#include "SpatialCommonTypes.h"
#include "SpatialGDKServicesConstants.h"
#include "SpatialGDKServicesModule.h"

#include "Utils/SchemaBundleParser.h"

#include "DesktopPlatformModule.h"
#include "Misc/FileHelper.h"

static const char* s_ReadComponentFieldTemplate =
	R"-(	if(Schema_Get{{FieldType}}Count(Object, {{FieldId}}) > 0)
	{
		{{Field}} = Schema_Get{{FieldType}}(Object, {{FieldId}});
	}
)-";

static const char* s_ReadFieldTemplate =
	R"-(	{{Field}} = Schema_Get{{FieldType}}(Object, {{FieldId}});
)-";

static const char* s_WriteFieldTemplate =
	R"-(	Schema_Add{{FieldType}}(Object, {{FieldId}}, {{Field}});
)-";

static const char* s_StructTemplate = R"-(
struct {{StructName}}
{
	void ReadFromObject(Schema_Object* Object)
	{
		{{StructRead}}
	}

	void WriteToObject(Schema_Object* Object)
	{
		{{StructWrite}}
	}

	{{StructFields}}
)-";

static const char* s_ComponentTemplate = R"-(
struct {{ComponentName}}
{
	static const Worker_ComponentId ComponentId = {{ComponentId}};

	{{ComponentName}}()
	{
	}

	{{ComponentName}}(const ComponentData& Data)
		: {{ComponentName}}(Data.GetUnderlying())
	{
	}

	{{ComponentName}}(Schema_ComponentData* Data)
	{
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data);
		ReadFromObject(ComponentObject);
	}

	ComponentData CreateComponentData() const
	{
		ComponentData Data(ComponentId);
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.GetUnderlying());

		WriteToObject(ComponentObject);

		return Data;
	}

	ComponentUpdate CreateComponentUpdate() const
	{
		ComponentUpdate Update(ComponentId);
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.GetUnderlying());

		WriteToObject(ComponentObject);

		return Update;
	}

	void ApplyComponentUpdate(const ComponentUpdate& Update) { ApplyComponentUpdate(Update.GetUnderlying()); }

	void ApplyComponentUpdate(Schema_ComponentUpdate* Update)
	{
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update);
		ReadFromObject(ComponentObject);
	}

	void ReadFromObject(Schema_Object* Object)
	{
		{{ComponentRead}}
	}

	void WriteToObject(Schema_Object* Object) const
	{
		{{ComponentWrite}}
	}

	{{ComponentFields}}
};)-";

struct TypeHandler
{
	FString SchemaTypeName;
	FString UETypeName;

	TypeHandler(FString InSchemaTypeName, FString InUETypeName)
		: SchemaTypeName(MoveTemp(InSchemaTypeName))
		, UETypeName(MoveTemp(InUETypeName))
		, EmitFieldFn([](TypeHandler const& Handler, const FString& FieldName) {
			return EmitField_Default(Handler, FieldName);
		})
		, EmitReadFn([](TypeHandler const& Handler, const FString& FieldName, uint32 FieldId) {
			return EmitRead_Default(Handler, FieldName, FieldId);
		})
		, EmitComponentReadFn([](TypeHandler const& Handler, const FString& FieldName, uint32 FieldId) {
			return EmitComponentRead_Default(Handler, FieldName, FieldId);
		})
		, EmitWriteFn([](TypeHandler const& Handler, const FString& FieldName, uint32 FieldId) {
			return EmitWrite_Default(Handler, FieldName, FieldId);
		})
	{
	}

	TFunction<FString(TypeHandler const&, const FString&)> EmitFieldFn;
	TFunction<FString(TypeHandler const&, const FString&, uint32)> EmitReadFn;
	TFunction<FString(TypeHandler const&, const FString&, uint32)> EmitComponentReadFn;
	TFunction<FString(TypeHandler const&, const FString&, uint32)> EmitWriteFn;

	FString EmitField(const FString& FieldName) const { return EmitFieldFn(*this, FieldName); }

	FString EmitRead(const FString& FieldName, int32 FieldId) const { return EmitReadFn(*this, FieldName, FieldId); }

	FString EmitComponentRead(const FString& FieldName, int32 FieldId) const { return EmitComponentReadFn(*this, FieldName, FieldId); }

	FString EmitWrite(const FString& FieldName, int32 FieldId) const { return EmitWriteFn(*this, FieldName, FieldId); }

	static FString EmitField_Default(TypeHandler const& Handler, const FString& FieldName)
	{
		return FString::Printf(TEXT("%s %s;\n"), *Handler.UETypeName, *FieldName);
	}

	static FString EmitRead_Default(TypeHandler const& Handler, const FString& FieldName, int32 FieldId)
	{
		return EmitRead_Impl_Default(Handler, s_ReadFieldTemplate, FieldName, FieldId);
	}

	static FString EmitComponentRead_Default(TypeHandler const& Handler, const FString& FieldName, int32 FieldId)
	{
		return EmitRead_Impl_Default(Handler, s_ReadComponentFieldTemplate, FieldName, FieldId);
	}

	static FString EmitRead_Impl_Default(TypeHandler const& Handler, FString Template, const FString& FieldName, int32 FieldId)
	{
		Template = Template.Replace(TEXT("{{FieldType}}"), *Handler.SchemaTypeName);
		Template = Template.Replace(TEXT("{{FieldId}}"), *FString::Printf(TEXT("%i"), FieldId));
		Template = Template.Replace(TEXT("{{Field}}"), *FieldName);
		return Template;
	}

	static FString EmitWrite_Default(TypeHandler const& Handler, const FString& FieldName, int32 FieldId)
	{
		FString Template(s_WriteFieldTemplate);
		Template = Template.Replace(TEXT("{{FieldType}}"), *Handler.SchemaTypeName);
		Template = Template.Replace(TEXT("{{FieldId}}"), *FString::Printf(TEXT("%i"), FieldId));
		Template = Template.Replace(TEXT("{{Field}}"), *FieldName);
		return Template;
	}
};

void AddHandlerFor(TMap<FString, TypeHandler>& TypesMap, FString SchemaType, FString UEType)
{
	TypeHandler Handler(SchemaType, MoveTemp(UEType));
	TypesMap.Add(MoveTemp(SchemaType), MoveTemp(Handler));
}

void USpatialGDKCodegenTool::GenerateSource()
{
	TMap<FString, TypeHandler> TypesMap;

	AddHandlerFor(TypesMap, TEXT("Bool"), TEXT("bool"));
	AddHandlerFor(TypesMap, TEXT("Int32"), TEXT("int32"));
	AddHandlerFor(TypesMap, TEXT("Int64"), TEXT("int64"));
	AddHandlerFor(TypesMap, TEXT("Uint32"), TEXT("uint32"));
	AddHandlerFor(TypesMap, TEXT("Uint64"), TEXT("uint64"));
	AddHandlerFor(TypesMap, TEXT("Float"), TEXT("float"));
	AddHandlerFor(TypesMap, TEXT("Double"), TEXT("double"));
	AddHandlerFor(TypesMap, TEXT("EntityId"), TEXT("Worker_EntityId"));

	TypeHandler StringHandler(TEXT("String"), TEXT("FString"));
	StringHandler.EmitComponentReadFn = [](TypeHandler const& Handler, FString const& FieldName, int32 FieldId) {
		return FString::Printf(TEXT("%s = GetStringFromSchema(Object, %i);\n"), *FieldName, FieldId);
	};
	StringHandler.EmitReadFn = [](TypeHandler const& Handler, FString const& FieldName, int32 FieldId) {
		return FString::Printf(TEXT("%s = GetStringFromSchema(Object, %i);\n"), *FieldName, FieldId);
	};
	StringHandler.EmitWriteFn = [](TypeHandler const& Handler, FString const& FieldName, int32 FieldId) {
		return FString::Printf(TEXT("AddStringFromSchema(Object, %i, %s);\n"), FieldId, *FieldName);
	};

	TypesMap.Add(StringHandler.SchemaTypeName, MoveTemp(StringHandler));

	TypeHandler CoordinatesHandler(TEXT("improbable.Coordinates"), TEXT("FVector"));
	CoordinatesHandler.EmitComponentReadFn = [](TypeHandler const& Handler, FString const& FieldName, int32 FieldId) {
		return FString::Printf(TEXT("%s = Coordinates::ToFVector(GetCoordinateFromSchema(Object, %i));\n"), *FieldName, FieldId);
	};
	CoordinatesHandler.EmitReadFn = [](TypeHandler const& Handler, FString const& FieldName, int32 FieldId) {
		return FString::Printf(TEXT("%s = Coordinates::ToFVector(GetCoordinateFromSchema(Object, %i));\n"), *FieldName, FieldId);
	};
	CoordinatesHandler.EmitWriteFn = [](TypeHandler const& Handler, FString const& FieldName, int32 FieldId) {
		return FString::Printf(TEXT("AddCoordinateToSchema(Object, %i, Coordinates::FromFVector(%s));\n"), FieldId, *FieldName);
	};

	TypeHandler EdgeLengthHandler = CoordinatesHandler;
	EdgeLengthHandler.SchemaTypeName = "improbable.EdgeLength";

	TypesMap.Add(CoordinatesHandler.SchemaTypeName, MoveTemp(CoordinatesHandler));
	TypesMap.Add(EdgeLengthHandler.SchemaTypeName, MoveTemp(EdgeLengthHandler));

	if (CustomSchemaPath.FilePath.IsEmpty())
	{
		return;
	}

	FString PluginDir = FSpatialGDKServicesModule::GetSpatialGDKPluginDirectory();

	FString BuildDir = FPaths::Combine(SpatialGDKServicesConstants::SpatialOSDirectory, TEXT("build"));
	FString SchemaDir = FPaths::Combine(SpatialGDKServicesConstants::SpatialOSDirectory, TEXT("schema"));
	FString CompiledSchemaDir = FPaths::Combine(BuildDir, TEXT("assembly/schema"));
	FString SchemaJsonPath = FPaths::Combine(CompiledSchemaDir, TEXT("schema.json"));

	IFileManager& Filesystem = IFileManager::Get();

	if (!FPaths::IsUnderDirectory(CustomSchemaPath.FilePath, SchemaDir))
	{
		FString ErrorMessage =
			FString::Printf(TEXT("File %s was not under directory %s. Make sure the schema file is part of the schema generation."),
							*CustomSchemaPath.FilePath, *SchemaDir);
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(ErrorMessage));
		return;
	}
	FString FileToConsider = CustomSchemaPath.FilePath;
	SchemaDir.Append("/");
	FPaths::MakePathRelativeTo(FileToConsider, *SchemaDir);

	TSet<FString> Files;
	Files.Add(FileToConsider);
	TArray<SpatialGDK::SchemaComponent> Components;
	SpatialGDK::ExtractComponentsDetailsFromSchemaJson(SchemaJsonPath, Components, Files);

	for (const auto& Component : Components)
	{
		FString FieldSection;
		FString ReadSection;
		FString WriteSection;

		bool bAllTypesHandled = true;

		for (const auto& Field : Component.Fields)
		{
			const TypeHandler* Handler = TypesMap.Find(Field.Type);

			if (Handler == nullptr)
			{
				FString ErrorMessage =
					FString::Printf(TEXT("Type %s part of component %s cannot be handled."), *Field.Type, *Component.Id.Name);
				FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(ErrorMessage));
				bAllTypesHandled = false;
				break;
			}

			FString FieldName = Field.Name;
			FieldName[0] = toupper(FieldName[0]);

			FieldSection.Append(Handler->EmitField(FieldName));
			ReadSection.Append(Handler->EmitComponentRead(FieldName, Field.FieldId));
			WriteSection.Append(Handler->EmitWrite(FieldName, Field.FieldId));
		}

		if (bAllTypesHandled)
		{
			FString ComponentName = Component.Id.Name;
			ComponentName = ComponentName.Replace(TEXT("."), TEXT("_"));
			ComponentName[0] = toupper(ComponentName[0]);

			FString ComponentClass = s_ComponentTemplate;
			ComponentClass = ComponentClass.Replace(TEXT("{{ComponentId}}"), *FString::Printf(TEXT("%i"), Component.Id.ComponentId));
			ComponentClass = ComponentClass.Replace(TEXT("{{ComponentName}}"), *ComponentName);
			ComponentClass = ComponentClass.Replace(TEXT("{{ComponentRead}}"), *ReadSection);
			ComponentClass = ComponentClass.Replace(TEXT("{{ComponentWrite}}"), *WriteSection);
			ComponentClass = ComponentClass.Replace(TEXT("{{ComponentFields}}"), *FieldSection);

			// UE_LOG(LogTemp, Log, TEXT("%s"), *ComponentClass);

			IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();

			TArray<FString> SavePath;
			bool bSavePath = DesktopPlatform->SaveFileDialog(
				FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr), TEXT("Save generated code"),
				FPaths::GetProjectFilePath(), FString::Printf(TEXT("%s.h"), *ComponentName), TEXT("*"), EFileDialogFlags::None, SavePath);

			if (bSavePath)
			{
				FFileHelper::SaveStringToFile(ComponentClass, *SavePath[0]);
			}
		}
	}
}
