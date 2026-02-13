#include "data/struct/chunk_array.h"

#include "types/vector3.h"
#include "three2d.h"

char* primitives_to_string(primitives val);

typedef struct mesh { chunk_array_t* vertices;
	chunk_array_t* segments;
	primitives primitive_type;
	
	
	
	
 } mesh;
size_t mesh_num_verts(mesh *instance);size_t mesh_num_segments(mesh *instance);int mesh_get_segment(mesh *instance, size_t index);vector3 mesh_get_vertex(mesh *instance, size_t index);