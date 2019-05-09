// Fill out your copyright notice in the Description page of Project Settings.


#include "OverlapingCone.h"
#include"Classes/Components/StaticMeshComponent.h"
// Sets default values
AOverlapingCone::AOverlapingCone()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	m_ConeMesh = CreateDefaultSubobject<UStaticMeshComponent>("Cone");

}

// Called when the game starts or when spawned
void AOverlapingCone::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AOverlapingCone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

