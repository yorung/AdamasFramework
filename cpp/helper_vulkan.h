#include "AFGraphicsDefinitions.inl"

typedef unsigned short AFIndex;
#define AFIndexTypeToDevice VK_INDEX_TYPE_UINT16

typedef VkFormat AFFormat;
#define AFF_INVALID VK_FORMAT_UNDEFINED
#define AFF_BC1_UNORM VK_FORMAT_BC1_RGBA_UNORM_BLOCK
#define AFF_BC2_UNORM VK_FORMAT_BC2_UNORM_BLOCK
#define AFF_BC3_UNORM VK_FORMAT_BC3_UNORM_BLOCK
#define AFF_R8G8B8A8_UNORM VK_FORMAT_R8G8B8A8_UNORM
#define AFF_B8G8R8A8_UNORM VK_FORMAT_B8G8R8A8_UNORM
#define AFF_R16G16B16A16_FLOAT VK_FORMAT_R16G16B16A16_SFLOAT
#define AFF_R32G32B32A32_FLOAT VK_FORMAT_R32G32B32A32_SFLOAT
#define AFF_R32G32B32_FLOAT VK_FORMAT_R32G32B32_SFLOAT
#define AFF_R32G32_FLOAT VK_FORMAT_R32G32_SFLOAT
#define AFF_R8G8B8A8_UINT VK_FORMAT_R8G8B8A8_UINT
#define AFF_D32_FLOAT VK_FORMAT_D32_SFLOAT
#define AFF_R32_FLOAT VK_FORMAT_R32_SFLOAT
#define AFF_R32_UINT VK_FORMAT_R32_UINT
#define AFF_R32_TYPELESS VK_FORMAT_R32_SFLOAT
#define AFF_D24_UNORM_S8_UINT VK_FORMAT_D24_UNORM_S8_UINT
#define AFF_D32_FLOAT_S8_UINT VK_FORMAT_D32_SFLOAT_S8_UINT

typedef VkVertexInputAttributeDescription InputElement;
class CInputElement : public InputElement
{
public:
	CInputElement(uint32_t location_, VkFormat format_, int offset_)
	{
		location = location_;
		format = format_;
		offset = offset_;
		binding = 0;
	}
};

VkResult _afHandleVKError(const char* file, const char* func, int line, const char* command, VkResult result);
#define afHandleVKError(command) do{ _afHandleVKError(__FILE__, __FUNCTION__, __LINE__, #command, command); } while(0)

template <typename Deleter, class Object>
inline void afSafeDeleteVk(Deleter deleter, VkDevice device, Object& object)
{
	if (object)
	{
		deleter(device, object, nullptr);
		object = 0;
	}
}

inline void afSafeUnmapVk(VkDevice device, VkDeviceMemory memory, void*& mappedMemory)
{
	if (mappedMemory)
	{
		vkUnmapMemory(device, memory);
		mappedMemory = nullptr;
	}
}

struct BufferContext
{
	VkDevice device = 0;
	VkBuffer buffer = 0;
	VkDeviceMemory memory = 0;
	VkDeviceSize size = 0;
	void* mappedMemory = nullptr;
	VkMemoryRequirements memoryRequirement = {};
	bool operator !() { return !buffer; }
};
typedef BufferContext AFBufferResource;

void afSafeDeleteBuffer(BufferContext& buffer);
void afWriteBuffer(BufferContext& buffer, int size, const void* srcData);

AFBufferResource afCreateBuffer(int size, const void* srcData, AFBufferType bufferType);

struct AFTexSubresourceData
{
	const void* ptr;
	uint32_t pitch;
	uint32_t pitchSlice;
};

class TextureContext
{
public:
	VkDevice device = 0;
	VkFormat format = VK_FORMAT_UNDEFINED;
	VkImage image = 0;
	VkDeviceMemory memory = 0;
	VkImageView view = 0;
	VkDescriptorSet descriptorSet = 0;
	TexDesc texDesc;
	int mipCount;
	~TextureContext();
};
typedef std::shared_ptr<TextureContext> SRVID;
typedef SRVID AFTexRef;

AFTexRef afCreateDynamicTexture(VkFormat format, const IVec2& size, uint32_t flags = AFTF_CPU_WRITE | AFTF_SRV);
AFTexRef afCreateTexture2D(AFFormat format, const struct TexDesc& desc, int mipCount, const AFTexSubresourceData datas[]);
void afWriteTexture(AFTexRef textureContext, const TexDesc& texDesc, void *image);
inline void afSafeDeleteTexture(AFTexRef& ref) { ref.reset(); }

void afBindBuffer(VkPipelineLayout pipelineLayout, int size, const void* buf, int descritorSetIndex);
void afBindTexture(VkPipelineLayout pipelineLayout, AFTexRef textureContext, int descritorSetIndex);

inline IVec2 afGetTextureSize(AFTexRef tex)
{
	return tex->texDesc.size;
}

void afSetVertexBuffer(AFBufferResource id, int stride);
void afSetIndexBuffer(AFBufferResource id);
void afSetVertexBuffer(int size, const void* buffer, int stride);

void afDrawIndexed(int numIndices, int start = 0, int instanceCount = 1);
void afDraw(int numVertices, int start = 0, int instanceCount = 1);

void afSetTextureName(AFTexRef tex, const char* name);

class AFRenderStates
{
	VkPipeline pipeline = 0;
	VkPipelineLayout pipelineLayout = 0;
public:
	~AFRenderStates() { Destroy(); }
	VkPipelineLayout GetPipelineLayout() { return pipelineLayout; }
	void Create(const char* shaderName, int numInputElements = 0, const InputElement* inputElements = nullptr, uint32_t flags = AFRS_NONE, int numSamplers = 0, const SamplerType samplerTypes[] = nullptr);
	void Apply();
	void Destroy();
	bool IsReady() { return pipeline != 0; }
};

class AFRenderTarget
{
	VkFramebuffer framebuffer = 0;
	AFTexRef renderTarget;
	AFTexRef depthStencil;
	bool currentStateIsRtv = false;
public:
	~AFRenderTarget() { Destroy(); }
	void Init(IVec2 size, AFFormat colorFormat, AFFormat depthStencilFormat = AFF_INVALID);
	void Destroy();
	void BeginRenderToThis();
	void EndRenderToThis();
	AFTexRef GetTexture();
};

class AFBufferStackAllocator
{
	VkDeviceSize allocatedSize = 0;
public:
	BufferContext bufferContext;
	void Create(VkBufferUsageFlags usage, int size);
	uint32_t Allocate(int size, const void* data);
	void ResetAllocation();
	void Destroy();
	~AFBufferStackAllocator() { assert(!bufferContext); }
};

class DeviceManVK
{
	VkDevice device = nullptr;
	VkInstance inst = nullptr;
	VkSurfaceKHR surface = 0;
	VkSwapchainKHR swapchain = 0;
	uint32_t swapChainCount = 0;
	VkImage swapChainImages[8] = {};
	VkImageView imageViews[8] = {};
	VkFramebuffer framebuffers[8] = {};
	VkSemaphore semaphore = 0;
	VkCommandPool commandPool = 0;
	VkPipelineCache pipelineCache = 0;
	uint32_t frameIndex = 0;
	RECT rc = {};
public:
	VkRenderPass primaryRenderPass = 0, offscreenR8G8B8A8D32S8RenderPass = 0, offscreenR16G16B16A16D32S8RenderPass = 0, offscreenR8G8B8A8RenderPass = 0, offscreenR16G16B16A16RenderPass = 0;
	VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
	VkPhysicalDevice physicalDevice = nullptr;
	VkDevice GetDevice() { return device; }
	VkPipelineCache GetPipelineCache() { return pipelineCache; }
	VkCommandBuffer commandBuffer = 0;
	VkDescriptorPool descriptorPool = 0;
	AFBufferStackAllocator uboAllocator;
	AFBufferStackAllocator vertexBufferAllocator;
	VkDescriptorSetLayout commonUboDescriptorSetLayout = 0;
	VkDescriptorSetLayout commonTextureDescriptorSetLayout = 0;
	VkDescriptorSet commonUboDescriptorSet = 0;
	VkSampler sampler = 0;
	void Create(HWND hWnd);
	void Present();
	void Destroy();
	void BeginRenderPass(VkRenderPass nextRenderPass, VkFramebuffer nextFramebuffer, IVec2 size, bool needDepth);
	void BeginRenderToSwapChain();
	void EndRenderPass();
	void Flush();
};
extern DeviceManVK deviceMan;

inline void afBeginRenderToSwapChain()
{
	deviceMan.BeginRenderToSwapChain();
}

inline void afEndRenderToSwapChain()
{
	deviceMan.EndRenderPass();
}

class AFCommandList
{
	AFRenderStates* currentRS = nullptr;
public:
	void SetRenderStates(AFRenderStates& rs)
	{
		rs.Apply();
		currentRS = &rs;
	}
	void SetTexture(AFTexRef texId, int descritorSetIndex)
	{
		afBindTexture(currentRS->GetPipelineLayout(), texId, descritorSetIndex);
	}
	void SetBuffer(int size, const void* buf, int descritorSetIndex)
	{
		afBindBuffer(currentRS->GetPipelineLayout(), size, buf, descritorSetIndex);
	}
	void SetVertexBuffer(int size, const void* buf, int stride)
	{
		afSetVertexBuffer(size, buf, stride);
	}
	void SetVertexBuffer(AFBufferResource vertexBuffer, int stride)
	{
		afSetVertexBuffer(vertexBuffer, stride);
	}
	void SetIndexBuffer(AFBufferResource indexBuffer)
	{
		afSetIndexBuffer(indexBuffer);
	}
	void Draw(int numVertices, int start = 0, int instanceCount = 1)
	{
		afDraw(numVertices, start, instanceCount);
	}
	void DrawIndexed(int numVertices, int start = 0, int instanceCount = 1)
	{
		afDrawIndexed(numVertices, start, instanceCount);
	}
};

#include "AFGraphicsFunctions.inl"
#include "AFDynamicQuadListVertexBuffer.inl"
