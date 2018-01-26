// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CodeWriter.h"
#include "Misc/FileHelper.h"

FCodeWriter::FCodeWriter() : Scope(0)
{
}

FCodeWriter& FCodeWriter::Print()
{
	OutputSource += TEXT("\n");
	return *this;
}

FCodeWriter& FCodeWriter::Print(const FString& String)
{
	TArray<FString> Lines;
	String.ParseIntoArray(Lines, TEXT("\n"), false);

	// Remove first line if empty.
	if (Lines[0].IsEmpty())
	{
		Lines.RemoveAt(0);
	}

	// Early exit if we have no more text.
	if (Lines.Num() == 0)
	{
		return *this;
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
		FString ScopeIdent;
		for (int ScopeLevel = 0; ScopeLevel < Scope; ++ScopeLevel)
		{
			ScopeIdent += FString(TEXT("\t"));
		}
		OutputSource += ScopeIdent + Line.Mid(TrimScope) + TEXT("\n");
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

void FCodeWriter::WriteToFile(const FString& Filename)
{
	check(Scope == 0);
	FFileHelper::SaveStringToFile(OutputSource, *Filename);
}

void FCodeWriter::Dump() {
	UE_LOG(LogTemp, Warning, TEXT("%s"), *OutputSource);
}
