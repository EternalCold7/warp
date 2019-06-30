// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "MyProjectCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Runtime/Engine/Public/TimerManager.h"

#include "NPC.h"
#include "Classes/Components/StaticMeshComponent.h"
#include "Classes/Components/SkeletalMeshComponent.h"
#include "ColisionStaticMeshComponent.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "WarpComponent.h"
#include "Runtime/Engine/Classes/Animation/SkeletalMeshActor.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/Particles/ParticleSystemComponent.h"
//////////////////////////////////////////////////////////////////////////
// AMyProjectCharacter

AMyProjectCharacter::AMyProjectCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	
	GetCharacterMovement()->bOrientRotationToMovement = true; 
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); 
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; 
	CameraBoom->bUsePawnControlRotation = true; 

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); 
	FollowCamera->bUsePawnControlRotation = false; 
	
	m_Sword = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("sword"));
	m_Sword->SetupAttachment(GetMesh(), FName("SwordSocket"));
	m_Sword->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, FName("SwordSocket"));

	m_OverlapingMesh = CreateDefaultSubobject<UColisionStaticMeshComponent>("overlap mesh");
	m_Sword->bEditableWhenInherited = true;

	m_WarpComponent = CreateDefaultSubobject<UWarpComponent>("WarpComponent");
	m_WarpComponent->m_ProjectCharacter = this;

	
	m_WarpComponent->AttachToComponent(GetRootComponent(),FAttachmentTransformRules::KeepRelativeTransform);
	
	WallRunningTickFunction.BindUFunction(this, FName("WallRunningTick"));
	

	m_WallRunTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("WallRunTimeline"));

	m_WallCollisionCapsule = CreateDefaultSubobject<UCapsuleComponent>("WallColisionCapsule");
	m_WallCollisionCapsule->AttachToComponent(GetRootComponent(),FAttachmentTransformRules::KeepRelativeTransform);
	m_WallCollisionCapsule->OnComponentBeginOverlap.AddDynamic(this,&AMyProjectCharacter::OnCapsuleBeginOverlap);
	m_WallCollisionCapsule->OnComponentEndOverlap.AddDynamic(this,&AMyProjectCharacter::OnCapsuleEndOverlap);
}

//////////////////////////////////////////////////////////////////////////
// Input

void AMyProjectCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMyProjectCharacter::DoubleJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Warp", IE_Pressed, this, &AMyProjectCharacter::Warp);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMyProjectCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMyProjectCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AMyProjectCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMyProjectCharacter::LookUpAtRate);

	PlayerInputComponent->BindTouch(IE_Pressed, this, &AMyProjectCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AMyProjectCharacter::TouchStopped);

	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AMyProjectCharacter::OnResetVR);
}





void AMyProjectCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AMyProjectCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void AMyProjectCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void AMyProjectCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}



void AMyProjectCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AMyProjectCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		if (!CanMove)
			return;
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AMyProjectCharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		if (!CanMove)
			return;
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}


void AMyProjectCharacter::BeginPlay() {
	Super::BeginPlay();

	if (m_WallRunningCurve) {
		m_WallRunTimeline->AddInterpFloat(m_WallRunningCurve, WallRunningTickFunction, FName("WallRun"));
		m_WallRunTimeline->SetLooping(false);
		m_WallRunTimeline->SetIgnoreTimeDilation(true);
	}
}







void AMyProjectCharacter::Warp()
{
	if (!CanWarp)
		return;
	CanWarp = false;
	IsInWarp = true;
	CanMove = false;
	
	m_WarpComponent->Warp();
}
void AMyProjectCharacter::ChangeFlagsAfterAnimation() {
	
	CanWarp = true;
	IsInWarp = false;
	CanMove = true;
}


void AMyProjectCharacter::DoubleJump()
{
	if (CurrentJumsCount <= 1) {
		ACharacter::LaunchCharacter(FVector(0,0,JumpHeight),false,true);
		CurrentJumsCount++;
		isJumping = true;
	}
}

void AMyProjectCharacter::Landed(const FHitResult& hit)
{
	Super::Landed(hit);
	CurrentJumsCount = 0;
	isJumping = false;
	IsOnWall = false;
}

void AMyProjectCharacter::Tick(float delta)
{
	Super::Tick(delta);
	if (GetMesh()->GlobalAnimRateScale > 0.9f)
		isJumping = false;
}


void AMyProjectCharacter::OnCapsuleEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	m_WallRunTimeline->Deactivate();
	if (OtherActor->ActorHasTag("RunWall")) {
		GetCharacterMovement()->GravityScale = 1.f;
		GetCharacterMovement()->SetPlaneConstraintNormal({ 0,0,0 });
		IsOnWall = false;
	}


}
void AMyProjectCharacter::WallRunningTick(float delta)
{
	if (UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetInputKeyTimeDown(FKey("Spacebar")) > 0.f) {
		if (IsOnWall) {
			
			GetCharacterMovement()->GravityScale = 0.3f;
			GetCharacterMovement()->SetPlaneConstraintNormal(FVector(0.f, 0.f, 1.f));
			GetCharacterMovement()->AddForce(m_PlayerDirection * 2000);
			return;
		}
		

	}
	
	GetCharacterMovement()->GravityScale = 1.f;
	GetCharacterMovement()->SetPlaneConstraintNormal({0,0,0});
	IsOnWall = false;
	

}

void AMyProjectCharacter::OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

	
	m_PlayerDirection = GetFollowCamera()->GetForwardVector();
	bool canRun = OtherActor->ActorHasTag(FName("RunWall")) ;
	bool can = GetCharacterMovement()->IsFalling();
	if (canRun && can) {
		IsOnWall = true;
		m_WallRunTimeline->PlayFromStart();
		UE_LOG(LogTemp, Warning, TEXT("Can run"));
	}

}