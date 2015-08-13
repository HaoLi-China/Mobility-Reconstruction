
#include "map_editor.h"
#include "map_geometry.h"
#include "map_components.h"
#include "../basic/assertions.h"

#include <set>
#include <cfloat>



MapEditor::MapEditor(Map* target) : MapMutator(target) {
	if(target != nil) {
		set_target(target) ;
	}
}

void MapEditor::set_target(Map* target) {
	MapMutator::set_target(target) ;

	if(vertex_normal_.is_bound())
		vertex_normal_.unbind() ;
	if (MapVertexNormal::is_defined(target))
		vertex_normal_.bind(target) ;

	if(is_locked_.is_bound())
		is_locked_.unbind() ;
	if (MapVertexLock::is_defined(target))
		is_locked_.bind(target) ;
}

//__________ CGAL interface __________________________________

MapEditor::Halfedge* MapEditor::make_polygon(int nb) {
	Halfedge* first = nil ;
	Halfedge* cur = nil ;
	for(int i=0; i<nb; i++) {
		if(first == nil) {
			first = new_edge() ;
			cur = first ;
			make_facet_key(cur, new_facet()) ;
		} else {
			make_sequence(cur, new_edge()) ;
			make_sequence(cur->next()->opposite(), cur->opposite()) ;
			set_halfedge_facet(cur->next(), cur->facet()) ;
			set_halfedge_vertex(
				cur->next()->opposite(), cur->vertex()
				) ;
			cur = cur->next() ;
		}
		make_vertex_key(cur, new_vertex()) ;
	}
	make_sequence(cur, first) ;
	make_sequence(first->opposite(), cur->opposite()) ;
	set_halfedge_vertex(first->opposite(), cur->vertex()) ;

	cur = first ;
	do {
		set_halfedge_tex_vertex(cur, new_tex_vertex()) ;
		set_halfedge_tex_vertex(
			cur->opposite()->prev(), cur->tex_vertex()
			) ;
		cur = cur->next() ;
	} while(cur != first) ;

	return first ;
}

MapEditor::Halfedge* MapEditor::make_triangle() {
	return make_polygon(3) ;
}

MapEditor::Halfedge* MapEditor::make_triangle(
	const vec3& p1, const vec3& p2, const vec3& p3
	) {
		Halfedge* result = make_triangle() ;
		result->vertex()->set_point(p1) ;
		result->next()->vertex()->set_point(p2) ;
		result->next()->next()->vertex()->set_point(p3) ;
		return result ;
}

void MapEditor::make_hole(Halfedge* h) {

	ogf_assert(!h->is_border()) ;

	// Note: I have the feeling that this code should be
	//  much simpler ...

	std::vector<Halfedge*> edges_to_delete ;

	delete_facet(h->facet()) ;
	Halfedge* end = h ;
	do {
		set_halfedge_facet(h, nil) ;
		Halfedge* g = h->next() ;
		bool h_opp_on_border = h->opposite()->is_border() ;
		bool g_opp_on_border = g->opposite()->is_border() ;
		if(h_opp_on_border) {
			// remove vertex X if it looks like that : 
			//
			//   ............
			//   ======X=====
			//
			// but not like that :
			//
			//     .....|
			//     .....|
			//     -----X-----
			//          |..... 
			//          |.....

			if(g_opp_on_border) {
				if(g->opposite()->next() == h->opposite()) {
					delete_vertex(h->vertex()) ;
				} else {
					make_vertex_key(h->opposite()->prev()) ;
					Halfedge* z1 = h->opposite()->prev() ;
					Halfedge* z2 = g->opposite()->next() ;
					make_sequence(z1, z2) ;
				}
			}
			edges_to_delete.push_back(h) ;
		} else {
			if(h->next()->opposite()->is_border()) {
				Halfedge* next = 
					h->next()->opposite()->next() ;
				make_sequence(h, next) ;
				make_vertex_key(h) ;
			}
			if(h->prev()->opposite()->is_border()) {
				Halfedge* prev = h->prev()->opposite()->prev() ;
				make_sequence(prev, h) ;
				make_vertex_key(prev) ;
			}
		}
		h = g ;
	} while(h != end) ;

	for(
		std::vector<Halfedge*>::iterator 
		it = edges_to_delete.begin() ; 
	it != edges_to_delete.end() ; it++
		) {
			delete_edge(*it) ;
	}
}

MapEditor::Halfedge* MapEditor::fill_hole(Halfedge* h) {
	ogf_assert(h->is_border()) ;
	Facet* facet = new_facet() ;
	set_facet_on_orbit(h, facet) ;
	make_facet_key(h) ;
	if(border_facet_.is_bound()) {
		border_facet_[facet] = true ;
	}
	return h ;
}

void MapEditor::fill_holes(int max_size) {
	std::vector<Halfedge*> to_fill ;
	Attribute<Halfedge, bool> is_marked(
		target()->halfedge_attribute_manager()
		) ;
	{FOR_EACH_HALFEDGE(Map, target(), it) {
		if(it->is_border() && !is_marked[it]) {
			int size = 0 ;
			Halfedge* h = it ;
			do {
				is_marked[h] = true ;
				h = h->next() ;
				size++ ;
			} while(h != it) ;
			if(max_size == -1 || size <= max_size) {
				to_fill.push_back(it) ;
			}
		}
	}}
	for(unsigned int i=0; i<to_fill.size(); i++) {
		fill_hole(to_fill[i]) ;
	}
}

MapEditor::Halfedge* MapEditor::add_vertex_and_facet_to_border(
	Halfedge* h, Halfedge* g
	) {
		ogf_assert(h->is_border()) ;
		ogf_assert(g->is_border()) ;
		ogf_assert(h != g) ;

		ogf_assert(halfedges_on_same_facet(h,g)) ;

		Halfedge* h2 = h->next() ;
		Halfedge* g2 = g->next() ;

		Halfedge* n1 = new_edge() ;
		Halfedge* n2 = new_edge() ;

		make_sequence(n1, n2) ;
		make_sequence(n2->opposite(), n1->opposite()) ;

		make_vertex_key(n1, new_vertex()) ;
		set_halfedge_vertex(n2->opposite(), n1->vertex()) ;

		make_sequence(n2, h2) ;
		set_halfedge_vertex(n2, h->vertex()) ;

		make_sequence(g, n1) ;
		set_halfedge_vertex(n1->opposite(), g->vertex()) ;

		make_sequence(h, n2->opposite()) ;
		make_sequence(n1->opposite(), g2) ;

		set_facet_on_orbit(n1, new_facet()) ;
		make_facet_key(n1) ;

		set_tex_vertex_on_orbit(n1, new_tex_vertex()) ;
		set_halfedge_tex_vertex(n1->opposite(), n1->prev()->tex_vertex()) ;
		set_halfedge_tex_vertex(n2->opposite(), n2->prev()->tex_vertex()) ;

		Halfedge* cur = n1 ;
		do {
			if(cur->tex_vertex() == nil) {
				set_halfedge_tex_vertex(
					cur, cur->opposite()->prev()->tex_vertex()
					) ;
			}
			cur = cur->next() ;
		} while(cur != n1) ;

		n1->vertex()->set_point(
			Geom::barycenter(h->vertex()->point(), g->vertex()->point())
			) ;

		n1->set_tex_coord(
			Geom::barycenter(h->tex_coord(), g->tex_coord())
			) ;

		return n1 ;
}

MapEditor::Halfedge* MapEditor::add_facet_to_border(
	Halfedge* h, Halfedge* g
	) {
		ogf_assert(h->is_border()) ;
		ogf_assert(g->is_border()) ;
		ogf_assert(h != g) ;
		ogf_assert(halfedges_on_same_facet(h, g)) ;

		Halfedge* h2 = h->next() ;
		Halfedge* g2 = g->next() ;

		Halfedge* n = new_edge() ;

		make_sequence(n, h2) ;
		set_halfedge_vertex(n, h->vertex()) ;

		make_sequence(g, n) ;
		set_halfedge_vertex(n->opposite(), g->vertex()) ;

		make_sequence(h, n->opposite()) ;
		make_sequence(n->opposite(), g2) ;

		set_facet_on_orbit(n, new_facet()) ;
		make_facet_key(n) ;

		set_halfedge_tex_vertex(n, n->opposite()->prev()->tex_vertex()) ;
		set_halfedge_tex_vertex(n->opposite(), n->prev()->tex_vertex()) ;

		return n ;
}

bool MapEditor::can_split_facet(
								Halfedge* h, Halfedge* g
								) 
{
	if(h == g) {
		return false ;
	}
	if(!halfedges_on_same_facet(h,g)) {
		return false ;
	}
	if(h->next() == g || g->next() == h) {
		return false ;
	}
	return true ;
}

bool MapEditor::split_facet(
							Halfedge* h, Halfedge* g
							) 
{
	if(!can_split_facet(h,g)) {
		return false ;
	}

	Map::Facet* f = h->facet() ;

	Halfedge* result = new_edge() ;

	make_sequence(result->opposite(), g->next()) ;
	make_sequence(result, h->next()) ;
	make_sequence(g, result) ;
	make_sequence(h, result->opposite()) ;

	set_halfedge_vertex(result, h->vertex()) ;
	set_halfedge_vertex(result->opposite(), g->vertex()) ;

	make_facet_key(result->opposite(), h->facet()) ;

	set_facet_on_orbit(result, new_facet(f)) ;
	make_facet_key(result) ;

	set_halfedge_tex_vertex(
		result, result->opposite()->prev()->tex_vertex()
		) ;
	set_halfedge_tex_vertex(
		result->opposite(), result->prev()->tex_vertex()
		) ;

	return true ;
}

bool MapEditor::can_join_facets(Halfedge* h) {
	if(h->is_border_edge()) {
		return false ; 
	}
	if(h->vertex()->degree() < 3) {
		return false ;
	}
	if(h->opposite()->vertex()->degree() < 3) {
		return false ;
	}
	if(h->facet() == h->opposite()->facet()) {
		return false ;
	}
	return true ;
}

bool MapEditor::join_facets(Halfedge* h) {
	if(!can_join_facets(h)) {
		return false ;
	}

	// TODO: something finer
	merge_tex_vertices(h->vertex()) ;
	merge_tex_vertices(h->opposite()->vertex()) ;

	Halfedge* result = h->next() ;

	make_sequence(h->opposite()->prev(), h->next()) ;
	make_sequence(h->prev(), h->opposite()->next()) ;

	make_vertex_key(h->prev()) ;
	make_vertex_key(h->opposite()->prev()) ;

	delete_facet(h->opposite()->facet()) ;

	make_facet_key(result) ;
	set_facet_on_orbit(result, result->facet()) ;

	delete_edge(h) ;

	return true ;
}

MapEditor::Vertex* MapEditor::create_center_vertex(Facet* f) {
	Halfedge* h = f->halfedge() ;
	Vertex* v = new_vertex() ;

	vec3 p = Geom::facet_barycenter(f) ;
	vec2 u = Geom::facet_barycenter2d(f) ;

	bool first = true ;
	Halfedge* it = h ;
	do {
		Halfedge* jt = it->next() ;

		Halfedge* z = new_edge() ;
		make_sequence(it, z) ;
		make_sequence(z->opposite(), jt) ;
		set_halfedge_vertex(z, v) ;
		set_halfedge_vertex(z->opposite(), it->vertex()) ;
		set_halfedge_tex_vertex(z->opposite(), it->tex_vertex()) ;
		if(first) {
			first = false ;
			make_vertex_key(z) ;
		} else {
			make_sequence(z, it->prev()) ;
			set_facet_on_orbit(it, new_facet(f)) ;
			make_facet_key(it) ;
		}

		it = jt ;
	} while(it != h) ;

	make_sequence(h->next(), h->prev()) ;
	set_facet_on_orbit(h, new_facet(f)) ;
	make_facet_key(h) ;

	set_tex_vertex_on_orbit(v->halfedge(), new_tex_vertex()) ;

	v->set_point(p) ;
	v->halfedge()->set_tex_coord(u) ;

	delete_facet(f) ;

	return v ;
}


MapEditor::Facet* MapEditor::erase_center_vertex(Vertex* v) {

	// Preprocessing: for a vertex on the border, add a facet
	{
		Halfedge* b = nil ;
		Halfedge* it = v->halfedge() ;
		do {
			if(it->is_border()) {
				b = it ;
				break ;
			}
			it = it->next_around_vertex() ;
		} while(it != v->halfedge()) ;
		if(b != nil) {
			if(b->prev() == b->next()) {
				return nil ;
			}
			add_facet_to_border(b->prev(), b->next()) ;
		}
	}

	Halfedge* h = v->halfedge()->prev() ;
	Halfedge* it = h ;
	do {
		Halfedge* jt = it->next() ;
		if(jt->vertex() == v) {
			jt = jt->opposite()->next() ;
			delete_edge(it->next()) ;
			delete_facet(it->facet()) ;
			make_sequence(it, jt) ;
			make_vertex_key(it) ;
		} 
		it = jt ;
	} while(it != h) ;
	set_facet_on_orbit(h, new_facet()) ;
	make_facet_key(h) ;

	// TODO: something finer
	Halfedge* cur = h ;
	do {
		merge_tex_vertices(cur->vertex()) ;
		cur = cur->next() ;
	} while(cur != h) ;

	delete_vertex(v) ;
	return h->facet() ;
}


MapEditor::Vertex* MapEditor::split_edge(
	Halfedge* h, bool triangulate, double alpha
	) {
		Vertex* ov1 = h->vertex() ;
		Vertex* ov2 = h->opposite()->vertex() ;

		vec3 p = h->vertex()->point()*(1.-alpha) + h->opposite()->vertex()->point()*alpha ;
		vec2 u = h->tex_coord()*(1.-alpha) +  h->prev()->tex_coord()*alpha ;

		Vertex* v = new_vertex() ;

		Halfedge* z1 = h-> next() ;
		Halfedge* z2 = h-> opposite()-> prev() ;
		Halfedge* r = new_edge(h) ;
		make_sequence(h, r) ;
		make_sequence(r-> opposite(), h-> opposite()) ;
		make_sequence(r, z1) ;
		make_sequence(z2, r-> opposite()) ;

		make_vertex_key(r, h-> vertex()) ;
		make_vertex_key(h, v) ;
		set_halfedge_vertex(r-> opposite(), v) ;

		set_halfedge_facet(r, h-> facet()) ;
		set_halfedge_facet(r-> opposite(), h-> opposite()-> facet()) ;

		set_tex_vertex_on_orbit(v-> halfedge(), new_tex_vertex()) ;
		set_halfedge_tex_vertex(r, r-> opposite()-> prev()-> tex_vertex()) ;


		v->set_point(p) ;
		v->halfedge()->set_tex_coord(u) ;

		if(triangulate) {
			Halfedge* first = v-> halfedge() ;
			Halfedge* cur   = first ;
			Halfedge* next  = cur-> next()-> opposite() ;
			do {
				if(!cur-> is_border()) {
					naive_triangulate_facet(cur) ;
				}
				cur = next ;
				next = next-> next()-> opposite() ;
			} while(cur != first) ;
		}

		return v ;
}

bool MapEditor::can_join_edges(Vertex* v) {
	if(v->degree() != 2) {
		return false ;
	}
	Halfedge* h = v->halfedge() ;
	if(!h->is_border() && h->facet()->nb_edges() < 4) {
		return false ;
	}
	h = h->opposite() ;
	if(!h->is_border() && h->facet()->nb_edges() < 4) {
		return false ;
	}
	return true ;
}

bool MapEditor::join_edges(Vertex* v) {

	// TODO: TexVertices

	if(!can_join_edges(v)) {
		return false ;
	}

	Halfedge* h = v->halfedge() ;
	Halfedge* r = h->next() ;
	Halfedge* z1 = r->next() ;
	Halfedge* z2 = r->opposite()->prev() ;

	make_sequence(h, z1) ;
	make_sequence(z2, h->opposite()) ;
	if(!h->is_border()) {
		make_facet_key(h) ;
	}
	if(!h->opposite()->is_border()) {
		make_facet_key(h->opposite()) ;
	}
	make_vertex_key(h) ;

	delete_vertex(h->vertex()) ;
	make_vertex_key(h, r->vertex()) ;
	delete_edge(r) ;


	return true ;
}


void MapEditor::erase_facet(Halfedge* h) {
	make_hole(h) ;
}

void MapEditor::erase_connected_component(Halfedge* h) {

	// Note: I compute a "vertices" connected component, in order
	//  to take non-manifold vertices into account. 

	std::vector<Vertex*> vertices ;
	target()->get_connected_component(h->vertex(), vertices) ;

	// Transferring into list of halfedges (note: I could
	//   do that on the vertices directly, but my initial
	//   code did that with halfedges ...

	std::vector<Halfedge*> halfedges ;
	{for(
		std::vector<Vertex*>::iterator it = vertices.begin() ;
	it != vertices.end() ; it++
		) {
			Halfedge* jt = (*it)->halfedge() ;
			do {
				halfedges.push_back(jt) ;
				jt = jt->next_around_vertex() ;
			} while(jt != (*it)->halfedge()) ;
	}}


	// Step 1 : delete vertices and facets.
	{for(
		std::vector<Halfedge*>::iterator 
		it = halfedges.begin() ; it != halfedges.end() ; it++
		) {
			Halfedge* cur = *it ;
			if(cur->vertex() != nil && cur->vertex()->halfedge() == cur) {
				delete_vertex(cur->vertex()) ;
				set_vertex_on_orbit(cur, nil) ;
			}
			if(cur->facet() != nil && cur->facet()->halfedge() == cur) {
				delete_facet(cur->facet()) ; 
				set_facet_on_orbit(cur, nil) ;
			}
	}}

	// Step 2 : delete halfedges
	{for(
		std::vector<Halfedge*>::iterator 
		it = halfedges.begin() ; it != halfedges.end() ; it++
		) {
			Halfedge* cur = *it ;
			delete_halfedge(cur) ;
	}}
}

//_____________________ additional functions __________________

bool MapEditor::can_glue(Halfedge* h0, Halfedge* h1) {

	// Checks that both Halfedges are on the border.
	if(!h0->is_border() || !h1->is_border()) {
		return false ;
	}

	// don't glue two halfedges on a same face
	if (  
		h0->opposite()->facet() == h1->opposite()->facet()
		) { 
			return false ;
	}

	// don't merge two vertices on a same halfedge
	if(
		halfedge_exists_between_vertices(
		h0->vertex(), h1->opposite()->vertex()
		) ||
		halfedge_exists_between_vertices(
		h1->vertex(), h0->opposite()->vertex()
		) 
		) {
			return false ;
	}

	if (
		!can_merge_vertices(h0,h1->opposite()) ||
		!can_merge_vertices(h1,h0->opposite())
		) {
			return false ;
	}

	return true;
}



bool MapEditor::glue(
					 Halfedge* h0, Halfedge* h1, bool do_merge_tex_vertices
					 )
{
	if (!can_glue(h0,h1)) {
		return false ;
	}

	vec3 new_p0 = Geom::barycenter(
		h0->vertex()->point(), h1->opposite()->vertex()->point() 
		) ;

	vec3 new_p1 = Geom::barycenter(
		h1->vertex()->point(), h0->opposite()->vertex()->point() 
		) ;


	// merge vertices if necessary

	Vertex* dest0 = h0->vertex() ;
	Vertex* dest1 = h1->vertex() ;

	Vertex* org0  = h0->opposite()->vertex() ;
	Vertex* org1  = h1->opposite()->vertex() ;

	if(is_locked_[dest1]) {
		is_locked_[org0] = true ;
	}

	if(is_locked_[dest0]) {
		is_locked_[org1] = true ;
	}

	if(org0 != dest1) {
		set_vertex_on_orbit(h1, org0) ;
		delete_vertex(dest1) ;
	}

	if (org1 != dest0) {
		set_vertex_on_orbit(h0, org1) ;
		delete_vertex (dest0);
	}

	// set halfedge connections

	make_sequence(h1->prev(), h0->next()) ;
	make_sequence(h0->prev(), h1->next()) ;
	make_opposite(h0->opposite(), h1->opposite()) ;
	make_vertex_key(h0->opposite()) ;
	make_vertex_key(h1->opposite()) ;

	org1->set_point(new_p0) ;
	org0->set_point(new_p1) ;

	if(do_merge_tex_vertices) {
		// TODO: something finer
		merge_tex_vertices(org0) ;
		merge_tex_vertices(org1) ;
	}

	delete_halfedge(h0);
	delete_halfedge(h1);

	return true;
}

bool MapEditor::can_unglue(Halfedge* h) {
	if(h->is_border_edge()) {
		return false ;
	}
	return (
		h->vertex()->is_on_border() ||
		h->opposite()->vertex()->is_on_border()
		) ;
}


bool MapEditor::unglue(Halfedge* h0, bool check) {
	// TODO: tex vertices : if already dissociated,
	// do not create a new Texvertex.

	if(check && !can_unglue(h0)) {
		return false ;
	}

	if(h0->is_border_edge()) {
		return false ;
	}

	Halfedge* h1 = h0->opposite() ;
	Vertex* v0 = h0->vertex() ;
	Vertex* v1 = h1->vertex() ;        

	bool v0_on_border = v0->is_on_border() ;
	bool v1_on_border = v1->is_on_border() ;

	ogf_assert(!check || (v0_on_border || v1_on_border)) ;

	Halfedge* n0 = new_halfedge(h0) ;
	Halfedge* n1 = new_halfedge(h1) ;
	make_opposite(n0, n1) ;
	set_halfedge_next(n0,n1) ;
	set_halfedge_prev(n0,n1) ;
	set_halfedge_next(n1,n0) ;
	set_halfedge_prev(n1,n0) ;

	make_opposite(h0, n0) ;
	make_opposite(h1, n1) ;

	// If v1 is on the border, find the predecessor and
	// the successor of the newly created edges, and
	// split v1 into two vertices. 
	if(v1_on_border) {
		Halfedge* next0 = h0->prev()->opposite() ;
		while(!next0->is_border()) {
			next0 = next0->prev()->opposite() ;
		}
		ogf_assert(next0 != h0) ;
		Halfedge* prev1 = h1->next()->opposite() ;
		while(!prev1->is_border()) {
			prev1 = prev1->next()->opposite() ;
		}
		ogf_assert(prev1 != h1) ;
		ogf_assert(prev1->vertex() == v1) ;
		ogf_assert(prev1->next() == next0) ;
		make_sequence(n0, next0) ;
		make_sequence(prev1, n1) ;
		set_vertex_on_orbit(n0, new_vertex(v1)) ;

		// If the TexVertices are shared, create a new one.
		if(h1->tex_vertex() == h0->prev()->tex_vertex()) {
			set_tex_vertex_on_orbit(n0, new_tex_vertex(h1->tex_vertex())) ;
		} else {
			set_halfedge_tex_vertex(n0, h0->prev()->tex_vertex()) ;
		}

		make_vertex_key(n0) ;
		make_vertex_key(h1) ;
	} else {
		set_halfedge_vertex(n0, v1) ;
		set_halfedge_tex_vertex(
			n0, n0->opposite()->prev()->tex_vertex()
			) ;
	}

	// If v0 is on the border, find the predecessor and
	// the successor of the newly created edges, and
	// split v0 into two vertices. 
	if(v0_on_border) {
		Halfedge* prev0 = h0->next()->opposite() ;
		while(!prev0->is_border()) {
			prev0 = prev0->next()->opposite() ;
		}
		ogf_assert(prev0 != h0) ;
		Halfedge* next1 = h1->prev()->opposite() ;
		while(!next1->is_border()) {
			next1 = next1->prev()->opposite() ;
		}
		ogf_assert(next1 != h1) ;
		ogf_assert(prev0->next() == next1) ;
		make_sequence(prev0, n0) ;
		make_sequence(n1, next1) ;
		set_vertex_on_orbit(n1, new_vertex(v0)) ;

		// If the TexVertices are shared, create a new one.
		if(h0->tex_vertex() == h1->prev()->tex_vertex()) {
			set_tex_vertex_on_orbit(n1, new_tex_vertex(h0->tex_vertex())) ;
		} else {
			set_halfedge_tex_vertex(n1,h1->prev()->tex_vertex()) ;
		}

		make_vertex_key(n1) ;
		make_vertex_key(h0) ;

	} else {
		set_halfedge_vertex(n1, v0) ;
		set_halfedge_tex_vertex(
			n1, n1->opposite()->prev()->tex_vertex()
			) ;
	}

	return true ;
}


bool MapEditor::can_collapse_edge(Halfedge* h) {

	{// disable glueing problems

		if ((!h->is_border() &&
			h->next()->opposite()->facet() 
			== h->prev()->opposite()->facet())
			|| (!h->opposite()->is_border() &&
			h->opposite()->next()->opposite()->facet() 
			== h->opposite()->prev()->opposite()->facet())
			)
			return false;
	}
	{// don't remove more than one vertex
		if (// it's a triangle
			h->next()->next()->next()==h            
			// the vertex is alone on border
			&& h->next()->opposite()->is_border()  
			&& h->prev()->opposite()->is_border()   
			)
			return false ;

		// the same on the other face
		if (// it's a triangle
			h->opposite()->next()->next()->next()==h            
			// the vertex is alone on border
			&& h->opposite()->next()->opposite()->is_border()
			&& h->opposite()->prev()->opposite()->is_border()
			)
			return false ;
	}

	// don't do stupid holes
	{
		if (
			(h->is_border()  && h->next()->next()->next()==h) ||
			(
			h->opposite()->is_border() && 
			h->opposite()->next()->next()->next()==h->opposite()
			)
			) { 
				return false;
		}
	}

	// don't merge holes (i.e. don't split surface)
	{
		if (
			!h->is_border() &&
			!h->opposite()->is_border() &&
			h->vertex()->is_on_border() &&
			h->opposite()->vertex()->is_on_border()
			) {
				return false;
		}
	}

	// be carefull of the toblerone case (don't remove volumes)
	{
		Halfedge* cir = h ;
		int nb_twice=0;

		std::set<Vertex*> marked ;

		// do { 
		//    cir->opposite()->vertex()->set_id(0);
		//    cir = cir->next_around_vertex();
		// } while ( cir != h);

		cir = h->opposite();
		do { 
			marked.insert(cir->opposite()->vertex()) ;
			cir = cir->next_around_vertex();
		} while ( cir != h->opposite());

		cir = h;
		do {
			if (
				marked.find(cir->opposite()->vertex()) != marked.end()
				) {
					nb_twice++;
			}
			marked.insert(cir->opposite()->vertex()) ;
			cir = cir->next_around_vertex();
		}while ( cir != h);

		if (h->next()->next()->next()==h)  {
			nb_twice--;
		}
		if (h->opposite()->next()->next()->next()==h->opposite()) { 
			nb_twice--;
		}

		if (nb_twice > 0) {
			//std::cerr<<" \nbe carefull of the toblerone case";
			return false;
		}
	}
	return true ;


	/*

	if(
	!h->next()->opposite()->is_border() ||
	!h->prev()->opposite()->is_border()
	) {
	return false ;
	}
	if(
	!h->opposite()->next()->opposite()->is_border() || 
	!h->opposite()->prev()->opposite()->is_border()
	) {
	return false ;
	}
	return true ;
	*/
}

bool MapEditor::collapse_edge(Halfedge* h) {

	if(!can_collapse_edge(h)) {
		return false ;
	}

	Vertex* dest = h->vertex() ;

	// everyone has the same vertex
	{	
		Vertex* v = h->opposite()->vertex() ;
		Halfedge* i ;
		Halfedge* ref;
		i = ref = h->opposite();
		do {
			Halfedge* local = i ;
			set_halfedge_vertex(local, dest) ;
			i = i->next_around_vertex() ;
		} while (i != ref);
		delete_vertex(v);
	}


	// remove links to current edge (facet & vertex)
	Halfedge* hDle = h;
	if ( !h->is_border() ) {
		set_facet_halfedge( hDle->facet(), hDle->next() ) ;
	}

	if (!h->opposite()->is_border()) {
		set_facet_halfedge(
			hDle->opposite()->facet(), hDle->opposite()->next() 
			) ;
	}
	set_vertex_halfedge( dest, hDle->next()->opposite() ) ;

	Halfedge* f0 = h->next() ;
	Halfedge* f1 = h->opposite()->prev() ;

	// update prev/next links
	{ 
		Halfedge* e = h->next() ;      
		make_sequence(hDle->prev(), e) ;
		e = hDle->opposite()->next() ;
		make_sequence(hDle->opposite()->prev(), e) ;
	}

	// remove null faces
	if (f0->next()->next() == f0) {
		remove_null_face(f0);
	}

	if (f1->next()->next() == f1) {
		remove_null_face(f1);
	}

	delete_edge(hDle);

	return true ;
}


//_____________________ utilities _____________________________

void MapEditor::remove_null_face(Halfedge* f0) {
	Halfedge* border = f0->next()->opposite() ;

	//remove facet
	delete_facet(f0->facet());

	// set link (fake set_opposite) to others
	make_sequence(f0, border->next()) ;
	make_sequence(border->prev(), f0) ;
	set_halfedge_facet(f0, border->facet()) ;

	// set links from others
	if (!f0->is_border()){
		make_facet_key(f0) ;
	}
	make_vertex_key(f0) ;
	make_vertex_key(f0->opposite()) ;
	delete_edge(border); 
}

bool MapEditor::can_merge_vertices(
								   Halfedge* h0, Halfedge* h1
								   ) {
									   // It's OK it they are already the same !
									   if (h0->vertex() == h1->vertex()) {
										   return true;
									   }
									   return (
										   orbits_are_compatible(h0, h1) &&
										   orbits_are_compatible(h1, h0) 
										   ) ;
} ;

bool MapEditor::orbits_are_compatible(
									  Halfedge* h0, Halfedge* h1
									  ) {

										  Halfedge* cir_h0 = h0 ;
										  do {
											  // Number of potential opposites half_edges
											  // (should not be greater than 1)
											  int nb_common = 0 ;
											  Halfedge* hh0 = cir_h0->opposite() ;
											  Halfedge* cir_h1 = h1 ;
											  do {
												  Halfedge* hh1 = cir_h1->opposite() ;
												  if (
													  hh0->vertex() == hh1->vertex() || 
													  (
													  hh0->vertex() == h0->opposite()->vertex() && 
													  hh1->vertex() == h1->opposite()->vertex()
													  ) || (
													  hh0->vertex() == h1->opposite()->vertex() && 
													  hh1->vertex() == h0->opposite()->vertex()
													  )
													  ) {
														  if ( 
															  hh0->opposite()->is_border() && hh1->is_border() || 
															  hh0->is_border() && hh1->opposite()->is_border()
															  ) {
																  // Found a potential opposite edge.
																  nb_common++ ; 
														  } else {
															  // Potential opposite edge not on the border.
															  return false ;
														  }
												  }
												  cir_h1 = cir_h1->next_around_vertex() ;
											  } while(cir_h1 != h1) ;
											  if (nb_common > 1) {
												  return false ;
											  }
											  cir_h0 = cir_h0->next_around_vertex() ;
										  } while(cir_h0 != h0) ;
										  return true ;
}

bool MapEditor::halfedge_exists_between_vertices(
	Vertex* v1, Vertex* v2
	) {
		Halfedge* cir = v1->halfedge() ;
		do {
			if(cir->opposite()->vertex() == v2) {
				return true ;
			}
			cir = cir->next_around_vertex() ;
		} while(cir != v1->halfedge()) ;
		return false ;
}


bool MapEditor::halfedges_on_same_vertex(
	Halfedge* h1, Halfedge* h2
	) {
		Halfedge* it = h1 ;
		do {
			if(it == h2) {
				return true ;
			}
			it = it->next_around_vertex() ;
		} while(it != h1) ;
		return false ;
}

bool MapEditor::halfedges_on_same_facet(
										Halfedge* h1, Halfedge* h2
										) {
											Halfedge* it = h1 ;
											do {
												if(it == h2) {
													return true ;
												}
												it = it->next() ;
											} while(it != h1) ;
											return false ;
}


bool MapEditor::vertex_is_split(const Vertex* v) {
	Halfedge* it = v->halfedge() ;
	TexVertex* tv = it->tex_vertex() ;
	it = it->next_around_vertex() ;
	while(it != v->halfedge()) {
		if(it->tex_vertex() != tv) {
			return true ;
		}
		it = it->next_around_vertex() ;
	}
	return false ;
}


void MapEditor::merge_tex_vertices(const Vertex* v_in) {
	Vertex* v = const_cast<Vertex*>(v_in) ;
	vec2 u = Geom::vertex_barycenter2d(v) ;
	set_tex_vertex_on_orbit(v->halfedge(), new_tex_vertex()) ;
	v->halfedge()->set_tex_coord(u) ;
}

void MapEditor::merge_tex_vertices(
								   const Vertex* v1, const Vertex* v2
								   ) 
{
	vec2 u1 = Geom::vertex_barycenter2d(v1) ;
	vec2 u2 = Geom::vertex_barycenter2d(v2) ;
	vec2 u  = Geom::barycenter(u1, u2) ;
	TexVertex* tv = new_tex_vertex() ;
	tv->set_tex_coord(u) ;
	set_tex_vertex_on_orbit(v1->halfedge(), tv) ;
	set_tex_vertex_on_orbit(v2->halfedge(), tv) ;
}

void MapEditor::merge_all_tex_vertices() {
	FOR_EACH_VERTEX(Map, target(), it) {
		merge_tex_vertices(it) ;
	}
}

void MapEditor::split_all_tex_vertices() {
	FOR_EACH_HALFEDGE(Map, target(), it) {
		// Note: the old ones are deallocated by 
		// reference counting.
		set_halfedge_tex_vertex(it,new_tex_vertex()) ;
	}
}


void MapEditor::naive_triangulate_facet(Halfedge* start) {
	Halfedge* cur = start->next()->next() ;
	while(cur->next() != start) {
		split_facet(start, cur) ;
		cur = cur->next()->opposite()->next() ;
	}
}


void MapEditor::naive_triangulation() {
	std::vector<Facet*> facets ;
	FOR_EACH_FACET(Map, target(), it) {
		if(!it->is_triangle()) {
			facets.push_back(it) ;
		}
	}

	std::vector<Facet*>::iterator it = facets.begin() ;
	for(; it != facets.end() ; it++) {
		naive_triangulate_facet((*it)->halfedge()) ;
	}
}

void MapEditor::center_triangulation() {
	std::vector<Facet*> facets ;
	FOR_EACH_FACET(Map, target(), it) {
		if(!it->is_triangle()) {
			facets.push_back(it) ;
		}
	}

	std::vector<Facet*>::iterator it = facets.begin() ;
	for(; it != facets.end(); it++) {
		create_center_vertex(*it) ;
	}
}

void MapEditor::reorient_facet( Map::Halfedge* first) {
	if ( first == nil) {
		return;
	}
	Map::Halfedge* last  = first;
	Map::Halfedge* prev  = first;
	Map::Halfedge* start = first;
	first = first->next();
	Map::Vertex* new_v = start->vertex() ;
	// Note: a _var is used to prevent deallocation
	MapTypes::TexVertex_var new_tv = start->tex_vertex() ;
	while (first != last) {
		Map::Vertex*  tmp_v = first->vertex() ;
		MapTypes::TexVertex_var tmp_tv = first->tex_vertex() ;
		set_halfedge_vertex(first, new_v) ;
		set_halfedge_tex_vertex(first, new_tv) ;
		set_vertex_halfedge(first->vertex(), first) ;
		new_v = tmp_v;
		new_tv = tmp_tv ;
		Map::Halfedge* next = first->next();
		set_halfedge_next(first, prev) ;
		set_halfedge_prev(first, next) ;
		prev  = first;
		first = next;
	}
	set_halfedge_vertex(start, new_v) ;
	set_halfedge_tex_vertex(start, new_tv) ;
	set_vertex_halfedge(start->vertex(), start) ;
	Map::Halfedge* next = start->next();
	set_halfedge_next(start, prev) ;
	set_halfedge_prev(start, next) ;
}


void MapEditor::inside_out(bool reorient_normal) {

	{ FOR_EACH_FACET(Map, target(), it) {
		reorient_facet(it->halfedge()) ;
	}}

	// Note: A border edge is now parallel to its opposite edge.
	// We scan all border edges for this property. If it holds, we
	// reorient the associated hole and search again until no border
	// edge with that property exists any longer. Then, all holes are
	// reoriented.

	{ FOR_EACH_HALFEDGE(Map, target(), it) {
		if ( 
			it->is_border() &&
			it->vertex() == it->opposite()->vertex()
			) {
				reorient_facet(it);
		}
	}}

	if(reorient_normal) {
		if (MapVertexNormal::is_defined(target())) { 
			MapVertexNormal vertex_normal(target()) ;
			FOR_EACH_VERTEX(Map, target(), it) 
				vertex_normal[it] = - vertex_normal[it] ;
		}
		if (MapFacetNormal::is_defined(target())) { 
			MapFacetNormal facet_normal(target()) ;
			FOR_EACH_FACET(Map, target(), it)
				facet_normal[it] = - facet_normal[it] ;
		}
	}
}

// Note: maybe this functions should not be there (map is
// lower level than map_algos, and MapEditor is in map,
// whereas MapComponent is in map_algos...).

void MapEditor::inside_out(MapComponent* comp, bool reorient_normal) {

	ogf_assert(comp->map() == target()) ;

	{ FOR_EACH_FACET(MapComponent, comp, it) {
		reorient_facet(it->halfedge()) ;
	}}

	// Note: A border edge is now parallel to its opposite edge.
	// We scan all border edges for this property. If it holds, we
	// reorient the associated hole and search again until no border
	// edge with that property exists any longer. Then, all holes are
	// reoriented.

	{ FOR_EACH_HALFEDGE(MapComponent, comp, it) {
		if ( 
			it->is_border() &&
			it->vertex() == it->opposite()->vertex()
			) {
				reorient_facet(it);
		}
	}}

	if(reorient_normal) {
		if (MapVertexNormal::is_defined(target())) { 
			MapVertexNormal vertex_normal(target()) ;
			FOR_EACH_VERTEX(MapComponent, comp, it) 
				vertex_normal[it] = - vertex_normal[it] ;
		}
		if (MapFacetNormal::is_defined(target())) { 
			MapFacetNormal facet_normal(target()) ;
			FOR_EACH_FACET(MapComponent, comp, it)
				facet_normal[it] = - facet_normal[it] ;
		}
	}

}


void MapEditor::copy_attributes(Vertex* to, Vertex* from) {
	target()->vertex_attribute_manager()->copy_record(to, from) ;
}

void MapEditor::copy_attributes(Halfedge* to, Halfedge* from) {
	target()->halfedge_attribute_manager()->copy_record(to, from) ;
}

void MapEditor::copy_attributes(Facet* to, Facet* from) {
	target()->facet_attribute_manager()->copy_record(to, from) ;
}

void MapEditor::copy_attributes(TexVertex* to, TexVertex* from) {
	target()->tex_vertex_attribute_manager()->copy_record(to, from) ;
}
