#pragma once

#define AF_WAIT_VBLANK 1

#ifdef _MSC_VER
#define NOMINMAX
#define _USE_MATH_DEFINES
#include <Windows.h>

#ifdef AF_GLES31
#include "wgl_grabber_gen.h"
#include "device_man_wgl.h"
#endif

#ifdef AF_VULKAN
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
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
#include <android/log.h>
//#include <gles3/gl31.h>
#include <gles2/gl2.h>
#include <gles2/gl2ext.h>
#endif

#include <cmath>

#include <vector>
#include <algorithm>
#include <string>
#include <map>
#include <stack>
#include <chrono>
#include <memory>
#include <limits>

#include <assert.h>
#include <stdint.h>

#include <../submodules/lua/src/lua.hpp>

static const int BONE_MAX = 50;

#include "af_math.h"
#include "af_lua_helpers.h"
#include "system_misc.h"
#include "helper.h"
#include "helper_text.h"

#ifdef _MSC_VER
#include "helper_win.h"
#endif

#ifdef AF_GLES
#include "helper_gl.h"
#endif

#ifdef AF_VULKAN
#include "helper_vulkan.h"
#endif

#ifdef AF_DX11
#include "helper_dx11.h"
#endif

#ifdef AF_DX12
#include "device_man_dx12.h"
#include "helper_dx12.h"
#endif

#include "AFGraphicsCompatibilityLayer.inl"

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
#include "app.h"
