#ifndef RELATION_HH
#define RELATION_HH

#include <vector>
#include <unordered_map>
#include <functional>
#include <typeinfo>
#include <typeindex>
#include <memory>
#include <algorithm>
#include <set>
#include <map>
#include <string>

#include <boost/variant.hpp>
#include <boost/iterator/iterator_facade.hpp>

class relation;
class vector_relation;
struct variable;

typedef boost::variant<unsigned int,std::string> variant;

namespace std 
{
	template<>
	class hash<::variant>
	{
	public:
    size_t operator()(const ::variant &v) const 
    {
			if(v.type() == typeid(bool))
				return hash<bool>()(::boost::get<bool>(v));
			else if(v.type() == typeid(unsigned int))
				return hash<unsigned int>()(::boost::get<unsigned int>(v));
			else if(v.type() == typeid(string))
				return hash<string>()(::boost::get<string>(v));
			else
				assert(false);
    }
	};	
	
	template<>
	class hash<vector<::variant>>
	{
	public:
    size_t operator()(const vector<::variant> &s) const 
    {
			return std::accumulate(s.cbegin(),s.cend(),0,[](size_t acc, const ::variant &v)
				{ return acc ^ hash<::variant>()(v); });
    }
	};	
}

bool operator<=(const variant &a, const variant &b);
bool operator>(const variant &a, const variant &b);
bool operator>=(const variant &a, const variant &b);

struct variable
{
	variable(bool b, variant v, std::string n);

	bool bound;
	variant instantiation;
	std::string name;
};

bool operator==(const variable &a, const variable &b);
std::ostream &operator<<(std::ostream &os, const variable &v);
inline variable operator"" _dl(const char *s, size_t sz)
{
	return variable(false,"",std::string(s));
}

class relation
{
public:
	typedef std::vector<variant> row;
	
	class viterator
	{
	public:
		virtual ~viterator(void);
		virtual viterator *clone(void) const = 0;

		virtual const row &dereference(void) const = 0;
		virtual bool equal(const viterator &i) const = 0;
		virtual void increment(void) = 0;
	};
	
	class iterator : public boost::iterator_facade<iterator,const row,boost::forward_traversal_tag>
	{
	public:
		iterator(viterator *vi);
		iterator(const iterator &i);
		~iterator(void);

		iterator &operator=(const iterator &i);
		
		const row &dereference(void) const;
		bool equal(const iterator &i) const;
		void increment(void);
	
	private:
		viterator* virt;
	};

	virtual ~relation(void) = 0;
	
	virtual iterator begin(void) const = 0;
	virtual iterator end(void) const = 0;
	virtual std::pair<iterator,iterator> equal_range(const std::vector<variable> &b) const = 0;
	unsigned int size(void) const;
	bool empty(void) const;

	virtual bool includes(const relation::row &r) const = 0;

	virtual bool insert(const row &r) = 0;
	bool insert(std::shared_ptr<relation> r);
	virtual void reject(std::function<bool(const row &)> f) = 0;
};
typedef std::shared_ptr<relation> rel_ptr;

class vector_relation : public relation
{
public:
	class find_iterator : public viterator
	{
	public:
		find_iterator(std::shared_ptr<const std::set<unsigned int>> s, std::set<unsigned int>::iterator i, const std::vector<relation::row> &r);
		virtual ~find_iterator(void);
		virtual viterator *clone(void) const;

		virtual const row &dereference(void) const;
		virtual bool equal(const viterator &i) const;
		virtual void increment(void);
	
	private:
		std::shared_ptr<const std::set<unsigned int>> indices;
		std::set<unsigned int>::const_iterator iter;
		const std::vector<relation::row> &source;
	};

	class vector_iterator : public viterator
	{
	public:
		vector_iterator(std::vector<row>::const_iterator i);
		virtual ~vector_iterator(void);
		virtual viterator *clone(void) const;

		virtual const row &dereference(void) const;
		virtual bool equal(const viterator &i) const;
		virtual void increment(void);
	
	private:
		std::vector<row>::const_iterator base;
	};

	virtual ~vector_relation(void);
	
	virtual iterator begin(void) const;
	virtual iterator end(void) const;
	virtual std::pair<iterator,iterator> equal_range(const std::vector<variable> &b) const;

	virtual bool includes(const relation::row &r) const;

	virtual bool insert(const row &r);
	virtual void reject(std::function<bool(const row &)> f);

private:
	std::vector<row> m_rows;
	mutable std::vector<std::unordered_multimap<variant,unsigned int>> m_indices;

	void index(void) const;
};

#endif
