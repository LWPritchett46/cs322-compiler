/*
 * SUGGESTION FROM THE CC TEAM:
 * double check the order of actions that are fired.
 * You can do this in (at least) two ways:
 * 1) by using gdb adding breakpoints to actions
 * 2) by adding printing statements in each action
 *
 * For 2), we suggest writing the code to make it straightforward to enable/disable all of them 
 * (e.g., assuming shouldIPrint is a global variable
 *    if (shouldIPrint) std::cout << "MY OUTPUT" << std::endl;
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

#define TAO_PEGTL_NO_MMAP
#include <tao/pegtl.hpp>
#include <tao/pegtl/contrib/analyze.hpp>
#include <tao/pegtl/contrib/raw_string.hpp>

#include "L2.h"
#include "parser.h"

namespace pegtl = TAO_PEGTL_NAMESPACE;

using namespace pegtl;

namespace L2 {

  /* 
   * Spilling shenanigans
   */

  std::string var_replace;
  std::string prefix;

  /* 
   * Grammar rules from now on.
   */
  struct name:
    pegtl::seq<
      pegtl::plus< 
        pegtl::sor<
          pegtl::alpha,
          pegtl::one< '_' >
        >
      >,
      pegtl::star<
        pegtl::sor<
          pegtl::alpha,
          pegtl::one< '_' >,
          pegtl::digit
        >
      >
    > {};

  /* 
   * Keywords.
   */
  struct str_return : TAO_PEGTL_STRING( "return" ) {};
  struct str_mem : TAO_PEGTL_STRING( "mem" ) {};

  struct str_arrow : TAO_PEGTL_STRING( "<-" ) {}; 
  struct str_add : TAO_PEGTL_STRING( "+=" ) {};
  struct str_sub : TAO_PEGTL_STRING( "-=" ) {};
  struct str_mul : TAO_PEGTL_STRING( "*=" ) {};
  struct str_and : TAO_PEGTL_STRING( "&=" ) {};

  struct str_shl : TAO_PEGTL_STRING( "<<=" ) {};
  struct str_shr : TAO_PEGTL_STRING( ">>=" ) {};

  struct str_lt : TAO_PEGTL_STRING( "<" ) {};
  struct str_leq : TAO_PEGTL_STRING( "<=" ) {};
  struct str_eq : TAO_PEGTL_STRING( "=" ) {};

  struct str_rax : TAO_PEGTL_STRING( "rax" ) {};
  struct str_rcx : TAO_PEGTL_STRING( "rcx" ) {};
  struct str_rdx : TAO_PEGTL_STRING( "rdx" ) {};
  struct str_rsi : TAO_PEGTL_STRING( "rsi" ) {};
  struct str_rdi : TAO_PEGTL_STRING( "rdi" ) {};
  struct str_rsp : TAO_PEGTL_STRING( "rsp" ) {};
  struct str_r8  : TAO_PEGTL_STRING( "r8"  ) {};
  struct str_r9  : TAO_PEGTL_STRING( "r9"  ) {};

  struct label:
    pegtl::seq<
      pegtl::one<':'>,
      name
    > {};

  struct function_name_rule:
    pegtl::seq<
      pegtl::one<'@'>,
      name
    > {};

  struct function_defn_rule:
    pegtl::seq<
      pegtl::one<'@'>,
      name
    > {};

  struct variable_rule:
    pegtl::seq<
      pegtl::one<'%'>,
      name
    > {};

  struct var_replace_rule:
    variable_rule {};

  struct prefix_rule:
    variable_rule {};

  struct shift_rule:
    pegtl::sor<
      str_rcx,
      variable_rule
    > {};

  struct argument_rule:
    pegtl::sor<
      str_rdi,
      str_rsi,
      str_rdx,
      shift_rule,
      str_r8,
      str_r9
    > {};

  struct writable_register_rule:
    pegtl::sor<
      argument_rule,
      str_rax
    > {};    
   
  struct register_rule:
    pegtl::sor<
      writable_register_rule,
      str_rsp
    > {};

  struct number:
    pegtl::seq<
      pegtl::opt<
        pegtl::sor<
          pegtl::one< '-' >,
          pegtl::one< '+' >
        >
      >,
      pegtl::plus< 
        pegtl::digit
      >
    >{};

  struct trivial_rule:
    pegtl::sor<
      register_rule,
      number
    > {};

  struct source_rule:
    pegtl::sor<
      label,
      trivial_rule,
      function_name_rule
    > {};

  struct argument_number:
    number {};

  struct local_number:
    number {};

  struct comment: 
    pegtl::disable< 
      TAO_PEGTL_STRING( "//" ), 
      pegtl::until< pegtl::eolf > 
    > {};

  /*
   * Separators.
   */
  struct spaces :
    pegtl::star< 
      pegtl::sor<
        pegtl::one< ' ' >,
        pegtl::one< '\t'>
      >
    > { };

  struct seps : 
    pegtl::star<
      pegtl::seq<
        spaces,
        pegtl::eol
      >
    > { };
  struct seps_with_comments : 
    pegtl::star< 
      pegtl::seq<
        spaces,
        pegtl::sor<
          pegtl::eol,
          comment
        >
      >
    > { };

  struct mem_access_rule:
    pegtl::seq<
      str_mem,
      spaces,
      register_rule,
      spaces,
      number
    > {};

  struct Instruction_return_rule:
    pegtl::seq<
      str_return
    > { };

  struct Instruction_assignment_rule:
    pegtl::seq<
      writable_register_rule,
      spaces,
      str_arrow,
      spaces,
      source_rule
    > {};

  struct Instruction_store_rule:
    pegtl::seq<
      mem_access_rule,
      spaces,
      str_arrow,
      spaces,
      source_rule
    > { };

  struct Instruction_load_rule:
    pegtl::seq<
      writable_register_rule,
      spaces,
      str_arrow,
      spaces,
      mem_access_rule
    > { };

  struct Instruction_stack_arg_rule:
    pegtl::seq<
      writable_register_rule,
      spaces,
      str_arrow,
      spaces,
      TAO_PEGTL_STRING( "stack-arg" ),
      spaces,
      number
    > {};

  struct Instruction_arith_rule:
    pegtl::seq<
      writable_register_rule,
      spaces,
      pegtl::sor<
        str_add,
        str_sub,
        str_mul,
        str_and
      >,
      spaces,
      trivial_rule
    > {};

  struct Instruction_inc_rule:
    pegtl::seq<
      writable_register_rule,
      spaces,
      TAO_PEGTL_STRING( "++" )
    > {};

  struct Instruction_dec_rule:
    pegtl::seq<
      writable_register_rule,
      spaces,
      TAO_PEGTL_STRING( "--" )
    > {};

  struct Instruction_mem_rhs_rule:
    pegtl::seq<
      writable_register_rule,
      spaces,
      pegtl::sor<
        str_add,
        str_sub
      >,
      spaces,
      mem_access_rule
    > {};

  struct Instruction_mem_lhs_rule:
    pegtl::seq<
      mem_access_rule,
      spaces,
      pegtl::sor<
        str_add,
        str_sub
      >,
      spaces,
      trivial_rule
    > {};

  struct Instruction_compare_assign_rule:
    pegtl::seq<
      writable_register_rule,
      spaces,
      str_arrow,
      spaces,
      trivial_rule,
      spaces,
      pegtl::sor< str_leq, str_lt, str_eq >,
      spaces,
      trivial_rule
    > {};

  struct Instruction_label_rule:
    pegtl::seq < label, spaces > {};

  struct Instruction_goto_rule:
    pegtl::seq<
      TAO_PEGTL_STRING( "goto" ),
      spaces,
      label
    > {};

  struct Instruction_cjump_rule:
    pegtl::seq<
      TAO_PEGTL_STRING( "cjump" ),
      spaces,
      trivial_rule,
      spaces,
      pegtl::sor< str_leq, str_lt, str_eq >,
      spaces,
      trivial_rule,
      spaces,
      label
    > {};

  struct Instruction_shift_rule:
    pegtl::seq<
      writable_register_rule,
      spaces,
      pegtl::sor<
        str_shl,
        str_shr
      >,
      spaces,
      pegtl::sor<
        shift_rule,
        number
      >
    > {};

  struct lea_offset_rule:
    pegtl::sor<
      pegtl::one< '1' >,
      pegtl::one< '2' >,
      pegtl::one< '4' >,
      pegtl::one< '8' >
    > {};

  struct Instruction_lea_rule:
    pegtl::seq<
      writable_register_rule,
      spaces,
      pegtl::one < '@' >,
      spaces,
      writable_register_rule,
      spaces,
      writable_register_rule,
      spaces,
      lea_offset_rule
    > {};

  struct Instruction_call_rule:
    pegtl::seq<
      TAO_PEGTL_STRING( "call" ),
      spaces,
      pegtl::sor< function_name_rule, writable_register_rule >,
      spaces,
      number
    > {};

  struct Instruction_call_print_rule:
    pegtl::seq<
      TAO_PEGTL_STRING( "call" ),
      spaces,
      TAO_PEGTL_STRING( "print" ),
      spaces,
      pegtl::one< '1' >
    > {};

  struct Instruction_call_input_rule:
    pegtl::seq<
      TAO_PEGTL_STRING( "call" ),
      spaces,
      TAO_PEGTL_STRING( "input" ),
      spaces,
      pegtl::one< '0' >
    > {};

  struct Instruction_call_allocate_rule:
    pegtl::seq<
      TAO_PEGTL_STRING( "call" ),
      spaces,
      TAO_PEGTL_STRING( "allocate" ),
      spaces,
      pegtl::one< '2' >
    > {};

  struct Instruction_call_tuple_error_rule:
    pegtl::seq<
      TAO_PEGTL_STRING( "call" ),
      spaces,
      TAO_PEGTL_STRING( "tuple-error" ),
      spaces,
      pegtl::one< '3' >
    > {};

  struct tensor_error_args:
    pegtl::sor<
      pegtl::one< '1' >,
      pegtl::one< '3' >,
      pegtl::one< '4' >
    > {};

  struct Instruction_call_tensor_error_rule:
    pegtl::seq<
      TAO_PEGTL_STRING( "call" ),
      spaces,
      TAO_PEGTL_STRING( "tensor-error" ),
      spaces,
      tensor_error_args
    > {};

  struct Instruction_rule:
    pegtl::sor<
      pegtl::seq< pegtl::at<Instruction_call_tensor_error_rule> , Instruction_call_tensor_error_rule  >,
      pegtl::seq< pegtl::at<Instruction_call_tuple_error_rule>  , Instruction_call_tuple_error_rule   >,
      pegtl::seq< pegtl::at<Instruction_call_allocate_rule>     , Instruction_call_allocate_rule      >,
      pegtl::seq< pegtl::at<Instruction_call_input_rule>        , Instruction_call_input_rule         >,
      pegtl::seq< pegtl::at<Instruction_call_print_rule>        , Instruction_call_print_rule         >,
      pegtl::seq< pegtl::at<Instruction_inc_rule>               , Instruction_inc_rule                >,
      pegtl::seq< pegtl::at<Instruction_dec_rule>               , Instruction_dec_rule                >,
      pegtl::seq< pegtl::at<Instruction_call_rule>              , Instruction_call_rule               >,
      pegtl::seq< pegtl::at<Instruction_lea_rule>               , Instruction_lea_rule                >,
      pegtl::seq< pegtl::at<Instruction_shift_rule>             , Instruction_shift_rule              >,
      pegtl::seq< pegtl::at<Instruction_cjump_rule>             , Instruction_cjump_rule              >,
      pegtl::seq< pegtl::at<Instruction_goto_rule>              , Instruction_goto_rule               >,
      pegtl::seq< pegtl::at<Instruction_label_rule>             , Instruction_label_rule              >,
      pegtl::seq< pegtl::at<Instruction_compare_assign_rule>    , Instruction_compare_assign_rule     >,
      pegtl::seq< pegtl::at<Instruction_mem_rhs_rule>           , Instruction_mem_rhs_rule            >,
      pegtl::seq< pegtl::at<Instruction_mem_lhs_rule>           , Instruction_mem_lhs_rule            >,
      pegtl::seq< pegtl::at<Instruction_arith_rule>             , Instruction_arith_rule              >,
      pegtl::seq< pegtl::at<Instruction_stack_arg_rule>         , Instruction_stack_arg_rule          >,
      pegtl::seq< pegtl::at<Instruction_load_rule>              , Instruction_load_rule               >,
      pegtl::seq< pegtl::at<Instruction_store_rule>             , Instruction_store_rule              >,
      pegtl::seq< pegtl::at<Instruction_return_rule>            , Instruction_return_rule             >,
      pegtl::seq< pegtl::at<Instruction_assignment_rule>        , Instruction_assignment_rule         >,
      pegtl::seq< pegtl::at<comment>                            , comment              >
    > { };

  struct Instructions_rule:
    pegtl::plus<
      pegtl::seq<
        seps,
        pegtl::bol,
        spaces,
        Instruction_rule,
        seps
      >
    > { };

  struct Function_rule:
    pegtl::seq<
      pegtl::seq<spaces, pegtl::one< '(' >>,
      seps_with_comments,
      pegtl::seq<spaces, function_defn_rule>,
      seps_with_comments,
      pegtl::seq<spaces, argument_number>,
      seps_with_comments,
      pegtl::opt<pegtl::seq<spaces, local_number>>,
      Instructions_rule,
      seps_with_comments,
      pegtl::seq<spaces, pegtl::one< ')' >>
    > {};

  // a hacked together case for the spill parser
  struct Function_spill_rule:
    pegtl::seq<
      pegtl::seq<spaces, pegtl::one< '(' >>,
      seps_with_comments,
      pegtl::seq<spaces, function_defn_rule>,
      seps_with_comments,
      pegtl::seq<spaces, argument_number>,
      seps_with_comments,
      Instructions_rule,
      seps_with_comments,
      pegtl::seq<spaces, pegtl::one< ')' >>,
      seps_with_comments,
      var_replace_rule,
      seps_with_comments,
      prefix_rule
    > {};

  struct Functions_rule:
    pegtl::plus<
      seps_with_comments,
      Function_rule,
      seps_with_comments
    > {};

  struct entry_point_rule:
    pegtl::seq<
      seps_with_comments,
      pegtl::seq<spaces, pegtl::one< '(' >>,
      seps_with_comments,
      function_defn_rule,
      seps_with_comments,
      Functions_rule,
      seps_with_comments,
      pegtl::seq<spaces, pegtl::one< ')' >>,
      seps
    > { };

  struct grammar : 
    pegtl::must< 
      entry_point_rule
    > {};


  template< typename State >
  Function* getCurrentFunction(State &s);

  template<>
  Function* getCurrentFunction<Program> (Program &p) {
    return p .functions.back();
  }

  template<>
  Function* getCurrentFunction<Function> (Function &f) {
    return &f;
  }


  /* 
   * Actions attached to grammar rules.
   */
  template< typename Rule >
  struct action : pegtl::nothing< Rule > {};

  template<> struct action < label > {
    template< typename Input, typename State >
    static void apply( const Input & in, State &s){
      auto l = new Label(in.string());

      s.parsed_items.push_back(l);
    }
  };

  template<> struct action < function_defn_rule > {
    template< typename Input, typename State >
	  static void apply( const Input & in, State &s){

      if constexpr (std::is_same_v<State, Program>) {
        if (s.entryPointLabel.empty()) {
        s.entryPointLabel = in.string();
      } else {
        auto newF = new Function();
        newF->name = in.string();
        s.functions.push_back(newF);
      }

      } else {
        // State is Function
        s.name = in.string();
      }
    }
  };

  template<> struct action < function_name_rule> {
    template< typename Input, typename State >
    static void apply( const Input & in, State &s){
      auto l = new Label(in.string());
      s.parsed_items.push_back(l);
    }
  };

  template<> struct action < argument_number > {
    template< typename Input, typename State >
	  static void apply( const Input & in, State &s){
      auto currentF = getCurrentFunction(s);
      currentF->arguments = std::stoll(in.string());
    }
  };

  template<> struct action < local_number > {
    template< typename Input, typename State >
	  static void apply( const Input & in, State &s){
      auto currentF = getCurrentFunction(s);
      currentF->locals = std::stoll(in.string());
    }
  };

  template<> struct action < number > {
    template < typename Input, typename State >
    static void apply( const Input & in, State &s){
      auto n = new Number(std::stoll(in.string()));
      s.parsed_items.push_back(n);
    }
  };

  template<> struct action < lea_offset_rule > {
    template< typename Input, typename State >
    static void apply( const Input &in, State &s){
      auto n = new Number(std::stoll(in.string()));
      s.parsed_items.push_back(n);
    }
  };

  template<> struct action < variable_rule > {
    template< typename Input, typename State >
    static void apply( const Input & in, State &s){
      auto v = new Variable(in.string());
      s.parsed_items.push_back(v);
    }
  };

  template<> struct action < var_replace_rule > {
    template < typename Input, typename State >
    static void apply( const Input &in, State&){
      var_replace = in.string();
    }
  };

  template<> struct action < prefix_rule > {
    template < typename Input, typename State >
    static void apply( const Input &in, State&){
      prefix = in.string();
    }
  };

  template<> struct action < tensor_error_args > {
    template < typename Input, typename State >
    static void apply( const Input & in, State &s){
      auto n = new Number(std::stoll(in.string()));
      s.parsed_items.push_back(n);
    }
  };

  template<> struct action < str_return > {
    template< typename Input, typename State >
	  static void apply( const Input & in, State &s){
      auto currentF = getCurrentFunction(s);
      auto i = new Instruction_ret();
      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < str_add > {
    template< typename Input, typename State >
    static void apply(const Input&, State &s){
      s.parsed_arith_ops.push_back(ADD);
    }
  };

  template<> struct action < str_sub > {
    template< typename Input, typename State >
    static void apply(const Input&, State &s){
      s.parsed_arith_ops.push_back(SUB);
    }
  };

  template<> struct action < str_mul > {
    template< typename Input, typename State >
    static void apply(const Input&, State &s){
      s.parsed_arith_ops.push_back(MUL);
    }
  };

  template<> struct action < str_and > {
    template< typename Input, typename State >
    static void apply(const Input&, State &s){
      s.parsed_arith_ops.push_back(AND);
    }
  };

  template<> struct action < str_lt > {
    template< typename Input, typename State >
    static void apply(const Input&, State &s){
      s.parsed_compare_ops.push_back(LT);
    }
  };

  template<> struct action < str_leq > {
    template< typename Input, typename State >
    static void apply(const Input&, State &s){
      s.parsed_compare_ops.push_back(LEQ);
    }
  };

  template<> struct action < str_eq > {
    template< typename Input, typename State >
    static void apply(const Input&, State &s){
      s.parsed_compare_ops.push_back(EQ);
    }
  };

  template<> struct action < str_shl > {
    template< typename Input, typename State >
    static void apply(const Input&, State &s){
      s.parsed_shift_ops.push_back(SHL);
    }
  };

  template<> struct action < str_shr > {
    template< typename Input, typename State >
    static void apply(const Input&, State &s){
      s.parsed_shift_ops.push_back(SHR);
    }
  };

  template<> struct action < str_rax > {
    template< typename Input, typename State >
    static void apply( const Input & in, State &s){
      auto r = new Register(RegisterID::rax);
      s.parsed_items.push_back(r);
    }
  };

  template<> struct action < str_rcx > {
    template< typename Input, typename State >
    static void apply( const Input & in, State &s){
      auto r = new Register(RegisterID::rcx);
      s.parsed_items.push_back(r);
    }
  };

  template<> struct action < str_rdx > {
    template< typename Input, typename State >
    static void apply( const Input & in, State &s){
      auto r = new Register(RegisterID::rdx);
      s.parsed_items.push_back(r);
    }
  };

  template<> struct action < str_rsi > {
    template< typename Input, typename State >
    static void apply( const Input & in, State &s){
      auto r = new Register(RegisterID::rsi);
      s.parsed_items.push_back(r);
    }
  };

  template<> struct action < str_rdi > {
    template< typename Input, typename State >
    static void apply( const Input & in, State &s){
      auto r = new Register(RegisterID::rdi);
      s.parsed_items.push_back(r);
    }
  };

  template<> struct action < str_rsp > {
    template< typename Input, typename State >
    static void apply( const Input & in, State &s){
      auto r = new Register(RegisterID::rsp);
      s.parsed_items.push_back(r);
    }
  };

  template<> struct action < str_r8 > {
    template< typename Input, typename State >
    static void apply( const Input & in, State &s){
      auto r = new Register(RegisterID::r8);
      s.parsed_items.push_back(r);
    }
  };

  template<> struct action < str_r9 > {
    template< typename Input, typename State >
    static void apply( const Input & in, State &s){
      auto r = new Register(RegisterID::r9);
      s.parsed_items.push_back(r);
    }
  };

  template<> struct action < mem_access_rule > {
    template< typename Input, typename State >
    static void apply(const Input & in, State &s){

      auto o = s.parsed_items.back(); s.parsed_items.pop_back();

      auto offset = dynamic_cast<Number *>(o);
      assert(offset && "Offset must be a number!");

      int64_t offset_val = offset->get_n();
      assert((offset_val % 8 == 0) && "Offset must be a multiple of 8!");

      delete offset;

      auto base = s.parsed_items.back(); s.parsed_items.pop_back();

      auto m = new Memory(base, offset_val);
      s.parsed_items.push_back(m);
    }
  };

  template<> struct action < Instruction_assignment_rule > {
    template< typename Input, typename State >
	  static void apply( const Input & in, State &s){

      /* 
       * Fetch the current function.
       */ 
      auto currentF = getCurrentFunction(s);

      /*
       * Fetch the last two tokens parsed.
       */
      auto src = s.parsed_items.back();
      s.parsed_items.pop_back();
      auto dst = s.parsed_items.back();
      s.parsed_items.pop_back();

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

  template<> struct action < Instruction_store_rule > {
    template< typename Input, typename State >
    static void apply( const Input & in, State &s){
      
      auto currentF = getCurrentFunction(s);

      auto src = s.parsed_items.back(); s.parsed_items.pop_back();
      auto mem = s.parsed_items.back(); s.parsed_items.pop_back();

      auto i = new Instruction_store(mem, src);

      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_load_rule > {
    template< typename Input, typename State >
    static void apply( const Input & in, State &s){
      
      auto currentF = getCurrentFunction(s);

      assert(!s.parsed_items.empty());
      auto mem = s.parsed_items.back(); s.parsed_items.pop_back();
      auto dst = s.parsed_items.back(); s.parsed_items.pop_back();

      auto i = new Instruction_load(dst, mem);

      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_stack_arg_rule > {
    template< typename Input, typename State >
    static void apply(const Input&, State &s){

      auto currentF = getCurrentFunction(s);

      auto o = s.parsed_items.back(); s.parsed_items.pop_back();

      auto offset = dynamic_cast<Number *>(o);
      assert(offset && "Offset must be a number!");

      int64_t offset_val = offset->get_n();
      assert((offset_val % 8 == 0) && "Offset must be a multiple of 8!");

      delete offset;

      auto dst = s.parsed_items.back(); s.parsed_items.pop_back();

      auto i = new Instruction_stack_arg(dst, offset_val);

      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_arith_rule > {
    template< typename Input, typename State >
    static void apply(const Input&, State &s){
      auto currentF = getCurrentFunction(s);

      assert(!s.parsed_items.empty());
      auto rhs = s.parsed_items.back(); s.parsed_items.pop_back();
      auto op = s.parsed_arith_ops.back(); s.parsed_arith_ops.pop_back();
      auto lhs = s.parsed_items.back(); s.parsed_items.pop_back();

      auto i = new Instruction_arith(lhs, op, rhs);

      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_inc_rule > {
    template< typename Input, typename State >
    static void apply(const Input&, State &s){

      auto currentF = getCurrentFunction(s);

      assert(!s.parsed_items.empty());
      auto val = s.parsed_items.back(); s.parsed_items.pop_back();

      auto i = new Instruction_inc(val);

      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_dec_rule > {
    template< typename Input, typename State >
    static void apply(const Input&, State &s){
      auto currentF = getCurrentFunction(s);

      auto val = s.parsed_items.back(); s.parsed_items.pop_back();

      auto i = new Instruction_dec(val);

      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_mem_rhs_rule > {
    template< typename Input, typename State >
    static void apply(const Input&, State &s){
      auto currentF = getCurrentFunction(s);

      auto mem = s.parsed_items.back(); s.parsed_items.pop_back();
      auto op = s.parsed_arith_ops.back(); s.parsed_arith_ops.pop_back();
      auto lhs = s.parsed_items.back(); s.parsed_items.pop_back();

      auto i = new Instruction_mem_rhs(lhs, op, mem);

      currentF->instructions.push_back(i);
    }
  };

    template<> struct action < Instruction_mem_lhs_rule > {
    template< typename Input, typename State >
    static void apply(const Input&, State &s){
      auto currentF = getCurrentFunction(s);

      assert(!s.parsed_items.empty());
      auto rhs = s.parsed_items.back(); s.parsed_items.pop_back();
      auto op = s.parsed_arith_ops.back(); s.parsed_arith_ops.pop_back();
      auto mem = s.parsed_items.back(); s.parsed_items.pop_back();

      auto i = new Instruction_mem_lhs(mem, op, rhs);

      currentF->instructions.push_back(i);
    }
  };

  template<> struct action< Instruction_compare_assign_rule > {
    template< typename Input, typename State >
    static void apply(const Input&, State &s){
      auto currentF = getCurrentFunction(s);

      assert(!s.parsed_items.empty());
      auto rhs = s.parsed_items.back(); s.parsed_items.pop_back();
      auto op = s.parsed_compare_ops.back(); s.parsed_compare_ops.pop_back();
      auto lhs = s.parsed_items.back(); s.parsed_items.pop_back();
      auto dst = s.parsed_items.back(); s.parsed_items.pop_back();

      auto i = new Instruction_compare_assign(dst, lhs, op, rhs);

      currentF->instructions.push_back(i);
    }
  };

  template<> struct action< Instruction_label_rule > {
    template< typename Input, typename State >
    static void apply(const Input&, State &s){

      auto currentF = getCurrentFunction(s);

      assert(!s.parsed_items.empty());
      auto lbl = s.parsed_items.back(); s.parsed_items.pop_back();

      auto i = new Instruction_label(lbl);

      currentF->instructions.push_back(i);

    }
  };

  template<> struct action< Instruction_goto_rule > {
    template< typename Input, typename State >
    static void apply(const Input&, State &s){
      auto currentF = getCurrentFunction(s);

      auto lbl = s.parsed_items.back(); s.parsed_items.pop_back();

      auto i = new Instruction_goto(lbl);

      currentF->instructions.push_back(i);
    }
  };

  template<> struct action< Instruction_cjump_rule > {
    template< typename Input, typename State >
    static void apply(const Input&, State &s){
      auto currentF = getCurrentFunction(s);

      auto lbl = s.parsed_items.back(); s.parsed_items.pop_back();
      auto rhs = s.parsed_items.back(); s.parsed_items.pop_back();
      auto op = s.parsed_compare_ops.back(); s.parsed_compare_ops.pop_back();
      auto lhs = s.parsed_items.back(); s.parsed_items.pop_back();

      auto i = new Instruction_cjump(lhs, op, rhs, lbl);

      currentF->instructions.push_back(i);
    }
  };

  template<> struct action< Instruction_shift_rule > {
    template< typename Input, typename State >
    static void apply(const Input&, State &s){
      auto currentF = getCurrentFunction(s);

      auto src = s.parsed_items.back(); s.parsed_items.pop_back();
      auto op = s.parsed_shift_ops.back(); s.parsed_shift_ops.pop_back();
      auto val = s.parsed_items.back(); s.parsed_items.pop_back();

      auto i = new Instruction_shift(val, op, src);

      currentF->instructions.push_back(i);

    }
  };

  template<> struct action< Instruction_lea_rule > {
    template< typename Input, typename State >
    static void apply(const Input&, State &s){
      auto currentF = getCurrentFunction(s);
      
      auto o = s.parsed_items.back(); s.parsed_items.pop_back();
      auto offset = dynamic_cast<Number *>(o);
      auto offset_val = offset->get_n();
      delete offset;

      auto src2 = s.parsed_items.back(); s.parsed_items.pop_back();
      auto src1 = s.parsed_items.back(); s.parsed_items.pop_back();
      auto dst = s.parsed_items.back(); s.parsed_items.pop_back();

      auto i = new Instruction_lea(dst, src1, src2, offset_val);

      currentF->instructions.push_back(i);
    }
  };

  template<> struct action< Instruction_call_rule > {
    template< typename Input, typename State >
    static void apply(const Input&, State &s){
      auto currentF = getCurrentFunction(s);

      auto a = s.parsed_items.back(); s.parsed_items.pop_back();
      auto args = dynamic_cast<Number *>(a);
      auto argsVal = args->get_n();
      delete args;

      auto func = s.parsed_items.back(); s.parsed_items.pop_back();

      auto i = new Instruction_call(func, argsVal);

      currentF->instructions.push_back(i);
    }
  };

  template<> struct action< Instruction_call_print_rule > {
    template< typename Input, typename State >
    static void apply(const Input&, State &s){
      auto currentF = getCurrentFunction(s);

      auto i = new Instruction_call_print();

      currentF->instructions.push_back(i);

    }
  };

  template<> struct action< Instruction_call_input_rule > {
    template< typename Input, typename State >
    static void apply(const Input&, State &s){
      auto currentF = getCurrentFunction(s);

      auto i = new Instruction_call_input();

      currentF->instructions.push_back(i);
    }
  };

  template<> struct action< Instruction_call_allocate_rule > {
    template< typename Input, typename State >
    static void apply(const Input&, State &s){
      auto currentF = getCurrentFunction(s);

      auto i = new Instruction_call_allocate();

      currentF->instructions.push_back(i);
    }
  };

  template<> struct action< Instruction_call_tuple_error_rule > {
    template< typename Input, typename State >
    static void apply(const Input&, State &s){
      auto currentF = getCurrentFunction(s);

      auto i = new Instruction_call_tuple_error();

      currentF->instructions.push_back(i);
    }
  };

  template<> struct action< Instruction_call_tensor_error_rule > {
    template< typename Input, typename State >
    static void apply(const Input&, State &s){
      auto currentF = getCurrentFunction(s);

      auto a = s.parsed_items.back(); s.parsed_items.pop_back();
      auto args = dynamic_cast<Number *>(a);
      auto argsVal = args->get_n();
      delete args;

      auto i = new Instruction_call_tensor_error(argsVal);

      currentF->instructions.push_back(i);
    }
  };

  void parse_file (const char *fileName, Program &p){

    /* 
     * Check the grammar for some possible issues.
     */
    if (pegtl::analyze< grammar >() != 0){
      std::cout << "There are problems with the grammar" << std::endl;
      exit(1);
    }

    /*
     * Parse.
     */
    file_input< > fileInput(fileName);
    parse< grammar, action >(fileInput, p);


    return;
  }

  void parse_function (const char *fileName, Function &f){

    if (pegtl::analyze< Function_rule >() != 0){
      std::cout << "There are problems with the grammar" << std::endl;
      exit(1);
    }

    file_input< > fileInput(fileName);
    parse< Function_rule, action >(fileInput, f);

  }

  std::vector<std::string> parse_for_spill (const char *fileName, Function &f){
    std::vector<std::string> result;

    if (pegtl::analyze< Function_spill_rule >() != 0){
      std::cout << "There are problems with the grammar" << std::endl;
      exit(1);
    }

    file_input< > fileInput(fileName);
    parse< Function_spill_rule, action >(fileInput, f);

    result.push_back(var_replace);
    result.push_back(prefix);

    return result;
  }

}
