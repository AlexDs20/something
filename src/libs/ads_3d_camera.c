#include "libs/ads_3d_camera.h"
#include "libs/ads_math.h"

static const f32 DEFAULT_SPEED_FACTOR = 1.0f;
static const f32 DEFAULT_ROTATION_SPEED_FACTOR = 0.005f;
static const f32 DEFAULT_FOV = F32_PI / 2;
static const f32 DEFAULT_ASPECT_RATIO = 16.0f / 9.0f;
static const f32 DEFAULT_NEAR_PLANE = 0.1f;
static const f32 DEFAULT_FAR_PLANE = 100.0f;
static const f32 DEFAULT_YAW = F32_PI;
static const f32 DEFAULT_PITCH = 0.0f;

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

f32 camera_horizontal_fov(const Camera* camera) {
    return camera->fov;
}

f32 camera_vertical_fov(const Camera* camera) {
    return 2.0f * f32_atan( (1.0f / camera->aspect_ratio) * f32_tan(camera->fov / 2.0f));
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

Camera camera_init(CameraType camera_type) {
    Camera camera;

    camera.camera_type = camera_type;
    camera.position = f32x3_splat(0.0f);

    camera.yaw = DEFAULT_YAW;
    camera.pitch = DEFAULT_PITCH;

    camera.fov = DEFAULT_FOV;
    camera.aspect_ratio = DEFAULT_ASPECT_RATIO;
    camera.near_plane = DEFAULT_NEAR_PLANE;
    camera.far_plane = DEFAULT_FAR_PLANE;

    camera.scaling_x = 1.0f / f32_tan(camera.fov / 2);
    camera.scaling_y = camera.scaling_x / camera.aspect_ratio;

    camera.speed = DEFAULT_SPEED_FACTOR;
    camera.rotation_speed = DEFAULT_ROTATION_SPEED_FACTOR;

    switch(camera_type) {
        case ADS_CAMERA_FIRST_PERSON: {
        } break;
        case ADS_CAMERA_FREE_FLIGHT: {
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

f32x4x4 camera_get_model_matrix(const Camera* camera) {
    f32x3 camera_up = f32x3_cross(camera->right, camera->direction);
#if 0
    f32x4x4 R = {
        .data = {
            camera->right.x, camera_up.x, -camera->direction.x,  0.0f,
            camera->right.y, camera_up.y, -camera->direction.y,  0.0f,
            camera->right.z, camera_up.z, -camera->direction.z,  0.0f,
            0.0f,            0.0f,        0.0f,                  1.0f,
        }
    };

    f32x4x4 T = {
        .data = {
            1.0f, 0.0f, 0.0f,  camera->position.x,
            0.0f, 1.0f, 0.0f,  camera->position.y,
            0.0f, 0.0f, 1.0f,  camera->position.z,
            0.0f, 0.0f, 0.0f,  1.0f,
        }
    };

    f32x4x4 result = f32x4x4_mul(&T, &R);
#else
    // Same but skipping the matmul
    f32x4x4 result = {
        .data = {
            camera->right.x, camera_up.x, -camera->direction.x, camera->position.x,
            camera->right.y, camera_up.y, -camera->direction.y, camera->position.y,
            camera->right.z, camera_up.z, -camera->direction.z, camera->position.z,
            0.0f,            0.0f,        0.0f,                 1.0f,
        }
    };
#endif
    return result;
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

f32x4x4 camera_get_projection_matrix(const Camera* camera) {
    // This perspective camera maps to clip space where the near and far planes 
    // are mapped to [-1, 0]  with 0 the near plane and -1 the far plane
    // This is just so that we keep the intuition that the camera always looks in -Z direction

    f32x4x4 result;
    switch (camera->camera_type) {
        case ADS_CAMERA_FREE_FLIGHT:
        case ADS_CAMERA_FIRST_PERSON: {
            const f32 n = camera->near_plane;
            const f32 f = camera->far_plane;
            const f32 sx = camera->scaling_x;
            const f32 sy = camera->scaling_y;

            result = (f32x4x4){
                .data = {
                    sx,   0.0f, 0.0f,      0.0f,
                    0.0f, sy,   0.0f,      0.0f,
                    0.0f, 0.0f, f / (f-n), f*n / (f-n),
                    0.0f, 0.0f, -1.0f,     0.0f,
                }
            };
        } break;
        default: {
            result = f32x4x4_identity();
        }
    }

    return result;
}
