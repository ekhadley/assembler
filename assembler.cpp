#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <regex>
#include <bitset>
#include <assert.h>
using namespace std;

int MEMORY_SIZE = 32767;

// used to remove comments from lines and ignore empty lines
bool ignorable(string line){
    line = regex_replace(line, regex("(//|#).*"), "");
    return line=="";
}

//regex matcher for named label definitions
bool isLabel(string line){
    return regex_match(line, regex("\\([a-zA-Z_][a-zA-Z0-9_]*?\\)"));
}

//regex matcher for addressing instructions
bool isAddress(string line){
    return regex_match(line, regex("@[0-9a-zA-Z_]*"));
}

// encodes address instructions.
// determines if instruction is named or explicit. If named, replace name with position of name's definition in program memory and encode to binary
string encodeAddressInstruction(string line, map<string, int> labels){
    string name = regex_replace(line, regex("@"), "");
    int addr;
    if(regex_match(name, regex("[0-9]*"))){
        addr = stoi(name);
    }
    else{
        if(labels.find(name)==labels.end()){
            std::cout << "attempt to access named label which has not been declared: " << line << endl;
            assert(0);
        }
        addr = labels[name];
        //std::cout << "label found: " << name << " associated line: " << addr << endl;
    }
    if(addr > MEMORY_SIZE){
        std::cout << "Memory address references exceeds memory size: " << line <<  endl;
        assert(0);
    }
    return "0" + bitset<15>(addr).to_string();
}
// spaghetti regex for matching any kind of compute instruction
bool isCompute(string line){
    return regex_match(line, regex("(([AMD]*?=)?(!?[AMD01(-1)])([\\+\\-\\&\\|]?[AMD01])?)(;J(GT|EQ|GE|LT|NE|LE|MP))?"));
}
//regex matcher for encoding destinations.
string encodeDest(string line){
    string destcode;
    if(!regex_match(line, regex("([AMD]*=)(.*)"))){destcode = "000";}
    else if(regex_match(line, regex("(M=)(.*)"))){destcode = "001";}
    else if(regex_match(line, regex("(D=)(.*)"))){destcode = "010";}
    else if(regex_match(line, regex("((MD)=)(.*)"))){destcode = "011";}
    else if(regex_match(line, regex("(A=)(.*)"))){destcode = "100";}
    else if(regex_match(line, regex("((AM)=)(.*)"))){destcode = "101";}
    else if(regex_match(line, regex("((AD)=)(.*)"))){destcode = "110";}
    else if(regex_match(line, regex("((AMD)=)(.*)"))){destcode = "111";}
    else{
        std::cout << "Unrecognized destination in detected compute instruction: " << line << endl;
        assert(0);
    }
    return destcode;
}
//regex matcher for generating opcodes( + bit 12 for deciding between A and M@A) (with extra cases for commutative operations)
string genOpcode(string line){
    string opcode;
    if(regex_match(line, regex("([AMD]*=)?(0)(.*)"))){opcode = "0101010";}
    else if(regex_match(line, regex("([AMD]*=)?(1)(.*)"))){opcode = "0111111";}
    else if(regex_match(line, regex("([AMD]*=)?(-1)(.*)"))){opcode = "0111010";}
    else if(regex_match(line, regex("([AMD]*=)?(D)(;.*)?"))){opcode = "0001100";}
    else if(regex_match(line, regex("([AMD]*=)?(A)(;.*)?"))){opcode = "0110000";}
    else if(regex_match(line, regex("([AMD]*=)?(M)(;.*)?"))){opcode = "1110000";}
    else if(regex_match(line, regex("([AMD]*=)?(!D)(;.*)?"))){opcode = "0001101";}
    else if(regex_match(line, regex("([AMD]*=)?(!A)(;.*)?"))){opcode = "0110001";}
    else if(regex_match(line, regex("([AMD]*=)?(!M)(;.*)?"))){opcode = "1110001";}
    else if(regex_match(line, regex("([AMD]*=)?(-D)(;.*)?"))){opcode = "0001111";}
    else if(regex_match(line, regex("([AMD]*=)?(-A)(;.*)?"))){opcode = "0110011";}
    else if(regex_match(line, regex("([AMD]*=)?(-M)(;.*)?"))){opcode = "1110011";}
    else if(regex_match(line, regex("([AMD]*=)?(D\\+1)(;.*)?"))){opcode = "0011111";}
    else if(regex_match(line, regex("([AMD]*=)?(1\\+D)(;.*)?"))){opcode = "0011111";}
    else if(regex_match(line, regex("([AMD]*=)?(A\\+1)(;.*)?"))){opcode = "0110111";}
    else if(regex_match(line, regex("([AMD]*=)?(1\\+A)(;.*)?"))){opcode = "0110111";}
    else if(regex_match(line, regex("([AMD]*=)?(M\\+1)(;.*)?"))){opcode = "1110111";}
    else if(regex_match(line, regex("([AMD]*=)?(1\\+M)(;.*)?"))){opcode = "1110111";}
    else if(regex_match(line, regex("([AMD]*=)?(D-1)(;.*)?"))){opcode = "0001110";}
    else if(regex_match(line, regex("([AMD]*=)?(A-1)(;.*)?"))){opcode = "0110010";}
    else if(regex_match(line, regex("([AMD]*=)?(M-1)(;.*)?"))){opcode = "1110010";}
    else if(regex_match(line, regex("([AMD]*=)?(D\\+A)(;.*)?"))){opcode = "0000010";}
    else if(regex_match(line, regex("([AMD]*=)?(A\\+D)(;.*)?"))){opcode = "0000010";}
    else if(regex_match(line, regex("([AMD]*=)?(D\\+M)(;.*)?"))){opcode = "1000010";}
    else if(regex_match(line, regex("([AMD]*=)?(M\\+D)(;.*)?"))){opcode = "1000010";}
    else if(regex_match(line, regex("([AMD]*=)?(D-A)(;.*)?"))){opcode = "0010011";}
    else if(regex_match(line, regex("([AMD]*=)?(D-M)(;.*)?"))){opcode = "1010011";}
    else if(regex_match(line, regex("([AMD]*=)?(A-D)(;.*)?"))){opcode = "0000111";}
    else if(regex_match(line, regex("([AMD]*=)?(M-D)(;.*)?"))){opcode = "1000111";}
    else if(regex_match(line, regex("([AMD]*=)?(D&A)(;.*)?"))){opcode = "0000000";}
    else if(regex_match(line, regex("([AMD]*=)?(A&D)(;.*)?"))){opcode = "0000000";}
    else if(regex_match(line, regex("([AMD]*=)?(D&M)(;.*)?"))){opcode = "1000000";}
    else if(regex_match(line, regex("([AMD]*=)?(M&D)(;.*)?"))){opcode = "1000000";}
    else if(regex_match(line, regex("([AMD]*=)?(D\\|A)(;.*)?"))){opcode = "0010101";}
    else if(regex_match(line, regex("([AMD]*=)?(A\\|D)(;.*)?"))){opcode = "0010101";}
    else if(regex_match(line, regex("([AMD]*=)?(D\\|M)(;.*)?"))){opcode = "1010101";}
    else if(regex_match(line, regex("([AMD]*=)?(M\\|D)(;.*)?"))){opcode = "1010101";}
    else{
        std::cout << "Unrecognized operation in detected compute instruction: " << line << endl;
        assert(0);
    }
    return opcode;
}
string encodeJump(string line){
    string jumpcode;
    
    if(!regex_match(line, regex("(.+)(;.+)"))){jumpcode = "000";}
    else if(regex_match(line, regex("(.+)(;JGT)"))){jumpcode = "001";}
    else if(regex_match(line, regex("(.+)(;JEQ)"))){jumpcode = "010";}
    else if(regex_match(line, regex("(.+)(;JGE)"))){jumpcode = "011";}
    else if(regex_match(line, regex("(.+)(;JLT)"))){jumpcode = "100";}
    else if(regex_match(line, regex("(.+)(;JNE)"))){jumpcode = "101";}
    else if(regex_match(line, regex("(.+)(;JLE)"))){jumpcode = "110";}
    else if(regex_match(line, regex("(.+)(;JMP)"))){jumpcode = "111";}
    else{
        std::cout << "Unrecognized jumpcode in detected compute instruction: " << line << endl;
        assert(0);
    }
    return jumpcode;
}
// concatenate the portions of the compute isntruction
string encodeComputeInstruction(string line){
    return "111" + genOpcode(line) + encodeDest(line) + encodeJump(line);
}

//change all characters to uppercase, remove whitespace and all text from comment symbol to end of line
void cleanLine(string& line){
    for(char& c: line){c=toupper(c);}
    line = regex_replace(line, regex("\\s+"), "");
    line = regex_replace(line, regex("(//|#).*"), "");
}
string b2hex(string instr){
    string hex;
    map<string, string> d = {
        {"0000", "0"},
        {"0001", "1"},
        {"0010", "2"},
        {"0011", "3"},
        {"0100", "4"},
        {"0101", "5"},
        {"0110", "6"},
        {"0111", "7"},
        {"1000", "8"},
        {"1001", "9"},
        {"1010", "A"},
        {"1011", "B"},
        {"1100", "C"},
        {"1101", "D"},
        {"1110", "E"},
        {"1111", "F"}};

    for(int i=0; i < 4; i++){
        string sub = instr.substr(i*4, 4);
        hex += d[sub];
    }
    return hex;
}


int main(int argc, char *argv[]){
    string name = argv[1];
    string bname = regex_replace(name, regex("\\.asm"), ".bin");
    string hname = regex_replace(name, regex("\\.asm"), ".txt");
    string local = "C:\\Users\\ek\\Desktop\\sdfghj\\assembler\\";
    ifstream input(local + name);
    ofstream binout(local + bname);
    ofstream hexout(local + hname);
    string line;
    map<string, int> labels;
    int lineNum = 0;

    std::cout << "mapping labels. . ." << endl;
    while(getline(input, line)){
        if(!ignorable(line)){

            cleanLine(line);
            
            if(isLabel(line)){
                //std::cout << "LABEL INSTRUCTION: " << line << endl;
                string labelname = line.erase(line.length()-1, 1).erase(0, 1);
                if(labels.find(labelname)!=labels.end()){
                    std::cout << "illegal declaration of labelled address: already defined at location: " << labels[labelname] << endl << line << endl;  
                    assert(0);
                }
                labels.insert({labelname, lineNum});
            }
            else{lineNum++;}
        }
    }
    std::cout << "Labels mapped: " << labels.size() << endl << endl;

    input.clear();
    input.seekg(0);
    std::cout << "Translating instructions . . ." << endl;
    lineNum = 0;
    while(getline(input, line)){
        if(!ignorable(line)){
            
            cleanLine(line);
            
            string instr;
            if(isAddress(line)){
                //std::cout << "[" << lineNum << "] " << "ADDRESS INSTRUCTION: " << line << endl;
                instr = encodeAddressInstruction(line, labels);
                //std::cout << "binarized address instruction: " << instr << endl << endl;
                lineNum++;
                binout << instr << endl;
                hexout << b2hex(instr) << endl;
            }
            else if(isCompute(line)){
                //std::cout << "[" << lineNum << "] " << " COMPUTE INSTRUCTION: " << line << endl;
                instr = encodeComputeInstruction(line);
                //std::cout << "translated instruction: " << instr << endl << endl;
                lineNum++;
                binout << instr << endl;
                hexout << b2hex(instr) << endl;
            }
            else if(!isLabel(line)){
                std::cout << "Type of insrtuction unknown: " << line << endl;
                assert(0);
            }
        }
    }
    std::cout << "assembly completed: " << lineNum << " lines generated." << endl;
    std::cout << "binary instructions stored at " + local + bname << endl;
    std::cout << "hex instructions stored at " + local + hname << endl;

    binout.close();
    hexout.close();
    return 0;
}