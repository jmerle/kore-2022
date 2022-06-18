import shutil
from kaggle_environments.envs.kore_fleets.helpers import Configuration, Observation
from pathlib import Path
from typing import Any, Dict

IS_KAGGLE = Path("/kaggle_simulations/agent").is_dir()

if IS_KAGGLE:
    source_file = Path("/opt/conda/lib/libpython3.7m.so.1.0")
    target_file = Path.cwd() / "libpython3.7m.so.1.0"
    shutil.copyfile(source_file, target_file)

import v02

# kaggle-environments uses contextlib's redirect_stdout and redirect_stderr to capture agent logs to StringIO instances
# Unfortunately C++ extension modules bypass Python's stdout/stderr, printing straight to the console instead
# This causes the logs to be missing from the generated replays
# The wurlitzer package fixes this by piping the extension module's stdout/stderr to Python's stdout/stderr
# This package is not available on Kaggle's servers, but leaderboard episodes don't contain logs anyway
try:
    import wurlitzer
    wurlitzer_available = True
except ImportError:
    wurlitzer_available = False

# All multi-file submissions must contain a main.py file where the last function is the agent's entrypoint
# For all my agents this function simply delegates the call to the C++ extension module
def agent(obs: Observation, config: Configuration) -> Dict[str, Any]:
    if IS_KAGGLE:
        agent_directory = "/kaggle_simulations/agent"
    else:
        agent_directory = "/home/jasper/Projects/kore-2022/agents/v02"

    if wurlitzer_available:
        with wurlitzer.sys_pipes():
            actions = v02.agent(obs, config, agent_directory)
    else:
        actions = v02.agent(obs, config, agent_directory)

    actions["agent"] = "v02"
    return actions
