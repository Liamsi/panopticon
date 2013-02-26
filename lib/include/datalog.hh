#ifndef DATALOG_HH
#define DATALOG_HH

#include <set>
#include <map>
#include <algorithm>
#include <functional>
#include <cassert>
#include <string>
#include <iomanip>
#include <sstream>
#include <list>
#include <unordered_set>
#include <unordered_map>
#include <cstring>
#include <memory>
#include <vector>

struct dsl;
class dsl_i;
class dsl_h;

struct predicate;
struct rule;
struct constraint;

#include "relation.hh"

struct predicate
{
	predicate(std::string n, std::initializer_list<variable> &lst, bool m);
	predicate(std::string n, const std::vector<variable> &lst, bool m);

	std::string name;
	std::vector<variable> variables;
	bool negated;
};

bool operator==(const predicate &a, const predicate &b);
std::ostream &operator<<(std::ostream &os, const predicate &p);

struct constraint
{
	enum Type
	{
		Less, LessOrEqual, Greater, GreaterOrEqual,
	};

	constraint(Type t, variable a, variable b);
	bool operator()(const std::unordered_map<std::string,unsigned int> &binding, const relation::row &r) const;

	Type type;
	variable operand1, operand2;
};

std::ostream &operator<<(std::ostream &os, const constraint &c);

struct rule
{
	rule(predicate h);
	rule(predicate h, std::initializer_list<predicate> &lst);
	rule(predicate h, const std::list<predicate> &plst);
	
	predicate head;
	std::list<predicate> body;
	std::list<constraint> constraints;
};
typedef std::shared_ptr<rule> rule_ptr;

std::ostream &operator<<(std::ostream &os, const rule &r);

template<typename... Args>
void insert(rel_ptr rel, Args&&... args)
{
	assert(rel->empty() || rel->begin()->size() == sizeof...(args));
	
	std::vector<variant> nr({variant(args)...});
	rel->insert(nr);
}

std::ostream &operator<<(std::ostream &os, const relation &a);
rel_ptr eval(std::string query, std::multimap<std::string,rule_ptr> &in, std::map<std::string,rel_ptr> &extensional);

class dsl_i
{
public:
	dsl_i(dsl &p, std::vector<variable> &v);

	dsl &parent;
	std::vector<variable> variables;
	std::list<dsl_i> tail;
	bool negated;
};

struct dsl
{
	dsl(std::string n);

	template<typename... Tail>
	dsl_i operator()(Tail&&... tail)
	{
		std::vector<variable> vars;

		fill(vars,tail...);
		return dsl_i(*this,vars);
	}

	std::string name;
	std::vector<rule_ptr> rules;
};

class dsl_h
{
public:
	dsl_h(dsl_i &p,dsl_i &o);

	dsl &parent;
	std::vector<variable> variables;
	rule_ptr cur_rule;
};

class dsl_c
{
public:
	dsl_c(constraint c);

	std::list<constraint> constraints;
};

void fill(std::vector<variable> &p);

template<typename Head>
void fill(std::vector<variable> &p, Head head)
{
	p.push_back(variable(true,head,""));
}

template<>
inline void fill(std::vector<variable> &p, variable head)
{
	p.push_back(head);
}

template<typename Head, typename... Tail>
void fill(std::vector<variable> &p, Head head, Tail&&... tail)
{
	p.push_back(variable(true,head,""));
	fill(p,tail...);
}
	
template<typename... Tail>
void fill(std::vector<variable> &p, variable head, Tail&&... tail)
{
	p.push_back(head);
	fill(p,tail...);
}

dsl_c operator<(variant a, variable b);
dsl_c operator<(variable a, variable b);
dsl_c operator<(variable a, variant b);
dsl_c operator<=(variant a, variable b);
dsl_c operator<=(variable a, variable b);
dsl_c operator<=(variable a, variant b);
dsl_c operator>(variant a, variable b);
dsl_c operator>(variable a, variable b);
dsl_c operator>(variable a, variant b);
dsl_c operator>=(variant a, variable b);
dsl_c operator>=(variable a, variable b);
dsl_c operator>=(variable a, variant b);

dsl_i operator!(dsl_i i);
dsl_h operator,(dsl_h h, dsl_i i);
dsl_h operator,(dsl_h h, dsl_c c);
dsl_h operator<<(dsl_i lhs, dsl_i rhs);

#endif
