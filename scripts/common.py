import json
import re
import subprocess
from dataclasses import dataclass
from datetime import datetime
from pathlib import Path
from typing import Optional

class Winner:
    AGENT1 = 0
    AGENT2 = 1
    DRAW = 2

@dataclass
class Match:
    replay_file: Path
    winner: Winner
    winner_reason: str

def get_agent_file(name: str, build_directory: Path) -> Path:
    for file in [
        build_directory / "agents" / name / "main.py",
        Path(__file__).parent.parent / "agents" / "opponents" / name / "main.py"
    ]:
        if file.is_file():
            return file

    raise ValueError(f"Agent '{name}' does not exist")

def format_path(path: Path) -> str:
    try:
        return str(path.relative_to(Path.cwd()))
    except ValueError:
        return str(path)

def run_match(agent1_file: Path, agent2_file: Path, index: Optional[int] = None) -> Match:
    agent1 = agent1_file.parent.name
    agent2 = agent2_file.parent.name

    timestamp = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
    replay_file = Path(__file__).parent.parent / "replays" / f"{agent1}-vs-{agent2}-at-{timestamp}{f'-{index}' if index is not None else ''}.html"
    replay_file.parent.mkdir(parents=True, exist_ok=True)

    args = [
        "kaggle-environments", "run",
        "--environment", "kore_fleets",
        "--agents", str(agent1_file), str(agent2_file),
        "--display", "html",
        "--out", str(replay_file)
    ]

    proc = subprocess.run(args)
    if proc.returncode != 0:
        raise RuntimeError(f"kaggle-environments exited with error code {proc.returncode} while running {agent1} against {agent2} to {format_path(replay_file)}")

    replay_content = replay_file.read_text(encoding="utf-8")
    if replay_content.startswith("{"):
        raise RuntimeError(eval(replay_content)["error"])

    replay_content = replay_content.replace('"info": {}', f'"info": {{ "TeamNames": ["{agent1}", "{agent2}"] }}')
    replay_file.write_text(replay_content, encoding="utf-8")

    episode = json.loads(re.search(r'"environment":\s*({.*}),\s*"logs":\s*\[', replay_content, flags=re.S).group(1))
    last_step = episode["steps"][-1]

    agent1_status = last_step[0]["status"]
    agent2_status = last_step[1]["status"]

    last_players = last_step[0]["observation"]["players"]
    agent1_has_entities = len(last_players[0][1]) + len(last_players[0][2]) > 0
    agent2_has_entities = len(last_players[1][1]) + len(last_players[1][2]) > 0

    agent1_reward = last_step[0]["reward"]
    agent2_reward = last_step[1]["reward"]

    agent1_label = f"{agent1}" if agent1 != agent2 else f"Agent 1 ({agent1})"
    agent2_label = f"{agent2}" if agent1 != agent2 else f"Agent 2 ({agent2})"

    if agent1_status == "DONE" and agent2_status == "ERROR":
        winner = Winner.AGENT1
        winner_reason = f"{agent1_label} wins by not crashing"
    elif agent1_status == "ERROR" and agent2_status == "DONE":
        winner = Winner.AGENT2
        winner_reason = f"{agent2_label} wins by not crashing"
    elif agent1_status == "ERROR" and agent2_status == "ERROR":
        winner = Winner.DRAW
        winner_reason = "Draw, both crashed"
    elif agent1_status == "DONE" and agent2_status == "TIMEOUT":
        winner = Winner.AGENT1
        winner_reason = f"{agent1_label} wins by not timing out"
    elif agent1_status == "TIMEOUT" and agent2_status == "DONE":
        winner = Winner.AGENT2
        winner_reason = f"{agent2_label} wins by not timing out"
    elif agent1_status == "TIMEOUT" and agent2_status == "TIMEOUT":
        winner = Winner.DRAW
        winner_reason = "Draw, both timed out"
    elif agent1_has_entities and not agent2_has_entities:
        winner = Winner.AGENT1
        winner_reason = f"{agent1_label} wins by elimination"
    elif not agent1_has_entities and agent2_has_entities:
        winner = Winner.AGENT2
        winner_reason = f"{agent2_label} wins by elimination"
    elif not agent1_has_entities and not agent2_has_entities:
        winner = Winner.DRAW
        winner_reason = "Draw, both eliminated"
    elif agent1_reward > agent2_reward:
        winner = Winner.AGENT1
        winner_reason = f"{agent1_label} wins by kore"
    elif agent1_reward < agent2_reward:
        winner = Winner.AGENT2
        winner_reason = f"{agent2_label} wins by kore"
    else:
        winner = Winner.DRAW
        winner_reason = "Draw, same kore"

    return Match(replay_file, winner, winner_reason)
