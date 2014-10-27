#include <cmath>
#include <algorithm>
#include <limits>
#include <vector>
#include <map>
#include <set>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


extern "C" {
	#include <SASTL.h>
}

float _min_x = INFINITY;
float _min_y = INFINITY;
float _min_z = INFINITY;

float _max_x = -INFINITY;
float _max_y = -INFINITY;
float _max_z = -INFINITY;

std::vector<sastl_triangle> s_triangles;

void read_stl(sastl* parser, int infd){
	while(1){
		uint8_t buffer[512];
		int ret = read(infd, buffer, 512);
		if(ret < 0)
			throw std::runtime_error("DEM READ ERROR");

		if(0 == ret){
			int final = sastl_parse(parser, NULL, 0);
			if(final != 0)
				throw std::runtime_error("READ ENDED WITH ERROR");
			break;
		}

		ret = sastl_parse(parser, buffer, ret);
		if(ret){
			throw std::runtime_error(std::string("PARSE ERROR ") + std::to_string(ret));
		}
	}
	close(infd);
}

int s_measure(const struct sastl_s* parser){
	const sastl_triangle &t = parser->triangle;

	if(t.v[0][0] < _min_x)
		_min_x = t.v[0][0];
	if(t.v[1][0] < _min_x)
		_min_x = t.v[1][0];
	if(t.v[2][0] < _min_x)
		_min_x = t.v[2][0];

	if(t.v[0][1] < _min_y)
		_min_y = t.v[0][1];
	if(t.v[1][1] < _min_y)
		_min_y = t.v[1][1];
	if(t.v[2][1] < _min_y)
		_min_y = t.v[2][1];

	if(t.v[0][2] < _min_z)
		_min_z = t.v[0][2];
	if(t.v[1][2] < _min_z)
		_min_z = t.v[1][2];
	if(t.v[2][2] < _min_z)
		_min_z = t.v[2][2];

	if(t.v[0][0] > _max_x)
		_max_x = t.v[0][0];
	if(t.v[1][0] > _max_x)
		_max_x = t.v[1][0];
	if(t.v[2][0] > _max_x)
		_max_x = t.v[2][0];

	if(t.v[0][1] > _max_y)
		_max_y = t.v[0][1];
	if(t.v[1][1] > _max_y)
		_max_y = t.v[1][1];
	if(t.v[2][1] > _max_y)
		_max_y = t.v[2][1];

	if(t.v[0][2] > _max_z)
		_max_z = t.v[0][2];
	if(t.v[1][2] > _max_z)
		_max_z = t.v[1][2];
	if(t.v[2][2] > _max_z)
		_max_z = t.v[2][2];

	s_triangles.push_back(t);

	return 0;
}


int main(int argc, const char* argv[]){
	if(argc < 2){
		fprintf(stderr, "Usage %s BINARY.stl scale_factor\n", argv[0]);
		return 1;
	}

	int infd = open(argv[1], O_RDONLY);
	try{
		if(-1 == infd)
			throw std::runtime_error("Unable to open input file!");

		float scale = (argc > 2) ? strtof(argv[2], NULL) : 1.0;
		if(0 == scale)
			throw std::runtime_error("Invalid scale factor!");

		sastl parser;
		sastl_init(&parser);
		parser.user_cb = s_measure;

		read_stl(&parser, infd);
		infd = -1;

		/* Print out file. */
		if(write(1, &parser.header, sizeof(parser.header)) < (int)sizeof(parser.header))
			throw std::runtime_error("Error writing output!");

		_min_x += _max_x;
		_min_x /= 2.0;

		_min_y += _max_y;
		_min_y /= 2.0;

		for(unsigned i = 0; i < s_triangles.size(); ++i){
			/* Adjust the bias. */
			sastl_triangle& t = s_triangles[i];

			/* Skip over zeroed triangles */
			if(t.normal[0] == 0 && t.normal[1] == 0 && t.normal[2] == 0)
				continue;

			t.v[0][0] -= _min_x;
			t.v[1][0] -= _min_x;
			t.v[2][0] -= _min_x;

			t.v[0][1] -= _min_y;
			t.v[1][1] -= _min_y;
			t.v[2][1] -= _min_y;

			t.v[0][2] -= _min_z;
			t.v[1][2] -= _min_z;
			t.v[2][2] -= _min_z;

			for(unsigned k = 0; k < 3; ++k){
				t.v[k][0] *= scale;
				t.v[k][1] *= scale;
				t.v[k][2] *= scale;
			}
			/* TODO buffer write calls for efficiency's sake. */
			static const unsigned TRIANGLE_SIZE =
				3 * sizeof(float) * 4 + sizeof(uint16_t);
			if(write(1, &t, TRIANGLE_SIZE) < (int)TRIANGLE_SIZE)
				throw std::runtime_error("Error writing triangle!");
		}
		close(1);

		return 0;
	}
	catch(const std::runtime_error& e){
		if(infd != -1)
			close(infd);
		fprintf(stderr, "RUNTIME ERROR %s\n", e.what());
		return 1;
	}
}
