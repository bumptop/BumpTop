#ifndef NX_FOUNDATION_NXQUICKSORT
#define NX_FOUNDATION_NXQUICKSORT
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

/**
NxQuickSort
sorts an array of Sortables or an array of pointers to Sortables.
(the second tends to be much faster)
In the first case Predicate is simply:

class SortableCompareDirect
	{
	public:
	inline bool operator () (const Sortable &a, const Sortable & b)
		{
		return a < b;
		}
	};
In the second case it should include the dereference:

class SortableComparePtr
	{
	public:
	inline bool operator () (const Sortable *a, const Sortable * b)
		{
		return (*a) < (*b);
		}
	};

Where it is assumed that Sortable implements the compare operator:

class SortElem
	{
	public:
	int sortKey;

	....

	inline bool operator <(const SortElem & e) const
		{
		return sortKey < e.sortKey;
		}
	};

This is not used by the below code directly, only the example
predicates above.

Called like this:
std::vector<SortElem> sortVector;
NxQuickSort<SortElem, SortElemCompareDirect>(&sortVector[0], &sortVector[sortVector.size()-1]);

//faster if SortElem is a large object, otherwise its about the same:
std::vector<SortElem * > sortPtrVector;
NxQuickSort<SortElem *, SortElemComparePtr>(&sortPtrVector[0], &sortPtrVector[sortPtrVector.size()-1]);
*/
template<class Sortable, class Predicate>
inline void NxQuickSort(Sortable * start, Sortable * end)
	{
	static Predicate p;	
	Sortable * i; 
	Sortable * j; 
	Sortable m;

	do
		{
		i = start;
		j = end;
		m = *(i + ((j - i) >> 2));

		while (i <= j) 
			{
			while(p(*i,m)) 
				i++;
			while(p(m,*j)) 
				j--;
			if (i <= j) 
				{
				if (i != j)
					{
					Sortable k = *i;
					*i = *j; 
					*j = k;
					}
				i++; 
				j--;
				}
			}
		if (start < j)
			NxQuickSort<Sortable,Predicate>(start, j);

		start = i;
		}
	while (i < end);	//we do this instead of recursing:

//	if (i < end) 
//		NxQuickSortIterate<SortIterator,Predicate>(i, end);
	}

#endif