// Fill out your copyright notice in the Description page of Project Settings.


#include "Cube/BouncingCube.h"

// Sets default values
ABouncingCube::ABouncingCube()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.016f; // ~60 FPS

	// 创建静态网格组件
	CubeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CubeMesh"));
	RootComponent = CubeMesh;
}

// Called when the game starts or when spawned
void ABouncingCube::BeginPlay()
{
	Super::BeginPlay();

	// 记录初始位置
	InitialLocation = GetActorLocation();
    
	// 随机化相位
	Phase = FMath::RandRange(0.0f, 2 * PI);
}

// Called every frame
void ABouncingCube::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RunningTime += DeltaTime;
    
	// 计算Z轴位置
	float ZOffset = Amplitude * FMath::Sin(Frequency * 2 * PI * RunningTime + Phase);
    
	// 更新位置
	FVector NewLocation = InitialLocation;
	NewLocation.Z += ZOffset;
	SetActorLocation(NewLocation);
}

