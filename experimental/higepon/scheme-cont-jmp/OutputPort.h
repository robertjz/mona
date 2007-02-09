#ifndef __OUTPUT_PORT_H__
#define __OUTPUT_PORT_H__

#include "Object.h"
#include "Charcter.h"

namespace monash {

class OutputPort : public Object
{
public:
    OutputPort(FILE* stream, uint32_t lineno = 0);
    virtual ~OutputPort();

public:
    virtual std::string toString();
    virtual int type() const;
    virtual uint32_t lineno() const { return lineno_; }
    virtual Object* eval(Environment* env);
    virtual bool eqv(Object* o);
    virtual bool eq(Object* o);
    virtual bool writeCharacter(Charcter* c);
    virtual bool write(Object* o);
    virtual bool display(Object* o);
    virtual void close();

protected:
    FILE* stream_;
    uint32_t lineno_;

};

}; // namespace monash

#endif // __OUTPUT_PORT_H__
