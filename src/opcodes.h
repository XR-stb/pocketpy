#ifdef OPCODE

OPCODE(NO_OP)
OPCODE(POP_TOP)
OPCODE(DUP_TOP_VALUE)
OPCODE(CALL)
OPCODE(CALL_UNPACK)
OPCODE(CALL_KWARGS)
OPCODE(CALL_KWARGS_UNPACK)
OPCODE(RETURN_VALUE)
OPCODE(ROT_TWO)

OPCODE(BINARY_OP)
OPCODE(COMPARE_OP)
OPCODE(BITWISE_OP)
OPCODE(IS_OP)
OPCODE(CONTAINS_OP)

OPCODE(UNARY_NEGATIVE)
OPCODE(UNARY_NOT)
OPCODE(UNARY_STAR)

OPCODE(BUILD_LIST)
OPCODE(BUILD_MAP)
OPCODE(BUILD_SET)
OPCODE(BUILD_SLICE)
OPCODE(BUILD_CLASS)
OPCODE(BUILD_TUPLE)
OPCODE(BUILD_TUPLE_REF)
OPCODE(BUILD_STRING)

OPCODE(LIST_APPEND)
OPCODE(IMPORT_NAME)
OPCODE(PRINT_EXPR)

OPCODE(GET_ITER)
OPCODE(FOR_ITER)

OPCODE(WITH_ENTER)
OPCODE(WITH_EXIT)
OPCODE(LOOP_BREAK)
OPCODE(LOOP_CONTINUE)

OPCODE(POP_JUMP_IF_FALSE)
OPCODE(JUMP_ABSOLUTE)
OPCODE(SAFE_JUMP_ABSOLUTE)
OPCODE(JUMP_IF_TRUE_OR_POP)
OPCODE(JUMP_IF_FALSE_OR_POP)

OPCODE(GOTO)

OPCODE(LOAD_CONST)
OPCODE(LOAD_NONE)
OPCODE(LOAD_TRUE)
OPCODE(LOAD_FALSE)
OPCODE(LOAD_EVAL_FN)
OPCODE(LOAD_FUNCTION)
OPCODE(LOAD_ELLIPSIS)
OPCODE(LOAD_NAME)
OPCODE(LOAD_NAME_REF)

OPCODE(ASSERT)
OPCODE(EXCEPTION_MATCH)
OPCODE(RAISE)
OPCODE(RE_RAISE)

OPCODE(BUILD_INDEX)
OPCODE(BUILD_ATTR)
OPCODE(BUILD_ATTR_REF)
OPCODE(STORE_NAME)
OPCODE(STORE_FUNCTION)
OPCODE(STORE_REF)
OPCODE(DELETE_REF)

OPCODE(TRY_BLOCK_ENTER)
OPCODE(TRY_BLOCK_EXIT)

OPCODE(YIELD_VALUE)

OPCODE(FAST_INDEX)      // a[x]
OPCODE(FAST_INDEX_REF)       // a[x]

OPCODE(INPLACE_BINARY_OP)
OPCODE(INPLACE_BITWISE_OP)

OPCODE(SETUP_CLOSURE)
OPCODE(SETUP_DECORATOR)
OPCODE(STORE_ALL_NAMES)

#endif