// Copyright (c) Improbable Worlds Ltd, All Rights Reserved


#include "SpatialTestRPCTimeoutCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

// Sets default values
ARPCTimeoutCharacter::ARPCTimeoutCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	
	const TCHAR* DefaultSkeletalMeshPathString = TEXT("SkeletalMesh'/Engine/EngineMeshes/SkeletalCube.SkeletalCube'");
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>DefaultSkeletalMeshFinder(DefaultSkeletalMeshPathString);
	USkeletalMesh* DefaultSkeletalMesh = DefaultSkeletalMeshFinder.Object;
	checkf(IsValid(DefaultSkeletalMesh), TEXT("Could not find failed material asset %ls"), DefaultSkeletalMeshPathString);

	GetMesh()->SetSkeletalMesh(DefaultSkeletalMesh);

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

												// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
}
