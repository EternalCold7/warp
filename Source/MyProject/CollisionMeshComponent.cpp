// Fill out your copyright notice in the Description page of Project Settings.


#include "CollisionMeshComponent.h"

void UCollisionMeshComponent::GetActorsToOverlap(TSet<AActor*> & actors , TSubclassOf<AActor> filter) {
	GetOverlappingActors(actors, filter);
}