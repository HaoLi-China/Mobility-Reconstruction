
#ifndef _GEOM_MAP_TYPES_H_
#define _GEOM_MAP_TYPES_H_

#include "geom_common.h"
#include "../basic/counted.h"
#include "../basic/smart_pointer.h"
#include "../basic/record_id.h"
#include "../basic/attribute_manager.h"
#include "../math/math_types.h"


class Map ;
class MapMutator ;

namespace MapTypes {

	class Vertex ;
	class Halfedge ;
	class Facet ;
	class MapMutator ;

	//___________________________________________________

	/**
	* Combinatorial Element. Combel is the base class for 
	* vertices, half-edges and polygons.
	*/
	class GEOM_API Combel : public Record {
	public:
		Combel()  { }
		~Combel() { }
	} ;

	//___________________________________________________

	/**
	* Texture coordinates can be shared between several corners, or independant.
	*/

	// Note: for the moment, each tex vertex keeps a pointer to the tex vertex 
	// attribute manager, to enable a correct management of the attributes. This 
	// will be improved in the future.

	class GEOM_API TexVertex : public Counted, public Combel {
	public:
		TexVertex(GenericAttributeManager<TexVertex>* attr_mngr) :
		  attribute_manager_(attr_mngr) { 
			  attribute_manager_->new_record(this) ;
		  }

		  TexVertex(
			  GenericAttributeManager<TexVertex>* attr_mngr, const TexVertex* rhs
			  ) : tex_coord_(rhs->tex_coord()), attribute_manager_(attr_mngr) { 
				  attribute_manager_->new_record(this, rhs) ;
		  }

		  ~TexVertex() { 
			  attribute_manager_->delete_record(this) ;
			  attribute_manager_ = nil ;
		  }
		  const vec2& tex_coord() const  { return tex_coord_ ; }
		  vec2& tex_coord()              { return tex_coord_ ; }
		  void set_tex_coord(const vec2& p) { tex_coord_ = p ; }

	private:
		vec2 tex_coord_ ;
		GenericAttributeManager<TexVertex>* attribute_manager_ ;
	} ;

	typedef SmartPointer<TexVertex> TexVertex_var ;

	//___________________________________________________


	/**
	* A vertex of a Map. Each Vertex has a geometry (i.e. a vec3)
	*/
	class GEOM_API Vertex : public Combel {
	public:
		Vertex() : halfedge_(nil) {  }
		Vertex(const vec3& p) : halfedge_(nil), point_(p) {  }
		~Vertex() { halfedge_ = nil ; }

		const vec3& point() const     { return point_ ; }
		vec3& point()                 { return point_ ; }
		void set_point(const vec3& p) { point_ = p ;    }

		Halfedge* halfedge() const { return halfedge_ ; }

		bool is_valid() const ;
		void assert_is_valid() const ;

		unsigned int degree() const ;
		bool is_on_border() const ;

		/* Returns true if "this" and v are in each other's one-rings */
		bool is_connected(const Vertex* v) const;

	protected:
		void set_halfedge(Halfedge* h) { halfedge_ = h ; }
		friend class ::Map ;
		friend class ::MapMutator ;

	private:
		Halfedge* halfedge_ ;
		vec3 point_ ;
	} ;

	//______________________________________________

	/**
	* Each edge of a Map is composed of two Halfedges.
	*/

	class GEOM_API Halfedge : public Combel {
	public:
		Halfedge() : 
		  opposite_(nil), next_(nil), 
			  prev_(nil), facet_(nil), vertex_(nil) {
		  }
		  ~Halfedge() { 
			  opposite_ = nil ; next_ = nil ;
			  prev_ = nil ; facet_ = nil ; vertex_ = nil ;
		  }

		  TexVertex* tex_vertex() const { return tex_vertex_ ; }

		  const vec2& tex_coord() const { 
			  return tex_vertex()->tex_coord() ; 
		  }

		  vec2& tex_coord() { 
			  return tex_vertex()->tex_coord() ; 
		  }

		  void set_tex_coord(const vec2& tex_coord_in) {
			  tex_vertex()->set_tex_coord(tex_coord_in) ; 
		  }

		  Halfedge* opposite() const { return opposite_ ; }
		  Halfedge* next() const { return next_ ; }
		  Halfedge* prev() const { return prev_ ; }

		  Halfedge* next_around_vertex() const {
			  return opposite()->prev() ;
		  }
		  Halfedge* prev_around_vertex() const {
			  return next()->opposite() ;
		  }

		  Facet*  facet() const { return facet_ ; }
		  Vertex* vertex() const { return vertex_ ; }

		  bool is_border() const { return facet_ == nil ; }
		  bool is_border_edge() const { 
			  return is_border() || opposite()->is_border() ; 
		  }

		  /** One halfedge per facet exactly is the facet key. */
		  bool is_facet_key() const ;

		  /** One halfedge per vertex exactly is the vertex key. */
		  bool is_vertex_key() const ;

		  /** 
		  * One halfedge per edge exactly is the edge key. 
		  * Note: this can be used for loops, to traverse one halfedge 
		  * per edge exactly (for instance, to draw the mesh).
		  */
		  bool is_edge_key() const ;
		  Halfedge* edge_key() const { 
			  return is_edge_key() ? const_cast<Halfedge*>(this) : opposite() ; 
		  }

		  bool is_valid() const ;
		  void assert_is_valid() const ;

	protected:
		void set_opposite(Halfedge* h) { opposite_ = h; }
		void set_next(Halfedge* h) { next_ = h; }
		void set_prev(Halfedge* h) { prev_ = h; }
		void set_facet(Facet* f) { facet_ = f ; }
		void set_vertex(Vertex* v) { vertex_ = v ; }
		void set_tex_vertex(TexVertex* t) { tex_vertex_ = t ; }

		friend class ::Map ;
		friend class ::MapMutator ;

	private:
		Halfedge* opposite_ ;
		Halfedge* next_ ;
		Halfedge* prev_ ;
		Facet* facet_ ;
		Vertex* vertex_ ;
		TexVertex_var tex_vertex_ ;
	} ;

	//______________________________________________


	/**
	* A Facet of a Map.
	*/

	class GEOM_API Facet : public Combel {
	public:
		Facet() : halfedge_(nil) { }
		~Facet() { halfedge_ = nil ; }

		Halfedge* halfedge() const { return halfedge_ ; }

		int degree() const ;
		int nb_edges() const { return degree() ; }
		int nb_vertices() const { return degree() ; }

		bool is_on_border() const ;
		bool is_triangle() const ;
		bool is_valid() const ;

		void assert_is_valid() const ;

	protected:
		void set_halfedge(Halfedge* h) { halfedge_ = h ; }
		friend class ::Map ;
		friend class ::MapMutator ;

	private:
		Halfedge* halfedge_ ;
	} ;

	//_________________________________________________________

	inline bool Halfedge::is_facet_key() const {
		return (facet_->halfedge() == this) ;
	}

	inline bool Halfedge::is_vertex_key() const {
		return (vertex_->halfedge() == this) ;
	}

	inline bool Halfedge::is_edge_key() const {
		// TODO: if the GarbageCollector is added, 
		// watch out, the edge keys can change...
		return (this < opposite_) ;
	}

	//_________________________________________________________

}

#endif

