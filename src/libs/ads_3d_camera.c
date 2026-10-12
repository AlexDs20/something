#include "libs/ads_3d_camera.h"
#include "libs/ads_math.h"

static const f32 DEFAULT_SPEED_FACTOR = 600.0f;
static const f32 DEFALU_ROTATION_SPEED_FACTOR = 0.025f;

static void camera_update_direction(Camera* camera) {
    f32 cy = f32_cos(camera->yaw);
    f32 sy = f32_sin(camera->yaw);

    f32 cp = f32_cos(camera->pitch);
    f32 sp = f32_sin(camera->pitch);

    camera->direction = f32x3_make(
        sy * cp,
        cy * cp,
        sp
    );

    camera->right = f32x3_make(
        cy,
        -sy,
        0.0f
    );
}

Camera camera_init(ADSCameraType camera_type) {
    Camera camera;

    camera.camera_type = camera_type;
    camera.speed = DEFAULT_SPEED_FACTOR;
    camera.rotation_speed = DEFALU_ROTATION_SPEED_FACTOR;

    camera.yaw = F32_PI;
    camera.pitch = 0;

    if (camera_type == ADS_CAMERA_FIRST_PERSON) {
        camera.position = f32x3_splat(0.0f);

        // camera.direction = f32x3_make(0.0f, 0.0f, -1.0f);
        // camera.right = f32x3_make(1.0f, 0.0f, 0.0f);

        camera_update_direction(&camera);
    }
    return camera;
}

void camera_move(Camera* camera, MotionDirection direction, f32 delta_time) {
    f32 amount = delta_time * camera->speed;
    switch (direction) {
        case ADS_MOTION_DIRECTION_FORWARD: {
            camera->position = f32x3_add(camera->position, f32x3_mull_f32(amount, camera->direction));
        } break;
        case ADS_MOTION_DIRECTION_BACKWARD: {
            camera->position = f32x3_add(camera->position, f32x3_mull_f32(-amount, camera->direction));
        } break;
        case ADS_MOTION_DIRECTION_RIGHT: {
            camera->position = f32x3_add(camera->position, f32x3_mull_f32(amount, camera->right));
        } break;
        case ADS_MOTION_DIRECTION_LEFT: {
            camera->position = f32x3_add(camera->position, f32x3_mull_f32(-amount, camera->right));
        } break;
        default: {
            ASSERT(false);
        } break;
    }
}

void camera_rotate(Camera* camera, s32x2 dx) {
    switch (camera->camera_type) {
        case ADS_CAMERA_FIRST_PERSON: {
            camera->yaw += (f32)dx.x * camera->rotation_speed;
            camera->pitch += (f32)dx.y * camera->rotation_speed;
            camera->pitch = f32_clamp(camera->pitch, -F32_PI_HALF+F32_EPSILON, F32_PI_HALF-F32_EPSILON);
        } break;
        default: {
        }
    }

    camera_update_direction(camera);
}

f32x4x4 camera_get_transformation(const Camera camera) {
    f32x4x4 m = f32x4x4_translate(f32x3_subu(camera.position));
    return m;
}
