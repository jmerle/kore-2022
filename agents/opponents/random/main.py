import math
from kaggle_environments.envs.kore_fleets.helpers import board_agent, ShipyardAction
from kaggle_environments.helpers import Direction
from random import randint, sample

@board_agent
def random_agent(board):
    me = board.current_player
    remaining_kore = me.kore
    shipyards = me.shipyards
    # randomize shipyard order
    shipyards = sample(shipyards, len(shipyards))
    for shipyard in shipyards:
        # 25% chance to launch a large fleet
        if randint(0, 3) == 0 and shipyard.ship_count > 10:
            dir_str = Direction.random_direction().to_char()
            dir2_str = Direction.random_direction().to_char()
            flight_plan = dir_str + str(randint(1, 10)) + dir2_str
            shipyard.next_action = ShipyardAction.launch_fleet_with_flight_plan(min(10, math.floor(shipyard.ship_count / 2)), flight_plan)
        # else spawn if possible
        elif remaining_kore > board.configuration.spawn_cost * shipyard.max_spawn:
            remaining_kore -= board.configuration.spawn_cost
            shipyard.next_action = ShipyardAction.spawn_ships(shipyard.max_spawn)
        # else launch a small fleet
        elif shipyard.ship_count >= 2:
            dir_str = Direction.random_direction().to_char()
            shipyard.next_action = ShipyardAction.launch_fleet_with_flight_plan(2, dir_str)
