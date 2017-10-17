# GPGPU-Ramulator-Simulator

The GPGPU-Sim and Ramulator source code are both included in this repository. The Original DRAM model is currently replaced by the DRAM model in Ramulator, which provides more flexible DRAM models selection for GPGPU simulation. The current integrated simulator can simulate GPUs with multiple DRAM standards: GDDR5, HBM, DDR3, DDR4, WIO, WIO2. 


## The Shoulders of Giants

GPGPU-Sim is a cycle-accurate graphics processing units (GPUs) simulator that is developed based on Nvidia Fermi architecture. GPGPU-Sim provides both functional and timing simulation results for CUDA and OPENCL workloads. The integrated simulator employs GPGPU-Sim V3.2.2 http://www.ece.ubc.ca/~aamodt/papers/gpgpusim.ispass09.pdf. The dev branch also provides the dev version of GPGPU-Sim with Ramulator.


Ramulator is a flexible DRAM simulator that can simulate different DRAM technologies. Its decoupled and modular design makes possible easily switching different DRAM standards, which enbales GPGPU-Sim with ramulator to simulate with the most state-of-the-art DRAM technology. The detailed design of Ramulator is explained at the paper http://users.ece.cmu.edu/~omutlu/pub/ramulator_dram_simulator-ieee-cal15.pdf.

## Environment Setup

The integrated simulator requires the 5.0+ compiler (gcc, g++, clang++) due to the employment of lambda function in ramulator. To satisfy the environment requirement, please refer to the README file in GPGPU-sim (https://github.com/gpgpu-sim/gpgpu-sim_distribution/blob/master/README) and in Ramulator (https://github.com/CMU-SAFARI/ramulator/blob/master/README.md). 

Currently, the simulator will read the HBM-config.cfg to generate corresponding DRAM standards. The clock of DRAM is still determined by the "DRAM Clock" gpgpu-sim.config rather than the one in the ramulator config file. in the  And the cache line size is set to 128 byte. Later, we will add flexible line size setup which is associated with the L2 cache parameter in gpgpu-sim.config. The current version is not the final version, it is still under developing and testing by star group. Currently, we are focusing:

2017-10-17
1. Make the original DRAM model in the gpgpu-sim optional, which enables researchers switch differet models (Currently, the original DRAM model is totally disabled)
2. Add GPUWattch support for the new DRAM model
3. Calibrate workloads performance with the new DRAM model (ramulator) and the original model


If you have any comments, questions, and suggestions, please contact guyo@oregonstate.edu (Yongbin Gu)
