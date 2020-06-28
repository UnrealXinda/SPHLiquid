#pragma once

struct FFluidSystemConfig
{
	float SpaceSizeX, SpaceSizeY, SpaceSizeZ;
	float GX, GY, GZ;

	float CellLength;
	float SmoothingRadius;
	float TimeStep;
	float M0;
	float Rho0;
	float RhoBoundary;
	float Stiff;
	float Visc;
	float SurfaceTensionIntensity;
	float AirPressure;

	int CellSizeX, CellSizeY, CellSizeZ;
};

class FFluidSystem final
{
public:

	FFluidSystem(
		const class FSPHParticles&      FluidParticles,
		const class FBoundaryParticles& BoundaryParticles,
		const class FFluidSolver&       FluidSolver,
		const FFluidSystemConfig&       Config);
	~FFluidSystem();

	FFluidSystem(const FFluidSystem&) = delete;
	FFluidSystem& operator=(const FFluidSystem&) = delete;

	int Size() const;
	int FluidSize() const;
	int BoundarySize() const;
	int TotalSize() const;

	float Step();

private:

	struct FImpl;
	FImpl* Impl;
};