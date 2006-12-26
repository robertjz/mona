#ifndef __OBJECT_H__
#define __OBJECT_H__

#define GC_NOT_DLL
#include "gc_cpp.h"
#include "Assert.h"
#include <iostream>
#include <string>
#include <vector>

namespace monash {

class Environment;

#ifdef USE_BOEHM_GC
class Object : public gc_cleanup
#else
class Object
#endif
{
public:
    Object();
    virtual ~Object();

public:
    virtual std::string toString()         = 0;
    virtual int type() const               = 0;
    virtual Object* eval(Environment* env) = 0;
    enum
    {
        NUMBER,
        STRING,
        QUOTE,
        VARIABLE,
        ASSIGNMENT,
        IF,
        LAMBDA,
        PROCEDURE,
        BEGIN,
        DEFINITION,
        COND,
        PAIR,
        APPLICATION,
        PRIMITIVE_PROCEDURE,
        AND,
        OR,
        LET,
        LET_ASTERISK,
    };
};

typedef std::vector<Object*> Objects;

}; // namespace monash

#endif // __OBJECT_H__
