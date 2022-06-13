import webbrowser
from argparse import ArgumentParser
from common import get_agent_file, run_match, Winner
from datetime import datetime
from multiprocessing import cpu_count, Manager, Pool, Process
from pathlib import Path
from typing import Any, List
from queue import Queue

def update_report(report_file: Path, timestamp: datetime, challenger_agent_file: Path, rows: List[Any]) -> None:
    title = f"run_against_all report for {challenger_agent_file.parent.name} at {timestamp.strftime('%Y-%m-%d %H:%M:%S')}"

    wins_as_blue = 0
    matches_played_as_blue = 0

    wins_as_red = 0
    matches_played_as_red = 0

    table_rows = ""
    for row in rows:
        opponent_agent_file = row["opponent_agent_file"]
        challenger_as_blue = row["challenger_as_blue"]
        challenger_as_red = row["challenger_as_red"]

        columns = [f"<td><b>{opponent_agent_file.parent.name}</b></td>"]

        for match, as_blue in [
            (challenger_as_blue, True),
            (challenger_as_red, False)
        ]:
            if match is not None:
                if as_blue:
                    matches_played_as_blue += 1
                else:
                    matches_played_as_red += 1

                status = {
                    Winner.AGENT1: "Win" if as_blue else "Lose",
                    Winner.AGENT2: "Lose" if as_blue else "Win",
                    Winner.DRAW: "Draw"
                }[match.winner]

                if status == "Win":
                    if as_blue:
                        wins_as_blue += 1
                    else:
                        wins_as_red += 1

                columns.append(f"""
<td class="{status.lower()}">
    <a href="https://jmerle.github.io/koreye-2022/?input=http://localhost:8000/{match.replay_file.name}" target="_blank">{status}</a>
</td>
                """.strip())
            else:
                columns.append("<td>Pending</td>")

        table_rows += f'<tr class="opponent">{"".join(columns)}</tr>'

    if matches_played_as_blue > 0:
        win_rate_as_blue = wins_as_blue / matches_played_as_blue * 100
    else:
        win_rate_as_blue = 0

    if matches_played_as_red > 0:
        win_rate_as_red = wins_as_red / matches_played_as_red * 100
    else:
        win_rate_as_red = 0

    with report_file.open("w+", encoding="utf-8") as file:
        file.write(f"""
<html>
<head>
    <title>{title}</title>

    <style>
        body {{
            font-family: sans-serif;
        }}

        table {{
            border-collapse: collapse;
        }}

        td, th {{
            border: 1px solid black;
            text-align: left;
            padding-left: 5px;
            padding-right: 5px;
        }}

        thead > tr > th {{
            border-bottom: 3px solid black;
        }}

        tr > th:first-child, tr.opponent > td:first-child {{
            border-right: 3px solid black;
        }}

        tr > td:empty {{
            border: 0;
        }}

        tbody > tr:last-child > td {{
            text-align: center;
        }}

        td.win {{
            background: #2ecc71;
        }}

        td.lose {{
            background: #e74c3c;
        }}

        td.draw {{
            background: #f1c40f;
        }}
    </style>
</head>
<body>
    <h1>{title}</h1>

    <table>
        <thead>
            <tr>
                <th>Opponent</th>
                <th>Challenger as blue</th>
                <th>Challenger as red</th>
            </tr>
        </thead>
        <tbody>
            {table_rows}
            <tr>
                <td></td>
                <td>Win rate: {win_rate_as_blue:,.2f}%</td>
                <td>Win rate: {win_rate_as_red:,.2f}%</td>
            </tr>
            <tr>
                <td></td>
                <td colspan="2">Win rate: {(win_rate_as_blue + win_rate_as_red) / 2:,.2f}%</td>
            </tr>
        </tbody>
    </table>
</body>
</html>
        """.strip() + "\n")

def queue_listener(queue: Queue, matches_to_run: List[List[Path]], report_file: Path, timestamp: datetime) -> None:
    challenger_agent_file = matches_to_run[0][0]

    rows = []
    for i in range(int(len(matches_to_run) / 2)):
        rows.append({
            "opponent_agent_file": matches_to_run[i * 2][1],
            "challenger_as_blue": None,
            "challenger_as_red": None
        })

    rows = sorted(rows, key=lambda row: row["opponent_agent_file"].parent.name)

    update_report(report_file, timestamp, challenger_agent_file, rows)

    prefixes = [f"{agent1_file.parent.name} vs {agent2_file.parent.name}:" for agent1_file, agent2_file in matches_to_run]
    prefix_length = max(len(prefix) for prefix in prefixes)

    for _ in range(len(matches_to_run)):
        agent1_file, agent2_file, match = queue.get()

        message = f"{agent1_file.parent.name} vs {agent2_file.parent.name}:"
        message += " " * (prefix_length - len(message) + 1)
        message += match.winner_reason
        print(message)

        if agent1_file == challenger_agent_file:
            challenger_side = "blue"
            opponent_agent_file = agent2_file
        else:
            challenger_side = "red"
            opponent_agent_file = agent1_file

        for row in rows:
            if row["opponent_agent_file"] == opponent_agent_file:
                row[f"challenger_as_{challenger_side}"] = match
                break

        update_report(report_file, timestamp, challenger_agent_file, rows)

def match_runner(queue: Queue, agent1_file: Path, agent2_file: Path) -> None:
    match = run_match(agent1_file, agent2_file)
    queue.put((agent1_file, agent2_file, match))

def main() -> None:
    parser = ArgumentParser(description="Run matches between an agent and all other agents.")
    parser.add_argument("agent", type=str, help="name of the agent to run against all others, relative to <build directory>/agents or <project root>/agents/opponents")
    parser.add_argument("-b", "--build-directory", type=Path, default=Path(__file__).parent.parent / "cmake-build-release", help="path to the directory containing the build output (defaults to cmake-build-release)")
    parser.add_argument("-o", "--open", action="store_true", help="open the report in the browser afterwards")

    args = parser.parse_args()

    challenger_agent_file = get_agent_file(args.agent, args.build_directory)

    matches_to_run = []
    for agents_directory in [
        args.build_directory / "agents",
        Path(__file__).parent.parent / "agents" / "opponents"
    ]:
        for agent_directory in agents_directory.iterdir():
            if not agent_directory.is_dir():
                continue

            opponent_agent_file = agent_directory / "main.py"
            if not opponent_agent_file.is_file() or opponent_agent_file.resolve() == challenger_agent_file.resolve():
                continue

            matches_to_run.append([challenger_agent_file, opponent_agent_file])
            matches_to_run.append([opponent_agent_file, challenger_agent_file])

    timestamp = datetime.now()

    report_file = Path(__file__).parent.parent / "reports" / f"{args.agent}-vs-all-at-{timestamp.strftime('%Y-%m-%d_%H-%M-%S')}.html"
    report_file.parent.mkdir(parents=True, exist_ok=True)

    print(f"Report file: file://{report_file.resolve()}")

    manager = Manager()
    queue = manager.Queue()

    queue_listener_process = Process(target=queue_listener, args=(queue, matches_to_run, report_file, timestamp))
    queue_listener_process.start()

    with Pool(max(1, cpu_count() - 1)) as pool:
        pool.starmap(match_runner, [(queue, agent1_file, agent2_file) for agent1_file, agent2_file in matches_to_run])

    queue_listener_process.join()

    if args.open:
        webbrowser.open(f"file://{report_file.resolve()}")

if __name__ == "__main__":
    main()
