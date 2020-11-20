// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

struct FFunctionSignature
{
	FString Type;
	FString NameAndParams;

	FString Declaration() const { return FString::Printf(TEXT("%s %s;"), *Type, *NameAndParams); }

	FString Definition() const { return FString::Printf(TEXT("%s %s"), *Type, *NameAndParams); }

	FString Definition(const FString& TypeName) const { return FString::Printf(TEXT("%s %s::%s"), *Type, *TypeName, *NameAndParams); }
};

class FCodeWriter
{
public:
	FCodeWriter();

	template <typename... T>
	FCodeWriter& Printf(const FString& Format, const T&... Args)
	{
		return Print(FString::Format(*Format, TArray<FStringFormatArg>{ Args... }));
	}

	FCodeWriter& PrintNewLine();
	FCodeWriter& Print(const FString& String);
	FCodeWriter& Indent();
	FCodeWriter& Outdent();

	FCodeWriter& BeginScope();
	FCodeWriter& BeginFunction(const FFunctionSignature& Signature);
	FCodeWriter& BeginFunction(const FFunctionSignature& Signature, const FString& TypeName);
	FCodeWriter& End();

	void WriteToFile(const FString& Filename);
	void Dump();

	FCodeWriter(const FCodeWriter& other) = delete;
	FCodeWriter& operator=(const FCodeWriter& other) = delete;

private:
	FString OutputSource;
	int Scope;
};
