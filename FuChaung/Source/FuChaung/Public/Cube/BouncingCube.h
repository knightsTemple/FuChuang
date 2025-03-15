// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BouncingCube.generated.h"

UCLASS()
class FUCHAUNG_API ABouncingCube : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABouncingCube();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	// 运行时计算参数
	float RunningTime = 0.0f;
	FVector InitialLocation;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	UPROPERTY(EditAnywhere, Category = "Bounce Settings" , BlueprintReadWrite)
	float Amplitude = 100.0f;  // 跳动幅度（单位：厘米）

	UPROPERTY(EditAnywhere, Category = "Bounce Settings" , BlueprintReadWrite)
	float Frequency = 1.0f;    // 跳动频率（Hz）

	UPROPERTY(EditAnywhere, Category = "Bounce Settings" , BlueprintReadWrite)
	float Phase = 0.0f;        // 初始相位（弧度）
private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* CubeMesh;

	
	
};
