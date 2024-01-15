#pragma once

#include <memory>
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

		MemoryLocation(const std::string &reg_id, int64_t offset);
	};

	struct Number : Value {
		int64_t value;

		Number(int64_t value);
	};

	struct LabelLocation : Value {
		std::string labelName;

		LabelLocation(const std::string &labelName);
	};

	/*
	 * Instruction interface.
	 */
	struct Instruction {};

	struct InstructionLabel : Instruction {
		std::string name;
	};

	/*
	 * Instructions.
	 */
	struct InstructionReturn : Instruction {};

	// TODO rename to `AssignOperator` for consistency
	enum struct AssignOperation {
		pure,
		add,
		subtract,
		multiply,
		bitwise_and,
		lshift,
		rshift
	};

	AssignOperation toAssignOperation(const std::string &str);

	struct InstructionAssignment : Instruction {
		std::unique_ptr<Value> source;
		AssignOperation op;
		std::unique_ptr<Value> destination;
	};

	enum struct ComparisonOperator {
		lt,
		le,
		eq
	};

	ComparisonOperator toComparisonOperator(const std::string &str);

	struct InstructionCompareAssignment : Instruction {
		std::unique_ptr<Register> destination;
		ComparisonOperator op;
		std::unique_ptr<Value> lhs;
		std::unique_ptr<Value> rhs;
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
