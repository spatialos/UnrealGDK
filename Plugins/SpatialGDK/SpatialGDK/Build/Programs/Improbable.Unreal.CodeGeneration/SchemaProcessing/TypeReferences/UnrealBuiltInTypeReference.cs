using System;
using System.Collections.Generic;
using Improbable.CodeGeneration.Model;
using Improbable.CodeGeneration.Utils;
using Improbable.Unreal.CodeGeneration.Model;

namespace Improbable.Unreal.CodeGeneration.SchemaProcessing.TypeReferences
{
    public sealed class UnrealBuiltInTypeReference : UnrealTypeReference
    {
        public UnrealBuiltInTypeReference(TypeReferenceRaw typeReference)
        {
            if (UnrealTypeMappings.UnsupportedSchemaTypes.Contains(typeReference.TypeName))
            {
                throw new ArgumentException(string.Format("Schema type '{0}' is currently not supported in Unreal.", typeReference.TypeName));
            }

            ReferenceType = ReferenceType.BuiltIn;
            if (UnrealTypeMappings.builtInTypeToRequiredInclude.ContainsKey(typeReference.TypeName))
            {
                RequiredIncludes = UnrealTypeMappings.builtInTypeToRequiredInclude[typeReference.TypeName];
            }
            else
            {
                RequiredIncludes = new List<string>();
            }

            UnderlyingQualifiedName = UnrealTypeMappings.builtInSchemaTypeToCppType[typeReference.TypeName];
            UnderlyingCapitalisedName = Formatting.CppQualifiedNameToCapitalisedCamelCase(UnderlyingQualifiedName);

            UnrealType = UnrealTypeMappings.cppTypeToUnrealType[UnderlyingQualifiedName];

            ConvertUnderlyingValueToUnrealMemberVariable = GenerateCppValueToUnrealValueString;
            AssignUnderlyingValueToUnrealMemberVariable = (capitalizedName, cppValue) => { return string.Format("{0} = {1}", capitalizedName, GenerateCppValueToUnrealValueString(cppValue)); };

            CheckInequality = (capitalizedName, compName) =>
            {
                switch (typeReference.TypeName)
                {
                    case BuiltInTypeConstants.builtInFloat:
                        return string.Format("!FMath::IsNearlyEqual({0},{1},(float)KINDA_SMALL_NUMBER)", capitalizedName, compName);
                    case BuiltInTypeConstants.builtInCoordinates:
                    case BuiltInTypeConstants.builtInVector3d:
                    case BuiltInTypeConstants.builtInVector3f:
                        return string.Format("!{0}.Equals({1})", capitalizedName, compName);
                    default:
                        return string.Format("{0} != {1}", capitalizedName, compName);
                }
            };
            ConvertUnrealValueToSnapshotValue = (VarName) => { return string.Format("{0}", VarName); };
            ConvertUnderlyingValueToUnrealLocalVariable = ConvertUnderlyingValueToUnrealMemberVariable;
            ConvertUnrealValueToUnderlyingValue = GenerateUnrealValueToCppValueString;

            DefaultInitialisationString = UnrealTypeMappings.BuiltInSchemaTypeToCppDefaultValue[typeReference.TypeName];
            ArgumentName = UnrealTypeMappings.CppTypesToPassByReference.Contains(UnderlyingQualifiedName) ? string.Format("const {0}&", UnrealType) : UnrealType;
            SnapshotType = UnrealType;
            UClassName = "";
            DefaultValue = UnrealTypeMappings.CppTypeToDefaultUnrealValue[UnderlyingQualifiedName];
        }

        private string GenerateCppValueToUnrealValueString(string cppValue)
        {
            if (!UnrealTypeMappings.CppTypeToUnrealValue.ContainsKey(UnderlyingQualifiedName))
            {
                throw new Exception("GenerateCppValueToUnrealValueString: Cpp type not contained in dictionary; must be a native cpp type.");
            }

            return string.Format(UnrealTypeMappings.CppTypeToUnrealValue[UnderlyingQualifiedName], cppValue);
        }

        private string GenerateUnrealValueToCppValueString(string unrealValue)
        {
            if (!UnrealTypeMappings.CppTypeToCppValueDictionary.ContainsKey(UnderlyingQualifiedName))
            {
                throw new Exception("GenerateUnrealValueToCppValueString: Cpp type not contained in CppTypeToCppValueDictionary dictionary; must be a native cpp type.");
            }

            return string.Format(UnrealTypeMappings.CppTypeToCppValueDictionary[UnderlyingQualifiedName], unrealValue);
        }
    }
}
