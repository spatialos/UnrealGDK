using System.Collections.Generic;
using Improbable.CodeGeneration.Model;
using Improbable.Unreal.CodeGeneration.SchemaProcessing.Details;
using Improbable.Unreal.CodeGeneration.SchemaProcessing.TypeReferences;
using NUnit.Framework;

namespace Improbable.Unreal.CodeGeneration.Test.SchemaProcessing.Unreal.TypeReferences
{
    public class UnrealListTypeReferenceTest
    {
        [Test]
        public static void unreallisttypereference_is_initiated_as_expected_when_passed_a_list_type_reference_with_a_built_in_type_as_contained_type()
        {
            var listTypeRaw = new FieldDefinitionRaw.ListTypeRaw()
            {
                valueType = new TypeReferenceRaw()
                {
                    sourceReference = new SourceReferenceRaw { line = "1", column = "1" },
                    builtInType = "float",
                    userType = null
                }
            };

            var containedTypeReference = new UnrealBuiltInTypeReference(listTypeRaw.valueType);

            var unrealListTypeReference = new UnrealListTypeReference(containedTypeReference);

            Assert.That(unrealListTypeReference.UnderlyingCapitalisedName == "Float");
            Assert.That(unrealListTypeReference.RequiredIncludes.Count == 1);
            Assert.That(unrealListTypeReference.RequiredIncludes.Contains("\"FloatList.h\""));
            Assert.That(unrealListTypeReference.UnderlyingQualifiedName == "float");
            Assert.That(unrealListTypeReference.UnrealType == "UFloatList*");

            Assert.That(unrealListTypeReference.AssignUnderlyingValueToUnrealMemberVariable("TestField", "val") == "if (TestField == nullptr) { TestField = NewObject<UFloatList>(this); } TestField->Init(val)");
            Assert.That(unrealListTypeReference.ConvertUnderlyingValueToUnrealLocalVariable("val") == "NewObject<UFloatList>()->Init(val)");
            Assert.That(unrealListTypeReference.ConvertUnderlyingValueToUnrealMemberVariable("val") == "NewObject<UFloatList>(this)->Init(val)");
            Assert.That(unrealListTypeReference.ConvertUnrealValueToSnapshotValue("val") == "val->GetUnderlying()");
            Assert.That(unrealListTypeReference.ConvertUnrealValueToUnderlyingValue("val") == "val->GetUnderlying()");
            Assert.That(unrealListTypeReference.DefaultInitialisationString == "worker::List<float>()");
            Assert.That(unrealListTypeReference.SnapshotType == "worker::List<float>");
        }

        [Test]
        public static void unreallisttypereference_is_initiated_as_expected_when_passed_a_list_type_reference_with_an_user_type_as_contained_type()
        {
            var userTypeDefinition = GenerateTypeDefinition();

            var unrealTypeDetails = new UnrealTypeDetails(userTypeDefinition, "TestType", new List<UnrealFieldDetails>(), null);
            var containedTypeReference = new UnrealUserTypeReference(unrealTypeDetails);

            var unrealListTypeReference = new UnrealListTypeReference(containedTypeReference);

            Assert.That(unrealListTypeReference.UnderlyingCapitalisedName == "ImprobableCodegenTestType");
            Assert.That(unrealListTypeReference.RequiredIncludes.Count == 2);
            Assert.That(unrealListTypeReference.RequiredIncludes.Contains("\"TestType.h\""));
            Assert.That(unrealListTypeReference.RequiredIncludes.Contains("\"ImprobableCodegenTestTypeList.h\""));
            Assert.That(unrealListTypeReference.UnderlyingQualifiedName == "improbable::codegen::TestType");
            Assert.That(unrealListTypeReference.UnrealType == "UImprobableCodegenTestTypeList*");

            Assert.That(unrealListTypeReference.AssignUnderlyingValueToUnrealMemberVariable("TestField", "val") == "if (TestField == nullptr) { TestField = NewObject<UImprobableCodegenTestTypeList>(this); } TestField->Init(val)");
            Assert.That(unrealListTypeReference.ConvertUnderlyingValueToUnrealLocalVariable("val") == "NewObject<UImprobableCodegenTestTypeList>()->Init(val)");
            Assert.That(unrealListTypeReference.ConvertUnderlyingValueToUnrealMemberVariable("val") == "NewObject<UImprobableCodegenTestTypeList>(this)->Init(val)");
            Assert.That(unrealListTypeReference.ConvertUnrealValueToUnderlyingValue("val") == "val->GetUnderlying()");
        }

        [Test]
        public static void unreallisttypereference_is_initiated_as_expected_when_passed_a_list_type_reference_with_an_enum_type_as_contained_type()
        {
            var userEnumDefinition = GenerateEnumDefinition();

            var unrealEnumDetails = new UnrealEnumDetails(userEnumDefinition, "TestEnum", null);
            var containedEnumReference = new UnrealEnumTypeReference(unrealEnumDetails);

            var unrealEnumTypeReference = new UnrealListTypeReference(containedEnumReference);

            Assert.That(unrealEnumTypeReference.UnderlyingCapitalisedName == "ImprobableCodegenTestEnum");
            Assert.That(unrealEnumTypeReference.RequiredIncludes.Count == 2);
            Assert.That(unrealEnumTypeReference.RequiredIncludes.Contains("\"ImprobableCodegenTestEnumList.h\""));
            Assert.That(unrealEnumTypeReference.RequiredIncludes.Contains("\"TestEnum.h\""));
            Assert.That(unrealEnumTypeReference.UnderlyingQualifiedName == "improbable::codegen::TestEnum");
            Assert.That(unrealEnumTypeReference.UnrealType == "UImprobableCodegenTestEnumList*");

            Assert.That(unrealEnumTypeReference.AssignUnderlyingValueToUnrealMemberVariable("TestField", "val") == "if (TestField == nullptr) { TestField = NewObject<UImprobableCodegenTestEnumList>(this); } TestField->Init(val)");
            Assert.That(unrealEnumTypeReference.ConvertUnderlyingValueToUnrealLocalVariable("val") == "NewObject<UImprobableCodegenTestEnumList>()->Init(val)");
            Assert.That(unrealEnumTypeReference.ConvertUnderlyingValueToUnrealMemberVariable("val") == "NewObject<UImprobableCodegenTestEnumList>(this)->Init(val)");
            Assert.That(unrealEnumTypeReference.ConvertUnrealValueToUnderlyingValue("val") == "val->GetUnderlying()");
        }

        private static TypeDefinitionRaw GenerateTypeDefinition()
        {
            var typeDefinitionRaw = new TypeDefinitionRaw();
            typeDefinitionRaw.name = "TestType";
            typeDefinitionRaw.qualifiedName = "improbable.codegen.TestType";
            typeDefinitionRaw.sourceReference = new SourceReferenceRaw() { line = "1", column = "1" };
            typeDefinitionRaw.optionSettings = null;
            typeDefinitionRaw.enumDefinitions = null;
            typeDefinitionRaw.typeDefinitions = null;
            typeDefinitionRaw.fieldDefinitions = null;

            return typeDefinitionRaw;
        }

        private static EnumDefinitionRaw GenerateEnumDefinition()
        {
            var enumDefinitionRaw = new EnumDefinitionRaw();
            enumDefinitionRaw.name = "TestEnum";
            enumDefinitionRaw.qualifiedName = "improbable.codegen.TestEnum";
            enumDefinitionRaw.sourceReference = new SourceReferenceRaw() { line = "1", column = "1" };
            enumDefinitionRaw.valueDefinitions = new EnumDefinitionRaw.ValueDefinitionRaw[]
            {
                new EnumDefinitionRaw.ValueDefinitionRaw
                {
                    name = "Test",
                    value = "1",
                    sourceReference = new SourceReferenceRaw() { line = "1", column = "1" }
                }
            };

            return enumDefinitionRaw;
        }
    }
}
