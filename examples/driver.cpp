#include <cstring>
#include <iostream>
#include <fstream>
#include <string>
#include <ios>
#include <vector>
#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include <cassert>
#include "extended_fuzzer.cpp"

int main(int argc, char** argv)//argv, which is an array of pointers to strings representing command-line arguments
                                // argc, which is an integer representing the number of command-line arguments
{
    assert(argc == 2);
    std::string filename = argv[1]; //initialize a string variable named filename with the value of the first command-line argument

    // https://stackoverflow.com/questions/7880/how-do-you-open-a-file-in-c
    std::ifstream input(filename, std::ios::binary);
    //create an input file stream object named input and associates it with the file whose name is stored in the filename variable

    std::vector<uint8_t> bytes;// vector used to store the bytes read from the input file

    uint8_t byte;
    while (input >> byte) //reads bytes from the input file stream input
    // The loop continues until there are no more bytes to read from the file
    {
        bytes.push_back(byte);//each byte read from the file is appended to the bytes vector using the push_back method
    }

    uint8_t *array = &bytes[0];//array that points to the memory address of the first element of the bytes vector
    //BAG uses this
    size_t len = bytes.size();

    std::cout << "got " << len << " bytes" << std::endl;

    //char* filename_cstr = new char[filename.length() + 1];

    //strcpy(filename_cstr, filename.c_str());

    LLVMFuzzerTestOneInputByFile(filename.c_str());
}
