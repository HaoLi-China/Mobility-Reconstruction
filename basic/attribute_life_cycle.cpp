
#include "attribute_life_cycle.h"


void AttributeLifeCycle::virtual_construct(
	Memory::pointer addr
	) 
{
	ogf_assert(false) ; // should not be called.
}

void AttributeLifeCycle::virtual_destroy(
	Memory::pointer addr
	) 
{
	ogf_assert(false) ; // should not be called.
}

void AttributeLifeCycle::virtual_copy(
									  Memory::pointer lhs, Memory::pointer rhs 
									  ) 
{
	ogf_assert(false) ; // should not be called.
}

void AttributeLifeCycle::virtual_copy_construct(
	Memory::pointer lhs, Memory::pointer rhs 
	) 
{
	ogf_assert(false) ; // should not be called.
}

void AttributeLifeCycle::notify_construct(
	Memory::pointer addr, Record* record
	) 
{
	ogf_assert(false) ; // should not be called.
}

void AttributeLifeCycle::notify_destroy(
										Memory::pointer addr, Record* record
										) 
{
	ogf_assert(false) ; // should not be called.
}

void AttributeLifeCycle::notify_copy(
									 Memory::pointer lhs, Record* record_lhs,
									 Memory::pointer rhs, const Record* record_rhs
									 ) 
{
	ogf_assert(false) ; // should not be called.
}

void AttributeLifeCycle::notify_copy_construct(
	Memory::pointer lhs, Record* record_lhs,
	Memory::pointer rhs, const Record* record_rhs
	)
{
	ogf_assert(false) ; // should not be called.
}