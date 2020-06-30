# SPHLiquid

SPHLiquid is a UE4 plugin for SPH fluid simulation. The actual simulation is a UE4 port of [Troy Zhai](https://github.com/TroyZhai/CPP-Fluid-Particles)'s awesome CUDA implementation of SPH fluid sim.

## How To Use
To run this plugin, please first build the [forked branch](https://github.com/UnrealXinda/CPP-Fluid-Particles). Remember to build the project as static library, not as executable file.  
Then copy the build library under LiquidSimLib/lib/x64. Note that if you have changes in the interface files (FluidSystem.h, FluidParticles.h, FluidSolver.h), you will also need to copy them to overwrite counterparts under LiquidSimLib/include.  https://www.youtube.com/watch?v=7ojILF1Dmro&t=0s
Finally, make changes to Build.cs file to make sure everything is referenced correctly.  

## Sample Result
![sph_liquid](sph_liquid.gif)

## Sample Video
[![](https://img.youtube.com/vi/7ojILF1Dmro/0.jpg)](https://www.youtube.com/watch?v=7ojILF1Dmro)
