/*
    Author : Karthikeya Sharma M
    Last Date Modified : December 4, 2023
    Subject : ECE 6140 Digital Systems Testing

    Description:
    The main.cpp file that implements PODEM (Path Oriented DEcision Making) Fault Simulation.
*/

#include <iostream>
#include <string>
#include <fstream>
#include "podem.h"
#include <cstring>
#include <vector>
#include <bitset>
#include "randomGenerator.h"
#include "testResultsParser.h"

using namespace std;

bool forwardSimulate = false;
bool status = false;

int count = 0;

std::vector<string> previousCircuitState;

/*
    ------------------------------------ BackTrace function ------------------------------------------
    This function is responsible for navigating to the primary input of the ciruit through an "x-path".
    @param objVect is the vector of strings which has values [k, vk] returned by objective function.
    @param netArray is the vector of structures of nets in the circuit.
    @param currentStack is the vector of structues of logic gates in the circuit.
    @param netValue is the object of the structure netVal, that stores the different possible netvalues - x,D,D',0 and 1.
*/

std::vector<string> BackTrace(std::vector<string> objVect, std::vector<AdvancedNet>& netArray, std::vector<LogicGate>& currentStack, netVal netValue)
{
    //Set the forwardSimulate flag to false by default.
    if (forwardSimulate)
    {
        forwardSimulate = false;
    }

    //k is net number, vk is net value corresponding to the net k
    unsigned int k = stoi(objVect[0]);
    std::bitset<1> vk = stoi(objVect[1]);

    std::bitset<1> v = vk;

    //Store the gate inputs.
    unsigned int gateInputs[2];

    //Primary Input net and value required for implication to be returned by the function.
    std::vector<string> primaryInput;

    std::cout << "Back Trace Path : ";

    //while k is a gate output
    while (netArray[k - 1].outputGateNum)
    {
        //i = inversion of k
        std::bitset<1> i; //i = inversion

        //if the gate type enum is INV, NOR, NAND, XNOR, the inversion of these gates is 1, else 0
        if (currentStack[(netArray[k - 1].outputGateNum) - 1].gateType == INV || currentStack[(netArray[k - 1].outputGateNum) - 1].gateType == NAND || currentStack[(netArray[k - 1].outputGateNum) - 1].gateType == NOR || currentStack[(netArray[k - 1].outputGateNum) - 1].gateType == XNOR)
            i = 1;
        else
            i = 0;

        gateInputs[0] = currentStack[(netArray[k - 1].outputGateNum) - 1].edgeInputNetNum1;
        gateInputs[1] = currentStack[(netArray[k - 1].outputGateNum) - 1].edgeInputNetNum2;

        std::vector<unsigned int> index;
        for (unsigned int ii = 0; ii < 2; ii++)
        {
            if (netArray[gateInputs[ii] - 1].netValueString == netValue.dontCare)
            {
                index.push_back(ii);
            }
        }

        //Stores the index to be selected from the index vector (the gate inputs)
        unsigned int j;

        if (index.size() == 1)
        {
            j = index[0];
        }
        else if (index.size() == 0)
        {
            //make the flag true
            forwardSimulate = true;

            //return back to PODEM function
            std::vector<string> f;
            f.push_back(to_string(gateInputs[0]));
            f.push_back(netArray[gateInputs[0] - 1].netValueString);
            return f;
        }
        else if (index.size() == 2)
        {
            //select an input (j) of k with value x (randomly select a gate)
            srand(static_cast<unsigned int>(std::time(nullptr)));

            //Randomly select an element from the gateInputs array bounds, need to do the same thing as objective.
            j = std::rand() % 2;
        }

        //assign the element selected to a variable randomInput.
        unsigned int randomInput = gateInputs[j];

        //XOR of the value on that "output" of the gate with the inversion parity of the gate.
        v = v ^ i;

        //Assigning k (output net of the current gate) to the output net of the next gate, which essentially is the input net of the current gate.
        k = randomInput;

        std::cout << k << " ";

    }

    std::cout << std::endl;

    //Packing (j,vj) in a vector to be passed to the imply function.
    primaryInput.push_back(to_string(k));
    primaryInput.push_back(v.to_string());

    std::cout << "PI Assignment (j, vj) : (" << primaryInput[0] << "," << primaryInput[1] << ")" << std::endl;

    //return the vector.
    return primaryInput;
}

/*
    ----------------------------------------------------------- Objective -------------------------------------------------------------------------------
    This function is responsible for instructing the other functions in PODEM() to bring a value opposite to the fault to excite the fault.
    Further, it is also responsible for assigning non-controlling values on any selected gate on the D-Frontier to make the fault propagate to the output.

    @param faultSaVal is a vector that contains the "faulty" net and the "current" value on that net.
    @param targetFault is a string that conveys which value is expected to be set on that "faulty" net to excite the fault.
    @param netArray is the vector of structures of nets in the circuit.
    @param currentStack is the vector of structues of logic gates in the circuit.
    @param netValue is an object of the structure netVal that contains different possible values of the net - 0,1,x,D,D'.
*/

std::vector<string> Objective(std::vector<string>& faultSaVal, string targetFault, std::vector<AdvancedNet>& netArray, std::vector<LogicGate>& currentStack, netVal netValue)
{
    /*
        The target Stuck-at fault is l-s-a-v
        -> faultSaVal[0] = l
        -> faultSaVal[1] = stuck-at fault value (net value)
    */

    //Set forward Simulate to false by default
    if (forwardSimulate)
    {
        forwardSimulate = false;
    }

    //If the value of l is "x" (don't care), then assign v bar to the line. v is the target fault.
    if (faultSaVal[1] == netValue.dontCare)
    {
        std::vector<string> v;
        v.push_back(faultSaVal[0]);
        if (targetFault == netValue.high)
        {
            v.push_back(netValue.low);
        }
        else if (targetFault == netValue.low)
        {
            v.push_back(netValue.high);
        }

        //Once the objective is set, can assign the targetFault to the vector element faSaVal[1] to execute the other part of the function in the next function call.
        if (netArray[stoi(faultSaVal[0]) - 1].netValueString == netValue.D || netArray[stoi(faultSaVal[0]) - 1].netValueString == netValue.Dbar)
        {
            if (targetFault == netValue.high)
            {
                faultSaVal[1] = netValue.Dbar;
                status = true;
            }
            else if (targetFault == netValue.low)
            {
                faultSaVal[1] = netValue.D;
                status = true;
            }
        }

        if (!status)
        {
            std::cout << "Objective (k, vk) : (" << v[0] << "," << v[1] << ")" << std::endl;
            return v;
        }
    }

    status = false;

    //Can be executed only when DFrontier has a logic gate.
    if (!DFrontier.empty())
    {
        //Select a gate g from the D frontier
        srand(static_cast<unsigned int>(std::time(nullptr)));
        unsigned int i = std::rand() % DFrontier.size();
        LogicGate g = DFrontier[i];

        //select an input j from the gate g with value x

        //Extract the gate input values in gateInputValues array
        string gateInputValues[2];
        gateInputValues[0] = netArray[g.edgeInputNetNum1 - 1].netValueString;
        gateInputValues[1] = netArray[g.edgeInputNetNum2 - 1].netValueString;

        //check both the inputs for don't care value, select one of them at random.
        std::vector<int> index;
        for (unsigned int ii = 0; ii < 2; ii++)
        {
            if (gateInputValues[ii] == netValue.dontCare)
                index.push_back(ii);
        }

        unsigned int j;
        //If both the inputs have "don't care" value, one of the inputs is selected at random. Else, the only input with "don't care" is selected.
        if (index.size() == 1)
        {
            j = index[0];
        }
        else if (index.size() == 0)
        {
            //make the flag true
            forwardSimulate = true;

            //return back to PODEM function
            std::vector<string> f;
            f.push_back(to_string(g.edgeInputNetNum1));
            f.push_back(gateInputValues[0]);
            return f;
        }
        else
        {
            srand(static_cast<unsigned int>(std::time(nullptr)));
            j = std::rand() % (index.size());
        }

        //jj stores the the gate input net number.
        unsigned int jj;

        if (j == 0)
        {
            jj = g.edgeInputNetNum1;
        }
        else if (j == 1)
        {
            jj = g.edgeInputNetNum2;
        }

        //c = controlling value of G
        string c;
        if (g.gateType == AND || g.gateType == NAND)
        {
            c = netValue.low;
        }
        else if (g.gateType == OR || g.gateType == NOR)
        {
            c = netValue.high;
        }

        //Objective to be returned for BackTracing.
        std::vector<string> objectiveOutput;
        objectiveOutput.push_back(to_string(jj));

        //Complement of controlling value of gate G.
        string cbar;
        if (c == netValue.high)
            cbar = netValue.low;
        else if (c == netValue.low)
            cbar = netValue.high;

        objectiveOutput.push_back(cbar);
        std::cout << "Objective (k, vk) : (" << objectiveOutput[0] << "," << objectiveOutput[1] << ")" << endl;

        //Return the objective packed in a vector (k,vk), this will be further used by the BackTrace function.
        return objectiveOutput;
    }
    else
    {
        std::vector<string> failure;
        failure.push_back("-1");
        failure.push_back("-1");
        std::cout << "FAILURE, DFrontier Empty" << std::endl;
        return failure;
    }
}

/*
    ----------------------------------------- ControlVal --------------------------------------------
    This function returns the control value of the gate accoording to the type of gate
    @param gateType is an enum that uniquely identifies the gate type based on the number input given.
*/

bool controlVal(unsigned int gateType)
{
    if (gateType == AND || gateType == NAND)
    {
        return 0;
    }
    else if (gateType == OR || gateType == NOR)
    {
        return 1;
    }
}

/*
    ---------------------------------------------------- updateDFrontiers ---------------------------------------------------------
    This function is responsible for updating the DFrontiers every time PODEM() calls itself.
    @param netArray is the vector of structures of nets in the circuit.
    @param currentStack is the vector of structues of logic gates in the circuit.
    @param netValue is an object of the structure netVal that contains different possible values of the net - 0,1,x,D,D'.
    @param DFrontier contains those gates whose inputs are either D or D'.
*/

void updateDfrontiers(std::vector<LogicGate>& currentStack, std::vector<AdvancedNet>& netArray, netVal netValue, std::vector<LogicGate>& DFrontier)
{

    //Iterate through all the nets and check each gate's input values.
    for (unsigned int ii = 0; ii < currentStack.size(); ii++)
    {
        if (currentStack[ii].gateType != INV || currentStack[ii].gateType != BUF)
        {
            if (netArray[(currentStack[ii].edgeOutputNetNum) - 1].netValueString == netValue.dontCare)
            {
                bool alreadyExists = false;

                for (const auto& element : DFrontier)
                {
                    if (element.gateNumber == currentStack[ii].gateNumber)
                    {
                        alreadyExists = true;
                        break;
                    }
                }

                if (!alreadyExists)
                {
                    if ((netArray[(currentStack[ii].edgeInputNetNum1) - 1].netValueString == netValue.D || netArray[(currentStack[ii].edgeInputNetNum1) - 1].netValueString == netValue.Dbar) || (netArray[(currentStack[ii].edgeInputNetNum2) - 1].netValueString == netValue.D || netArray[(currentStack[ii].edgeInputNetNum2) - 1].netValueString == netValue.Dbar))
                    {
                        DFrontier.push_back(currentStack[ii]);
                    }
                }
            }
        }
    }


    //Iterate through all the gates in the DFrontier to check if the inputs are determined by Imply(). If yes, remove them gates from the DFrontier.
    for (auto it = DFrontier.begin(); it != DFrontier.end();)
    {
        LogicGate gate = *it;

        if (netArray[gate.edgeOutputNetNum - 1].netValueString != netValue.dontCare)
        {
            // Erase and get the iterator to the next element
            it = DFrontier.erase(it);
        }
        else
        {
            // Move to the next element
            ++it;
        }
    }

    std::cout << std::endl << "D-Frontier Gates : ";
    for (unsigned int ii = 0; ii < DFrontier.size(); ii++)
    {
        cout << DFrontier[ii].gateNumber << " ";
        //cout << DFrontier[ii].edgeInputNetNum1 << " " << DFrontier[ii].edgeInputNetNum2 << " " << DFrontier[ii].edgeOutputNetNum << endl;
    }
    std::cout << std::endl;
}

/*
    ---------------------------------------------------------------------- Imply -----------------------------------------------------------------------------
    This function does the "forward simulation" of the circuit, setting the other inputs of the gate whose one of the inputs is set to a non-controlling value.
    @param backTraceVect is a vector that returns (j,vj), the primary input net number and value, from where the simulation needs to be started.
    @param netArray is the vector of structures of nets in the circuit.
    @param currentStack is the vector of structues of logic gates in the circuit.
    @param netValue is an object of the structure netVal that contains different possible values of the net - 0,1,x,D,D'.
    @param faultSaVal is a vector that contains the "faulty" net and the "current" value on that net.
    @param DFrontier contains those gates whose inputs are either D or D'.
*/

bool Imply(std::vector<string> backTraceVect, std::vector<LogicGate>& currentStack, std::vector<AdvancedNet>& netArray, std::vector<unsigned int>& inputNets, netVal netValue, std::vector<string>& faultSaVal, std::vector<LogicGate>& DFrontier)
{
    
    // for (unsigned int ii = 0; ii < netArray.size(); ii++)
    // {
    //     cout << netArray[ii].netValueString << " ";
    // }
    // std::cout << std::endl << "--------------------------------------------------------" << std::endl;
    

    unsigned int j = stoi(backTraceVect[0]);
    string vj = backTraceVect[1];

    unsigned int gi1, gi2, gout, gType;

    //Clear the previous state before updating the next "previous state" of the circuit before implying.
    previousCircuitState.clear();

    //Save the previous state of the circuit before implication.
    for (unsigned int jj = 0; jj < netArray.size(); jj++)
    {
        previousCircuitState.push_back(netArray[jj].netValueString);
    }

    //Set the PI net value
    netArray[j - 1].netValueString = vj;

    //Iterate through all the gates to imply (logic simulate) the other "implied" outputs. Loop throughout the gateArray to imply the outputs for the available inputs.
    for (unsigned int ii = 0; ii < currentStack.size(); ii++)
    {
        gi1 = currentStack[ii].edgeInputNetNum1;
        gi2 = currentStack[ii].edgeInputNetNum2;
        gout = currentStack[ii].edgeOutputNetNum;
        gType = currentStack[ii].gateType;

        //If the Fault is located at a primary input, make it D/Dbar.
        if (gType == INV || gType == BUF)
        {
            for (unsigned int kk = 0; kk < inputNets.size(); kk++)
            {
                //Check if this net is a faulty net.
                if (gi1 == inputNets[kk] && gi1 == stoi(faultSaVal[0]))
                {
                    if (netArray[gi1 - 1].netValueString == faultSaVal[1])
                    {
                        //Fault Not Excited.
                        cout << "FAILURE, fault not Excited" << endl;
                        return false;
                    }
                    else
                    {
                        if (netArray[gi1 - 1].netValueString == netValue.low && faultSaVal[1] == netValue.high)
                        {
                            netArray[gi1 - 1].netValueString = netValue.Dbar;
                        }
                        else if (netArray[gi1 - 1].netValueString == netValue.high && faultSaVal[1] == netValue.low)
                        {
                            netArray[gi1 - 1].netValueString = netValue.D;
                        }
                    }
                }
            }
        }
        else
        {
            for (unsigned int kk = 0; kk < inputNets.size(); kk++)
            {
                //Check if this net is a faulty net.
                if (gi1 == inputNets[kk] && gi1 == stoi(faultSaVal[0]))
                {
                    if (netArray[gi1 - 1].netValueString == faultSaVal[1])
                    {
                        //Fault Not Excited.
                        cout << "FAILURE, fault not Excited" << endl;
                        return false;
                    }
                    else
                    {
                        if (netArray[gi1 - 1].netValueString == netValue.low && faultSaVal[1] == netValue.high)
                        {
                            netArray[gi1 - 1].netValueString = netValue.Dbar;
                        }
                        else if (netArray[gi1 - 1].netValueString == netValue.high && faultSaVal[1] == netValue.low)
                        {
                            netArray[gi1 - 1].netValueString = netValue.D;
                        }
                    }
                }

                //Check if this net is a faulty net.
                if (gi2 == inputNets[kk] && gi2 == stoi(faultSaVal[0]))
                {
                    if (netArray[gi2 - 1].netValueString == faultSaVal[1])
                    {
                        //Fault Not Excited.
                        cout << "FAILURE, fault not Excited" << endl;
                        return false;
                    }
                    else
                    {
                        if (netArray[gi2 - 1].netValueString == netValue.low && faultSaVal[1] == netValue.high)
                        {
                            netArray[gi2 - 1].netValueString = netValue.Dbar;
                        }
                        else if (netArray[gi2 - 1].netValueString == netValue.high && faultSaVal[1] == netValue.low)
                        {
                            netArray[gi2 - 1].netValueString = netValue.D;
                        }
                    }
                }
            }
        }

        if (netArray[gout - 1].netValueString == netValue.dontCare)
        {
            if (netArray[gi1 - 1].netValueString != netValue.dontCare && (gType == INV || gType == BUF))
            {
                // Check if the input net is the faulty net. If yes, check if the fault can be excited or not. 
                // If yes, store the D/Dbar in a string called excitedFault and pass it to the evaluateGateInput function, without modifying the netArray inputs to the gate.
                string evalOutput;

                if (gi1 == stoi(faultSaVal[0]))
                {
                    if (netArray[gi1 - 1].netValueString == faultSaVal[1])
                    {
                        //Fault Not Excited.
                        cout << "FAILURE, fault not Excited" << endl;
                        return false;
                    }
                    else
                    {
                        if (netArray[gi1 - 1].netValueString == netValue.low && faultSaVal[1] == netValue.high)
                        {
                            //Exciting stuck-at 1
                            netArray[gi1 - 1].netValueString = netValue.Dbar;
                        }
                        else if (netArray[gi1 - 1].netValueString == netValue.high && faultSaVal[1] == netValue.low)
                        {
                            //Exciting stuck-at 0
                            netArray[gi1 - 1].netValueString = netValue.D;
                        }
                    }
                }

                evalOutput = evaluateGateInput(currentStack[ii], netArray[gi1 - 1].netValueString, netArray[gi1 - 1].netValueString);

                //need to check if evalOutput is exciting a fault or not. If yes, return D/Dbar. Else, return FAILURE.
                if (currentStack[ii].edgeOutputNetNum == stoi(faultSaVal[0]))
                {
                    if (evalOutput == faultSaVal[1])
                    {
                        //Fault Not Excited.
                        cout << "FAILURE, fault not Excited" << endl;
                        return false;
                    }
                    else
                    {
                        if (evalOutput == netValue.low && faultSaVal[1] == netValue.high)
                        {
                            //Exciting stuck-at 1
                            evalOutput = netValue.Dbar;
                        }
                        else if (evalOutput == netValue.high && faultSaVal[1] == netValue.low)
                        {
                            //Exciting stuck-at 0
                            evalOutput = netValue.D;
                        }
                    }
                }

                netArray[(currentStack[ii].edgeOutputNetNum) - 1].netValueString = evalOutput;
            }

            else if (netArray[gi1 - 1].netValueString == to_string(controlVal(gType)) && (gType == AND || gType == NAND || gType == OR || gType == NOR))
            {
                // Check if the input net is the faulty net. If yes, check if the fault can be excited or not. 
                // If yes, store the D/Dbar in a string called excitedFault and pass it to the evaluateGateInput function, without modifying the netArray inputs to the gate.
                string evalOutput;

                //Check if the input net 1 is the faulty net.
                if (currentStack[ii].edgeInputNetNum1 == stoi(faultSaVal[0]))
                {
                    if (netArray[gi1 - 1].netValueString == faultSaVal[1])
                    {
                        //Fault Not Excited.
                        cout << "FAILURE, fault not Excited" << endl;
                        return false;
                    }
                    else
                    {
                        if (netArray[gi1 - 1].netValueString == netValue.low && faultSaVal[1] == netValue.high)
                        {
                            //Exciting stuck-at 1
                            netArray[gi1 - 1].netValueString = netValue.Dbar;
                        }
                        else if (netArray[gi1 - 1].netValueString == netValue.high && faultSaVal[1] == netValue.low)
                        {
                            //Exciting stuck-at 0
                            netArray[gi1 - 1].netValueString = netValue.D;
                        }
                    }
                }
                //Check if the input net 2 is the faulty net.
                else if (currentStack[ii].edgeInputNetNum2 == stoi(faultSaVal[0]))
                {
                    if (netArray[gi2 - 1].netValueString == faultSaVal[1])
                    {
                        //Fault Not Excited.
                        cout << "FAILURE, fault not Excited" << endl;
                        return false;
                    }
                    else
                    {
                        if (netArray[gi2 - 1].netValueString == netValue.low && faultSaVal[1] == netValue.high)
                        {
                            //Exciting stuck-at 1
                            netArray[gi2 - 1].netValueString = netValue.Dbar;
                        }
                        else if (netArray[gi2 - 1].netValueString == netValue.high && faultSaVal[1] == netValue.low)
                        {
                            //Exciting stuck-at 0
                            netArray[gi2 - 1].netValueString = netValue.D;
                        }
                    }
                }

                evalOutput = evaluateGateInput(currentStack[ii], netArray[gi1 - 1].netValueString, netArray[gi2 - 1].netValueString);

                //need to check if evalOutput is exciting a fault or not. If yes, return D/Dbar. Else, return FAILURE.
                if (currentStack[ii].edgeOutputNetNum == stoi(faultSaVal[0]))
                {
                    if (evalOutput == faultSaVal[1])
                    {
                        //Fault Not Excited.
                        cout << "FAILURE, fault not Excited" << endl;
                        return false;
                    }
                    else
                    {
                        if (evalOutput == netValue.low && faultSaVal[1] == netValue.high)
                        {
                            //Exciting stuck-at 1
                            evalOutput = netValue.Dbar;
                        }
                        else if (evalOutput == netValue.high && faultSaVal[1] == netValue.low)
                        {
                            //Exciting stuck-at 0
                            evalOutput = netValue.D;
                        }
                    }
                }

                netArray[(currentStack[ii].edgeOutputNetNum) - 1].netValueString = evalOutput;
            }

            else if (netArray[(currentStack[ii].edgeInputNetNum2) - 1].netValueString == to_string(controlVal(gType)) && (gType == AND || gType == NAND || gType == OR || gType == NOR))
            {
                // Check if the input net is the faulty net. If yes, check if the fault can be excited or not. 
                // If yes, store the D/Dbar in a string called excitedFault and pass it to the evaluateGateInput function, without modifying the netArray inputs to the gate.
                string evalOutput;

                if (currentStack[ii].edgeInputNetNum1 == stoi(faultSaVal[0]))
                {
                    if (netArray[gi1 - 1].netValueString == faultSaVal[1])
                    {
                        //Fault Not Excited.
                        cout << "FAILURE, fault not Excited" << endl;
                        return false;
                    }
                    else
                    {
                        if (netArray[gi1 - 1].netValueString == netValue.low && faultSaVal[1] == netValue.high)
                        {
                            //Exciting stuck-at 1
                            netArray[gi1 - 1].netValueString = netValue.Dbar;
                        }
                        else if (netArray[gi1 - 1].netValueString == netValue.high && faultSaVal[1] == netValue.low)
                        {
                            //Exciting stuck-at 0
                            netArray[gi1 - 1].netValueString = netValue.D;
                        }
                    }
                }
                else if (currentStack[ii].edgeInputNetNum2 == stoi(faultSaVal[0]))
                {
                    if (netArray[gi2 - 1].netValueString == faultSaVal[1])
                    {
                        //Fault Not Excited.
                        cout << "FAILURE, fault not Excited" << endl;
                        return false;
                    }
                    else
                    {
                        if (netArray[gi2 - 1].netValueString == netValue.low && faultSaVal[1] == netValue.high)
                        {
                            //Exciting stuck-at 1
                            netArray[gi2 - 1].netValueString = netValue.Dbar;
                        }
                        else if (netArray[gi2 - 1].netValueString == netValue.high && faultSaVal[1] == netValue.low)
                        {
                            //Exciting stuck-at 0
                            netArray[gi2 - 1].netValueString = netValue.D;
                        }
                    }
                }

                evalOutput = evaluateGateInput(currentStack[ii], netArray[gi1 - 1].netValueString, netArray[gi2 - 1].netValueString);

                //need to check if evalOutput is exciting a fault or not. If yes, return D/Dbar. Else, return FAILURE.
                if (currentStack[ii].edgeOutputNetNum == stoi(faultSaVal[0]))
                {
                    if (evalOutput == faultSaVal[1])
                    {
                        //Fault Not Excited.
                        cout << "FAILURE, fault not Excited" << endl;
                        return false;
                    }
                    else
                    {
                        if (evalOutput == netValue.low && faultSaVal[1] == netValue.high)
                        {
                            //Exciting stuck-at 1
                            evalOutput = netValue.Dbar;
                        }
                        else if (evalOutput == netValue.high && faultSaVal[1] == netValue.low)
                        {
                            //Exciting stuck-at 0
                            evalOutput = netValue.D;
                        }
                    }
                }

                netArray[(currentStack[ii].edgeOutputNetNum) - 1].netValueString = evalOutput;
            }

            else if (netArray[(currentStack[ii].edgeInputNetNum1) - 1].netValueString != netValue.dontCare && netArray[(currentStack[ii].edgeInputNetNum2) - 1].netValueString != netValue.dontCare && (gType == AND || gType == NAND || gType == OR || gType == NOR))
            {
                // Check if the input net is the faulty net. If yes, check if the fault can be excited or not. 
                // If yes, store the D/Dbar in a string called excitedFault and pass it to the evaluateGateInput function, without modifying the netArray inputs to the gate.
                string evalOutput;

                if (currentStack[ii].edgeInputNetNum1 == stoi(faultSaVal[0]))
                {
                    if (netArray[gi1 - 1].netValueString == faultSaVal[1])
                    {
                        //Fault Not Excited.
                        cout << "FAILURE, fault not Excited" << endl;
                        return false;
                    }
                    else
                    {
                        if (netArray[gi1 - 1].netValueString == netValue.low && faultSaVal[1] == netValue.high)
                        {
                            //Exciting stuck-at 1
                            netArray[gi1 - 1].netValueString = netValue.Dbar;
                        }
                        else if (netArray[gi1 - 1].netValueString == netValue.high && faultSaVal[1] == netValue.low)
                        {
                            //Exciting stuck-at 0
                            netArray[gi1 - 1].netValueString = netValue.D;
                        }
                    }
                }
                else if (currentStack[ii].edgeInputNetNum2 == stoi(faultSaVal[0]))
                {
                    if (netArray[gi2 - 1].netValueString == faultSaVal[1])
                    {
                        //Fault Not Excited.
                        cout << "FAILURE, fault not Excited" << endl;
                        return false;
                    }
                    else
                    {
                        if (netArray[gi2 - 1].netValueString == netValue.low && faultSaVal[1] == netValue.high)
                        {
                            //Exciting stuck-at 1
                            netArray[gi2 - 1].netValueString = netValue.Dbar;
                        }
                        else if (netArray[gi2 - 1].netValueString == netValue.high && faultSaVal[1] == netValue.low)
                        {
                            //Exciting stuck-at 0
                            netArray[gi2 - 1].netValueString = netValue.D;
                        }
                    }
                }

                evalOutput = evaluateGateInput(currentStack[ii], netArray[gi1 - 1].netValueString, netArray[gi2 - 1].netValueString);

                //need to check if evalOutput is exciting a fault or not. If yes, return D/Dbar. Else, return FAILURE.
                if (currentStack[ii].edgeOutputNetNum == stoi(faultSaVal[0]))
                {
                    if (evalOutput == faultSaVal[1])
                    {
                        //Fault Not Excited.
                        cout << "FAILURE, fault not Excited" << endl;
                        return false;
                    }
                    else
                    {
                        if (evalOutput == netValue.low && faultSaVal[1] == netValue.high)
                        {
                            //Exciting stuck-at 1
                            evalOutput = netValue.Dbar;
                        }
                        else if (evalOutput == netValue.high && faultSaVal[1] == netValue.low)
                        {
                            //Exciting stuck-at 0
                            evalOutput = netValue.D;
                        }
                    }
                }

                netArray[(currentStack[ii].edgeOutputNetNum) - 1].netValueString = evalOutput;
            }
        }
    }

    return true;
}

/*
    ------------------------------------------------------------- PODEM -------------------------------------------------------------
    Main PODEM function, all other functions are called from here.
    @param netArray is the vector of structures of nets in the circuit.
    @param currentStack is the vector of structues of logic gates in the circuit.
    @param outputNets is a vector that contains the output net numbers of the circuit.
    @param netValue is an object of the structure netVal that contains different possible values of the net - 0,1,x,D,D'.
    @param faultSaVal is a vector that contains the "faulty" net and the "current" value on that net.
    @param DFrontier contains those gates whose inputs are either D or D'.
    @param outputStream is a file object that will be used to write the outputs given by PODEM.
*/

bool podem(std::vector<AdvancedNet>& netArray, std::vector<LogicGate>& currentStack, std::vector<unsigned int>& outputNets, std::vector<unsigned int>& inputNets, netVal netValue, std::vector<string> faultSaVal, string targetVal, std::vector<string> faultSaValOG, std::vector<LogicGate>& DFrontier, std::ofstream &outputStream)
{
    /*
        1. If (error at PO) then return SUCCESS
            Iterate through the primary outputs to check if the fault has been propagted to the output, fault list (keep checking for one fault only)
        2. If (Test not possible) then return FAILURE
            (a) Line l stuck-at v has value v
            (b) Dfrontier is empty (X-path check)
            (c) Error propagation look ahead will become empty.
    */
    updateDfrontiers(currentStack, netArray, netValue, DFrontier);

    //Iterate the output nets to see if the output net has a fault. If so, return SUCCESS
    for (unsigned int ii = 0; ii < outputNets.size(); ii++)
    {
        if (netArray[outputNets[ii] - 1].netValueString == netValue.D || netArray[outputNets[ii] - 1].netValueString == netValue.Dbar)
        {
            cout << "TEST VECTOR : ";

            // cout << "Final input values before D/D' conversion" << endl;
            // for (unsigned int ii = 0; ii < inputNets.size(); ii++)
            // {
            //     cout << netArray[inputNets[ii] - 1].netValueString << " ";
            // }
            // std::cout << std::endl << "--------------------------------------------------------" << std::endl;

            for (unsigned int ii = 0; ii < inputNets.size(); ii++)
            {
                if (netArray[inputNets[ii] - 1].netValueString == netValue.D)
                {
                    netArray[inputNets[ii] - 1].netValueString = netValue.high;
                }
                else if (netArray[inputNets[ii] - 1].netValueString == netValue.Dbar)
                {
                    netArray[inputNets[ii] - 1].netValueString = netValue.low;
                }

                std::cout << netArray[inputNets[ii] - 1].netValueString;
                outputStream << netArray[inputNets[ii] - 1].netValueString;
            }
            outputStream << endl;
            std::cout << std::endl;

            std::cout << "SUCCESS" << std::endl << std::endl;

            cout << "Output Net Values" << endl;
            for (unsigned int ii = 0; ii < outputNets.size(); ii++)
            {
                cout << netArray[outputNets[ii] - 1].netValueString << " ";
            }
            std::cout << std::endl << "--------------------------------------------------------" << std::endl;

            return true;
        }
    }

    //FAILURE CHECK
    unsigned int nonDValues = 0;

    for (unsigned int ii = 0; ii < outputNets.size(); ii++)
    {
        if (netArray[outputNets[ii] - 1].netValueString == netValue.high || netArray[outputNets[ii] - 1].netValueString == netValue.low)
        {
            nonDValues++;
        }
    }

    if (nonDValues == outputNets.size())
    {
        std::cout << "FAILURE, ALL THE OUTPUT NETS HAVE A DETERMINISTIC VALUE" << std::endl;
        return false;
    }

    //OBJECTIVE AND BACKTRACING FUNCTION CALL
    std::vector<string> objVect, backTraceVect;

    objVect = Objective(faultSaVal, targetVal, netArray, currentStack, netValue);
    if (objVect[0] == "-1" && objVect[1] == "-1")
    {
        return false;
    }

    if (!forwardSimulate)
    {
        backTraceVect = BackTrace(objVect, netArray, currentStack, netValue);
    }
    else
    {
        backTraceVect = objVect;
    }

    //IMPLICATION FUNCTION CALL
    bool implyStatus;

    //imply(j,vj)
    implyStatus = Imply(backTraceVect, currentStack, netArray, inputNets, netValue, faultSaValOG, DFrontier);

    if (implyStatus && podem(netArray, currentStack, outputNets, inputNets, netValue, faultSaVal, targetVal, faultSaValOG, DFrontier, outputStream))
    {
        std::cout << "SUCCESS" << std::endl;
        return true;
    }

    //-----------------------------------------------------------------------------------------------------------------

    //imply(j,vj bar)
    //Make the circuit's previous state as the current state.
    for (unsigned int kk = 0; kk < netArray.size(); kk++)
    {
        netArray[kk].netValueString = previousCircuitState[kk];
    }

    if (backTraceVect[1] == netValue.low)
        backTraceVect[1] = netValue.high;
    else
        backTraceVect[1] = netValue.low;

    implyStatus = Imply(backTraceVect, currentStack, netArray, inputNets, netValue, faultSaValOG, DFrontier);

    if (implyStatus && podem(netArray, currentStack, outputNets, inputNets, netValue, faultSaVal, targetVal, faultSaValOG, DFrontier, outputStream))
    {
        std::cout << "SUCCESS" << std::endl;
        return true;
    }

    //-----------------------------------------------------------------------------------------------------------------

    //imply(j,x)
    //Make the circuit's previous state as the current state.
    for (unsigned int kk = 0; kk < netArray.size(); kk++)
    {
        netArray[kk].netValueString = previousCircuitState[kk];
    }

    backTraceVect[1] = netValue.dontCare;

    implyStatus = Imply(backTraceVect, currentStack, netArray, inputNets, netValue, faultSaValOG, DFrontier);

    return false;
}

int main(int argc, char* argv[])
{

    //freopen("Results/CommandLineOutputs.txt","w",stdout);

    if (argc == 2 && (strcmp(argv[1], "--h") || strcmp(argv[1], "--help")))
    {
        std::cout << std::endl;
        std::cout << "----------------------------------------------------" << std::endl;
        std::cout << "PODEM (Path Oriented DEcision Making) Test Generator" << std::endl;
        std::cout << "----------------------------------------------------" << std::endl;
        std::cout << std::endl;
        std::cout << "  ----------- Usage ----------  " << std::endl;
        std::cout << "./a.exe (or) ./a.out -netlist <value1> -faultyFile <value2> -randomFaultMode <value3> -testVectorsLoc <value4>" << std::endl;
        std::cout << "./a.exe                           : Executable file" << std::endl;
        std::cout << "-netlist <value1>                 : A flag that enables parsing of netlist file in .txt format specified in place of value1, such as s27.txt" << std::endl;
        std::cout << "-faultyFile <value2>              : A flag that specifies the nets in which the faults must be injected" << std::endl;
        std::cout << "-randomFaultMode <value3>         : A flag that specifies whether randomly generated faults must be used for the test generation. The value must be either 0 or 1. 0 - Custom Faults, 1 - Random Faults." << std::endl;
        std::cout << "-testVectorsLoc <value4>          : A flag that specifies the location where test vectors generated by PODEM will be stored." << std::endl;
        std::cout << std::endl;
        return 1;
    }

    if (argc != 9)
    {
        std::cout << "Invalid Input Arguments!" << std::endl << std::endl;
        std::cout << "  ----------- Usage ----------  " << std::endl;
        std::cout << "./a.exe (or) ./a.out -netlist <value1> -faultyFile <value2> -randomFaultMode <value3> -testVectorsLoc <value4>" << std::endl;
        std::cout << "./a.exe                           : Executable file" << std::endl;
        std::cout << "-netlist <value1>                 : A flag that enables parsing of netlist file in .txt format specified in place of value1, such as s27.txt" << std::endl;
        std::cout << "-faultyFile <value2>              : A flag that specifies the nets in which the faults must be injected" << std::endl;
        std::cout << "-randomFaultMode <value3>         : A flag that specifies whether randomly generated faults must be used for the test generation. The value must be either 0 or 1. 0 - Custom Faults, 1 - Random Faults." << std::endl;
        std::cout << "-testVectorsLoc <value4>          : A flag that specifies the location where test vectors generated by PODEM will be stored." << std::endl;
        std::cout << std::endl;
        return 1;
    }

    //Mode of operation
    unsigned int mode;

    //File objects for netlist file and faults file.
    fstream netlistFile, faultsFile;

    //String that stores the location of both netlist file and faults file.
    string netlistFileLoc, faultsFileLoc, testVectorsLoc;


    if (argc > 1)
    {
        for (int ii = 1; ii < argc; ii++)
        {
            if (string(argv[ii]) == "-netlist" && ii + 1 < argc)
            {
                netlistFileLoc = argv[ii + 1];
            }
            else if (string(argv[ii]) == "-faultyFile" && ii + 1 < argc)
            {
                faultsFileLoc = argv[ii + 1];
            }
            else if (string(argv[ii]) == "-randomFaultMode" && ii + 1 < argc)
            {
                mode = stoi(argv[ii + 1]);
            }
            else if (string(argv[ii]) == "-testVectorsLoc" && ii + 1 < argc)
            {
                testVectorsLoc = argv[ii + 1];
            }
        }
    }

    //Initializing the stacks.
    std::vector<LogicGate> currentStack;

    //Maximum value of net in the given netlist.
    unsigned int nMaxNet = 0;

    //Declare a vector of
    std::vector<unsigned int> inputNets, outputNets;

    //Open the netlist file.
    netlistFile.open(netlistFileLoc, ios::in);

    //Each line of the netist file opened by the fstream object
    string line;

    unsigned int gateNumber = 0;

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
                        gateNumber++;
                        logicGateArray.gateNumber = gateNumber;
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
                        gateNumber++;
                        logicGateArray.gateNumber = gateNumber;
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
                        gateNumber++;
                        logicGateArray.gateNumber = gateNumber;
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
                        gateNumber++;
                        logicGateArray.gateNumber = gateNumber;
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
                        gateNumber++;
                        logicGateArray.gateNumber = gateNumber;
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
                        gateNumber++;
                        logicGateArray.gateNumber = gateNumber;
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
                        gateNumber++;
                        logicGateArray.gateNumber = gateNumber;
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
                        gateNumber++;
                        logicGateArray.gateNumber = gateNumber;
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
        cerr << "File not found!" << std::endl;
        return 1;
    }

    //Initialize all the nets in the circuit with Don't Care value.
    std::vector<AdvancedNet> netArray(nMaxNet);

    for (unsigned int ii = 0; ii < currentStack.size(); ii++)
    {
        netArray[currentStack[ii].edgeInputNetNum1 - 1].inputGateNum = currentStack[ii].gateNumber;
        netArray[currentStack[ii].edgeInputNetNum2 - 1].inputGateNum = currentStack[ii].gateNumber;
        netArray[currentStack[ii].edgeOutputNetNum - 1].outputGateNum = currentStack[ii].gateNumber;
    }

    netVal netValue;

    for (unsigned int ii = 0; ii < netArray.size(); ii++)
    {
        //ii+1 is net number
        for (unsigned int jj = 0; jj < currentStack.size(); jj++)
        {
            if (currentStack[jj].edgeInputNetNum1 == ii + 1)
            {
                netArray[ii].inputGateNum = currentStack[jj].gateNumber;
            }
        }
    }

    if (mode)
    {
        randomFaultGenerator(netArray.size(), netArray.size(), faultsFileLoc);
    }

    //Close the file to free the memory.
    netlistFile.close();

    for (int ii = 0; ii < nMaxNet; ii++)
    {
        netArray[ii].netValueString = netValue.dontCare;
        netArray[ii].netNumber = ii + 1;
    }

    //Open the netlist file.
    faultsFile.open(faultsFileLoc, ios::in);

    //FaultSaVal is the string containing each line's faulty net number and stuck-at fault.
    string faultSaVal;

    string netValueStr;

    //The vector that needs to be passed to PODEM() which will further be passed to Objective()
    std::vector<string> faultCurrentVal;

    ofstream testVectors("Results/testVectors.txt");

    if (faultsFile.is_open())
    {
        std::vector<string> v;

        while (std::getline(faultsFile, faultSaVal))
        {
            //Clear the vectors
            faultCurrentVal.clear();
            v.clear();

            //Initialize all the nets to don't care
            for (unsigned int ii = 0; ii < netArray.size(); ii++)
            {
                netArray[ii].netValueString = netValue.dontCare;
            }

            //Split the faultSaVal into vector of strings having delimiter ' '.
            v = splitString(faultSaVal);

            //Pack the faulty net number and the current value of the net number that needs to be passed to the PODEM for setting the Objective. Essentially, faultCurrentVal will have (faultyNetNumber, currentValue_FaultyNetNumber)
            netValueStr = netArray[stoi(v[0]) - 1].netValueString;
            faultCurrentVal.push_back(v[0]);
            faultCurrentVal.push_back(netValueStr);

            std::cout << "======================================================================================================" << std::endl;
            std::cout << "Fault : " << v[0] << "/" << v[1] << std::endl;

            podem(netArray, currentStack, outputNets, inputNets, netValue, faultCurrentVal, v[1], v, DFrontier, testVectors);

            std::cout << std::endl;
            DFrontier.clear();
        }
    }

    ofstream accurateTestVectors("./Deductive_FS/Results/accurateTestVectors.txt");

    fstream readTestVectors;
    readTestVectors.open(testVectorsLoc, ios::in);

    string testVector;

    parser(readTestVectors, accurateTestVectors);

    testVectors.close();
    faultsFile.close();
    accurateTestVectors.close();
    readTestVectors.close();

    return 0;
}