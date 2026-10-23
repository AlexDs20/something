#ifndef ADS_3D_CAMERA_H
#define ADS_3D_CAMERA_H
#include "base/types.h"

typedef enum {
    ADS_CAMERA_NONE,
    ADS_CAMERA_FIRST_PERSON,
    ADS_CAMERA_FREE_FLIGHT,
    ADS_CAMERA_COUNT,
} CameraType;

typedef enum {
    ADS_MOTION_DIRECTION_NONE,
    ADS_MOTION_DIRECTION_FORWARD,
    ADS_MOTION_DIRECTION_BACKWARD,
    ADS_MOTION_DIRECTION_LEFT,
    ADS_MOTION_DIRECTION_RIGHT,
    ADS_MOTION_DIRECTION_COUNT,
} MotionDirection;

typedef struct _Camera {
    CameraType camera_type;

    f32x3 position;

    f32x3 direction;
    f32x3 right;

    f32 yaw;            // Left/right
    f32 pitch;          // up/down
    f32 roll;           // tilt side NOT used currently (until plane like camera)

    // Needed for "Perspective" camera
    f32 fov;            // horizongal FOV
    f32 aspect_ratio;
    f32 near_plane;
    f32 far_plane;
    f32 scaling_x;
    f32 scaling_y;

    f32 speed;
    f32 rotation_speed;
} Camera;

#ifdef __cplusplus
extern "C" {
#endif
Camera camera_init(CameraType camera_type);

f32x4x4 camera_get_model_matrix(const Camera* camera);
f32x4x4 camera_get_view_matrix(const Camera* camera);
f32x4x4 camera_get_projection_matrix(const Camera* camera);
void camera_move(Camera* camera, MotionDirection direction, f32 delta_time);
void camera_rotate(Camera* camera, s32x2 dx);

#ifdef __cplusplus
}
#endif

#endif // ADS_3D_CAMERA_H
