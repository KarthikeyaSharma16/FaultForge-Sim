/*
    Author : Karthikeya Sharma M
    Last Date Modified : September 29, 2023
    Subject : ECE 6140 Digital Systems Testing

    Description:
    The main.cpp file that implements the logic Simulator.
*/

#include <iostream>
#include <string>
#include "nodesandedges.h"
#include <fstream>
#include <vector>
#include <cstring>

#define DONT_CARE -1

using namespace std;

void evaluateCircuit(std::vector<LogicGate>& currentStack, std::vector<LogicGate>& evaluatedStack, std::vector<LogicGate>& unevaluatedStack, std::vector<Net>& netArray)
{
    // check if both inputs to the gate are available and the Don't care flag is 0.
    // If so, evaluate the gate, set the don't care flag of the output net to 0, and push the gate to the evaluated stack.
    // If not available, push the gate to unevaluated stack.
    // go with popping the next gate.

    while (!currentStack.empty())
    {
        LogicGate poppedOutGate = currentStack.back();
        currentStack.pop_back();

        std::bitset<1> gateInput1, gateInput2, output;
        bool gateEvaluated = false;

        //If not gatetype is invert,check if net array has both inputs <>-1
        if (poppedOutGate.gateType != INV && poppedOutGate.gateType != BUF)
        {
            if (netArray[poppedOutGate.edgeInputNetNum1 - 1].dontCare != -1 && netArray[poppedOutGate.edgeInputNetNum2 - 1].dontCare != -1)
            {
                gateInput1 = netArray[poppedOutGate.edgeInputNetNum1 - 1].netValue;
                gateInput2 = netArray[poppedOutGate.edgeInputNetNum2 - 1].netValue;
                output = evaluateGate2Input(poppedOutGate, gateInput1, gateInput2);
                netArray[poppedOutGate.edgeOutputNetNum - 1].netValue = output;
                netArray[poppedOutGate.edgeOutputNetNum - 1].dontCare = 0;
                gateEvaluated = true; //set as gate evaluated
            }
        }
        else
        {
            if (netArray[poppedOutGate.edgeInputNetNum1 - 1].dontCare != -1)
            {
                gateInput1 = netArray[poppedOutGate.edgeInputNetNum1 - 1].netValue;
                output = evaluateGate2Input(poppedOutGate, gateInput1,0); //2nd input can be ignored as output is evaluated for a single input
                netArray[poppedOutGate.edgeOutputNetNum - 1].netValue = output;
                netArray[poppedOutGate.edgeOutputNetNum - 1].dontCare = 0;
                gateEvaluated = true; //set as gate evaluated
            }
        }
        
        //If gate is evaluated, move to evaluated stack. Else move it to unevaluated stack.
        if (gateEvaluated)
        {
            evaluatedStack.push_back(poppedOutGate);
        }
        else
        {
            unevaluatedStack.push_back(poppedOutGate);
        }

        //Transfer the contents of the unevaluated stack to the current stack if the current stack is devoid of logic gate structures.
        if (currentStack.empty())
        {
            while (!unevaluatedStack.empty()) 
            {
                currentStack.push_back(unevaluatedStack.back());
                unevaluatedStack.pop_back();
            }
        }
    }

    std::cout << "Stack Evaluation completed!" << endl;
}

int main()
{
    /*
        Declaring three stacks to simplify stack operations. Each stack is defined in brief:
        Current Stack holds the current gates to be popped in the stack.
        Evaluated Stack holds the gates whose outputs have been evaluated.
        Unevaluated stack holds the gates whose outputs have not been evaluated due to unavailability of both the inputs.
    */

    std::vector<LogicGate> currentStack, evaluatedStack, unevaluatedStack;

    //Maximum value of net in the given netlist.
    unsigned int nMaxNet = 0;

    //Declare a vector of
    std::vector<int> inputNets, outputNets;

    //Create a file object
    fstream netlistFile;
    std::cout << "Enter the location of the directory where all netlist files have been stored. All netlist files will be evaluated with the given input values." << endl;
    string path;
    std::getline(cin, path);

    //Open the netlist file using the open method
    //input from user for file location (path and name)
    netlistFile.open(path, ios::in);
    string line;

    //verify if the file opened
    if (netlistFile.is_open())
    {
        //Declare a vector of strings to obtain the split string in the form of vector based on the delimiter ' '.
        std::vector<string> lineToVect;

        while (std::getline(netlistFile, line))
        {
            lineToVect = splitString(line);
            if (!lineToVect.empty())
            {
                if (lineToVect[0] == "INPUT")
                {
                    int ii;
                    for (ii = 2; ii <= lineToVect.size() - 2; ii++)
                    {
                        inputNets.push_back(stoi(lineToVect[ii])); //Ignore space
                    }
                }
                else if (lineToVect[0] == "OUTPUT")
                {
                    int jj;
                    for (jj = 2; jj <= lineToVect.size() - 2; jj++)
                    {
                        outputNets.push_back(stoi(lineToVect[jj])); //Ignore space
                    }
                }
                else
                {
                    //Create an instance of the structure LogicGate declared in nodesandedges.h header file.
                    LogicGate logicGateArray;
                    if (lineToVect[0] == "AND")
                    {
                        logicGateArray.gateType = AND;
                        logicGateArray.edgeOutputNetNum = stoi(lineToVect[3]);
                        logicGateArray.edgeInputNetNum1 = stoi(lineToVect[1]);
                        logicGateArray.edgeInputNetNum2 = stoi(lineToVect[2]);
                        currentStack.push_back(logicGateArray);

                        //Compute the maximum net value when reading each line of the file.
                        nMaxNet = computeMax(stoi(lineToVect[1]), stoi(lineToVect[2]), stoi(lineToVect[3]));
                    }
                    else if (lineToVect[0] == "NAND")
                    {
                        logicGateArray.gateType = NAND;
                        logicGateArray.edgeOutputNetNum = stoi(lineToVect[3]);
                        logicGateArray.edgeInputNetNum1 = stoi(lineToVect[1]);
                        logicGateArray.edgeInputNetNum2 = stoi(lineToVect[2]);
                        currentStack.push_back(logicGateArray);

                        //Compute the maximum net value when reading each line of the file.
                        nMaxNet = computeMax(stoi(lineToVect[1]), stoi(lineToVect[2]), stoi(lineToVect[3]));
                    }
                    else if (lineToVect[0] == "OR")
                    {
                        logicGateArray.gateType = OR;
                        logicGateArray.edgeOutputNetNum = stoi(lineToVect[3]);
                        logicGateArray.edgeInputNetNum1 = stoi(lineToVect[1]);
                        logicGateArray.edgeInputNetNum2 = stoi(lineToVect[2]);
                        currentStack.push_back(logicGateArray);

                        //Compute the maximum net value when reading each line of the file.
                        nMaxNet = computeMax(stoi(lineToVect[1]), stoi(lineToVect[2]), stoi(lineToVect[3]));
                    }
                    else if (lineToVect[0] == "NOR")
                    {
                        logicGateArray.gateType = NOR;
                        logicGateArray.edgeOutputNetNum = stoi(lineToVect[3]);
                        logicGateArray.edgeInputNetNum1 = stoi(lineToVect[1]);
                        logicGateArray.edgeInputNetNum2 = stoi(lineToVect[2]);
                        currentStack.push_back(logicGateArray);

                        //Compute the maximum net value when reading each line of the file.
                        nMaxNet = computeMax(stoi(lineToVect[1]), stoi(lineToVect[2]), stoi(lineToVect[3]));
                    }
                    else if (lineToVect[0] == "XOR")
                    {
                        logicGateArray.gateType = XOR;
                        logicGateArray.edgeOutputNetNum = stoi(lineToVect[3]);
                        logicGateArray.edgeInputNetNum1 = stoi(lineToVect[1]);
                        logicGateArray.edgeInputNetNum2 = stoi(lineToVect[2]);
                        currentStack.push_back(logicGateArray);

                        //Compute the maximum net value when reading each line of the file.
                        nMaxNet = computeMax(stoi(lineToVect[1]), stoi(lineToVect[2]), stoi(lineToVect[3]));
                    }
                    else if (lineToVect[0] == "XNOR")
                    {
                        logicGateArray.gateType = XNOR;
                        logicGateArray.edgeOutputNetNum = stoi(lineToVect[3]);
                        logicGateArray.edgeInputNetNum1 = stoi(lineToVect[1]);
                        logicGateArray.edgeInputNetNum2 = stoi(lineToVect[2]);
                        currentStack.push_back(logicGateArray);

                        //Compute the maximum net value when reading each line of the file.
                        nMaxNet = computeMax(stoi(lineToVect[1]), stoi(lineToVect[2]), stoi(lineToVect[3]));
                    }
                    else if (lineToVect[0] == "INV")
                    {
                        logicGateArray.gateType = INV;
                        logicGateArray.edgeOutputNetNum = stoi(lineToVect[2]);
                        logicGateArray.edgeInputNetNum1 = stoi(lineToVect[1]);
                        logicGateArray.edgeInputNetNum2 = logicGateArray.edgeInputNetNum1;
                        currentStack.push_back(logicGateArray);

                        //Compute the maximum net value when reading each line of the file.
                        nMaxNet = computeMax(stoi(lineToVect[1]), stoi(lineToVect[2]), stoi(lineToVect[2]));
                    }
                    else if (lineToVect[0] == "BUF")
                    {
                        logicGateArray.gateType = BUF;
                        logicGateArray.edgeOutputNetNum = stoi(lineToVect[2]);
                        logicGateArray.edgeInputNetNum1 = stoi(lineToVect[1]);
                        logicGateArray.edgeInputNetNum2 = logicGateArray.edgeInputNetNum1;
                        currentStack.push_back(logicGateArray);

                        //Compute the maximum net value when reading each line of the file.
                        nMaxNet = computeMax(stoi(lineToVect[1]), stoi(lineToVect[2]), stoi(lineToVect[2]));
                    }
                }
            }
        }
    }
    else
    {
        cerr << "File not found!" << endl;
        return 1;
    }

    //Close the file to free the memory.
    netlistFile.close();

    //Initialize all the nets in the circuit with Don't Care value.
    std::vector<Net> netArray(nMaxNet);
    for (int ii = 0; ii < nMaxNet; ii++)
    {
        netArray[ii].dontCare = DONT_CARE;
        netArray[ii].netNumber = ii + 1;
    }

    //Get the location of the txt file having the input values for evaluating the circuit.
    //Create a file object inputsFile
    fstream inputsFile;

    //Print out a message to the console prompting the user to enter the path of the txt file having the inputs.
    std::cout << "Enter the location of the directory where the inputs to the netlist file is stored." << endl;

    //A string that stores the input file path.
    string inputFilePath;

    //Getline function to get the input.
    std::getline(cin, inputFilePath);

    //Open the file using the input file path entered.
    inputsFile.open(inputFilePath, ios::in);

    cout << inputsFile.is_open() << endl;

    //String that stores input bit combination in the file in every line.
    string inputBits;

    //A vector that stores the input binary bits in the form of a vector.
    std::vector<int> binaryDigits;

    //Output stream object that stores the resultant output vectors.
    ofstream saveResult("result.txt");

    if (inputsFile.is_open())
    {
        while (std::getline(inputsFile, inputBits))
        {
            //Reset binaryDigits every iteration.
            binaryDigits.clear();
            
            // Reset netArray to its initial state before processing each line of input
            for (int jj = 0; jj < netArray.size(); jj++)
            {
                netArray[jj].netValue = 0; // Reset netValue to 0
                netArray[jj].dontCare = -1; // Reset dontCare to -1
            }

            //Convert the string inputBits into binary vector binaryDigits. 
            for (char digit : inputBits)
            {
                if (digit == '0' || digit == '1') {
                    int binaryDigit = digit - '0'; // Convert character to integer
                    binaryDigits.push_back(binaryDigit);
                }
                else
                {
                    // Handle invalid characters if needed
                    std::cerr << "Invalid character in input: " << digit << std::endl;
                }
            }

            //Assign inputs values entered by the user to the input nets inputNets has the input nets stored.
            for (int jj = 0; jj < inputNets.size(); jj++)
            {
                netArray[inputNets[jj] - 1].netValue = binaryDigits[jj];
                netArray[inputNets[jj] - 1].dontCare = 0; //Indicates the availability of inputs.    

                std::cout << netArray[inputNets[jj] - 1].netValue << " " << netArray[inputNets[jj] - 1].dontCare << endl;
            }

            //Call the function Evaluate circuit.
            evaluateCircuit(currentStack, evaluatedStack, unevaluatedStack, netArray);

            //print outputs to the console.
            for (int ii = 0; ii < outputNets.size(); ii++)
            {
                std::cout << "Output Node : " << netArray[outputNets[ii] - 1].netNumber << " Value : " << netArray[outputNets[ii] - 1].netValue << endl;
            }
            
            //Check if the results.txt file is opened and print the outputs of the respective output nets.
            if (saveResult.is_open())
            {
                for (int ii = 0; ii < outputNets.size(); ii++)
                {
                    saveResult << netArray[outputNets[ii] - 1].netValue;
                }
                saveResult << endl;
            }

            //Reload the current stack with the logic gates to evaluate the outputs for the other inputs.
            if (currentStack.empty())
            {
                while (!evaluatedStack.empty())
                {
                    currentStack.push_back(evaluatedStack.back());
                    evaluatedStack.pop_back();
                }
            }
        }
        
        saveResult.close();
        inputsFile.close();
    }
    else
    {
        cerr << "Input File not found!" << endl;
        return 1;
    }

    return 0;
}