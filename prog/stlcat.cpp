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


/*********************************************************************************
 * Simple program to cat STL files together. No manifold checking performed...
 ********************************************************************************* */


std::vector<sastl_triangle> s_triangles;

/* All this does is add triangles. */
int s_append_cb(const struct sastl_s* parser){
	const sastl_triangle &t = parser->triangle;
	s_triangles.push_back(t);
	return 0;
}

int main(int argc, const char* argv[]){
	if(argc < 2){
		fprintf(stderr, "Usage %s BINARY1.stl BINARY2.stl ....\n", argv[0]);
		return 1;
	}

	sastl parser;
	sastl_init(&parser);
	parser.user_cb = s_append_cb;

	char err_buffer[1024];
	int infd = -1;
	try{
		/* Read each STL file passed to the program. */
		for(int i = 1; i < argc; ++i){
			infd = open(argv[i], O_RDONLY);
			if(-1 == infd){
				snprintf(err_buffer, 1023, "Unable to open input file %s\n", argv[i]);
				throw std::runtime_error(err_buffer);
			}

			unsigned byte_count = 0;
			while(1){
				uint8_t buffer[2048];
				int ret = read(infd, buffer, 2048);
				if(ret < 0){
					snprintf(err_buffer, 1023, "Read error after %u %i %s %s\n", byte_count, ret, strerror(errno), argv[i]);
					throw std::runtime_error(err_buffer);
				}
				byte_count += ret;

				if(0 == ret){
					int final = sastl_parse(&parser, NULL, 0);
					if(final != 0){
						snprintf(err_buffer, 1023, "Final parse ended with %i %s\n", final, argv[i]);
						throw std::runtime_error(err_buffer);
					}
					break;
				}

				ret = sastl_parse(&parser, buffer, ret);
				if(ret){
					snprintf(err_buffer, 1023, "Parser error %i %s\n", ret, argv[i]);
					throw std::runtime_error(err_buffer);
				}
			}
			close(infd);
			infd = -1;
		}

		/* At this point we have read in all the triangles, time to output to stdout. */

		/* Fix triangle count. */
		parser.header.triangle_count = s_triangles.size();

		int ret = write(1, &parser.header, sizeof(parser.header));
		if(ret < 0){
			throw std::runtime_error("Error writing output");
		}

		/* Not an efficient way to output triangles, but whatever (should be minimizing write() calls) */
		for(unsigned i = 0; i < s_triangles.size(); ++i){
			const sastl_triangle& t = s_triangles[i];
			int ret = write(1, &t, 3 * sizeof(float) * 4 + sizeof(uint16_t));
			if(ret < 0){
				throw std::runtime_error("Error writing output");
			}
		}

		return 0;
	}
	catch(const std::exception& e){
		if(infd != -1)
			close(infd);
		fprintf(stderr, "RUNTIME ERROR: %s\n", e.what());
		return 1;
	}
}
