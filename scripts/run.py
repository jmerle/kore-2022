import webbrowser
from argparse import ArgumentParser
from common import format_path, get_agent_file, run_match
from pathlib import Path

def main() -> None:
    parser = ArgumentParser(description="Run a single match between two agents.")
    parser.add_argument("agent1", type=str, help="name of the first agent, relative to <build directory>/agents or <project root>/agents/opponents")
    parser.add_argument("agent2", type=str, help="name of the second agent, relative to <build directory>/agents or <project root>/agents/opponents")
    parser.add_argument("-b", "--build-directory", type=Path, default=Path(__file__).parent.parent / "cmake-build-release", help="path to the directory containing the build output (defaults to cmake-build-release)")
    parser.add_argument("-o", "--open", action="store_true", help="open the replay in Koreye 2022 afterwards (assumes the replays directory is served on port 8000)")

    args = parser.parse_args()

    agent1_file = get_agent_file(args.agent1, args.build_directory)
    agent2_file = get_agent_file(args.agent2, args.build_directory)

    print(f"Running {args.agent1} against {args.agent2}")

    match = run_match(agent1_file, agent2_file)

    print(f"Replay file: {format_path(match.replay_file)}")
    print(f"Outcome: {match.winner_reason}")

    if args.open:
        webbrowser.open(f"https://jmerle.github.io/koreye-2022/?input=http://localhost:8000/{match.replay_file.name}")

if __name__ == "__main__":
    main()
