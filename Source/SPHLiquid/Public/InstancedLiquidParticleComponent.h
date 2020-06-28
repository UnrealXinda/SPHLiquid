// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "FluidSystem.h"
#include "FluidParticles.h"
#include "FluidSolver.h"
#include "InstancedLiquidParticleComponent.generated.h"

UENUM()
enum class ESPHSolverType
{
	WCSPH = 0 UMETA(DisplayName = "WCSPH"),
	DFSPH = 1 UMETA(DisplayName = "DFSPH"),
	PBD   = 2 UMETA(DisplayName = "PBD"),
};

USTRUCT(BlueprintType)
struct FSPHSystemConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	ESPHSolverType SolverType;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 1, ClampMax = 10))
	int32 IterationCount;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 1, ClampMax = 100))
	int32 SizeX;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 1, ClampMax = 100))
	int32 SizeY;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 1, ClampMax = 100))
	int32 SizeZ;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.0))
	FVector SpaceSize;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.0))
	float Spacing;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.0))
	float TimeStep;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.0))
	float M0;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.0))
	float Rho0;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.0))
	float Stiff;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.0))
	float Visc;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.0))
	float SurfaceTensionIntensity;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.0))
	float AirPressure;
};

/**
 *
 */
UCLASS(hidecategories = (Object, LOD, Instances), editinlinenew, meta = (BlueprintSpawnableComponent), ClassGroup = Rendering, DisplayName = "InstancedLiquidParticleComponent")
class SPHLIQUID_API UInstancedLiquidParticleComponent : public UInstancedStaticMeshComponent
{
	GENERATED_BODY()

public:

	UInstancedLiquidParticleComponent(const FObjectInitializer& Initializer);

	virtual void OnRegister() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SPH Config")
	FSPHSystemConfig SystemConfig;

	TUniquePtr<FFluidSystem>       FluidSystem;
	TUniquePtr<FSPHParticles>      FluidParticles;
	TUniquePtr<FBoundaryParticles> BoundaryParticles;
	TUniquePtr<FFluidSolver>       FluidSolver;

	TArray<FVector>                ParticlePositions;

protected:

	void InitializeFluidParticles();
	void InitializeFluidSolver();
	void InitializeFluidSystem();

};
