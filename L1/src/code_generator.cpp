#include <string>
#include <iostream>
#include <fstream>

#include <code_generator.h>

using namespace std;

namespace L1{
  void generate_code(Program &p){

    /* 
     * Open the output file.
     */ 
    std::ofstream outputFile;
    outputFile.open("prog.S");
   
    /* 
     * Generate target code
     */ 
    outputFile << ".text\n";
    outputFile << "  .globl go\n";
    outputFile << "go:\n";
    outputFile << "  pushq %rbx\n";
    outputFile << "  pushq %rbp\n";
    outputFile << "  pushq %r12\n";
    outputFile << "  pushq %r13\n";
    outputFile << "  pushq %r14\n";
    outputFile << "  pushq %r15\n";

    std::string entryPoint = p.entryPointLabel;
    entryPoint = "_" + entryPoint.substr(1);
    outputFile << "  call " << entryPoint << "\n";

    outputFile << "  popq %r15\n";
    outputFile << "  popq %r14\n";
    outputFile << "  popq %r13\n";
    outputFile << "  popq %r12\n";
    outputFile << "  popq %rbp\n";
    outputFile << "  popq %rbx\n";
    outputFile << "  retq\n";

    Context c;

    for (auto f : p.functions) {
      std::string funcName = f->name;
      funcName = "_" + funcName.substr(1);
      outputFile << funcName << ":\n";
      if (f->locals != 0) {
        uint64_t offset = f->locals * 8;
        outputFile << "  subq $" << offset << ", %rsp\n";
      }

      c.args = f->arguments;
      c.locals = f->locals;

      for (auto i : f->instructions) {
        i->generate(outputFile, c);
      }
    }

    /* 
     * Close the output file.
     */ 
    outputFile.close();
   
    return ;
  }
}
