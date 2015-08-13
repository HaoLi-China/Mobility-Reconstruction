
#include "map_editor_ext.h"
#include "map_geometry.h"
#include "../basic/logger.h"



bool MapEditorExt::zip_edge(Map::Vertex* src) {
	Map::Halfedge* h1 = nil ;
	Map::Halfedge* h2 = nil ;
	Map::Halfedge* it = src->halfedge() ;
	do {
		if(it->is_border()) {
			if(h1 == nil) {
				h1 = it ;
			} else {
				h2 = it ;
			}
		}
		if(it->opposite()->is_border()) {
			if(h1 == nil) {
				h1 = it->opposite() ;
			} else {
				h2 = it->opposite() ;
			}
		}
		it = it->next_around_vertex() ;
	} while(it != src->halfedge()) ;

	if(h1 != nil && h2 != nil) {
		glue(h1, h2) ;
		return true ;
	} 
	else
		return false;
}


void MapEditorExt::compute_normals_around_vertex(Map::Vertex* v) {
	if (MapVertexNormal::is_defined(target())) {
		MapVertexNormal normals(target()) ;
		normals[v] = Geom::vertex_normal(v);

		Map::Halfedge* h = v->halfedge() ;
		do {
			normals[h->opposite()->vertex()] = Geom::vertex_normal(h->opposite()->vertex()) ;
			h = h->next_around_vertex() ;
		} while(h != v->halfedge()) ;
	}

	if(MapFacetNormal::is_defined(target())) {
		MapFacetNormal normals(target()) ;
		Map::Halfedge* h = v->halfedge() ;
		do {
			if(h->facet() != nil)
				normals[h->facet()] = Geom::facet_normal(h->facet()) ;
			h = h->next_around_vertex() ;
		} while(h != v->halfedge()) ;
	}
}

void MapEditorExt::compute_normals_around_facet(Map::Facet* f) {
	if(MapFacetNormal::is_defined(target())) {
		MapFacetNormal normals(target()) ;
		normals[f] = Geom::facet_normal(f) ;
	}     

	if (MapVertexNormal::is_defined(target())) {
		MapVertexNormal normals(target()) ;
		Map::Halfedge* h = f->halfedge() ;
		do {
			normals[h->vertex()] = Geom::vertex_normal(h->vertex()) ;
			h = h->next() ;
		} while(h != f->halfedge()) ;
	}
}

void MapEditorExt::compute_normals_around_edge(Map::Halfedge* h) {
	if (MapVertexNormal::is_defined(target())) {
		MapVertexNormal normals(target()) ;
		normals[h->vertex()] = Geom::vertex_normal(h->vertex()) ;
		normals[h->prev()->vertex()] = Geom::vertex_normal(h->prev()->vertex()) ;
	}

	if(MapFacetNormal::is_defined(target())) {
		MapFacetNormal normals(target()) ;
		if(h->facet() != nil) {
			normals[h->facet()] = Geom::facet_normal(h->facet()) ;
		}
		if(h->opposite()->facet() != nil) {
			normals[h->opposite()->facet()] = Geom::facet_normal(h->opposite()->facet()) ;
		}
	}     
}


//_____________________________________________________________________________

//----------------------------- utilities for split_facet ------------------

static bool halfedge_exists_between(Map::Vertex* v1, Map::Vertex* v2) {
	Map::Halfedge* h = v1->halfedge() ;
	do {
		if(h->opposite()->vertex() == v2) {
			return true ;
		}
		h = h->next_around_vertex() ;
	} while(h != v1->halfedge()) ;
	return false ;
}

typedef std::vector<Map::Vertex*> VertexLoop ;
static void split_loop(
					   const VertexLoop& loop, Map::Vertex*& v1, Map::Vertex*& v2,
					   Map::Facet* f
					   ) 
{
	std::vector<double> s ;
	s.push_back(0.0) ;
	double cur_s = 0 ;
	{ for(unsigned int i=1; i<loop.size(); i++) {
		cur_s += length(loop[i]->point() - loop[i-1]->point()) ;
		s.push_back(cur_s) ;
	}}
	double total_length = cur_s ;
	total_length += length(loop[loop.size() - 1]->point() - loop[0]->point()) ;

	double best_rij = Numeric::big_double ;
	v1 = nil ; 
	v2 = nil ;

	for(unsigned int i=0; i<loop.size(); i++) {
		for(unsigned int j=0; j<loop.size(); j++) {

			// Do not split using vertices
			// already connected by an edge.
			if((i == j) || halfedge_exists_between(loop[i], loop[j])) {
				continue ;
			}

			double dsij = ogf_min(s[i] - s[j], total_length - (s[i] - s[j])) ;
			double dxij = length(loop[i]->point() - loop[j]->point()) ;
			dsij = ogf_max(dsij, 1e-6) ;
			dxij = ogf_max(dxij, 1e-6) ;
			double rij = dxij / dsij ;
			if(rij < best_rij) {
				best_rij = rij ;
				v1 = loop[i] ;
				v2 = loop[j] ;
			}
		}
	}
}

static Map::Halfedge* split_facet(Map* map, Map::Facet* f) {
	VertexLoop loop ;
	Map::Halfedge* h = f->halfedge() ; 
	do {
		loop.push_back(h->vertex()) ;
		h = h->next() ;
	} while(h != f->halfedge()) ;
	Map::Vertex* v1 ;
	Map::Vertex* v2 ;
	split_loop(loop, v1, v2, f) ;

	if(v1 == nil || v2 == nil) {
		return nil ;
	}

	Map::Halfedge* h1 = v1->halfedge() ;
	while(h1->facet() != f) {
		h1 = h1->next_around_vertex() ;
	}
	Map::Halfedge* h2 = v2->halfedge() ;
	while(h2->facet() != f) {
		h2 = h2->next_around_vertex() ;
	}

	MapEditor editor(map) ;
	editor.split_facet(h1, h2) ;
	return h1->next() ;
}

void MapEditorExt::subdivide_facet(Map::Facet* f, int max_nb_vertices) {
	int nb_vertices = f->nb_vertices() ;
	if(f->nb_vertices() <= max_nb_vertices) {
		return ;
	}
	Map::Halfedge* h = ::split_facet(target(), f) ;
	if(h != nil) {
		Map::Facet* f1 = h->facet() ;
		Map::Facet* f2 = h->opposite()->facet() ;

		if(
			f1->nb_vertices() >= nb_vertices 
			) {
				MapVertexLock is_locked(target()) ;
				Map::Halfedge* h = f1->halfedge() ;
				do {
					is_locked[h->vertex()] = true ;
					h = h->next() ;
				} while(h != f1->halfedge()) ;
				Logger::warn("MapEditorExt") << "encountered strange topology" << std::endl ;
				return ;
		}

		if(
			f2->nb_vertices() >= nb_vertices 
			) {
				MapVertexLock is_locked(target()) ;
				Map::Halfedge* h = f2->halfedge() ;
				do {
					is_locked[h->vertex()] = true ;
					h = h->next() ;
				} while(h != f2->halfedge()) ;
				Logger::warn("MapEditorExt") << "encountered strange topology" << std::endl ;
				return ;
		}

		subdivide_facet(f1, max_nb_vertices) ;
		subdivide_facet(f2, max_nb_vertices) ;
	}
}

//_________________________________________________________

Map::Facet* MapEditorExt::create_facet_between_edges(
	Map::Halfedge* h1, Map::Halfedge* h2
	) {
		// both are border edges
		ogf_assert(h1->is_border() && h2->is_border());
		// they are not successive
		ogf_assert(h1->next() != h2 && h2->next() != h1);

		// Liangliang: test for degeneration case. E.g., two edges share the same 
		// geometry (this usually happens in a seam or a cut path).
		if ((distance2(h1->vertex()->point(), h2->opposite()->vertex()->point()) < 1e-15) ||
			(distance2(h2->vertex()->point(), h1->opposite()->vertex()->point()) < 1e-15)) {
				Logger::warn("MapEditorExt") << "facet with two identical vertices (ignored) " << std::endl ;
				return nil;
		}

		Halfedge* k2 = make_polygon(4) ;
		Facet* f = k2->facet();

		if(!k2->is_border()) {
			k2 = k2->opposite() ;
		}

		k2->vertex()->set_point(h2->prev()->vertex()->point()) ;
		k2->prev()->vertex()->set_point(h2->vertex()->point()) ;

		Map::Halfedge* k1 = k2->next()->next() ;

		k1->vertex()->set_point(h1->prev()->vertex()->point()) ;
		k1->prev()->vertex()->set_point(h1->vertex()->point()) ;

		Map::VertexAttributeManager * mgr = target()->vertex_attribute_manager() ;
		mgr->copy_record(k1->vertex(), h1->prev()->vertex()) ;
		mgr->copy_record(k1->prev()->vertex(), h1->vertex()) ;
		mgr->copy_record(k2->vertex(), h2->prev()->vertex()) ;
		mgr->copy_record(k2->prev()->vertex(), h2->vertex()) ;		

		Map::Halfedge* k1n = k1->next();
		Map::Halfedge* k2n = k2->next();

		glue(h2, k2) ;
		glue(h1, k1) ;

		if(k1n->next()->next() == k1n) {
			zip_edge(k1n->vertex()) ;
		}

		if(k2n->next()->next() == k2n) {
			zip_edge(k2n->vertex()) ;
		}

		return f ;
}

