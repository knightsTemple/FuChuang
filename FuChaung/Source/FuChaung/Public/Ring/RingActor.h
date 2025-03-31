// RingActor.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RingActor.generated.h"

// 环形方块数据结构
USTRUCT()
struct FRingBlockData
{
	GENERATED_BODY()

	// 方块在环上的角度位置
	float MajorAngle = 0.0f;
    
	// 是否被光照抬升
	bool bRaisedByLight = false;
	// 光照抬升高度
	float LightBounceHeight = 0.0f;
	// 光照抬升目标高度
	float LightBounceTarget = 0.0f;
    
	// 是否被定时器抬升
	bool bRaisedByTimer = false;
	// 定时器抬升高度
	float TimerRaiseHeight = 0.0f;  
};

// 光照数据结构
USTRUCT()
struct FLightData
{
	GENERATED_BODY()

	// 当前角度
	float CurrentAngle = 0.0f;
	// 移动方向（1为正方向，-1为负方向）
	float Direction = 1.0f;
	// 最小角度
	float MinAngle = 0.0f;
	// 最大角度
	float MaxAngle = 0.0f;
	// 移动速度
	float Speed = 90.0f;
};

// 方块位置数据结构
USTRUCT()
struct FBlockPositionData
{
	GENERATED_BODY()
    
	// 法线方向
	FVector Normal;
	// 基础位置
	FVector BasePosition;
};

// 环形Actor类
UCLASS()
class FUCHAUNG_API ARingActor : public AActor
{
	GENERATED_BODY()
    
public:    
	ARingActor();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// 环形方块组件
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UInstancedStaticMeshComponent* RingBlocks;

	// 环形主半径
	UPROPERTY(EditAnywhere, Category="Settings", meta=(ClampMin="100"))
	float TorusMajorRadius = 500.0f;

	// 环形次半径
	UPROPERTY(EditAnywhere, Category="Settings", meta=(ClampMin="10"))
	float TorusMinorRadius = 50.0f;

	// 总方块数量
	UPROPERTY(EditAnywhere, Category="Settings")
	int32 TotalBlocks = 120;

	// 影响范围
	UPROPERTY(EditAnywhere, Category="Settings")
	float InfluenceRange = 10.0f;

	// 抬升高度
	UPROPERTY(EditAnywhere, Category="Settings")
	float RaiseHeight = 50.0f;

	// 最大弹跳高度
	UPROPERTY(EditAnywhere, Category="Settings")
	float MaxBounceHeight = 50.0f;

	// 弹跳速度
	UPROPERTY(EditAnywhere, Category="Settings", meta=(ClampMin="0.1"))
	float BounceSpeed = 300.0f;

private:
	// 方块数据数组
	TArray<FRingBlockData> BlockDataArray;
	// 位置数据数组
	TArray<FBlockPositionData> PositionDataArray;
	// 光照数据数组
	TArray<FLightData> Lights;
	// 定时器句柄
	FTimerHandle TimerHandle;

	// 生成环形方块
	void GenerateRingBlocks();
	// 更新光照
	void UpdateLights(float DeltaTime);
	// 根据光照更新方块
	void UpdateBlocksByLights();
	// 更新方块变换
	void UpdateBlocksTransform();
	// 抬升所有方块
	void RaiseAllBlocks();
};