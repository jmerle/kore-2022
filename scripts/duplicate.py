import shutil
from argparse import ArgumentParser
from pathlib import Path

def main() -> None:
    parser = ArgumentParser(description="Duplicate an agent and update references to the source agent's name in the target agent.")
    parser.add_argument("source_agent", type=str, help="the name of the agent to copy, relative to the <project root>/agents directory")
    parser.add_argument("target_agent", type=str, help="the name of the new agent, relative to the <project root>/agents directory")

    args = parser.parse_args()

    agent_directory = Path(__file__).parent.parent / "agents"

    source_directory = agent_directory / args.source_agent
    if not source_directory.is_dir():
        raise ValueError(f"'{source_directory}' does not exist")

    target_directory = agent_directory / args.target_agent
    if target_directory.is_dir():
        raise ValueError(f"'{target_directory}' already exists")

    print(f"Copying '{source_directory}' to '{target_directory}'")
    shutil.copytree(source_directory, target_directory)

    for file_name in ["main.py", "main.cpp"]:
        file = target_directory / file_name

        print(f"Updating references to '{source_directory.name}' in '{file}' to '{target_directory.name}'")

        content = file.read_text(encoding="utf-8")
        content = content.replace(source_directory.name, target_directory.name)
        file.write_text(content, encoding="utf-8")

if __name__ == "__main__":
    main()
