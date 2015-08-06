//
//  main.cpp
//  ArticleDynamicFiniteDomain
//
//  Created by Gabriel Aubut-Lussier on 2015-07-30.
//

#include <boost/icl/interval_map.hpp>
#include <boost/icl/split_interval_map.hpp>
#include <boost/icl/concept/interval.hpp>
#include <iostream>
#include <set>

class DynamicDomain
{
public:
	DynamicDomain(int intValue, std::set<DynamicDomain*>* collection);
	
	DynamicDomain* getPredecessor();
	DynamicDomain* getSuccessor();
	
	int intValue;
	
private:
	std::set<DynamicDomain*>* collection;
};

namespace std
{
	template <>
	struct less<DynamicDomain*>
	{
		bool operator()( const DynamicDomain* lhs, const DynamicDomain* rhs ) const
		{
			return lhs->intValue < rhs->intValue;
		}
	};
}

namespace boost { namespace icl {
	template<>
	bool is_empty<boost::icl::closed_interval<DynamicDomain*>>(const boost::icl::closed_interval<DynamicDomain*>& object)
	{
		return object.lower() == nullptr || object.upper() == nullptr || boost::icl::domain_less<boost::icl::closed_interval<DynamicDomain*>>(object.upper(), object.lower());
	}
}}

namespace boost { namespace icl { namespace detail {
	template <>
	struct successor<DynamicDomain*, true>
	{
		inline static DynamicDomain* apply(DynamicDomain* value)
		{
			return value->getSuccessor();
		}
	};
	
	template <>
	struct successor<DynamicDomain*, false>
	{
		inline static DynamicDomain* apply(DynamicDomain* value)
		{
			return value->getPredecessor();
		}
	};
	
	template <>
	struct predecessor<DynamicDomain*, true>
	{
		inline static DynamicDomain* apply(DynamicDomain* value)
		{
			return value->getPredecessor();
		}
	};
	
	template <>
	struct predecessor<DynamicDomain*, false>
	{
		inline static DynamicDomain* apply(DynamicDomain* value)
		{
			return value->getSuccessor();
		}
	};
}}}

template <typename Domain, typename Codomain, typename Traits = boost::icl::partial_absorber, template<class>class Compare = std::less, template<class>class Combine = boost::icl::inplace_plus, template<class>class Section = boost::icl::inter_section>
using IntervalMapSimple = boost::icl::interval_map<Domain*, Codomain, Traits, Compare, Combine, Section, boost::icl::closed_interval<Domain*>>;

typedef IntervalMapSimple<DynamicDomain, int> MapType;

int main(int argc, const char * argv[])
{
	MapType::value_type test;
	std::set<DynamicDomain*> elements = {
		new DynamicDomain(7, &elements),
		new DynamicDomain(1, &elements),
		new DynamicDomain(2, &elements),
		new DynamicDomain(6, &elements),
		new DynamicDomain(3, &elements),
		new DynamicDomain(4, &elements),
		new DynamicDomain(9001, &elements),
		new DynamicDomain(5, &elements)
	};
	MapType map;
	std::set<DynamicDomain*>::iterator it = elements.begin();
	map.add(std::make_pair(MapType::interval_type::closed_interval(*it, *std::next(it, 1)), 1));
	map.add(std::make_pair(MapType::interval_type::closed_interval(*std::next(it, 2), *std::next(it, 3)), 1));
	map.add(std::make_pair(MapType::interval_type::closed_interval(*std::next(it, 1), *std::next(it, 3)), 1));
	for (auto element : map) {
		std::cout << "Domain: [" << element.first.lower()->intValue << ", " << element.first.upper()->intValue << "] = " << element.second << std::endl;
	}
}

DynamicDomain::DynamicDomain(int intValue, std::set<DynamicDomain*>* collection)
: intValue(intValue), collection(collection)
{}

DynamicDomain*
DynamicDomain::getPredecessor()
{
	DynamicDomain* result;
	std::set<DynamicDomain*>::iterator it = collection->lower_bound(this);
	if (it == collection->begin()) {
		result = nullptr;
	} else {
		result = *std::prev(it);
	}
	return result;
}

DynamicDomain*
DynamicDomain::getSuccessor()
{
	DynamicDomain* result;
	std::set<DynamicDomain*>::iterator it = std::next(collection->lower_bound(this));
	if (it == collection->end()) {
		result = nullptr;
	} else {
		result = *it;
	}
	return result;
}
