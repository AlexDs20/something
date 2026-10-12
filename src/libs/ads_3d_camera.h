#ifndef ADS_3D_CAMERA_H
#define ADS_3D_CAMERA_H
#include "base/types.h"

typedef enum {
    ADS_CAMERA_NONE,
    ADS_CAMERA_FIRST_PERSON,
    ADS_CAMERA_COUNT,
} ADSCameraType;

typedef struct _Camera {
    ADSCameraType camera_type;

    f32x3 position;

    f32x3 direction;
    f32x3 right;
} Camera;

#ifdef __cplusplus
extern "C" {
#endif
Camera camera_init(ADSCameraType camera_type);

f32x4x4 camera_get_transformation(const Camera camera);
void camera_move_forward(Camera* camera, f32 amount);
void camera_move_right(Camera* camera, f32 amount);

#ifdef __cplusplus
}
#endif

#endif // ADS_3D_CAMERA_H
