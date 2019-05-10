// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "ColisionStaticMeshComponent.generated.h"

/**
 * 
 */
UCLASS()
class MYPROJECT_API UColisionStaticMeshComponent : public UStaticMeshComponent
{
	GENERATED_BODY()
public:
	UFUNCTION()
	void GetActorsToOverlap(TSet<AActor*>& actors, TSubclassOf<AActor> filter);
};
