//===----------------------------------------------------------------------===//
//  Author : Premsai Chinthamreddy                                           //
//===--------------------------------------------------------------------===//
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"
#include <cstdio>
using namespace llvm;

namespace {
	// Function pass to analyse every function in source code 
	
	struct functionAnalysis : public FunctionPass {
		static int functioncount ;
		static char ID;
		functionAnalysis() : FunctionPass(ID) {}
		
		bool runOnFunction(Function & F) override {

			// Declaring and Initializing variables for counting
			int instcount = 0;
			int mem_access_inst_count = 0;
			int arth_inst_count = 0;
			int branch_inst_count = 0;
			int mem_address_inst_count = 0;
			int bitwise_inst_count = 0;
			
			errs() << "Function Name: " << F.getName() << '\n';
			functioncount++;
			for (auto& B : F) {
				for(auto& I : B) {
					instcount++;
					std::string opN = I.getOpcodeName();
					if(isArthimeticInstruction(opN)) {
						arth_inst_count++;
					}else if (isMemoryAccessInstruction(opN)){
						mem_access_inst_count++;
					}else if (isBranchingInstruction(opN)) {
						branch_inst_count++;
					}else if (isMemoryAddressingInstruction(opN)) {
						mem_address_inst_count++;
					}else if (isBitwiseInstruction(opN)) {
						bitwise_inst_count++;
					}
				}
			}
			errs() << "------------------------------------\n";
			errs() << "| Instruction Type    |   Count     \n";
			errs() << "------------------------------------\n";
			errs() << "| Memory Access       | " << mem_access_inst_count <<"\n";
			errs() << "| Memory Addressing   | " << mem_address_inst_count <<"\n";
			errs() << "| Arthimetic          | " << arth_inst_count << "\n";
			errs() << "| Bitwise             | " << bitwise_inst_count <<"\n";
			errs() << "| Branching           | " << branch_inst_count <<"\n";
			errs() << "-------------------------------------\n";

			errs() << "Total instructions in this function: " << instcount << "\n\n";
			return false;
		}
		bool isArthimeticInstruction(std::string& opCodeName) {
			// Basic Athimetic Operators add, sub, mul 
			if(opCodeName == "add" || opCodeName =="sub" || opCodeName == "mul"){
				return true;
			// Floating point arthimetic operators fadd, fsub, fmul
			}else if(opCodeName == "fadd" || opCodeName == "fsub" || opCodeName == "fmul"){
				return true;
			// Unsigned integer division operations udiv, urem
			}else if(opCodeName == "udiv" || opCodeName == "urem"){
				return true;
			// Signed Integer division operations sdiv, srem
			}else if(opCodeName == "sdiv" || opCodeName == "srem") {
				return true;
			// Floating point division operations fdiv, frem
			}else if(opCodeName == "fdiv" || opCodeName == "frem") {
				return true;
			}else {
				return false;
			}
		}
		bool isMemoryAccessInstruction(std::string& opCodeName) {
			// Basic Read and Write instructions load, store
			if(opCodeName == "load" || opCodeName == "store") {
				return true;
			// atomic operations which access memory cmpxchg, atomicrmw
			}else if(opCodeName == "cmpxchg" || opCodeName == "atomicrmw"){
				return true;
			}else {
				return false;
			}
		}
		bool isMemoryAddressingInstruction(std::string& opCodeName) {
			// alloca - Allocates memory on stack and returns the pointer
			if(opCodeName == "alloca"){
				return true;
			//getelementptr - Calculates the address but does not acces memory
			}else if(opCodeName == "getelementptr"){
				return true;
			}else {
				return false;
			}
		}
		bool isBranchingInstruction(std::string& opCodeName) {
			// basic branching instructions br, indirectbr
			if(opCodeName == "br" || opCodeName == "indirectbr"){
				return true;
			// Switching instruction switch
			}else if(opCodeName == "switch"){
				return true;
			}else {
				return false;
			}
		}
		bool isBitwiseInstruction(std::string& opCodeName) {
			// Basic logical operators - and, or, xor
			if(opCodeName == "and" || opCodeName == "or" || opCodeName == "xor"){
				return true;
			// Shift operators - shl, lshr (logical shift right), ashr (arthimetic shift right)
			}else if(opCodeName == "shl" || opCodeName == "lshr" || opCodeName == "ashr"){
				return true;
			}else {
				return false;
			}
		}		


		
	};
}
char functionAnalysis::ID = 0;
int functionAnalysis::functioncount = 0;
static RegisterPass<functionAnalysis> A("functionanalysis", "Function analysis prints instruction count for different categories in a function");
