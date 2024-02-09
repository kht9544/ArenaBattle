// Fill out your copyright notice in the Description page of Project Settings.


#include "ABCharacter.h"
#include "ABAnimInstance.h"
#include "DrawDebugHelpers.h"
#include "ABWeapon.h"
#include "ABCharacterStatComponent.h"
#include "Components/WidgetComponent.h"
#include "ABCharacterWidget.h"
#include "ABAIController.h"
#include "ABGameInstance.h"
#include "ABCharacterSetting.h"
#include "Engine/AssetManager.h"
#include "ABPlayerController.h"
#include "ABSection.h"
#include "ABPlayerState.h"

// Sets default values
AABCharacter::AABCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SPRINGARM"));
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("CAMERA"));
	CharacterStat = CreateDefaultSubobject<UABCharacterStatComponent>(TEXT("CHARACTERSTAT"));
	HPBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HPBARWIDGET"));

	SpringArm->SetupAttachment(GetCapsuleComponent());
	Camera->SetupAttachment(SpringArm);
	HPBarWidget->SetupAttachment(GetMesh());

	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f,0.0f,-88.0f),FRotator(0.0f,-90.f,0.0f));
	SpringArm->TargetArmLength=400.0f;
	SpringArm->SetRelativeRotation(FRotator(-15.0f,0.0f,0.0f));
	HPBarWidget->SetRelativeLocation(FVector(0.0f,0.0f,180.0f));
	HPBarWidget->SetWidgetSpace(EWidgetSpace::Screen);

	static ConstructorHelpers::FObjectFinder<USkeletalMesh>SK_CARDBOARD(TEXT("/Game/InfinityBladeWarriors/Character/CompleteCharacters/SK_CharM_Cardboard.SK_CharM_Cardboard"));

	if(SK_CARDBOARD.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(SK_CARDBOARD.Object);
	}

	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	static ConstructorHelpers::FClassFinder<UAnimInstance> WARRIOR_ANIM(TEXT("/Game/Book/ABP_WarriorAnim.ABP_WarriorAnim_C"));

	if (WARRIOR_ANIM.Succeeded())
	{
		GetMesh()->SetAnimInstanceClass(WARRIOR_ANIM.Class);
	}

	static ConstructorHelpers::FClassFinder<UUserWidget>UI_HUD(TEXT("/Game/Book/UI/UI_HPBar.UI_HPBar_C"));
	if(UI_HUD.Succeeded())
	{
		HPBarWidget->SetWidgetClass(UI_HUD.Class);
		HPBarWidget->SetDrawSize(FVector2D(150.0f,50.0f));
	}


	ArmLengthSpeed = 3.0f;
	ArmRotationSpeed = 10.0f;
	GetCharacterMovement()->JumpZVelocity = 400.0f;

	

	SetControlMode(EControlMode::TPS);
	IsAttacking = false;

	MaxCombo = 4;
	AttackEndComboState();

	GetCapsuleComponent()->SetCollisionProfileName(TEXT("ABCharacter"));

	AttackRange = 200.0f;
	AttackRadius = 50.0f;

	AIControllerClass = AABAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	auto DefaultSetting = GetDefault<UABCharacterSetting>();
	if(DefaultSetting->CharacterAssets.Num()>0)
	{
		for(auto CharacterAsset : DefaultSetting->CharacterAssets)
		{
			UE_LOG(LogTemp, Display, TEXT("Your message"));
		}
	}
	AssetIndex = 4;

	SetActorHiddenInGame(true);
	HPBarWidget->SetHiddenInGame(true);
	SetCanBeDamaged(false);

	DeadTimer = 5.0f;
}

// Called when the game starts or when spawned
void AABCharacter::BeginPlay()
{
	Super::BeginPlay();

	bIsPlayer = IsPlayerControlled();
	if (bIsPlayer)
		ABPlayerController = Cast<AABPlayerController>(GetController());
	else
		ABAIController = Cast<AABAIController>(GetController());

	auto DefaultSetting = GetDefault<UABCharacterSetting>();

	if (bIsPlayer)
		AssetIndex = 4;
	else
		AssetIndex = FMath::RandRange(0, DefaultSetting->CharacterAssets.Num() - 1);

	CharacterAssetToLoad = DefaultSetting->CharacterAssets[AssetIndex];
	AssetStreamingHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad
	(CharacterAssetToLoad, FStreamableDelegate::CreateUObject(this, &AABCharacter::OnAssetLoadCompleted));
	SetCharacterState(ECharacterState::LOADING);
}



void AABCharacter::SetControlMode(EControlMode NewControlMode)
{
	CurrentControlMode = NewControlMode;
	switch (CurrentControlMode)
	{
	case EControlMode::TPS:
		//SpringArm->TargetArmLength         = 450.0f; //springarm길이 조정
		//SpringArm->SetRelativeRotation(FRotator::ZeroRotator);//부모액터와의 상대적인 각도 조절
		//SpringArm->bUsePawnControlRotation = true;//입력에따라 회전
		//SpringArm->bInheritPitch		   = true;//회전가능 
		ArmLengthTo						   = 450.0f;
		SpringArm->bUsePawnControlRotation = true;
		SpringArm->bInheritPitch		   = true;
		SpringArm->bInheritRoll			   = true;
		SpringArm->bInheritYaw			   = true;
		SpringArm->bDoCollisionTest		   = true;
		bUseControllerRotationYaw		   = false;

		GetCharacterMovement()->bOrientRotationToMovement = true;
		GetCharacterMovement()->bUseControllerDesiredRotation = false;
		GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f);

		break;
	
	case EControlMode::QUARTERVIEW:
		//SpringArm->TargetArmLength		   = 800.0f;
		//SpringArm->SetRelativeRotation(FRotator(-45.0f, 0.0f, 0.0f));
		ArmLengthTo						   = 800.0f;
		ArmRotationTo					   = FRotator(-45.0f, 0.0f, 0.0f);
		SpringArm->bUsePawnControlRotation = false;
		SpringArm->bInheritPitch		   = false;
		SpringArm->bInheritRoll			   = false;
		SpringArm->bInheritYaw			   = false;
		SpringArm->bDoCollisionTest		   = false;
		bUseControllerRotationYaw		   = false;

		GetCharacterMovement()->bOrientRotationToMovement = false;
		GetCharacterMovement()->bUseControllerDesiredRotation = true;
		GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
		break;

	case EControlMode::NPC:
		bUseControllerRotationYaw = false;
		GetCharacterMovement()->bUseControllerDesiredRotation = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;
		GetCharacterMovement()->RotationRate = FRotator(0.0f,400.0f,0.0f);

		break;
	}
}

// Called every frame
void AABCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, ArmLengthTo, DeltaTime, ArmLengthSpeed);

	switch (CurrentControlMode)
	{
	case EControlMode::QUARTERVIEW:
		SpringArm->SetRelativeRotation(FMath::RInterpTo(
        SpringArm->GetRelativeRotation(), ArmRotationTo, DeltaTime, ArmRotationSpeed));
		if (DirectionToMove.SizeSquared() > 0.0f)
		{
			GetController()->SetControlRotation(FRotationMatrix::MakeFromX(DirectionToMove).Rotator());
			AddMovementInput(DirectionToMove);
		}
		break;
	}
}

void AABCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	ABAnim = Cast<UABAnimInstance>(GetMesh()->GetAnimInstance());

	ABAnim->OnMontageEnded.AddDynamic(this, &AABCharacter::OnAttackMontageEnded);

	ABAnim->OnNextAttackCheck.AddLambda([this]() -> void
		{
			CanNextCombo = false;

			if (IsComboInputOn)
			{
				AttackStartComboState();
				ABAnim->JumpToAttackMontageSection(CurrentCombo);
			}
		});

		ABAnim->OnAttackHitCheck.AddUObject(this,&AABCharacter::AttackCheck);

		CharacterStat->OnHPIsZero.AddLambda([this]() -> void
	{
			ABAnim->SetDeadAnim();
			SetActorEnableCollision(false);
	});
}

// Called to bind functionality to input
void AABCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("UpDown"),this,&AABCharacter::UpDown);
	PlayerInputComponent->BindAxis(TEXT("LeftRight"),this,&AABCharacter::LeftRight);
	PlayerInputComponent->BindAxis(TEXT("Turn"),this,&AABCharacter::Turn);
	PlayerInputComponent->BindAxis(TEXT("LookUp"),this,&AABCharacter::LookUp);

	PlayerInputComponent->BindAction(TEXT("ViewChange"),EInputEvent::IE_Pressed,this,&AABCharacter::ViewChange);
	PlayerInputComponent->BindAction(TEXT("Jump"),EInputEvent::IE_Pressed,this,&AABCharacter::Jump);
	PlayerInputComponent->BindAction(TEXT("Attack"),EInputEvent::IE_Pressed,this,&AABCharacter::Attack);
}

void AABCharacter::UpDown(float NewAxisValue)
{
	switch (CurrentControlMode)
	{
	case EControlMode::TPS:
		AddMovementInput(FRotationMatrix(FRotator(0.0f,
        GetControlRotation().Yaw, 0.0f)).GetUnitAxis(EAxis::X), NewAxisValue);
		break;
	case EControlMode::QUARTERVIEW:
		DirectionToMove.X = NewAxisValue;
		break;
	}
}

void AABCharacter::LeftRight(float NewAxisValue)
{
	switch (CurrentControlMode)
	{
	case EControlMode::TPS:
		AddMovementInput(FRotationMatrix(FRotator(0.0f, 
        GetControlRotation().Yaw, 0.0f)).GetUnitAxis(EAxis::Y), NewAxisValue);
		break;
	case EControlMode::QUARTERVIEW:
		DirectionToMove.Y = NewAxisValue;
	}
}

float AABCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, 
AController* EventInstigator, AActor* DamageCauser)
{
	float FinalDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	
	CharacterStat->SetDamage(FinalDamage);
	if (CurrentState == ECharacterState::DEAD)
	{
		if (EventInstigator->IsPlayerController())
		{
			auto TempABPlayerController = Cast<AABPlayerController>(EventInstigator);
			TempABPlayerController->NPCKill(this);
		}
	}

	return FinalDamage;
}

void AABCharacter::Turn(float NewAxisValue)
{
	AddControllerYawInput(NewAxisValue);
}

void AABCharacter::LookUp(float NewAxisValue)
{
	AddControllerPitchInput(NewAxisValue);
}

void AABCharacter::ViewChange()
{
	switch (CurrentControlMode)
	{
	case EControlMode::TPS:
		GetController()->SetControlRotation(GetActorRotation());
		SetControlMode(EControlMode::QUARTERVIEW);
		break;
	case EControlMode::QUARTERVIEW:
		GetController()->SetControlRotation(SpringArm->GetRelativeRotation());
		SetControlMode(EControlMode::TPS);
		break;
	}
}
void AABCharacter::Attack()
{
	if (IsAttacking)
	{
		if (CanNextCombo)
		{
			IsComboInputOn = true;
		}
	}
	else
	{
		AttackStartComboState();
		ABAnim->PlayAttackMontage();
		ABAnim->JumpToAttackMontageSection(CurrentCombo);
		IsAttacking = true;
	}
}

void AABCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (IsPlayerControlled())
	{
		SetControlMode(EControlMode::QUARTERVIEW);
		GetCharacterMovement()->MaxWalkSpeed = 600.0f;
	}
	else
	{
		SetControlMode(EControlMode::NPC);
		GetCharacterMovement()->MaxWalkSpeed = 300.0f;
	}
}

void AABCharacter::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	IsAttacking = false;
	AttackEndComboState();
	OnAttackEnd.Broadcast();
}

void AABCharacter::AttackStartComboState()
{
	CanNextCombo   = true;
	IsComboInputOn = false;
	CurrentCombo   = FMath::Clamp<int32>(CurrentCombo + 1, 1, MaxCombo);
}

void AABCharacter::AttackEndComboState()
{
	IsComboInputOn = false;
	CanNextCombo   = false;
	CurrentCombo   = 0;
}

void AABCharacter::AttackCheck()
{
	FHitResult HitResult;
	FCollisionQueryParams Params(NAME_None, false, this);//NAME_NONE:TraceTag의이름(tracetag없음),false:간단한 검사,this:충돌 검사시 제외
	bool bResult = GetWorld()->SweepSingleByChannel
	(
		HitResult, //HitFound
		GetActorLocation(), //Start
		GetActorLocation()+GetActorForwardVector()*AttackRange,//End
		FQuat::Identity,
		ECollisionChannel::ECC_GameTraceChannel2,
		FCollisionShape::MakeSphere(AttackRadius),//CollisionShape - supports Box, Sphere, Capsule
		Params//Additional parameters used for the trace
	);
#if ENABLE_DRAW_DEBUG
	
	FVector TraceVec    = GetActorForwardVector() * AttackRange;
	FVector Center      = GetActorLocation() + TraceVec * 0.5f;
	float HalfHeight    = AttackRange * 0.5f + AttackRadius;
	FQuat CapsuleRot    = FRotationMatrix::MakeFromZ(TraceVec).ToQuat();
	FColor DrawColor    = bResult ? FColor::Green : FColor::Red;
	float DebugLifeTime = 3.0f;

	DrawDebugCapsule
	(
		GetWorld(),
		Center,
		HalfHeight,
		AttackRadius,
		CapsuleRot,
		DrawColor,
		false,
		DebugLifeTime
	);

#endif

	if (bResult)
	{
		if (HitResult.GetActor()->IsValidLowLevel())
		{
			UGameplayStatics::ApplyDamage(HitResult.GetActor(), CharacterStat->GetAttack(),
            GetController(), this, UDamageType::StaticClass());
		}
	}
}

bool AABCharacter::CanSetWeapon()
{
	return (nullptr == CurrentWeapon);
}

void AABCharacter::SetWeapon(AABWeapon* NewWeapon)
{
	FName WeaponSocket(TEXT("hand_rSocket"));
	if (nullptr != NewWeapon)
	{
		NewWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocket);
		NewWeapon->SetOwner(this);
		CurrentWeapon = NewWeapon;
	}
}

void AABCharacter::OnAssetLoadCompleted()
{
	USkeletalMesh* AssetLoaded = Cast<USkeletalMesh>(AssetStreamingHandle->GetLoadedAsset());
	AssetStreamingHandle.Reset();
	GetMesh()->SetSkeletalMesh(AssetLoaded);

	SetCharacterState(ECharacterState::READY);
}

void AABCharacter::SetCharacterState(ECharacterState NewState)
{
	CurrentState = NewState;
	switch (CurrentState)
	{
	case ECharacterState::LOADING:
	{
		if (bIsPlayer)
		{
			DisableInput(ABPlayerController);

			auto ABPlayerState = Cast<AABPlayerState>(GetPlayerState());
			CharacterStat->SetNewLevel(ABPlayerState->GetCharacterLevel());
		}
		SetActorHiddenInGame(true);
		HPBarWidget->SetHiddenInGame(true);
		SetCanBeDamaged(false);
		break;
	}
	case ECharacterState::READY:
	{
		SetActorHiddenInGame(false);
		HPBarWidget->SetHiddenInGame(false);
		SetCanBeDamaged(true);

		CharacterStat->OnHPIsZero.AddLambda([this]()->void{SetCharacterState(ECharacterState::DEAD);	});

		auto CharacterWidget = Cast<UABCharacterWidget>(HPBarWidget->GetUserWidgetObject());
		CharacterWidget->BindCharacterStat(CharacterStat);

		if(bIsPlayer)
		{
			SetControlMode(EControlMode::QUARTERVIEW);
			GetCharacterMovement()->MaxWalkSpeed = 600.0f;
			EnableInput(ABPlayerController);
		}
		else
		{
			SetControlMode(EControlMode::NPC);
			GetCharacterMovement()->MaxWalkSpeed = 300.0f;
			ABAIController->RunAI();
		}
		break;
	}
case ECharacterState::DEAD:
	{
		SetActorEnableCollision(false);
		GetMesh()->SetHiddenInGame(false);
		HPBarWidget->SetHiddenInGame(true);
		ABAnim->SetDeadAnim();
		SetCanBeDamaged(false);

		if (bIsPlayer)
			DisableInput(ABPlayerController);
		else
			ABAIController->StopAI();


		GetWorld()->GetTimerManager().SetTimer(DeadTimerHandle, FTimerDelegate::CreateLambda([this]() ->void
		{
			if (bIsPlayer)
			ABPlayerController->RestartLevel();
			else
				Destroy();
		}), DeadTimer, false);

		break;
		}
	}
}

ECharacterState AABCharacter::GetCharacterState() const
{
	return CurrentState;
}


int32 AABCharacter::GetExp() const
{
	return CharacterStat->GetDropExp();
}