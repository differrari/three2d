#pragma once

#include "string/slice.h"
#include "mesh.h"

void geolisp_init();
void geolisp_eval(string_slice slice);

mesh* geolisp_get_mesh(int num);