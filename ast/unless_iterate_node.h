#pragma once

#include <string>
#include <cdk/ast/expression_node.h>

namespace p6
{

    /**
     * Class for describing the conditional iteration instruction:
     *   unless <condition> iterate <vector> for <count> using <function-name>
     */
    class unless_iterate_node : public cdk::basic_node
    {
        cdk::expression_node *_condition; // guarda do unless (int)
        cdk::expression_node *_vector;    // ponteiro para a zona de memória
        cdk::expression_node *_count;     // número de elementos a processar (int)
        std::string _function;            // nome da função a aplicar

    public:
        unless_iterate_node(int lineno, cdk::expression_node *condition, cdk::expression_node *vector,
                            cdk::expression_node *count, const std::string &function) :
            cdk::basic_node(lineno), _condition(condition), _vector(vector), _count(count), _function(function)
        {
        }

        cdk::expression_node *condition() { return _condition; }

        cdk::expression_node *vector() { return _vector; }

        cdk::expression_node *count() { return _count; }

        const std::string &function() const { return _function; }

        void accept(basic_ast_visitor *sp, int level) { sp->do_unless_iterate_node(this, level); }
    };

} // p6
