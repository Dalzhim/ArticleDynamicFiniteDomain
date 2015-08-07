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
	DynamicDomain(int intValue, std::set<std::unique_ptr<DynamicDomain>>* collection);
	
	DynamicDomain* getPredecessor();
	DynamicDomain* getSuccessor();
	
	int intValue;
	
private:
	std::set<std::unique_ptr<DynamicDomain>>* collection;
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
using closed_interval_map = boost::icl::interval_map<Domain, Codomain, Traits, Compare, Combine, Section, boost::icl::closed_interval<Domain>>;

typedef closed_interval_map<DynamicDomain*, int> MapType;

int main(int argc, const char * argv[])
{
	std::set<std::unique_ptr<DynamicDomain>> domainSet;
	domainSet.insert(std::unique_ptr<DynamicDomain>(new DynamicDomain(7, &domainSet)));
	domainSet.insert(std::unique_ptr<DynamicDomain>(new DynamicDomain(1, &domainSet)));
	domainSet.insert(std::unique_ptr<DynamicDomain>(new DynamicDomain(2, &domainSet)));
	domainSet.insert(std::unique_ptr<DynamicDomain>(new DynamicDomain(6, &domainSet)));
	domainSet.insert(std::unique_ptr<DynamicDomain>(new DynamicDomain(3, &domainSet)));
	domainSet.insert(std::unique_ptr<DynamicDomain>(new DynamicDomain(4, &domainSet)));
	domainSet.insert(std::unique_ptr<DynamicDomain>(new DynamicDomain(9001, &domainSet)));
	domainSet.insert(std::unique_ptr<DynamicDomain>(new DynamicDomain(5, &domainSet)));
	
	MapType intervalMap;
	std::set<std::unique_ptr<DynamicDomain>>::iterator it = domainSet.begin();
	intervalMap.add(std::make_pair(MapType::interval_type::closed_interval(it->get(), std::next(it, 1)->get()), 1));
	intervalMap.add(std::make_pair(MapType::interval_type::closed_interval(std::next(it, 2)->get(), std::next(it, 3)->get()), 1));
	intervalMap.add(std::make_pair(MapType::interval_type::closed_interval(std::next(it, 1)->get(), std::next(it, 3)->get()), 1));
	for (auto element : intervalMap) {
		std::cout << "Domain: [" << element.first.lower()->intValue << ", " << element.first.upper()->intValue << "] = " << element.second << std::endl;
	}
}

DynamicDomain::DynamicDomain(int intValue, std::set<std::unique_ptr<DynamicDomain>>* collection)
: intValue(intValue), collection(collection)
{}

DynamicDomain*
DynamicDomain::getPredecessor()
{
	DynamicDomain* result;
	std::set<std::unique_ptr<DynamicDomain>>::iterator it = std::lower_bound(collection->begin(), collection->end(), this, [](const std::unique_ptr<DynamicDomain>& lhs, DynamicDomain* rhs) {
		return std::less<DynamicDomain*>()(lhs.get(), rhs);
	});
	if (it == collection->begin()) {
		result = nullptr;
	} else {
		result = std::prev(it)->get();
	}
	return result;
}

DynamicDomain*
DynamicDomain::getSuccessor()
{
	DynamicDomain* result;
	std::set<std::unique_ptr<DynamicDomain>>::iterator it = std::next(std::lower_bound(collection->begin(), collection->end(), this, [](const std::unique_ptr<DynamicDomain>& lhs, DynamicDomain* rhs) {
		return std::less<DynamicDomain*>()(lhs.get(), rhs);
	}));
	if (it == collection->end()) {
		result = nullptr;
	} else {
		result = it->get();
	}
	return result;
}
