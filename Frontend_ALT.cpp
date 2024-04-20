#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <stack>
#include <fstream>
#include <cstdio>
#include <stdlib.h>
#include <time.h>
#include <map>


typedef enum {
    COND_JUMP,
    UNCOND_JUMP,
    LAB_DEFINE,
    LAB_CREA,
    ARITH,
    COMMENT,
    RAM,
    NUM,
    MOVE,
    SET,
    C_REGISTER,
    R_REGISTER,
    EMPTY,
} tokType;

typedef struct
{
    tokType type;
    std::string string;
}token;

std::map<tokType, int> effectiveLineNumberDict = {
    {COND_JUMP, 1},
    {UNCOND_JUMP, 1},
    {ARITH, 1},
    {RAM, 2},
    {MOVE, 1},
    {SET, 1},
    {LAB_DEFINE, 0},
};







int lineNum = 0;
std::vector<int> effectiveLineNumber;
std::string asmFileName = "asm.txt";
std::string macFileName = "MachineCode.txt";
std::map<std::string, int> labelDict;
std::string formattedString = "";


void appendToFile(const std::string data) 
{
    std::ofstream outFile(macFileName, std::ios::app); 

    if (!outFile.is_open()) {

        printf("Cannot open file\n");
        exit(1);
    }

    outFile << data;
    outFile.close();
}


std::string getRegisterNumber(std::string regNum)
{
    int number = -1;
    int regNumber = 0;
    try
    {
        regNumber = std::stoi(regNum.substr(1));
    }
    catch(...)
    {
        printf("Unrecognised register\n");
        exit(1);
    }
    
    

    
    if(regNum[0] == 'R')
    {
        regNumber+=2;
    }
    else if(regNum[0] == 'C')
    {
        regNumber--;
    }
    else
    {
        printf("Unrecognised register\n");
        exit(1);
    }

    std::string regName = std::to_string(regNumber);
    
    return regName;
}




bool is_num(std::string string)
{

    for (int i = 0; i < string.size(); i++)
    {
        if(isdigit(string[i]) == false)
        {
            return false;
        }

    }
    
    return true;
}



void createEffectiveLineNum(void)
{

    std::ifstream inputFile(asmFileName);

    if (!inputFile.is_open()) {
        printf("Cannot open file\n");
        exit(1);
    }

    std::string line;
    effectiveLineNumber.push_back(0);
    while (std::getline(inputFile, line)) {

        line = line.substr(0, 3);

        

        if (line == "NEQ" || line == "EQA" || line == "LTE" ||
            line == "LES" || line == "GTE" || line == "GRT")
        {
            effectiveLineNumber.push_back(lineNum + effectiveLineNumberDict[COND_JUMP]);

        }

        else if (line == "JMP")
        {
            effectiveLineNumber.push_back(lineNum + effectiveLineNumberDict[UNCOND_JUMP]);
        }

        else if (line == "ADD" || line == "SUB" || line == "MUL" ||
                line == "DIV")
        {
            effectiveLineNumber.push_back(lineNum + effectiveLineNumberDict[ARITH]);
        }

        else if (line == "MOV")
        {
            effectiveLineNumber.push_back(lineNum + effectiveLineNumberDict[MOVE]);
        }

        else if (line == "SET")
        {
            effectiveLineNumber.push_back(lineNum + effectiveLineNumberDict[SET]);
        }

        else if (line == "STR" || line == "LOD")
        {
            effectiveLineNumber.push_back(lineNum + effectiveLineNumberDict[RAM]);
        }

        else if(line == "LAB")
        {
            effectiveLineNumber.push_back(lineNum + effectiveLineNumberDict[LAB_DEFINE]);
        }

        else if(line == "")
        {
        }

        else
        {
            std::cout << "Unrecognised: '" << line << "'" << std::endl;
            exit(1);
        }

        lineNum+=effectiveLineNumber.back() - lineNum;
        
        

    }

    inputFile.close();




    return;
}


void generateLabelAddresses(void)
{
    std::ifstream inputFile(asmFileName);

    if (!inputFile.is_open()) {
        printf("Cannot open file\n");
        exit(1);
    }

    std::string line;
    std::string labName;
    int index = 0;
    while (std::getline(inputFile, line)) {


        
        int ind;
        for (int i = 3; i < line.size(); i++)
        {
            if(line[i] != ' ')
            {
                ind = i;
                break;
            }

        }


        labName = line.substr(ind);

        line = line.substr(0, 3);
        

        

        if(line == "LAB")
        {
            labelDict[labName] = effectiveLineNumber[index];
        }

        index++;
    }



    return;
}


std::vector<token> tokenise(const std::string line) 
{
    std::vector<token> tokens;
    token newToken;

    std::istringstream iss(line);
    int count = 1;

    while (iss >> newToken.string) {

        
        if(count > 4)
        {
            printf("\n\nIncorrect format\n\n");
            exit(1);
        }
        count++;

        newToken.type = EMPTY;
        std::string substring = (newToken.string).substr(1);


        if(newToken.string == "NEQ" || newToken.string == "EQA" || newToken.string == "LTE" ||
        newToken.string == "LES" || newToken.string == "GTE" || newToken.string == "GRT")
        {
            newToken.type = COND_JUMP;
        }

        else if(newToken.string == "JMP")
        {
            newToken.type = UNCOND_JUMP;
        }

        else if(newToken.string == "ADD" || newToken.string == "SUB" || newToken.string == "MUL" ||
        newToken.string == "DIV")
        {
            newToken.type = ARITH;
        }

        else if(newToken.string == "MOV")
        {
            newToken.type = MOVE;
        }

        else if(newToken.string == "SET")
        {
            newToken.type = SET;
        }

        else if(newToken.string == "STR" || newToken.string == "LOD")
        {
            newToken.type = RAM;
        }

        else if(newToken.string[0] == 'C' && is_num(substring) == true)
        {
            newToken.type = C_REGISTER;
        }

        else if(newToken.string[0] == 'R' && is_num(substring) == true)
        {
            newToken.type = R_REGISTER;
        }

        else if(is_num(newToken.string) == true)
        {
            newToken.type = NUM;
        }

        else if(newToken.string == "#")
        {
            newToken.type = COMMENT;
        }

        else if(newToken.string == "LAB")
        {
            newToken.type = LAB_CREA;
        }

        else
        {
            newToken.type = LAB_DEFINE;
        }


        tokens.push_back(newToken);
    }



    return tokens;
}


void parse(std::vector<token> tokens)
{ 

    std::ofstream file(macFileName, std::ios::app);
    

    if (!file.is_open()) {
        printf("Cannot open file\n");
        exit(1);
    }



    //[INS] [SRC] [SRC] [DEST]
        //std::cout << "Current: " << tokens[i].string << std::endl;
    if(tokens[0].string == "LAB")
    {
        return;
    }


    else if(tokens[0].type == UNCOND_JUMP)
    {
        formattedString = "32    0    0    " + std::to_string(labelDict[tokens[1].string]) + "\n";
        appendToFile(formattedString);
    }

    else if(tokens[0].type == ARITH)
    {

        if(tokens[0].string == "ADD")
        {
            formattedString = "0    " + getRegisterNumber(tokens[1].string) + "    " 
            + getRegisterNumber(tokens[2].string) + "    " + getRegisterNumber(tokens[3].string) + "\n";
            appendToFile(formattedString);
        }
        else if(tokens[0].string == "SUB")
        {
            formattedString = "1    " + getRegisterNumber(tokens[1].string) + "    " 
            + getRegisterNumber(tokens[2].string) + "    " + getRegisterNumber(tokens[3].string) + "\n";
            appendToFile(formattedString);
        }
        else if(tokens[0].string == "MUL")
        {
            formattedString = "6    " + getRegisterNumber(tokens[1].string) + "    " 
            + getRegisterNumber(tokens[2].string) + "    " + getRegisterNumber(tokens[3].string) + "\n";
            appendToFile(formattedString); 
        }
        else if(tokens[0].string == "DIV")
        {
            formattedString = "9    " + getRegisterNumber(tokens[1].string) + "    " 
            + getRegisterNumber(tokens[2].string) + "    " + getRegisterNumber(tokens[3].string) + "\n";
            appendToFile(formattedString); 
        }
        else
        {
            printf("Parse error\n");
            exit(1);
        }


    }

    
    else if(tokens[0].type == COND_JUMP)
    {
        if(tokens[0].string == "NEQ")
        {
            formattedString = "33    " + getRegisterNumber(tokens[1].string) + "    " 
            + getRegisterNumber(tokens[2].string) + "    " + std::to_string(labelDict[tokens[3].string]) + "\n";
            appendToFile(formattedString);
        }
        else if(tokens[0].string == "EQA")
        {
            formattedString = "32    " + getRegisterNumber(tokens[1].string) + "    " 
            + getRegisterNumber(tokens[2].string) + "    " + std::to_string(labelDict[tokens[3].string]) + "\n";
            appendToFile(formattedString);
        }
        else if(tokens[0].string == "LTE")
        {
            formattedString = "35    " + getRegisterNumber(tokens[1].string) + "    " 
            + getRegisterNumber(tokens[2].string) + "    " + std::to_string(labelDict[tokens[3].string]) + "\n";
            appendToFile(formattedString); 
        }
        else if(tokens[0].string == "LES")
        {
            formattedString = "34    " + getRegisterNumber(tokens[1].string) + "    " 
            + getRegisterNumber(tokens[2].string) + "    " + std::to_string(labelDict[tokens[3].string]) + "\n";
            appendToFile(formattedString); 
        }
        else if(tokens[0].string == "GTE")
        {
            formattedString = "37    " + getRegisterNumber(tokens[1].string) + "    " 
            + getRegisterNumber(tokens[2].string) + "    " + std::to_string(labelDict[tokens[3].string]) + "\n";
            appendToFile(formattedString); 
        }
        else if(tokens[0].string == "GRT")
        {
            formattedString = "36    " + getRegisterNumber(tokens[1].string) + "    " 
            + getRegisterNumber(tokens[2].string) + "    " + std::to_string(labelDict[tokens[3].string]) + "\n";
            appendToFile(formattedString); 
        }
        else
        {
            printf("Parse error\n");
            exit(1);
        }
        

    }

    else if(tokens[0].type == MOVE)
    {
        formattedString = "7    " + getRegisterNumber(tokens[1].string) + "    0    " 
        + getRegisterNumber(tokens[2].string) + "\n";
        appendToFile(formattedString);
    }

    else if(tokens[0].type == SET)
    {
        formattedString = "135    " + tokens[1].string + "    0    "+  getRegisterNumber(tokens[2].string)  
        + "\n";
        appendToFile(formattedString);
    }

    else if(tokens[0].type == RAM)
    {
        formattedString = "135    " + tokens[1].string + "    0    5\n";
        appendToFile(formattedString);

        if(tokens[0].string == "LOD")
        {
            formattedString = "18    0    0    " + getRegisterNumber(tokens[2].string) + "\n";
            appendToFile(formattedString);
        }
        else if(tokens[0].string == "STR")
        {
            formattedString = "17    0    0    " + getRegisterNumber(tokens[2].string) + "\n";
            appendToFile(formattedString);
        }
        else
        {
            printf("Parse error RAM\n");
            exit(1);
        }

    }


    file.close();
    return;
}





int main(int argc, char * argv[]) 
{

    clock_t start_time = clock();
    //printLineNum();
    //printLabelDict();
    //printTokens(tokens);

    std::vector<std::string> commandArgs(argv, argv + argc);

    if(argc != 3)
    {
        printf("Expected 2 arguments\n");
        printf("1 - source | 2 - destination\n");
        exit(1);
    }


    std::cout << commandArgs[0] << " | " << commandArgs[2] << " | " << commandArgs[1] << std::endl;


    std::string dest = commandArgs[2];
    macFileName = dest;
    asmFileName = commandArgs[1];



    std::string fileName = macFileName;

    std::ofstream outputFile(fileName, std::ios::trunc);

    if (!outputFile.is_open()) {
        printf("Cannot open file\n");
        exit(1); 
    }

    outputFile.close();


    createEffectiveLineNum();
    generateLabelAddresses();

    std::ifstream inputFile(asmFileName);

    if (!inputFile.is_open()) {
        printf("Cannot open file\n");
        exit(1);
    }

    std::string line;
    while (std::getline(inputFile, line)) {

        std::vector<token> tokens = tokenise(line);



        parse(tokens);

    }
    inputFile.close();


    clock_t end_time = clock();
    double elapsed_time = static_cast<double>(end_time - start_time) / CLOCKS_PER_SEC;
    printf("\n");

    std::cout << "Compile time: " << elapsed_time << "s\n";

    return 0;
}
