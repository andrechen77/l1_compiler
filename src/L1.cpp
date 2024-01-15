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

	// ComparisonOperator

	std::map<std::string, ComparisonOperator> strToCmpOp {
		{"<", ComparisonOperator::lt},
		{"<=", ComparisonOperator::le},
		{"=", ComparisonOperator::eq}
	};

	ComparisonOperator toComparisonOperator(const std::string &str) {
		return strToCmpOp[str];
	}

	// Number methods

	Number::Number(int64_t value) : value {value} {}

	// std::string Number::toString() const {
	// 	return std::string("Number ") + std::to_string(this->value);
	// }

	// InstructionLabel

	InstructionLabel::InstructionLabel(const std::string &labelName) : label {std::make_unique<LabelLocation>(labelName)} {}

	// // Label methods

	// Label::Label(const std::string &name) : name {name} {}

	// std::string Label::toString() const {
	// 	return std::string("Label :") + this->name;
	// }

	// // FunctionName methods

	// FunctionName::FunctionName(const std::string &name) : name {name} {}

	// std::string FunctionName::toString() const {
	// 	return std::string("FunctionName @") + this->name;
	// }
}
