#ifndef __TEST_RESULTS_PARSER_H__
#define __TEST_RESULTS_PARSER_H__

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>

using namespace std;

// Function to replace 'x' with random 0s and 1s
void replaceXWithRandom(std::string &str) {
    // Seed for random number generation
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    for (char &ch : str) {
        if (ch == 'x') {
            // Generate a random number (either 0 or 1)
            int randomBit = std::rand() % 2;
            
            // Replace 'x' with the random bit
            ch = '0' + randomBit;
        }
    }
}

void parser(std::fstream& testVectors, std::ofstream& accurateTestVectors)
{
    string line;

    if (testVectors.is_open())
    {
        while(getline(testVectors,line))
        {
            replaceXWithRandom(line);
            accurateTestVectors << line << endl;
        }       
    }
}

#endif