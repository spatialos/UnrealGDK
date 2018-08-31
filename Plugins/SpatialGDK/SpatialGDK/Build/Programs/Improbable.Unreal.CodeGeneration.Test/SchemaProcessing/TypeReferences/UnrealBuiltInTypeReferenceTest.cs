using System;
using Improbable.CodeGeneration.Model;
using Improbable.Unreal.CodeGeneration.Model;
using Improbable.Unreal.CodeGeneration.SchemaProcessing.TypeReferences;
using NUnit.Framework;

namespace Improbable.Unreal.CodeGeneration.Test.SchemaProcessing.Unreal.TypeReferneces
{
    public class UnrealBuiltInTypeReferenceTest
    {
        [Test]
        public static void unrealbuiltintypereference_is_initiated_as_expected_when_passed_a_built_in_type_reference()
        {
            var builtInTypeReference = GenerateBuiltInTypeReference("int32");

            var unrealBuiltInTypeReference = new UnrealBuiltInTypeReference(builtInTypeReference);

            Assert.That(unrealBuiltInTypeReference.UnderlyingCapitalisedName == "StdInt32T");
            Assert.That(unrealBuiltInTypeReference.RequiredIncludes.Count == 0);
            Assert.That(unrealBuiltInTypeReference.UnderlyingQualifiedName == "std::int32_t");
            Assert.That(unrealBuiltInTypeReference.UnrealType == "int");

            Assert.That(unrealBuiltInTypeReference.AssignUnderlyingValueToUnrealMemberVariable("TestField", "val") == "TestField = static_cast<int>(val)");
            Assert.That(unrealBuiltInTypeReference.ConvertUnderlyingValueToUnrealLocalVariable("val") == "static_cast<int>(val)");
            Assert.That(unrealBuiltInTypeReference.ConvertUnderlyingValueToUnrealMemberVariable("val") == "static_cast<int>(val)");
            Assert.That(unrealBuiltInTypeReference.ConvertUnrealValueToSnapshotValue("val") == "val");
            Assert.That(unrealBuiltInTypeReference.ConvertUnrealValueToUnderlyingValue("1") == "static_cast<std::int32_t>(1)");
            Assert.That(unrealBuiltInTypeReference.DefaultInitialisationString == "0");
            Assert.That(unrealBuiltInTypeReference.SnapshotType == "int");
        }

        [Test]
        public static void unrealbuiltintypereference_is_initiated_as_expected_when_passed_a_standard_library_type()
        {
            var builtInTypeReference = GenerateStandardLibraryReference();

            var unrealBuiltInTypeReference = new UnrealBuiltInTypeReference(builtInTypeReference);

            Assert.That(unrealBuiltInTypeReference.UnderlyingCapitalisedName == "ImprobableCoordinates");
            Assert.That(unrealBuiltInTypeReference.RequiredIncludes.Count == 2);
            Assert.That(unrealBuiltInTypeReference.RequiredIncludes.Contains("\"improbable/standard_library.h\""));
            Assert.That(unrealBuiltInTypeReference.RequiredIncludes.Contains("\"SpatialOSConversionFunctionLibrary.h\""));
            Assert.That(unrealBuiltInTypeReference.UnderlyingQualifiedName == "improbable::Coordinates");
            Assert.That(unrealBuiltInTypeReference.UnrealType == "FVector");

            Assert.That(unrealBuiltInTypeReference.AssignUnderlyingValueToUnrealMemberVariable("TestField", "val") == "TestField = USpatialOSConversionFunctionLibrary::SpatialOsCoordinatesToUnrealCoordinates(FVector(static_cast<float>(val.x()), static_cast<float>(val.y()), static_cast<float>(val.z())))");
            Assert.That(unrealBuiltInTypeReference.ConvertUnderlyingValueToUnrealLocalVariable("val") == "USpatialOSConversionFunctionLibrary::SpatialOsCoordinatesToUnrealCoordinates(FVector(static_cast<float>(val.x()), static_cast<float>(val.y()), static_cast<float>(val.z())))");
            Assert.That(unrealBuiltInTypeReference.ConvertUnderlyingValueToUnrealMemberVariable("val") == "USpatialOSConversionFunctionLibrary::SpatialOsCoordinatesToUnrealCoordinates(FVector(static_cast<float>(val.x()), static_cast<float>(val.y()), static_cast<float>(val.z())))");
            Assert.That(unrealBuiltInTypeReference.ConvertUnrealValueToSnapshotValue("val") == "val");
            Assert.That(unrealBuiltInTypeReference.ConvertUnrealValueToUnderlyingValue("val") == "USpatialOSConversionFunctionLibrary::UnrealCoordinatesToSpatialOsCoordinatesCast(val)");
            Assert.That(unrealBuiltInTypeReference.DefaultInitialisationString == "improbable::Coordinates(0, 0, 0)");
            Assert.That(unrealBuiltInTypeReference.SnapshotType == "FVector");
        }

        [Test]
        public static void unrealbuiltintypereference_throws_when_given_an_unsupported_built_in_type()
        {
            foreach (var type in UnrealTypeMappings.UnsupportedSchemaTypes)
            {
                var builtInTypeReference = GenerateBuiltInTypeReference(type);
                Assert.Throws<ArgumentException>(() => new UnrealBuiltInTypeReference(builtInTypeReference));
            }
        }

        private static TypeReferenceRaw GenerateBuiltInTypeReference(string typename)
        {
            var rawTypeReference = new TypeReferenceRaw();
            rawTypeReference.builtInType = typename;
            return rawTypeReference;
        }

        private static TypeReferenceRaw GenerateStandardLibraryReference()
        {
            var rawTypeReference = new TypeReferenceRaw();
            rawTypeReference.userType = "improbable.Coordinates";
            var test = rawTypeReference.IsBuiltInType;
            return rawTypeReference;
        }
    }
}
