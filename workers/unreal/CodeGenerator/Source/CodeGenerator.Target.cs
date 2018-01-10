
using UnrealBuildTool;
using System.Collections.Generic;

public class CodeGeneratorTarget : TargetRules
{
	public CodeGeneratorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		ExtraModuleNames.Add("CodeGenerator");
	}

	public override void SetupGlobalEnvironment(
		TargetInfo Target,
		ref LinkEnvironmentConfiguration OutLinkEnvironmentConfiguration,
		ref CPPEnvironmentConfiguration OutCPPEnvironmentConfiguration
		)
	{
		bBuildEditor = false;
		bBuildWithEditorOnlyData = false;
	}
}
