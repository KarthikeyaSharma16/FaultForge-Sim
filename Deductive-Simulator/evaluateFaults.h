/*
	Author : Karthikeya Sharma M
	Last Date Modified : November 10, 2023
	Subject : ECE 6140 Digital Systems Testing

	Description:
	This header file gives the function definitions to be used in the deductive fault simulator.
*/

#pragma once
#ifndef _EVALUATE_FAULTS_H_
#define _EVALUATE_FAULTS_H_

#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <bitset>
#include <string>

using namespace std;

//Combine the net number and stuck-at fault value with a "/" in a string. This will be used later to append to the vector of fault lists.
std::string netAndStuckAtFault(unsigned int a, std::bitset<1> b)
{
	std::string netStuckAt;
	std::string slash = "/";
	netStuckAt += std::to_string(a);
	netStuckAt += slash;
	netStuckAt += b.to_string();
	return netStuckAt;
}

std::vector<std::string> Union(std::vector<std::string> input1FaultList, std::vector<std::string> input2FaultList)
{
	std::vector<std::string> outputFaultList;
	
	for (int ii = 0; ii < input1FaultList.size(); ii++)
	{
		outputFaultList.push_back(input1FaultList[ii]);
	}

	for (int ii = 0; ii < input2FaultList.size(); ii++)
	{
		outputFaultList.push_back(input2FaultList[ii]);
	}

	return outputFaultList;
}

std::vector<std::string> Intersection(std::vector<std::string> input1FaultList, std::vector<string> input2FaultList)
{
	std::vector<std::string> outputFaultList;

	for (int ii = 0; ii < input1FaultList.size(); ii++)
	{
		for (int jj = 0; jj < input2FaultList.size(); jj++)
		{
			if (input1FaultList[ii] == input2FaultList[jj])
			{
				outputFaultList.push_back(input1FaultList[ii]);
			}
		}
	}

	return outputFaultList;
}

std::vector<std::string> Difference(std::vector<string> input1FaultList, std::vector<string> input2FaultList)
{
	//Essentially, input1 - input2 removing the common faults.

	for (int ii = 0; ii < input1FaultList.size(); ) {
		bool found = false;
		for (int jj = 0; jj < input2FaultList.size(); jj++) {
			if (input1FaultList[ii] == input2FaultList[jj]) {
				found = true;
				break; // Exit the inner loop if a common element is found.
			}
		}
		if (found) 
		{
			input1FaultList.erase(input1FaultList.begin() + ii);
		}
		else 
		{
			ii++; // Only increment the index if no common element is found.
		}
	}

	return input1FaultList;

}

std::string combineInputs(std::bitset<1> a, std::bitset<1> b)
{
	std::string inputs;
	inputs += a.to_string();
	inputs += b.to_string();
	return inputs;
}

#endif