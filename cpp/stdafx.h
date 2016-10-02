#pragma once

#ifdef _MSC_VER
#define NOMINMAX
#define _USE_MATH_DEFINES
#include <Windows.h>

#ifdef AF_GLES31
#include "wgl_grabber_gen.h"
#include "device_man_wgl.h"
#endif

#ifdef AF_DX11
#include <d3d11.h>
#include <wrl.h>	// ComPtr
using Microsoft::WRL::ComPtr;
#endif

#ifdef AF_DX12
#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl.h>
using Microsoft::WRL::ComPtr;
#endif

#else
#include <jni.h>
#include <android/log.h>
extern JNIEnv* jniEnv;
extern const char* boundJavaClass;
#include <gles3/gl31.h>
#endif

#include <cmath>

#include <vector>
#include <algorithm>
#include <string>
#include <map>
#include <stack>
#include <chrono>
#include <memory>

#include <assert.h>
#include <stdint.h>

#include <../submodules/lua/src/lua.hpp>

static const int BONE_MAX = 50;

#include "af_math.h"
#include "af_lua_helpers.h"
#include "system_misc.h"
#include "helper.h"
#include "helper_text.h"
#include "helper_win.h"

#ifdef AF_GLES31
#include "shader_man_gl.h"
#include "helper_gl.h"
#endif

#ifdef AF_DX11
#include "shader_man_dx11.h"
#include "device_man_dx11.h"
#include "helper_dx11.h"
#endif

#ifdef AF_DX12
#include "device_man_dx12.h"
#include "helper_dx12.h"
#endif

#include "matrix_man.h"
#include "matrix_stack.h"
#include "tex_man.h"
#include "dib.h"
#include "fps.h"
#include "font_man.h"
#include "sky_man.h"
#include "glow.h"
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
#include "hub.h"
#include "app.h"
