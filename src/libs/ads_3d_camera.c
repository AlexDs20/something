#include "libs/ads_3d_camera.h"
#include "libs/ads_math.h"

static const f32 DEFAULT_SPEED_FACTOR = 600.0f;
static const f32 DEFAULT_ROTATION_SPEED_FACTOR = 0.005f;
// static const f32x3 WORLD_UP = {0.0f, 1.0f, 0.0f};

void camera_debug_print(const Camera* camera) {
    printf("\nPitch: %.3f   Yaw: %.3f   Dir:(%.3f,%.3f,%.3f)   Right:(%.3f,%.3f,%.3f)",
            camera->pitch,
            camera->yaw,
            camera->direction.x,
            camera->direction.y,
            camera->direction.z,
            camera->right.x,
            camera->right.y,
            camera->right.z
            );
}

static void camera_update_direction(Camera* camera) {
    f32 cy = f32_cos(camera->yaw);
    f32 sy = f32_sin(camera->yaw);

    f32 cp = f32_cos(camera->pitch);
    f32 sp = f32_sin(camera->pitch);

    camera->direction = f32x3_make(
        cp * sy,
        sp,
        cp * cy
    );
    camera->right = f32x3_make(
        -cy,
        0.0f,
        sy
    );
}

Camera camera_init(ADSCameraType camera_type) {
    Camera camera;

    camera.camera_type = camera_type;
    camera.speed = DEFAULT_SPEED_FACTOR;
    camera.rotation_speed = DEFAULT_ROTATION_SPEED_FACTOR;

    camera.yaw = F32_PI;
    camera.pitch = 0;

    switch(camera_type) {
        case ADS_CAMERA_FIRST_PERSON: {
            camera.position = f32x3_splat(0.0f);
        } break;
        case ADS_CAMERA_FREE_FLIGHT: {
            camera.position = f32x3_splat(0.0f);
        } break;
        default:{
            NOT_IMPLEMENTED("Other cameras init");
        }
    }
    camera_update_direction(&camera);
    return camera;
}

void camera_move(Camera* camera, MotionDirection direction, f32 delta_time) {
    f32 amount = delta_time * camera->speed;
    if (camera->camera_type == ADS_CAMERA_FREE_FLIGHT) {
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
    else if (camera->camera_type == ADS_CAMERA_FIRST_PERSON) {
        f32x3 horizontal_forward = f32x3_make(f32_sin(camera->yaw), 0.0f, f32_cos(camera->yaw));
        switch (direction) {
            case ADS_MOTION_DIRECTION_FORWARD: {
                camera->position = f32x3_add(camera->position, f32x3_mull_f32(amount, horizontal_forward));
            } break;
            case ADS_MOTION_DIRECTION_BACKWARD: {
                camera->position = f32x3_add(camera->position, f32x3_mull_f32(-amount, horizontal_forward));
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
}

static f32 camera_wrap_yaw(f32 yaw) {
    while (yaw >= F32_PI) {
        yaw -= F32_TAU;
    }
    while (yaw < -F32_PI) {
        yaw += F32_TAU;
    }

    return yaw;
}

void camera_rotate(Camera* camera, s32x2 dx) {
    // dx.y decreases when going up on the screen -> need - for pitch
    switch (camera->camera_type) {
        case ADS_CAMERA_FIRST_PERSON: {
            camera->yaw -= (f32)dx.x * camera->rotation_speed;
            camera->pitch -= (f32)dx.y * camera->rotation_speed;
            camera->pitch = f32_clamp(camera->pitch, -F32_PI_HALF+F32_1DEG, F32_PI_HALF-F32_1DEG);

            camera->yaw = camera_wrap_yaw(camera->yaw);
            camera_update_direction(camera);
        } break;
        case ADS_CAMERA_FREE_FLIGHT: {
            camera->yaw -= (f32)dx.x * camera->rotation_speed;
            camera->pitch -= (f32)dx.y * camera->rotation_speed;
            camera->pitch = f32_clamp(camera->pitch, -F32_PI_HALF+F32_1DEG, F32_PI_HALF-F32_1DEG);

            camera->yaw = camera_wrap_yaw(camera->yaw);
            camera_update_direction(camera);
        } break;
        default: {
            NOT_IMPLEMENTED("Camera rotation for this camera type");
        }
    }
}

f32x4x4 camera_get_view_matrix(const Camera* camera) {
    // Model = Translation(Rotation(x)) = (T * R) * x
    // Here we take the inverse:
    // (T * R) => (T*R)^-1 = R^-1 * T^-1 = R^t * (-T)
    //
    // Rotation matrix is build by making a matrix where each column is one of the vector of the new basis
    f32x3 camera_up = f32x3_cross(camera->right, camera->direction);

    f32x4x4 result = {
        .data = {
             camera->right.x,      camera->right.y,      camera->right.z,      -f32x3_dot(camera->right, camera->position),
             camera_up.x,          camera_up.y,          camera_up.z,          -f32x3_dot(camera_up, camera->position),
            -camera->direction.x, -camera->direction.y, -camera->direction.z,   f32x3_dot(camera->direction, camera->position),
            0.0f, 0.0f, 0.0f, 1.0f,
        }
    };

    return result;
}
