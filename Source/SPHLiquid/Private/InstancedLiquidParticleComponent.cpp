// Fill out your copyright notice in the Description page of Project Settings.


#include "InstancedLiquidParticleComponent.h"

#include "Async/ParallelFor.h"

DECLARE_STATS_GROUP(TEXT("SPH"),              STATGROUP_SPH,        STATCAT_Advanced);
DECLARE_CYCLE_STAT (TEXT("Step"),             STAT_Step,            STATGROUP_SPH);
DECLARE_CYCLE_STAT (TEXT("Copy Back"),        STAT_CopyBack,        STATGROUP_SPH);
DECLARE_CYCLE_STAT (TEXT("Update Rendering"), STAT_UpdateRendering, STATGROUP_SPH);

UInstancedLiquidParticleComponent::UInstancedLiquidParticleComponent(const FObjectInitializer& Initializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	bWantsInitializeComponent = true;

	SystemConfig.IterationCount          = 3;
	SystemConfig.SolverType              = ESPHSolverType::WCSPH;
	SystemConfig.SizeX                   = 24;
	SystemConfig.SizeY                   = 24;
	SystemConfig.SizeZ                   = 36;
	SystemConfig.SpaceSize               = FVector::OneVector;
	SystemConfig.Spacing                 = 0.02f;
	SystemConfig.TimeStep                = 0.002f;
	SystemConfig.M0                      = 76.596750762082e-6f;
	SystemConfig.Rho0                    = 1.0f;
	SystemConfig.Stiff                   = 10.0f;
	SystemConfig.Visc                    = 5e-4f;
	SystemConfig.SurfaceTensionIntensity = 0.0001f;
	SystemConfig.AirPressure             = 0.0001f;
}

void UInstancedLiquidParticleComponent::OnRegister()
{
	Super::OnRegister();

	InitializeFluidParticles();
	InitializeFluidSolver();
	InitializeFluidSystem();
}

void UInstancedLiquidParticleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	{
		SCOPE_CYCLE_COUNTER(STAT_Step);

		for (int32 Idx = 0; Idx < SystemConfig.IterationCount; ++Idx)
		{
			FluidSystem->Step();
		}
	}

	{
		SCOPE_CYCLE_COUNTER(STAT_CopyBack);
		FluidParticles->CopyBackParticleTransforms(reinterpret_cast<float*>(PerInstanceSMData.GetData()));
	}

	// Force recreation of the render data when proxy is created
	InstanceUpdateCmdBuffer.NumEdits++;

	MarkRenderStateDirty();
}

FPrimitiveSceneProxy* UInstancedLiquidParticleComponent::CreateSceneProxy()
{
	SCOPE_CYCLE_COUNTER(STAT_UpdateRendering);
	return Super::CreateSceneProxy();
}

void UInstancedLiquidParticleComponent::InitializeFluidParticles()
{
	const FVector SpaceSize             = SystemConfig.SpaceSize;
	const float Spacing                 = SystemConfig.Spacing;
	const float SmoothingRadius         = 2.0f * Spacing;
	const float CellLength              = 1.01f * SmoothingRadius;

	const int32 SizeX                   = SystemConfig.SizeX;
	const int32 SizeY                   = SystemConfig.SizeY;
	const int32 SizeZ                   = SystemConfig.SizeZ;
	const int32 ParticleSize            = SizeX * SizeY * SizeZ;

	const int32 CellSizeX               = FMath::CeilToInt(SpaceSize.X / CellLength);
	const int32 CellSizeY               = FMath::CeilToInt(SpaceSize.Y / CellLength);
	const int32 CellSizeZ               = FMath::CeilToInt(SpaceSize.Z / CellLength);

	const int32 CompactSizeX            = 2 * CellSizeX;
	const int32 CompactSizeY            = 2 * CellSizeY;
	const int32 CompactSizeZ            = 2 * CellSizeZ;

	TArray<FVector> Positions;
	ParticlePositions.Empty();
	ParticlePositions.AddUninitialized(ParticleSize);

	for (int Z = 0; Z < SizeZ; ++Z)
	{
		for (int Y = 0; Y < SizeY; ++Y)
		{
			for (int X = 0; X < SizeX; ++X)
			{
				float PosX = 0.27f + Spacing * X;
				float PosY = 0.27f + Spacing * Y;
				float PosZ = 0.10f + Spacing * Z;

				Positions.Emplace(PosX, PosY, PosZ);
			}
		}
	}

	FluidParticles = MakeUnique<FSPHParticles>(reinterpret_cast<float*>(Positions.GetData()), ParticleSize);

	// Add instances
	ClearInstances();
	for (FVector Loc : Positions)
	{
		AddInstance(FTransform(FRotator::ZeroRotator, Loc, FVector(0.001)));
	}

	Positions.Empty();

	// Top and bottom
	for (int32 X = 0; X < CompactSizeX; ++X)
	{
		for (int32 Y = 0; Y < CompactSizeY; ++Y)
		{
			FVector Temp = FVector(X, Y, 0) / FVector(CompactSizeX - 1, CompactSizeY - 1, CompactSizeZ - 1) * SpaceSize;
			Positions.Add(0.99f * Temp + 0.005f * SpaceSize);

			Temp = FVector(X, Y, CompactSizeZ - 1) / FVector(CompactSizeX - 1, CompactSizeY - 1, CompactSizeZ - 1) * SpaceSize;
			Positions.Add(0.99f * Temp + 0.005f * SpaceSize);
		}
	}

	// Left and right
	for (int32 X = 0; X < CompactSizeX; ++X)
	{
		for (int32 Z = 0; Z < CompactSizeZ - 2; ++Z)
		{
			FVector Temp = FVector(X, 0, Z + 1) / FVector(CompactSizeX - 1, CompactSizeY - 1, CompactSizeZ - 1) * SpaceSize;
			Positions.Add(0.99f * Temp + 0.005f * SpaceSize);

			Temp = FVector(X, CompactSizeY - 1, Z + 1) / FVector(CompactSizeX - 1, CompactSizeY - 1, CompactSizeZ - 1) * SpaceSize;
			Positions.Add(0.99f * Temp + 0.005f * SpaceSize);
		}
	}

	// Front and back
	for (int32 Y = 0; Y < CompactSizeY - 2; ++Y)
	{
		for (int32 Z = 0; Z < CompactSizeZ - 2; ++Z)
		{
			FVector Temp = FVector(0, Y + 1, Z + 1) / FVector(CompactSizeX - 1, CompactSizeY - 1, CompactSizeZ - 1) * SpaceSize;
			Positions.Add(0.99f * Temp + 0.005f * SpaceSize);

			Temp = FVector(CompactSizeX - 1, Y + 1, Z + 1) / FVector(CompactSizeX - 1, CompactSizeY - 1, CompactSizeZ - 1) * SpaceSize;
			Positions.Add(0.99f * Temp + 0.005f * SpaceSize);
		}
	}

	// initiate boundary particles
	BoundaryParticles = MakeUnique<FBoundaryParticles>(reinterpret_cast<float*>(Positions.GetData()), Positions.Num());
}

void UInstancedLiquidParticleComponent::InitializeFluidSolver()
{
	ESPHSolver Solver;
	switch (SystemConfig.SolverType)
	{
	case ESPHSolverType::PBD:
		Solver = ESPHSolver::PBD;
		break;
	case ESPHSolverType::DFSPH:
		Solver = ESPHSolver::DFSPH;
		break;
	default:
		Solver = ESPHSolver::WCSPH;
		break;
	}

	FluidSolver = MakeUnique<FFluidSolver>(Solver, FluidParticles->Size());
}

void UInstancedLiquidParticleComponent::InitializeFluidSystem()
{
	const FVector Gravity          = FVector(0, 0, GetWorld()->GetGravityZ() / 100.0f);
	const float SmoothingRadius    = 2.0f * SystemConfig.Spacing;
	const float CellLength         = 1.01f * SmoothingRadius;
	const int32 CellSizeX          = FMath::CeilToInt(SystemConfig.SpaceSize.X / CellLength);
	const int32 CellSizeY          = FMath::CeilToInt(SystemConfig.SpaceSize.Y / CellLength);
	const int32 CellSizeZ          = FMath::CeilToInt(SystemConfig.SpaceSize.Z / CellLength);

	FFluidSystemConfig Config;
	Config.SpaceSizeX              = SystemConfig.SpaceSize.X;
	Config.SpaceSizeY              = SystemConfig.SpaceSize.Y;
	Config.SpaceSizeZ              = SystemConfig.SpaceSize.Z;
	Config.GX                      = Gravity.X;
	Config.GY                      = Gravity.Y;
	Config.GZ                      = Gravity.Z;
	Config.CellLength              = CellLength;
	Config.SmoothingRadius         = SmoothingRadius;
	Config.TimeStep                = SystemConfig.TimeStep;
	Config.M0                      = SystemConfig.M0;
	Config.Rho0                    = SystemConfig.Rho0;
	Config.RhoBoundary             = 1.4f * SystemConfig.Rho0;
	Config.Stiff                   = SystemConfig.Stiff;
	Config.Visc                    = SystemConfig.Visc;
	Config.SurfaceTensionIntensity = SystemConfig.SurfaceTensionIntensity;
	Config.AirPressure             = SystemConfig.AirPressure;
	Config.CellSizeX               = CellSizeX;
	Config.CellSizeY               = CellSizeY;
	Config.CellSizeZ               = CellSizeZ;

	FluidSystem = MakeUnique<FFluidSystem>(*FluidParticles, *BoundaryParticles, *FluidSolver, Config);
}
