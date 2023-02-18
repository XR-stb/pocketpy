#pragma once

#include "vm.h"

PyVar VM::run_frame(Frame* frame){
    while(frame->has_next_bytecode()){
        const Bytecode& byte = frame->next_bytecode();
        // if(true || frame->_module != builtins){
        //     printf("%d: %s (%d) %s\n",                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 frame->_ip, OP_NAMES[byte.op], byte.arg, frame->stack_info().c_str());
        // }
        switch (byte.op)
        {
        case OP_NO_OP: break;       // do nothing
        case OP_LOAD_CONST: frame->push(frame->co->consts[byte.arg]); break;
        case OP_LOAD_LAMBDA: {
            PyVar obj = frame->co->consts[byte.arg];
            setattr(obj, __module__, frame->_module);
            frame->push(obj);
        } break;
        case OP_LOAD_NAME_REF: {
            frame->push(PyRef(NameRef(frame->co->names[byte.arg])));
        } break;
        case OP_LOAD_NAME: {
            frame->push(NameRef(frame->co->names[byte.arg]).get(this, frame));
        } break;
        case OP_STORE_NAME: {
            auto& p = frame->co->names[byte.arg];
            NameRef(p).set(this, frame, frame->pop_value(this));
        } break;
        case OP_BUILD_ATTR: {
            int name = byte.arg >> 1;
            bool _rvalue = byte.arg % 2 == 1;
            auto& attr = frame->co->names[name];
            PyVar obj = frame->pop_value(this);
            AttrRef ref = AttrRef(obj, NameRef(attr));
            if(_rvalue) frame->push(ref.get(this, frame));
            else frame->push(PyRef(ref));
        } break;
        case OP_BUILD_INDEX: {
            PyVar index = frame->pop_value(this);
            auto ref = IndexRef(frame->pop_value(this), index);
            if(byte.arg == 0) frame->push(PyRef(ref));
            else frame->push(ref.get(this, frame));
        } break;
        case OP_STORE_REF: {
            PyVar obj = frame->pop_value(this);
            PyVarRef r = frame->pop();
            PyRef_AS_C(r)->set(this, frame, std::move(obj));
        } break;
        case OP_DELETE_REF: {
            PyVarRef r = frame->pop();
            PyRef_AS_C(r)->del(this, frame);
        } break;
        case OP_BUILD_SMART_TUPLE:
        {
            pkpy::Args items = frame->pop_n_reversed(byte.arg);
            bool done = false;
            for(int i=0; i<items.size(); i++){
                if(!items[i]->is_type(tp_ref)) {
                    done = true;
                    for(int j=i; j<items.size(); j++) frame->try_deref(this, items[j]);
                    frame->push(PyTuple(std::move(items)));
                    break;
                }
            }
            if(done) break;
            frame->push(PyRef(TupleRef(std::move(items))));
        } break;
        case OP_BUILD_STRING:
        {
            pkpy::Args items = frame->pop_n_values_reversed(this, byte.arg);
            StrStream ss;
            for(int i=0; i<items.size(); i++) ss << PyStr_AS_C(asStr(items[i]));
            frame->push(PyStr(ss.str()));
        } break;
        case OP_LOAD_EVAL_FN: {
            frame->push(builtins->attr(m_eval));
        } break;
        case OP_LIST_APPEND: {
            pkpy::Args args(2);
            args[1] = frame->pop_value(this);            // obj
            args[0] = frame->top_value_offset(this, -2);     // list
            fast_call(m_append, std::move(args));
        } break;
        case OP_STORE_FUNCTION:
            {
                PyVar obj = frame->pop_value(this);
                const pkpy::Function_& fn = PyFunction_AS_C(obj);
                setattr(obj, __module__, frame->_module);
                frame->f_globals()[fn->name] = obj;
            } break;
        case OP_BUILD_CLASS:
            {
                const Str& clsName = frame->co->names[byte.arg].first;
                PyVar clsBase = frame->pop_value(this);
                if(clsBase == None) clsBase = _t(tp_object);
                check_type(clsBase, tp_type);
                PyVar cls = new_type_object(frame->_module, clsName, clsBase);
                while(true){
                    PyVar fn = frame->pop_value(this);
                    if(fn == None) break;
                    const pkpy::Function_& f = PyFunction_AS_C(fn);
                    setattr(fn, __module__, frame->_module);
                    setattr(cls, f->name, fn);
                }
            } break;
        case OP_RETURN_VALUE: return frame->pop_value(this);
        case OP_PRINT_EXPR:
            {
                const PyVar expr = frame->top_value(this);
                if(expr == None) break;
                *_stdout << PyStr_AS_C(asRepr(expr)) << '\n';
            } break;
        case OP_POP_TOP: frame->_pop(); break;
        case OP_BINARY_OP:
            {
                pkpy::Args args(2);
                args[1] = frame->pop_value(this);
                args[0] = frame->top_value(this);
                frame->top() = fast_call(BINARY_SPECIAL_METHODS[byte.arg], std::move(args));
            } break;
        case OP_BITWISE_OP:
            {
                frame->push(
                    fast_call(BITWISE_SPECIAL_METHODS[byte.arg],
                    frame->pop_n_values_reversed(this, 2))
                );
            } break;
        case OP_COMPARE_OP:
            {
                pkpy::Args args(2);
                args[1] = frame->pop_value(this);
                args[0] = frame->top_value(this);
                frame->top() = fast_call(CMP_SPECIAL_METHODS[byte.arg], std::move(args));
            } break;
        case OP_IS_OP:
            {
                PyVar rhs = frame->pop_value(this);
                bool ret_c = rhs == frame->top_value(this);
                if(byte.arg == 1) ret_c = !ret_c;
                frame->top() = PyBool(ret_c);
            } break;
        case OP_CONTAINS_OP:
            {
                PyVar rhs = frame->pop_value(this);
                bool ret_c = PyBool_AS_C(call(rhs, __contains__, pkpy::one_arg(frame->pop_value(this))));
                if(byte.arg == 1) ret_c = !ret_c;
                frame->push(PyBool(ret_c));
            } break;
        case OP_UNARY_NEGATIVE:
            frame->top() = num_negated(frame->top_value(this));
            break;
        case OP_UNARY_NOT:
            {
                PyVar obj = frame->pop_value(this);
                const PyVar& obj_bool = asBool(obj);
                frame->push(PyBool(!PyBool_AS_C(obj_bool)));
            } break;
        case OP_POP_JUMP_IF_FALSE:
            if(!PyBool_AS_C(asBool(frame->pop_value(this)))) frame->jump_abs(byte.arg);
            break;
        case OP_LOAD_NONE: frame->push(None); break;
        case OP_LOAD_TRUE: frame->push(True); break;
        case OP_LOAD_FALSE: frame->push(False); break;
        case OP_LOAD_ELLIPSIS: frame->push(Ellipsis); break;
        case OP_ASSERT:
            {
                PyVar _msg = frame->pop_value(this);
                Str msg = PyStr_AS_C(asStr(_msg));
                PyVar expr = frame->pop_value(this);
                if(asBool(expr) != True) _error("AssertionError", msg);
            } break;
        case OP_EXCEPTION_MATCH:
            {
                const auto& _e = PyException_AS_C(frame->top());
                Str name = frame->co->names[byte.arg].first;
                frame->push(PyBool(_e.match_type(name)));
            } break;
        case OP_RAISE:
            {
                PyVar obj = frame->pop_value(this);
                Str msg = obj == None ? "" : PyStr_AS_C(asStr(obj));
                Str type = frame->co->names[byte.arg].first;
                _error(type, msg);
            } break;
        case OP_RE_RAISE: _raise(); break;
        case OP_BUILD_LIST:
            frame->push(PyList(
                frame->pop_n_values_reversed(this, byte.arg).to_list()));
            break;
        case OP_BUILD_MAP:
            {
                pkpy::Args items = frame->pop_n_values_reversed(this, byte.arg*2);
                PyVar obj = call(builtins->attr("dict"));
                for(int i=0; i<items.size(); i+=2){
                    call(obj, __setitem__, pkpy::two_args(items[i], items[i+1]));
                }
                frame->push(obj);
            } break;
        case OP_BUILD_SET:
            {
                PyVar list = PyList(
                    frame->pop_n_values_reversed(this, byte.arg).to_list()
                );
                PyVar obj = call(builtins->attr("set"), pkpy::one_arg(list));
                frame->push(obj);
            } break;
        case OP_DUP_TOP: frame->push(frame->top_value(this)); break;
        case OP_CALL:
            {
                int ARGC = byte.arg & 0xFFFF;
                int KWARGC = (byte.arg >> 16) & 0xFFFF;
                pkpy::Args kwargs(0);
                if(KWARGC > 0) kwargs = frame->pop_n_values_reversed(this, KWARGC*2);
                pkpy::Args args = frame->pop_n_values_reversed(this, ARGC);
                PyVar callable = frame->pop_value(this);
                PyVar ret = call(callable, std::move(args), kwargs, true);
                if(ret == _py_op_call) return ret;
                frame->push(std::move(ret));
            } break;
        case OP_JUMP_ABSOLUTE: frame->jump_abs(byte.arg); break;
        case OP_SAFE_JUMP_ABSOLUTE: frame->jump_abs_safe(byte.arg); break;
        case OP_GOTO: {
            const Str& label = frame->co->names[byte.arg].first;
            int* target = frame->co->labels.try_get(label);
            if(target == nullptr) _error("KeyError", "label '" + label + "' not found");
            frame->jump_abs_safe(*target);
        } break;
        case OP_GET_ITER:
            {
                PyVar obj = frame->pop_value(this);
                PyVar iter_obj = asIter(obj);
                PyVarRef var = frame->pop();
                check_type(var, tp_ref);
                PyIter_AS_C(iter_obj)->var = var;
                frame->push(std::move(iter_obj));
            } break;
        case OP_FOR_ITER:
            {
                // top() must be PyIter, so no need to try_deref()
                auto& it = PyIter_AS_C(frame->top());
                PyVar obj = it->next();
                if(obj != nullptr){
                    PyRef_AS_C(it->var)->set(this, frame, std::move(obj));
                }else{
                    int blockEnd = frame->co->blocks[byte.block].end;
                    frame->jump_abs_safe(blockEnd);
                }
            } break;
        case OP_LOOP_CONTINUE:
            {
                int blockStart = frame->co->blocks[byte.block].start;
                frame->jump_abs(blockStart);
            } break;
        case OP_LOOP_BREAK:
            {
                int blockEnd = frame->co->blocks[byte.block].end;
                frame->jump_abs_safe(blockEnd);
            } break;
        case OP_JUMP_IF_FALSE_OR_POP:
            {
                const PyVar expr = frame->top_value(this);
                if(asBool(expr)==False) frame->jump_abs(byte.arg);
                else frame->pop_value(this);
            } break;
        case OP_JUMP_IF_TRUE_OR_POP:
            {
                const PyVar expr = frame->top_value(this);
                if(asBool(expr)==True) frame->jump_abs(byte.arg);
                else frame->pop_value(this);
            } break;
        case OP_BUILD_SLICE:
            {
                PyVar stop = frame->pop_value(this);
                PyVar start = frame->pop_value(this);
                pkpy::Slice s;
                if(start != None) {check_type(start, tp_int); s.start = (int)PyInt_AS_C(start);}
                if(stop != None) {check_type(stop, tp_int); s.stop = (int)PyInt_AS_C(stop);}
                frame->push(PySlice(s));
            } break;
        case OP_IMPORT_NAME:
            {
                const Str& name = frame->co->names[byte.arg].first;
                auto it = _modules.find(name);
                if(it == _modules.end()){
                    auto it2 = _lazy_modules.find(name);
                    if(it2 == _lazy_modules.end()){
                        _error("ImportError", "module '" + name + "' not found");
                    }else{
                        const Str& source = it2->second;
                        CodeObject_ code = compile(source, name, EXEC_MODE);
                        PyVar _m = new_module(name);
                        _exec(code, _m, pkpy::make_shared<pkpy::NameDict>());
                        frame->push(_m);
                        _lazy_modules.erase(it2);
                    }
                }else{
                    frame->push(it->second);
                }
            } break;
        case OP_YIELD_VALUE: return _py_op_yield;
        // TODO: using "goto" inside with block may cause __exit__ not called
        case OP_WITH_ENTER: call(frame->pop_value(this), __enter__); break;
        case OP_WITH_EXIT: call(frame->pop_value(this), __exit__); break;
        case OP_TRY_BLOCK_ENTER: frame->on_try_block_enter(); break;
        case OP_TRY_BLOCK_EXIT: frame->on_try_block_exit(); break;
        default:
            throw std::runtime_error(Str("opcode ") + OP_NAMES[byte.op] + " is not implemented");
            break;
        }
    }

    if(frame->co->src->mode == EVAL_MODE || frame->co->src->mode == JSON_MODE){
        if(frame->_data.size() != 1) throw std::runtime_error("_data.size() != 1 in EVAL/JSON_MODE");
        return frame->pop_value(this);
    }

    if(!frame->_data.empty()) throw std::runtime_error("_data.size() != 0 in EXEC_MODE");
    return None;
}
