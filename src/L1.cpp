#include "L1.h"
#include <map>

namespace L1 {
	std::string Item::toString() const {
		return "(unimplemented)";
	}

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

	std::string Register::toString() const {
		return std::string("Register ") + this->str;
	}
}
