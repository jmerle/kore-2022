#pragma once

#include <filesystem>

struct Configuration {
    /**
     * Total number of steps/turns in the run.
     */
    int episodeSteps = 400;

    /**
     * Maximum runtime (seconds) to obtain an action from an agent.
     */
    double actTimeout = 3.0;

    /**
     * Maximum runtime (seconds) of an episode (not necessarily DONE).
     */
    double runTimeout = 9600.0;

    /**
     * Maximum runtime (seconds) to initialize an agent.
     */
    double agentTimeout = 60.0;

    /**
     * The starting amount of kore available on the board.
     */
    double startingKore = 2750.0;

    /**
     * The number of cells vertically and horizontally on the board.
     */
    int size = 21;

    /**
     * The amount of kore to spawn a new ship.
     */
    double spawnCost = 10.0;

    /**
     * The amount of ships needed from a fleet to create a shipyard.
     */
    int convertCost = 50;

    /**
     * The rate kore regenerates on the board.
     */
    double regenRate = 0.02;

    /**
     * The maximum kore that can be in any cell.
     */
    double maxRegenCellKore = 500.0;

    /**
     * The seed to the random number generator (0 means no seed).
     */
    int randomSeed = 0;

    /**
     * Path to the directory containing the agent's files.
     */
    std::filesystem::path agentDirectory;
};
