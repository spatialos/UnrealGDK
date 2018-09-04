using System;
using System.Collections.Generic;

namespace Improbable.Unreal.CodeGeneration.SchemaProcessing.TypeReferences
{
    public abstract class UnrealTypeReference : IUnrealTypeReference
    {
        public ReferenceType ReferenceType { get; protected set; }
        public Func<string, string, string> AssignUnderlyingValueToUnrealMemberVariable { get; protected set; }

        public Func<string, string, string> CheckInequality { get; protected set; }

        public Func<string, string> ConvertUnderlyingValueToUnrealLocalVariable { get; protected set; }

        public Func<string, string> ConvertUnderlyingValueToUnrealMemberVariable { get; protected set; }

        public Func<string, string> ConvertUnrealValueToSnapshotValue { get; protected set; }

        public Func<string, string> ConvertUnrealValueToUnderlyingValue { get; protected set; }

        public List<string> RequiredIncludes { get; protected set; }

        public string UnderlyingCapitalisedName { get; protected set; }

        public string UnderlyingQualifiedName { get; protected set; }

        public string UnrealType { get; protected set; }

        public virtual string DefaultInitialisationString { get; protected set; }

        public string ArgumentName { get; protected set; }

        public string SnapshotType { get; protected set; }

        public string UClassName { get; protected set; }

        public string CapitalisedName { get; protected set; }

        public string DefaultValue { get; protected set; }
    }
}
