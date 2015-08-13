#include "point_set_geometry.h"
#include "point_set.h"
#include "iterators.h"
#include "../basic/logger.h"
#include "../basic/attribute_copier.h"	


namespace Geom {



	template <class RECORD> 
	inline void bind_attribute_copiers(
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

	PointSet* duplicate(const PointSet* from) {
		if (!from)
			return nil;

		PointSet* to = new PointSet;
		if (to) {
			// initialize this if you want to copy only specified attributes
			std::set<std::string> attributes_to_copy ;

			// set true if you want to copy all attributes defined on source
			bool copy_all_attributes = true;

			//-----------------------------------------------------------------------
			// step 1: setup attribute copiers
			std::vector<std::string> names ;
			from->vertex_attribute_manager()->list_named_attributes(names) ;
			std::vector< AttributeCopier<PointSet::Vertex> >	copiers;
			for(unsigned int i=0; i<names.size(); i++) {
				if(copy_all_attributes || (attributes_to_copy.find(names[i]) != attributes_to_copy.end())) {
					bind_source(copiers, from->vertex_attribute_manager(), names[i]) ;
				}
			}
			bind_destinations(copiers, to->vertex_attribute_manager()) ;

			//-----------------------------------------------------------------------
			// step 2: do copy while constructing the point set
			FOR_EACH_VERTEX_CONST(PointSet, from, it) {
				const PointSet::Vertex* s = it;
				PointSet::Vertex* t = to->new_vertex(s->point()) ; 
				copy_attributes(copiers, t, const_cast<PointSet::Vertex*>(s)) ;
			}
		}

		return to;
	}


    Box3d bounding_box(const PointSet* pset) {
		ogf_assert(pset->size_of_vertices() > 0);
		Box3d result ;
		FOR_EACH_VERTEX_CONST(PointSet, pset, it) {
			result.add_point(it->point()) ;
		}
		return result ;
	}


}
