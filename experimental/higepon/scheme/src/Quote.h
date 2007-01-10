#ifndef __QUOTE_H__
#define __QUOTE_H__

#include "Object.h"
#include "scheme.h"

namespace monash {

class Quote : public Object
{
public:
    Quote(Node* node, uint32_t lineno = 0);
    virtual ~Quote();

public:
    virtual std::string toString();
    virtual std::string toStringValue();
    virtual int type() const;
    virtual Object* eval(Environment* env);
    virtual uint32_t lineno() const { return lineno_; }
    virtual std::string text() const { return value_;}
    virtual Node* node() { return node_;}
    virtual bool eqv(Object* o);
    virtual bool eq(Object* o);

    virtual Quote* car();
    virtual Quote* cdr();

protected:
    Node* node_;
    std::string value_;
    uint32_t lineno_;
};

}; // namespace monash

#endif // __QUOTE_H__
