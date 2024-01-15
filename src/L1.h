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

	struct Value {};

	struct Register : Value {
		RegisterID id;
		std::string str;

		Register(const std::string &id);
	};


	struct MemoryLocation : Value {
		Register reg;
		int64_t offset;
	};

	struct Number : Value {
		int64_t value;
	};

	struct FunctionName  {
		std::string name;
	};

	/*
	 * Instruction interface.
	 */
	struct Instruction {};

	struct Label : Instruction {
		std::string name;
	};

	/*
	 * Instructions.
	 */
	struct InstructionReturn : Instruction {};

	struct InstructionAssignment : Instruction {
		std::unique_ptr<Value> source;
		std::unique_ptr<Value> destination;
	};

	/*
	 * Function.
	 */
	struct Function {
		std::string name;
		int64_t num_arguments;
		int64_t num_locals;
		// TODO consider changing to value type instead of ptr type
		std::vector<std::unique_ptr<Instruction>> instructions;
	};

	struct Program {
		std::string entryPointLabel;
		std::vector<std::unique_ptr<Function>> functions;
	};
}
