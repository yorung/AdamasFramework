#pragma once

#ifdef _MSC_VER
#define NOMINMAX
#include <Windows.h>
#include "wgl_grabber_gen.h"
#else
#include <jni.h>
#include <android/log.h>
extern JNIEnv* jniEnv;
extern const char* boundJavaClass;
#include <gles3/gl31.h>
#endif

#define _USE_MATH_DEFINES
#include <cmath>

#include <vector>
#include <algorithm>
#include <string>
#include <map>
#include <deque>
#include <stack>
#include <chrono>
#include <random>
#include <functional>

#include <assert.h>
#include <time.h>
#include <stdint.h>
#include <ctype.h>

#include <../submodules/lua/src/lua.hpp>

static const int BONE_MAX = 50;

#define AF_GLES31

#include "af_math.h"
#include "af_lua_helpers.h"
#include "system_misc.h"
#include "hub.h"
#include "helper_gl.h"
#include "helper.h"
#include "helper_text.h"
#include "matrix_man.h"
#include "matrix_stack.h"
#include "tex_man.h"
#include "shader_man.h"
#include "dib.h"
#include "fps.h"
#include "font_man.h"
#include "sky_man.h"
#include "glow.h"
#include "water_surface.h"
#include "water_surface_classic.h"
#include "dev_camera.h"
#include "joint_db.h"
#include "mesh_renderer.h"
#include "mesh_x.h"
#include "debug_renderer.h"
#include "bvh.h"
#include "mesh_man.h"
#include "sprite_renderer.h"
#include "stock_objects.h"
#include "letterbox.h"
#include "input_man.h"
#include "voice.h"
#include "lua_man.h"
#include "lua_bind.h"
#include "app.h"
