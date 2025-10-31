//
// Created by urani on 10/31/2025.
//

#ifndef RANDOMDATASETGENERATOR_H
#define RANDOMDATASETGENERATOR_H
#include <vector>
#include <random>
#include <limits>

struct RandomDatasetGenerator{
    std::vector<std::size_t> random_size_ts;
    std::vector<int> random_ints;
    RandomDatasetGenerator(std::size_t N){
        random_size_ts.reserve(N);
        random_ints.reserve(N);

        std::random_device rd;
        std::mt19937_64 engine(rd());

        std::uniform_int_distribution<std::size_t> dist_size_t(
            0, std::numeric_limits<std::size_t>::max()-1
        );
        std::uniform_int_distribution<int> dist_int(
            std::numeric_limits<int>::min(),
            std::numeric_limits<int>::max()
        );

        for (std::size_t i = 0; i < N; ++i) {
            random_size_ts.push_back(dist_size_t(engine));
            random_ints.push_back(dist_int(engine));
        }
    }
};
#endif //RANDOMDATASETGENERATOR_H
