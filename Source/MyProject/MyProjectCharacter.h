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
protected:

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	void Warp();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Overlaping")
	class UColisionStaticMeshComponent * m_OverlapingMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sword")
		UStaticMeshComponent* m_Sword;

	UFUNCTION()
	void FindCurrentEnemy();
	float m_LowestLength;
	class ANPC* EnemyToWarp;
	virtual void BeginPlay() override;
	FTimerHandle m_TimerHandle;
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim")
		float KEK = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim")
		float LOL = 0;

	FOnTimelineFloat InterpCharFunction{};
	FOnTimelineFloat InterpFOVFunction{};
	FOnTimelineFloat InterpSwordFunction{};
	FOnTimelineEvent TimelineFinished{};
	FOnTimelineEvent BloomFinished{};
	FOnTimelineFloat InterpBloomEffect{};
	class UTimelineComponent* m_Timeline;
	class UTimelineComponent* m_PostBloomTimeline;
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
	UFUNCTION()
		void Bloom(float a);
	UFUNCTION()
		void Empty();
};

