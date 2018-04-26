// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CodeWriter.h"
#include "Misc/FileHelper.h"

FCodeWriter::FCodeWriter()
	: Scope(0)
{
}

FCodeWriter& FCodeWriter::PrintNewLine()
{
	OutputSource += TEXT("\r\n");
	return *this;
}

FCodeWriter& FCodeWriter::Print(const FString& String)
{
	TArray<FString> Lines;
	String.Replace(TEXT("\r\n"), TEXT("\n")).ParseIntoArray(Lines, TEXT("\n"), false);

	if (Lines.Num() == 0)
	{
		return *this;
	}

	// Remove first line if empty.
	if (Lines[0].IsEmpty())
	{
		// Early exit if we have no other.
		if (Lines.Num() == 1)
		{
			return *this;
		}
		Lines.RemoveAt(0);
	}

	// Replace 4 spaces with tabs.
	for (auto& Line : Lines)
	{
		Line.Replace(TEXT("    "), TEXT("\t"));
	}

	// Determine scope to trim by.
	int TrimScope = 0;
	for (int i = 0; i < Lines[0].Len(); ++i)
	{
		if (Lines[0][i] != '\t')
		{
			TrimScope = i;
			break;
		}
	}

	// Add lines to output.
	for (auto& Line : Lines)
	{
		FString TrimmedLine = Line.Mid(TrimScope);
		if (!TrimmedLine.IsEmpty())
		{
			FString ScopeIndent;
			for (int ScopeLevel = 0; ScopeLevel < Scope; ++ScopeLevel)
			{
				ScopeIndent += FString(TEXT("\t"));
			}
			TrimmedLine = ScopeIndent + TrimmedLine;
		}
		OutputSource += TrimmedLine + TEXT("\r\n");
	}

	return *this;
}

FCodeWriter& FCodeWriter::Indent()
{
	Scope++;
	return *this;
}

FCodeWriter& FCodeWriter::Outdent()
{
	check(Scope > 0);
	Scope--;
	return *this;
}

FCodeWriter& FCodeWriter::BeginScope()
{
	Print("{");
	Indent();
	return *this;
}

FCodeWriter& FCodeWriter::BeginFunction(const FFunctionSignature& Signature)
{
	Print(Signature.Definition());
	BeginScope();
	return *this;
}

FCodeWriter& FCodeWriter::BeginFunction(const FFunctionSignature& Signature, const FString& TypeName)
{
	Print(Signature.Definition(TypeName));
	BeginScope();
	return *this;
}

FCodeWriter& FCodeWriter::End()
{
	Outdent();
	Print("}");
	return *this;
}

void FCodeWriter::WriteToFile(const FString& Filename)
{
	check(Scope == 0);
	FFileHelper::SaveStringToFile(OutputSource, *Filename);
}

void FCodeWriter::Dump()
{
	UE_LOG(LogTemp, Warning, TEXT("%s"), *OutputSource);
}
