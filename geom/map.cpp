#include "map.h"
#include "map_geometry.h"
#include "map_attributes.h"

#include <stack>
#include <algorithm>



MapCombelObserver<Map::Vertex>::MapCombelObserver(Map* m) : map_(m) {
	map_->add_vertex_observer(this);
}

MapCombelObserver<Map::Vertex>::~MapCombelObserver() {
	map_->remove_vertex_observer(this);
}


MapCombelObserver<Map::Halfedge>::MapCombelObserver(Map* m) : map_(m) {
	map_->add_halfedge_observer(this);
}

MapCombelObserver<Map::Halfedge>::~MapCombelObserver() {
	map_->remove_halfedge_observer(this);
}


MapCombelObserver<Map::Facet>::MapCombelObserver(Map* m) : map_(m) {
	map_->add_facet_observer(this);
}

MapCombelObserver<Map::Facet>::~MapCombelObserver() {
	map_->remove_facet_observer(this);
}

//////////////////////////////////////////////////////////////////////////

Map::~Map() { 
	clear(); 
}

// __________________ stored normals ____________________


// prepare normal vectors for all faces.
void Map::compute_facet_normals() {
	MapFacetNormal normals(this) ;
	FOR_EACH_FACET_CONST(Map, this, it) {
		normals[it] = Geom::facet_normal(it);
	}
}

// prepare normal vectors for all vertices.
void Map::compute_vertex_normals() {
	MapVertexNormal normals(this);
	FOR_EACH_VERTEX_CONST(Map, this, it) {
		normals[it] = Geom::vertex_normal(it);
	}
}

// ____________________ Predicates _______________________

bool Map::is_triangulated() const {
	FOR_EACH_FACET_CONST(Map, this, it) {
		if(!it->is_triangle()) {
			return false ;
		}
	}
	return true ;
}

// ____________________ Observers ________________________

void Map::add_vertex_observer(MapCombelObserver<Vertex>* obs) {
	vertex_observers_.push_back(obs) ;
}

void Map::remove_vertex_observer(MapCombelObserver<Vertex>* obs) {
	std::vector<MapCombelObserver<Vertex>* >::iterator it = std::find(
		vertex_observers_.begin(), vertex_observers_.end(), obs
		) ;
	ogf_assert(it != vertex_observers_.end()) ;
	vertex_observers_.erase(it) ;
}

void Map::add_halfedge_observer(MapCombelObserver<Halfedge>* obs) {
	halfedge_observers_.push_back(obs)  ;
}

void Map::remove_halfedge_observer(MapCombelObserver<Halfedge>* obs) {
	std::vector<MapCombelObserver<Halfedge>* >::iterator it = std::find(
		halfedge_observers_.begin(), halfedge_observers_.end(), obs
		) ;
	ogf_assert(it != halfedge_observers_.end()) ;
	halfedge_observers_.erase(it) ;
}

void Map::add_facet_observer(MapCombelObserver<Facet>* obs) {
	facet_observers_.push_back(obs) ;
}

void Map::remove_facet_observer(MapCombelObserver<Facet>* obs) {
	std::vector<MapCombelObserver<Facet>* >::iterator it = std::find(
		facet_observers_.begin(), facet_observers_.end(), obs
		) ;
	ogf_assert(it != facet_observers_.end()) ;
	facet_observers_.erase(it) ;
}


void Map::notify_add_vertex(Vertex* v) {
	for(
		std::vector<MapCombelObserver<Vertex>* >::iterator
		it=vertex_observers_.begin(); it!=vertex_observers_.end(); it++
		) {
			(*it)->add(v) ;
	}
}

void Map::notify_remove_vertex(Vertex* v) {
	for(
		std::vector<MapCombelObserver<Vertex>* >::iterator
		it=vertex_observers_.begin(); it!=vertex_observers_.end(); it++
		) {
			(*it)->remove(v) ;
	}
}

void Map::notify_add_halfedge(Halfedge* h) {
	for(
		std::vector<MapCombelObserver<Halfedge>* >::iterator
		it=halfedge_observers_.begin(); 
	it!=halfedge_observers_.end(); it++
		) {
			(*it)->add(h) ;
	}
}

void Map::notify_remove_halfedge(Halfedge* h) {
	for(
		std::vector<MapCombelObserver<Halfedge>* >::iterator
		it=halfedge_observers_.begin(); 
	it!=halfedge_observers_.end(); it++
		) {
			(*it)->remove(h) ;
	}
}

void Map::notify_add_facet(Facet* f) {
	for(
		std::vector<MapCombelObserver<Facet>* >::iterator
		it=facet_observers_.begin(); 
	it!=facet_observers_.end(); it++
		) {
			(*it)->add(f) ;
	}
}

void Map::notify_remove_facet(Facet* f) {
	for(
		std::vector<MapCombelObserver<Facet>* >::iterator
		it=facet_observers_.begin(); 
	it!=facet_observers_.end(); it++
		) {
			(*it)->remove(f) ;
	}
}

// ____________________ Modification _____________________

void Map::clear() {
	vertices_.clear() ;
	halfedges_.clear() ;
	facets_.clear() ;

	vertex_attribute_manager_.clear() ;
	halfedge_attribute_manager_.clear() ;
	facet_attribute_manager_.clear() ;
	tex_vertex_attribute_manager_.clear() ;
}


void Map::clear_inactive_items() {
	// TODO: traverse the inactive items list, 
	//  and remove the attributes ...
	vertices_.clear_inactive_items() ;
	halfedges_.clear_inactive_items() ;
	facets_.clear_inactive_items() ;
}

// _____________________ Low level ______________________

Map::Halfedge* Map::new_edge() {
	Halfedge* h1 = new_halfedge() ;
	Halfedge* h2 = new_halfedge() ;
	h1->set_opposite(h2) ;
	h2->set_opposite(h1) ;
	h1->set_next(h2) ;
	h2->set_next(h1) ;
	h1->set_prev(h2) ;
	h2->set_prev(h1) ;
	return h1 ;
}

void Map::delete_edge(Halfedge* h) {
	delete_halfedge(h->opposite()) ;
	delete_halfedge(h) ;
}

Map::Vertex* Map::new_vertex() {
	Vertex* result = vertices_.create() ;
	vertex_attribute_manager_.new_record(result) ;
	notify_add_vertex(result) ;
	return result ;
}

Map::Vertex* Map::new_vertex(const Map::Vertex* rhs) {
	Vertex* result = vertices_.create() ;
	result->set_point(rhs->point()) ;
	vertex_attribute_manager_.new_record(result, rhs) ;
	notify_add_vertex(result) ;
	return result ;
}

Map::Halfedge* Map::new_halfedge() {
	Halfedge* result = halfedges_.create() ;
	halfedge_attribute_manager_.new_record(result) ;
	notify_add_halfedge(result) ;
	return result ;
}

Map::Halfedge* Map::new_halfedge(const Map::Halfedge* rhs) {
	Halfedge* result = halfedges_.create() ;
	halfedge_attribute_manager_.new_record(result, rhs) ;
	notify_add_halfedge(result) ;
	return result ;
}

Map::Facet* Map::new_facet() {
	Facet* result = facets_.create() ;
	facet_attribute_manager_.new_record(result) ;
	notify_add_facet(result) ;
	return result ;
}

Map::Facet* Map::new_facet(const Map::Facet* rhs) {
	Facet* result = facets_.create() ;
	facet_attribute_manager_.new_record(result, rhs) ;
	notify_add_facet(result) ;
	return result ;
}

Map::TexVertex* Map::new_tex_vertex() {
	// For the moment, TexVertex does Attribute management
	// (it keeps a pointer to the AttributeManager)
	TexVertex* result = new TexVertex(tex_vertex_attribute_manager()) ;
	return result ;
}

Map::TexVertex* Map::new_tex_vertex(const Map::TexVertex* rhs) {
	// For the moment, TexVertex does Attribute management
	// (it keeps a pointer to the AttributeManager)
	TexVertex* result = new TexVertex(tex_vertex_attribute_manager(),rhs) ;
	return result ;
}

void Map::delete_vertex(Vertex* v) {
	notify_remove_vertex(v) ;
	vertex_attribute_manager_.delete_record(v) ;
	vertices_.destroy(v) ;
}

void Map::delete_halfedge(Halfedge* h) {
	notify_remove_halfedge(h) ;
	halfedge_attribute_manager_.delete_record(h) ;
	halfedges_.destroy(h) ;
}

void Map::delete_facet(Facet* f) {
	notify_remove_facet(f) ;
	facet_attribute_manager_.delete_record(f) ;
	facets_.destroy(f) ;
}

void Map::delete_tex_vertex(TexVertex* tv) {
	// For the moment, TexVertex does Attribute management
	// (it keeps a pointer to the AttributeManager)
	delete tv ;
}

void Map::activate_vertex(Vertex* v) {
	notify_add_vertex(v) ;
	vertices_.activate(v) ;
}

void Map::activate_halfedge(Halfedge* h) {
	notify_add_halfedge(h) ;
	halfedges_.activate(h) ;
}

void Map::activate_facet(Facet* f) {
	notify_add_facet(f) ;
	facets_.activate(f) ;
}

void Map::deactivate_vertex(Vertex* v) {
	notify_remove_vertex(v) ;
	vertices_.deactivate(v) ;
}

void Map::deactivate_halfedge(Halfedge* h) {
	notify_remove_halfedge(h) ;
	halfedges_.deactivate(h) ;
}

void Map::deactivate_facet(Facet* f) {
	notify_remove_facet(f) ;
	facets_.deactivate(f) ;
}

void Map::get_connected_component(Vertex* h, std::vector<Vertex*>& l) 
{
	Attribute<Vertex, bool> visited(vertex_attribute_manager()) ;

	std::stack<Vertex*> stack ;
	stack.push(h) ;

	while(!stack.empty()) {
		Vertex* top = stack.top() ;
		stack.pop() ;
		if(!visited[top]) {
			visited[top] = true ;
			l.push_back(top) ;
			Halfedge* cir = top->halfedge() ;
			do {
				Vertex* cur = cir->opposite()->vertex() ;
				if(!visited[cur]) {
					stack.push(cur) ;
				}
				cir = cir->next_around_vertex() ;
			} while(cir != top->halfedge()) ;
		}
	}
}

bool Map::is_valid() const {
	bool ok = true ;
	{ FOR_EACH_VERTEX_CONST(Map, this, it) {
		ok = ok && it->is_valid() ;
	}}
	{ FOR_EACH_HALFEDGE_CONST(Map, this, it) {
		ok = ok && it->is_valid() ;
	}}
	{ FOR_EACH_FACET_CONST(Map, this, it) {
		ok = ok && it->is_valid() ;
	}}
	return ok ;
}


void Map::assert_is_valid() const {
	{ FOR_EACH_VERTEX_CONST(Map, this, it) {
		it->assert_is_valid() ;
	}}
	{ FOR_EACH_HALFEDGE_CONST(Map, this, it) {
		it->assert_is_valid() ;
	}}
	{ FOR_EACH_FACET_CONST(Map, this, it) {
		it->assert_is_valid() ;
	}}
}

//_________________________________________________________

Map::Halfedge* MapMutator::new_edge(Map::Halfedge* rhs) {
	Halfedge* h1 = new_halfedge(rhs) ;
	Halfedge* h2 = new_halfedge(rhs->opposite()) ;
	h1->set_opposite(h2) ;
	h2->set_opposite(h1) ;
	h1->set_next(h2) ;
	h2->set_next(h1) ;
	h1->set_prev(h2) ;
	h2->set_prev(h1) ;
	return h1 ;
}

void MapMutator::set_vertex_on_orbit(Halfedge* h, Vertex* v) {
	Halfedge* it = h ;
	do {
		it->set_vertex(v) ;
		it = it->next_around_vertex() ;
	} while(it != h) ;
}


void MapMutator::set_tex_vertex_on_orbit(Halfedge* h, TexVertex* tv) {
	Halfedge* it = h ;
	do {
		it->set_tex_vertex(tv) ;
		it = it->next_around_vertex() ;
	} while(it != h) ;
}

void MapMutator::set_facet_on_orbit(Halfedge* h, Facet* f) {
	Halfedge* it = h ;
	do {
		it->set_facet(f) ;
		it = it->next() ;
	} while(it != h) ;
}


