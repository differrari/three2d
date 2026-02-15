#include "three2d.h"
#include "types/matrix4.h"

t2d_pipeline *current_pipeline;
vector3 camera_pos = {-15,00,100};

vector4 default_vert_shader(vector3 pos){
    vector4 cam_pos = (vector4){pos.x + camera_pos.x, pos.y - camera_pos.y, pos.z - camera_pos.z, 1};
    
    vector4 out_v = matrix4_mul(cam_pos, current_pipeline->proj_matrix);
    
    return out_v;
}

argbcolor default_frag_shader(vector4 frag_coord, int trig_id){
    return (argbcolor){
        .alpha = 0xFF,
        .red = ((trig_id) % 225) + 30,
        .green = ((trig_id + 30) % 225) + 30,
        .blue = ((trig_id + 50) % 225) + 30,
    };
}

void t2d_make_camera(t2d_pipeline *pipeline, float fov, float aspect, float near, float far){
    float tanfov = 0.726542528f;
    
    pipeline->proj_matrix = matrix_zero();
    
    pipeline->proj_matrix.m[0][0] = 1.0f/(tanfov*aspect);
    pipeline->proj_matrix.m[1][1] = 1.0f/tanfov;
    pipeline->proj_matrix.m[2][2] = -near/(far-near);
    pipeline->proj_matrix.m[3][2] = -(far*near)/(far-near);
    pipeline->proj_matrix.m[2][3] = -1.0f;
}

void t2d_vert_shader(t2d_pipeline *pipeline, vertex_shader shader){
    pipeline->vert_shader = shader;
}

void t2d_frag_shader(t2d_pipeline *pipeline, fragment_shader shader){
    pipeline->frag_shader = shader;
}
