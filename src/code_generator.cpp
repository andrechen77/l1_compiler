#include <string>
#include <iostream>
#include <fstream>

#include <code_generator.h>

using namespace std;

namespace L1 {
	std::string mangle_name(const std::string &name) {
		return std::string("_") + name;
	}

	void write_start_fn(std::ostream &o, const std::string &entryPointLabel) {
		o <<
			".text\n"
			"	.globl go\n"
			"go:\n"
			"	pushq %rbx\n"
			"	pushq %rbp\n"
			"	pushq %r12\n"
			"	pushq %r13\n"
			"	pushq %r14\n"
			"	pushq %r15\n"
			"	call "
			<< mangle_name(entryPointLabel) << "\n"
			"	popq %r15\n"
			"	popq %r14\n"
			"	popq %r13\n"
			"	popq %r12\n"
			"	popq %rbp\n"
			"	popq %rbx\n"
			"	retq\n";
	}

	void write_instruction(std::ostream &o, Program &p, Function &f, Instruction &instruction) {
		o << instruction.to_x86(p, f);
	}

	void write_function(std::ostream &o, Program &p, Function &function) {
		// function name label
		o << mangle_name(function.name) << ":\n";

		// allocate space for local variables
		o << "\tsubq $" << (function.num_locals * 8) << ", %rsp\n";

		for (auto &instruction : function.instructions) {
			write_instruction(o, p, function, *instruction);
		}
	}

	void generate_code(Program &p){
		/*
		 * Open the output file.
		 */
		std::ofstream o;
		o.open("prog.S");

		/*
		 * Generate target code
		 */
		write_start_fn(o, p.entryPointLabel);
		for (auto &function : p.functions) {
			write_function(o, p, *function);
		}

		/*
		 * Close the output file.
		 */
		o.close();

		return;
	}
}
