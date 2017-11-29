// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Commandlets/Commandlet.h"
#include "GenerateSchemaCommandlet.generated.h"

UCLASS()
class UGenerateSchemaCommandlet : public UCommandlet {
	GENERATED_BODY()
public:
	UGenerateSchemaCommandlet();
	~UGenerateSchemaCommandlet();

	virtual int32 Main(const FString& Params) override;
};

class CodeWriter
{
public:
	CodeWriter();
	CodeWriter& Print();
	CodeWriter& Print(const FString& String);
		
	void WriteToFile(const FString& Filename);
	void Dump();
	CodeWriter& Indent();
	CodeWriter& Outdent();

private:
	FString OutputSource;
	int Scope;
};

