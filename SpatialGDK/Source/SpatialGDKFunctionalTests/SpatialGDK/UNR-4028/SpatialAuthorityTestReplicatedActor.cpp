// Fill out your copyright notice in the Description page of Project Settings.


#include "SpatialAuthorityTestReplicatedActor.h"
//#include "Components/SceneComponent.h"

// Sets default values
ASpatialAuthorityTestReplicatedActor::ASpatialAuthorityTestReplicatedActor()
	: Super()
{
	bReplicates = true;
}

//void ASpatialAuthorityTestActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
//{
//	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
//}

//// Called when the game starts or when spawned
//void ASpatialAuthorityTestReplicatedActor::BeginPlay()
//{
//	Super::BeginPlay();
//	
//	if( OwnerTest == nullptr )
//	{
//		ensureMsgf(false, TEXT("AuthoritySpatialTestActor needs Test to be set"));
//		return;
//	}
//
//	if (HasAuthority())
//	{
//		AuthorityOnBeginPlay = OwnerTest->GetLocalFlowController();
//	}
//}
//
//// Called every frame
//void ASpatialAuthorityTestReplicatedActor::Tick(float DeltaTime)
//{
//	Super::Tick(DeltaTime);
//
//}

