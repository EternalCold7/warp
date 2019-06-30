// Fill out your copyright notice in the Description page of Project Settings.


#include "WarpComponent.h"
#include"Engine/World.h"
#include "Runtime/Engine/Classes/Particles/ParticleSystemComponent.h"
#include "Runtime/Engine/Public/TimerManager.h"
#include "ColisionStaticMeshComponent.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "MyProjectCharacter.h"
#include "Runtime/Engine/Classes/Materials/MaterialParameterCollection.h"
#include "Runtime/Engine/Classes/Materials/MaterialParameterCollectionInstance.h"
#include "Classes/Components/SkeletalMeshComponent.h"
#include "Classes/Components/StaticMeshComponent.h"
#include "NPC.h"
#include "Camera/CameraComponent.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/Animation/SkeletalMeshActor.h"
// Sets default values for this component's properties
UWarpComponent::UWarpComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;


	m_ParticleFollowing = CreateDefaultSubobject<UParticleSystemComponent>("FollowingParticle");



	
	bEditableWhenInherited = true;



	InterpCharFunction.BindUFunction(this, FName("CharTimelineFloatReturn"));
	InterpSwordFunction.BindUFunction(this, FName("SwordTimelineFloatReturn"));
	InterpFOVFunction.BindUFunction(this, FName("FOVTimelineFloatReturn"));
	TimelineFinished.BindUFunction(this, FName("OnTimelineFinished"));


	
	InterpBloomEffect.BindUFunction(this, FName("Bloom"));


	m_Timeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("Timeline"));
	m_PostBloomTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("PostBloom"));

}


void UWarpComponent::FindCurrentEnemy()
{
	/*if (IsInWarp)
		return;*/
	TMap<ANPC*, float> enemies;
	TSet<AActor*> overlapingActors;
	if (!m_ProjectCharacter->m_OverlapingMesh)
		return;
	m_ProjectCharacter->m_OverlapingMesh->GetActorsToOverlap(overlapingActors, TSubclassOf<ANPC>());
	for (auto actor : overlapingActors) {
		auto npc = Cast<ANPC>(actor);
		if (!npc) {
			continue;
		}
		auto PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		FVector2D sLoc;
		PC->ProjectWorldLocationToScreen(m_ProjectCharacter->GetActorLocation(), sLoc);
		int32 x, y;
		PC->GetViewportSize(x, y);
		sLoc.X = -sLoc.X + (float)x;
		sLoc.Y = -sLoc.Y + (float)y;

		float distance = sLoc.X * sLoc.X + sLoc.Y * sLoc.Y;
		enemies.Add(npc, distance);
	}
	int i = 0;
	for (auto npc : enemies) {
		npc.Key->ShowCross = false;

		if (i == 0) {
			m_LowestLength = npc.Value;
			EnemyToWarp = npc.Key;
			i++;
			continue;
		}

		if (npc.Value <= m_LowestLength)
			EnemyToWarp = npc.Key;

	}
	if (EnemyToWarp)
		EnemyToWarp->ShowCross = true;

}

ASkeletalMeshActor* UWarpComponent::CreatePostMesh(const FVector& pos)
{
	FActorSpawnParameters a;
	a.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	a.bNoFail = true;

	auto postMesh = GetWorld()->SpawnActor<ASkeletalMeshActor>(m_ProjectCharacter->GetMesh()->GetComponentLocation(), m_ProjectCharacter->GetMesh()->GetComponentRotation(), a);

	if (!postMesh)
		return nullptr;
	postMesh->GetSkeletalMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	postMesh->GetSkeletalMeshComponent()->SetSkeletalMesh(m_PostMesh);
	postMesh->GetSkeletalMeshComponent()->SetMaterial(0, m_FadeMaterial);
	postMesh->GetSkeletalMeshComponent()->SetMaterial(1, m_FadeMaterial);
	postMesh->GetSkeletalMeshComponent()->PlayAnimation(m_FadeAnimation, false);
	postMesh->GetSkeletalMeshComponent()->SetPosition(0.3f);

	postMesh->GetSkeletalMeshComponent()->GlobalAnimRateScale = 0.f;
	if (ensure(m_ParticleFollowing))
		m_ParticleFollowing->Activate();
	return postMesh;
}

void UWarpComponent::CharTimelineFloatReturn(float value)
{
	auto to = EnemyToWarp->m_WarpLocation->GetComponentLocation();
	FVector newLoc;
	newLoc.X = UKismetMathLibrary::Ease(m_CurrLocation.X, to.X, value, EEasingFunc::ExpoIn);
	newLoc.Y = UKismetMathLibrary::Ease(m_CurrLocation.Y, to.Y, value, EEasingFunc::ExpoIn);
	newLoc.Z = UKismetMathLibrary::Ease(m_CurrLocation.Z, to.Z, value, EEasingFunc::ExpoIn);
	m_ProjectCharacter->SetActorLocation(newLoc);
}

void UWarpComponent::SwordTimelineFloatReturn(float value)
{
	auto to = EnemyToWarp->m_WarpLocation->GetComponentLocation();
	FVector newLoc;
	newLoc.X = UKismetMathLibrary::Ease(m_SwordLocation.X, to.X, value, EEasingFunc::ExpoIn);
	newLoc.Y = UKismetMathLibrary::Ease(m_SwordLocation.Y, to.Y, value, EEasingFunc::ExpoIn);
	newLoc.Z = UKismetMathLibrary::Ease(m_SwordLocation.Z, to.Z, value, EEasingFunc::ExpoIn);
	m_ProjectCharacter->m_Sword->SetWorldLocationAndRotation(newLoc, m_SwordRotation);
}

void UWarpComponent::FOVTimelineFloatReturn(float value)
{
	m_ProjectCharacter->FollowCamera->SetFieldOfView(value);
}

void UWarpComponent::OnTimelineFinished()
{
	auto attachRule = FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, true);
	m_ProjectCharacter->m_Sword->AttachToComponent(m_ProjectCharacter->GetMesh(), attachRule, FName("SwordSocket"));

	m_ProjectCharacter->GetMesh()->SetVisibility(true);

	m_ProjectCharacter->GetMesh()->GlobalAnimRateScale = 1.f;
	m_ProjectCharacter->GetMesh()->SetPosition(0.7);
	if (!m_CamShake) {
		UE_LOG(LogTemp, Error, TEXT("No camera shake"));
	}
	else {
		UGameplayStatics::GetPlayerController(GetWorld(), 0)->ClientPlayCameraShake(m_CamShake, 4.f);

	}

	m_ProjectCharacter->GetWorldTimerManager().SetTimer(m_TimerHandle, m_ProjectCharacter, &AMyProjectCharacter::ChangeFlagsAfterAnimation, 0.84f, false);


	GetWorld()->GetTimerManager().SetTimer(m_AdditionalTimerHandle, this, &UWarpComponent::BackToPlaceSwordAndActorRotation, 0.2f, false);
}

// Called when the game starts
void UWarpComponent::BeginPlay()
{
	Super::BeginPlay();
	m_ParticleFollowing->AttachToComponent(m_ProjectCharacter->m_Sword, FAttachmentTransformRules::KeepRelativeTransform, FName("SwordSocket"));
	if (m_CharCurve && m_FOVCurve && m_SwordCurve) {
		m_Timeline->SetTimelineFinishedFunc(TimelineFinished);
		m_Timeline->AddInterpFloat(m_CharCurve, InterpCharFunction, FName("Alpha"));
		m_Timeline->AddInterpFloat(m_SwordCurve, InterpSwordFunction, FName("Sword"));
		m_Timeline->AddInterpFloat(m_FOVCurve, InterpFOVFunction, FName("FOV"));

		m_Timeline->SetLooping(false);
		m_Timeline->SetIgnoreTimeDilation(true);
	}
	if (m_BloomCurve)
	{
		
		m_PostBloomTimeline->AddInterpFloat(m_BloomCurve, InterpBloomEffect);
		m_PostBloomTimeline->SetLooping(false);
		m_PostBloomTimeline->SetIgnoreTimeDilation(true);
	}

	
	if (m_MatParamCollection)
		m_MatColInst = GetWorld()->GetParameterCollectionInstance(m_MatParamCollection);

	GetWorld()->GetTimerManager().SetTimer(m_TimerHandle, this, &UWarpComponent::FindCurrentEnemy, 0.01f, true);

	
	if (ensure(m_ParticleFollowing)) {
		m_ParticleFollowing->Deactivate();
		if (ensure(m_ProjectCharacter->m_Sword)) {
			FVector max, min;
			m_ProjectCharacter->m_Sword->GetLocalBounds(max, min);
			m_ParticleFollowing->SetRelativeLocation(FVector(max));
		}

	}

	
}


void UWarpComponent::BackToPlaceSwordAndActorRotation() {
	GetWorld()->DestroyActor(clone);

	if (ensure(m_ParticleFollowing))
		m_ParticleFollowing->Deactivate();


	auto rot = m_ProjectCharacter->GetMesh()->GetComponentRotation();
	rot.Roll = 0;
	auto rot2 = m_ProjectCharacter->GetActorRotation();
	rot2.Roll = 0;
	m_ProjectCharacter->GetMesh()->SetWorldRotation(rot);
	m_ProjectCharacter->SetActorRotation(rot2);


	m_PostBloomTimeline->PlayFromStart();
}



// Called every frame
void UWarpComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UWarpComponent::Warp()
{
	if (!EnemyToWarp /*|| !CanWarp*/)
		return;
	/*CanWarp = false;
	IsInWarp = true;
	CanMove = false;*/


	auto rot = UKismetMathLibrary::FindLookAtRotation(m_ProjectCharacter->GetActorLocation(), EnemyToWarp->GetActorLocation());
	m_ProjectCharacter->SetActorRotation(rot);
	m_ProjectCharacter->GetMesh()->SetPosition(0.3f);
	GetWorld()->GetTimerManager().SetTimer(m_TimerHandle, this, &UWarpComponent::SetupWarpAnimation, 0.5f, false);
}


void UWarpComponent::SetupWarpAnimation() {
	m_CurrLocation = m_ProjectCharacter->GetActorLocation();
	m_SwordLocation = m_ProjectCharacter->m_Sword->GetComponentLocation();
	m_SwordRotation = m_ProjectCharacter->m_Sword->GetComponentRotation();
	if (m_MatColInst)
		m_MatColInst->SetScalarParameterValue(FName("BlueOpacityEffect"), 1.f);
	m_ProjectCharacter->m_Sword->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, true));
	m_ProjectCharacter->GetMesh()->GlobalAnimRateScale = 0.5;
	m_ProjectCharacter->GetMesh()->SetVisibility(false);
	clone = CreatePostMesh(m_ProjectCharacter->GetActorLocation());

	m_Timeline->PlayFromStart();

}
void UWarpComponent::Bloom(float a)
{
	if (m_MatColInst)
		m_MatColInst->SetScalarParameterValue("BlueOpacityEffect", a);
}
