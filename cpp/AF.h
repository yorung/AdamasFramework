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
#include <memory>
#include <limits>

#include <assert.h>
#include <stdint.h>

#include "af_math.h"

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
#include "helper_dx12.h"
#include "device_man_dx12.h"
#endif

#include "AFGraphicsCompatibilityLayer.inl"


