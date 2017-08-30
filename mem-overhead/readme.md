# Hello with makefile

    openssl genrsa -out Enclave/Enclave_private.pem -3 3072
    make
    ./memory-overhead

Actually, there are many different build mode supported by this makefile:

* Hardware Mode, Debug build: `make`
* Hardware Mode, Pre-release build: `make SGX_PRERELEASE=1 SGX_DEBUG=0`
* Hardware Mode, Release build: `make SGX_DEBUG=0`
* Simulation Mode, Debug build: `make SGX_MODE=SIM`
* Simulation Mode, Pre-release build: `make SGX_MODE=SIM SGX_PRERELEASE=1 SGX_DEBUG=0`
* Simulation Mode, Release build: `make SGX_MODE=SIM SGX_DEBUG=0`
