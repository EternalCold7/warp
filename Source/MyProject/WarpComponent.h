// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Runtime/Engine/Classes/Components/TimelineComponent.h"
#include "WarpComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MYPROJECT_API UWarpComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class AMyProjectCharacter* m_ProjectCharacter;
	 
	// Sets default values for this component's properties
	UWarpComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ParticleSystem")
		class UParticleSystemComponent* m_ParticleFollowing;

	FTimerHandle m_AdditionalTimerHandle;
	UFUNCTION()
		void FindCurrentEnemy();
	float m_LowestLength;
	class ANPC* EnemyToWarp;

	FVector m_CurrLocation;
	FVector m_SwordLocation;
	FRotator m_SwordRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UMaterialParameterCollection* m_MatParamCollection;


	class UMaterialParameterCollectionInstance* m_MatColInst;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
		class USkeletalMesh* m_PostMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
		class UMaterialInterface* m_FadeMaterial;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
		class UAnimationAsset* m_FadeAnimation;
	

	class ASkeletalMeshActor* CreatePostMesh(const FVector& pos);
	ASkeletalMeshActor* clone = nullptr;

	FOnTimelineFloat InterpCharFunction{};
	FOnTimelineFloat InterpFOVFunction{};
	FOnTimelineFloat InterpSwordFunction{};
	FOnTimelineEvent TimelineFinished{};
	FOnTimelineEvent BloomFinished{};
	FOnTimelineFloat InterpBloomEffect{};
	class UTimelineComponent* m_Timeline;
	UTimelineComponent* m_PostBloomTimeline;
	FTimerHandle m_TimerHandle;

	UFUNCTION()
		void CharTimelineFloatReturn(float value);
	UFUNCTION()
		void SwordTimelineFloatReturn(float value);
	UFUNCTION()
		void FOVTimelineFloatReturn(float value);
	UFUNCTION()
		void OnTimelineFinished();
	UPROPERTY(EditAnywhere, Category = "Timeline")
		class UCurveFloat* m_CharCurve;
	UPROPERTY(EditAnywhere, Category = "Timeline")
		class UCurveFloat* m_SwordCurve;
	UPROPERTY(EditAnywhere, Category = "Timeline")
		class UCurveFloat* m_FOVCurve;
	UPROPERTY(EditAnywhere, Category = "Timeline")
		class UCurveFloat* m_BloomCurve;


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<class UCameraShake> m_CamShake = nullptr;
	void BackToPlaceSwordAndActorRotation();
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	void SetupWarpAnimation();
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void Bloom(float a);
	void Warp();
};
