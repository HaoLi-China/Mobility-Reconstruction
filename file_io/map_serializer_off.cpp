
#include "map_serializer_off.h"
#include "../basic/logger.h"
#include "../geom/map_builder.h"
#include "../geom/map_enumerator.h"


MapSerializer_off::MapSerializer_off() 
{
	read_supported_ = true ;
	write_supported_ = true ;
}

bool MapSerializer_off::do_read(
								std::istream& in, AbstractMapBuilder& builder) 
{
	// Vertex index starts by 0 in off format.
	std::string magic ;
	in >> magic ;

	// NOFF is for Grimage "visual shapes".
	if(magic != "OFF" && magic != "NOFF") {
		Logger::err("MapSerializer") 
			<< "File is not an OFF file" << std::endl ;
		return false;
	}

	int nb_vertices, nb_edges, nb_facets ;
	in >> nb_vertices >> nb_facets >> nb_edges ;

	builder.begin_surface();

	for(int i=0; i<nb_vertices; i++) {
		vec3 p ;
		in >> p ;
		builder.add_vertex(p) ;
	}

	for(int i=0; i<nb_facets; i++) {
		int nb_vertices ;
		in >> nb_vertices ;

		builder.begin_facet();
		for(int j=0; j<nb_vertices; j++) {
			int index ;
			in >> index ;
			builder.add_vertex_to_facet(index);
		}
		builder.end_facet();
	}

	builder.end_surface();

	return true;
}

bool MapSerializer_off::do_write(
								 std::ostream& out, const Map* mesh) const
{
	out << "OFF" << std::endl ;
	out << mesh->size_of_vertices() << " " 
		<< mesh->size_of_facets() << " "
		<< mesh->size_of_halfedges() / 2
		<< std::endl ;

	// Off files numbering starts with 0
	Attribute<Vertex, int> vertex_id(mesh->vertex_attribute_manager());
	MapEnumerator::enumerate_vertices(const_cast<Map*>(mesh), vertex_id, 0);

	// Output Vertices
	FOR_EACH_VERTEX_CONST(Map, mesh, it) {
		out << it->point() << std::endl ;
	} 

	// Output facets
	FOR_EACH_FACET_CONST(Map, mesh, it) {
		out << it->nb_vertices() << " " ;
		Map::Halfedge* jt = it->halfedge() ;
		do {
			out << vertex_id[jt->vertex()]
			<< " " ;
			jt = jt->next() ;
		} while(jt != it->halfedge()) ;
		out << std::endl ;
	}

	return true ;
}


