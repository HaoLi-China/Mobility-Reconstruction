#include "scalar_surface_render.h"
#include "texture_factory.h"
#include "../geom/map.h"
#include "../geom/map_geometry.h"
#include "../image/image_io.h"
#include "../basic/logger.h"
#include "../basic/file_utils.h"



ScalarSurfaceRender::ScalarSurfaceRender(Map* obj) 
: PlainSurfaceRender(obj) 
{   
	use_color_attribute_ = false;

	smooth_shading_ = false;

	location_ = on_facets;
	edge_attr_style_ = sticks;
}


void ScalarSurfaceRender::set_location(SurfaceAttributeLocation loc) {
	location_ = loc ;
}


void ScalarSurfaceRender::set_edge_attribute_style(EdgeAttributeStyle x) {
	edge_attr_style_ = x ; 
}


void ScalarSurfaceRender::draw() {
	switch(location_) {
		case on_vertices :
			vertex_attr_.bind_if_defined(target(), attribute_name_) ;
			break ;
		case on_edges:
			halfedge_attr_.bind_if_defined(target(), attribute_name_) ;
			break ;
		case on_facets:
			facet_attr_.bind_if_defined(target(), attribute_name_) ;
			break ;
	}

	if (use_texture_)
		create_textures_if_needed() ;

	PlainSurfaceRender::draw() ;

	// force to draw edge attributes even when the surface flag in unset
	if (!surface_style_.visible && location_ == on_edges && (edge_attr_style_ == arrows || edge_attr_style_ == dual)) {
		draw_with_edge_attribute() ;
	}

	if(!mesh_style_.visible && location_ == on_edges && edge_attr_style_==sticks) {
		draw_mesh() ;
	}

	if(vertex_attr_.is_bound()) {
		vertex_attr_.unbind() ;
	}

	if(halfedge_attr_.is_bound()) {
		halfedge_attr_.unbind() ;
	}

	if(facet_attr_.is_bound()) {
		facet_attr_.unbind() ;
	}
}


void ScalarSurfaceRender::draw_surface() 
{
	if(vertex_attr_.is_bound())
		draw_with_vertex_attribute() ;

	else if(halfedge_attr_.is_bound()) {
		if(edge_attr_style_ == corners) {
			glColor4fv(surface_style_.color.data()) ;
			glEnable(GL_LIGHTING);
			MapFacetNormal normal(target()) ;
			FOR_EACH_FACET_CONST(Map, target(), it) {
				const vec3& n = normal[it];

				glNormal3dv(n.data());
				glBegin(GL_POLYGON);
				Map::Halfedge* h = it->halfedge() ;
				do {
					const vec3 p = h->prev()->vertex()->point() + 0.33 * Geom::vector(h);
					glVertex3dv(p.data());

					const vec3 q = h->vertex()->point() - 0.33 * Geom::vector(h);
					glVertex3dv(q.data());

					h = h->next() ;
				} while(h != it->halfedge()) ;
				glEnd();
			}
			draw_with_edge_attribute() ;
		} 
		else if(edge_attr_style_ == arrows || edge_attr_style_ == dual) {
			draw_with_edge_attribute() ;
			PlainSurfaceRender::draw_surface() ;
		}
		else
			PlainSurfaceRender::draw_surface() ;
	} 
	else if(facet_attr_.is_bound())
		draw_with_facet_attribute() ;
	else
		PlainSurfaceRender::draw_surface() ;
}

void ScalarSurfaceRender::draw_mesh() {
	if(halfedge_attr_.is_bound() && (edge_attr_style_ == sticks))
		draw_with_edge_attribute() ;
	else
		PlainSurfaceRender::draw_mesh() ;
}

void ScalarSurfaceRender::draw_with_vertex_attribute()
{
	if(!vertex_attr_.is_bound()) {
		PlainSurfaceRender::draw_surface() ;
		return ;
	}

	double maximum = maximum_ ;
	double minimum = minimum_ ;
	if(auto_range_) {
		maximum = -Numeric::big_double ;
		minimum =  Numeric::big_double ;
		FOR_EACH_VERTEX(Map, target(), it) {
			maximum = ogf_max(maximum, vertex_attr_[it]) ;
			minimum = ogf_min(minimum, vertex_attr_[it]) ;
		}
		Logger::status() << "Value range: [" << minimum << ", " << maximum << "]" << std::endl ;
	}

	if(minimum == maximum) {
		maximum += 1.0 ;
	}
	vertex_attr_.set_range(minimum, maximum) ;

	glEnable(GL_LIGHTING);
	if (use_texture_ && colormap_texture_)
		colormap_texture_->bind();

	glShadeModel(GL_SMOOTH);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	
	
	if(smooth_shading_) { 
		if (!MapVertexNormal::is_defined(target()))
			target()->compute_vertex_normals();
		MapVertexNormal normal(target()) ;

		FOR_EACH_FACET_CONST(Map, target(), it) {
			glBegin(GL_POLYGON);
			Map::Halfedge* jt = it->halfedge() ;
			do {
				Map::Vertex* v = jt->vertex();

				const vec3& n = normal[v];
				glNormal3dv(n.data());

				double att = vertex_attr_[v] ;
				set_color(att);

				const vec3& p = v->point();
				glVertex3dv(p.data());
				jt = jt->next() ;
			} while(jt != it->halfedge()) ;
			glEnd();
		}

	} 
	else {
		if(!MapFacetNormal::is_defined(target()))
			target()->compute_facet_normals() ;
		MapFacetNormal normal(target()) ;

		FOR_EACH_FACET_CONST(Map, target(), it) {
			const vec3& n = normal[it];
			glNormal3dv(n.data());

			glBegin(GL_POLYGON);
			Map::Halfedge* jt = it->halfedge() ;
			do {
				Map::Vertex* v = jt->vertex();

				double att = vertex_attr_[v] ;
				set_color(att);

				const vec3& p = v->point();
				glVertex3dv(p.data());

				jt = jt->next() ;
			} while(jt != it->halfedge()) ;

			glEnd();
		}
	}

	if (use_texture_ && colormap_texture_)
		colormap_texture_->unbind();
}


void ScalarSurfaceRender::draw_with_edge_attribute() 
{        
	if(!halfedge_attr_.is_bound()) {
		PlainSurfaceRender::draw_mesh() ;
		return ;
	}

	double maximum = maximum_ ;
	double minimum = minimum_ ;
	if(auto_range_) {
		maximum = -Numeric::big_double ;
		minimum =  Numeric::big_double ;
		FOR_EACH_HALFEDGE(Map, target(), it) {
			maximum = ogf_max(maximum, halfedge_attr_[it]) ;
			minimum = ogf_min(minimum, halfedge_attr_[it]) ;
		}
		Logger::status() << "Value range: [" << minimum << ", " << maximum << "]" << std::endl ;
	}
	if(minimum == maximum) {
		maximum += 1.0 ;
	}
	halfedge_attr_.set_range(minimum, maximum) ;

	if(edge_attr_style_ == sticks || edge_attr_style_ == arrows)
		glDisable(GL_LIGHTING) ;

	glColor4fv(mesh_style_.color.data());
	glLineWidth(mesh_style_.width);

	if (use_texture_ && colormap_texture_)
		colormap_texture_->bind();

	switch(edge_attr_style_) {
		case sticks: 
			{
				glBegin(GL_LINES);
				FOR_EACH_EDGE_CONST(Map, target(), it) {
					double att = halfedge_attr_[it] ;
					set_color(att);
					const vec3& p = it->vertex()->point();
					glVertex3dv(p.data());
					const vec3& q = it->opposite()->vertex()->point();
					glVertex3dv(q.data());
				}
				glEnd();
			} 
			break ;
		case arrows:
			{
				if(!MapFacetNormal::is_defined(target()))
					target()->compute_facet_normals() ;
				MapFacetNormal normal(target()) ;

				glBegin(GL_LINES);
				FOR_EACH_FACET_CONST(Map, target(), it) {
					vec3 p = Geom::facet_barycenter(it) ;
					vec3 N = normal[it] ;
					Map::Halfedge* h = it->halfedge() ;
					do {
						double att = halfedge_attr_[h] ;
						set_color(att);

						vec3 p1 = h->prev()->vertex()->point() ;
						vec3 p2 = h->vertex()->point() ;
						p1 = p1 + 0.3 * (p - p1) ;
						p2 = p2 + 0.3 * (p - p2) ;
						p = Geom::barycenter(p1,p2) ;
						p1 = p1 + 0.3 * (p - p1) ;
						p2 = p2 + 0.3 * (p - p2) ;
						glVertex3dv(p1.data());
						glVertex3dv(p2.data());
						p = p + 0.8 * (p2 - p) ;
						vec3 H = cross(N, (p2 - p1)) ;

						glVertex3dv(p2.data());

						vec3 q = p + 0.04 * H;
						glVertex3dv(q.data());
						glVertex3dv(p2.data());

						q = p - 0.04 * H;	glVertex3dv(q.data());
						q = p + 0.04 * H;	glVertex3dv(q.data());
						q = p - 0.04 * H;	glVertex3dv(q.data());
						h = h->next() ;
					} while(h != it->halfedge()) ;
				}
				glEnd();	
			}
			break ;
		case corners: 
			{
				if(!MapFacetNormal::is_defined(target()))
					target()->compute_facet_normals() ;
				MapFacetNormal normal(target()) ;

				glBegin(GL_TRIANGLES);
				FOR_EACH_FACET_CONST(Map, target(), it) {
					vec3 n = normal[it];

					glNormal3dv(n.data());

					Map::Halfedge* h = it->halfedge() ;
					do {
						double att = halfedge_attr_[h] ;
						set_color(att);

						const vec3& p = h->vertex()->point() ;
						vec3 q = p + 0.33 * Geom::vector(h->next());
						vec3 r = p - 0.33 * Geom::vector(h);
						glVertex3dv(p.data());
						glVertex3dv(q.data());
						glVertex3dv(r.data());

						h = h->next() ;
					} while(h != it->halfedge()) ;
				}
				glEnd();  
			} 
			break ;
		case dual: 
			{
				if(!MapFacetNormal::is_defined(target()))
					target()->compute_facet_normals() ;
				MapFacetNormal normal(target()) ;

				glBegin(GL_LINES);
				FOR_EACH_FACET_CONST(Map, target(), it) {
					vec3 N = normal[it] ;
					Map::Halfedge* h = it->halfedge() ;
					do {
						if (!h->is_border_edge()) {
							vec3 p = h->vertex()->point() ;
							double att = halfedge_attr_[h] ;
							set_color(att) ;

							vec3 p1 = Geom::facet_barycenter(h->facet()) ;
							vec3 p2 = Geom::facet_barycenter(h->opposite()->facet()) ;
							p1 = p1 + 0.1 * (p - p1) ;
							p2 = p2 + 0.1 * (p - p2) ;
							p = Geom::barycenter(p1, p2) ;
							p1 = p1 + 0.1 * (p - p1) ;
							p2 = p2 + 0.1 * (p - p2) ;
							glVertex3dv(p1.data());
							glVertex3dv(p2.data());

							p = p + 0.9 * (p2 - p) ;
							vec3 H = cross(N, p2 - p1) ;

							glVertex3dv(p2.data());
							vec3 p3 = p + 0.14 * H;						
							glVertex3dv(p3.data());

							glVertex3dv(p2.data());
							vec3 p4 = p - 0.14 * H;						
							glVertex3dv(p4.data());

							glVertex3dv(p3.data());
							glVertex3dv(p4.data());
						}
						h = h->next() ;
					} while(h != it->halfedge()) ;
				}
				glEnd();
			} 
			break ;
	}

	if (use_texture_ && colormap_texture_)
		colormap_texture_->unbind();
}

void ScalarSurfaceRender::draw_with_facet_attribute()
{
	double maximum = maximum_ ;
	double minimum = minimum_ ;

	if(auto_range_) {
		maximum = -Numeric::big_double ;
		minimum =  Numeric::big_double ;
		FOR_EACH_FACET(Map, target(), it) {
			maximum = ogf_max(maximum, facet_attr_[it]) ;
			minimum = ogf_min(minimum, facet_attr_[it]) ;
		}
		Logger::status() << "Value range: [" << minimum << ", " << maximum << "]" << std::endl ;
	}
	if(minimum == maximum) {
		maximum += 1.0 ;
	}
	facet_attr_.set_range(minimum, maximum) ;

	glEnable(GL_LIGHTING);
	if (use_texture_ && colormap_texture_)
		colormap_texture_->bind();

	if(smooth_shading_) { 
		if (!MapVertexNormal::is_defined(target()))
			target()->compute_vertex_normals();
		MapVertexNormal normal(target()) ;

		glShadeModel(GL_SMOOTH);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	
		FOR_EACH_FACET_CONST(Map, target(), it) {
			double att = facet_attr_[it] ;
			set_color(att);

			glBegin(GL_POLYGON);
			Map::Halfedge* jt = it->halfedge() ;
			do {
				Map::Vertex* v = jt->vertex();

				const vec3& n = normal[v];
				glNormal3dv(n.data());

				const vec3& p = v->point();
				glVertex3dv(p.data());
				jt = jt->next() ;
			} while(jt != it->halfedge()) ;

			glEnd();
		}
	} else {
		if(!MapFacetNormal::is_defined(target()))
			target()->compute_facet_normals() ;
		MapFacetNormal normal(target()) ;

		glShadeModel(GL_FLAT);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	
		FOR_EACH_FACET_CONST(Map, target(), it) {
			const vec3& n = normal[it];

			glNormal3dv(n.data());

			double att = facet_attr_[it] ;
			set_color(att);

			glBegin(GL_POLYGON);
			Map::Halfedge* jt = it->halfedge() ;
			do {
				const vec3& p = jt->vertex()->point();
				glVertex3dv(p.data());
				jt = jt->next() ;
			} while(jt != it->halfedge()) ;

			glEnd();
		}
	}

	if (use_texture_ && colormap_texture_)
		colormap_texture_->unbind();
}


void ScalarSurfaceRender::draw_border()
{
	PlainSurfaceRender::draw_border() ;

	if( location_ == on_facets && 
		MapFacetAttribute<int>::is_defined(target(), attribute_name_)) 
	{
		glDisable(GL_LIGHTING);

		glColor4fv(border_style_.color.data());
		glLineWidth(ogf_max(border_style_.width / 2.0f, 1.0f)) ;

		MapFacetAttribute<int> values(target(), attribute_name_) ;
		glBegin(GL_LINES);
		FOR_EACH_EDGE_CONST(Map, target(), it) {
			Map::Facet* f1 = it->facet() ;
			Map::Facet* f2 = it->opposite()->facet() ;
			if(f1 != nil && f2 != nil && values[f1] != values[f2]) {
				const vec3& p = it->vertex()->point();
				const vec3& q = it->opposite()->vertex()->point();
				glVertex3dv(p.data());
				glVertex3dv(q.data());
			}
		}
		glEnd();
	}
}
