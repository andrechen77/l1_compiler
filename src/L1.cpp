#include "L1.h"
#include <map>

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

	// std::string Register::toString() const {
	// 	return std::string("Register ") + this->str;
	// }

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
}
