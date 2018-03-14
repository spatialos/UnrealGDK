// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

struct FFunctionSignature
{
	FString Type;
	FString Name;
	FString Params;

	FString Declaration()
	{
		return FString::Printf(TEXT("%s %s%s;"), *Type, *Name, *Params);
	}

	FString Definition(const FString& TypeName)
	{
		return FString::Printf(TEXT("%s %s::%s%s"), *Type, *TypeName, *Name, *Params);
	}
};

class FCodeWriter
{
public:
	FCodeWriter();

	template <typename... T>
	FCodeWriter& Printf(const FString& Format, const T&... Args)
	{
		return Print(FString::Printf(*Format, Args...));
	}

	FCodeWriter& Print();
	FCodeWriter& Print(const FString& String);
	FCodeWriter& Indent();
	FCodeWriter& Outdent();
	FCodeWriter& StartScope();
	FCodeWriter& EndScope();

	void WriteToFile(const FString& Filename);
	void Dump();

private:
	FString OutputSource;
	int Scope;
};

// TODO: Roll these into `FCodeWriter`.

FORCEINLINE void StartGeneratingFunction(
	FCodeWriter& SourceWriter,
	FString ReturnType,
	FString FunctionNameAndParams,
	FString TypeBindingName)
{
	SourceWriter.Print();
	SourceWriter.Printf("%s %s::%s", *ReturnType, *TypeBindingName, *FunctionNameAndParams);
	SourceWriter.Print("{");
	SourceWriter.Indent();
}

FORCEINLINE void EndGeneratingFunction(FCodeWriter& SourceWriter)
{
	SourceWriter.EndScope();
}