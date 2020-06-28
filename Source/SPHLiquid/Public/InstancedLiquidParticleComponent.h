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


/**
 *
 */
UCLASS(hidecategories = (Object, LOD, Instances), editinlinenew, meta = (BlueprintSpawnableComponent), ClassGroup = Rendering, DisplayName = "InstancedLiquidParticleComponent")
class SPHLIQUID_API UInstancedLiquidParticleComponent : public UInstancedStaticMeshComponent
{
	GENERATED_BODY()

public:

	UInstancedLiquidParticleComponent(const FObjectInitializer& Initializer);

	virtual void InitializeComponent() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:

	UPROPERTY(EditAnywhere, Category = "SPH Config", meta = (ClampMin = 1, ClampMax = 10))
	int32 IterationCount;

	UPROPERTY(EditAnywhere, Category = "SPH Config")
	ESPHSolverType SolverType;

	TUniquePtr<FFluidSystem>       FluidSystem;
	TUniquePtr<FSPHParticles>      FluidParticles;
	TUniquePtr<FBoundaryParticles> BoundaryParticles;
	TUniquePtr<FFluidSolver>       FluidSolver;

	TArray<FVector> ParticlePositions;
};
