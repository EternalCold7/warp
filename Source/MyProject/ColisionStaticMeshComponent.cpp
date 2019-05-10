// Fill out your copyright notice in the Description page of Project Settings.


#include "ColisionStaticMeshComponent.h"

void UColisionStaticMeshComponent::GetActorsToOverlap(TSet<AActor*>& actors, TSubclassOf<AActor> filter)
{
	GetOverlappingActors(actors, filter);
}
