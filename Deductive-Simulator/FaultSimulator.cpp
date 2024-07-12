/*
    Author : Karthikeya Sharma M
    Last Date Modified : November 11, 2023
    Subject : ECE 6140 Digital Systems Testing

    Description:
    The main.cpp file that implements the fault Simulator.
*/

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cstring>
#include <ctime>
#include <set>
#include <unordered_set>
#include <cstdlib>
#include "nodesandedges.h"
#include "evaluateFaults.h"
#include "randomGenerator.h"

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

    //std::cout << "Stack Evaluation completed!" << endl;
}

//Propagate Fault list by comparing the actual logical inputs to execute one of the following operations - Intersection, Union or Difference based on the type of Gate input.
std::vector<string> propagateFaultList(LogicGate currentGate, std::vector<string> input1FaultList, std::vector<string> input2FaultList, string inputs)
{
    std::vector<string> outputFaultList;

    switch (currentGate.gateType)
    {
    case AND:
        switch (stoi(inputs, 0, 2))
        {
        case 0b00:
            return Intersection(input1FaultList, input2FaultList);
            break;
        case 0b01:
            return Difference(input1FaultList, input2FaultList);
            break;
        case 0b10:
            return Difference(input2FaultList, input1FaultList);
            break;
        case 0b11:
            return Union(input1FaultList, input2FaultList);
            break;
        }
        break;
    case NAND:
        switch (stoi(inputs, 0, 2))
        {
        case 0b00:
            return Intersection(input1FaultList, input2FaultList);
            break;
        case 0b01:
            return Difference(input1FaultList, input2FaultList);
            break;
        case 0b10:
            return Difference(input2FaultList, input1FaultList);
            break;
        case 0b11:
            return Union(input1FaultList, input2FaultList);
            break;
        }
        break;
    case OR:
        switch (stoi(inputs, 0, 2))
        {
        case 0b00:
            return Union(input1FaultList, input2FaultList);
            break;
        case 0b01:
            return Difference(input2FaultList, input1FaultList);
            break;
        case 0b10:
            return Difference(input1FaultList, input2FaultList);
            break;
        case 0b11:
            return Intersection(input1FaultList, input2FaultList);
            break;
        }
        break;
    case NOR:
        switch (stoi(inputs, 0, 2))
        {
        case 0b00:
            return Union(input1FaultList, input2FaultList);
            break;
        case 0b01:
            return Difference(input2FaultList, input1FaultList);
            break;
        case 0b10:
            return Difference(input1FaultList, input2FaultList);
            break;
        case 0b11:
            return Intersection(input1FaultList, input2FaultList);
            break;
        }
        break;
    case XOR:
        for (int ii = 0; ii < input1FaultList.size(); ii++)
        {
            for (int jj = 0; jj < input2FaultList.size(); jj++)
            {
                if (input1FaultList[ii] == input2FaultList[jj])
                {
                    input1FaultList.erase(input1FaultList.begin() + ii);
                    input2FaultList.erase(input2FaultList.begin() + jj);
                }
            }
        }
        return Union(input1FaultList, input2FaultList);
        break;
    case XNOR:
        for (int ii = 0; ii < input1FaultList.size(); ii++)
        {
            for (int jj = 0; jj < input2FaultList.size(); jj++)
            {
                if (input1FaultList[ii] != input2FaultList[jj])
                {
                    input1FaultList.erase(input1FaultList.begin() + ii);
                    input2FaultList.erase(input2FaultList.begin() + jj);
                }
            }
        }
        return Union(input1FaultList, input2FaultList);
        break;
    case INV:
        return input1FaultList;
        break;
    case BUF:
        return input1FaultList;
        break;
    }
}

//A function that computes fault coverage, and optionally outputs the list of faults detected.
double computeFaultCoverage(std::fstream& fs, int totalFaults) {
    std::string line;
    std::set<std::string> faultsDetected;
    double faultCoverage = 0.0;

    while (std::getline(fs, line)) {
        std::vector<std::string> lineToVect = splitString(line);
        if (lineToVect.size() >= 7 && lineToVect[0] == "Output" && lineToVect[5] == "List") {
            for (int ii = 7; ii < lineToVect.size(); ii++) {
                faultsDetected.insert(lineToVect[ii]);
            }
        }
    }

    //Print out the faults detected.
    cout << "Overall Detected Faults : ";
    for (const std::string& fault : faultsDetected) {
        std::cout << fault << " ";
    }
    cout << endl;

    int totalFaultsDetected = faultsDetected.size();
    std::cout << "Total Number of Detected Faults : " << totalFaultsDetected << endl;
    faultCoverage = (static_cast<double>(totalFaultsDetected) / static_cast<double>(totalFaults)) * 100;
    return faultCoverage;
}

int main(int argc, char* argv[])
{
    /*
        Declaring three stacks to simplify stack operations. Each stack is defined in brief:
        Current Stack holds the current gates to be popped in the stack.
        Evaluated Stack holds the gates whose outputs have been evaluated.
        Unevaluated stack holds the gates whose outputs have not been evaluated due to unavailability of both the inputs.
    */

    if (argc == 2 && (strcmp(argv[1], "-h") || strcmp(argv[1], "--help")))
    {
        cout << endl;
        cout << " --------------------------------------- " << endl;
        cout << " ------ Deductive Fault Simulator ------ " << endl;
        cout << " --------------------------------------- " << endl << endl;
        std::cout << " ----- Usage ----- " << endl << "<./a.exe (or) ./a.out> -netlist <value1> -netlistInputs <value2> -option <value3> -customInputs <value4> -customFaults <value5> -faultyInputs <value6> -numFaults <value7> -numInputVectors <value8>" << endl << endl;
        cout << " ----- Helper Arguments Sample ----- " << endl;
        cout << "Value 1 : netlist.txt (for example)" << endl;
        cout << "Value 2 : netlistInputs.txt (for example)" << endl;
        cout << "Value 3 : 0 for simulating faults on all nets, 1 for taking as input a list of faults from a file for simulation purposes." << endl; 
        cout << "Value 4 : 0 for random input vectors, 1 for custom input vectors" << endl;
        cout << "Value 5 : 0 for random stuck-at faults, 1 for custom stuck-at fault testing." << endl;
        cout << "Value 6 : Faults.txt (for example)" << endl;
        cout << "Value 7 : 188 (For example)" << endl; 
        cout << "Value 8 : 22 (For Example)" << endl;
        return 1;
    }

    if (argc != 17)
    {
        cout << "Incorrect Command Line Input!" << endl;
        std::cout << " ----- Usage ----- " << endl << "<./a.exe (or) ./a.out> -netlist <value1> -netlistInputs <value2> -option <value3> -customInputs <value4> -customFaults <value5> -faultyInputs <value6> -numFaults <value7> -numInputVectors <value8>" << endl << endl;
        cout << " ----- Helper Arguments Sample ----- " << endl;
        cout << "Value 1 : netlist.txt (for example)" << endl;
        cout << "Value 2 : netlistInputs.txt (for example)" << endl;
        cout << "Value 3 : 0 for simulating faults on all nets, 1 for taking as input a list of faults from a file for simulation purposes." << endl; 
        cout << "Value 4 : 0 for random input vectors, 1 for custom input vectors" << endl;
        cout << "Value 5 : 0 for random stuck-at faults, 1 for custom stuck-at fault testing." << endl;
        cout << "Value 6 : Faults.txt (for example)" << endl;
        cout << "Value 7 : 188 (For example)" << endl; 
        cout << "Value 8 : 22 (For Example)" << endl;
        return 1;
    }

    std::vector<LogicGate> currentStack, evaluatedStack, unevaluatedStack;

    //Maximum value of net in the given netlist.
    unsigned int nMaxNet = 0;

    //Declare a vector of
    std::vector<int> inputNets, outputNets, outputFaultyNets;

    //Create a file object
    fstream netlistFile;

    //Nelist path
    string path;

    //A string that stores the input file path.
    string inputFilePath;

    //A string that stores the faulty file path.
    string faultyFilePath;

    //Mode for specifying to either consider faults on all the circuit nets or inject faults in particular nets of the circuit.
    int mode;
    
    //numFaults to specify number of faults to be injected.
    int numFaults;
    
    //numVectors to specify number of Vectors to be used for the deductive fault simulation.
    int numVectors;

    bool customInputs, customFaults;

    if (argc > 1) 
    {
        for (int ii = 1; ii < argc; ii++) 
        {
            if (std::string(argv[ii]) == "-netlist" && ii + 1 < argc) 
            {
                path = argv[ii + 1];
            } 
            else if (std::string(argv[ii]) == "-netlistInputs" && ii + 1 < argc) 
            {
                inputFilePath = argv[ii + 1];
            }
            else if (std::string(argv[ii]) == "-option" && ii + 1 < argc)
            {
                mode = std::stoi(argv[ii+1]);
            }
            else if (std::string(argv[ii]) == "-faultyInputs" && ii + 1 < argc)
            {
                faultyFilePath = argv[ii + 1];
            }
            else if (std::string(argv[ii]) == "-numFaults" && ii + 1 < argc)
            {
                numFaults = std::stoi(argv[ii + 1]);
            }
            else if (std::string(argv[ii]) == "-numInputVectors" && ii + 1 < argc)
            {
                numVectors = std::stoi(argv[ii + 1]);
            }
            else if (std::string(argv[ii]) == "-customInputs" && ii + 1 < argc)
            {
                customInputs = std::stoi(argv[ii + 1]);
            }
            else if (std::string(argv[ii]) == "-customFaults" && ii + 1 < argc)
            {
                customFaults = std::stoi(argv[ii + 1]);
            }
        }
    }

    //Open the netlist file using the open method, input from user for file location (path and name)
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
    
    //Create a file object faultsFile
    fstream faultsFile;

    //Output stream object that stores the resultant output vectors.
    //ofstream saveCoverageResult("Results/coverageResult.txt", ios::trunc);

    //Open the file using the input file path entered.
    inputsFile.open(inputFilePath, ios::in);

    //Open the file using the input file path entered.
    faultsFile.open(faultyFilePath, ios::in);

    //Random Input Generation
    if (!customInputs)
    {
        randomVectorGenerator(numVectors, inputNets.size(), inputFilePath);
    }

    //Random Fault Generation
    if (mode == 1 && !customFaults)
    {
        randomFaultGenerator(numFaults, netArray.size(), faultyFilePath);
    }

    //String that stores input bit combination in the file in every line.
    string inputBits;

    //String that stores the node number and the faulty bit value.
    string faultyBitPos;

    //A vector that stores the input binary bits in the form of a vector.
    std::vector<int> binaryDigits;

    //Output stream object that stores the resultant output vectors.
    ofstream saveResult("Results/result.txt");

    //Create a vector of structures to hold the original value of Net data values after logic simulation.
    std::vector<Net> netArrayOG;

    //Create a vector of strings to hold the value of net number and fault per line of the faulty file.
    std::vector<string> netNumFault;

    unsigned int testVectCount = 0;

    if (inputsFile.is_open())
    {
        while (std::getline(inputsFile, inputBits))
        {
            unsigned int totalFaultsCount = 0;

            //Reset binaryDigits every iteration.
            binaryDigits.clear();

            // Reset netArray to its initial state before processing each line of input
            for (int jj = 0; jj < netArray.size(); jj++)
            {
                netArray[jj].netValue = 0; // Reset netValue to 0
                netArray[jj].dontCare = -1; // Reset dontCare to -1
                netArray[jj].faultList.clear(); //Clear the fault list.
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
            }

            //Call the function Evaluate circuit.
            evaluateCircuit(currentStack, evaluatedStack, unevaluatedStack, netArray);

            testVectCount += 1;

            //Assigning netArray to netArrayOG before injecting faults into netArray.
            netArrayOG = netArray;

            //Injecting faults.
            //Duplicating NetArray obtained - assign netArray to another vector of structs called netArrayOG. Inject the faults into the netArray.

            std::vector<unsigned int> faultsNets;

            if (mode == 1)
            {
                //Perform Fault Simulation.
                if (faultsFile.is_open())
                {
                    while (getline(faultsFile, faultyBitPos))
                    {
                        netNumFault = splitString(faultyBitPos);
                        if (!netNumFault.empty())
                        {
                            //netNumFault[0] = Net Number
                            //netNumFault[1] = Fault to be injected.
                            string faultyData = netNumFault[1];
                            std::bitset<1> faultyDataBinaryVal(faultyData);
                            netArray[stoi(netNumFault[0]) - 1].netValue = faultyDataBinaryVal;
                            totalFaultsCount++;
                            faultsNets.push_back(stoi(netNumFault[0]));
                            //cout << "Net Number : " << netArray[stoi(netNumFault[0]) - 1].netNumber << " Fault : " << netArray[stoi(netNumFault[0]) - 1].netValue << endl;
                        }
                    }
                }
            }

            //Reset the file pointer's position to the beginning of the file
            faultsFile.clear(); // Clear any error flags that might be set
            faultsFile.seekg(0, std::ios::beg);

            //Transfer the gates in the evaluated stack to the current stack.
            if (currentStack.empty())
            {
                while (!evaluatedStack.empty())
                {
                    currentStack.push_back(evaluatedStack.back());
                    evaluatedStack.pop_back();
                }
            }

            while (!currentStack.empty())
            {
                LogicGate currentGate = currentStack.back();
                evaluatedStack.push_back(currentGate);
                currentStack.pop_back();

                //Compare the popped out gate's input net values with the ones stored in the net array. If there is a value mismatch, add the net number and the fault value to the vector of fault list.
                bitset<1> gateInput1OG = netArrayOG[currentGate.edgeInputNetNum1 - 1].netValue;
                bitset<1> gateInput2OG = netArrayOG[currentGate.edgeInputNetNum2 - 1].netValue;
                bitset<1> gateOutputOG = netArrayOG[currentGate.edgeOutputNetNum - 1].netValue;

                /*
                    For mode 0, the fault is propagated if the net value is opposite to the input value. That is, considering all faults at all nets, one logic value (suppose logic 0)
                    can excite s-a-1 fault, and s-a-0 remains unexcited.
                */

                if (mode == 0)
                {
                    totalFaultsCount++;
                    bool setMatch = false;

                    //Maintain a flag variable, set to false by default. If match found, set to true, break out of the loop.
                    for (int ii = 0; ii < netArray[currentGate.edgeInputNetNum1 - 1].faultList.size(); ii++)
                    {
                        if (netArray[currentGate.edgeInputNetNum1 - 1].faultList[ii] == netAndStuckAtFault(currentGate.edgeInputNetNum1, ~gateInput1OG))
                        {
                            setMatch = true;
                            break;
                        }
                    }

                    if (!setMatch)
                    {
                        netArray[currentGate.edgeInputNetNum1 - 1].faultList.push_back(netAndStuckAtFault(currentGate.edgeInputNetNum1, ~gateInput1OG));
                    }
                    
                    if (currentGate.gateType == AND || currentGate.gateType == NAND || currentGate.gateType == OR || currentGate.gateType == NOR || currentGate.gateType == XOR || currentGate.gateType == XNOR)
                    {
                        bool setMatch = false;
                        
                        //Maintain a flag variable, set to false by default. If match found, set to true, break out of the loop.
                        for (int ii = 0; ii < netArray[currentGate.edgeInputNetNum2 - 1].faultList.size(); ii++)
                        {
                            if (netArray[currentGate.edgeInputNetNum2 - 1].faultList[ii] == netAndStuckAtFault(currentGate.edgeInputNetNum2, ~gateInput2OG))
                            {
                                setMatch = true;
                                break;
                            }
                        }

                        if (!setMatch)
                        {
                            netArray[currentGate.edgeInputNetNum2 - 1].faultList.push_back(netAndStuckAtFault(currentGate.edgeInputNetNum2, ~gateInput2OG));
                        }
                    }

                    //Combine binary Inputs into a string.
                    string combinedInputs = combineInputs(netArrayOG[currentGate.edgeInputNetNum1 - 1].netValue, netArrayOG[currentGate.edgeInputNetNum2 - 1].netValue);

                    //Call the propagatedFaults Function.
                    std::vector<string> propagatedFaults = propagateFaultList(currentGate, netArray[currentGate.edgeInputNetNum1 - 1].faultList, netArray[currentGate.edgeInputNetNum2 - 1].faultList, combinedInputs);

                    //Update the netArray with the propagated output fault list.
                    for (unsigned int ii = 0; ii < propagatedFaults.size(); ii++)
                    {
                        netArray[currentGate.edgeOutputNetNum - 1].faultList.push_back(propagatedFaults[ii]);
                    }

                    //If the circuit's output fault is excited, push it to the output fault list.
                    for (unsigned int kk = 0; kk < outputNets.size(); kk++)
                    {
                        if (currentGate.edgeOutputNetNum == outputNets[kk])
                        {
                            netArray[currentGate.edgeOutputNetNum - 1].faultList.push_back(netAndStuckAtFault(outputNets[kk], ~gateOutputOG));
                            break;
                        }
                    }
                }

                /*
                    For mode 1, where custom faults/random faults are injected in the circuit, extract the net values of the gate popped from the current stack to compare 
                    with the original logical values to detect if there is a fault in the net. Propagate the fault list accordingly.
                */

                else if (mode == 1)
                {
                    bitset<1> gateInput1 = netArray[currentGate.edgeInputNetNum1 - 1].netValue;
                    bitset<1> gateInput2 = netArray[currentGate.edgeInputNetNum2 - 1].netValue;
                    bitset<1> gateOutput = netArray[currentGate.edgeOutputNetNum - 1].netValue;

                    if (gateInput1 != gateInput1OG)
                    {
                        bool setMatch = false;

                        //check whether the fault is already existing in the fault list for the same net.
                        //Maintain a flag variable, set to false by default. If match found, set to true, break out of the loop.
                        for (int ii = 0; ii < netArray[currentGate.edgeInputNetNum1 - 1].faultList.size(); ii++)
                        {
                            if (netArray[currentGate.edgeInputNetNum1 - 1].faultList[ii] == netAndStuckAtFault(currentGate.edgeInputNetNum1, gateInput1))
                            {
                                setMatch = true;
                                break;
                            }
                        }

                        if (!setMatch)
                        {
                            netArray[currentGate.edgeInputNetNum1 - 1].faultList.push_back(netAndStuckAtFault(currentGate.edgeInputNetNum1, gateInput1));
                        }
                    }

                    if (currentGate.gateType == AND || currentGate.gateType == NAND || currentGate.gateType == OR || currentGate.gateType == NOR || currentGate.gateType == XOR || currentGate.gateType == XNOR)
                    {
                        bool setMatch = false;
                        if (gateInput2 != gateInput2OG)
                        {
                            //check whether the fault is already existing in the fault list for the same net.
                            //Maintain a flag variable, set to false by default. If match found, set to true, break out of the loop.
                            for (int ii = 0; ii < netArray[currentGate.edgeInputNetNum2 - 1].faultList.size(); ii++)
                            {
                                if (netArray[currentGate.edgeInputNetNum2 - 1].faultList[ii] == netAndStuckAtFault(currentGate.edgeInputNetNum2, gateInput2))
                                {
                                    setMatch = true;
                                    break;
                                }
                            }

                            if (!setMatch)
                            {
                                netArray[currentGate.edgeInputNetNum2 - 1].faultList.push_back(netAndStuckAtFault(currentGate.edgeInputNetNum2, gateInput2));
                            }
                        }
                    }

                    //Combine binary Inputs into a string.
                    string combinedInputs = combineInputs(netArrayOG[currentGate.edgeInputNetNum1 - 1].netValue, netArrayOG[currentGate.edgeInputNetNum2 - 1].netValue);

                    //Call the propagatedFaults Function.
                    std::vector<string> propagatedFaults = propagateFaultList(currentGate, netArray[currentGate.edgeInputNetNum1 - 1].faultList, netArray[currentGate.edgeInputNetNum2 - 1].faultList, combinedInputs);

                    //Update the netArray with the propagated output fault list.
                    for (unsigned int ii = 0; ii < propagatedFaults.size(); ii++)
                    {
                        netArray[currentGate.edgeOutputNetNum - 1].faultList.push_back(propagatedFaults[ii]);
                    }

                    //If the circuit's output fault is excited, push it to the output fault list
                    for (unsigned int kk = 0; kk < outputNets.size(); kk++)
                    {
                        if (currentGate.edgeOutputNetNum == outputNets[kk] && gateOutput != gateOutputOG)
                        {
                            netArray[currentGate.edgeOutputNetNum - 1].faultList.push_back(netAndStuckAtFault(outputNets[kk], netArray[currentGate.edgeOutputNetNum - 1].netValue));
                            break;
                        }
                    }
                }  
            }   

            std::unordered_set<std::string> totalFaultsDetected;
            int currentTestVectFaultsDetected;

            //Check if the results.txt file is opened and print the outputs of the respective output nets.
            if (saveResult.is_open())
            {
                saveResult << "Test Vector : " << testVectCount << endl;
                for (int ii = 0; ii < outputNets.size(); ii++)
                {
                    if (!(netArray[outputNets[ii] - 1].faultList.size() > 0))
                    {
                        saveResult << "Output Net : " << outputNets[ii] << " Faults Not Detected by the Test Vector." << endl;
                        continue;
                    }

                    saveResult << "Output Net : " << outputNets[ii] << " Fault List : ";
                    for (int jj = 0; jj < netArray[outputNets[ii] - 1].faultList.size(); jj++)
                    {
                        saveResult << netArray[outputNets[ii] - 1].faultList[jj] << " ";
                        if (totalFaultsDetected.find(netArray[outputNets[ii] - 1].faultList[jj]) == totalFaultsDetected.end())
                        {
                            totalFaultsDetected.insert(netArray[outputNets[ii] - 1].faultList[jj]);
                        }
                    }
                    saveResult << endl;
                }

                currentTestVectFaultsDetected = totalFaultsDetected.size();
                
                saveResult << "Fault coverage for the Test Vector : " << (static_cast<double>(totalFaultsDetected.size()) / totalFaultsCount) * 100 << "% and Number of Faults Detected : " << currentTestVectFaultsDetected << endl;
                saveResult << endl;
            }

            totalFaultsDetected.clear();

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

        if (mode == 1)
        {
            faultsFile.close();
        }
    }
    else
    {
        cerr << "Input File not found!" << endl;
        return 1;
    }

    //Open a fstream to iterate through the "result.txt" file to compute the fault coverage.
    fstream faults;
    faults.open("Results/result.txt", ios::in);
    double faultCoverage;

    //If mode = 1, the fault coverage is computed according to the number of faults injected in the circuit (for the purpose of deductive fault simulation)
    //If mode = 0, the fault coverage is computed considering faults on all the circuit nets.
    if (mode)
    {
        faultCoverage = computeFaultCoverage(faults, numFaults);
    }
    else
    {
        faultCoverage = computeFaultCoverage(faults, 2*netArray.size());
    }

    std::cout << "Fault Coverage : " << faultCoverage << "%" << endl;

    //saveCoverageResult << faultCoverage << endl;
    //saveCoverageResult.close();
    
    return 0;
}