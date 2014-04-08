#include <string.h>
#include "SASTL.h"

enum {
	ST_HEADER
	,ST_TRIANGLE
};

enum {
	TRIANGLE_SIZE = 3 * sizeof(float) * 4 + sizeof(uint16_t)
};

sastl* sastl_init(sastl* parser){
	memset(parser, 0, sizeof(*parser));
	return parser;
}

int sastl_parse(sastl* parser, const uint8_t *buffer, uint32_t length){
	if(!buffer){
		if(parser->_state != ST_TRIANGLE)
			return -1;
		return parser->_count;
	}

	const uint8_t* cur = buffer;
	const uint8_t* const end = buffer + length;
	while(cur != end){
		switch(parser->_state){
			case ST_HEADER:
				{
					/* Determine bounds */
					unsigned bound = (end - cur > 84 - parser->_count) ?
						84 - parser->_count : end - cur;

					memcpy((uint8_t*)&(parser->header) + parser->_count, cur, bound);
					parser->_count += bound;
					cur += bound;
					if(84 == parser->_count){
						/* TODO correct for endianness on triangle count. */
						parser->_count = 0;
						parser->_state = ST_TRIANGLE;
					}
				}
				break;

			case ST_TRIANGLE:
				{
					uint8_t* tri = (uint8_t*)(&parser->triangle) + parser->_count;
					/* Determine bounds */
					unsigned bound = (end - cur > TRIANGLE_SIZE - parser->_count) ?
						TRIANGLE_SIZE - parser->_count : end - cur;

					memcpy(tri, cur, bound);
					cur += bound;
					parser->_count += bound;
					if(TRIANGLE_SIZE == parser->_count){
						/* TODO correct for endianness on triangle data. */
						/* Issue callback. */
						int ret = parser->user_cb(parser);
						if(ret)
							return ret;
						parser->_count = 0;
					}
				}
				break;
		}
	}

	return 0;
}
