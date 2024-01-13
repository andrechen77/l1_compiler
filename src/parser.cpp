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

#include <L1.h>
#include <parser.h>

namespace pegtl = TAO_PEGTL_NAMESPACE;

using namespace pegtl;

namespace L1 {

	/*
	 * Tokens parsed
	 */
	std::vector<Item *> parsed_items;

	template<typename Rule>
	struct with_lookahead : seq<at<Rule>, Rule> {};

	/*
	TODO
	when do we use pegtl::at? whenever there are two patterns that share a common
	prefix?
	*/

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

	// "F" in the grammar
	struct tensor_error_arg_number : sor<
		one<'1'>,
		one<'3'>,
		one<'4'>
	> {};

	// "E" in the grammar
	struct lea_factor : sor<
		one<'1'>,
		one<'2'>,
		one<'4'>,
		one<'8'>
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

	struct register_shift_rule : sor<
		register_rcx_rule
	> {};

	// "a" in the grammer
	struct register_idk_rule : sor<
		register_rdi_rule,
		register_rsi_rule,
		register_rdx_rule,
		register_shift_rule,
		register_r8_rule,
		register_r9_rule
	> {};

	// "w" in the grammar
	struct register_writable_rule : sor<
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
	> {};

	// "x" in the grammar
	struct register_any_rule : sor<
		register_writable_rule,
		register_rsp_rule
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

	struct argument_number : number {};

	struct local_number : number {};

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

	// TODO add the other types of instructions
	struct Instruction_rule : sor<
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
		with_lookahead<Instruction_assignment_compare_rule>,
		with_lookahead<Instruction_cjump_rule>,
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
			seps, // TOOD why not seps with comments?
			bol,
			spaces,
			Instruction_rule,
			seps
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
		// TODO why isn't there the opportunity for comments here
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
		seps // TODO why no comments allowed here
	> { };

	struct grammar : must<
		entry_point_rule
	> {};

	/*
	 * Actions attached to grammar rules.
	 */
	template<typename Rule>
	struct action : pegtl::nothing<Rule> {};

	template<> struct action<function_name_rule> {
		template<typename Input>
		static void apply(const Input &in, Program &p) {
			if (p.entryPointLabel.empty()) {
				p.entryPointLabel = in.string();
			} else {
				auto newF = new Function();
				newF->name = in.string();
				p.functions.push_back(newF);
			}
		}
	};

	template<> struct action<argument_number> {
		template<typename Input>
		static void apply(const Input &in, Program &p) {
			auto currentF = p.functions.back();
			currentF->arguments = std::stoll(in.string());
		}
	};

	template<> struct action<local_number> {
		template<typename Input>
		static void apply(const Input &in, Program &p) {
			auto currentF = p.functions.back();
			currentF->locals = std::stoll(in.string());
		}
	};

	template<> struct action <str_return> {
		template<typename Input>
		static void apply(const Input &in, Program &p) {
			auto currentF = p.functions.back();
			auto i = new Instruction_ret();
			currentF->instructions.push_back(i);
		}
	};

	template<> struct action<register_rdi_rule> {
		template<typename Input>
		static void apply(const Input &in, Program &p) {
			auto r = new Register(RegisterID::rdi);
			parsed_items.push_back(r);
		}
	};

	template<> struct action<register_rax_rule> {
		template<typename Input>
		static void apply(const Input &in, Program &p) {
			auto r = new Register(RegisterID::rax);
			parsed_items.push_back(r);
		}
	};

	template<> struct action<Instruction_assignment_rule> {
		template<typename Input>
		static void apply(const Input &in, Program &p) {

			/*
			 * Fetch the current function.
			 */
			auto currentF = p.functions.back();

			/*
			 * Fetch the last two tokens parsed.
			 */
			auto src = parsed_items.back();
			parsed_items.pop_back();
			auto dst = parsed_items.back();
			parsed_items.pop_back();

			/*
			 * Create the instruction.
			 */
			auto i = new Instruction_assignment(dst, src);

			/*
			 * Add the just-created instruction to the current function.
			 */
			currentF->instructions.push_back(i);
		}
	};

	Program parse_file(char *fileName) {

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
		Program p;
		parse<grammar, action>(fileInput, p);

		return p;
	}
}
