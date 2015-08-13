

#ifndef _GEOM_ITERATORS_H_
#define _GEOM_ITERATORS_H_


// Note: I dont't like macros, but iterators are really too 
//  annoying to use.

#define FOR_EACH_VERTEX(T, mesh, it)                         \
	for(                                                  \
	T::Vertex_iterator it=(mesh)->vertices_begin();    \
	it!=(mesh)->vertices_end(); it++                   \
	)

#define FOR_EACH_HALFEDGE(T, mesh, it)                          \
	for(                                                     \
	T::Halfedge_iterator it=(mesh)->halfedges_begin();    \
	it!=(mesh)->halfedges_end(); it++                     \
	)

#define FOR_EACH_EDGE(T, mesh, it)                              \
	for(                                                     \
	T::Halfedge_iterator it=(mesh)->halfedges_begin();    \
	it!=(mesh)->halfedges_end(); it++                     \
	) if(it->is_edge_key())                                  \


#define FOR_EACH_FACET(T, mesh, it)                       \
	for(                                               \
	T::Facet_iterator it=(mesh)->facets_begin();    \
	it!=(mesh)->facets_end(); it++                  \
	)


#define FOR_EACH_CELL(T, mesh, it)                        \
	for(                                               \
	T::Cell_iterator it=(mesh)->cells_begin();      \
	it!=(mesh)->cells_end(); it++                   \
	)

#define FOR_EACH_VERTEX_CONST(T, mesh, it)                       \
	for(                                                      \
	T::Vertex_const_iterator it=(mesh)->vertices_begin();    \
	it!=(T::Vertex_const_iterator)(mesh)->vertices_end(); it++ \
	)

// Leon added (T::Halfedge_const_iterator)
#define FOR_EACH_HALFEDGE_CONST(T, mesh, it)                        \
	for(                                                         \
	T::Halfedge_const_iterator it=(mesh)->halfedges_begin();    \
	it!=(T::Halfedge_const_iterator)(mesh)->halfedges_end(); it++ \
	)  
#define FOR_EACH_EDGE_CONST(T, mesh, it)                           \
	for(                                                        \
	T::Halfedge_const_iterator it=(mesh)->halfedges_begin(); \
	it!=(T::Halfedge_const_iterator)(mesh)->halfedges_end(); it++ \
	) if(it->is_edge_key())                                     \


#define FOR_EACH_FACET_CONST(T, mesh, it)                     \
	for(                                                   \
	T::Facet_const_iterator it=(mesh)->facets_begin();    \
	it!=(T::Facet_const_iterator)(mesh)->facets_end(); it++ \
	)

#define FOR_EACH_CELL_CONST(T, mesh, it)                   \
	for(                                                \
	T::Cell_const_iterator it=(mesh)->cells_begin(); \
	it!=(mesh)->cells_end(); it++                    \
	)


// This stupid C++ compiler requires the 'typename' keyword !!
// This means we cannot use the classic FOR_EACH_FACET macro.

#define FOR_EACH_FACET_GENERIC(MESHTYPE, MESH, IT)                  \
	for( typename MESHTYPE::Facet_iterator IT = MESH->facets_begin();   \
	IT != MESH->facets_end(); IT++ )                              \

#define FOR_EACH_HALFEDGE_GENERIC(MESHTYPE, MESH, IT)                  \
	for( typename MESHTYPE::Halfedge_iterator IT = MESH->halfedges_begin();   \
	IT != MESH->halfedges_end(); IT++ )      

#define FOR_EACH_VERTEX_GENERIC(MESHTYPE, MESH, IT)                  \
	for( typename MESHTYPE::Vertex_iterator IT = MESH->vertices_begin(); \
	IT != MESH->vertices_end(); IT++ )                             \



#endif
