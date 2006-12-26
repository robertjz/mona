#ifndef __COND_H__
#define __COND_H__

#include "Begin.h"
#include "Number.h"
#include "SpecialIf.h"
#include <vector>
#include <map>

namespace monash {

typedef std::pair<Object*, Objects*> Clause;
typedef std::vector<Clause*> Clauses;

class Cond : public Object
{
public:
    Cond(Clauses* clauses, Objects* elseActions);
    virtual ~Cond();

public:
    virtual std::string toString();
    virtual int type() const;
    virtual Object* eval(Environment* env);
    virtual Objects* elseActions() const { return elseActions_;}
    virtual Clauses* clauses() const { return clauses_; }
    virtual Object* expand();

protected:
    virtual Object* expandInternal(Clauses::iterator it);
    Clauses* clauses_;
    Objects* elseActions_;
};

}; // namespace monash

#endif // __COND_H__
