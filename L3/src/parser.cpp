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

#include "L3.h"
#include "parser.h"

namespace pegtl = TAO_PEGTL_NAMESPACE;

using namespace pegtl;

namespace L3 {

  /*
  * Grammar rules from now on.
  */
  struct name :
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

  struct str_arrow : 
    pegtl::seq<
      pegtl::one< '<' >,
      pegtl::one< '-' >
    > {};
  struct str_return : TAO_PEGTL_STRING( "return" ) {};
  struct str_define : TAO_PEGTL_STRING( "define" ) {};
  struct str_load : TAO_PEGTL_STRING( "load" ) {};
  struct str_store : TAO_PEGTL_STRING( "store" ) {};
  struct str_br : TAO_PEGTL_STRING( "br" ) {};

  struct str_call : TAO_PEGTL_STRING( "call" ) {};
  struct str_print : TAO_PEGTL_STRING( "print" ) {};
  struct str_allocate : TAO_PEGTL_STRING( "allocate" ) {};
  struct str_input : TAO_PEGTL_STRING( "input" ) {};
  struct str_tuple_error : TAO_PEGTL_STRING( "tuple-error" ) {};
  struct str_tensor_error : TAO_PEGTL_STRING( "tensor-error" ) {};

  struct runtime_function :
    pegtl::sor<
      str_print,
      str_allocate,
      str_input,
      str_tuple_error,
      str_tensor_error
    > {};

  struct str_add : TAO_PEGTL_STRING( "+" ) {};
  struct str_sub : TAO_PEGTL_STRING( "-" ) {};
  struct str_mul : TAO_PEGTL_STRING( "*" ) {};
  struct str_and : TAO_PEGTL_STRING( "&" ) {};
  struct str_shl : pegtl::seq< pegtl::one< '<' >, pegtl::one< '<' >> {};
  struct str_shr : TAO_PEGTL_STRING( ">>" ) {};

  struct op_rule :
    pegtl::sor<
      str_add,
      str_sub,
      str_mul,
      str_and,
      str_shl,
      str_shr
    > {};

  struct str_lt : pegtl::one< '<' > {};
  struct str_leq : pegtl::seq< pegtl::one<'<'>, pegtl::one<'='> > {};
  struct str_eq : TAO_PEGTL_STRING( "=" ) {};
  struct str_geq : TAO_PEGTL_STRING( ">=" ) {};
  struct str_gt : TAO_PEGTL_STRING( ">" ) {};

  struct cmp_rule :
    pegtl::sor<
      str_leq,
      str_geq,
      str_lt,
      str_gt,
      str_eq
    > {};

  struct label :
    pegtl::seq <
      pegtl::one< ':' >,
      name
    > {};

  struct function_name_rule :
    pegtl::seq<
      pegtl::one< '@' >,
      name
    > {};

  struct function_defn_rule :
    pegtl::seq<
      pegtl::one< '@' >,
      name
    > {};

  struct variable_rule :
    pegtl::seq<
      pegtl::one< '%' >,
      name
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
      variable_rule,
      number
    > {};

  struct source_rule:
    pegtl::sor<
      trivial_rule,
      label,
      function_name_rule
    > {};

  struct callee_rule:
    pegtl::sor<
      variable_rule,
      function_name_rule,
      runtime_function
    > {};

  struct comment :
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

  struct args:
    pegtl::opt<
      pegtl::seq<
        trivial_rule,
        pegtl::star<
          pegtl::seq<
            spaces,
            pegtl::one<','>,
            spaces,
            trivial_rule
          >
        >
      >
    > {};

  struct vars:
    pegtl::opt<
      pegtl::seq<
        variable_rule,
        pegtl::star<
          pegtl::seq<
            spaces,
            pegtl::one<','>,
            spaces,
            variable_rule
          >
        >
      >
    > {};

  struct Instruction_return_rule:
    pegtl::seq< 
      str_return 
    > {};

  struct Instruction_return_val_rule:
    pegtl::seq<
      str_return,
      spaces,
      trivial_rule
    > {};

  struct Instruction_assignment_rule:
    pegtl::seq<
      variable_rule,
      spaces,
      str_arrow,
      spaces,
      source_rule
    > {};

  struct Instruction_op_assign_rule:
    pegtl::seq<
      variable_rule,
      spaces,
      str_arrow,
      spaces,
      trivial_rule,
      spaces,
      op_rule,
      spaces,
      trivial_rule
    > {};

  struct Instruction_cmp_assign_rule:
    pegtl::seq<
      variable_rule,
      spaces,
      str_arrow,
      spaces,
      trivial_rule,
      spaces,
      cmp_rule,
      spaces,
      trivial_rule
    > {};

  struct Instruction_load_rule:
    pegtl::seq<
      variable_rule,
      spaces,
      str_arrow,
      spaces,
      str_load,
      spaces,
      variable_rule
    > {};

  struct Instruction_store_rule:
    pegtl::seq<
      str_store,
      spaces,
      variable_rule,
      spaces,
      str_arrow,
      spaces,
      source_rule
    > {};

  struct Instruction_label_rule:
    pegtl::seq<
      label
    > {};

  struct Instruction_branch_rule:
    pegtl::seq<
      str_br,
      spaces,
      label
    > {};

  struct Instruction_branch_cond_rule:
    pegtl::seq<
      str_br,
      spaces,
      trivial_rule,
      spaces,
      label
    > {};

  struct Instruction_call_rule:
    pegtl::seq<
      str_call,
      spaces,
      callee_rule,
      spaces,
      pegtl::one<'('>,
      spaces,
      args,
      spaces,
      pegtl::one<')'>
    > {};

  struct Instruction_call_assign_rule:
    pegtl::seq<
      variable_rule,
      spaces,
      str_arrow,
      str_call,
      spaces,
      callee_rule,
      spaces,
      pegtl::one<'('>,
      spaces,
      args,
      spaces,
      pegtl::one<')'>
    > {};

  struct Instruction_rule:
    pegtl::sor<
      pegtl::seq< pegtl::at<Instruction_call_assign_rule> , Instruction_call_assign_rule >,
      pegtl::seq< pegtl::at<Instruction_call_rule> , Instruction_call_rule >,
      pegtl::seq< pegtl::at<Instruction_load_rule> , Instruction_load_rule >,
      pegtl::seq< pegtl::at<Instruction_store_rule> , Instruction_store_rule >,
      pegtl::seq< pegtl::at<Instruction_op_assign_rule> , Instruction_op_assign_rule >,
      pegtl::seq< pegtl::at<Instruction_cmp_assign_rule> , Instruction_cmp_assign_rule >,
      pegtl::seq< pegtl::at<Instruction_assignment_rule> , Instruction_assignment_rule >,
      pegtl::seq< pegtl::at<Instruction_branch_cond_rule> , Instruction_branch_cond_rule >,
      pegtl::seq< pegtl::at<Instruction_branch_rule> , Instruction_branch_rule >,
      pegtl::seq< pegtl::at<Instruction_return_val_rule> , Instruction_return_val_rule >,
      pegtl::seq< pegtl::at<Instruction_return_rule> , Instruction_return_rule >,
      pegtl::seq< pegtl::at<Instruction_label_rule> , Instruction_label_rule >,
      pegtl::seq< pegtl::at<comment> , comment >
    > {};

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

  struct Function_rule :
    pegtl::seq<
      seps_with_comments,
      pegtl::seq< spaces, str_define>,
      pegtl::seq< spaces, function_defn_rule>,
      seps_with_comments,
      pegtl::seq< spaces, pegtl::one< '(' >>,
      seps_with_comments,
      pegtl::seq< spaces, vars>,
      seps_with_comments,
      pegtl::seq< spaces, pegtl::one< ')' >>,
      seps_with_comments,
      pegtl::seq< spaces, pegtl::one< '{' >>,
      seps_with_comments,
      Instructions_rule,
      seps_with_comments,
      pegtl::seq< spaces, pegtl::one< '}' >>
    > {};

  struct entry_point_rule :
    pegtl::plus<
      seps_with_comments,
      Function_rule,
      seps_with_comments
    > {};



  /*
  * Actions attached to grammar rules
  */
  template< typename Rule >
  struct action : pegtl::nothing< Rule > {};

  template<> struct action < str_add > {
    template< typename Input >
    static void apply(const Input &in, Program &p) {
      p.parsedOps.push_back(Op::ADD);
    }
  };

  template<> struct action < str_sub > {
    template< typename Input >
    static void apply(const Input &in, Program &p) {
      p.parsedOps.push_back(Op::SUB);
    }
  };

  template<> struct action < str_mul > {
    template< typename Input >
    static void apply(const Input &in, Program &p) {
      p.parsedOps.push_back(Op::MUL);
    }
  };

  template<> struct action < str_and > {
    template< typename Input >
    static void apply(const Input &in, Program &p) {
      p.parsedOps.push_back(Op::AND);
    }
  };

  template<> struct action < str_shl > {
    template< typename Input >
    static void apply(const Input &in, Program &p) {
      p.parsedOps.push_back(Op::SHL);
    }
  };

  template<> struct action < str_shr > {
    template< typename Input >
    static void apply(const Input &in, Program &p) {
      p.parsedOps.push_back(Op::SHR);
    }
  };

  template<> struct action < str_leq > {
    template< typename Input >
    static void apply(const Input &in, Program &p) {
      p.parsedCmps.push_back(Cmp::LEQ);
    }
  };

  template<> struct action < str_geq > {
    template< typename Input >
    static void apply(const Input &in, Program &p) {
      p.parsedCmps.push_back(Cmp::GEQ);
    }
  };

  template<> struct action < str_lt > {
    template< typename Input >
    static void apply(const Input &in, Program &p) {
      p.parsedCmps.push_back(Cmp::LT);
    }
  };

  template<> struct action < str_gt > {
    template< typename Input >
    static void apply(const Input &in, Program &p) {
      p.parsedCmps.push_back(Cmp::GT);
    }
  };

  template<> struct action < str_eq > {
    template< typename Input >
    static void apply(const Input &in, Program &p) {
      p.parsedCmps.push_back(Cmp::EQ);
    }
  };

  template<> struct action < label > {
    template< typename Input >
    static void apply(const Input &in, Program &p) {
      auto l = new Label(in.string());

      p.parsedItems.push_back(l);
    }
  };

  template<> struct action < function_name_rule > {
    template< typename Input >
    static void apply(const Input &in, Program &p) {
      auto l = new Label(in.string());

      p.parsedItems.push_back(l);
    }
  };

  template<> struct action < function_defn_rule > {
    template< typename Input >
    static void apply(const Input &in, Program &p) {
      auto newF = new Function();
      newF->name = in.string();
      p.functions.push_back(newF);
    }
  };

  template<> struct action < variable_rule > {
    template< typename Input >
    static void apply(const Input &in, Program &p) {
      auto v = new Variable(in.string());

      p.parsedItems.push_back(v);
    }
  };

  template<> struct action < number > {
    template< typename Input >
    static void apply(const Input &in, Program &p) {
      auto n = new Number(std::stoll(in.string()));

      p.parsedItems.push_back(n);
    }
  };

  template<> struct action < args > {
    template< typename Input >
    static void apply(const Input &in, Program &p) {
      std::vector<Item *> items;

      while (!p.parsedItems.empty()) {
        auto item = p.parsedItems.back(); p.parsedItems.pop_back();
        items.push_back(item);
      }

      auto il = new ItemList(items);

      p.parsedItemLists.push_back(il);
    }
  };

  template<> struct action < vars > {
    template< typename Input >
    static void apply(const Input &in, Program &p) {
      auto currentF = p.functions.back();

      while (!p.parsedItems.empty()) {
        auto item = p.parsedItems.back(); p.parsedItems.pop_back();
        auto var = dynamic_cast<Variable *>(item);
        currentF->arguments.push_back(var);
      }
    }
  };

  template<> struct action < Instruction_return_rule > {
    template< typename Input >
    static void apply(const Input&, Program &p) {
      auto currentF = p.functions.back();

      auto i = new Instruction_return();

      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_return_val_rule > {
    template< typename Input >
    static void apply(const Input&, Program &p) {
      auto currentF = p.functions.back();

      auto val = p.parsedItems.back(); p.parsedItems.pop_back();

      auto i = new Instruction_return_val(val);

      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_assignment_rule > {
    template< typename Input >
    static void apply(const Input&, Program &p) {
      auto currentF = p.functions.back();

      auto src = p.parsedItems.back(); p.parsedItems.pop_back();
      auto dst = p.parsedItems.back(); p.parsedItems.pop_back();

      auto i = new Instruction_assignment(dst, src);

      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_op_assign_rule > {
    template< typename Input >
    static void apply(const Input&, Program &p) {
      auto currentF = p.functions.back();

      auto rhs = p.parsedItems.back(); p.parsedItems.pop_back();
      auto op  = p.parsedOps.back();   p.parsedOps.pop_back();
      auto lhs = p.parsedItems.back(); p.parsedItems.pop_back();
      auto dst = p.parsedItems.back(); p.parsedItems.pop_back();

      auto i = new Instruction_op_assign(dst, lhs, op, rhs);

      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_cmp_assign_rule > {
    template< typename Input >
    static void apply(const Input&, Program &p) {
      auto currentF = p.functions.back();

      auto rhs = p.parsedItems.back(); p.parsedItems.pop_back();
      auto cmp  = p.parsedOps.back();  p.parsedCmps.pop_back();
      auto lhs = p.parsedItems.back(); p.parsedItems.pop_back();
      auto dst = p.parsedItems.back(); p.parsedItems.pop_back();

      auto i = new Instruction_op_assign(dst, lhs, cmp, rhs);

      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_load_rule > {
    template< typename Input >
    static void apply(Input&, Program &p) {
      auto currentF = p.functions.back();

      auto ptr = p.parsedItems.back(); p.parsedItems.pop_back();
      auto dst = p.parsedItems.back(); p.parsedItems.pop_back();

      auto i = new Instruction_load(dst, ptr);

      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_store_rule > {
    template< typename Input >
    static void apply(Input&, Program &p) {
      auto currentF = p.functions.back();

      auto src = p.parsedItems.back(); p.parsedItems.pop_back();
      auto ptr = p.parsedItems.back(); p.parsedItems.pop_back();

      auto i = new Instruction_store(ptr, src);

      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_label_rule > {
    template< typename Input >
    static void apply(Input&, Program &p) {
      auto currentF = p.functions.back();

      auto lbl = p.parsedItems.back(); p.parsedItems.pop_back();

      auto i = new Instruction_label(lbl);

      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_branch_rule > {
    template< typename Input >
    static void apply(Input&, Program &p) {
      auto currentF = p.functions.back();

      auto lbl = p.parsedItems.back(); p.parsedItems.pop_back();

      auto i = new Instruction_branch(lbl);

      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_branch_cond_rule > {
    template< typename Input >
    static void apply(Input&, Program &p) {
      auto currentF = p.functions.back();

      auto lbl = p.parsedItems.back(); p.parsedItems.pop_back();
      auto cond = p.parsedItems.back(); p.parsedItems.pop_back();

      auto i = new Instruction_branch_cond(cond, lbl);

      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_call_rule > {
    template< typename Input >
    static void apply(Input&, Program &p) {
      auto currentF = p.functions.back();

      auto args = p.parsedItemLists.back(); p.parsedItemLists.pop_back();
      auto callee = p.parsedItems.back(); p.parsedItems.pop_back();

      auto i = new Instruction_call(callee, args);

      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_call_assign_rule > {
    template< typename Input >
    static void apply(Input&, Program &p) {
      auto currentF = p.functions.back();

      auto args = p.parsedItemLists.back(); p.parsedItemLists.pop_back();
      auto callee = p.parsedItems.back(); p.parsedItems.pop_back();
      auto dst = p.parsedItems.back(); p.parsedItems.pop_back();

      auto i = new Instruction_call_assign(dst, callee, args);

      currentF->instructions.push_back(i);
    }
  };

}