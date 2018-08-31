using System.Collections.Generic;
using Improbable.CodeGeneration.Model;
using Improbable.CodeGeneration.Test;
using Improbable.Unreal.CodeGeneration.SchemaProcessing.Details;
using Improbable.Unreal.CodeGeneration.SchemaProcessing.TypeReferences;
using NUnit.Framework;

namespace Improbable.Unreal.CodeGeneration.Test.SchemaProcessing.Unreal.TypeReferences
{
    public class UnrealUserTypeReferenceTest
    {
        [Test]
        public static void unrealusertypereference_is_initiated_as_expected_when_passed_an_user_type_reference()
        {
            var fieldDefinitionFloat = ModelTypeFactory.GenerateFieldDefinition("test", "1", new TypeReferenceRaw
            {
                sourceReference = new SourceReferenceRaw { line = "1", column = "1" },
                builtInType = "float"
            });

            var fieldDefinitionInt32 = ModelTypeFactory.GenerateFieldDefinition("test", "1", new TypeReferenceRaw
            {
                sourceReference = new SourceReferenceRaw { line = "1", column = "1" },
                builtInType = "int32"
            });

            var userTypeDefinition = ModelTypeFactory.GenerateTypeDefinition("TestType", "improbable.codegen.TestType", null, null, new FieldDefinitionRaw[]
            {
                fieldDefinitionFloat,
                fieldDefinitionInt32
            });

            var unrealFieldDetails = new List<UnrealFieldDetails>();
            unrealFieldDetails.Add(new UnrealFieldDetails(fieldDefinitionFloat, new UnrealBuiltInTypeReference(fieldDefinitionFloat.singularType)));
            unrealFieldDetails.Add(new UnrealFieldDetails(fieldDefinitionInt32, new UnrealBuiltInTypeReference(fieldDefinitionInt32.singularType)));

            var unrealTypeDetails = new UnrealTypeDetails(userTypeDefinition, "TestType", unrealFieldDetails, null);

            var unrealUserTypeReference = new UnrealUserTypeReference(unrealTypeDetails);

            Assert.That(unrealUserTypeReference.UnderlyingCapitalisedName == "ImprobableCodegenTestType");
            Assert.That(unrealUserTypeReference.RequiredIncludes.Count == 1);
            Assert.That(unrealUserTypeReference.RequiredIncludes.Contains("\"TestType.h\""));
            Assert.That(unrealUserTypeReference.UnderlyingQualifiedName == "improbable::codegen::TestType");
            Assert.That(unrealUserTypeReference.UnrealType == "UTestType*");

            Assert.That(unrealUserTypeReference.AssignUnderlyingValueToUnrealMemberVariable("TestField", "val") == "if (TestField == nullptr) { TestField = NewObject<UTestType>(this); } TestField->Init(val)");
            Assert.That(unrealUserTypeReference.ConvertUnderlyingValueToUnrealLocalVariable("val") == "NewObject<UTestType>()->Init(val)");
            Assert.That(unrealUserTypeReference.ConvertUnderlyingValueToUnrealMemberVariable("val") == "NewObject<UTestType>(this)->Init(val)");
            Assert.That(unrealUserTypeReference.ConvertUnrealValueToSnapshotValue("val") == "val->GetUnderlying()");
            Assert.That(unrealUserTypeReference.ConvertUnrealValueToUnderlyingValue("val") == "val->GetUnderlying()");

            Assert.That(unrealUserTypeReference.DefaultInitialisationString == "improbable::codegen::TestType(0, 0)");
            Assert.That(unrealUserTypeReference.SnapshotType == "improbable::codegen::TestType");
        }
    }
}
