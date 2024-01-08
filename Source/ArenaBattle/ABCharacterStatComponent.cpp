// Fill out your copyright notice in the Description page of Project Settings.


#include "ABCharacterStatComponent.h"
#include "ABGameInstance.h"

// Sets default values for this component's properties
UABCharacterStatComponent::UABCharacterStatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;

	Level = 1;
	// ...
}


// Called when the game starts
void UABCharacterStatComponent::BeginPlay()
{
	Super::BeginPlay();
	SetNewLevel(Level);
	// ...
	
}

void UABCharacterStatComponent::SetNewLevel(int32 NewLevel)
{
	auto ABGameInstance = Cast<UABGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));

	if (ABGameInstance)
	{
		CurrentStatData = ABGameInstance->GetABCharacterData(NewLevel);

		if (CurrentStatData)
		{
			Level = NewLevel;
			SetHP(CurrentStatData->MaxHP);
		}
		else
		{
			// 처리할 수 없는 레벨이거나 데이터가 없는 경우에 대한 예외 처리 추가
			UE_LOG(LogTemp, Error, TEXT("Cannot find data for level %d"), NewLevel);
		}
	}
	else
	{
		// ABGameInstance가 null인 경우에 대한 예외 처리 추가
		UE_LOG(LogTemp, Error, TEXT("ABGameInstance is null"));
	}
}

void UABCharacterStatComponent::InitializeComponent()
{
	Super::InitializeComponent();
	SetNewLevel(Level);
}

void UABCharacterStatComponent::SetHP(float NewHP)
{
	CurrentHP = NewHP;
	OnHPChanged.Broadcast();
	if (CurrentHP < KINDA_SMALL_NUMBER)
	{
		CurrentHP = 0.0f;
		OnHPIsZero.Broadcast();
	}
}



void UABCharacterStatComponent::SetDamage(float NewDamage)
{
	SetHP(FMath::Clamp<float>(CurrentHP - NewDamage, 0.0f, CurrentStatData->MaxHP));
}

float UABCharacterStatComponent::GetAttack()
{
	return CurrentStatData->Attack;
}

float UABCharacterStatComponent::GetHPRatio()
{
	return (CurrentStatData->MaxHP < KINDA_SMALL_NUMBER) ? 0.0f : (CurrentHP / CurrentStatData->MaxHP);
}

// Called every frame
void UABCharacterStatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

