/*
    Author : Karthikeya Sharma M
    Last Date Modified : November 11, 2023
    Subject : ECE 6140 Digital Systems Testing

    Description:
    The header file defining datatypes to be used for the execution of PODEM (Path Oriented DEcision Making) Fault Simulation.
*/

#ifndef _PODEM_H_
#define _PODEM_H_

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <bitset>
#include <ctime>
#include <cstdlib>
#include <random>

using namespace std;

std::vector<string> BackTrace();
std::vector<string> Objective();
void podem();
void Imply_and_Check();
void Look_Ahead_Propagation();

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

struct LogicGate 
{
    //Declaring an instance of the enum created for determining the logic gate type.
	gateTypeNum gateType;

	//A unique identifier of the gate.
	unsigned int gateNumber;

	//edgeInputNetNum stores the input net numbers. 
	unsigned int edgeInputNetNum1, edgeInputNetNum2;

	//edgeOutputNetNum stores the output net number.
	unsigned int edgeOutputNetNum;
};

/*
    Possible Net values : 1,0,x,D,D'
*/
struct netVal 
{
    string high = "1";
    string low = "0";
    string dontCare = "x";
    string D = "D";
    string Dbar = "D'";
};

/*
    Advanced Net : Capability to handle x, D and D' apart from binary 0 and 1
*/
struct AdvancedNet
{
    //Number of the net
	unsigned int netNumber; 

	//Input gate numbers (next gate numbers)
	unsigned int inputGateNum;

	//Output Gate Number (previous gate number)
	unsigned int outputGateNum;

	//Data on the net
	struct netVal* netValue;

    string netValueString;

	string faultList;
};

//List of Dfrontiers.
std::vector<LogicGate> DFrontier;

string evaluateGateInput(LogicGate logicGate, string input1, string input2)
{
	netVal netValue;
	//switch statement used to compare which two-input logic gate's output has to be evaluated.
	switch (logicGate.gateType)
	{
	case AND:
		if (input1 == netValue.low && (input2 == netValue.low || input2 == netValue.high || input2 == netValue.dontCare || input2 == netValue.Dbar || input2 == netValue.D))
		{
			return input1;
		}
		else if (input1 == netValue.high && (input2 == netValue.low || input2 == netValue.high || input2 == netValue.dontCare || input2 == netValue.Dbar || input2 == netValue.D))
		{
			return input2;
		}
		else if (input1 == netValue.dontCare)
		{
			if (input2 == netValue.low)
				return input2;
			else if (input2 == netValue.high || input2 == netValue.dontCare || input2 == netValue.Dbar || input2 == netValue.D)
				return input1;
		}
		else if (input1 == netValue.D)
		{
			if (input2 == netValue.low || input2 == netValue.dontCare)
				return input2;
			else if (input2 == netValue.high || input2 == netValue.D)
				return input1;
			else if (input2 == netValue.Dbar) 
				return netValue.low;
		}
		else if (input1 == netValue.Dbar)
		{
			if (input2 == netValue.low || input2 == netValue.dontCare)
				return input2;
			else if (input2 == netValue.high || input2 == netValue.Dbar)
				return input1;
			else if (input2 == netValue.D) 
				return netValue.low;
		}
		break;
	case NAND:
		if (input1 == netValue.low && (input2 == netValue.low || input2 == netValue.high || input2 == netValue.dontCare || input2 == netValue.Dbar || input2 == netValue.D))
		{
			return netValue.high;
		}
		else if (input1 == netValue.high)
		{
			if (input2 == netValue.low)
				return netValue.high;
			else if (input2 == netValue.high)
				return netValue.low;
			else if (input2 == netValue.dontCare)
				return netValue.dontCare;
			else if (input2 == netValue.Dbar)
				return netValue.D;
			else if (input2 == netValue.D)
				return netValue.Dbar;
		}
		else if (input1 == netValue.dontCare)
		{
			if (input2 == netValue.low)
				return netValue.high;
			else if (input2 == netValue.high || input2 == netValue.dontCare || input2 == netValue.Dbar || input2 == netValue.D)
				return input1;
		}
		else if (input1 == netValue.D)
		{
			if (input2 == netValue.low || input2 == netValue.Dbar)
				return netValue.high;
			else if (input2 == netValue.dontCare)
				return input2;
			else if (input2 == netValue.high || input2 == netValue.D)
				return netValue.Dbar;
		}
		else if (input1 == netValue.Dbar)
		{
			if (input2 == netValue.low || input2 == netValue.D)
				return netValue.high;
			else if (input2 == netValue.high || input2 == netValue.Dbar)
				return netValue.D;
			else if (input2 == netValue.dontCare)
				return netValue.dontCare;
		}
		break;
	case OR:
		if (input1 == netValue.low && (input2 == netValue.low || input2 == netValue.high || input2 == netValue.dontCare || input2 == netValue.Dbar || input2 == netValue.D))
		{
			return input2;
		}
		else if (input1 == netValue.high && (input2 == netValue.low || input2 == netValue.high || input2 == netValue.dontCare || input2 == netValue.Dbar || input2 == netValue.D))
		{
			return input1;
		}
		else if (input1 == netValue.dontCare)
		{
			if (input2 == netValue.high)
				return input2;
			else if (input2 == netValue.low || input2 == netValue.dontCare || input2 == netValue.Dbar || input2 == netValue.D)
				return input1;
		}
		else if (input1 == netValue.D)
		{
			if (input2 == netValue.low || input2 == netValue.D)
				return input1;
			else if (input2 == netValue.high || input2 == netValue.dontCare)
				return input2;
			else if (input2 == netValue.Dbar)
				return netValue.high;
		}
		else if (input1 == netValue.Dbar)
		{
			if (input2 == netValue.low || input2 == netValue.Dbar)
				return input1;
			else if (input2 == netValue.high || input2 == netValue.dontCare)
				return input2;
			else if (input2 == netValue.D)
				return netValue.high;
		}
		break;
	case NOR:
		if (input1 == netValue.low)
		{
			if (input2 == netValue.low)
				return netValue.high;
			else if (input2 == netValue.high) 
				return netValue.low;
			else if (input2 == netValue.dontCare)
				return netValue.dontCare;
			else if (input2 == netValue.Dbar)
				return netValue.D;
			else if (input2 == netValue.D)
				return netValue.Dbar;
		}
		else if (input1 == netValue.high && (input2 == netValue.low || input2 == netValue.high || input2 == netValue.dontCare || input2 == netValue.Dbar || input2 == netValue.D))
		{
			return netValue.low;
		}
		else if (input1 == netValue.dontCare)
		{
			if (input2 == netValue.low || input2 == netValue.dontCare || input2 == netValue.Dbar || input2 == netValue.D)
				return input1;
			else if (input2 == netValue.high)
				return netValue.low;
		}
		else if (input1 == netValue.D)
		{
			if (input2 == netValue.low || input2 == netValue.D)
				return netValue.Dbar;
			else if (input2 == netValue.high || input2 == netValue.Dbar)
				return netValue.low;
			else if (input2 == netValue.dontCare)
				return input2;
		}
		else if (input1 == netValue.Dbar)
		{
			if (input2 == netValue.low || input2 == netValue.Dbar)
				return netValue.D;
			else if (input2 == netValue.high || input2 == netValue.D)
				return netValue.low;
			else if (input2 == netValue.dontCare)
				return input2;
		}
		break;
	case XOR:
		if (input1 == netValue.low)
		{
			if (input2 == netValue.low)
				return input1;
			else if (input2 == netValue.high)
				return input2;
			else if (input2 == netValue.dontCare)
				return input2;
			else if (input2 == netValue.Dbar)
				return input2;
			else if (input2 == netValue.D)
				return input1;
		}
		else if (input1 == netValue.high)
		{
			if (input2 == netValue.low)
				return input1;
			else if (input2 == netValue.high)
				return netValue.low;
			else if (input2 == netValue.dontCare)
				return input2;
			else if (input2 == netValue.Dbar)
				return netValue.D;
			else if (input2 == netValue.D)
				return netValue.Dbar;
		}
		else if (input1 == netValue.dontCare)
		{
			if (input2 == netValue.low || input2 == netValue.high || input2 == netValue.dontCare || input2 == netValue.Dbar || input2 == netValue.D)
				return input1;
		}
		else if (input1 == netValue.D)
		{
			if (input2 == netValue.low)
				return input1;
			else if (input2 == netValue.high)
				return netValue.Dbar;
			else if (input2 == netValue.dontCare)
				return input2;
			else if (input2 == netValue.Dbar)
				return netValue.high;
			else if (input2 == netValue.D)
				return netValue.low;
		}
		else if (input1 == netValue.Dbar)
		{
			if (input2 == netValue.low)
				return input1;
			else if (input2 == netValue.high)
				return netValue.D;
			else if (input2 == netValue.dontCare)
				return input2;
			else if (input2 == netValue.Dbar)
				return netValue.low;
			else if (input2 == netValue.D)
				return netValue.high;
		}
		break;
	case XNOR:
		if (input1 == netValue.low)
		{
			if (input2 == netValue.low)
				return netValue.high;
			else if (input2 == netValue.high)
				return input1;
			else if (input2 == netValue.dontCare)
				return input2;
			else if (input2 == netValue.Dbar)
				return netValue.D;
			else if (input2 == netValue.D)
				return netValue.Dbar;
		}
		else if (input1 == netValue.high)
		{
			if (input2 == netValue.low || input2 == netValue.high || input2 == netValue.dontCare || input2 == netValue.Dbar || input2 == netValue.D)
				return input2;
		}
		else if (input1 == netValue.dontCare)
		{
			if (input2 == netValue.low || input2 == netValue.high || input2 == netValue.dontCare || input2 == netValue.Dbar || input2 == netValue.D)
				return input1;
		}
		else if (input1 == netValue.D)
		{
			if (input2 == netValue.low)
				return netValue.Dbar;
			else if (input2 == netValue.high)
				return netValue.D;
			else if (input2 == netValue.dontCare)
				return input2;
			else if (input2 == netValue.Dbar)
				return netValue.low;
			else if (input2 == netValue.D)
				return netValue.high;
		}
		else if (input1 == netValue.Dbar)
		{
			if (input2 == netValue.low)
				return netValue.D;
			else if (input2 == netValue.high)
				return netValue.Dbar;
			else if (input2 == netValue.dontCare)
				return input2;
			else if (input2 == netValue.Dbar)
				return netValue.high;
			else if (input2 == netValue.D)
				return netValue.low;
		}
		break;
	case INV:
		if (input1 == netValue.low)
			return netValue.high;
		else if (input1 == netValue.high)
			return netValue.low;
		else if (input1 == netValue.dontCare)
			return netValue.dontCare;
		else if (input1 == netValue.Dbar)
			return netValue.D;
		else if (input1 == netValue.D)
			return netValue.Dbar;
	case BUF:
		return input1;
		break;
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