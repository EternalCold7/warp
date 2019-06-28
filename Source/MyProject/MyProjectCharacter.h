// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Runtime/Engine/Classes/Components/TimelineComponent.h"

#include "MyProjectCharacter.generated.h"

UCLASS(config = Game)
class AMyProjectCharacter : public ACharacter
{
	GENERATED_BODY()

		/** Camera boom positioning the camera behind the character */
		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* FollowCamera;
public:
	AMyProjectCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseLookUpRate;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool IsInWarp = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool CanWarp = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool CanMove = true;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool IsOnWall = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		uint8 MaxJumpsCount = 2;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		uint8 CurrentJumsCount = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UCapsuleComponent* m_WallCollisionCapsule;
	//virtual void OnLanded(const FHitResult& result) override;
	UFUNCTION()
		void DoubleJump();
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
		float JumpHeight = 600.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool isJumping = false;
	virtual void Landed(const FHitResult& hit) override;
	virtual void Tick(float delta);
protected:

	void OnResetVR();

	void MoveForward(float Value);

	void MoveRight(float Value);

	void TurnAtRate(float Rate);


	void LookUpAtRate(float Rate);

	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

protected:

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


public:
	UFUNCTION()
	void OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	void OnCapsuleEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	void Warp();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overlaping")
	class UColisionStaticMeshComponent * m_OverlapingMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sword")
		UStaticMeshComponent* m_Sword;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ParticleSystem")
		class UParticleSystemComponent* m_ParticleFollowing;
	UFUNCTION()
	void FindCurrentEnemy();
	float m_LowestLength;
	class ANPC* EnemyToWarp;
	virtual void BeginPlay() override;
	FTimerHandle m_TimerHandle;
	FTimerHandle m_AdditionalTimerHandle;
	void SetupWarpAnimation();

	FVector m_CurrLocation;
	FVector m_SwordLocation;
	FRotator m_SwordRotation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UMaterialParameterCollection* m_MatParamCollection;


	class UMaterialParameterCollectionInstance* m_MatColInst;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
		USkeletalMesh* m_PostMesh;
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
	UTimelineComponent* m_WallRunTimeline;
	FOnTimelineFloat WallRunningTickFunction{};

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
	UPROPERTY(EditAnywhere, Category = "Timeline")
		class UCurveFloat* m_WallRunningCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class UCameraShake> m_CamShake = nullptr;
	UFUNCTION()
		void Bloom(float a);
	UFUNCTION()
		void Empty();
	void BackToPlaceSwordAndActorRotation();
	void ChangeFlagsAfterAnimation();
	UFUNCTION()
	void WallRunningTick(float delta);
	UPROPERTY()
	FVector m_PlayerDirection;

};

