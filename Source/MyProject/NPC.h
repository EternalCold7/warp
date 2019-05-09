// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Engine/Classes/Components/TimelineComponent.h"
#include "GameFramework/Character.h"
#include "NPC.generated.h"

UCLASS()
class MYPROJECT_API ANPC : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ANPC();
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class USceneComponent * m_WarpLocation = nullptr;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UWidgetComponent* m_WidgetComponent = nullptr;
	bool ShowCross = false;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	class UTimelineComponent* m_Timeline;
	class AMyProjectCharacter* m_Player;
	FTimerHandle FuzeTimerHandle;
	UPROPERTY(EditAnywhere, Category = "Timeline")
		class UCurveFloat* m_Curve;
	void TraceAndRotateWarpLocation();
public:	
	void SetTarget();
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	FOnTimelineFloat InterpFunction{};
	FOnTimelineEvent TimelineFinished{};
	UFUNCTION()
		void TimelineFloatReturn(float value);
	UFUNCTION()
		void OnTimelineFinished();
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
