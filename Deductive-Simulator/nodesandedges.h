/*
	Author : Karthikeya Sharma M
	Last Date Modified : September 29, 2023
	Subject : ECE 6140 Digital Systems Testing

	Description:
	This header file describes the structures for logic gates and nets present in the circuit.
*/

#pragma once
#ifndef _NODESANDEDGES_H_
#define _NODESANDEDGES_H_
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <bitset>

using namespace std;

/*
Enum for Gate Type:
Logic Gate Numbers: AND - 0, NAND - 1, OR - 2, NOR - 3, XOR - 4, XNOR - 5, INV - 6, BUF - 7
*/
enum gateTypeNum {
	AND,
	NAND,
	OR,
	NOR,
	XOR,
	XNOR,
	INV,
	BUF
};

//Declaring structure for 2-input Logic Gate
struct LogicGate {
	//Declaring an instance of the enum created for determining the logic gate type.
	gateTypeNum gateType;

	//edgeInputNetNum stores the input net numbers. 
	unsigned int edgeInputNetNum1, edgeInputNetNum2;

	//edgeOutputNetNum stores the output net number.
	unsigned int edgeOutputNetNum;
};

//Declaring structure for Nets
struct Net {
	//Number of the net
	unsigned int netNumber; 

	//Input gate numbers (next gate numbers)
	unsigned int inputGateNum1;

	//Output Gate Number (previous gate number)
	unsigned int outputGateNum;

	//Data on the net
	std::bitset<1> netValue;

	//Don't care Variable flag. If -1, the output of a gate/net value is unknown. If it becomes 0, the output is known.
	int dontCare;

	//Fault List
	vector<string> faultList;
};

//A function that takes in structure object, and bitset variables input1 and input2 (1 bit each) as inputs to evaluate the output of each two input logic gate.
std::bitset<1> evaluateGate2Input(LogicGate logicGate, std::bitset<1> input1, std::bitset<1> input2)
{
	//switch statement used to compare which two-input logic gate's output has to be evaluated.
	switch (logicGate.gateType)
	{
	case AND:
		return (input1 & input2);
		break;
	case NAND:
		return ~(input1 & input2);
		break;
	case OR:
		return (input1 | input2);
		break;
	case NOR:
		return ~(input1 | input2);
		break;
	case XOR:
		return (input1 ^ input2);
		break;
	case XNOR:
		return ~(input1 ^ input2);
		break;
	case INV:
		return ~(input1);
	case BUF:
		return input1;
	}
}

//A function that takes in string input and gives out vector of strings based on the delimiter ' '.
std::vector<string> splitString(string s)
{
	//Token obtained from the original string s
	string strToken;

	//stringstream
	stringstream stringStream(s);
	vector<string> v;
	while (getline(stringStream, strToken, ' '))
	{
		v.push_back(strToken);
	}
	return v;
}

unsigned int computeMax(unsigned int a, unsigned int b, unsigned int c)
{
	if (a > b && a > c)
	{
		return a;
	}
	else if (b > a && b > c)
	{
		return b;
	}
	else if (c > a && c > b)
	{
		return c;
	}
}

#endif