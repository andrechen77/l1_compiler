#include "L1.h"
#include "code_generator.h"
#include <map>
#include <iostream>

namespace L1 {
	// Register methods

	std::map<std::string, RegisterID> strToRegId {
		{ "rax", RegisterID::rax },
		{ "rbx", RegisterID::rbx },
		{ "rcx", RegisterID::rcx },
		{ "rdx", RegisterID::rdx },
		{ "rdi", RegisterID::rdi },
		{ "rsi", RegisterID::rsi },
		{ "r8", RegisterID::r8 },
		{ "r9", RegisterID::r9 },
		{ "r10", RegisterID::r10 },
		{ "r11", RegisterID::r11 },
		{ "r12", RegisterID::r12 },
		{ "r13", RegisterID::r13 },
		{ "r14", RegisterID::r14 },
		{ "r15", RegisterID::r15 },
		{ "rbp", RegisterID::rbp },
		{ "rsp", RegisterID::rsp }
	};

	Register::Register(const std::string &id) : id {strToRegId[id]}, str {id} {}

	std::string Register::lowerBytesName() const {
		static const std::string lowerNames[] = {
			"al", "bl", "cl", "dl",
			"dil", "sil", "r8b", "r9b",
			"r10b", "r11b", "r12b", "r13b",
			"r14b", "r15b", "bpl"
		};
		return lowerNames[static_cast<int>(this->id)];
	}

	// LabelLocation

	LabelLocation::LabelLocation(const std::string &labelName) : labelName {labelName} {}

	// MemoryLocation

	MemoryLocation::MemoryLocation(const std::string &reg_id, int64_t offset) :
		reg {reg_id}, offset {offset}
	{};

	// AssignOperation

	std::map<std::string, AssignOperation> strToAssOp {
		{"<-", AssignOperation::pure},
		{"+=", AssignOperation::add},
		{"-=", AssignOperation::subtract},
		{"*=", AssignOperation::multiply},
		{"&=", AssignOperation::bitwise_and},
		{"<<=", AssignOperation::lshift},
		{">>=", AssignOperation::rshift}
	};
	AssignOperation toAssignOperation(const std::string &str) {
		return strToAssOp[str];
	}

	std::string assOpToStr(AssignOperation op) {
		static std::string arr[] = {"<-", "+=", "-=", "*=", "&=", "<<=", ">>="};
		return arr[static_cast<int>(op)];
	}

	// ComparisonOperator

	std::map<std::string, ComparisonOperator> strToCmpOp {
		{"<", ComparisonOperator::lt},
		{"<=", ComparisonOperator::le},
		{"=", ComparisonOperator::eq}
	};

	ComparisonOperator toComparisonOperator(const std::string &str) {
		return strToCmpOp[str];
	}

	bool executeComparisonOperator(ComparisonOperator op, int64_t lhs, int64_t rhs) {
		switch (op) {
			case ComparisonOperator::lt:
				return lhs < rhs;
			case ComparisonOperator::le:
				return lhs <= rhs;
			case ComparisonOperator::eq:
				return lhs == rhs;
			default:
				std::cerr << "not posdifasdfjeble" << std::endl;
				exit(1);
		}
	}

	std::string cmpOpToStr(ComparisonOperator op) {
		static std::string arr[] = {"<", "<=", "="};
		return arr[static_cast<int>(op)];
	}

	// Number methods

	Number::Number(int64_t value) : value {value} {}

	// std::string Number::toString() const {
	// 	return std::string("Number ") + std::to_string(this->value);
	// }

	// InstructionLabel

	InstructionLabel::InstructionLabel(const std::string &labelName) : label {std::make_unique<LabelLocation>(labelName)} {}

	std::string Register::toString() const {
		return this->str;
	}

	std::string MemoryLocation::toString() const {
		return this->reg.toString() + "[" + std::to_string(this->offset) + "]";
	}

	std::string Number::toString() const {
		return std::to_string(this->value);
	}

	std::string LabelLocation::toString() const {
		return this->labelName;
	}

	std::string InstructionLabel::toString() const {
		std::string result = "[label ";
		result += this->label->toString();
		return result + "]";
	}

	std::string InstructionReturn::toString() const {
		return "[return]";
	}

	std::string InstructionAssignment::toString() const {
		std::string result = "[assign ";
		result += this->destination->toString() + " ";
		result += assOpToStr(this->op) + " ";
		result += this->source->toString();
		return result + "]";
	}

	std::string InstructionCompareAssignment::toString() const {
		std::string result = "[compareassign ";
		result += this->destination->toString() + " <- ";
		result += this->lhs->toString();
		result += cmpOpToStr(this->op);
		result += this->rhs->toString();
		return result + "]";
	}

	std::string InstructionCompareJump::toString() const {
		std::string result = "[cjump ";
		result += this->lhs->toString() + " ";
		result += cmpOpToStr(this->op) + " ";
		result += this->rhs->toString() + " ";
		result += this->label->toString();
		return result + "]";
	}

	std::string InstructionGoto::toString() const {
		std::string result = "[goto ";
		result += this->label->toString();
		return result + "]";
	}

	std::string InstructionCallFunction::toString() const {
		std::string result = "[callfun ";
		result += this->functionName + " " + std::to_string(this->num_arguments);
		return result + "]";
	}

	std::string InstructionCallRegister::toString() const {
		std::string result = "[callreg ";
		result += this->reg->toString() + " " + std::to_string(this->num_arguments);
		return result + "]";
	}

	std::string InstructionLeaq::toString() const {
		std::string result = "[leaq ";
		result += this->regStore->toString() + " <- ";
		result += this->regRead->toString() + " + ";
		result += this->regOffset->toString() + " * ";
		result += std::to_string(this->scale);
		return result + "]";
	}

	std::string Function::toString() const {
		std::string result = "\t(";
		result += this->name;
		for (auto &instruction : this->instructions) {
			result += "\n\t\t";
			result += instruction->toString();
		}
		return result + "\n\t)";
	}

	std::string Program::toString() const {
		std::string result = "(";
		result += this->entryPointLabel;
		for (auto &function : this->functions) {
			result += "\n";
			result += function->toString();
		}
		return result + "\n)";
	}

	// to_x86 methods here

	std::string Instruction::to_x86(Program &p, Function &f) const {
		return "\t// -----\n";
	}

	std::string InstructionLabel::to_x86(Program &p, Function &f) const {
		return L1::mangle_name(this->label->labelName) + ":\n";
	}

	std::string InstructionReturn::to_x86(Program &p, Function &f) const {
		// find how much stack space to rewind for both the local variables
		// and the stack arguments
		// L1 convention is return addr, args, then locals
		auto num_stack_args = f.num_arguments - 6;
		if (num_stack_args < 0) {
			num_stack_args = 0;
		}

		auto num_bytes = 8 * (num_stack_args + f.num_locals);
		// don't +1 for the return address because that will be popped by `retq`

		return std::string("\taddq $") + std::to_string(num_bytes) + ", %rsp\n\tretq\n";
	}

	std::string InstructionAssignment::to_x86(Program &p, Function &f) const {
        static const std::string x86_keywords[] = {"movq", "addq", "subq", "mulq", "andq", "salq", "salq"};
        std::string result = "\t";
        std::string operator_str = x86_keywords[static_cast<int>(op)];
        std::string source = this->source->to_x86(p, f);
        std::string destination = this->destination->to_x86(p, f);

		// TODO figure out a better way to check
		if (this->op == AssignOperation::lshift || this->op == AssignOperation::rshift) {
			auto reg_ptr = dynamic_cast<Register *>(this->source.get());
			if (reg_ptr && reg_ptr->id == RegisterID::rcx) {
            	source = "%cl";
			}
		}
        result += operator_str + " " + source + ", " + destination + "\n";
        return result;
    }

    std::string Value::to_x86(Program &p, Function &f) const {
        return "I am a value YAY";
    }

    std::string Register::to_x86(Program &p, Function &f) const {
        std::string result = "%" + str;
        return result;
    }

    std::string MemoryLocation::to_x86(Program &p, Function &f) const {
        std::string result = std::to_string(offset);
        result += "(" + reg.to_x86(p, f) + ")";
        return result;
    }

    std::string Number::to_x86(Program &p, Function &f) const {
        return "$" + std::to_string(value);
    }

    std::string LabelLocation::to_x86(Program &p, Function &f) const {
        return "_" + labelName;
    }

	std::string InstructionCompareAssignment::to_x86(Program &p, Function &f) const {
		static const std::string x86_cmp_keywords[] = {"l", "le", "eq", "ge", "g"};

		Value *lhs = this->lhs.get();
		Value *rhs = this->rhs.get();

		Number *l_num_ptr = dynamic_cast<Number *>(this->lhs.get());
		Number *r_num_ptr = dynamic_cast<Number *>(this->rhs.get());

		if (l_num_ptr && r_num_ptr) {
			// both are constants
			int64_t lhs_value = l_num_ptr->value;
			int64_t rhs_value = r_num_ptr->value;
			bool result = executeComparisonOperator(this->op, lhs_value, rhs_value);
			return std::string("\tmovq $") + (result ? "1" : "0") + ", " + this->destination->to_x86(p, f) + "\n";
		}

		int swap = 1; // whether to use gt/ge operators instead of le/lt
		if (l_num_ptr) {
			// lhs is a constant so swap lhs and rhs so that lhs is non-constant
			auto temp = lhs;
			lhs = rhs;
			rhs = temp;
			swap = -1;
		}

		std::string lowerBits = dynamic_cast<Register *>(lhs)->lowerBytesName();
		std::string result = "\tcmpq ";
		result += rhs->to_x86(p, f) + ", " + lhs->to_x86(p, f) + "\n";
		result += "\tset" + x86_cmp_keywords[static_cast<int>(this->op) * swap + 2];
		result += " %" + lowerBits + "\n";
		result += "\tmovzbq %" + lowerBits + ", " + lhs->to_x86(p, f) + "\n";
		return result;
	}


	std::string InstructionGoto::to_x86(Program &p, Function &f) const {
		return std::string("\tjmp ") + L1::mangle_name(this->label->labelName) + "\n";
	}

	std::string InstructionCallFunction::to_x86(Program &p, Function &f) const {
		// TODO add argument-checking?
		if (this->isStd) {
			if (this->functionName == "tensor_error") {
				switch (this->num_arguments) {
					case 1:
						return "\tcall array_tensor_error_null\n";
					case 3:
						return "\tcall array_tensor\n";
					case 4:
						return "\tcall tensor_error\n";
					default:
						std::cerr << "Invalid number of tensor args... how did this get past the parser?\n";
						exit(1);
				}
			} else {
				return std::string("\tcall ") + this->functionName + "\n";
			}
		} else {
			auto numStackArgs = this->num_arguments - 6;
			if (numStackArgs < 0) {
				numStackArgs = 0;
			}
			auto numBytes = 8 * (numStackArgs + 1); // +1 to account for return addr
			return std::string("\tsubq $") + std::to_string(numBytes) + ", %rsp\n"
				"\tjmp " + L1::mangle_name(this->functionName) + "\n";
		}
	}
}
