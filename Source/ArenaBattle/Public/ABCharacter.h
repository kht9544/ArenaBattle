// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaBattle.h"
#include "GameFramework/Character.h"
#include "ABCharacter.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnAttackEndDelegate);

UCLASS()
class ARENABATTLE_API AABCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AABCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	enum class EControlMode{
		TPS,
		QUARTERVIEW,
		NPC
	};

	void SetControlMode(EControlMode NewControlMode);

	EControlMode CurrentControlMode = EControlMode::TPS;
	FVector DirectionToMove = FVector::ZeroVector;

	FRotator ArmRotationTo = FRotator::ZeroRotator;
	float ArmLengthTo = 0.0f;
	float ArmLengthSpeed=0.0f;
	float ArmRotationSpeed = 0.0f;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void PostInitializeComponents() override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere,Category="Camera")
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere,Category="Camera")
	UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere,Category="UI")
	class UWidgetComponent* HPBarWidget;

	void AttackCheck();
	
	UPROPERTY(EditInstanceOnly,BlueprintReadOnly,Category="Attack",Meta=(AllowPrivateAccess=true))
	float AttackRange;

	UPROPERTY(EditInstanceOnly,BlueprintReadOnly,Category="Attack",Meta=(AllowPrivateAccess=true))
	float AttackRadius;

	bool CanSetWeapon();
	void SetWeapon(class AABWeapon* NewWeapon);

	void Attack();
	FOnAttackEndDelegate OnAttackEnd;

	void PossessedBy(AController* NewController);

	UPROPERTY(VisibleAnywhere,Category="Weapon")
	class AABWeapon* CurrentWeapon;

	UPROPERTY(VisibleAnywhere,Category="Stat")
	class UABCharacterStatComponent* CharacterStat;

private:
	void UpDown(float NewAxisValue);
	void LeftRight(float NewAxisValue);
	void Turn(float NewAxisValue);
	void LookUp(float NewAxisValue);

	void ViewChange();

	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void AttackStartComboState();
	void AttackEndComboState();

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = Attack, Meta = (AllowPrivateAccess = true))
	bool IsAttacking;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = Attack, Meta = (AllowPrivateAccess = true))
	bool CanNextCombo;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = Attack, Meta = (AllowPrivateAccess = true))
	bool IsComboInputOn;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = Attack, Meta = (AllowPrivateAccess = true))
	int32 CurrentCombo;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = Attack, Meta = (AllowPrivateAccess = true))
	int32 MaxCombo;


	UPROPERTY()
	class UABAnimInstance* ABAnim;

	void OnAssetLoadCompleted();

	FSoftObjectPath CharacterAssetToLoad = FSoftObjectPath(nullptr);
	TSharedPtr<struct FStreamableHandle> AssetStreamingHandle;
};