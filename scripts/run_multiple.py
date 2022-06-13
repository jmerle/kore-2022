import webbrowser
from argparse import ArgumentParser
from common import get_agent_file, run_match, Winner
from datetime import datetime
from multiprocessing import cpu_count, Manager, Pool, Process
from pathlib import Path
from typing import Any, List
from queue import Queue

def update_report(report_file: Path, timestamp: datetime, agent1_file: Path, agent2_file: Path, rows: List[Any]) -> None:
    title = f"run_multiple report for {len(rows)} matches of {agent1_file.parent.name} against {agent2_file.parent.name} at {timestamp.strftime('%Y-%m-%d %H:%M:%S')}"

    agent1_wins = 0
    agent2_wins = 0
    completed_matches = 0

    table_rows = ""
    for row in rows:
        file1 = row["file1"]
        file2 = row["file2"]
        result = row["result"]

        columns = [f"<td>{file1.parent.name}</td>", f"<td>{file2.parent.name}</td>"]

        if result is not None:
            clazz = {
                Winner.AGENT1: "win" if file1 == agent1_file else "lose",
                Winner.AGENT2: "lose" if file1 == agent1_file else "win",
                Winner.DRAW: "draw"
            }[result.winner]

            if clazz == "win":
                agent1_wins += 1
            elif clazz == "lose":
                agent2_wins += 1

            completed_matches += 1

            columns.append(f"""
<td class="{clazz}">
    <a href="https://jmerle.github.io/koreye-2022/?input=http://localhost:8000/{result.replay_file.name}" target="_blank">
        {result.winner_reason}
    </a>
</td>
            """.strip())
        else:
            columns.append("<td>Pending</td>")

        table_rows += f'<tr>{"".join(columns)}</tr>'

    agent1_win_rate = 0 if completed_matches == 0 else (agent1_wins / completed_matches) * 100
    agent2_win_rate = 0 if completed_matches == 0 else (agent2_wins / completed_matches) * 100

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

        tr > th:nth-child(2), tr > td:nth-child(2) {{
            border-right: 3px solid black;
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
                <th>Agent 1</th>
                <th>Agent 2</th>
                <th>Result</th>
            </tr>
        </thead>
        <tbody>
            {table_rows}
        </tbody>
    </table>

    <p>
        {agent1_file.parent.name} win rate: {agent1_win_rate:,.2f}%
        <br/>
        {agent2_file.parent.name} win rate: {agent2_win_rate:,.2f}%
    </p>
</body>
</html>
        """.strip() + "\n")

def queue_listener(queue: Queue, agent1_file: Path, agent2_file: Path, matches_to_run: List[List[Path]], report_file: Path, timestamp: datetime) -> None:
    rows = []
    for match in matches_to_run:
        rows.append({
            "file1": match[0],
            "file2": match[1],
            "result": None
        })

    update_report(report_file, timestamp, agent1_file, agent2_file, rows)

    prefixes = [f"{file1.parent.name} vs {file2.parent.name}:" for file1, file2, _ in matches_to_run]
    prefix_length = max(len(prefix) for prefix in prefixes)

    for _ in range(len(matches_to_run)):
        file1, file2, match = queue.get()

        message = f"{file1.parent.name} vs {file2.parent.name}:"
        message += " " * (prefix_length - len(message) + 1)
        message += match.winner_reason
        print(message)

        first_row = next(row for row in rows if row["file1"] == file1 and row["file2"] == file2 and row["result"] is None)
        first_row["result"] = match

        update_report(report_file, timestamp, agent1_file, agent2_file, rows)

def match_runner(queue: Queue, agent1_file: Path, agent2_file: Path, index: int) -> None:
    match = run_match(agent1_file, agent2_file, index)
    queue.put((agent1_file, agent2_file, match))

def main() -> None:
    parser = ArgumentParser(description="Run multiple matches between two agents.")
    parser.add_argument("agent1", type=str, help="name of the first agent, relative to <build directory>/agents or <project root>/agents/opponents")
    parser.add_argument("agent2", type=str, help="name of the second agent, relative to <build directory>/agents or <project root>/agents/opponents")
    parser.add_argument("count", type=int, help="number of matches to run")
    parser.add_argument("-b", "--build-directory", type=Path, default=Path(__file__).parent.parent / "cmake-build-release", help="path to the directory containing the build output (defaults to cmake-build-release)")
    parser.add_argument("-o", "--open", action="store_true", help="open the report in the browser afterwards")

    args = parser.parse_args()

    agent1_file = get_agent_file(args.agent1, args.build_directory)
    agent2_file = get_agent_file(args.agent2, args.build_directory)

    print(f"Running {args.count} matches of {args.agent1} against {args.agent2}")

    matches_to_run = []
    for i in range(args.count):
        if i < args.count / 2:
            matches_to_run.append([agent1_file, agent2_file, i])
        else:
            matches_to_run.append([agent2_file, agent1_file, i])

    timestamp = datetime.now()

    report_file = Path(__file__).parent.parent / "reports" / f"{args.agent1}-vs-{args.agent2}-at-{timestamp.strftime('%Y-%m-%d_%H-%M-%S')}.html"
    report_file.parent.mkdir(parents=True, exist_ok=True)

    print(f"Report file: file://{report_file.resolve()}")

    manager = Manager()
    queue = manager.Queue()

    queue_listener_process = Process(target=queue_listener, args=(queue, agent1_file, agent2_file, matches_to_run, report_file, timestamp))
    queue_listener_process.start()

    with Pool(max(1, cpu_count() - 1)) as pool:
        pool.starmap(match_runner, [(queue, file1, file2, index) for file1, file2, index in matches_to_run])

    queue_listener_process.join()

    if args.open:
        webbrowser.open(f"file://{report_file.resolve()}")

if __name__ == "__main__":
    main()
