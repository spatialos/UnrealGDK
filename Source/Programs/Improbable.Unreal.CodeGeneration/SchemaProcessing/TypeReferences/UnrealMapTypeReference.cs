using System.Collections.Generic;
using System.Linq;

namespace Improbable.Unreal.CodeGeneration.SchemaProcessing.TypeReferences
{
    public sealed class UnrealMapTypeReference : UnrealTypeReference
    {
        public readonly IUnrealTypeReference KeyType;
        public readonly IUnrealTypeReference ValueType;

        public UnrealMapTypeReference(IUnrealTypeReference containedTypeReferenceKey, IUnrealTypeReference containedTypeReferenceValue)
        {
            ReferenceType = ReferenceType.Map;
            KeyType = containedTypeReferenceKey;
            ValueType = containedTypeReferenceValue;

            var className = string.Format("{0}To{1}Map", containedTypeReferenceKey.UnderlyingCapitalisedName, containedTypeReferenceValue.UnderlyingCapitalisedName);
            UnrealType = string.Format("U{0}*", className);
            UnderlyingQualifiedName = string.Format("worker::Map<{0}, {1}>", containedTypeReferenceKey.UnderlyingQualifiedName, containedTypeReferenceValue.UnderlyingQualifiedName);
            UnderlyingCapitalisedName = null;
            ConvertUnderlyingValueToUnrealMemberVariable = (cppMap) =>
            {
                // Set owning object as new uobject's outer.
                return string.Format("NewObject<U{0}>(this)->Init({1})", className, cppMap);
            };
            AssignUnderlyingValueToUnrealMemberVariable = (capitalizedName, cppMap) =>
            {
                return string.Format(@"if ({0} == nullptr) {{ {0} = NewObject<U{1}>(this); }} {0}->Init({2})",
                                     capitalizedName,
                                     className,
                                     cppMap);
            };
            CheckInequality = (capitalizedName, compName) => { return string.Format("{0} && (*{0} != {1})", capitalizedName, compName); };
            ConvertUnrealValueToSnapshotValue = (VarName) => { return ConvertUnrealValueToUnderlyingValue(VarName); };
            ConvertUnderlyingValueToUnrealLocalVariable = (cppMap) =>
            {
                //set static instance package as the uobject's outer
                return string.Format("NewObject<U{0}>()->Init({1})", className, cppMap);
            };
            ConvertUnrealValueToUnderlyingValue = (unrealMap) => { return string.Format("{0}->GetUnderlying()", unrealMap); };
            RequiredIncludes = new List<string>();
            RequiredIncludes.Add(string.Format("\"{0}.h\"", className));
            RequiredIncludes.AddRange(containedTypeReferenceKey.RequiredIncludes);
            RequiredIncludes.AddRange(containedTypeReferenceValue.RequiredIncludes);
            RequiredIncludes = RequiredIncludes.Distinct().ToList();

            DefaultInitialisationString = string.Format("worker::Map<{0}, {1}>()", KeyType.UnderlyingQualifiedName,
                                                        ValueType.UnderlyingQualifiedName);
            ArgumentName = UnrealType;
            SnapshotType = string.Format("worker::Map<{0}, {1}>", containedTypeReferenceKey.UnderlyingQualifiedName, containedTypeReferenceValue.UnderlyingQualifiedName);
            UClassName = string.Format("U{0}", className);
            DefaultValue = "nullptr";
        }
    }
}
