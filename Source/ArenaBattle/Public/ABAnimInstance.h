// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaBattle.h"
#include "Animation/AnimInstance.h"
#include "ABAnimInstance.generated.h"


DECLARE_MULTICAST_DELEGATE(FOnNextAttackCheckDelegate);
DECLARE_MULTICAST_DELEGATE(FOnAttackHitComboDelegate);

/**
 * 
 */
UCLASS()
class ARENABATTLE_API UABAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	UABAnimInstance();
	virtual void NativeUpdateAnimation(float DeltaSeconds)override;

	void PlayAttackMontage();
	void JumpToAttackMontageSection(int32 NewSection);

	void SetDeadAnim(){IsDead = true;}

public:
	FOnNextAttackCheckDelegate OnNextAttackCheck;
	FOnAttackHitComboDelegate OnAttackHitCheck;
private:

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Pawn",Meta=(AllowPrivateAccess = true))
	float CurrentPawnSpeed;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Pawn",Meta=(AllowPrivateAccess=true))
	bool IsInAir;

		
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Attack, Meta = (AllowPrivateAccess = true))
	UAnimMontage* AttackMontage;

	UFUNCTION()
	void AnimNotify_AttackHitCheck();

	UFUNCTION()
	void AnimNotify_NextAttackCheck();

	FName GetAttackMontageSectionName(int32 Section);

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Pawn",Meta=(AllowPrivateAccess = true))
	bool IsDead;
};
