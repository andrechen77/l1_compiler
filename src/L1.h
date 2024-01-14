#pragma once

#include <vector>
#include <string>

namespace L1 {

	enum struct RegisterID {
		rax,
		rbx,
		rcx,
		rdx,
		rdi,
		rsi,
		r8,
		r9,
		r10,
		r11,
		r12,
		r13,
		r14,
		r15,
		rbp,
		rsp
	};

	// Every component of the AST is-a Item
	struct Item {};

	struct Register : Item {
		RegisterID ID;
	};

	/*
	 * Instruction interface.
	 */
	struct Instruction : Item {};

	/*
	 * Instructions.
	 */
	struct Instruction_ret : Instruction {};

	struct Instruction_assignment : Instruction {
		Item *source;
		Item *destination;
	};

	/*
	 * Function.
	 */
	struct Function : Item {
		std::string name;
		int64_t num_arguments;
		int64_t num_locals;
		// TODO consider changing to value type instead of ptr type
		std::vector<Instruction *> instructions;
	};

	struct FunctionName : Item {
		std::string name;
	};

	struct Program {
		std::string entryPointLabel;
		std::vector<Function *> functions;
	};
}
