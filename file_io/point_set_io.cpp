#include "point_set_io.h"
#include "point_set_serializer_ply.h"
#include "../geom/point_set.h"
#include "../geom/iterators.h"
#include "../basic/stop_watch.h"
#include "../basic/file_utils.h"
#include "../basic/progress.h"
#include "../basic/logger.h"
#include <fstream>



PointSet* PointSetIO::read(const std::string& file_name)
{
	std::ifstream in(file_name.c_str());
	if (in.fail()) {
		Logger::err(title()) << "cannot open file: " << file_name << std::endl;
		return nil;
	}
	in.close();

	Logger::out(title()) << "reading file..." << std::endl;

	StopWatch w;
	PointSet* object = nil;

	std::string ext = FileUtils::extension(file_name);
	String::to_lowercase(ext);

	if (ext == "ply") {
		object = PointSetSerializer_ply::load(file_name);
	}

	else {
		PointSet* point_set = new PointSet;
		if (ext == "xyz")
			load_xyz(point_set, file_name);
		else if (ext == "bxyz")
			load_bxyz(point_set, file_name);

		else if (ext == "pn")
			load_pn(point_set, file_name);
		else if (ext == "bpn")
			load_bpn(point_set, file_name);

		else if (ext == "pnc")
			load_pnc(point_set, file_name);
		else if (ext == "bpnc")
			load_bpnc(point_set, file_name);

		else {
			Logger::err(title()) << "reading file failed (unknown file format)" << std::endl;
			delete point_set;
			return nil;
		}

		if (point_set->size_of_vertices() < 1) {
			Logger::err(title()) << "reading file failed (no data exist)" << std::endl;
			delete point_set;
			return nil;
		}
		object = point_set;
	}

	Logger::out(title()) << "reading file done. Time: "
		<< w.elapsed() << " seconds" << std::endl;

	return object;
}

bool PointSetIO::save(const std::string& file_name, const PointSet* point_set) {
	if (!point_set) {
		Logger::err(title()) << "Point set is null" << std::endl;
		return false;
	}

	std::ofstream out(file_name.c_str());
	if (out.fail()) {
		Logger::err(title()) << "cannot open file: \'" << file_name << "\' for writing" << std::endl;
		return false;
	}
	Logger::out(title()) << "saving file..." << std::endl;
	out.close();

	StopWatch w;
	std::string ext = FileUtils::extension(file_name);
	String::to_lowercase(ext);

	out.precision(16);

	if (ext == "pn")
		save_pn(point_set, file_name);
	else if (ext == "bpn")
		save_bpn(point_set, file_name);

	else if (ext == "ply")
		PointSetSerializer_ply::save(file_name, point_set);

	else if (ext == "pnc")
		save_pnc(point_set, file_name);
	else if (ext == "bpnc")
		save_bpnc(point_set, file_name);

	else if (ext == "xyz")
		save_xyz(point_set, file_name);
	else if (ext == "bxyz")
		save_bxyz(point_set, file_name);

	else {
		Logger::err(title()) << "saving file failed (unknown file format)" << std::endl;
		return false;
	}

	Logger::out(title()) << "saving file done. Time: "
		<< w.elapsed() << " seconds" << std::endl;

	return true;
}

//HaoLi:load_xyz by its scheme 
void PointSetIO::load_xyz(PointSet* pointSet, const std::string& file_name) {

	int cols = get_file_cols(file_name);

	std::ifstream input(file_name.c_str());
	if (input.fail()) {
		Logger::err(title()) << "could not open file\'" << file_name << "\'" << std::endl;
		return;
	}

	printf("cols:%d\n", cols);

	switch (cols){
	case 3:
		while (!input.eof()) {
			float x, y, z;
			input >> x >> y >> z;

			if (input.good()) {
				pointSet->new_vertex(vec3(x, y, z));
			}
		}
		break;
	case 6:{
			   PointSetNormal normals6(pointSet);

			   while (!input.eof()) {
				   float x, y, z, nx, ny, nz;
				   input >> x >> y >> z >> nx >> ny >> nz;

				   if (input.good()) {
					   PointSet::Vertex* v = pointSet->new_vertex(vec3(x, y, z));
					   normals6[v] = vec3(nx, ny, nz);
				   }
			   }
			   break;
	}
	case 9:{
			   PointSetNormal normals9(pointSet);
			   //PointSetColor colors9(pointSet);

			   while (!input.eof()) {
				   float x, y, z, nx, ny, nz, r, g, b;
				   input >> x >> y >> z >> nx >> ny >> nz >> r >> g >> b;

				   if (input.good()) {
					   PointSet::Vertex* v = pointSet->new_vertex(vec3(x, y, z));
					   normals9[v] = vec3(nx, ny, nz);
					   //colors9[v] = Color(r, g, b);
				   }
			   }
			   break;
	}

	}
}

//HaoLi:get file columns
int PointSetIO::get_file_cols(const std::string& file_name){
	std::ifstream infile(file_name.c_str());
	infile >> std::noskipws;

	int col = 0;
	bool flag = 1;

	char chr;
	while (infile >> chr && flag)
	{
		switch (chr)
		{
		case '\n':
			flag = 0;
			break;
		case ' ':
			++col;
			break;
		default:;
		}
	}
	infile.close();

	col++;
	return col;
}


void PointSetIO::save_xyz(const PointSet* pointSet, const std::string& file_name) {
	// open file
	std::ofstream output(file_name.c_str());
	if (output.fail()) {
		Logger::err(title()) << "could not open file\'" << file_name << "\'" << std::endl;
		return;
	}
	output.precision(16);

	ProgressLogger progress(pointSet->size_of_vertices());
	FOR_EACH_VERTEX_CONST(PointSet, pointSet, it) {
		const vec3& p = it->point();
		output << p << std::endl;
		progress.next();
	}
}


void PointSetIO::load_bxyz(PointSet* pointSet, const std::string& file_name) {
	std::ifstream input(file_name.c_str(), std::fstream::binary);
	if (input.fail()) {
		Logger::err(title()) << "could not open file\'" << file_name << "\'" << std::endl;
		return;
	}

	std::streamoff begin_pos = input.tellg();
	input.seekg(0, std::ios::end);
	std::streamoff end_pos = input.tellg();
	// num of points in the file
	int num = static_cast<int>(end_pos - begin_pos) / 12;  // 12 bytes per point
	input.seekg(0, std::ios::beg);

	float* data = new float[num * 3];
	input.read((char*)data, num * 12);	// read the entire blocks

	ProgressLogger progress(num);
	for (int i = 0; i < num; ++i) {
		progress.notify(i);
		pointSet->new_vertex(vec3(data + i * 3));
	}
	delete[] data;
}


void PointSetIO::save_bxyz(const PointSet* pointSet, const std::string& file_name) {
	// open file
	std::ofstream output(file_name.c_str(), std::fstream::binary);
	if (output.fail()) {
		Logger::err(title()) << "could not open file\'" << file_name << "\'" << std::endl;
		return;
	}

	// TODO: save large blocks
	ProgressLogger progress(pointSet->size_of_vertices());
	FOR_EACH_VERTEX_CONST(PointSet, pointSet, it) {
		const vec3& p = it->point();
		float x = static_cast<float>(p.x);    output.write((char*)&x, 4);
		float y = static_cast<float>(p.y);    output.write((char*)&y, 4);
		float z = static_cast<float>(p.z);    output.write((char*)&z, 4);
		progress.next();
	}
}


void PointSetIO::load_pn(PointSet* pointSet, const std::string& file_name) {
	std::ifstream input(file_name.c_str());
	if (input.fail()) {
		Logger::err(title()) << "could not open file\'" << file_name << "\'" << std::endl;
		return;
	}

	PointSetNormal normals(pointSet);
	while (!input.eof()) {
		float x, y, z, nx, ny, nz;
		input >> x >> y >> z >> nx >> ny >> nz;

		if (input.good()) {
			PointSet::Vertex* v = pointSet->new_vertex(vec3(x, y, z));
			normals[v] = vec3(nx, ny, nz);
		}
	}
}


void PointSetIO::save_pn(const PointSet* pointSet, const std::string& file_name) {
	// open file
	std::ofstream output(file_name.c_str());
	if (output.fail()) {
		Logger::err(title()) << "could not open file\'" << file_name << "\'" << std::endl;
		return;
	}
	output.precision(16);

	PointSetNormal normals(const_cast<PointSet*>(pointSet));
	ProgressLogger progress(pointSet->size_of_vertices());
	FOR_EACH_VERTEX_CONST(PointSet, pointSet, it) {
		const vec3& p = it->point();
		const vec3& n = normals[it];
		output << p << " " << n << std::endl;
		progress.next();
	}
}


void PointSetIO::load_bpn(PointSet* pointSet, const std::string& file_name) {
	std::ifstream input(file_name.c_str(), std::fstream::binary);
	if (input.fail()) {
		Logger::err(title()) << "could not open file\'" << file_name << "\'" << std::endl;
		return;
	}

	// num of points in the file
	int num;
	input.read((char*)(&num), sizeof(int));

	PointSetNormal normals(pointSet);

	float* data = new float[num * 6];
	input.read((char*)data, num * 24);	// read the entire blocks
	ProgressLogger progress(num);
	for (int i = 0; i < num; ++i) {
		progress.notify(i);
		PointSet::Vertex* v = pointSet->new_vertex(vec3(data + i * 6));
		normals[v] = vec3(data + i * 6 + 3);
	}

	delete[] data;
}


void PointSetIO::save_bpn(const PointSet* pointSet, const std::string& file_name) {
	// open file
	std::ofstream output(file_name.c_str(), std::fstream::binary);
	if (output.fail()) {
		Logger::err(title()) << "could not open file\'" << file_name << "\'" << std::endl;
		return;
	}

	int num = pointSet->size_of_vertices();

	output.write((char*)&num, sizeof(int));
	PointSetNormal normals(const_cast<PointSet*>(pointSet));

	// TODO: save large blocks
	ProgressLogger progress(pointSet->size_of_vertices());
	FOR_EACH_VERTEX_CONST(PointSet, pointSet, it) {
		const vec3& p = it->point();
		float x = static_cast<float>(p.x);    output.write((char*)&x, 4);
		float y = static_cast<float>(p.y);    output.write((char*)&y, 4);
		float z = static_cast<float>(p.z);    output.write((char*)&z, 4);

		const vec3& n = normals[it];
		float nx = static_cast<float>(n.x);    output.write((char*)&nx, 4);
		float ny = static_cast<float>(n.y);    output.write((char*)&ny, 4);
		float nz = static_cast<float>(n.z);    output.write((char*)&nz, 4);

		progress.next();
	}
}


void PointSetIO::load_pnc(PointSet* pointSet, const std::string& file_name) {
	std::ifstream input(file_name.c_str());
	if (input.fail()) {
		Logger::err(title()) << "could not open file\'" << file_name << "\'" << std::endl;
		return;
	}

	PointSetNormal	normals(pointSet);
	PointSetColor	colors(pointSet);

	float x, y, z, nx, ny, nz, r, g, b;
	while (!input.eof()) {
		input >> x >> y >> z >> nx >> ny >> nz >> r >> g >> b;

		if (input.good()) {
			PointSet::Vertex* v = pointSet->new_vertex(vec3(x, y, z));
			normals[v] = vec3(nx, ny, nz);
			colors[v] = Color(r, g, b);
		}
	}
}


void PointSetIO::save_pnc(const PointSet* pointSet, const std::string& file_name) {
	// open file
	std::ofstream output(file_name.c_str());
	if (output.fail()) {
		Logger::err(title()) << "could not open file\'" << file_name << "\'" << std::endl;
		return;
	}
	output.precision(16);

	PointSetNormal	normals(const_cast<PointSet*>(pointSet));
	PointSetColor	colors(const_cast<PointSet*>(pointSet));

	ProgressLogger progress(pointSet->size_of_vertices());
	FOR_EACH_VERTEX_CONST(PointSet, pointSet, it) {
		const vec3& p = it->point();
		const vec3& n = normals[it];
		const Color& c = colors[it];
		output << p << " " << n << " " << c.r() << " " << c.g() << " " << c.b() << std::endl;

		progress.next();
	}
}


// each line: x y z nx ny nz r g b. All are float
void PointSetIO::load_bpnc(PointSet* pointSet, const std::string& file_name) {
	std::ifstream input(file_name.c_str(), std::fstream::binary);
	if (input.fail()) {
		Logger::err(title()) << "could not open file\'" << file_name << "\'" << std::endl;
		return;
	}

	PointSetNormal	normals(pointSet);
	PointSetColor	colors(pointSet);

	// check size of types
	int line_size = sizeof(float)* 9;

	std::streamoff begin_pos = input.tellg();
	input.seekg(0, std::ios::end);
	std::streamoff end_pos = input.tellg();
	// num of points in the file
	int num = static_cast<int>(end_pos - begin_pos) / line_size;

	input.seekg(0, std::ios::beg);

	float* data = new float[num * 9];
	input.read((char*)data, num * line_size);	// read the entire blocks

	ProgressLogger progress(num);
	for (int i = 0; i < num; ++i) {
		progress.notify(i);

		PointSet::Vertex* v = pointSet->new_vertex(vec3(data + i * 9));
		normals[v] = vec3(data + i * 9 + 3);
		colors[v] = Color(data + i * 9 + 6);
	}
	delete[] data;
}


void PointSetIO::save_bpnc(const PointSet* pointSet, const std::string& file_name) {
	int line_size = sizeof(float)* 9;

	// open file
	std::ofstream output(file_name.c_str(), std::fstream::binary);
	if (output.fail()) {
		Logger::err(title()) << "could not open file\'" << file_name << "\'" << std::endl;
		return;
	}

	int num = pointSet->size_of_vertices();
	PointSetNormal	normals(const_cast<PointSet*>(pointSet));
	PointSetColor	colors(const_cast<PointSet*>(pointSet));

	ProgressLogger progress(pointSet->size_of_vertices());
	FOR_EACH_VERTEX_CONST(PointSet, pointSet, it) {
		const vec3& p = it->point();
		float x = static_cast<float>(p.x);    output.write((char*)&x, 4);
		float y = static_cast<float>(p.y);    output.write((char*)&y, 4);
		float z = static_cast<float>(p.z);    output.write((char*)&z, 4);

		const vec3& n = normals[it];
		float nx = static_cast<float>(n.x);    output.write((char*)&nx, 4);
		float ny = static_cast<float>(n.y);    output.write((char*)&ny, 4);
		float nz = static_cast<float>(n.z);    output.write((char*)&nz, 4);

		const Color& c = colors[it];
		float r = c.r();    output.write((char*)&r, 4);
		float g = c.g();    output.write((char*)&g, 4);
		float b = c.b();    output.write((char*)&b, 4);

		progress.next();
	}
}
