﻿//------------------------------------------------------------------------------
// <auto-generated>
//     This code was generated by a tool.
//     Runtime Version:4.0.30319.42000
//
//     Changes to this file may cause incorrect behavior and will be lost if
//     the code is regenerated.
// </auto-generated>
//------------------------------------------------------------------------------

namespace Improbable.Unreal.CodeGeneration {
    using System.Collections.Generic;
    using System.Linq;
    using System;
    using Improbable.CodeGeneration.Jobs;
    
    
    public partial class UnrealComponentHeaderGenerator : UnrealComponentHeaderGeneratorBase {
        
        public virtual string TransformText() {
            this.GenerationEnvironment = null;
            
            #line 4 "Templates\UnrealComponentHeaderGenerator.tt"



            
            #line default
            #line hidden
            
            #line 7 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(@"// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#pragma once

#include ""SpatialOsComponent.h""
#include ""CoreMinimal.h""
#include ""UObject/ObjectMacros.h""
#include """);
            
            #line default
            #line hidden
            
            #line 17 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture( unrealComponent.CapitalisedName ));
            
            #line default
            #line hidden
            
            #line 17 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("ComponentUpdate.h\"\r\n#include \"");
            
            #line default
            #line hidden
            
            #line 18 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture( unrealComponent.CapitalisedName ));
            
            #line default
            #line hidden
            
            #line 18 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("AddComponentOp.h\"\r\n#include \"ComponentId.h\"\r\n#include \"SpatialOSViewTypes.h\"\r\n#in" +
                    "clude \"SpatialOSWorkerTypes.h\"\r\n#include \"");
            
            #line default
            #line hidden
            
            #line 22 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture( unrealComponent.UnderlyingPackageDetails.Include ));
            
            #line default
            #line hidden
            
            #line 22 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("\"\r\n\r\n");
            
            #line default
            #line hidden
            
            #line 24 "Templates\UnrealComponentHeaderGenerator.tt"
var requiredIncludes = unrealComponent.FieldDetailsList.SelectMany(fieldDetails => { return fieldDetails.TypeReference.RequiredIncludes; })
														 .Where(include => include != null)
														 .Distinct();
foreach (var requiredInclude in requiredIncludes) { 
            
            #line default
            #line hidden
            
            #line 28 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("#include ");
            
            #line default
            #line hidden
            
            #line 28 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture( requiredInclude ));
            
            #line default
            #line hidden
            
            #line 28 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("\r\n");
            
            #line default
            #line hidden
            
            #line 29 "Templates\UnrealComponentHeaderGenerator.tt"
 } 
            
            #line default
            #line hidden
            
            #line 30 "Templates\UnrealComponentHeaderGenerator.tt"
 foreach (var commandDetails in unrealComponent.CommandDetailsList) { 
            
            #line default
            #line hidden
            
            #line 31 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("#include \"");
            
            #line default
            #line hidden
            
            #line 31 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture( commandDetails.CapitalisedName ));
            
            #line default
            #line hidden
            
            #line 31 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("CommandResponder.h\"\r\n");
            
            #line default
            #line hidden
            
            #line 32 "Templates\UnrealComponentHeaderGenerator.tt"
 } 
            
            #line default
            #line hidden
            
            #line 33 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("\r\n");
            
            #line default
            #line hidden
            
            #line 34 "Templates\UnrealComponentHeaderGenerator.tt"
 foreach (var eventDetails in unrealComponent.EventDetailsList) { 
            
            #line default
            #line hidden
            
            #line 35 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("#include \"");
            
            #line default
            #line hidden
            
            #line 35 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture( eventDetails.EventTypeReference.CapitalisedName ));
            
            #line default
            #line hidden
            
            #line 35 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(".h\"\r\n");
            
            #line default
            #line hidden
            
            #line 36 "Templates\UnrealComponentHeaderGenerator.tt"
 } 
            
            #line default
            #line hidden
            
            #line 37 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("\r\n#include \"");
            
            #line default
            #line hidden
            
            #line 38 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture( unrealComponent.CapitalisedName ));
            
            #line default
            #line hidden
            
            #line 38 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("Component.generated.h\"\r\n\r\nclass UCallbackDispatcher;\r\nclass UComponentUpdateOpWra" +
                    "pperBase;\r\n\r\nUSTRUCT()\r\nstruct F");
            
            #line default
            #line hidden
            
            #line 44 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture( unrealComponent.CapitalisedName ));
            
            #line default
            #line hidden
            
            #line 44 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("ComponentSnapshot\r\n{\r\n\tGENERATED_BODY()\r\n\r\n\tF");
            
            #line default
            #line hidden
            
            #line 48 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture( unrealComponent.CapitalisedName ));
            
            #line default
            #line hidden
            
            #line 48 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("ComponentSnapshot()");
            
            #line default
            #line hidden
            
            #line 48 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture( FillSnapshotInitializerList() ));
            
            #line default
            #line hidden
            
            #line 48 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("\r\n\t{\r\n\t}\r\n\r\n");
            
            #line default
            #line hidden
            
            #line 52 "Templates\UnrealComponentHeaderGenerator.tt"
 foreach (var fieldDetails in unrealComponent.FieldDetailsList) { 
            
            #line default
            #line hidden
            
            #line 53 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("\t");
            
            #line default
            #line hidden
            
            #line 53 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture( fieldDetails.TypeReference.SnapshotType + " " + fieldDetails.CapitalisedName ));
            
            #line default
            #line hidden
            
            #line 53 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(";\r\n");
            
            #line default
            #line hidden
            
            #line 54 "Templates\UnrealComponentHeaderGenerator.tt"
 } 
            
            #line default
            #line hidden
            
            #line 55 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("};\r\n\r\nUCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))\r\nclass " +
                    "SPATIALOS_API U");
            
            #line default
            #line hidden
            
            #line 58 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture( unrealComponent.CapitalisedName ));
            
            #line default
            #line hidden
            
            #line 58 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("Component : public USpatialOsComponent\r\n{\r\n    GENERATED_BODY()\r\n\r\npublic:\r\n    U" +
                    "");
            
            #line default
            #line hidden
            
            #line 63 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture( unrealComponent.CapitalisedName ));
            
            #line default
            #line hidden
            
            #line 63 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("Component();\r\n\tvirtual ~U");
            
            #line default
            #line hidden
            
            #line 64 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture( unrealComponent.CapitalisedName ));
            
            #line default
            #line hidden
            
            #line 64 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("Component() override = default;\r\n\r\n    virtual void BeginDestroy() override;\r\n\r\n " +
                    "   UFUNCTION(BlueprintPure, Category = \"");
            
            #line default
            #line hidden
            
            #line 68 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture( unrealComponent.CapitalisedName ));
            
            #line default
            #line hidden
            
            #line 68 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(@"Component"")
    FComponentId GetComponentId();

    const static worker::ComponentId ComponentId;

    void Init(const TWeakPtr<SpatialOSConnection>& InConnection, const TWeakPtr<SpatialOSView>& InView, worker::EntityId InEntityId, UCallbackDispatcher* InCallbackDispatcher) override;

	void Disable(const worker::EntityId InEntityId, UCallbackDispatcher* CallbackDispatcher);

	void ApplyInitialState(const UAddComponentOpWrapperBase& Op) override;

	DEPRECATED(12.1, ""Please use TriggerAutomaticComponentUpdate."")
	void ReplicateChanges(float DeltaSeconds) override;

	void TriggerAutomaticComponentUpdate(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = """);
            
            #line default
            #line hidden
            
            #line 84 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture( unrealComponent.CapitalisedName ));
            
            #line default
            #line hidden
            
            #line 84 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("Component\")\r\n\tvoid TriggerManualComponentUpdate();\r\n\r\n");
            
            #line default
            #line hidden
            
            #line 87 "Templates\UnrealComponentHeaderGenerator.tt"
 foreach (var fieldDetails in unrealComponent.FieldDetailsList) { 
            
            #line default
            #line hidden
            
            #line 88 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("    UPROPERTY(BlueprintAssignable, Category = \"");
            
            #line default
            #line hidden
            
            #line 88 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture( unrealComponent.CapitalisedName ));
            
            #line default
            #line hidden
            
            #line 88 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("Component\")\r\n    FSpatialComponentUpdated On");
            
            #line default
            #line hidden
            
            #line 89 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture( fieldDetails.CapitalisedName ));
            
            #line default
            #line hidden
            
            #line 89 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("Update;\r\n\r\n");
            
            #line default
            #line hidden
            
            #line 91 "Templates\UnrealComponentHeaderGenerator.tt"
 } 
            
            #line default
            #line hidden
            
            #line 92 "Templates\UnrealComponentHeaderGenerator.tt"
 foreach (var eventDetails in unrealComponent.EventDetailsList) { 
            
            #line default
            #line hidden
            
            #line 93 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("    UPROPERTY(BlueprintAssignable, Category = \"");
            
            #line default
            #line hidden
            
            #line 93 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture( unrealComponent.CapitalisedName ));
            
            #line default
            #line hidden
            
            #line 93 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("Component\")\r\n    F");
            
            #line default
            #line hidden
            
            #line 94 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture( eventDetails.EventTypeReference.CapitalisedName ));
            
            #line default
            #line hidden
            
            #line 94 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("Delegate On");
            
            #line default
            #line hidden
            
            #line 94 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture( eventDetails.CapitalisedName ));
            
            #line default
            #line hidden
            
            #line 94 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(";\r\n\tUFUNCTION(BlueprintCallable, Category = \"");
            
            #line default
            #line hidden
            
            #line 95 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture( unrealComponent.CapitalisedName ));
            
            #line default
            #line hidden
            
            #line 95 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("Component\")\r\n");
            
            #line default
            #line hidden
            
            #line 96 "Templates\UnrealComponentHeaderGenerator.tt"
 if (eventDetails.NumTypeFields() > 0) { 
            
            #line default
            #line hidden
            
            #line 97 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("    void ");
            
            #line default
            #line hidden
            
            #line 97 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture( eventDetails.CapitalisedName ));
            
            #line default
            #line hidden
            
            #line 97 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("(");
            
            #line default
            #line hidden
            
            #line 97 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture( eventDetails.EventTypeReference.ArgumentName ));
            
            #line default
            #line hidden
            
            #line 97 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(" Data);\r\n");
            
            #line default
            #line hidden
            
            #line 98 "Templates\UnrealComponentHeaderGenerator.tt"
 } 
            
            #line default
            #line hidden
            
            #line 99 "Templates\UnrealComponentHeaderGenerator.tt"
 else { 
            
            #line default
            #line hidden
            
            #line 100 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("\tvoid ");
            
            #line default
            #line hidden
            
            #line 100 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture( eventDetails.CapitalisedName ));
            
            #line default
            #line hidden
            
            #line 100 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("();\r\n");
            
            #line default
            #line hidden
            
            #line 101 "Templates\UnrealComponentHeaderGenerator.tt"
 } 
            
            #line default
            #line hidden
            
            #line 102 "Templates\UnrealComponentHeaderGenerator.tt"
 } 
            
            #line default
            #line hidden
            
            #line 103 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("\r\n");
            
            #line default
            #line hidden
            
            #line 104 "Templates\UnrealComponentHeaderGenerator.tt"
 foreach (var commandDetails in unrealComponent.CommandDetailsList) { 
            
            #line default
            #line hidden
            
            #line 105 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("    UPROPERTY(BlueprintAssignable, Category = \"");
            
            #line default
            #line hidden
            
            #line 105 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture( unrealComponent.CapitalisedName ));
            
            #line default
            #line hidden
            
            #line 105 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("Component\")\r\n    ");
            
            #line default
            #line hidden
            
            #line 106 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture( commandDetails.UnrealCommandDelegateName ));
            
            #line default
            #line hidden
            
            #line 106 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(" On");
            
            #line default
            #line hidden
            
            #line 106 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture( commandDetails.UnderlyingCapitalisedName ));
            
            #line default
            #line hidden
            
            #line 106 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("CommandRequest;\r\n");
            
            #line default
            #line hidden
            
            #line 107 "Templates\UnrealComponentHeaderGenerator.tt"
 } 
            
            #line default
            #line hidden
            
            #line 108 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(@"
	DEPRECATED(12.1, ""This function is deprecated, the component updates are now sent automatically. Use MaxUpdatesPerSecond property to control the update frequency, or set it to 0 and use TriggerManualComponentUpdate for more control."")
    UFUNCTION(BlueprintCallable, Category = """);
            
            #line default
            #line hidden
            
            #line 110 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture( unrealComponent.CapitalisedName ));
            
            #line default
            #line hidden
            
            #line 110 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(@"Component"", meta=(DeprecatedFunction, DeprecationMessage=""This function is deprecated, the component updates are now sent automatically. Use MaxUpdatesPerSecond property to control the update frequency, or set it to 0 and use TriggerManualComponentUpdate for more control.""))
    void SendComponentUpdate(U");
            
            #line default
            #line hidden
            
            #line 111 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture( unrealComponent.CapitalisedName ));
            
            #line default
            #line hidden
            
            #line 111 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("ComponentUpdate* update);\r\n\r\n    UPROPERTY(BlueprintAssignable, Category = \"");
            
            #line default
            #line hidden
            
            #line 113 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture( unrealComponent.CapitalisedName ));
            
            #line default
            #line hidden
            
            #line 113 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("Component\")\r\n    FSpatialComponentUpdated OnComponentUpdate;\r\n\r\n");
            
            #line default
            #line hidden
            
            #line 116 "Templates\UnrealComponentHeaderGenerator.tt"
 foreach (var fieldDetails in unrealComponent.FieldDetailsList) { 
            
            #line default
            #line hidden
            
            #line 117 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("\tDEPRECATED(12.0, \"This function is deprecated, access the ");
            
            #line default
            #line hidden
            
            #line 117 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture( fieldDetails.CapitalisedName ));
            
            #line default
            #line hidden
            
            #line 117 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(" property directly.\")\r\n    UFUNCTION(BlueprintPure, Category = \"");
            
            #line default
            #line hidden
            
            #line 118 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture( unrealComponent.CapitalisedName ));
            
            #line default
            #line hidden
            
            #line 118 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("Component\", meta=(DeprecatedFunction, DeprecationMessage=\"This function is deprec" +
                    "ated, access the ");
            
            #line default
            #line hidden
            
            #line 118 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture( fieldDetails.CapitalisedName ));
            
            #line default
            #line hidden
            
            #line 118 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(" property directly.\"))\r\n    ");
            
            #line default
            #line hidden
            
            #line 119 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture( fieldDetails.TypeReference.UnrealType ));
            
            #line default
            #line hidden
            
            #line 119 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(" Get");
            
            #line default
            #line hidden
            
            #line 119 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture( fieldDetails.CapitalisedName ));
            
            #line default
            #line hidden
            
            #line 119 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("();\r\n");
            
            #line default
            #line hidden
            
            #line 120 "Templates\UnrealComponentHeaderGenerator.tt"
 } 
            
            #line default
            #line hidden
            
            #line 121 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("\r\n");
            
            #line default
            #line hidden
            
            #line 122 "Templates\UnrealComponentHeaderGenerator.tt"
 foreach (var fieldDetails in unrealComponent.FieldDetailsList) { 
            
            #line default
            #line hidden
            
            #line 123 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("\tUPROPERTY(EditAnywhere, BlueprintReadWrite)\r\n    ");
            
            #line default
            #line hidden
            
            #line 124 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture( fieldDetails.TypeReference.UnrealType ));
            
            #line default
            #line hidden
            
            #line 124 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(" ");
            
            #line default
            #line hidden
            
            #line 124 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture( fieldDetails.CapitalisedName ));
            
            #line default
            #line hidden
            
            #line 124 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(";\r\n");
            
            #line default
            #line hidden
            
            #line 125 "Templates\UnrealComponentHeaderGenerator.tt"
 } 
            
            #line default
            #line hidden
            
            #line 126 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("\r\nprivate:\r\n\tvoid GenerateSnapshot();\r\n\r\n\tUPROPERTY()\r\n\tU");
            
            #line default
            #line hidden
            
            #line 131 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture( unrealComponent.CapitalisedName ));
            
            #line default
            #line hidden
            
            #line 131 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("ComponentUpdate* ComponentUpdater;\r\n\r\n");
            
            #line default
            #line hidden
            
            #line 133 "Templates\UnrealComponentHeaderGenerator.tt"
 foreach (var eventDetails in unrealComponent.EventDetailsList) { 
            
            #line default
            #line hidden
            
            #line 134 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("\tUPROPERTY()\r\n\t");
            
            #line default
            #line hidden
            
            #line 135 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture( eventDetails.EventTypeReference.UnrealType ));
            
            #line default
            #line hidden
            
            #line 135 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(" ");
            
            #line default
            #line hidden
            
            #line 135 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture( eventDetails.CapitalisedName ));
            
            #line default
            #line hidden
            
            #line 135 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("Wrapper;\r\n\t");
            
            #line default
            #line hidden
            
            #line 136 "Templates\UnrealComponentHeaderGenerator.tt"
 } 
            
            #line default
            #line hidden
            
            #line 137 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("\r\n\tF");
            
            #line default
            #line hidden
            
            #line 138 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture( unrealComponent.CapitalisedName ));
            
            #line default
            #line hidden
            
            #line 138 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(@"ComponentSnapshot* Snapshot;

    void OnComponentUpdateDispatcherCallback(UComponentUpdateOpWrapperBase& Op);

	void OnAuthorityChangeDispatcherCallback(const worker::AuthorityChangeOp& Op) override;

	void ReplicateChangesInternal();
	void SendComponentUpdateInternal(U");
            
            #line default
            #line hidden
            
            #line 145 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture( unrealComponent.CapitalisedName ));
            
            #line default
            #line hidden
            
            #line 145 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("ComponentUpdate* update);\r\n    void ApplyComponentUpdate(const ");
            
            #line default
            #line hidden
            
            #line 146 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture( unrealComponent.UnderlyingQualifiedName ));
            
            #line default
            #line hidden
            
            #line 146 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("::Update& update);\r\n\tvoid NotifyUpdate(const ");
            
            #line default
            #line hidden
            
            #line 147 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture( unrealComponent.UnderlyingQualifiedName ));
            
            #line default
            #line hidden
            
            #line 147 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("::Update& update);\r\n\r\n");
            
            #line default
            #line hidden
            
            #line 149 "Templates\UnrealComponentHeaderGenerator.tt"
 foreach (var commandDetails in unrealComponent.CommandDetailsList) { 
            
            #line default
            #line hidden
            
            #line 150 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("\r\n    void On");
            
            #line default
            #line hidden
            
            #line 151 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture( commandDetails.UnderlyingCapitalisedName ));
            
            #line default
            #line hidden
            
            #line 151 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("CommandRequestDispatcherCallback(\r\n        const worker::CommandRequestOp<");
            
            #line default
            #line hidden
            
            #line 152 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture( unrealComponent.UnderlyingQualifiedName ));
            
            #line default
            #line hidden
            
            #line 152 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("::Commands::");
            
            #line default
            #line hidden
            
            #line 152 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(this.ToStringHelper.ToStringWithCulture( commandDetails.UnderlyingCapitalisedName ));
            
            #line default
            #line hidden
            
            #line 152 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write(">& op);\r\n\r\n");
            
            #line default
            #line hidden
            
            #line 154 "Templates\UnrealComponentHeaderGenerator.tt"
 } 
            
            #line default
            #line hidden
            
            #line 155 "Templates\UnrealComponentHeaderGenerator.tt"
            this.Write("};\r\n");
            
            #line default
            #line hidden
            return this.GenerationEnvironment.ToString();
        }
        
        public virtual void Initialize() {
        }
    }
    
    public class UnrealComponentHeaderGeneratorBase {
        
        private global::System.Text.StringBuilder builder;
        
        private global::System.Collections.Generic.IDictionary<string, object> session;
        
        private global::System.CodeDom.Compiler.CompilerErrorCollection errors;
        
        private string currentIndent = string.Empty;
        
        private global::System.Collections.Generic.Stack<int> indents;
        
        private ToStringInstanceHelper _toStringHelper = new ToStringInstanceHelper();
        
        public virtual global::System.Collections.Generic.IDictionary<string, object> Session {
            get {
                return this.session;
            }
            set {
                this.session = value;
            }
        }
        
        public global::System.Text.StringBuilder GenerationEnvironment {
            get {
                if ((this.builder == null)) {
                    this.builder = new global::System.Text.StringBuilder();
                }
                return this.builder;
            }
            set {
                this.builder = value;
            }
        }
        
        protected global::System.CodeDom.Compiler.CompilerErrorCollection Errors {
            get {
                if ((this.errors == null)) {
                    this.errors = new global::System.CodeDom.Compiler.CompilerErrorCollection();
                }
                return this.errors;
            }
        }
        
        public string CurrentIndent {
            get {
                return this.currentIndent;
            }
        }
        
        private global::System.Collections.Generic.Stack<int> Indents {
            get {
                if ((this.indents == null)) {
                    this.indents = new global::System.Collections.Generic.Stack<int>();
                }
                return this.indents;
            }
        }
        
        public ToStringInstanceHelper ToStringHelper {
            get {
                return this._toStringHelper;
            }
        }
        
        public void Error(string message) {
            this.Errors.Add(new global::System.CodeDom.Compiler.CompilerError(null, -1, -1, null, message));
        }
        
        public void Warning(string message) {
            global::System.CodeDom.Compiler.CompilerError val = new global::System.CodeDom.Compiler.CompilerError(null, -1, -1, null, message);
            val.IsWarning = true;
            this.Errors.Add(val);
        }
        
        public string PopIndent() {
            if ((this.Indents.Count == 0)) {
                return string.Empty;
            }
            int lastPos = (this.currentIndent.Length - this.Indents.Pop());
            string last = this.currentIndent.Substring(lastPos);
            this.currentIndent = this.currentIndent.Substring(0, lastPos);
            return last;
        }
        
        public void PushIndent(string indent) {
            this.Indents.Push(indent.Length);
            this.currentIndent = (this.currentIndent + indent);
        }
        
        public void ClearIndent() {
            this.currentIndent = string.Empty;
            this.Indents.Clear();
        }
        
        public void Write(string textToAppend) {
            this.GenerationEnvironment.Append(textToAppend);
        }
        
        public void Write(string format, params object[] args) {
            this.GenerationEnvironment.AppendFormat(format, args);
        }
        
        public void WriteLine(string textToAppend) {
            this.GenerationEnvironment.Append(this.currentIndent);
            this.GenerationEnvironment.AppendLine(textToAppend);
        }
        
        public void WriteLine(string format, params object[] args) {
            this.GenerationEnvironment.Append(this.currentIndent);
            this.GenerationEnvironment.AppendFormat(format, args);
            this.GenerationEnvironment.AppendLine();
        }
        
        public class ToStringInstanceHelper {
            
            private global::System.IFormatProvider formatProvider = global::System.Globalization.CultureInfo.InvariantCulture;
            
            public global::System.IFormatProvider FormatProvider {
                get {
                    return this.formatProvider;
                }
                set {
                    if ((value != null)) {
                        this.formatProvider = value;
                    }
                }
            }
            
            public string ToStringWithCulture(object objectToConvert) {
                if ((objectToConvert == null)) {
                    throw new global::System.ArgumentNullException("objectToConvert");
                }
                global::System.Type type = objectToConvert.GetType();
                global::System.Type iConvertibleType = typeof(global::System.IConvertible);
                if (iConvertibleType.IsAssignableFrom(type)) {
                    return ((global::System.IConvertible)(objectToConvert)).ToString(this.formatProvider);
                }
                global::System.Reflection.MethodInfo methInfo = type.GetMethod("ToString", new global::System.Type[] {
                            iConvertibleType});
                if ((methInfo != null)) {
                    return ((string)(methInfo.Invoke(objectToConvert, new object[] {
                                this.formatProvider})));
                }
                return objectToConvert.ToString();
            }
        }
    }
}
