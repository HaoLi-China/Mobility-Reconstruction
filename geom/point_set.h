
#ifndef _GEOM_POINT_SET_H_
#define _GEOM_POINT_SET_H_

#include "geom_common.h"
#include "../math/attribute_adapter.h"
#include "../math/math_types.h"
#include "../basic/dlist.h"
#include "../basic/object.h"
#include "../basic/attribute.h"
#include "../image/color.h"

namespace PointSetTypes {

	/**
	* Combinatorial Element. Combel is the base class for vertices.
	*/
	class GEOM_API Combel : public Record {
	public:
		Combel()  { }
		~Combel() { }
	} ;


	class GEOM_API Vertex : public Combel {
	public:
		Vertex() { } ;
		const vec3& point() const     { return point_ ; }
		vec3& point()                 { return point_ ; }
		void set_point(const vec3& p) { point_ = p ;    }
		void set_label(unsigned int l) { label_ = l;    }
		unsigned int label() {return label_;}
	private:
		vec3 point_ ;
		unsigned int label_;
	} ;
} 

class GEOM_API PointSet : public Object 
{
public:

	// __________________ types ___________________

	typedef PointSetTypes::Vertex			Vertex ;
	typedef DList<Vertex>::iterator			Vertex_iterator ;
	typedef DList<Vertex>::const_iterator	Vertex_const_iterator ;
	typedef GenericAttributeManager<Vertex> VertexAttributeManager ;

	PointSet() ;
	virtual ~PointSet() ;

	// __________________ access ___________________

	Vertex_iterator vertices_begin()    { return vertices_.begin() ;  }
	Vertex_iterator vertices_end()      { return vertices_.end() ;    }
	Vertex_const_iterator vertices_begin() const { 
		return vertices_.begin() ;  
	}
	Vertex_const_iterator vertices_end() const { 
		return vertices_.end() ;    
	}

	int size_of_vertices() const  { return vertices_.size() ;  }

	// ___________________ attributes _______________________

	VertexAttributeManager* vertex_attribute_manager() const {
		return const_cast<VertexAttributeManager*>(&vertex_attribute_manager_) ;
	}

	// __________________ modification ______________________

	void clear() ;

	// Note: there is no VertexSetEditor or VertexSetMutator classes,
	//   possible modifications are simple and are made public here.

	Vertex* new_vertex() ;
	Vertex* new_vertex(const vec3& p) ;
	void	delete_vertex(Vertex* v) ;

private:
	DList<Vertex> vertices_ ;
	VertexAttributeManager vertex_attribute_manager_ ;        
} ;

//___________________________________________________________________

template <class T> 
class PointSetAttribute : public Attribute<PointSet::Vertex, T> {
public:
	typedef Attribute<PointSet::Vertex, T> superclass ;

	void bind(PointSet* vertex_set, const std::string& name) {
		superclass::bind(
			vertex_set->vertex_attribute_manager(), name 
			) ;
	}

	void bind(PointSet* vertex_set) { 
		superclass::bind(vertex_set->vertex_attribute_manager()) ; 
	}

	bool bind_if_defined(PointSet* vertex_set, const std::string& name) {
		return superclass::bind_if_defined(vertex_set->vertex_attribute_manager(), name) ;
	}

	PointSetAttribute() { }

	PointSetAttribute(PointSet* vertex_set) {
		bind(vertex_set) ;
	}

	PointSetAttribute(PointSet* vertex_set, const std::string& name) { 
		bind(vertex_set, name) ;
	}

	static bool is_defined(PointSet* vertex_set, const std::string& name) {
		return superclass::is_defined(
			vertex_set->vertex_attribute_manager(), name
			) ;
	}
} ;

//=====================================================================

class PointSetNormal : public PointSetAttribute<vec3> {
public:
	typedef PointSetAttribute<vec3> superclass ;
	PointSetNormal() { }
	PointSetNormal(PointSet* pset) : superclass(pset, "normal") { }
	void bind(PointSet* pset) { superclass::bind(pset, "normal");  }
	static bool is_defined(PointSet* pset) {
		return superclass::is_defined(pset, "normal") ;
	}
} ;

//______________________________________________________________

class PointSetColor : public PointSetAttribute<Color> {
public:
	typedef PointSetAttribute<Color> superclass ;
	PointSetColor() { }
	PointSetColor(PointSet* pset) : superclass(pset, "color") { }
	void bind(PointSet* pset) { superclass::bind(pset, "color");  }
	static bool is_defined(PointSet* pset) {
		return superclass::is_defined(pset, "color") ;
	}
} ;

//______________________________________________________________

class PointSetLock : public PointSetAttribute<bool> {
public:
	typedef PointSetAttribute<bool> superclass ;
	PointSetLock() { }
	PointSetLock(PointSet* vset) : superclass(vset, "lock") { }
	void bind(PointSet* vset) { superclass::bind(vset, "lock") ; }
	static bool is_defined(PointSet* vset) {
		return superclass::is_defined(vset,"lock") ;
	}
} ;

//_________________________________________________________


class PointSetAttributeAdapter : public AttributeAdapter<PointSet::Vertex> {
public:
	typedef AttributeAdapter<PointSet::Vertex> superclass ;
	bool bind_if_defined(PointSet* vertex_set, const std::string& name) {
		return superclass::bind_if_defined(vertex_set->vertex_attribute_manager(), name) ;
	}
} ;






#endif
