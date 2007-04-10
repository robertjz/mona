#ifndef __SCHEME_H__
#define __SCHEME_H__

#ifdef GLOBAL_VALUE_DEFINED
#define GLOBAL /* */
#define GLOBAL_VAL(v) = (v)
#else
#define GLOBAL extern "C"
#define GLOBAL_VAL(v) /* */
#endif
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#ifdef MONA
#include <monapi.h>
#else
#include <sys/time.h>    // rlimit
#include <sys/resource.h> // rlimit
#endif
#include "util/Vector.h"
#include "util/String.h"
#include "util/Pair.h"
#include "util/HashMap.h"

namespace util {
    typedef Vector<String*> Strings;
};

#include "MacroFilter.h"
#include "QuoteFilter.h"
#include "Object.h"
#include "Environment.h"
#include "Procedure.h"
#include "Number.h"
#include "SString.h"
#include "Charcter.h"
#include "Vector.h"
#include "Pair.h"
#include "SpecialIf.h"
#include "Assignment.h"
#include "Begin.h"
#include "Definition.h"
#include "Application.h"
#include "Continuation.h"
#include "SExp.h"
#include "Parser.h"
#include "Translator.h"
#include "Macro.h"
#include "Error.h"
#include "ExtRepParser.h"
#include "OutputPort.h"
#include "InputPort.h"
#include "StringReader.h"
#include "Scanner.h"
#include "Values.h"
#include "Nil.h"
#include "Eof.h"
#include "RiteralConstant.h"
#include "procedures/True.h"
#include "procedures/False.h"
#include "procedures/Undef.h"
#include "procedures/Procedure.h"
#include "procedures/Set.h"

namespace monash {
class DynamicWind;
class Pair;
class InputPort;
typedef ::util::Vector<DynamicWind*> DynamicWinds;
typedef ::util::Vector< ::util::Pair<Variable*, Object*> > DefaultProcedures;

}

::util::String load(const ::util::String& file);
void scheme_register_primitives(monash::Environment* env);
void scheme_const_init();
void scheme_expand_stack(uint32_t mb);
void scheme_input_loop();
void scheme_init();
monash::Object* scheme_eval_string(::util::String& input, monash::Environment* env, bool out = false);
int scheme_exec_file(const ::util::String& file);
monash::SExp* objectToSExp(monash::Object* o);
monash::SExp* pairToSExp(monash::Pair* p);


GLOBAL monash::True* g_true;
GLOBAL monash::False* g_false;
GLOBAL monash::Eof* g_eof;
GLOBAL monash::Undef* g_undef;
GLOBAL monash::Nil* g_nil;
GLOBAL monash::Objects* g_no_arg;
GLOBAL monash::DynamicWinds* g_dynamic_winds;
GLOBAL monash::InputPort* g_defaultInputPort;
//GLOBAL monash::DefaultProcedures procedures;
GLOBAL monash::OutputPort* g_currentOutputPort;
GLOBAL monash::OutputPort* g_defaultOutputPort;
GLOBAL monash::InputPort* g_currentInputPort;
GLOBAL monash::Environment* g_top_env;
GLOBAL bool g_gc_initialized GLOBAL_VAL(false);

GLOBAL FILE* g_transcript GLOBAL_VAL(NULL);
GLOBAL ::util::HashMap<int>* g_provide_map;


#define SCM_TRUE   g_true
#define SCM_FALSE  g_false
#define SCM_UNDEF  g_undef
#define SCM_NO_ARG g_no_arg
#define SCM_NIL    g_nil
#define SCM_EOF    g_eof

// notice!
// don't use like below
// SCHEME_WRITE(stream, o->eval);
// o->eval will be called twice!
#define SCHEME_WRITE(stream, ...)                                       \
{                                                                       \
    fprintf(stream, __VA_ARGS__);                                       \
    int __file = fileno(stream);                                        \
    int __out  = fileno(stdout);                                        \
    int __err  = fileno(stderr);                                        \
    if (g_transcript != NULL && (__file == __out || __file == __err))   \
    {                                                                   \
        fprintf(g_transcript, __VA_ARGS__);                             \
        fflush(g_transcript);                                           \
    }                                                                   \
}

#define TRANSCRIPT_WRITE(...)                                   \
{                                                               \
    if (g_transcript != NULL)                                   \
    {                                                           \
        fprintf(g_transcript, __VA_ARGS__);                     \
    }                                                           \
}

#define SCM_APPLY1(name, e, ret, arg1)             \
{                                                  \
    Object* proc = (new Variable(name))->eval(e);  \
    Objects* args = new Objects;                   \
    args->add(arg1);                               \
    ret = Kernel::apply(proc, args, e);            \
}

#define SCM_EVAL(proc, e, ret, arg1)               \
{                                                  \
    Objects* args = new Objects;                   \
    args->add(arg1);                               \
    args->add(e);                                  \
    ret = Kernel::apply(proc, args, e);            \
}

// (a b . c)
#define SCM_LIST_CONS(objects, o, ret, lineno)              \
{                                                           \
    ret = new Pair(SCM_NIL, SCM_NIL, lineno);               \
    Pair* p = ret;                                          \
    for (int i = 0; i < objects->size(); i++)               \
    {                                                       \
        p->setCar(objects->get(i));                         \
        if (i == objects->size() - 1)                       \
        {                                                   \
            p->setCdr(o);                                   \
        }                                                   \
        else                                                \
        {                                                   \
            Pair* tmp = new Pair(SCM_NIL, SCM_NIL, lineno); \
            p->setCdr(tmp);                                 \
            p = tmp;                                        \
        }                                                   \
    }                                                       \
}

// (a b c)
#define SCM_LIST(objects, ret, lineno)                      \
{                                                           \
    ret = new ::monash::Pair(SCM_NIL, SCM_NIL, lineno);     \
    ::monash::Pair* p = ret;                                \
    for (int i = 0; i < objects->size(); i++)               \
    {                                                       \
        p->setCar(objects->get(i));                         \
        if (i != objects->size() - 1)                       \
        {                                                   \
            ::monash::Pair* tmp = new ::monash::Pair(SCM_NIL, SCM_NIL, lineno); \
            p->setCdr(tmp);                                 \
            p = tmp;                                        \
        }                                                   \
    }                                                       \
}



#endif // __SCHEME_H__