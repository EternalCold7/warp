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
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* FollowCamera;
	AMyProjectCharacter();
	
	UTimelineComponent* m_WallRunTimeline;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseTurnRate;
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
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UWarpComponent* m_WarpComponent;
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


	virtual void BeginPlay() override;

	FOnTimelineFloat WallRunningTickFunction{};

	class UCurveFloat* m_WallRunningCurve;
	void ChangeFlagsAfterAnimation();
	UFUNCTION()
	void WallRunningTick(float delta);
	UPROPERTY()
	FVector m_PlayerDirection;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sword")
		UStaticMeshComponent* m_Sword;
};

