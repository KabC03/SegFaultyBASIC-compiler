//tinyBASIC compiler in C++
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <stack>
#include <fstream>
#include <cstdio>
#include <stdlib.h>
#include <time.h>


struct node {
    int value;  //Value contained in node
    node * next; //Pointer to next element
};
int stackLen(node * head) { //Print list length
    
    if(head == NULL) return INT_MIN;
    
    node * currentPtr = head;
    int length = 1;



    while (currentPtr->next != NULL) {
        currentPtr = currentPtr->next;
        length++;
    }

    return length;
}
bool stackPush(node ** head, int val) { //Linked list, push
    /*
     head - pointer to list head
     value - value to push on stack
     
     returns false on failure and true on success
     */

    node * newPtr = (node*)malloc(sizeof(node));
    if(newPtr == NULL || val == INT_MIN) return false;
    newPtr->value = val;


    newPtr->next = *head;
    *head = newPtr;
    
    return true;
}
int stackPop(node ** head) { //Linked list, pop
    /*
     head - pointer to list head
     value - value to push on stack
     
     returns INT_MIN on failure and value on success
     */
    
    if((*head) == NULL) return INT_MIN;
    
    int returnVal = (*head)->value;
    node * nodePtr = *head;



    *head = (*head)->next;
    free(nodePtr);
    
    return returnVal;
}
bool printStack(node ** head) { //Linked list, print stack
    /*
     head - pointer to lsit head
     
     returns false on failure and true on success
     */
    
    if(*head == NULL) {
        printf("NULL\n");
        return false;
    }
    
    node * currentPtr = (*head);
    
    while (currentPtr->next != NULL) {
        printf("%d - ",currentPtr->value);

        currentPtr = currentPtr->next;
    }
    printf("%d - NULL\n",currentPtr->value);
    
    return true;
}










typedef enum
{
    //Token type
    
    //Reserved
    RESERVED,
    //Variables and immediate
    USER_VAR,
    USER_IMM,
    //Other
    ASSIGNMENT, // '='
    OPERATOR, // +,-,*,/
    CONDITION, // >=, ==
    LABEL,
    
    EMPTY, // EMPTY ENUM
    TERM, // LAST TOKEN IN VECTOR
    
} type;

typedef struct
{
    //Token information
    type tokType;
    std::string tokStr;
    
} token;

typedef struct
{
    //Label information
    int jumpAddress;
    std::string name;
    
} label;

const int RAMsize = 10;
const int numRegisters = 6;

std::string destFile = "/Users/kabirchandra/Documents/C++/TinyBASIC compiler/TinyBASIC compiler/asm.txt";
//^^TEMPORARY

std::string formattedString;
std::vector<std::string> registerStates = {""}; //Holds current variables in register
int registerIndex = -1;


std::vector<std::string> varList = {""}; //Holds variables in RAM

std::stack<label> gotoJumpAddress; //Holds jump addresses for if/goto
node * condStack; //Keeps track of if statements

int stackDepth = 0;

std::vector<std::string> RESERVED_WORDS =
{
    //List of reserved words
    "IF",
    "END",
    "GOTO",
    "LABEL",
    "LET",
    "REM",
    "CLEAR",
};



bool initialiseVectors(int numRegisters, int RAMsize)
{
    //Initialises register and RAM vectors for indexing
    if(numRegisters <= 0 || RAMsize <= 0)
    {
        return false;
    }
    
    
    for (int i = 0; i < numRegisters; i++)
    {
        registerStates.push_back("");

    }
    for (int i = 0; i < RAMsize; i++)
    {
        varList.push_back("");
    }
    
    return true;
}


void appendToFile(const std::string data) 
{
    /*
     - Append to destination file
     */
    std::ofstream outFile(destFile, std::ios::app); 

    if (!outFile.is_open()) {

        printf("Cannot open file");
        exit(1);
    }

    outFile << data;
    outFile.close();  // Close the file
}


int memoryOp(std::string varName, char mode)
{
    // 'a' - access | 's' - storage | 'c' - clear | 'p' - print memory
    /*
     
     - Manages RAM indexing
     - Directly works on "varList"
     - Returns index for variable in RAM, -1 if not present
     
     
     
     */
    
    if(mode == 'a') //Access variable
    {
        for (int i = 0; i < RAMsize; i++) {
            
            if(varList[i] == varName)
            {
                return i;
            }
        }
        return -1; //Not in RAM
    }
    
    
    else if(mode == 's') //Store varaible
    {
        for (int i = 0; i < RAMsize; i++) {
            
            if(varList[i] == "")
            {
                varList[i] = varName;
                return i;
            }
        }
        printf("Out of memory\n");

        exit(1);
    }
    
    
    else if(mode == 'c') //Clear variable
    {
        for (int i = 0; i < RAMsize; i++) {
            
            if(varList[i] == varName)
            {
                varList[i] = "";
                return i;
            }
        }
        return -1; //Not in RAM
    }
    else if(mode == 'p') //Print variables
    {

        std::cout << "\n\n===== PRINTING RAM STATES =====" << std::endl;
        for (int i = 0; i < RAMsize; i++) {
            
            std::cout << i << " || '" << varList[i] << "'" << std::endl;
        }
        return -1; //Not in RAM
    }
    else
    {
        printf("Called memoryOp with invalid argument\n");
        exit(1);
    }
    
    return -1;
}


int regOp(std::string variable, char mode)
{
    // 'a' - access || 's' - store || 't' - temp store || 'c' - clear || 'p' - print
    /*

     - Handles current values in register
     - Keeps track of current variables in registers
     - Returns register number of variable if present, -1 if not present
     */
    
    if(mode == 'a')
    {
        for (int i = 0; i < registerStates.size(); i++) {
            if(variable == registerStates[i])
                return i;
        }
        
        return -1;
        
    }
    else if(mode == 's')
    {
        registerIndex = (registerIndex + 1) % numRegisters;
        registerStates[registerIndex] = variable;
        
        return registerIndex;
    }
    else if(mode == 't') //EXPERIMENTAL
    {
        registerStates[(registerIndex + 1) % numRegisters] = variable;
        
        return registerIndex;
    }
    else if(mode == 'c')
    {
        for (int i = 0; i < registerStates.size(); i++) {
            if(variable == registerStates[i])
            {
                registerStates[i] = "";
                return i;
            }
        }
        
        return -1;
    }
    
    
    else if(mode == 'p')
    {
        std::cout << "\n\n===== PRINTING REGISTER STATES =====" << std::endl;
        for (int i = 0; i < registerStates.size() - 1; i++) {
            std::cout << i << " || '" << registerStates[i] << "'\n";
        }
        
        return -1;
    }
    else
    {
        return -2;
    }
    
    return -1;
}


std::vector<token> tokenise(std::string line) //INITIALISE VECTOR TO ""
{
    /*
     - Tokenises 'line' and returns vector of struct tokens
     - Final token in vector contains "Empty" tokType

     */
    std::vector<token> tokens;
    token newToken;
    newToken.tokType = EMPTY;
    newToken.tokStr = "";
    
    std::stringstream check(line);
    std::string intermediate;
    
    while(getline(check, intermediate, ' '))
    {
        //if(intermediate == "" || intermediate == " ") continue; //EXPERIMENTAL

        newToken.tokType = EMPTY;
        newToken.tokStr = "";
        
        //Single character tokens
        

        
        switch (intermediate[0]) {

            case '!': //EXPERIMENTAL:

                switch (intermediate[1]) {
                    case '=':
                        newToken.tokType = CONDITION;
                        break;
                    default:
                        std::cout << "Unrecognised token: " << intermediate << std::endl;
                        exit(1);
                }

            case '=':
                
                switch (intermediate[1]) {
                    case '=':

                        newToken.tokType = CONDITION;
                        break;
                    default:
                        newToken.tokType = ASSIGNMENT;
                }
                
                goto END;
                break;
                
            case '+':
            case '-':
            case '*':
            case '/':

                newToken.tokType = OPERATOR;
                
                goto END;
                break;
                
            case '>':
            case '<':
                switch (intermediate[1]) {
                    case '=':
                        newToken.tokType = CONDITION;
                        break;
                        
                    default:
                        newToken.tokType = CONDITION;;
                }
                
                goto END;
                break;
                
            default:
                break;
        }
        
        
        
        //Multi line tokens
        for (int i = 0; i < RESERVED_WORDS.size(); i++) {
            
            if(intermediate == RESERVED_WORDS[i])
            {
                newToken.tokType = RESERVED; //RESERVED WORD
                goto END;
            }
        }
        for (int i = 0; i < intermediate.size(); i++) {
                        
            if(isalpha(intermediate[i]) == false && isdigit(intermediate[i]) == false)
            {
                newToken.tokType = EMPTY;
                goto END; //INVALID INPUT
            }
            else if(isdigit(intermediate[i]) == false)
            {
                newToken.tokType = USER_VAR;
                goto END; //NOT IMMEDIATE
                
            }

        }
        newToken.tokType = USER_IMM;

        
        
        
        END:
        if(newToken.tokType == EMPTY)
        {
            std::cout << "Unrecognised token: " << intermediate << std::endl;
            exit(1);
        }
        newToken.tokStr = intermediate;
        
        tokens.push_back(newToken);
    }
    
    
    newToken.tokStr = "";
    newToken.tokType = EMPTY;
    tokens.push_back(newToken);
    return tokens;
}


int doArithmatic(std::vector<token> tokens) //*
{
    /*
     - Handles arithmatic expressions
     - e.g var1 + 30 / 2
     - Directly writes to output file and returns void
     - Returns where doArithmatic fails
     */
    
    
    //EXPERIMENTAL LINE
    if(tokens.size() <= 1 || (tokens[0].tokType != USER_VAR && tokens[0].tokType != USER_IMM)) //EXPERIMENTAL
    {
        printf("Expected expression\n");
        exit(1);
    }
    //EXPERIMENTAL LINE
    
    

    
    
    
    char prevOperator = '+';
    int i = 0;
    while(tokens[i].tokType == USER_VAR
          || tokens[i].tokType == USER_IMM
          || tokens[i].tokType == OPERATOR) {
        
        /*
        if(i == 1)
        {
            //Set C2 = 0
                //printf("Set C2 = 0\n");
            formattedString = "SET    0    C2\n";
            appendToFile(formattedString);
        }
        */
        
        
        if(i % 2 == 0) //Expect variable or immediate
        {
            if(tokens[i].tokType != USER_VAR && tokens[i].tokType != USER_IMM)
            {
                printf("Expected value\n");
                exit(1);
            }
            else
            {
                if(tokens[i].tokType == USER_VAR)
                {
                    int regIndex = regOp(tokens[i].tokStr, 'a');
                    
                    if(regIndex < 0) //Not in register
                    {
                        //Get from RAM
                        int memIndex = memoryOp(tokens[i].tokStr, 'a');
                        
                        if(memIndex < 0) //Not in RAM
                        {
                            printf("Unrecognised variable\n");
                            exit(1);
                        }
                        else
                        {
                            if(i == 0)
                            {
                                //LOAD RAM[memIndex] -> C2 ||
                                    //printf("LOAD RAM[%d] -> C2",memIndex);

                                formattedString = "LOD    " + std::to_string(memIndex) + "    C1\n";
                                appendToFile(formattedString);
                                
                                //std::cout << "VAR(" << tokens[i].tokStr << ") Loading RAM[" << memIndex << "] -> C2" << std::endl;

                                regIndex = regOp(tokens[i].tokStr, 's');
                                
                                
                                //MOV C2 -> regIndex ||
                                    //printf("MOV C2 -> %d\n",regIndex);
                                formattedString = "MOV    C1    R" + std::to_string(regIndex) + "\n";
                                appendToFile(formattedString);
                            }
                            else
                            {
                                //LOAD RAM[memIndex] -> C2 ||
                                    //printf("LOAD RAM[%d] -> C2",memIndex);
                                formattedString = "LD    " + std::to_string(memIndex) + "    C2\n";
                                appendToFile(formattedString);
                                
                                //std::cout << "VAR(" << tokens[i].tokStr << ") Loading RAM[" << memIndex << "] -> C2" << std::endl;

                                regIndex = regOp(tokens[i].tokStr, 's');
                                
                                
                                //MOV C2 -> regIndex ||
                                    //printf("MOV C2 -> %d\n",regIndex);
                                formattedString = "MOV    C1    R" + std::to_string(regIndex) + "\n";
                                appendToFile(formattedString);
                            }

                        }
                        
                        
                    }
                    else
                    {
                        if(i == 0)
                        {
                            //MOV register[regIndex] -> C2 ||
                                //printf("Loading REGISTER[%d] -> C2\n",registerIndex);
                            formattedString = "MOV    R" + std::to_string(regIndex) + "    C1\n";
                            appendToFile(formattedString);
                        }
                        else
                        {
                            //MOV register[regIndex] -> C2 ||
                                //printf("Loading REGISTER[%d] -> C2\n",registerIndex);
                            formattedString = "MOV    R" + std::to_string(regIndex) + "    C2\n";
                            appendToFile(formattedString);
                            
                        }
                    }
                    
                }
                else
                {
                    if(i == 0)
                    {
                        //MOV immediate to C2 ||
                            //std::cout << "MOV IMM(" << tokens[i].tokStr << ") -> C2" << std::endl;
                        
                        formattedString = "SET    " + tokens[i].tokStr + "    C1\n";
                        appendToFile(formattedString);
                    }
                    else
                    {
                        //MOV immediate to C2 ||
                            //std::cout << "MOV IMM(" << tokens[i].tokStr << ") -> C2" << std::endl;
                        
                        formattedString = "SET    " + tokens[i].tokStr + "    C2\n";
                        appendToFile(formattedString);
                    }
                    
                    


                }
            }
        }
        else //Expect operator
        {
            
            if(tokens[i].tokType != OPERATOR)
            {
                printf("Expected operator\n");
                exit(1);
            }
            else
            {
                if(i > 2)
                {
                    switch (prevOperator) {
                        case '+':
                            //ADD C1 and C2 -> C1 ||
                                //printf("ADD C1 C2 -> C1\n");

                            formattedString = "ADD    C1    C2    C1\n";
                            appendToFile(formattedString);
                            break;
                        case '-':
                            //SUB C1 and C2 -> C1 ||
                                //printf("SUB C1 C2 -> C1\n");

                            formattedString = "SUB    C1    C2    C1\n";
                            appendToFile(formattedString);
                            break;
                        case '*':
                            //MUL C1 and C2 -> C1 ||
                                //printf("MUL C1 C2 -> C1\n");

                            formattedString = "MUL    C1    C2    C1\n";
                            appendToFile(formattedString);
                            break;
                        case '/':
                            //DIV C1 and C2 -> C1 ||
                                //printf("DIV C1 C2 -> C1\n");

                            formattedString = "DIV    C1    C2    C1\n";
                            appendToFile(formattedString);
                            break;
                            
                        default:
                            printf("Arithmatic error\n");
                            exit(1);
                    }
                    prevOperator =tokens[i].tokStr[0];
                }

            }
        }
        i++;
    }
    
    
    
    if(i > 1)
    {
        switch (prevOperator) {
            case '+':
                //ADD C1 and C2 -> C1 ||
                    //printf("ADD C1 C2 -> C1\n");

                formattedString = "ADD    C1    C2    C1\n";
                appendToFile(formattedString);
                break;
            case '-':
                //SUB C1 and C2 -> C1 ||
                    //printf("SUB C1 C2 -> C1\n");

                formattedString = "SUB    C1    C2    C1\n";
                appendToFile(formattedString);
                break;
            case '*':
                //MUL C1 and C2 -> C1 ||
                    //printf("MUL C1 C2 -> C1\n");

                formattedString = "MUL    C1    C2    C1\n";
                appendToFile(formattedString);
                break;
            case '/':
                //DIV C1 and C2 -> C1 ||
                    //printf("DIV C1 C2 -> C1\n");

                formattedString = "DIV    C1    C2    C1\n";
                appendToFile(formattedString);
                break;
                
            default:

                printf("Arithmatic error\n");
                exit(1);
        }
    }
        
    return i;
}


void parse(std::vector<token> tokens) //*
{
    /*
     - Parses token vector and produces intermediate assembly
     - Does not return anything
     */
    
    
    
    
    if (tokens[0].tokStr == "IF")
    {
        /*
         Moves comparison result into registers then compares
         */

        if(tokens[1].tokType == EMPTY)
        {
            printf("Expected statement\n");
            exit(1);
        }
        
        
        std::vector<token> arithmaticTokens = {};
        std::copy(tokens.begin() + 1, tokens.end(), std::back_inserter(arithmaticTokens));
        
        //MOV C1 D1 ||
            //printf("MOV C2 D1\n");
        
        //First argument

        int returnIndex = doArithmatic(arithmaticTokens);
        


        int regIndex1 = regOp("ARG1", 't'); //CHANGED FROM 's'

        /* ^^ One lines changed FROM
        int regIndex2 = regOp("ARG2", 's');
        */


        formattedString = "MOV    C1    R" + std::to_string(regIndex1) + "\n";
        appendToFile(formattedString);
        
        //Second argument
        arithmaticTokens = {};
        std::copy(tokens.begin() + returnIndex + 2, tokens.end(), std::back_inserter(arithmaticTokens));
        
        doArithmatic(arithmaticTokens);
        
        //MOV C2 D2 ||
            //printf("MOV C2 D2\n");




        registerIndex++; //EXPERIMENTAL
        int regIndex2 = regOp("ARG2", 't'); //CHANGED FROM 's'

        /* ^^ Two lines changed FROM
        int regIndex2 = regOp("ARG2", 's');
        */

        formattedString = "MOV    C1    R" + std::to_string(regIndex2) + "\n";
        appendToFile(formattedString);
        

        registerIndex--; //EXPERIMENTAL


        //Do comparison
        arithmaticTokens = {};
        std::copy(tokens.begin() + returnIndex + 1, tokens.end(), std::back_inserter(arithmaticTokens));
        
        std::string condOperator = arithmaticTokens[0].tokStr;
        
        if(condOperator == "==")
        {
            formattedString = "NEQ    R" + std::to_string(regIndex1) + "    R" + std::to_string(regIndex2) + "    "; //Not equal
            appendToFile(formattedString);
        }
        else if(condOperator == "!=")
        {
            formattedString = "EQA    R" + std::to_string(regIndex1) + "    R" + std::to_string(regIndex2) + "    ";
            appendToFile(formattedString);
        }
        else if(condOperator == ">")
        {
            formattedString = "LTE    R" + std::to_string(regIndex1) + "    R" + std::to_string(regIndex2) + "    ";
            appendToFile(formattedString);
        }
        else if(condOperator == ">=")
        {
            formattedString = "LES    R" + std::to_string(regIndex1) + "    R" + std::to_string(regIndex2) + "    ";
            appendToFile(formattedString);
        }
        else if(condOperator == "<")
        {
            formattedString = "GTE    R" + std::to_string(regIndex1) + "    R" + std::to_string(regIndex2) + "    ";
            appendToFile(formattedString);
        }
        else if(condOperator == "<=")
        {
            formattedString = "GRT    R" + std::to_string(regIndex1) + "    R" + std::to_string(regIndex2) + "    ";
            appendToFile(formattedString);
        }
        else
        {
            printf("Comparitor error\n");
            exit(1);
        }
        
        //Jump statement
        stackPush(&condStack, stackDepth);
        formattedString = "L" + std::to_string(stackDepth) + "\n";
        stackDepth++;

        appendToFile(formattedString);
        
        
        
    }
    
    else if (tokens[0].tokStr == "END")
    {
        if(stackLen(condStack) == INT_MIN) //Empty stack
        {
            printf("Unexpected END\n");
            exit(1);
        }
        else
        {
            formattedString = "LAB    L" + std::to_string(stackPop(&condStack)) + "\n";
            appendToFile(formattedString);
            stackDepth--;
        }
    }
    else if (tokens[0].tokStr == "LET") //Declaration
    {
        if(tokens[1].tokType != USER_VAR
           && tokens[2].tokType != ASSIGNMENT)
        {
            printf("Unexpected token\n");
            exit(1);
        }
        else
        {
            
            std::vector<token> arithmaticTokens = {};
            std::copy(tokens.begin() + 3, tokens.end(), std::back_inserter(arithmaticTokens));
            
            doArithmatic(arithmaticTokens);
            
            int memIndex = memoryOp(tokens[1].tokStr, 'a');
            if(memIndex < 0) //Not in RAM
            {
                memIndex = memoryOp(tokens[1].tokStr, 's');
                
                
                //MOV C2 -> RAM[memIndex] ||
                formattedString = "STR    " + std::to_string(memIndex) + "    C1\n";
                appendToFile(formattedString);
                
                
            }
            else
            {
                //MOV C2 -> RAM[memIndex] ||
                formattedString = "STR    " + std::to_string(memIndex) + "    C1\n";
                appendToFile(formattedString);
                
            }
            int regIndex = regOp(tokens[1].tokStr, 'a');
            
            if(regIndex < 0) //Not in register
            {
                regIndex = regOp(tokens[1].tokStr, 's');
                //MOV C2 -> Register[regOpIndex]
                formattedString = "MOV    C1    R" + std::to_string(regIndex) + "\n";
                appendToFile(formattedString);
            }
            else
            {
                //MOV C2 -> Register[regOpIndex]
                formattedString = "MOV    C1    R" + std::to_string(regIndex) + "\n";
                appendToFile(formattedString);
            }

            
        }
    }
    else if (tokens[0].tokStr == "CLEAR") //Clear variable
    {
        regOp(tokens[1].tokStr, 'c'); //Clear from register
        if(memoryOp(tokens[1].tokStr, 'c') == -1)
        {
            printf("Unknown variable\n");
            exit(1);
        }
        
    }
    else if (tokens[0].tokStr == "GOTO") //Unconditional jump
    {
        
        if(tokens[1].tokType != USER_VAR)
        {
            printf("Expected label\n");
            exit(1);
        }
        
        formattedString = "JMP    " + tokens[1].tokStr + "\n";
        appendToFile(formattedString);
        
    }
    else if (tokens[0].tokStr == "LAB") //Unconditional jump
    {
        if(tokens[1].tokType != USER_VAR)
        {
            printf("Expected label\n");
            exit(1);
        }
        
        formattedString = "LAB    " + tokens[1].tokStr + "\n"; //Label
        appendToFile(formattedString);
        
    }
    
    else if (tokens[0].tokStr == "REM") //Comment
    {
        formattedString = "# ";
        
        for (int i = 1; i < tokens.size(); i++) {
            formattedString = formattedString + " " + tokens[i].tokStr;
        }

        formattedString = formattedString + "\n";
        appendToFile(formattedString);

    }
    
    
    else //Invalid token
    {
        printf("Invalid token\n");
        exit(1);
    }

    
    
    
    return;
}




int main(int argc, char * argv[]) 
{
    clock_t start_time = clock();

    initialiseVectors(numRegisters, RAMsize);
    

    std::vector<std::string> commandArgs(argv, argv + argc);
    
    
    
    // 0 = Source file
    // 1 = Destination file
    
    if(argc != 3)
    {
        printf("Expected 2 arguments\n");
        printf("1 - source | 2 - destination\n");
        exit(1);
    }


    //std::string dest = commandArgs[1];
    //std::string source = commandArgs[2];
    
    std::cout << commandArgs[0] << " | " << commandArgs[2] << " | " << commandArgs[1] << std::endl;


    std::string dest = commandArgs[2];
    destFile = dest;
    std::string source = commandArgs[1];


    
    


    std::ifstream inputFile(source);

    if (!inputFile.is_open()) 
    {
        printf("Could not open source file\n");
        exit(1);
    }
    if (inputFile.good()) 
    {
        
    } else {
        printf("Could not open source file\n");
        exit(1);
    }


    std::ofstream outputFile(dest, std::ios::trunc); //Clear contents

    if (!outputFile.is_open()) {
        printf("Could not open destination file\n");
        exit(1);
    }
    outputFile.close(); // Close file


    std::string line;
    while (std::getline(inputFile, line)) {

        //std::cout << line << std::endl;
        
        if(line == "\n" || line == "") continue;
        
        std::vector<token> tokens = tokenise(line);
        
        
        parse(tokens);

    }


    regOp("", 'p');
    memoryOp("", 'p');
    inputFile.close(); // Close file

    clock_t end_time = clock();
    double elapsed_time = static_cast<double>(end_time - start_time) / CLOCKS_PER_SEC;
    printf("\n");

    std::cout << "Compile time: " << elapsed_time << "s\n";


    return 0;
}

