#include "Pair.h"

using namespace monash;
using namespace std;

Pair::Pair(Object* first, Object* second, uint32_t lineno) : first_(first), second_(second), lineno_(lineno)
{
}

Pair::~Pair()
{
}

std::string Pair::toString()
{
    return "pair(" + first_->toString() + ", " + second_->toString() + ")";
}

int Pair::type() const
{
    return Object::PAIR;
}

Object* Pair::eval(Environment* env)
{
    return this;
}

bool Pair::eqv(Object* o)
{
    if (!o->isPair()) return false;
    Pair* p = (Pair*)o;
    return this == p;
}

bool Pair::eq(Object* o)
{
    return false;
}

