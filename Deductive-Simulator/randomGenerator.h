/*
	Author : Karthikeya Sharma M
	Last Date Modified : November 11, 2023
	Subject : ECE 6140 Digital Systems Testing

	Description:
	This header file defines the random vector and random fault generator functions.
*/

#pragma once
#ifndef _RANDOM_GENERATOR_H_
#define _RANDOM_GENERATOR_H_

#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <set>
#include <ctime>
#include <string>

using namespace std;

void randomVectorGenerator(int numTestVectors, int NumInputs, const std::string& inputFilePath)
{
    std::ofstream ofs;
    ofs.open(inputFilePath);

    std::srand(static_cast<unsigned>(std::time(nullptr)));
    std::set<std::string> uniqueVectors;  // Set to store unique vectors

    while (uniqueVectors.size() < static_cast<size_t>(numTestVectors))
    {
        std::string vector;
        for (int i = 0; i < NumInputs; i++)
        {
            int n = std::rand() % 2;
            vector += std::to_string(n);
        }

        // Check if the generated vector is unique
        if (uniqueVectors.find(vector) == uniqueVectors.end())
        {
            uniqueVectors.insert(vector);
            ofs << vector;

            if (uniqueVectors.size() < static_cast<size_t>(numTestVectors))
            {
                ofs << std::endl;  // Write endl if it's not the last vector
            }
        }
    }

    ofs.close();
}

void randomFaultGenerator(int numNets, int numNetsMaxRange, string inputFilePath) {
    std::ofstream ofs;
    ofs.open(inputFilePath);
    vector<int> netNum;

    for (int i = 0; i < numNets; i++) {
        int n = rand() % 2;
        int m;
        bool set = false;

        do {
            m = rand() % numNetsMaxRange;
            if (m == 0) m = numNetsMaxRange;
            set = false;

            for (int ii = 0; ii < netNum.size(); ii++) {
                if (m == netNum[ii]) {
                    set = true;
                    break;
                }
            }
        } while (set);

        netNum.push_back(m);
        ofs << m << " " << n;

        if (i < numNets - 1) {
            ofs << endl;  // Write endl if it's not the last line
        }
    }

    ofs.close();
}

#endif