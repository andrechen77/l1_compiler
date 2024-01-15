/*
 * SUGGESTION FROM THE CC TEAM:
 * double check the order of actions that are fired.
 * You can do this in (at least) two ways:
 * 1) by using gdb adding breakpoints to actions
 * 2) by adding printing statements in each action
 *
 * For 2), we suggest writing the code to make it straightforward to enable/disable all of them
 * (e.g., assuming shouldIPrint is a global variable
 *    if (shouldIPrint) std::cerr << "MY OUTPUT" << std::endl;
 * )
 */
#include <sched.h>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <set>
#include <iterator>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <stdint.h>
#include <assert.h>

#include <tao/pegtl.hpp>
#include <tao/pegtl/contrib/analyze.hpp>
#include <tao/pegtl/contrib/raw_string.hpp>
#include <tao/pegtl/contrib/parse_tree.hpp>
#include <tao/pegtl/contrib/parse_tree_to_dot.hpp>

#include <L1.h>
#include <parser.h>

namespace pegtl = TAO_PEGTL_NAMESPACE;

using namespace pegtl;

namespace L1 {
	template<typename Rule>
	struct with_lookahead : seq<at<Rule>, Rule> {};

	template<typename SpecificRule, typename GeneralRule>
	struct upcast : seq<at<SpecificRule>, GeneralRule> {};

	/*
	 * Grammar rules from now on.
	 */
	struct name : seq<
		// TODO consider removing the plus bc we don't need it
		plus<
			sor<
				alpha,
				one<'_'>
			>
		>,
		star<
			sor<
				alpha,
				one<'_'>,
				digit
			>
		>
	> {};

	struct number : sor<
		seq<
			opt<
				sor<
					one<'-'>,
					one<'+'>
				>
			>,
			range<'1', '9'>,
			star<
				digit
			>
		>,
		one<'0'>
	> {};

	struct argument_number : seq<number> {};

	struct local_number : seq<number> {};

	// "F" in the grammar
	struct tensor_error_arg_number : upcast<
		sor<
			one<'1'>,
			one<'3'>,
			one<'4'>
		>,
		number
	> {};

	// "E" in the grammar
	struct lea_factor : upcast<
		sor<
			one<'1'>,
			one<'2'>,
			one<'4'>,
			one<'8'>
		>,
		number
	> {};

	/*
	 * Keywords.
	 */
	struct str_return : TAO_PEGTL_STRING("return") {};
	struct str_arrow : TAO_PEGTL_STRING("\x3c-") {};
	struct str_rax : TAO_PEGTL_STRING("rax") {};
	struct str_rbx : TAO_PEGTL_STRING("rbx") {};
	struct str_rcx : TAO_PEGTL_STRING("rcx") {};
	struct str_rdx : TAO_PEGTL_STRING("rdx") {};
	struct str_rdi : TAO_PEGTL_STRING("rdi") {};
	struct str_rsi : TAO_PEGTL_STRING("rsi") {};
	struct str_r8 : TAO_PEGTL_STRING("r8") {};
	struct str_r9 : TAO_PEGTL_STRING("r9") {};
	struct str_r10 : TAO_PEGTL_STRING("r10") {};
	struct str_r11 : TAO_PEGTL_STRING("r11") {};
	struct str_r12 : TAO_PEGTL_STRING("r12") {};
	struct str_r13 : TAO_PEGTL_STRING("r13") {};
	struct str_r14 : TAO_PEGTL_STRING("r14") {};
	struct str_r15 : TAO_PEGTL_STRING("r15") {};
	struct str_rbp : TAO_PEGTL_STRING("rbp") {};
	struct str_rsp : TAO_PEGTL_STRING("rsp") {};
	struct str_plus : TAO_PEGTL_STRING("\x2b\x3d") {};
	struct str_minus : TAO_PEGTL_STRING("\x2d\x3d") {};
	struct str_times : TAO_PEGTL_STRING("\x2a\x3d") {};
	struct str_bitwise_and : TAO_PEGTL_STRING("\x26\x3d") {};
	struct str_lshift : TAO_PEGTL_STRING("\x3c\x3c=") {};
	struct str_rshift : TAO_PEGTL_STRING(">>=") {};
	struct str_lt : TAO_PEGTL_STRING("\x3c") {};
	struct str_le : TAO_PEGTL_STRING("\x3c=") {};
	struct str_eq : TAO_PEGTL_STRING("=") {};
	struct str_mem : TAO_PEGTL_STRING("mem") {};
	struct str_goto : TAO_PEGTL_STRING("goto") {};
	struct str_cjump : TAO_PEGTL_STRING("cjump") {};
	struct str_call : TAO_PEGTL_STRING("call") {};
	struct str_print : TAO_PEGTL_STRING("print") {};
	struct str_input : TAO_PEGTL_STRING("input") {};
	struct str_allocate : TAO_PEGTL_STRING("allocate") {};
	struct str_tuple_error : TAO_PEGTL_STRING("tuple-error") {};
	struct str_tensor_error : TAO_PEGTL_STRING("tensor-error") {};

	struct label : seq<
		one<':'>,
		name
	> {};

	struct function_name_rule : seq<
		one<'@'>,
		name
	> {};

	struct register_rax_rule : str_rax {};
	struct register_rbx_rule : str_rbx {};
	struct register_rcx_rule : str_rcx {};
	struct register_rdx_rule : str_rdx {};
	struct register_rdi_rule : str_rdi {};
	struct register_rsi_rule : str_rsi {};
	struct register_r8_rule : str_r8 {};
	struct register_r9_rule : str_r9 {};
	struct register_r10_rule : str_r10 {};
	struct register_r11_rule : str_r11 {};
	struct register_r12_rule : str_r12 {};
	struct register_r13_rule : str_r13 {};
	struct register_r14_rule : str_r14 {};
	struct register_r15_rule : str_r15 {};
	struct register_rbp_rule : str_rbp {};
	struct register_rsp_rule : str_rsp {};

	struct register_rule : sor<
		register_rax_rule,
		register_rbx_rule,
		register_rcx_rule,
		register_rdx_rule,
		register_rdi_rule,
		register_rsi_rule,
		register_r8_rule,
		register_r9_rule,
		register_r10_rule,
		register_r11_rule,
		register_r12_rule,
		register_r13_rule,
		register_r14_rule,
		register_r15_rule,
		register_rbp_rule,
		register_rsp_rule
	> {};

	struct register_shift_rule : upcast<
		sor<
			register_rcx_rule
		>,
		register_rule
	> {};

	// "a" in the grammer
	struct register_idk_rule : upcast<
		sor<
			register_rdi_rule,
			register_rsi_rule,
			register_rdx_rule,
			register_shift_rule,
			register_r8_rule,
			register_r9_rule
		>,
		register_rule
	> {};

	// "w" in the grammar
	struct register_writable_rule : upcast<
		sor<
			register_idk_rule,
			register_rax_rule,
			register_rbx_rule,
			register_rbp_rule,
			register_r10_rule,
			register_r11_rule,
			register_r12_rule,
			register_r13_rule,
			register_r14_rule,
			register_r15_rule
		>,
		register_rule
	> {};

	// "x" in the grammar
	struct register_any_rule : upcast<
		sor<
			register_writable_rule,
			register_rsp_rule
		>,
		register_rule
	> {};

	// "u" in the grammar
	struct call_dest_rule : sor<
		register_writable_rule,
		function_name_rule
	> {};

	// "t" in the grammar
	struct arithmetic_value_rule : sor<
		register_any_rule,
		number
	> {};

	// "s" in the grammar
	struct source_value_rule : sor<
		arithmetic_value_rule,
		label,
		function_name_rule
	> {};

	// "aop" in the grammar
	struct arithmetic_operator : sor<
		str_plus,
		str_minus,
		str_times,
		str_bitwise_and
	> {};

	// "sop" in the grammar
	struct shift_operator : sor<
		str_lshift,
		str_rshift
	> {};

	// "cmp" in the grammar
	struct comparison_operator : sor<
		str_lt,
		str_le,
		str_eq
	> {};

	// TODO remove bc it's not necessary
	struct function_name : label {};

	struct comment : disable<
		TAO_PEGTL_STRING("//"),
		until<eolf>
	> {};

	/*
	 * Separators.
	 * TODO add disable?
	 */
	struct spaces : star<
		sor<
			one<' '>,
			one<'\t'>
		>
	> {};

	struct seps : star<
		seq<
			spaces,
			eol
		>
	> {};

	struct seps_with_comments : star<
		seq<
			spaces,
			sor<
				eol,
				comment
			>
		>
	> {};

	struct Instruction_return_rule : seq<
		str_return
	> {};


	// TODO distinguish between the different classes of registers "w", "a", etc
	// also add the different types of assignments
	struct Instruction_assignment_rule : seq<
		register_writable_rule,
		spaces,
		str_arrow,
		spaces,
		source_value_rule
	> {};

	struct Instruction_memory_read_rule : seq<
		register_writable_rule,
		spaces,
		str_arrow,
		spaces,
		str_mem,
		spaces,
		register_any_rule,
		spaces,
		number
	> {};


	struct Instruction_memory_write_rule : seq<
		str_mem,
		spaces,
		register_any_rule,
		spaces,
		number,
		spaces,
		str_arrow,
		spaces,
		register_writable_rule
	> {};

	struct Instruction_arithmetic_operation_rule : seq<
		register_writable_rule,
		spaces,
		arithmetic_operator,
		spaces,
		arithmetic_value_rule
	> {};

	struct Instruction_shift_operation_register_rule : seq<
		register_writable_rule,
		spaces,
		shift_operator,
		spaces,
		register_shift_rule
	> {};

	struct Instruction_shift_operation_immediate_rule : seq<
		register_writable_rule,
		spaces,
		shift_operator,
		spaces,
		number
	> {};

	struct Instruction_plus_write_memory_rule : seq<
		str_mem,
		spaces,
		register_any_rule,
		spaces,
		number,
		spaces,
		str_plus,
		spaces,
		arithmetic_value_rule
	> {};

	struct Instruction_minus_write_memory_rule : seq<
		str_mem,
		spaces,
		register_any_rule,
		spaces,
		number,
		spaces,
		str_minus,
		spaces,
		arithmetic_value_rule
	> {};

	struct Instruction_plus_read_memory_rule : seq<
		register_writable_rule,
		spaces,
		str_plus,
		spaces,
		str_mem,
		spaces,
		register_any_rule,
		spaces,
		number
	> {};

	struct Instruction_minus_read_memory_rule : seq<
		register_writable_rule,
		spaces,
		str_minus,
		spaces,
		str_mem,
		spaces,
		register_any_rule,
		spaces,
		number
	> {};

	struct Instruction_assignment_compare_rule : seq<
		register_writable_rule,
		spaces,
		str_arrow,
		spaces,
		arithmetic_value_rule,
		spaces,
		comparison_operator,
		spaces,
		arithmetic_value_rule
	> {};

	struct Instruction_cjump_rule : seq<
		str_cjump,
		spaces,
		arithmetic_value_rule,
		spaces,
		comparison_operator,
		spaces,
		arithmetic_value_rule,
		spaces,
		label
	> {};

	struct Instruction_goto_rule : seq<
		str_goto,
		spaces,
		label
	> {};

	struct Instruction_call_rule : seq<
		str_call,
		spaces,
		call_dest_rule,
		spaces,
		number
	> {};

	struct Instruction_call_print_rule : seq<
		str_call,
		spaces,
		str_print,
		spaces,
		one<'1'>
	> {};

	struct Instruction_call_input_rule : seq<
		str_call,
		spaces,
		str_input,
		spaces,
		one<'0'>
	> {};

	struct Instruction_call_allocate_rule : seq<
		str_call,
		spaces,
		str_allocate,
		spaces,
		one<'2'>
	> {};

	struct Instruction_call_tuple_error_rule : seq<
		str_call,
		spaces,
		str_tuple_error,
		spaces,
		one<'3'>
	> {};

	struct Instruction_call_tensor_error_rule : seq<
		str_call,
		spaces,
		str_tensor_error,
		spaces,
		tensor_error_arg_number
	>{};

	struct Instruction_writable_increment_rule : seq<
		register_writable_rule,
		spaces,
		one<'+'>,
		one<'+'>
	> {};

	struct Instruction_writable_decrement_rule : seq<
		register_writable_rule,
		spaces,
		one<'-'>,
		one<'-'>
	> {};

	struct Instruction_leaq_rule : seq<
		register_writable_rule,
		spaces,
		one<'@'>,
		spaces,
		register_writable_rule,
		spaces,
		register_writable_rule,
		spaces,
		lea_factor
	> {};

	struct Instruction_rule : sor<
		with_lookahead<Instruction_return_rule>,
		with_lookahead<Instruction_assignment_compare_rule>,
		with_lookahead<Instruction_assignment_rule>,
		with_lookahead<Instruction_memory_read_rule>,
		with_lookahead<Instruction_memory_write_rule>,
		with_lookahead<Instruction_arithmetic_operation_rule>,
		with_lookahead<Instruction_shift_operation_register_rule>,
		with_lookahead<Instruction_shift_operation_immediate_rule>,
		with_lookahead<Instruction_plus_write_memory_rule>,
		with_lookahead<Instruction_plus_read_memory_rule>,
		with_lookahead<Instruction_minus_write_memory_rule>,
		with_lookahead<Instruction_minus_read_memory_rule>,
		with_lookahead<Instruction_cjump_rule>,
		with_lookahead<label>,
		with_lookahead<Instruction_goto_rule>,
		with_lookahead<Instruction_call_rule>,
		with_lookahead<Instruction_call_print_rule>,
		with_lookahead<Instruction_call_input_rule>,
		with_lookahead<Instruction_call_allocate_rule>,
		with_lookahead<Instruction_call_tuple_error_rule>,
		with_lookahead<Instruction_call_tensor_error_rule>,
		with_lookahead<Instruction_writable_increment_rule>,
		with_lookahead<Instruction_writable_decrement_rule>,
		with_lookahead<Instruction_leaq_rule>
	> {};

	struct Instructions_rule : plus<
		seq<
			seps_with_comments,
			bol,
			spaces,
			Instruction_rule,
			seps_with_comments
		>
	> {};

	struct Function_rule: seq<
		seq<spaces, one< '(' >>,
		seps_with_comments,
		seq<spaces, function_name_rule>,
		seps_with_comments,
		seq<spaces, argument_number>,
		seps_with_comments,
		seq<spaces, local_number>,
		seps_with_comments,
		Instructions_rule,
		seps_with_comments,
		seq<spaces, one<')'>>
	> {};

	struct Functions_rule : plus<
		seps_with_comments,
		Function_rule,
		seps_with_comments
	> {};

	struct entry_point_rule : seq<
		seps_with_comments,
		seq<spaces, one< '(' >>,
		seps_with_comments,
		function_name_rule,
		seps_with_comments,
		Functions_rule,
		seps_with_comments,
		seq<spaces, one< ')' >>,
		seps_with_comments
	> { };

	struct grammar : must<
		entry_point_rule
	> {};

	template<typename Rule>
	struct selector : tao::pegtl::parse_tree::selector<
		Rule,
		tao::pegtl::parse_tree::store_content::on<
			name,
			number,
			register_rule,
			arithmetic_operator,
			shift_operator,
			comparison_operator,
			function_name
		>,
		tao::pegtl::parse_tree::remove_content::on<
			entry_point_rule,
			label,
			function_name_rule,
			argument_number,
			local_number,
			// tensor_error_arg_number,
			// lea_factor,
			// call_dest_rule,
			// arithmetic_value_rule,
			// source_value_rule,
			Functions_rule,
			Function_rule,
			Instructions_rule,
			// Instruction_rule,
			Instruction_return_rule,
			Instruction_assignment_rule,
			Instruction_memory_read_rule,
			Instruction_memory_write_rule,
			Instruction_arithmetic_operation_rule,
			Instruction_shift_operation_register_rule,
			Instruction_shift_operation_immediate_rule,
			Instruction_plus_write_memory_rule,
			Instruction_minus_write_memory_rule,
			Instruction_plus_read_memory_rule,
			Instruction_minus_read_memory_rule,
			Instruction_assignment_compare_rule,
			Instruction_cjump_rule,
			Instruction_goto_rule,
			Instruction_call_rule,
			Instruction_call_print_rule,
			Instruction_call_input_rule,
			Instruction_call_allocate_rule,
			Instruction_call_tuple_error_rule,
			Instruction_call_tensor_error_rule,
			Instruction_writable_increment_rule,
			Instruction_writable_decrement_rule,
			Instruction_leaq_rule
		>
		// tao::pegtl::parse_tree::apply< my_helper >::on<
		// 	my_rule_7,
		// 	my_rule_8
		// >
	> {};
	template<typename Rule> struct selector<at<Rule>> : std::true_type {
		static void transform(std::unique_ptr<pegtl::parse_tree::node> &n) {
			n.reset();
		}
	};

	// using node = pegtl::parse_tree::node;
	using node_ptr = std::unique_ptr<tao::pegtl::parse_tree::node>;

	std::unique_ptr<Value> parse_source_value(const node_ptr &node) {
		// node is a variant of source_value
		if (node->is_type<register_rule>()) {
			return std::make_unique<Register>(node->string());
		} else if (node->is_type<number>()) {
			return std::make_unique<Number>(std::stoll(node->string()));
		} else if (node->is_type<label>()) {
			return std::make_unique<LabelLocation>(node->children[0]->string());
		} else if (node->is_type<function_name_rule>()) {
			return std::make_unique<LabelLocation>(node->children[0]->string());
		} else {
			exit(1);
			// TODO error
		}
	}

	std::unique_ptr<Value> parse_memory_location(const node_ptr &reg_node, const node_ptr &offset_node) {
		auto reg_id = reg_node->string();
		auto offset = std::stoll(offset_node->string());
		if (offset % 8 != 0) {
			std::cerr << "Offset must be a multiple of 8\n";
			exit(1);
		}

		return std::make_unique<MemoryLocation>(reg_id, offset);
	}

	std::unique_ptr<Instruction> parse_instruction(const node_ptr &inst_node) {
		if (inst_node->is_type<Instruction_return_rule>()) {
			auto inst = std::make_unique<InstructionReturn>();
			return inst;
		} else if (inst_node->is_type<Instruction_assignment_rule>()) {
			// children: register_writable, source_value
			auto inst = std::make_unique<InstructionAssignment>();
			inst->destination = std::make_unique<Register>(inst_node->children[0]->string());
			inst->source = parse_source_value(inst_node->children[1]);
			inst->op = AssignOperation::pure;
			return inst;
		} else if (inst_node->is_type<Instruction_memory_read_rule>()) {
			// children: register_writable, register, number
			auto inst = std::make_unique<InstructionAssignment>();
			inst->destination = std::make_unique<Register>(inst_node->children[0]->string());
			inst->source = parse_memory_location(inst_node->children[1], inst_node->children[2]);
			inst->op = AssignOperation::pure;
			return inst;
		} else if (inst_node->is_type<Instruction_memory_write_rule>()) {
			// children: register, number, register_writable
			auto inst = std::make_unique<InstructionAssignment>();
			inst->destination = parse_memory_location(inst_node->children[0], inst_node->children[1]);
			inst->source = std::make_unique<Register>(inst_node->children[2]->string());
			inst->op = AssignOperation::pure;
			return inst;
		} else if (
			inst_node->is_type<Instruction_arithmetic_operation_rule>() ||
			inst_node->is_type<Instruction_shift_operation_register_rule>() ||
			inst_node->is_type<Instruction_shift_operation_immediate_rule>()
		) {
			// children: register_writable, arithmetic_operator, arithmetic_value
			auto inst = std::make_unique<InstructionAssignment>();
			inst->destination = std::make_unique<Register>(inst_node->children[0]->string());
			inst->op = toAssignOperation(inst_node->children[1]->string());
			inst->source = parse_source_value(inst_node->children[2]);
			return inst;
		} else if (inst_node->is_type<Instruction_plus_write_memory_rule>()) {
            auto inst = std::make_unique<InstructionAssignment>();
            inst->destination = parse_memory_location(inst_node->children[0], inst_node->children[1]);
            inst->source = std::make_unique<Register>(inst_node->children[2]->string());
            inst->op = AssignOperation::add;
			return inst;
        } else if (inst_node->is_type<Instruction_minus_write_memory_rule>()) {
            auto inst = std::make_unique<InstructionAssignment>();
            inst->destination = parse_memory_location(inst_node->children[0], inst_node->children[1]);
            inst->source = std::make_unique<Register>(inst_node->children[2]->string());
            inst->op = AssignOperation::subtract;
			return inst;
        } else if (inst_node->is_type<Instruction_plus_read_memory_rule>()) {
            auto inst = std::make_unique<InstructionAssignment>();
            inst->destination = std::make_unique<Register>(inst_node->children[0]->string());
            inst->source = parse_memory_location(inst_node->children[1], inst_node->children[2]);
            inst->op = AssignOperation::add;
			return inst;
        } else if (inst_node->is_type<Instruction_minus_read_memory_rule>()) {
            auto inst = std::make_unique<InstructionAssignment>();
            inst->destination = std::make_unique<Register>(inst_node->children[0]->string());
            inst->source = parse_memory_location(inst_node->children[1], inst_node->children[2]);
            inst->op = AssignOperation::subtract;
			return inst;
		} else if (inst_node->is_type<Instruction_assignment_compare_rule>()) {
			// children: register_writable, arithmetic_value, comparison_operator, arithmetic_value
			auto inst = std::make_unique<InstructionCompareAssignment>();
			inst->destination = std::make_unique<Register>(inst_node->children[0]->string());
			inst->lhs = parse_source_value(inst_node->children[1]);
			inst->op = toComparisonOperator(inst_node->children[2]->string());
			inst->rhs = parse_source_value(inst_node->children[3]);
			return inst;
		} else if (inst_node->is_type<Instruction_cjump_rule>()) {
			// children: arithmetic_value, comparison_operator, arithmetic_value, label
			auto inst = std::make_unique<InstructionCompareJump>();
			inst->lhs = parse_source_value(inst_node->children[0]);
			inst->op = toComparisonOperator(inst_node->children[1]->string());
			inst->rhs = parse_source_value(inst_node->children[2]);
			inst->label = std::make_unique<LabelLocation>(inst_node->children[3]->children[0]->string());
			return inst;
		} else if (inst_node->is_type<label>()) {
			// children: name
			return std::make_unique<InstructionLabel>(inst_node->children[0]->string());
		} else if (inst_node->is_type<Instruction_goto_rule>()) {
			// children: label
			auto inst = std::make_unique<InstructionGoto>();
			inst->label = std::make_unique<LabelLocation>(inst_node->children[0]->children[0]->string());
			return inst;
		} else if (inst_node->is_type<Instruction_call_print_rule>()) {
            auto inst = std::make_unique<InstructionCallFunction>();
            inst->functionName = "print";
			inst->isStd = true;
            inst->num_arguments = 1;
            return inst;
		} else if (inst_node->is_type<Instruction_call_allocate_rule>()) {
            auto inst = std::make_unique<InstructionCallFunction>();
            inst->functionName = "allocate";
			inst->isStd = true;
            inst->num_arguments = 2;
            return inst;
        } else if (inst_node->is_type<Instruction_call_input_rule>()) {
            auto inst = std::make_unique<InstructionCallFunction>();
            inst->functionName = "input";
			inst->isStd = true;
            inst->num_arguments = 0;
            return inst;
        } else if (inst_node->is_type<Instruction_call_tuple_error_rule>()) {
            auto inst = std::make_unique<InstructionCallFunction>();
            inst->functionName = "tuple_error";
			inst->isStd = true;
            inst->num_arguments = 3;
            return inst;
        } else if (inst_node->is_type<Instruction_call_tensor_error_rule>()) {
            auto inst = std::make_unique<InstructionCallFunction>();
            inst->functionName = "tensor_error";
			inst->isStd = true;
            inst->num_arguments = std::stoll(inst_node->children[0]->string());
            return inst;
		} else if (inst_node->is_type<Instruction_call_rule>()){
            if (inst_node->children[0]->is_type<register_rule>()){
                auto inst = std::make_unique<InstructionCallRegister>();
                inst->reg = std::make_unique<Register>(inst_node->children[0]->string());
                inst->num_arguments = std::stoll(inst_node->children[1]->string());
				return inst;
            } else {
                auto inst = std::make_unique<InstructionCallFunction>();
                inst->functionName = inst_node->children[0]->children[0]->string();
				inst->isStd = false;
                inst->num_arguments = std::stoll(inst_node->children[1]->string());
				return inst;
            }
		} else if (inst_node->is_type<Instruction_writable_increment_rule>()) {
            auto inst = std::make_unique<InstructionIncrement>();
            inst->reg = std::make_unique<Register>(inst_node->children[0]->string());
            return inst;
        } else if (inst_node->is_type<Instruction_writable_decrement_rule>()) {
            auto inst = std::make_unique<InstructionDecrement>();
            inst->reg = std::make_unique<Register>(inst_node->children[0]->string());
            return inst;
		} else if (inst_node->is_type<Instruction_leaq_rule>()) {
            auto inst = std::make_unique<InstructionLeaq>();
            inst->regStore = std::make_unique<Register>(inst_node->children[0]->string());
            inst->regRead = std::make_unique<Register>(inst_node->children[1]->string());
            inst->regOffset = std::make_unique<Register>(inst_node->children[2]->string());
            inst->scale = std::stoll(inst_node->children[3]->string());
            return inst;
		}
		std::cerr << "unknown instruction type " << inst_node->type << std::endl;
		exit(1);
	}

	std::unique_ptr<Function> parse_function(const node_ptr &function_rule) {
		auto f = std::make_unique<Function>();

		f->name = function_rule->children[0]->children[0]->string();
		f->num_arguments = std::stoll(function_rule->children[1]->children[0]->string());
		f->num_locals = std::stoll(function_rule->children[2]->children[0]->string());

		for (node_ptr &instruction : function_rule->children[3]->children) {
			f->instructions.push_back(parse_instruction(instruction));
		}

		return f;
	}

	std::unique_ptr<Program> parse_tree(const node_ptr &root) {
		auto p = std::make_unique<Program>();

		node_ptr &programNode = root->children[0];

		// entry point
		node_ptr &entryNameNode = programNode->children[0]->children[0];
		p->entryPointLabel = entryNameNode->string();

		// functions
		node_ptr &functionsNode = programNode->children[1];
		for (node_ptr &function_ptr : functionsNode->children) {
			p->functions.push_back(parse_function(function_ptr));
		}

		return p;
	}

	std::unique_ptr<Program> parse_file(char *fileName) {

		/*
		 * Check the grammar for some possible issues.
		 */
		if (pegtl::analyze<grammar>() != 0) {
			std::cerr << "There are problems with the grammar" << std::endl;
			exit(1);
		}

		/*
		 * Parse.
		 */
		file_input<> fileInput(fileName);
		auto root = pegtl::parse_tree::parse<grammar, selector>(fileInput);
		if (root) {
			return parse_tree(root);
		} else {
			std::cerr << "no prase u bad" << std::endl;
			exit(1);
		}
		// if (root) {
		// 	parse_tree::print_dot( std::cout, *root );
		// }

	}
}
