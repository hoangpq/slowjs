#include "slowjs/interpret.h"

#include "slowjs/ast.h"
#include "slowjs/vector.h"

struct vector_context;

struct closure {
  struct vector_context *ctx;
  vector_string parameters;
  vector_statement body;
};
typedef struct closure closure;

enum value_type { VALUE_NUMBER, VALUE_CLOSURE };
typedef enum value_type value_type;

struct value {
  value_type type;
  union {
    double number;
    closure closure;
  } value;
};
typedef struct value value;

DECLARE_VECTOR(value)

struct context {
  vector_char id;
  value value;
};
typedef struct context context;

DECLARE_VECTOR(context)

interpret_error interpret_expression(expression, vector_context *, value *);

interpret_error interpret_operator(operator o, vector_context *ctx,
                                   value *result) {
  value left, right;
  interpret_error error = interpret_expression(o.left_operand, ctx, &left);
  if (error != E_INTERPRET_OK) {
    return error;
  }

  error = interpret_expression(o.right_operand, ctx, &right);
  if (error != E_INTERPRET_OK) {
    return error;
  }

  result->type = VALUE_NUMBER;
  switch (o.type) {
  case OPERATOR_PLUS:
    result->value.number = left.value.number + right.value.number;
    break;
  case OPERATOR_MINUS:
    result->value.number = left.value.number - right.value.number;
    break;
  case OPERATOR_TIMES:
    result->value.number = left.value.number * right.value.number;
    break;
  case OPERATOR_DIV:
    result->value.number = left.value.number / right.value.number;
  }

  return E_INTERPRET_OK;
}

interpret_error interpret_expression(expression e, vector_context *ctx,
                                     value *result) {
  int i;
  switch (e.type) {
  case EXPRESSION_IDENTIFIER:
    for (i = 0; i < ctx->index; i++) {
      if (strncmp(ctx->elements[i].id.elements,
                  e.expression.identifier.elements, ctx->elements[i].id.size)) {
        *result = ctx->elements[i].value;
        return E_INTERPRET_OK;
      }
    }

    // TODO: handle semantics correctly and usefully
    return E_INTERPRET_CRASH;
  case EXPRESSION_NUMBER:
    result->type = VALUE_NUMBER;
    result->value.number = e.expression.number;
    return E_INTERPRET_OK;
  case EXPRESSION_CALL:
    return interpret_function_call(e.expression.function_call, ctx, result);
  case EXPRESSION_OPERATOR:
    return interpret_operator(e.expression.operator, ctx, result);
  }
}

interpret_error interpret_block(vector_expression body, vector_context *ctx,
                                value *result) {
  int i;
  interpret_error error;
  value nothing;
  for (i = 0; i < body.index; i++) {
    error = interpret_expression(body.elements[i], ctx, &nothing);
  }
}

interpret_error interpret_function_call(function_call fc, vector_context *ctx,
                                        value *result) {
  value function = interpret_expression(fc.function);
  if (function.type != VALUE_CLOSURE) {
    return E_INTERPRET_CALL_NONFUNCTION;
  }

  vector_context child_ctx;
  if (vector_context_copy(&child_ctx, ctx->elements, ctx->index)) {
    return E_INTERPRET_CRASH;
  }

  int i;
  value v;
  vector_char p;
  interpret_error error;
  context mapping;
  for (i = 0; i < function.value.closure.parameters.index; i++) {
    p = function.value.closure.parameters.elements[i];
    error = interpret_expression(p, ctx, &v);
    if (error != E_INTERPRET_OK) {
      return error;
    }

    mapping.id = p;
    mapping.value = v;
    vector_context_push(&child_ctx, mapping);
  }

  return interpret_block(function.value.closure.body, &child_ctx, result);
}

interpret_error interpret_function_declaration(function_declaration fd,
                                               vector_context *ctx) {
  closure c = {ctx, fd.parameters, fd.body};
  context mapping = {fd.name, c};
  return (interpret_error)vector_context_push(ctx, mapping);
}

interpret_error interpet_declaration(declaration d, vector_context *ctx) {
  if (d.type == DECLARATION_FUNCTION) {
    return interpret_function_declaration(d.declaration.function, ctx);
  }

  // return interpret_variable_list(d.variable_list, ctx);
}

interpret_error interpret(ast program) {
  interpret_error error;
  int i;
  declaration d;
  function_declaration main;
  bool found_main = false;

  vector_context ctx;

  for (i = 0; i < program.declarations->index; i++) {
    d = program.declarations->elements[i];

    error = interpret_declaration(d, &ctx);
    if (error != E_INTERPRET_OK) {
      return error;
    }

    if (d.type == DECLARATION_FUNCTION &&
        strcmp((string)d.declaration.function.name->elements, "main")) {
      fd = d.declaration.function;
      found_main = true;
    }
  }

  if (!found_main) {
    LOG_ERROR("interpret", "Expected main function", 0);
    return E_INTERPRET_NO_MAIN;
  }

  expression function;
  function.type = EXPRESSION_IDENTIFIER;
  memcpy(function->);
  vector_expression arguments;
  function_call fc = {function, arguments};
  value v;
  error = interpret_function_call(fc, &ctx, &v);
  if (error != E_INTERPRET_OK) {
    return error;
  }
  printf("%lf\n", v.double);

  return E_INTERPRET_OK;
}