import itertools
from argparse import ArgumentParser
from collections import defaultdict
from copy import deepcopy
from dataclasses import dataclass
from enum import Enum, IntEnum
from pathlib import Path
from typing import Dict, List, Optional, Set, Tuple

class Direction(IntEnum):
    NORTH = 0
    EAST = 1
    SOUTH = 2
    WEST = 3

    def opposite(self) -> "Direction":
        return {
            Direction.NORTH: Direction.SOUTH,
            Direction.SOUTH: Direction.NORTH,
            Direction.EAST: Direction.WEST,
            Direction.WEST: Direction.EAST
        }[self]

class PartType(Enum):
    TURN = 0
    MOVE = 1
    CONVERT = 2

@dataclass
class Part:
    type: PartType
    direction: Optional[Direction]
    steps: Optional[int]

    @staticmethod
    def turn(direction: Direction) -> "Part":
        return Part(PartType.TURN, direction, None)

    @staticmethod
    def move(steps: int) -> "Part":
        return Part(PartType.MOVE, None, steps)

    @staticmethod
    def convert() -> "Part":
        return Part(PartType.CONVERT, None, None)

@dataclass
class Cell:
    x: int
    y: int

    def move(self, direction: Direction) -> "Cell":
        x = self.x
        y = self.y

        if direction == Direction.NORTH:
            y += 1
        elif direction == Direction.EAST:
            x += 1
        elif direction == Direction.SOUTH:
            y -= 1
        else:
            x -= 1

        if x == -1:
            x = 20
        elif x == 21:
            x = 0

        if y == -1:
            y = 20
        elif y == 21:
            y = 0

        return Cell(x, y)

def validate_plan(plan: List[Part]) -> bool:
    if len(plan) == 0:
        return False

    if plan[0].type != PartType.TURN:
        return False

    current_direction = None
    previous_direction = None

    for part in plan:
        if part.type != PartType.TURN:
            continue

        if part.direction == current_direction:
            return False

        if part.direction == previous_direction and current_direction == part.direction.opposite():
            return False

        previous_direction, current_direction = current_direction, part.direction

    for i in range(len(plan) - 1):
        if plan[i].type == PartType.MOVE:
            if plan[i - 1].type != PartType.TURN or (plan[i + 1].type != PartType.TURN and plan[i + 1].type != PartType.CONVERT):
                return False

    return True

def evaluate_plan(plan: List[Part]) -> Optional[List[Cell]]:
    plan = [deepcopy(part) for part in plan]
    is_convert_plan = plan[-1].type == PartType.CONVERT

    current_direction = plan[0].direction
    plan.pop(0)

    current_cell = Cell(0, 0).move(current_direction)
    cells = [current_cell]

    while True:
        while len(plan) > 0 and plan[0].type == PartType.MOVE and plan[0].steps == 0:
            plan.pop(0)

        if len(plan) > 0:
            if plan[0].type == PartType.TURN:
                current_direction = plan[0].direction
                plan.pop(0)
            elif plan[0].type == PartType.MOVE:
                if plan[0].steps == 1:
                    plan.pop(0)
                else:
                    plan[0].steps -= 1
            elif plan[0].type == PartType.CONVERT:
                plan.pop(0)
        else:
            if current_cell in cells:
                cells_on_path = []

                if current_direction == Direction.NORTH or current_direction == Direction.SOUTH:
                    for y in range(21):
                        cells_on_path.append(Cell(current_cell.x, y))
                else:
                    for x in range(21):
                        cells_on_path.append(Cell(x, current_cell.y))

                if all(cell in cells for cell in cells_on_path):
                    break

        if is_convert_plan and len(plan) == 0:
            cells.append(deepcopy(current_cell))
            break

        next_cell = current_cell.move(current_direction)
        cells.append(next_cell)
        current_cell = next_cell

        if next_cell.x == 0 and next_cell.y == 0:
            if len(plan) > 0:
                return None
            break

    return cells

def plan_to_string(plan: List[Part]) -> str:
    string = ""

    for part in plan:
        if part.type == PartType.TURN:
            string += {
                Direction.NORTH: "N",
                Direction.EAST: "E",
                Direction.SOUTH: "S",
                Direction.WEST: "W"
            }[part.direction]
        elif part.type == PartType.MOVE:
            string += str(part.steps)
        elif part.type == PartType.CONVERT:
            string += "C"

    return string

def write_plans(plans_by_offset: Dict[Tuple[int, int], List[Tuple[int, str]]], path: Path) -> None:
    if not path.parent.is_dir():
        path.parent.mkdir(parents=True)

    with path.open("w+", encoding="utf-8") as file:
        file.write(str(len(plans_by_offset)) + "\n")

        for dx in range(21):
            for dy in range(21):
                if (dx, dy) not in plans_by_offset:
                    continue

                plans = plans_by_offset[(dx, dy)]

                steps_options = sorted(set(steps for steps, _ in plans))
                unique_plans = len(set(plan for _, plan in plans))

                file.write(f"{dx} {dy} {len(steps_options)} {unique_plans}\n")

                plans_seen = set()

                for steps_option in steps_options:
                    sorted_plans = sorted([plan for steps, plan in plans if steps == steps_option], key=lambda plan: (len(plan), plan))

                    file.write(f"{steps_option} {len(sorted_plans)}\n")
                    for plan in sorted_plans:
                        is_first = plan not in plans_seen
                        plans_seen.add(plan)

                        file.write(f"{1 if is_first else 0} {plan}\n")

    print(f"Successfully generated {path.resolve().relative_to(Path.cwd())}")

def main() -> None:
    parser = ArgumentParser(description="Generate possible flight plans.")
    parser.add_argument("agent", type=str, help="name of the agent to store the plans in, relative to <build directory>/agents")

    args = parser.parse_args()

    agent_directory = Path(__file__).parent.parent / "agents" / args.agent
    if not (agent_directory / "main.py").is_file():
        raise ValueError(f"Agent '{args.agent}' does not exist")

    available_parts = [
        Part.turn(Direction.NORTH),
        Part.turn(Direction.EAST),
        Part.turn(Direction.SOUTH),
        Part.turn(Direction.WEST),
        Part.move(1),
        Part.move(2),
        Part.move(3),
        Part.move(4),
        Part.move(5),
        Part.move(6),
        Part.move(7),
        Part.move(8),
        Part.move(9),
        Part.move(10),
        Part.move(11)
    ]

    target_plans = defaultdict(set)
    convert_plans = defaultdict(set)

    min_length = 1
    max_length = 7

    for i in range(min_length, max_length + 1):
        possible_parts = available_parts if i == 1 else available_parts[:-(i - 1)]

        for plan in itertools.product(possible_parts, repeat=i):
            plan_variants = [list(plan)]

            if len(plan) < max_length:
                plan_variants.append([*plan, Part.convert()])

            for plan_variant in plan_variants:
                if plan_variant[-1].type == PartType.MOVE:
                    continue

                if not validate_plan(plan_variant):
                    continue

                cells = evaluate_plan(plan_variant)
                if cells is None:
                    continue

                plan_string = plan_to_string(plan_variant)

                if plan_variant[-1].type != PartType.CONVERT:
                    for j, cell in enumerate(cells):
                        target_plans[(cell.x, cell.y)].add((j + 1, plan_string))
                else:
                    convert_plans[(cells[-1].x, cells[-1].y)].add((len(cells), plan_string))

        print(f"Successfully gathered flight plans of size {i}")

    data_directory = agent_directory / "data"

    write_plans(target_plans, data_directory / "target-plans.txt")
    write_plans(convert_plans, data_directory / "convert-plans.txt")

if __name__ == "__main__":
    main()
