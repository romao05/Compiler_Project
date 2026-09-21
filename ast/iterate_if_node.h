#pragma once

#include <cdk/ast/expression_node.h>
#include <string>

namespace p6
{

    /**
     * Class for describing iterate-if nodes.
     */
    class iterate_if_node : public cdk::basic_node
    {
        cdk::expression_node *_count;
        cdk::expression_node *_vector;
        const std::string _function;
        cdk::expression_node *_condition;

    public:
        iterate_if_node(int lineno, cdk::expression_node *vector, cdk::expression_node *count, cdk::expression_node *condition, const std::string &function) : basic_node(lineno), _vector(vector), _count(count), _condition(condition), _function(function)
        {
        }

        cdk::expression_node *count() { return _count; }

        cdk::expression_node *vector() { return _vector; }

        const std::string &function() { return _function; }

        cdk::expression_node *condition() { return _condition; }

        void accept(basic_ast_visitor *sp, int level) { sp->do_iterate_if_node(this, level); }
    };

} // p6