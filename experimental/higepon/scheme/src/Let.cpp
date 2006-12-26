#include "Let.h"

using namespace monash;

Let::Let(Objects* body, Variables* variables, Objects* values) : body_(body), variables_(variables), values_(values)
{
}

Let::~Let()
{
}

std::string Let::toString()
{
    return std::string("let");
}

int Let::type() const
{
    return Object::LET;
}

Application* Let::expand()
{
    Lambda* lambda = new Lambda(body_, variables_);ASSERT(lambda);
    Application* application = new Application(lambda, values_); ASSERT(application); return application;
}

Object* Let::eval(Environment* env)
{
    Object* application = expand();
    return application->eval(env);
}
