
#include "map_serializer_ply2.h"
#include "../geom/map_builder.h"
#include "../basic/logger.h"
#include "../geom/map_enumerator.h"


MapSerializer_ply2::MapSerializer_ply2() 
{
	read_supported_ = true ;
	write_supported_ = true ;
}

bool MapSerializer_ply2::do_read(
								 std::istream& input, AbstractMapBuilder& builder)
{
	int nb_vertices, nb_facets ;
	input >> nb_vertices >> nb_facets ;

	builder.begin_surface();

	for(int i = 0; i<nb_vertices; i++) {
		double x,y,z ;
		input >> x >> y >> z ;
		builder.add_vertex( vec3(x,y,z) );
	}

	for(int i=0; i<nb_facets; i++) {
		int nb ;
		input >> nb ;

		builder.begin_facet();
		for(int i=0; i<nb; i++) {
			int p ;
			input >> p ;
			builder.add_vertex_to_facet(p);
		}

		builder.end_facet();
	}

	builder.end_surface();

	return true;
}

bool MapSerializer_ply2::do_write(
								  std::ostream& out, const Map* mesh) const
{
	out << mesh->size_of_vertices() << std::endl;
	out << mesh->size_of_facets()   << std::endl;

	// ply2 files numbering starts with 0
	Attribute<Vertex, int> vertex_id(mesh->vertex_attribute_manager());
	MapEnumerator::enumerate_vertices(const_cast<Map*>(mesh), vertex_id, 0);

	// Output vertices
	FOR_EACH_VERTEX_CONST(Map, mesh, it) {
		out << it->point() << std::endl ;
	} 

	// Output facets
	FOR_EACH_FACET_CONST(Map, mesh, it) {
		out << it->nb_vertices() << " " ;
		Map::Halfedge* jt = it->halfedge() ;
		do {
			out << vertex_id[jt->vertex()] << " " ;
			jt = jt->next() ;
		} while(jt != it->halfedge()) ;
		out << std::endl ;
	}

	return true ;
}

