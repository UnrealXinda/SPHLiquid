#pragma once

enum class ESPHSolver
{
	WCSPH = 0,
	DFSPH = 1,
	PBD = 2
};

class FFluidSolver final
{
public:

	FFluidSolver(ESPHSolver SolverType, int FluidParticleSize);
	~FFluidSolver();

	FFluidSolver(const FFluidSolver&) = delete;
	FFluidSolver& operator=(const FFluidSolver&) = delete;

private:

	struct FImpl;
	FImpl* Impl;
};