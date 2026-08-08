#include "geolisp.h"
#include "interpreter/repl.h"
#include "mesh.h"
#include "interpreter/repl.h"
#include "ir/manual_gen.h"

imaginal_repl_ctx geolisp_ctx;

chunk_array_t *meshes;

mesh make_simple_mesh(){
    mesh m = {};
    m.vertices = chunk_array_create(sizeof(vector3), 64);
    m.segments = chunk_array_create(sizeof(int), 64);
    m.primitive_type = primitives_trig;
    return m;
}

vector3 geolisp_parse_v3(codegen exp){
    // imaginal_print(exp);
    vector3 v = {};
    v.x = car_double(exp);
    exp = cdr(exp);
    v.y = car_double(exp);
    exp = cdr(exp);
    v.z = car_double(exp);
    return v;
}

void geolisp_push_trig(mesh *m, codegen trig){
    int a = car_integer(trig);
    trig = cdr(trig);
    int b = car_integer(trig);
    trig = cdr(trig);
    int c = car_integer(trig);
    
    chunk_array_push(m->segments, &a);
    chunk_array_push(m->segments, &b);
    chunk_array_push(m->segments, &c);
}

codegen geolisp_script_fallback(codegen exp, codegen args, codegen *env){
    string_slice fn = car_id(exp);
    if (slice_lit_match(fn, "createmesh", true)){
        mesh m = make_simple_mesh();
        size_t id = chunk_array_push(meshes, &m);
        return make_int_atom(id);
    }
    if (slice_lit_match(fn, "push_vert", true)){
        print("push_vert:");
        imaginal_print(args);
        int mid = car_integer(car(args));
        print("MID: %i",mid);
        args = cdr(args);
        imaginal_print(car(car(args)));
        vector3 v = geolisp_parse_v3(car(car(args)));//NOTE: This double car is suspicious, issue might've been elsewhere. car(car) should point to x, not the list, so we might have the wrong value
        mesh *m = geolisp_get_mesh(mid);
        if (!m) return nil_exp;
        print("V34 %f,%f,%f",v.x,v.y,v.z);
        return make_int_atom(chunk_array_push(m->vertices, &v));
    }
    if (slice_lit_match(fn, "push_trig", true)){
        int mid = car_integer(car(args));
        args = cdr(args);
        mesh *m = geolisp_get_mesh(mid);
        if (!m) return nil_exp;
        imaginal_print(car(car(args)));
        geolisp_push_trig(m, car(car(args)));//NOTE: suspicious double car, see push_vert
        return make_true_atom();
    }
    // imaginal_print(args);
    return car(args);
}


void geolisp_init(){
    meshes = chunk_array_create(sizeof(mesh), 32);
    geolisp_ctx = (imaginal_repl_ctx){
        .environment = nil_exp,
        .fallback = geolisp_script_fallback,
        .output = nil_exp,
        .should_print = true
    };   
}

void geolisp_eval(string_slice slice){
    repl_run(slice, &geolisp_ctx);
}

mesh* geolisp_get_mesh(int num){
    return chunk_array_get(meshes, num);
}