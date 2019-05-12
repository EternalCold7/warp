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
#include "OverlapingCone.h"
#include "NPC.h"
#include "Classes/Components/StaticMeshComponent.h"
#include "Classes/Components/SkeletalMeshComponent.h"
#include "ColisionStaticMeshComponent.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "Runtime/Engine/Classes/Materials/MaterialParameterCollection.h"
#include "Runtime/Engine/Classes/Materials/MaterialParameterCollectionInstance.h"
#include "Runtime/Engine/Classes/Animation/SkeletalMeshActor.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
//////////////////////////////////////////////////////////////////////////
// AMyProjectCharacter

AMyProjectCharacter::AMyProjectCharacter()
{
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

	m_OverlapingMesh = CreateDefaultSubobject<UColisionStaticMeshComponent>("overlap mesh");
	m_Sword = CreateDefaultSubobject<UStaticMeshComponent>("swor");

	m_Sword->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, FName("SwordSocket"));

	InterpCharFunction.BindUFunction(this, FName("CharTimelineFloatReturn"));
	InterpSwordFunction.BindUFunction(this, FName("SwordTimelineFloatReturn"));
	InterpFOVFunction.BindUFunction(this, FName("FOVTimelineFloatReturn"));
	TimelineFinished.BindUFunction(this, FName("OnTimelineFinished"));
	m_Timeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("Timeline"));
	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

//////////////////////////////////////////////////////////////////////////
// Input

void AMyProjectCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Warp", IE_Pressed, this, &AMyProjectCharacter::Warp);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMyProjectCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMyProjectCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AMyProjectCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMyProjectCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AMyProjectCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AMyProjectCharacter::TouchStopped);

	// VR headset functionality
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

	if (m_CharCurve && m_FOVCurve && m_SwordCurve) {
		m_Timeline->SetTimelineFinishedFunc(TimelineFinished);
		m_Timeline->AddInterpFloat(m_CharCurve, InterpCharFunction, FName("Alpha"));
		m_Timeline->AddInterpFloat(m_SwordCurve, InterpSwordFunction, FName("Sword"));
		m_Timeline->AddInterpFloat(m_FOVCurve, InterpFOVFunction, FName("FOV"));
		
		m_Timeline->SetLooping(false);
		m_Timeline->SetIgnoreTimeDilation(true);
	}


	GetWorldTimerManager().SetTimer(m_TimerHandle, this, &AMyProjectCharacter::FindCurrentEnemy, 0.01f, true);


}
void AMyProjectCharacter::FindCurrentEnemy() {

	if (IsInWarp)
		return;
	TMap<ANPC*, float> enemies;
	TSet<AActor*> overlapingActors;
	if (!m_OverlapingMesh)
		return;
	m_OverlapingMesh->GetActorsToOverlap(overlapingActors, TSubclassOf<ANPC>());
	for (auto actor : overlapingActors) {
		auto npc = Cast<ANPC>(actor);
		auto distanceVector = GetActorLocation() - npc->GetActorLocation();
		float distance = distanceVector.X * distanceVector.X + distanceVector.Y * distanceVector.Y + distanceVector.Z * distanceVector.Z;
		enemies.Add(npc, distance);
	}
	int i = 0;
	for (auto npc : enemies) {
		npc.Key->ShowCross = false;
		UE_LOG(LogTemp, Warning, TEXT("enemy %s"), *npc.Key->GetName());
		if (i == 0) {
			m_LowestLength = npc.Value;
			EnemyToWarp = npc.Key;
			i++;
			continue;
		}
			
		if (npc.Value <= m_LowestLength)
			EnemyToWarp = npc.Key;

	}
	if(EnemyToWarp)
		EnemyToWarp->ShowCross = true;

}
void AMyProjectCharacter::SetupWarpAnimation() {
	m_CurrLocation = GetActorLocation();
	m_SwordLocation = m_Sword->GetComponentLocation();
	m_SwordRotation = m_Sword->GetComponentRotation();
	/*if (m_MatParamCollection)
		m_MatParamCollection->SetScalarParameterValue(FName("BlueOpacityEffect"), 1.f);*/
	m_Sword->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, true));
	GetMesh()->GlobalAnimRateScale = KEK;
	GetMesh()->SetVisibility(false);
	clone = CreatePostMesh(GetActorLocation());
	
	m_Timeline->PlayFromStart();

}

ASkeletalMeshActor* AMyProjectCharacter::CreatePostMesh(const FVector& pos) {
	FActorSpawnParameters a;
	a.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	a.bNoFail = true;

	auto postMesh = GetWorld()->SpawnActor<ASkeletalMeshActor>(GetMesh()->GetComponentLocation(), GetMesh()->GetComponentRotation(),a);
	
	if (!postMesh)
		return nullptr;
	postMesh->GetSkeletalMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	postMesh->GetSkeletalMeshComponent()->SetSkeletalMesh(m_PostMesh);
	postMesh->GetSkeletalMeshComponent()->SetMaterial(0, m_FadeMaterial);
	postMesh->GetSkeletalMeshComponent()->SetMaterial(1, m_FadeMaterial);
	postMesh->GetSkeletalMeshComponent()->PlayAnimation(m_FadeAnimation,false);
	postMesh->GetSkeletalMeshComponent()->SetPosition(0.5f);

	postMesh->GetSkeletalMeshComponent()->GlobalAnimRateScale = 0.f;
	return postMesh;
}


void AMyProjectCharacter::CharTimelineFloatReturn(float value)
{
	
	auto to = EnemyToWarp->m_WarpLocation->GetComponentLocation();
	FVector newLoc;
	newLoc.X =UKismetMathLibrary::Ease(m_CurrLocation.X, to.X, value, EEasingFunc::ExpoIn);
	newLoc.Y =UKismetMathLibrary::Ease(m_CurrLocation.Y, to.Y, value, EEasingFunc::ExpoIn);
	newLoc.Z =UKismetMathLibrary::Ease(m_CurrLocation.Z, to.Z, value, EEasingFunc::ExpoIn);
	SetActorLocation(newLoc);
}

void AMyProjectCharacter::SwordTimelineFloatReturn(float value)
{
	auto to = EnemyToWarp->m_WarpLocation->GetComponentLocation();
	FVector newLoc;
	newLoc.X = UKismetMathLibrary::Ease(m_SwordLocation.X, to.X, value, EEasingFunc::ExpoIn);
	newLoc.Y = UKismetMathLibrary::Ease(m_SwordLocation.Y, to.Y, value, EEasingFunc::ExpoIn);
	newLoc.Z = UKismetMathLibrary::Ease(m_SwordLocation.Z, to.Z, value, EEasingFunc::ExpoIn);
	m_Sword->SetWorldLocationAndRotation(newLoc,m_SwordRotation);
}

void AMyProjectCharacter::FOVTimelineFloatReturn(float value)
{
	/*auto to = EnemyToWarp->m_WarpLocation->GetComponentLocation();
	auto camLoc = FollowCamera->GetComponentLocation();
	FVector newLoc;
	newLoc.X = UKismetMathLibrary::Ease(camLoc.X, to.X, value, EEasingFunc::ExpoIn);
	newLoc.Y = UKismetMathLibrary::Ease(camLoc.Y, to.Y, value, EEasingFunc::ExpoIn);
	newLoc.Z = UKismetMathLibrary::Ease(camLoc.Z, to.Z, value, EEasingFunc::ExpoIn);
	FollowCamera->SetWorldLocation(newLoc);
*/
	FollowCamera->SetFieldOfView(value);
}

void AMyProjectCharacter::OnTimelineFinished()
{

	auto attachRule = FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, true);
	m_Sword->AttachToComponent(GetMesh(), attachRule, FName("SwordSocket"));

	GetMesh()->SetVisibility(true);
	GetMesh()->GlobalAnimRateScale = 1.f;
	GetMesh()->SetPosition(1.2);
	if (!m_CamShake) {
		UE_LOG(LogTemp, Error, TEXT("No camera shake"));
	}
	else {
		UGameplayStatics::GetPlayerController(GetWorld(), 0)->ClientPlayCameraShake(m_CamShake,4.f);

	}

	GetWorldTimerManager().SetTimer(m_TimerHandle, 0.84f, false);
	CanWarp = true;
	IsInWarp = false;
	CanMove = true;

	GetWorldTimerManager().SetTimer(m_TimerHandle, 0.2f, false);
	GetWorld()->DestroyActor(clone);
}

void AMyProjectCharacter::Warp()
{
	if (!EnemyToWarp || !CanWarp)
		return;
	CanWarp = false;
	IsInWarp = true;
	CanMove = false;


	auto rot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), EnemyToWarp->GetActorLocation());
	SetActorRotation(rot);
	GetMesh()->SetPosition(0.3f);
	GetWorldTimerManager().SetTimer(m_TimerHandle, this, &AMyProjectCharacter::SetupWarpAnimation, 0.5f);

}