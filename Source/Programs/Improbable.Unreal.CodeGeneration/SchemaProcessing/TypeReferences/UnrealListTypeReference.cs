using System.Collections.Generic;

namespace Improbable.Unreal.CodeGeneration.SchemaProcessing.TypeReferences
{
    public sealed class UnrealListTypeReference : UnrealTypeReference
    {
        public readonly IUnrealTypeReference UnderlyingValueType;

        public UnrealListTypeReference(IUnrealTypeReference containedTypeReference)
        {
            ReferenceType = ReferenceType.List;
            UnderlyingValueType = containedTypeReference;

            var className = string.Format("{0}List", containedTypeReference.UnderlyingCapitalisedName);
            UnrealType = string.Format("U{0}*", className);
            UnderlyingQualifiedName = containedTypeReference.UnderlyingQualifiedName;
            UnderlyingCapitalisedName = containedTypeReference.UnderlyingCapitalisedName;
            ConvertUnderlyingValueToUnrealMemberVariable = (cppList) =>
            {
                // Set owning object as new uobject's outer.
                return string.Format("NewObject<U{0}>(this)->Init({1})", className, cppList);
            };
            AssignUnderlyingValueToUnrealMemberVariable = (capitalizedName, cppList) =>
            {
                return string.Format(@"if ({0} == nullptr) {{ {0} = NewObject<U{1}>(this); }} {0}->Init({2})",
                                     capitalizedName,
                                     className,
                                     cppList);
            };
            CheckInequality = (capitalizedName, compName) => { return string.Format("{0} && (*{0} != {1})", capitalizedName, compName); };
            ConvertUnderlyingValueToUnrealLocalVariable = (cppList) =>
            {
                // Set static instance package as the uobject's outer.
                return string.Format("NewObject<U{0}>()->Init({1})", className, cppList);
            };
            ConvertUnrealValueToSnapshotValue = (VarName) => { return ConvertUnrealValueToUnderlyingValue(VarName); };
            ConvertUnrealValueToUnderlyingValue = (cppList) => { return string.Format("{0}->GetUnderlying()", cppList); };

            RequiredIncludes = new List<string>();
            RequiredIncludes.Add(string.Format("\"{0}.h\"", className));
            RequiredIncludes.AddRange(containedTypeReference.RequiredIncludes);

            DefaultInitialisationString = string.Format("worker::List<{0}>()", UnderlyingQualifiedName);
            ArgumentName = UnrealType;
            SnapshotType = string.Format("worker::List<{0}>", UnderlyingQualifiedName);
            UClassName = string.Format("U{0}", className);
            DefaultValue = "nullptr";
        }
    }
}
