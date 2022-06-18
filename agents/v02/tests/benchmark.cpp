#include <benchmark/benchmark.h>

#include <tests/utilities.h>

#include <strategy/FlightPlanDatabase.h>

void copy_36310051_50(benchmark::State &state) {
    auto data = parseDataFile("36310051.json");
    auto board = createBoard(data, 49);

    for (auto _ : state) {
        benchmark::DoNotOptimize(board.copy());
    }
}

void copy_36310051_250(benchmark::State &state) {
    auto data = parseDataFile("36310051.json");
    auto board = createBoard(data, 249);

    for (auto _ : state) {
        benchmark::DoNotOptimize(board.copy());
    }
}

void simulate_36310051_50_to_51(benchmark::State &state) {
    auto data = parseDataFile("36310051.json");
    auto board = createBoard(data, 49);

    for (auto _ : state) {
        board.copy().next();
    }
}

void simulate_36310051_283_to_284(benchmark::State &state) {
    auto data = parseDataFile("36310051.json");
    auto board = createBoard(data, 282);

    for (auto _ : state) {
        board.copy().next();
    }
}

void simulate_36310051_50_to_100(benchmark::State &state) {
    auto data = parseDataFile("36310051.json");
    auto board = createBoard(data, 49);

    for (auto _ : state) {
        auto currentBoard = board.copy();
        for (int i = 0; i < 50; i++) {
            currentBoard = currentBoard.copy();
            currentBoard.next();
        }
    }
}

void simulate_36310051_50_to_100_no_copy(benchmark::State &state) {
    auto data = parseDataFile("36310051.json");
    auto board = createBoard(data, 49);

    for (auto _ : state) {
        auto currentBoard = board.copy();
        for (int i = 0; i < 50; i++) {
            currentBoard.next();
        }
    }
}

void simulate_36310051_250_to_300(benchmark::State &state) {
    auto data = parseDataFile("36310051.json");
    auto board = createBoard(data, 249);

    for (auto _ : state) {
        auto currentBoard = board.copy();
        for (int i = 0; i < 50; i++) {
            currentBoard = currentBoard.copy();
            currentBoard.next();
        }
    }
}

void simulate_36310051_250_to_300_no_copy(benchmark::State &state) {
    auto data = parseDataFile("36310051.json");
    auto board = createBoard(data, 249);

    for (auto _ : state) {
        auto currentBoard = board.copy();
        for (int i = 0; i < 50; i++) {
            currentBoard.next();
        }
    }
}

BENCHMARK(copy_36310051_50);
BENCHMARK(copy_36310051_250);
BENCHMARK(simulate_36310051_50_to_51);
BENCHMARK(simulate_36310051_283_to_284);
BENCHMARK(simulate_36310051_50_to_100);
BENCHMARK(simulate_36310051_50_to_100_no_copy);
BENCHMARK(simulate_36310051_250_to_300);
BENCHMARK(simulate_36310051_250_to_300_no_copy);

BENCHMARK_MAIN();
