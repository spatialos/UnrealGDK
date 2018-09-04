using System.Collections.Generic;
using Improbable.CodeGeneration.Model;
using Improbable.Unreal.CodeGeneration.SchemaProcessing.Details;
using Improbable.Unreal.CodeGeneration.SchemaProcessing.TypeReferences;
using NUnit.Framework;

namespace Improbable.Unreal.CodeGeneration.Test.SchemaProcessing.Unreal.TypeReferences
{
    public class UnrealOptionTypeReferenceTest
    {
        [Test]
        public static void unrealoptiontypereference_is_initiated_as_expected_when_passed_an_option_type_reference_with_a_built_in_type_as_contained_type()
        {
            var optionTypeRaw = new FieldDefinitionRaw.OptionTypeRaw()
            {
                valueType = new TypeReferenceRaw()
                {
                    sourceReference = new SourceReferenceRaw { line = "1", column = "1" },
                    builtInType = "float",
                    userType = null
                }
            };

            var containedTypeReference = new UnrealBuiltInTypeReference(optionTypeRaw.valueType);
            var unrealOptionTypeReference = new UnrealOptionTypeReference(containedTypeReference);

            Assert.That(unrealOptionTypeReference.UnderlyingCapitalisedName == "Float");
            Assert.That(unrealOptionTypeReference.RequiredIncludes.Count == 1);
            Assert.That(unrealOptionTypeReference.RequiredIncludes.Contains("\"FloatOption.h\""));
            Assert.That(unrealOptionTypeReference.UnderlyingQualifiedName == "float");
            Assert.That(unrealOptionTypeReference.UnrealType == "UFloatOption*");

            Assert.That(unrealOptionTypeReference.AssignUnderlyingValueToUnrealMemberVariable("TestField", "val") == "if (TestField == nullptr) { TestField = NewObject<UFloatOption>(this); } TestField->Init(val)");
            Assert.That(unrealOptionTypeReference.ConvertUnderlyingValueToUnrealLocalVariable("val") == "NewObject<UFloatOption>()->Init(val)");
            Assert.That(unrealOptionTypeReference.ConvertUnderlyingValueToUnrealMemberVariable("val") == "NewObject<UFloatOption>(this)->Init(val)");
            Assert.That(unrealOptionTypeReference.ConvertUnrealValueToSnapshotValue("val") == "val->GetUnderlying()");
            Assert.That(unrealOptionTypeReference.ConvertUnrealValueToUnderlyingValue("val") == "val->GetUnderlying()");
            Assert.That(unrealOptionTypeReference.SnapshotType == "worker::Option<float>");
        }

        [Test]
        public static void unrealoptiontypereference_is_initiated_as_expected_when_passed_an_option_type_reference_with_an_user_type_as_contained_type()
        {
            var userTypeDefinition = GenerateTypeDefinition();

            var unrealTypeDetails = new UnrealTypeDetails(userTypeDefinition, "TestType", new List<UnrealFieldDetails>(), null);
            var containedTypeReference = new UnrealUserTypeReference(unrealTypeDetails);

            var unrealOptionTypeReference = new UnrealOptionTypeReference(containedTypeReference);

            Assert.That(unrealOptionTypeReference.UnderlyingCapitalisedName == "ImprobableCodegenTestType");
            Assert.That(unrealOptionTypeReference.RequiredIncludes.Count == 2);
            Assert.That(unrealOptionTypeReference.RequiredIncludes.Contains("\"ImprobableCodegenTestTypeOption.h\""));
            Assert.That(unrealOptionTypeReference.RequiredIncludes.Contains("\"TestType.h\""));
            Assert.That(unrealOptionTypeReference.UnderlyingQualifiedName == "improbable::codegen::TestType");
            Assert.That(unrealOptionTypeReference.UnrealType == "UImprobableCodegenTestTypeOption*");

            Assert.That(unrealOptionTypeReference.AssignUnderlyingValueToUnrealMemberVariable("TestField", "val") == "if (TestField == nullptr) { TestField = NewObject<UImprobableCodegenTestTypeOption>(this); } TestField->Init(val)");
            Assert.That(unrealOptionTypeReference.ConvertUnderlyingValueToUnrealLocalVariable("val") == "NewObject<UImprobableCodegenTestTypeOption>()->Init(val)");
            Assert.That(unrealOptionTypeReference.ConvertUnderlyingValueToUnrealMemberVariable("val") == "NewObject<UImprobableCodegenTestTypeOption>(this)->Init(val)");
            Assert.That(unrealOptionTypeReference.ConvertUnrealValueToUnderlyingValue("val") == "val->GetUnderlying()");
            Assert.That(unrealOptionTypeReference.ConvertUnrealValueToSnapshotValue("val") == "val->GetUnderlying()");
            Assert.That(unrealOptionTypeReference.DefaultInitialisationString == "worker::Option<improbable::codegen::TestType>()");
            Assert.That(unrealOptionTypeReference.SnapshotType == "worker::Option<improbable::codegen::TestType>");
        }

        [Test]
        public static void unrealoptiontypereference_is_initiated_as_expected_when_passed_an_option_type_reference_with_an_enum_type_as_contained_type()
        {
            var userEnumDefinition = GenerateEnumDefinition();

            var unrealEnumDetails = new UnrealEnumDetails(userEnumDefinition, "TestEnum", null);
            var containedEnumReference = new UnrealEnumTypeReference(unrealEnumDetails);

            var unrealEnumTypeReference = new UnrealOptionTypeReference(containedEnumReference);

            Assert.That(unrealEnumTypeReference.UnderlyingCapitalisedName == "ImprobableCodegenTestEnum");
            Assert.That(unrealEnumTypeReference.RequiredIncludes.Count == 2);
            Assert.That(unrealEnumTypeReference.RequiredIncludes.Contains("\"ImprobableCodegenTestEnumOption.h\""));
            Assert.That(unrealEnumTypeReference.RequiredIncludes.Contains("\"TestEnum.h\""));
            Assert.That(unrealEnumTypeReference.UnderlyingQualifiedName == "improbable::codegen::TestEnum");
            Assert.That(unrealEnumTypeReference.UnrealType == "UImprobableCodegenTestEnumOption*");

            Assert.That(unrealEnumTypeReference.AssignUnderlyingValueToUnrealMemberVariable("TestField", "val") == "if (TestField == nullptr) { TestField = NewObject<UImprobableCodegenTestEnumOption>(this); } TestField->Init(val)");
            Assert.That(unrealEnumTypeReference.ConvertUnderlyingValueToUnrealLocalVariable("val") == "NewObject<UImprobableCodegenTestEnumOption>()->Init(val)");
            Assert.That(unrealEnumTypeReference.ConvertUnderlyingValueToUnrealMemberVariable("val") == "NewObject<UImprobableCodegenTestEnumOption>(this)->Init(val)");
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
