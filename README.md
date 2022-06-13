# Kore 2022

This repository contains my code for the [Kore 2022](https://www.kaggle.com/competitions/kore-2022) competition.

My agents can be found in the [`agents/v*`](./agents) directories. The [`agents/opponents`](./agents/opponents) directory contains agents that were open-sourced by the competition's organizers and other participants, which are used to test my agents against.

## Development

All my agents are written in C++ and are called in Python as extension modules. This imposes some very specific prerequisites, because the agents have to be built using the same architecture and Python version as Kaggle's servers use.

Prerequisites:
- Linux x86
- Python 3.7
- CMake 3.16+
- GCC 9+
- GNU Tar

Building instructions:
```bash
# Install all Python dependencies
$ pip install -r requirements.txt

# Create the directory that will contain the build output
$ mkdir build

# Navigate to the directory you just created
$ cd build

# Generate build system files
$ cmake -DCMAKE_BUILD_TYPE=Release ..

# Build all agents
# Change <cpus> to the number of CPUs in your machine
$ cmake --build . --parallel <cpus>

# Each ./agents/v* directory now contains:
# - A v*.cpython-37m-x86_64-linux-gnu.so file containing the compiled extension module
# - A main.py file that defines an agent function which proxies all calls to the compiled extension module
# - A submission.tar.gz file that is submission-ready for Kaggle
# - A v*_test binary that runs the unit tests
# - A v*_benchmark binary that runs the benchmarks
# - A data directory containing data files for the agent
# - A test-data directory containing data files for the unit tests and the benchmarks
```

The [`scripts`](./scripts) directory contains various scripts that are useful during development. Their usage information can be found by running them with the `--help` flag.
