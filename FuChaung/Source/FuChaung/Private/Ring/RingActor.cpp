// RingActor.cpp
#include "Ring/RingActor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"

ARingActor::ARingActor()
{
    PrimaryActorTick.bCanEverTick = true;
    
    RingBlocks = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("RingBlocks"));
    // 在构造函数中创建空根
    USceneComponent* NewRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(NewRoot);
    RingBlocks->SetupAttachment(NewRoot); // 将网格组件挂载到新根节点

    // 保持原有位置计算逻辑不变
}

void ARingActor::BeginPlay()
{
    Super::BeginPlay();
    
    Lights.Add({ 90.0f, 1.0f, 45.0f, 135.0f, 90.0f });
    Lights.Add({ 180.0f, -1.0f, 135.0f, 225.0f, 90.0f });
    Lights.Add({ 270.0f, 1.0f, 225.0f, 315.0f, 90.0f });
    Lights.Add({ 0.0f, -1.0f, 315.0f, 45.0f, 90.0f });

    GenerateRingBlocks();
    GetWorldTimerManager().SetTimer(TimerHandle, this, &ARingActor::RaiseAllBlocks, 2.0f, true);
}

void ARingActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateLights(DeltaTime);
    UpdateBlocksByLights();
    UpdateBlocksTransform();
}

void ARingActor::GenerateRingBlocks()
{
    if (!RingBlocks) return;

    BlockDataArray.Empty();
    PositionDataArray.Empty();
    RingBlocks->ClearInstances();

    UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (!CubeMesh) return;

    RingBlocks->SetStaticMesh(CubeMesh);

    const FTransform ActorTransform = GetActorTransform();
    const float MajorStep = 360.0f / TotalBlocks;
    const float MinorStep = 360.0f / 6;

    for (int32 i = 0; i < TotalBlocks; i++)
    {
        const float MajorAngle = i * MajorStep;
        const float MajorRad = FMath::DegreesToRadians(MajorAngle);

        for (int32 j = 0; j < 6; j++)
        {
            const float MinorAngle = j * MinorStep;
            const float MinorRad = FMath::DegreesToRadians(MinorAngle);

            // 计算局部坐标（相对于Actor位置）
            const FVector LocalPosition(
                (TorusMajorRadius + TorusMinorRadius * FMath::Cos(MinorRad)) * FMath::Cos(MajorRad),
                (TorusMajorRadius + TorusMinorRadius * FMath::Cos(MinorRad)) * FMath::Sin(MajorRad),
                TorusMinorRadius * FMath::Sin(MinorRad)
            );

            // 转换为世界坐标
            const FVector WorldPosition = ActorTransform.TransformPosition(LocalPosition);

            // 计算世界空间法线方向
            const FVector LocalNormal(
                FMath::Cos(MajorRad) * FMath::Cos(MinorRad),
                FMath::Sin(MajorRad) * FMath::Cos(MinorRad),
                FMath::Sin(MinorRad)
            );
            const FVector WorldNormal = ActorTransform.TransformVectorNoScale(LocalNormal).GetSafeNormal();

            FTransform Transform;
            Transform.SetLocation(WorldPosition);
            Transform.SetRotation(FRotationMatrix::MakeFromZ(WorldNormal).ToQuat());

            RingBlocks->AddInstance(Transform);
            
            BlockDataArray.Add({ MajorAngle });
            PositionDataArray.Add({ WorldNormal, WorldPosition });
        }
    }
}

void ARingActor::UpdateLights(float DeltaTime)
{
    for (FLightData& Light : Lights)
    {
        // 更新角度并归一化到0-360范围
        Light.CurrentAngle += Light.Direction * Light.Speed * DeltaTime;
        Light.CurrentAngle = FMath::Fmod(Light.CurrentAngle, 360.0f);
        if (Light.CurrentAngle < 0.0f)
            Light.CurrentAngle += 360.0f;

        bool bShouldReverse = false;
        if (Light.Direction > 0)
        {
            // 处理正方向移动
            if (Light.MinAngle <= Light.MaxAngle)
            {
                if (Light.CurrentAngle > Light.MaxAngle)
                    bShouldReverse = true;
            }
            else // 环状区间（如315°→45°）
            {
                if (Light.CurrentAngle > Light.MaxAngle && Light.CurrentAngle < Light.MinAngle)
                    bShouldReverse = true;
            }
        }
        else
        {
            // 处理负方向移动
            if (Light.MinAngle <= Light.MaxAngle)
            {
                if (Light.CurrentAngle < Light.MinAngle)
                    bShouldReverse = true;
            }
            else // 环状区间
            {
                if (Light.CurrentAngle < Light.MinAngle && Light.CurrentAngle > Light.MaxAngle)
                    bShouldReverse = true;
            }
        }

        if (bShouldReverse)
        {
            Light.Direction *= -1;
            // 钳制到边界
            Light.CurrentAngle = (Light.Direction > 0) ? Light.MinAngle : Light.MaxAngle;
            // 再次归一化
            Light.CurrentAngle = FMath::Fmod(Light.CurrentAngle, 360.0f);
            if (Light.CurrentAngle < 0.0f)
                Light.CurrentAngle += 360.0f;
        }
    }
}


void ARingActor::UpdateBlocksByLights()
{
    const float HeightChangeThreshold = 1.0f; // 高度变化阈值
    
    for (int32 i = 0; i < BlockDataArray.Num(); i++)
    {
        FRingBlockData& Block = BlockDataArray[i];
        Block.bRaisedByLight = false;

        // 光照检测（保持原逻辑）
        for (const FLightData& Light : Lights)
        {
            float Delta = FMath::Abs(FMath::Fmod(Block.MajorAngle - Light.CurrentAngle + 360.0f, 360.0f));
            if (Delta <= InfluenceRange || (360.0f - Delta) <= InfluenceRange)
            {
                Block.bRaisedByLight = true;
                break;
            }
        }

        // ▼▼▼ 修复部分开始 ▼▼▼
        if (Block.bRaisedByLight)
        {
            // 仅在未被定时器影响时执行跳动逻辑
            if (!Block.bRaisedByTimer)
            {
                // 到达目标高度后生成新随机高度
                if (FMath::IsNearlyEqual(Block.LightBounceHeight, Block.LightBounceTarget, HeightChangeThreshold))
                {
                    Block.LightBounceTarget = FMath::FRandRange(0.0f, MaxBounceHeight);
                }
            }
        }
        else
        {
            Block.LightBounceTarget = 0.0f; // 离开光照区域时复位目标
        }
        

        // 高度插值（保持原逻辑）
        Block.LightBounceHeight = FMath::FInterpTo(
            Block.LightBounceHeight,
            Block.LightBounceTarget,
            GetWorld()->GetDeltaSeconds(),
            BounceSpeed
        );
    }
}

void ARingActor::UpdateBlocksTransform()
{
    if (!RingBlocks || PositionDataArray.Num() != BlockDataArray.Num()) return;

    const FTransform ActorTransform = GetActorTransform();
    
    for (int32 i = 0; i < BlockDataArray.Num(); i++)
    {
        const FRingBlockData& Block = BlockDataArray[i];
        FBlockPositionData& PosData = PositionDataArray[i];
        const bool bRaised = Block.bRaisedByLight || Block.bRaisedByTimer;

        // 动态更新基础位置（跟随Actor移动）
        const FVector LocalPosition = ActorTransform.InverseTransformPosition(PosData.BasePosition);
        const FVector NewWorldPosition = ActorTransform.TransformPosition(LocalPosition);
        PosData.BasePosition = NewWorldPosition;

        // 更新法线方向（跟随Actor旋转）
        const FVector LocalNormal = ActorTransform.InverseTransformVectorNoScale(PosData.Normal);
        PosData.Normal = ActorTransform.TransformVectorNoScale(LocalNormal).GetSafeNormal();

        FVector FinalPosition = PosData.BasePosition;
    
        // 叠加两种高度效果
        if (Block.bRaisedByLight) {
            FinalPosition += PosData.Normal * Block.LightBounceHeight;
        }
        if (Block.bRaisedByTimer) {
            FinalPosition += PosData.Normal * Block.TimerRaiseHeight; // 使用新变量
        }

        FTransform Transform;
        Transform.SetLocation(FinalPosition);
        Transform.SetRotation(FRotationMatrix::MakeFromZ(PosData.Normal).ToQuat());

        // TODO: 此处应添加材质变更逻辑
        // 当Block.bRaisedByLight为true时切换为高亮材质
        // 当Block.bRaisedByTimer为true时切换为定时器材质
        // 使用RingBlocks->SetMaterial()实现

        
        RingBlocks->UpdateInstanceTransform(i, Transform, true);
    }
    RingBlocks->MarkRenderStateDirty();
}

void ARingActor::RaiseAllBlocks()
{
    for (int32 i = 0; i < BlockDataArray.Num(); i++)
    {
        // 安全捕获索引
        const int32 SafeIndex = i;
        
        // 设置定时器抬升参数
        BlockDataArray[SafeIndex].TimerRaiseHeight = FMath::FRandRange(0.0f, RaiseHeight);
        BlockDataArray[SafeIndex].bRaisedByTimer = true;

        // 使用WeakThis避免Actor被销毁后的空指针
        TWeakObjectPtr<ARingActor> WeakThis(this);
        FTimerHandle Handle;
        GetWorld()->GetTimerManager().SetTimer(Handle, [WeakThis, SafeIndex]() {
            if (WeakThis.IsValid() && 
                WeakThis->BlockDataArray.IsValidIndex(SafeIndex))
            {
                WeakThis->BlockDataArray[SafeIndex].bRaisedByTimer = false;
                WeakThis->BlockDataArray[SafeIndex].TimerRaiseHeight = 0.0f;
            }
        }, 1.0f, false);
    }
}