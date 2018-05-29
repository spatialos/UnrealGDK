using System.Collections.Generic;
using Improbable.CodeGeneration.Model;

namespace Improbable.Unreal.CodeGeneration.Model
{
    public static class UnrealTypeMappings
    {
        public static readonly Dictionary<string, string> builtInSchemaTypeToCppType = new Dictionary<string, string>
        {
            { BuiltInTypeConstants.builtInDouble, cppDouble },
            { BuiltInTypeConstants.builtInFloat, cppFloat },
            { BuiltInTypeConstants.builtInInt32, cppInt32 },
            { BuiltInTypeConstants.builtInInt64, cppInt64 },
            { BuiltInTypeConstants.builtInUint32, cppUint32 },
            { BuiltInTypeConstants.builtInUint64, cppUint64 },
            { BuiltInTypeConstants.builtInSint32, cppInt32 },
            { BuiltInTypeConstants.builtInSint64, cppInt64 },
            { BuiltInTypeConstants.builtInFixed32, cppUint32 },
            { BuiltInTypeConstants.builtInFixed64, cppUint64 },
            { BuiltInTypeConstants.builtInSfixed32, cppInt32 },
            { BuiltInTypeConstants.builtInSfixed64, cppUint64 },
            { BuiltInTypeConstants.builtInBool, cppBool },
            { BuiltInTypeConstants.builtInString, cppString },
            { BuiltInTypeConstants.builtInBytes, cppString },
            { BuiltInTypeConstants.builtInEntityId, cppEntityId },
            { BuiltInTypeConstants.builtInCoordinates, cppCoordinates },
            { BuiltInTypeConstants.builtInVector3d, cppVector3d },
            { BuiltInTypeConstants.builtInVector3f, cppVector3f }
        };

        public static readonly Dictionary<string, List<string>> builtInTypeToRequiredInclude = new Dictionary<string, List<string>>
        {
            { BuiltInTypeConstants.builtInEntityId, new List<string> { "\"EntityId.h\"" } },
            {
                BuiltInTypeConstants.builtInCoordinates, new List<string>
                {
                    "\"improbable/standard_library.h\"",
                    "\"SpatialOSConversionFunctionLibrary.h\""
                }
            },
            { BuiltInTypeConstants.builtInVector3d, new List<string> { "\"improbable/vector3.h\"" } },
            { BuiltInTypeConstants.builtInVector3f, new List<string> { "\"improbable/vector3.h\"" } },
            { BuiltInTypeConstants.builtInString, new List<string> { "<string>" } }
        };

        public static readonly Dictionary<string, string> cppTypeToUnrealType = new Dictionary<string, string>
        {
            { cppFloat, "float" },
            { cppInt32, "int" },
            { cppInt64, "int64" },
            { cppUint64, "int64" },
            { cppBool, "bool" },
            { cppString, "FString" },
            { cppEntityId, "FEntityId" },
            { cppCoordinates, "FVector" },
            { cppVector3f, "FVector" },
            // NOTE: These are temporarily valid as built-in types like Coordinates and worker requirements use these types
            { cppUint32, "int" },
            { cppVector3d, "FVector" },
            { cppDouble, "float" },
        };

        public static readonly Dictionary<string, string> BuiltInSchemaTypeToCppDefaultValue = new Dictionary<string, string>
        {
            { BuiltInTypeConstants.builtInDouble, "0" },
            { BuiltInTypeConstants.builtInFloat, "0" },
            { BuiltInTypeConstants.builtInInt32, "0" },
            { BuiltInTypeConstants.builtInInt64, "0" },
            { BuiltInTypeConstants.builtInUint32, "0" },
            { BuiltInTypeConstants.builtInUint64, "0" },
            { BuiltInTypeConstants.builtInSint32, "0" },
            { BuiltInTypeConstants.builtInSint64, "0" },
            { BuiltInTypeConstants.builtInFixed32, "0" },
            { BuiltInTypeConstants.builtInFixed64, "0" },
            { BuiltInTypeConstants.builtInSfixed32, "0" },
            { BuiltInTypeConstants.builtInSfixed64, "0" },
            { BuiltInTypeConstants.builtInBool, "false" },
            { BuiltInTypeConstants.builtInString, "\"\"" },
            { BuiltInTypeConstants.builtInBytes, "\"\"" },
            { BuiltInTypeConstants.builtInEntityId, "worker::EntityId(0)" },
            { BuiltInTypeConstants.builtInCoordinates, "improbable::Coordinates(0, 0, 0)" },
            { BuiltInTypeConstants.builtInVector3d, "improbable::Vector3d(0, 0, 0)" },
            { BuiltInTypeConstants.builtInVector3f, "improbable::Vector3f(0, 0, 0)" }
        };

        public static readonly Dictionary<string, string> CppTypeToDefaultUnrealValue = new Dictionary<string, string>
        {
            { cppDouble, "0.0f" },
            { cppFloat, "0.0f" },
            { cppInt32, "0" },
            { cppInt64, "0" },
            { cppUint32, "0" },
            { cppUint64, "0" },
            { cppBool, "false" },
            { cppString, "FString()" },
            { cppEntityId, "FEntityId()" },
            { cppCoordinates, "FVector()" },
            { cppVector3d, "FVector()" },
            { cppVector3f, "FVector()" },
        };

        public static readonly Dictionary<string, string> CppTypeToUnrealValue = new Dictionary<string, string>()
        {
            { cppDouble, "static_cast<float>({0})" },
            { cppFloat, "{0}" },
            { cppInt32, "static_cast<int>({0})" },
            { cppInt64, "static_cast<int>({0})" },
            { cppUint32, "static_cast<int>({0})" },
            { cppUint64, "static_cast<int>({0})" },
            { cppBool, "{0}" },
            { cppString, "FString({0}.c_str())" },
            { cppEntityId, "FEntityId({0})" },
            { cppCoordinates, "USpatialOSConversionFunctionLibrary::SpatialOsCoordinatesToUnrealCoordinates(FVector(static_cast<float>({0}.x()), static_cast<float>({0}.y()), static_cast<float>({0}.z())))" },
            { cppVector3d, "FVector(static_cast<float>({0}.x()), static_cast<float>({0}.y()), static_cast<float>({0}.z()))" },
            { cppVector3f, "FVector(static_cast<float>({0}.x()), static_cast<float>({0}.y()), static_cast<float>({0}.z()))" },
        };

        public static readonly Dictionary<string, string> CppTypeToCppValueDictionary = new Dictionary<string, string>()
        {
            { cppDouble, "static_cast<" + cppDouble + ">({0})" },
            { cppFloat, "{0}" },
            { cppInt32, "static_cast<" + cppInt32 + ">({0})" },
            { cppInt64, "static_cast<" + cppInt64 + ">({0})" },
            { cppUint32, "static_cast<" + cppUint32 + ">({0})" },
            { cppUint64, "static_cast<" + cppUint64 + ">({0})" },
            { cppBool, "{0}" },
            { cppString, "TCHAR_TO_UTF8(*{0})" },
            { cppEntityId, "({0}).ToSpatialEntityId()" },
            { cppCoordinates, "USpatialOSConversionFunctionLibrary::UnrealCoordinatesToSpatialOsCoordinatesCast({0})" },
            { cppVector3d, "improbable::Vector3d(static_cast<double>({0}.X), static_cast<double>({0}.Y), static_cast<double>({0}.Z))" },
            { cppVector3f, "improbable::Vector3f(static_cast<double>({0}.X), static_cast<double>({0}.Y), static_cast<double>({0}.Z))" },
        };

        public static readonly HashSet<string> CppTypesToPassByReference = new HashSet<string>()
        {
            { cppString },
            { cppCoordinates },
            { cppVector3d },
            { cppVector3f }
        };

        public static readonly HashSet<string> UnsupportedSchemaTypes = new HashSet<string>()
        {
            { BuiltInTypeConstants.builtInSint32 },
            { BuiltInTypeConstants.builtInSint64 },
            { BuiltInTypeConstants.builtInFixed32 },
            { BuiltInTypeConstants.builtInFixed64 },
            { BuiltInTypeConstants.builtInSfixed32 },
            { BuiltInTypeConstants.builtInSfixed64 },
            // Note: These are temporarily valid as built-in types like Coordinates and worker requirements use these types
            //{ BuiltInTypeConstants.builtInUint32 },
            //{ BuiltInTypeConstants.builtInVector3d },
            //{ BuiltInTypeConstants.builtInDouble }
        };

        private const string cppDouble = "double";
        private const string cppFloat = "float";
        private const string cppInt32 = "std::int32_t";
        private const string cppInt64 = "std::int64_t";
        private const string cppUint32 = "std::uint32_t";
        private const string cppUint64 = "std::uint64_t";
        private const string cppBool = "bool";
        private const string cppString = "std::string";
        private const string cppEntityId = "worker::EntityId";
        private const string cppCoordinates = "improbable::Coordinates";
        private const string cppVector3d = "improbable::Vector3d";
        private const string cppVector3f = "improbable::Vector3f";
    }
}
