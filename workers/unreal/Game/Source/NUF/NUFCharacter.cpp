// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "NUFCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PossessPawnRequest.h"
#include "PossessPawnResponse.h"
#include "NUFGameStateBase.h"
#include "NUF/SpatialNetDriver.h"
#include "VehicleCppPawn.h"

#include "UnrealNetwork.h"

//////////////////////////////////////////////////////////////////////////
// ANUFCharacter

ANUFCharacter::ANUFCharacter()
{
	// Hack to ensure that the game state is created and set to tick on a client as we don't replicate it
	UWorld* World = GetWorld();
	if (World && World->GetGameState() == nullptr)
	{
		AGameStateBase* GameState = World->SpawnActor<AGameStateBase>(ANUFGameStateBase::StaticClass());
		World->SetGameState(GameState);
		Cast<ANUFGameStateBase>(GameState)->FakeServerHasBegunPlay();
	}

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

												// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	PossessPawnComponent = CreateDefaultSubobject<UPossessPawnComponent>(TEXT("PossessPawn"));
   	OnPossessPawnAckDelegate.BindUFunction(this, "OnPossessPawnRequestAck");

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

void ANUFCharacter::BeginPlay()
{
	Super::BeginPlay();

	USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(GetWorld()->GetNetDriver());
	if (SpatialNetDriver != nullptr) {
		EntityRegistry = SpatialNetDriver->GetEntityRegistry();
		Commander = NewObject<UCommander>(this, UCommander::StaticClass(), TEXT("NUFCharacterCommander"))->Init(nullptr, SpatialNetDriver->GetSpatialOS()->GetConnection(), SpatialNetDriver->GetSpatialOS()->GetView());
		auto View = SpatialNetDriver->GetSpatialOS()->GetView();
		auto Connection = SpatialNetDriver->GetSpatialOS()->GetConnection();

		if (PossessPawnComponent) {
			PossessPawnComponent->OnPossessPawnCommandRequest.AddDynamic(this, &ANUFCharacter::OnPossessPawnRequest);
		}
	}
}

void ANUFCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

//////////////////////////////////////////////////////////////////////////
// Input

void ANUFCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &ANUFCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ANUFCharacter::MoveRight);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ANUFCharacter::Interact);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ANUFCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ANUFCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ANUFCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ANUFCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ANUFCharacter::OnResetVR);
}

void ANUFCharacter::Interact() {
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("On screen message from Character"));
}

void ANUFCharacter::OnPossessPawnRequest(UPossessPawnCommandResponder* Responder)
{
	UE_LOG(LogTemp, Warning, TEXT("Command recieved"));
	FEntityId CarId = Responder->GetRequest()->GetPawnId();
	AVehicleCppPawn* Car = Cast<AVehicleCppPawn>(EntityRegistry->GetActorFromEntityId(CarId));
	GetController()->Possess(Car);

	UPossessPawnResponse* Response = NewObject<UPossessPawnResponse>(this);
	Responder->SendResponse(Response);
}

void ANUFCharacter::OnPossessPawnRequestAck(const FSpatialOSCommandResult& Result, UPossessPawnResponse* Response) {
	if (Result.StatusCode != ECommandResponseCode::Success) {
		UE_LOG(LogTemp, Warning,
			TEXT("PossessPawn command failed from entity %d with message %s"),
			PossessPawnComponent->GetEntityId(), *Result.ErrorMessage);
	}
}

void ANUFCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void ANUFCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}

void ANUFCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
}

void ANUFCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ANUFCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ANUFCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ANUFCharacter::MoveRight(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}
