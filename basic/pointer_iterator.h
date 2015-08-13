
#ifndef _POINTER_ITERATOR_H_
#define _POINTER_ITERATOR_H_



namespace Iterators {

	//_________________________________________________________

	/**
	* PointerIterator makes it possible to iterate on a container
	* of pointers and automatically does the indirections.
	*/
	template <class CONTAINER, class T>
	class PointerIterator 
	{
	public:
		typedef PointerIterator<CONTAINER, T>    thisclass ;
		typedef typename CONTAINER::iterator     inner_iterator ;

		PointerIterator(const inner_iterator& rhs) : iterator_(rhs) {}

		thisclass& operator++() {
			iterator_++ ;
			return *this ;
		}

		thisclass& operator++(int) {
			iterator_++ ;
			return *this ;
		}

		bool operator==(const thisclass& rhs) {
			return rhs.iterator_ == iterator_ ;
		}

		bool operator!=(const thisclass& rhs) {
			return rhs.iterator_ != iterator_ ;
		}

		operator T*()   { return *iterator_; }
		T* operator->() { return *iterator_; }
		T& operator*()  { return **iterator_; }

		//______________free function____________________

		friend bool operator<(const thisclass& lhs, const thisclass& rhs) {
			return lhs.iterator_ < rhs.iterator_;
		}

	private:
		inner_iterator iterator_ ;
	} ;


	//_________________________________________________________

	/**
	* PointerConstIterator makes it possible to iterate on a container
	* of pointers and automatically does the indirections.
	*/
	template <class CONTAINER, class T> 
	class PointerConstIterator 
	{
	public:
		typedef PointerConstIterator<CONTAINER, T> thisclass ;
		typedef typename CONTAINER::const_iterator inner_iterator ;

		PointerConstIterator(
			const inner_iterator& rhs
			) : iterator_(rhs) {
		}

		thisclass& operator++() {
			iterator_++ ;
			return *this ;
		}

		thisclass& operator++(int) {
			iterator_++ ;
			return *this ;
		}

		bool operator==(const thisclass& rhs) {
			return rhs.iterator_ == iterator_ ;
		}

		bool operator!=(const thisclass& rhs) {
			return rhs.iterator_ != iterator_ ;
		}

		operator const T*() { return *iterator_; }
		const T* operator->() { return *iterator_; }
		const T& operator*() { return **iterator_; }

		//______________free function____________________

		friend bool operator<(const thisclass& lhs, const thisclass& rhs) {
			return lhs.iterator_ < rhs.iterator_;
		}

	private:
		inner_iterator iterator_ ;
	} ;

}


#endif

