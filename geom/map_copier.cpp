#include "map_copier.h"
#include "map_builder.h"



MapCopier::MapCopier() { 
	copy_all_attributes_ = false ;
	current_source_ = nil ;
	current_destination_ = nil ;
	// By default: copy vertex locks
	declare_vertex_attribute_to_copy("lock") ;
}

MapCopier::~MapCopier() { 
}        

void MapCopier::copy(
					 MapBuilder& builder, MapComponent* component, 
					 MapVertexAttribute<int>& vertex_id, MapTexVertexAttribute<int>& tex_vertex_id, 
					 int& cur_vertex_id, int& cur_tex_vertex_id
					 )
{

	bind_attribute_copiers(builder.target(), component->map()) ;

	// Step 1 : clear vertex and tex vertex ids
	FOR_EACH_VERTEX(MapComponent, component, it) {
		vertex_id[it] = -1 ;
	}

	FOR_EACH_HALFEDGE(MapComponent, component, it) {
		tex_vertex_id[it->tex_vertex()] = -1 ;
	}

	// Step 2: enumerate vertices
	FOR_EACH_VERTEX(MapComponent, component, it) {
		vertex_id[it] = cur_vertex_id ;
		builder.add_vertex(it->point()) ;
		copy_vertex_attributes(builder.current_vertex(), it) ;
		cur_vertex_id++ ;
	}

	// Step 3: enumerate tex vertices
	FOR_EACH_HALFEDGE(MapComponent, component, it) {
		if(tex_vertex_id[it->tex_vertex()] == -1) {
			tex_vertex_id[it->tex_vertex()] = cur_tex_vertex_id ;
			builder.add_tex_vertex(it->tex_vertex()->tex_coord()) ;
			copy_tex_vertex_attributes(builder.current_tex_vertex(), it->tex_vertex()) ;
			cur_tex_vertex_id++ ;
		}
	}

	// Step 4: create facets
	FOR_EACH_FACET(MapComponent, component, it) {
		Map::Halfedge* h = it->halfedge() ;
		builder.begin_facet() ;
		do {
			builder.add_vertex_to_facet(vertex_id[h->vertex()]) ;
			builder.set_corner_tex_vertex(tex_vertex_id[h->tex_vertex()]) ;
			h = h->next() ;
		} while(h != it->halfedge()) ;
		builder.end_facet() ;
		copy_facet_attributes(builder.current_facet(), it) ;
	}
	// TODO: copy halfedge attributes
}

void MapCopier::copy(
					 MapBuilder& builder, Map* source,
					 MapVertexAttribute<int>& vertex_id, MapTexVertexAttribute<int>& tex_vertex_id, 
					 int& cur_vertex_id, int& cur_tex_vertex_id
					 )
{
	bind_attribute_copiers(builder.target(), source) ;

	// Step 1 : clear vertex and tex vertex ids
	FOR_EACH_VERTEX(Map, source, it) {
		vertex_id[it] = -1 ;
	}
	FOR_EACH_HALFEDGE(Map, source, it) {
		tex_vertex_id[it->tex_vertex()] = -1 ;
	}

	// Step 2: enumerate vertices
	FOR_EACH_VERTEX(Map, source, it) {
		vertex_id[it] = cur_vertex_id ;
		builder.add_vertex(it->point()) ;
		copy_vertex_attributes(builder.current_vertex(), it) ;
		cur_vertex_id++ ;
	}


	// Step 3: enumerate tex vertices
	FOR_EACH_HALFEDGE(Map, source, it) {
		if(tex_vertex_id[it->tex_vertex()] == -1) {
			tex_vertex_id[it->tex_vertex()] = cur_tex_vertex_id ;
			builder.add_tex_vertex(it->tex_vertex()->tex_coord()) ;
			copy_tex_vertex_attributes(builder.current_tex_vertex(), it->tex_vertex()) ;
			cur_tex_vertex_id++ ;
		}
	}

	// Step 4: create facets
	FOR_EACH_FACET(Map, source, it) {
		Map::Halfedge* h = it->halfedge() ;
		builder.begin_facet() ;
		do {
			builder.add_vertex_to_facet(vertex_id[h->vertex()]) ;
			builder.set_corner_tex_vertex(tex_vertex_id[h->tex_vertex()]) ;
			h = h->next() ;
		} while(h != it->halfedge()) ;
		builder.end_facet() ;
		copy_facet_attributes(builder.current_facet(), it) ;
		// TODO: copy halfedge attributes
	}
}

template <class RECORD> inline void bind_attribute_copiers(
	std::vector< AttributeCopier<RECORD> >& copiers,
	AttributeManager* to, AttributeManager* from,
	const std::set<std::string>& attributes_to_copy,
	bool copy_all_attributes
	) {
		copiers.clear() ;
		std::vector<std::string> names ;
		from->list_named_attributes(names) ;
		for(unsigned int i=0; i<names.size(); i++) {
			if(copy_all_attributes || (attributes_to_copy.find(names[i]) != attributes_to_copy.end())) {
				bind_source(copiers, from, names[i]) ;
			}
		}
		bind_destinations(copiers, to) ;
}

void MapCopier::bind_attribute_copiers(Map* destination, Map* source) {
	::bind_attribute_copiers(
		vertex_attribute_copiers_,
		destination->vertex_attribute_manager(), source->vertex_attribute_manager(), 
		vertex_attributes_to_copy_,
		copy_all_attributes_
		) ;

	::bind_attribute_copiers(
		tex_vertex_attribute_copiers_,
		destination->tex_vertex_attribute_manager(), source->tex_vertex_attribute_manager(), 
		tex_vertex_attributes_to_copy_,
		copy_all_attributes_
		) ;

	::bind_attribute_copiers(
		halfedge_attribute_copiers_,
		destination->halfedge_attribute_manager(), source->halfedge_attribute_manager(), 
		halfedge_attributes_to_copy_,
		copy_all_attributes_
		) ;

	::bind_attribute_copiers(
		facet_attribute_copiers_,
		destination->facet_attribute_manager(), source->facet_attribute_manager(), 
		facet_attributes_to_copy_,
		copy_all_attributes_
		) ;

}

