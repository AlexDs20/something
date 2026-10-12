#include "libs/ads_3d_camera.h"
#include "libs/ads_math.h"

Camera camera_init(ADSCameraType camera_type) {
    Camera camera;
    camera.camera_type = camera_type;

    if (camera_type == ADS_CAMERA_FIRST_PERSON) {
        camera.position = f32x3_splat(0.0f);
        camera.direction = f32x3_make(0.0f, 1.0f, 0.0f);
        camera.right = f32x3_make(1.0f, 0.0f, 0.0f);
    }
    return camera;
}


f32x4x4 camera_get_transformation(const Camera camera) {
    f32x4x4 m = f32x4x4_translate(f32x3_subu(camera.position));
    return m;
}

void camera_move_forward(Camera* camera, f32 amount) {
    camera->position = f32x3_add(camera->position, f32x3_mull_f32(amount, camera->direction));
}

void camera_move_right(Camera* camera, f32 amount) {
    camera->position = f32x3_add(camera->position, f32x3_mull_f32(amount, camera->right));
}
