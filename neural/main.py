#!/usr/bin/env python3

import neat


def fitness_function(genomes, config):
    nets = []
    genomes = []
    birds = []

    for id, g in genomes:
        net = neat.nn.FeedForwardNetwork.create(g, config)
        nets.append(net)

        g.fitness = 0
        birds.append(Bird())


if __name__ == "__main__":
    # Set configuration file
    config_path = "config.ini"
    config = neat.Config(
        neat.DefaultGenome,
        neat.DefaultReproduction,
        neat.DefaultSpeciesSet,
        neat.DefaultStagnation,
        config_path,
    )

    # Create core evolution algorithm class
    p = neat.Population(config)

    # Add reporter for fancy statistical result
    p.add_reporter(neat.StdOutReporter(True))
    p.add_reporter(neat.StatisticsReporter())

    # Run NEAT
    ITERATIONS = 1000
    p.run(fitness_function, n=ITERATIONS)
