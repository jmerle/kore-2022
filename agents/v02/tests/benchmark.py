import json
from copy import deepcopy
from IPython import get_ipython
from kaggle_environments.envs.kore_fleets.kore_fleets import Board, ShipyardAction
from pathlib import Path

def create_board(data_file: str, step: int) -> Board:
    data_file = Path(__file__).parent / "data" / "36310051.json"
    data = json.loads(data_file.read_text(encoding="utf-8"))

    board = Board(data["steps"][step][0]["observation"], data["configuration"])

    if step < len(data["steps"]) - 1:
        for step_part in data["steps"][step + 1]:
            for shipyard_id, action in step_part["action"].items():
                board.shipyards[shipyard_id].next_action = ShipyardAction.from_str(action)

    return board

def simulate_multiple_steps(board: Board, steps: int) -> None:
    current_board = board
    for _ in range(steps):
        current_board = current_board.next()

def copy_36310051_50() -> None:
    board = create_board("36310051.json", 49)

    get_ipython().run_line_magic("timeit", "deepcopy(board)")

def copy_36310051_250() -> None:
    board = create_board("36310051.json", 249)

    get_ipython().run_line_magic("timeit", "deepcopy(board)")

def simulate_36310051_50_to_51() -> None:
    board = create_board("36310051.json", 49)

    get_ipython().run_line_magic("timeit", "board.next()")

def simulate_36310051_283_to_284() -> None:
    board = create_board("36310051.json", 282)

    get_ipython().run_line_magic("timeit", "board.next()")

def simulate_36310051_50_to_100() -> None:
    board = create_board("36310051.json", 49)

    get_ipython().run_line_magic("timeit", "simulate_multiple_steps(board, 50)")

def simulate_36310051_250_to_300() -> None:
    board = create_board("36310051.json", 249)

    get_ipython().run_line_magic("timeit", "simulate_multiple_steps(board, 50)")

def main() -> None:
    funcs = [copy_36310051_50,
             copy_36310051_250,
             simulate_36310051_50_to_51,
             simulate_36310051_283_to_284,
             simulate_36310051_50_to_100,
             simulate_36310051_250_to_300]

    name_length = max([len(func.__name__) for func in funcs])

    for func in funcs:
        print(f"{func.__name__}{' ' * (name_length + 1 - len(func.__name__))}", end="")
        func()

if __name__ == "__main__":
    main()
