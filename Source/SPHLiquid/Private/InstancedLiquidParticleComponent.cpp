// Fill out your copyright notice in the Description page of Project Settings.


#include "InstancedLiquidParticleComponent.h"

#include "Async/ParallelFor.h"

DECLARE_STATS_GROUP(TEXT("sph"), STATGROUP_SPH, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Step"), STAT_Step, STATGROUP_SPH);
DECLARE_CYCLE_STAT(TEXT("Copy Back"), STAT_CopyBack, STATGROUP_SPH);

UInstancedLiquidParticleComponent::UInstancedLiquidParticleComponent(const FObjectInitializer& Initializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	bWantsInitializeComponent = true;

	IterationCount = 3;
	SolverType = ESPHSolverType::WCSPH;
}

void UInstancedLiquidParticleComponent::InitializeComponent()
{
	const FVector spaceSize = FVector(3, 1, 1);
	const FVector sphG = FVector(0.0f, -9.8f, 0.0f);
	const float sphSpacing = 0.02f;
	const float sphSmoothingRadius = 2.0f * sphSpacing;
	const float sphCellLength = 1.01f * sphSmoothingRadius;
	const float dt = 0.002f;
	const float sphRho0 = 1.0f;
	const float sphRhoBoundary = 1.4f * sphRho0;
	const float sphM0 = 76.596750762082e-6f;
	const float sphStiff = 10.0f;
	const float sphVisc = 5e-4f;
	const float sphSurfaceTensionIntensity = 0.0001f;
	const float sphAirPressure = 0.0001f;

	const int32 SizeX = 24;
	const int32 SizeY = 36;
	const int32 SizeZ = 24;
	const int32 ParticleSize = SizeX * SizeY * SizeZ;

	const int32 CellSizeX = FMath::CeilToInt(spaceSize.X / sphCellLength);
	const int32 CellSizeY = FMath::CeilToInt(spaceSize.Y / sphCellLength);
	const int32 CellSizeZ = FMath::CeilToInt(spaceSize.Z / sphCellLength);

	const int32 CompactSizeX = 2 * CellSizeX;
	const int32 CompactSizeY = 2 * CellSizeY;
	const int32 CompactSizeZ = 2 * CellSizeZ;

	TArray<FVector> Positions;
	ParticlePositions.Empty();
	ParticlePositions.AddUninitialized(ParticleSize);

	for (int Y = 0; Y < SizeY; ++Y)
	{
		for (int Z = 0; Z < SizeZ; ++Z)
		{
			for (int X = 0; X < SizeX; ++X)
			{
				float PosX = 0.27f + sphSpacing * X;
				float PosY = 0.10f + sphSpacing * Y;
				float PosZ = 0.27f + sphSpacing * Z;

				Positions.Emplace(PosX, PosY, PosZ);
			}
		}
	}

	//for (auto i = 0; i < 36; ++i)
	//{
	//	for (auto j = 0; j < 24; ++j)
	//	{
	//		for (auto k = 0; k < 24; ++k)
	//		{
	//			float PosX = 0.27f + sphSpacing * j;
	//			float PosY = 0.10f + sphSpacing * i;
	//			float PosZ = 0.27f + sphSpacing * k;

	//			Positions.Emplace(PosX, PosY, PosZ);
	//		}
	//	}
	//}

	FluidParticles = MakeUnique<FSPHParticles>(reinterpret_cast<float*>(Positions.GetData()), ParticleSize);

	for (FVector Loc : Positions)
	{
		AddInstance(FTransform(FRotator::ZeroRotator, Loc, FVector(0.001)));
	}

	Positions.Empty();

	// Front and back
	for (int32 X = 0; X < CompactSizeX; ++X)
	{
		for (int32 Y = 0; Y < CompactSizeY; ++Y)
		{
			FVector Temp = FVector(X, Y, 0) / FVector(CompactSizeX - 1, CompactSizeY - 1, CompactSizeZ - 1) * spaceSize;
			Positions.Add(0.99f * Temp + 0.005f * spaceSize);

			Temp = FVector(X, Y, CompactSizeZ - 1) / FVector(CompactSizeX - 1, CompactSizeY - 1, CompactSizeZ - 1) * spaceSize;
			Positions.Add(0.99f * Temp + 0.005f * spaceSize);
		}
	}

	// Top and bottom
	for (int32 X = 0; X < CompactSizeX; ++X)
	{
		for (int32 Z = 0; Z < CompactSizeZ - 2; ++Z)
		{
			FVector Temp = FVector(X, 0, Z + 1) / FVector(CompactSizeX - 1, CompactSizeY - 1, CompactSizeZ - 1) * spaceSize;
			Positions.Add(0.99f * Temp + 0.005f * spaceSize);

			Temp = FVector(X, CompactSizeY - 1, Z + 1) / FVector(CompactSizeX - 1, CompactSizeY - 1, CompactSizeZ - 1) * spaceSize;
			Positions.Add(0.99f * Temp + 0.005f * spaceSize);
		}
	}

	// Left and right
	for (int32 Y = 0; Y < CompactSizeY - 2; ++Y)
	{
		for (int32 Z = 0; Z < CompactSizeZ - 2; ++Z)
		{
			FVector Temp = FVector(0, Y + 1, Z + 1) / FVector(CompactSizeX - 1, CompactSizeY - 1, CompactSizeZ - 1) * spaceSize;
			Positions.Add(0.99f * Temp + 0.005f * spaceSize);

			Temp = FVector(CompactSizeX - 1, Y + 1, Z + 1) / FVector(CompactSizeX - 1, CompactSizeY - 1, CompactSizeZ - 1) * spaceSize;
			Positions.Add(0.99f * Temp + 0.005f * spaceSize);
		}
	}

	// initiate boundary particles
	BoundaryParticles = MakeUnique<FBoundaryParticles>(reinterpret_cast<float*>(Positions.GetData()), Positions.Num());

	ESPHSolver Solver;
	switch (SolverType)
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

	FluidSolver = MakeUnique<FFluidSolver>(Solver, ParticleSize);

	FFluidSystemConfig Config;
	Config.SpaceSizeX = spaceSize.X;
	Config.SpaceSizeY = spaceSize.Y;
	Config.SpaceSizeZ = spaceSize.Z;
	Config.GX = sphG.X;
	Config.GY = sphG.Y;
	Config.GZ = sphG.Z;
	Config.CellLength = sphCellLength;
	Config.SmoothingRadius = sphSmoothingRadius;
	Config.TimeStep = dt;
	Config.M0 = sphM0;
	Config.Rho0 = sphRho0;
	Config.RhoBoundary = sphRhoBoundary;
	Config.Stiff = sphStiff;
	Config.Visc = sphVisc;
	Config.SurfaceTensionIntensity = sphSurfaceTensionIntensity;
	Config.AirPressure = sphAirPressure;
	Config.CellSizeX = CellSizeX;
	Config.CellSizeY = CellSizeY;
	Config.CellSizeZ = CellSizeZ;

	FluidSystem = MakeUnique<FFluidSystem>(*FluidParticles, *BoundaryParticles, *FluidSolver, Config);
}

void UInstancedLiquidParticleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	{
		SCOPE_CYCLE_COUNTER(STAT_Step);

		for (int32 Idx = 0; Idx < IterationCount; ++Idx)
		{
			FluidSystem->Step();
		}
	}

	{
		//FluidParticles->CopyBackParticlePositions(reinterpret_cast<float*>(ParticlePositions.GetData()));

		//ParallelFor(FluidParticles->Size(), [this](int32 Index)
		//{
		//	FMatrix& Mat = PerInstanceSMData[Index].Transform;
		//	FVector& Loc = ParticlePositions[Index];

		//	Mat.M[0][0] = 1.0f; Mat.M[0][1] = 0.0f; Mat.M[0][2] = 0.0f; Mat.M[0][3] = 0.0f;
		//	Mat.M[1][0] = 0.0f; Mat.M[1][1] = 1.0f; Mat.M[1][2] = 0.0f; Mat.M[1][3] = 0.0f;
		//	Mat.M[2][0] = 0.0f; Mat.M[2][1] = 0.0f; Mat.M[2][2] = 1.0f; Mat.M[2][3] = 0.0f;
		//	Mat.M[3][0] = Loc.X * 1000; Mat.M[3][1] = Loc.Y * 1000; Mat.M[3][2] = Loc.Z * 1000; Mat.M[3][3] = 1.0f;
		//	//FTransform Transform(Loc * 1000);
		//	//Mat = Transform.ToMatrixWithScale();
		//});

		SCOPE_CYCLE_COUNTER(STAT_CopyBack);
		FluidParticles->CopyBackParticleTransforms(reinterpret_cast<float*>(PerInstanceSMData.GetData()));
	}

	// Force recreation of the render data when proxy is created
	InstanceUpdateCmdBuffer.NumEdits++;

	MarkRenderStateDirty();
}
