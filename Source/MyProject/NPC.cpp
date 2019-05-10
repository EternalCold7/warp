// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC.h"
#include "Runtime/Engine/Classes/Components/SceneComponent.h"
#include "Runtime/Engine/Classes/Camera/CameraComponent.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"
#include "Runtime/UMG/Public/Components/WidgetComponent.h"
#include "Engine/World.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "MyProjectCharacter.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "Runtime/Engine/Public/TimerManager.h"

// Sets default values
ANPC::ANPC()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	m_WarpLocation = CreateDefaultSubobject<USceneComponent>("WarpLocation");
	m_WidgetComponent = CreateDefaultSubobject<UWidgetComponent>("Widget");
	m_WidgetComponent->SetupAttachment(m_WarpLocation);

	m_Timeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("Timeline"));

	InterpFunction.BindUFunction(this, FName("TimelineFloatReturn"));
	TimelineFinished.BindUFunction(this,FName("OnTimelineFinished"));
}

// Called when the game starts or when spawned
void ANPC::BeginPlay()
{
	Super::BeginPlay();
	auto PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	m_Player = Cast<AMyProjectCharacter>(PC->GetPawn());
	if (m_Curve) {
		m_Timeline->AddInterpFloat(m_Curve, InterpFunction, FName("Alpha"));
		m_Timeline->SetTimelineFinishedFunc(TimelineFinished);
		m_Timeline->SetLooping(false);
		m_Timeline->SetIgnoreTimeDilation(true);
	}
	auto actorLoc = GetActorLocation();
	actorLoc.Z += 30;
	m_WarpLocation->SetWorldLocation(actorLoc);
	m_WidgetComponent->SetWorldLocation(actorLoc);

	GetWorldTimerManager().SetTimer(FuzeTimerHandle, this, &ANPC::SetTarget, 0.01f, true);
}

// Called every frame
void ANPC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	TraceAndRotateWarpLocation();

	
}

void ANPC::TimelineFloatReturn(float value)
{
	auto a = m_WidgetComponent->GetComponentScale();
	FVector b = !ShowCross ? FVector{0.f, 0.f, 0.f} : FVector{0.2f, 0.2f, 0.2f};

	m_WidgetComponent->SetWorldScale3D(FMath::InterpExpoInOut(a, b, value));
}

void ANPC::OnTimelineFinished()
{
}

// Called to bind functionality to input
void ANPC::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ANPC::TraceAndRotateWarpLocation() {


	FHitResult hitRes;
	auto location = GetActorLocation();
	if (!m_Player)
	{
		UE_LOG(LogTemp, Error, TEXT("NO CHARACTER SETUPED"));
		return;
	}
	GetWorld()->LineTraceSingleByChannel(hitRes, location, m_Player->GetActorLocation(), ECollisionChannel::ECC_Visibility);

	auto diff = hitRes.TraceStart - hitRes.TraceEnd;
	auto distance = (diff.X * diff.X + diff.Y * diff.Y + diff.Z * diff.Z);
	bool moreThanMinimum = distance >= 150.f;
	m_WarpLocation->SetVisibility(moreThanMinimum, true);


	if (moreThanMinimum) {


		auto dirVec = UKismetMathLibrary::GetDirectionUnitVector(location, hitRes.TraceEnd);
		auto newWarpLoc = location + dirVec * 100;
		auto warpLocation = m_WarpLocation->GetComponentLocation();
		newWarpLoc.Z = warpLocation.Z;
		auto rotation = UKismetMathLibrary::FindLookAtRotation(warpLocation, hitRes.TraceEnd);

		m_WarpLocation->SetWorldLocationAndRotation(newWarpLoc, rotation);

		auto widgetLoc = m_WidgetComponent->GetComponentLocation();
		auto newWidgetLoc = location + dirVec * 30;
		newWidgetLoc.Z = widgetLoc.Z;
		m_WidgetComponent->SetWorldLocation(newWidgetLoc);

	}
}

void ANPC::SetTarget() {
	TraceAndRotateWarpLocation();

	if (ShowCross)
	{
		m_Timeline->Play();
	}
	else
	{
		m_Timeline->Reverse();

	}
}