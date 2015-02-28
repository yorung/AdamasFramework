#ifdef _MSC_VER
#define NOMINMAX
#include <Windows.h>
#include "wgl_grabber_gen.h"
#else
#include <jni.h>
#include <android/log.h>
extern JNIEnv* jniEnv;
extern const char* boundJavaClass;
#include <gles2/gl2.h>
#include <gles2/gl2ext.h>
#endif

#include <vector>
#include <algorithm>
#include <string>
#include <map>
#include <deque>

#define _USE_MATH_DEFINES
#include <math.h>
#include <assert.h>
#include <time.h>
#include <stdint.h>
#include <ctype.h>

static const int BONE_MAX = 50;

#include "af_math.h"
#include "hub.h"
#include "helper.h"
#include "helper_gldx.h"
#include "helper_text.h"
#include "matrix_man.h"
#include "tex_man.h"
#include "shader_man.h"
#include "dib.h"
#include "fps.h"
#include "font_man.h"
#include "water_surface.h"
#include "app.h"
#include "joint_db.h"
#include "mat_man.h"
#include "mesh_renderer.h"
#include "mesh_x.h"
#include "debug_renderer.h"
#include "bvh.h"
