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

	struct Value {
		virtual std::string toString() const = 0;
	};

	struct Register : Value {
		RegisterID id;
		std::string str;

		Register(const std::string &id);

		virtual std::string toString() const override;
	};


	struct MemoryLocation : Value {
		Register reg;
		int64_t offset;

		MemoryLocation(const std::string &reg_id, int64_t offset);

		virtual std::string toString() const override;
	};

	struct Number : Value {
		int64_t value;

		Number(int64_t value);

		virtual std::string toString() const override;
	};

	struct LabelLocation : Value {
		std::string labelName;

		LabelLocation(const std::string &labelName);

		virtual std::string toString() const override;
	};

	struct Program;
	struct Function;

	/*
	 * Instruction interface.
	 */
	struct Instruction {
		virtual std::string toString() const = 0;
		virtual std::string to_x86(Program &p, Function &f) const;
	};

	struct InstructionLabel : Instruction {
		std::unique_ptr<LabelLocation> label;

		InstructionLabel(const std::string &labelName);
		virtual std::string toString() const override;
	};

	/*
	 * Instructions.
	 */
	struct InstructionReturn : Instruction {
		virtual std::string toString() const override;
		virtual std::string to_x86(Program &p, Function &f) const override;
	};

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

		virtual std::string toString() const override;
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

		virtual std::string toString() const override;
	};

	struct InstructionCompareJump : Instruction {
		ComparisonOperator op;
		std::unique_ptr<Value> lhs;
		std::unique_ptr<Value> rhs;
		std::unique_ptr<LabelLocation> label;

		virtual std::string toString() const override;
	};

	struct InstructionGoto : Instruction {
		std::unique_ptr<LabelLocation> label;

		virtual std::string toString() const override;
	};

	struct InstructionCallFunction : Instruction {
        std::string functionName;
        int64_t num_arguments;

		virtual std::string toString() const override;
    };

	struct InstructionCallRegister : Instruction {
        std::unique_ptr<Register> reg;
        int64_t num_arguments;

		virtual std::string toString() const override;
    };

	struct InstructionLeaq : Instruction {
        std::unique_ptr<Register> regStore;
        std::unique_ptr<Register> regRead;
        std::unique_ptr<Register> regOffset;
        int64_t scale;

		virtual std::string toString() const override;
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

		std::string toString() const;
	};

	struct Program {
		std::string entryPointLabel;
		std::vector<std::unique_ptr<Function>> functions;

		std::string toString() const;
	};
}
