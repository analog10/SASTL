#ifndef __SASTL_PARSE_H__
#define __SASTL_PARSE_H__

#include <stdint.h>

struct sastl_s;

/** @brief Callback function type. */
typedef int(*sastl_cb)(const struct sastl_s* parser);

typedef struct sastl_triangle_s {
	float normal[3];
	float v[3][3];
	uint16_t attribute;
} sastl_triangle;

typedef struct sastl_header_s {
	/** @brief Header data. */
	uint8_t header[80];

	/** @brief Number of triangles in file. */
	uint32_t triangle_count;
} sastl_header;

typedef struct sastl_s {

	/** @brief  */
	sastl_cb user_cb;

	/** @brief  */
	void* user_data;

	/** @brief Header parsed from file. */
	sastl_header header;

	/** @brief Currently parsed triangle. */
	sastl_triangle triangle;


	/** @brief Number of triangles parsed so far. */
	unsigned triangles_parsed;


	/** @brief Current parsing state. */
	unsigned _state;

	/** @brief Current parse count in current state. */
	unsigned _count;

} sastl;


sastl* sastl_init(sastl* parser);

int sastl_parse(sastl* parser, const uint8_t *buffer, uint32_t length);

#endif
