// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/MeshComponent.h"
#include "CollisionMeshComponent.generated.h"

/**
 * 
 */
UCLASS()
class MYPROJECT_API UCollisionMeshComponent : public UMeshComponent
{
	GENERATED_BODY()
public:
	UFUNCTION()
	void GetActorsToOverlap(TSet<AActor*>& actors, TSubclassOf<AActor> filter);
};

