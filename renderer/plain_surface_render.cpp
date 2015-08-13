
#include "plain_surface_render.h"
#include "../opengl/opengl_info.h"



PlainSurfaceRender::PlainSurfaceRender(Map* obj)
: SurfaceRender(obj)
{
	smooth_shading_ = false;
	use_color_attribute_ = true;
}

void PlainSurfaceRender::draw_surface() {

	if (use_color_attribute_)
		facet_color_.bind_if_defined(target(), "color") ;
	if (use_color_attribute_ && (!facet_color_.is_bound()))
		vertex_color_.bind_if_defined(target(), "color") ;

	glEnable(GL_LIGHTING);
	glColor4fv(surface_style_.color.data());

	if (vertex_color_.is_bound()) {
		glShadeModel(GL_SMOOTH);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		if (smooth_shading_) {
			if (!MapVertexNormal::is_defined(target()))
				target()->compute_vertex_normals();
			MapVertexNormal normal(target()) ;

			FOR_EACH_FACET_CONST(Map, target(), it) {
				glBegin(GL_POLYGON);
				Map::Halfedge* jt = it->halfedge() ;
				do {
					Map::Vertex* v = jt->vertex();
					glColor4fv(vertex_color_[v].data());

					const vec3& n = normal[v];
					glNormal3dv(n.data());

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
					glColor4fv(vertex_color_[v].data());

					const vec3& p = v->point();
					glVertex3dv(p.data());
					jt = jt->next() ;
				} while(jt != it->halfedge()) ;
				glEnd();
			}
		}
	} 
	else if (smooth_shading_) {
		glShadeModel(GL_SMOOTH);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	

		if (!MapVertexNormal::is_defined(target()))
			target()->compute_vertex_normals();
		MapVertexNormal normal(target()) ;

		FOR_EACH_FACET_CONST(Map, target(), it) {
			if (facet_color_.is_bound())
				glColor4fv(facet_color_[it].data());

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
		glShadeModel(GL_FLAT);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	

		if(!MapFacetNormal::is_defined(target()))
			target()->compute_facet_normals() ;
		MapFacetNormal normal(target()) ;

		FOR_EACH_FACET_CONST(Map, target(), it) {
			const vec3& n = normal[it];

			glNormal3dv(n.data());
			if (facet_color_.is_bound())
				glColor4fv(facet_color_[it].data());

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

	if(facet_color_.is_bound()) {
		facet_color_.unbind() ;
	}
	if(vertex_color_.is_bound()) {
		vertex_color_.unbind() ;
	}	
}
void PlainSurfaceRender::draw_mesh() {
	glColor4fv(mesh_style_.color.data());

	glDisable(GL_LIGHTING);
	glLineWidth(mesh_style_.width);
	glBegin(GL_LINES);
	FOR_EACH_EDGE_CONST(Map, target(), it) {
		if (it->is_border_edge() && border_style_.visible)
			continue;
		const vec3& p = it->vertex()->point();
		const vec3& q = it->opposite()->vertex()->point();
		glVertex3dv(p.data());
		glVertex3dv(q.data());
	}
	glEnd();
}

void PlainSurfaceRender::draw_border() {
	glDisable(GL_LIGHTING);

	glColor4fv(border_style_.color.data());
	glLineWidth(border_style_.width);

	glBegin(GL_LINES);
	FOR_EACH_HALFEDGE_CONST(Map, target(), it) {
		if(it->is_border()) {
			const vec3& p = it->vertex()->point();
			const vec3& q = it->opposite()->vertex()->point();
			glVertex3dv(p.data());
			glVertex3dv(q.data());
		}
	}
	glEnd();
}

void PlainSurfaceRender::draw_vertices() {
	glDisable(GL_LIGHTING);

	glColor4fv(vertices_style_.color.data());
	glPointSize(vertices_style_.size);

	glBegin(GL_POINTS);
	FOR_EACH_VERTEX_CONST(Map, target(), it) {
		const vec3& p = it->point();
		glVertex3dv(p.data());
	}
	glEnd();
}

void PlainSurfaceRender::draw_anchors() {
	glDisable(GL_LIGHTING);

	glColor4fv(anchors_style_.color.data());
	glPointSize(anchors_style_.size);

	MapVertexLock is_locked(target()) ;
	glBegin(GL_POINTS);
	FOR_EACH_VERTEX_CONST(Map, target(), it) {
		if(is_locked[it]) {
			const vec3& p = it->point();
			glVertex3dv(p.data());
		}
	}
	glEnd();
}

void PlainSurfaceRender::draw_pins() {

}

void PlainSurfaceRender::set_use_color_attribute(bool x) { 
	use_color_attribute_ = x ;
}

