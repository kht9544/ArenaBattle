// Fill out your copyright notice in the Description page of Project Settings.


#include "ABSection.h"

// Sets default values
AABSection::AABSection()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MESH"));
	RootComponent = Mesh;

	static ConstructorHelpers::FObjectFinder<UStaticMesh>SM_SQUARE(TEXT("/Game/Book/StaticMesh/SM_SQUARE.SM_SQUARE"));
	if(SM_SQUARE.Succeeded())
		Mesh->SetStaticMesh(SM_SQUARE.Object);

	static ConstructorHelpers::FObjectFinder<UStaticMesh>SM_GATE(TEXT("/Game/Book/StaticMesh/SM_GATE.SM_GATE"));

	static TArray<FName> GateSockets = Mesh->GetAllSocketNames();
	
	for (auto GateSocket : GateSockets)
	{
		if (GateSocket.GetStringLength() > 2)
		{
			UStaticMeshComponent* NewGate = CreateDefaultSubobject<UStaticMeshComponent>(*GateSocket.ToString());
			NewGate->SetStaticMesh(SM_GATE.Object);
			NewGate->SetupAttachment(RootComponent, GateSocket);
			NewGate->SetRelativeLocation(FVector(0.0f, -80.5f, 0.0f));
			GateMeshes.Add(NewGate);
		}
	}


}

// Called when the game starts or when spawned
void AABSection::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AABSection::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

