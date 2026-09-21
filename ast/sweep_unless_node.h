#pragma once

#include <cdk/ast/expression_node.h>

namespace p6
{

    /**
     * Class for describing sizeof expressions.
     */
    class sweep_unless_node : public cdk::basic_node
    {
        cdk::expression_node *_vector;
        cdk::expression_node *_low;
        cdk::expression_node *_high;
        cdk::expression_node *_condition;
        std::string _function;

    public:
        sweep_unless_node(int lineno, cdk::expression_node *vector, cdk::expression_node *low, cdk::expression_node *high, const std::string &function, cdk::expression_node *condition) : cdk::basic_node(lineno), _vector(vector), _low(low), _high(high), _function(function), _condition(condition)
        {
        }

        cdk::expression_node *vector() { return _vector; }

        cdk::expression_node *low() { return _low; }

        cdk::expression_node *high() { return _high; }

        cdk::expression_node *condition() { return _condition; }

        const std::string &function() { return _function; }

        void accept(basic_ast_visitor *sp, int level) { sp->do_sweep_unless_node(this, level); }
    };

} // p6