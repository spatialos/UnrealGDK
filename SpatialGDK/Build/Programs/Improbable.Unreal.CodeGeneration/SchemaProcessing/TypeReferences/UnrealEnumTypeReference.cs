using System.Collections.Generic;
using Improbable.CodeGeneration.Utils;
using Improbable.Unreal.CodeGeneration.SchemaProcessing.Details;

namespace Improbable.Unreal.CodeGeneration.SchemaProcessing.TypeReferences
{
    public sealed class UnrealEnumTypeReference : UnrealTypeReference
    {
        public UnrealEnumTypeReference(UnrealEnumDetails unrealEnum)
        {
            ReferenceType = ReferenceType.Enum;

            UnderlyingQualifiedName = unrealEnum.UnderlyingQualifiedName;
            UnderlyingCapitalisedName = Formatting.CppQualifiedNameToCapitalisedCamelCase(UnderlyingQualifiedName);
            UnrealType = string.Format("E{0}", unrealEnum.CapitalisedName);
            RequiredIncludes = new List<string>();
            RequiredIncludes.Add(string.Format("\"{0}.h\"", unrealEnum.CapitalisedName));
            ConvertUnderlyingValueToUnrealMemberVariable = (cppValue) =>
            {
                // Set owning object as new uobject's outer.
                return string.Format("static_cast<E{0}>({1})", unrealEnum.CapitalisedName, cppValue);
            };
            AssignUnderlyingValueToUnrealMemberVariable = (capitalizedName, cppValue) =>
            {
                // Set owning object as new uobject's outer.
                return string.Format("{0} = static_cast<E{1}>({2})", capitalizedName, unrealEnum.CapitalisedName, cppValue);
            };
            CheckInequality = (capitalizedName, compName) => { return string.Format("{0} != {1}", capitalizedName, compName); };
            ConvertUnderlyingValueToUnrealLocalVariable = (cppValue) =>
            {
                // Set static instance package as the uobject's outer.
                return string.Format("static_cast<E{0}>({1})", unrealEnum.CapitalisedName, cppValue);
            };
            ConvertUnrealValueToSnapshotValue = (VarName) => { return string.Format("{0}", VarName); };
            var underlyingQualifiedName = UnderlyingQualifiedName;
            ConvertUnrealValueToUnderlyingValue = (unrealValue) => { return string.Format("static_cast<{0}>({1})", underlyingQualifiedName, unrealValue); };

            DefaultInitialisationString = string.Format("static_cast<{0}>(0)", UnderlyingQualifiedName);
            ArgumentName = UnrealType;
            SnapshotType = UnrealType;
            UClassName = "";
            DefaultValue = string.Format("static_cast<{0}>(0)", UnrealType);
        }
    }
}
