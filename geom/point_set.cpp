
#include "point_set.h"


PointSet::PointSet() {
}

PointSet::~PointSet() {
}

void PointSet::clear() {
	vertices_.clear() ;
	vertex_attribute_manager_.clear() ;
}


PointSet::Vertex* PointSet::new_vertex() {
	Vertex* result = vertices_.create() ;
	vertex_attribute_manager_.new_record(result) ;
	// TODO: notify_add_vertex(result) if we
	// add VertexObserver
	return result ;
}

PointSet::Vertex* PointSet::new_vertex(const vec3& p) {
	Vertex* result = new_vertex() ;
	result->set_point(p) ;
	return result ;
}

void PointSet::delete_vertex(Vertex* v) {
	// TODO: notify_remove_vertex(v) if we
	// add VertexObserver
	vertex_attribute_manager_.delete_record(v) ;
	vertices_.destroy(v) ;
}




