#include <relation.hh>

/*
 * variant
 */
bool operator<=(const variant &a, const variant &b)
{
	return a == b || a < b;
}

bool operator>(const variant &a, const variant &b)
{
	return !(a <= b);
}

bool operator>=(const variant &a, const variant &b)
{
	return a == b || a > b;
}

/*
 * variable
 */
variable::variable(bool b, variant v, std::string n)
: bound(b), instantiation(v), name(n)
{
	return;
}

bool operator==(const variable &a, const variable &b)
{
	return a.bound == b.bound && 
				((a.bound && a.instantiation == b.instantiation) ||
				(!a.bound && a.name == b.name));
}

std::ostream &operator<<(std::ostream &os, const variable &v)
{	
	if(v.bound)
		os << v.instantiation;
	else
		os << v.name;
	return os;
}

/*
 * relation::viterator
 */
relation::viterator::~viterator(void)
{
	return;
}

/*
 * relation::iterator
 */
relation::iterator::iterator(viterator *vi)
: virt(vi)
{
	return;
}

relation::iterator::iterator(const relation::iterator &i)
: virt(i.virt ? i.virt->clone() : 0)
{
	return;
}

relation::iterator::~iterator(void)
{
	if(virt)
		delete virt;
}

relation::iterator &relation::iterator::operator=(const relation::iterator &i)
{
	if(virt)
		delete virt;
	virt = i.virt;

	return *this;
}
		
const relation::row &relation::iterator::dereference(void) const
{
	assert(virt);
	return virt->dereference();
}

bool relation::iterator::equal(const relation::iterator &i) const
{
	return (!i.virt && !virt) || (i.virt && virt && i.virt->equal(*virt));
}

void relation::iterator::increment(void)
{
	assert(virt);
	virt->increment();
}

/*
 * relation
 */
relation::~relation(void)
{
	return;
}

unsigned int relation::size(void) const
{
	return std::distance(begin(),end());
}

bool relation::empty(void) const
{
	return begin() == end();
}

bool relation::insert(rel_ptr r)
{
	assert(r);
	bool ret = false;

	for(const relation::row &s: *r)
		ret |= insert(s);
	
	return ret;
}

/*
 * vector_relation::find_iterator
 */
vector_relation::find_iterator::find_iterator(std::shared_ptr<const std::set<unsigned int>> s, std::set<unsigned int>::const_iterator i, const std::vector<relation::row> &r)
: indices(s), iter(i), source(r)
{
	assert(s);
}

vector_relation::find_iterator::~find_iterator(void)
{
	return;
}

relation::viterator *vector_relation::find_iterator::clone(void) const
{
	return new find_iterator(indices,iter,source);
}

const relation::row &vector_relation::find_iterator::dereference(void) const
{
	assert(source.size() > *iter);
	return source[*iter];
}

bool vector_relation::find_iterator::equal(const relation::viterator &i) const
{
	const find_iterator *j = dynamic_cast<const find_iterator*>(&i);
	return j && j->indices == indices && &(j->source) == &source && j->iter == iter;
}

void vector_relation::find_iterator::increment(void)
{
	iter++;
}

/*
 * vector_relation::vector_iterator
 */
vector_relation::vector_iterator::vector_iterator(std::vector<row>::const_iterator i)
: base(i)
{
	return;
}

vector_relation::vector_iterator::~vector_iterator(void)
{
	return;
}

relation::viterator *vector_relation::vector_iterator::clone(void) const
{
	return new vector_iterator(base);
}

const relation::row &vector_relation::vector_iterator::dereference(void) const
{
	return *base;
}

bool vector_relation::vector_iterator::equal(const relation::viterator &i) const
{	
	const vector_iterator *j = dynamic_cast<const vector_iterator*>(&i);
	return j && j->base == base;
}

void vector_relation::vector_iterator::increment(void)
{
	base++;
}

/*
 * vector_relation
 */
vector_relation::~vector_relation(void)
{
	return;
}

relation::iterator vector_relation::begin(void) const
{
	return iterator(new vector_iterator(m_rows.begin()));
}

relation::iterator vector_relation::end(void) const
{
	return iterator(new vector_iterator(m_rows.end()));
}

std::pair<relation::iterator,relation::iterator> vector_relation::equal_range(const std::vector<variable> &b) const
{
	if(m_rows.empty()) return std::make_pair(end(),end());
	assert(b.size() == m_rows[0].size());

	if(m_indices.empty()) index();
	assert(b.size() == m_indices.size());
	
	unsigned int col = 0;
	std::set<unsigned int> *ret = 0;
	std::multimap<std::string,unsigned int> unbound;
	bool all = true;
	bool last_pass = false;

	while(col < b.size())
	{
		const variable &var = b[col];
		const std::unordered_multimap<variant,unsigned int> &idx = m_indices[col];

		if(var.bound)
		{
			auto n = idx.equal_range(var.instantiation);

			if(!ret)
			{
				ret = new std::set<unsigned int>();
				while(n.first != n.second)
					ret->insert((n.first++)->second);
			}
			else
			{
				std::set<unsigned int> eqr;
				std::set<unsigned int> *nret = new std::set<unsigned int>();

				while(n.first != n.second)
					eqr.insert((n.first++)->second);

				std::set_intersection(eqr.begin(),eqr.end(),ret->begin(),ret->end(),std::inserter(*nret,nret->begin()));
				delete ret;
				ret = nret;
			}

			all = false;
		}
		else
		{
			last_pass |= unbound.count(var.name) > 0;
			unbound.insert(std::make_pair(var.name,col));
		}

		++col;
	}

	if(all)
	{
		unsigned int i = 0;

		ret = new std::set<unsigned int>();
		while(i < m_rows.size())
			ret->insert(i++);
	}

	assert(ret);

	if(last_pass && unbound.size() > 1)
	{
		auto i = ret->begin();
		while(i != ret->end())
		{
			const row &r = m_rows[*i];
			auto j = unbound.begin();

			while(std::next(j) != unbound.end())
			{
				auto n = std::next(j);

				if(n->first == j->first)
				{
					if(!(r[n->second] == r[j->second]))
					{
						i = ret->erase(i);
						break;
					}
				}
				++j;
			}

			++i;
		}
	}
	
	std::shared_ptr<const std::set<unsigned int>> s(ret);
	return std::make_pair(iterator(new find_iterator(s,s->begin(),m_rows)),iterator(new find_iterator(s,s->end(),m_rows)));
}

bool vector_relation::includes(const relation::row &r) const
{
	std::vector<variable> b;
	bool ret = false;
	
	if(m_rows.empty() || r.size() != m_rows[0].size())
		return ret;

	for(const variant &v: r)
		b.push_back(variable(true,v,""));
	
	auto coll = equal_range(b);

	return coll.first != coll.second;
}

bool vector_relation::insert(const relation::row &r)
{
	assert(m_rows.empty() || r.size() == m_rows[0].size());

	if(!includes(r))
	{
		m_rows.push_back(r);
		unsigned int j = 0;

		while(j < r.size())
		{
			while(m_indices.size() <= j)
				m_indices.push_back(std::unordered_multimap<variant,unsigned int>());
			m_indices[j].insert(std::make_pair(r[j],m_rows.size() - 1));

			++j;
		}
		return true;
	}
	else
		return false;
}

void vector_relation::reject(std::function<bool(const relation::row &)> f)
{
	auto i = m_rows.begin();
	std::vector<row> n;
	
	n.reserve(m_rows.size());
	while(i != m_rows.end())
	{
		if(f(*i))
			m_indices.clear();
		else
			n.push_back(*i);
		++i;
	}

	m_rows = n;
}

void vector_relation::index(void) const
{
	m_indices.clear();

	auto i = m_rows.cbegin();
	while(i != m_rows.cend())
	{
		const row &r = *i;
		unsigned int j = 0;

		while(j < r.size())
		{
			while(m_indices.size() <= j)
				m_indices.push_back(std::unordered_multimap<variant,unsigned int>());
			m_indices[j].insert(std::make_pair(r[j],std::distance(m_rows.begin(),i)));

			++j;
		}

		++i;
	}
}
