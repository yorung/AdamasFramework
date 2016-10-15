typedef unsigned short AFIndex;
#define AFIndexTypeToDevice VK_INDEX_TYPE_UINT16

typedef VkFormat AFFormat;
#define AFF_INVALID VK_FORMAT_UNDEFINED
#define AFF_BC1_UNORM VK_FORMAT_BC1_RGBA_UNORM_BLOCK
#define AFF_BC2_UNORM VK_FORMAT_BC2_UNORM_BLOCK
#define AFF_BC3_UNORM VK_FORMAT_BC3_UNORM_BLOCK
#define AFF_R8G8B8A8_UNORM VK_FORMAT_R8G8B8A8_UNORM
#define AFF_R32G32B32_FLOAT VK_FORMAT_R32G32B32_SFLOAT
#define AFF_R32G32_FLOAT VK_FORMAT_R32G32_SFLOAT
#define AFF_R8G8B8A8_UINT VK_FORMAT_R8G8B8A8_UINT
#define AFF_R32_UINT VK_FORMAT_R32_UINT

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

void afSafeDeleteBuffer(BufferContext& buffer);
void afWriteBuffer(BufferContext& buffer, int size, const void* srcData);
BufferContext CreateBuffer(VkDevice device, VkBufferUsageFlags usage, const VkPhysicalDeviceMemoryProperties& memoryProperties, int size, const void* srcData);

typedef BufferContext VBOID;
typedef BufferContext IBOID;
typedef BufferContext UBOID;
VBOID afCreateVertexBuffer(int size, const void* srcData);
VBOID afCreateDynamicVertexBuffer(int size, const void* srcData = nullptr);
IBOID afCreateIndexBuffer(int numIndi, const AFIndex* indi);
UBOID afCreateUBO(int size, const void* srcData = nullptr);
IBOID afCreateQuadListIndexBuffer(int numQuads);

struct AFTexSubresourceData
{
	const void* ptr;
	uint32_t pitch;
	uint32_t pitchSlice;
};

struct TextureContext
{
	VkDevice device = 0;
	VkFormat format = VK_FORMAT_UNDEFINED;
	VkImage image = 0;
	VkDeviceMemory memory = 0;
	VkImageView view = 0;
	VkDescriptorSet descriptorSet = 0;
	TexDesc texDesc;
	bool operator !() const { return !image; }
	bool operator !=(const TextureContext& r) const { return image != r.image; }
	bool operator ==(const TextureContext& r) const { return image == r.image; }
};
typedef TextureContext SRVID;

SRVID afLoadTexture(const char* name, TexDesc& desc);
SRVID LoadTextureViaOS(const char* name, IVec2& size);
SRVID afCreateDynamicTexture(VkFormat format, const IVec2& size, void *image = nullptr);
SRVID afCreateTexture2D(AFFormat format, const struct TexDesc& desc, int mipCount, const AFTexSubresourceData datas[]);
void afWriteTexture(TextureContext& textureContext, const TexDesc& texDesc, void *image);
void afSafeDeleteTexture(TextureContext& textureContext);

void afBindBuffer(VkPipelineLayout pipelineLayout, int size, const void* buf, int descritorSetIndex);
void afBindBuffer(int size, const void* buf, int descritorSetIndex);
void afBindTexture(VkPipelineLayout pipelineLayout, const TextureContext& textureContext, int descritorSetIndex);
void afBindTexture(const TextureContext& textureContext, int descritorSetIndex);

inline IVec2 afGetTextureSize(SRVID tex)
{
	return tex.texDesc.size;
}

void afSetVertexBuffer(VBOID id, int stride);
void afSetIndexBuffer(IBOID id);
void afSetVertexBuffer(int size, const void* buffer, int stride);

void afDrawIndexed(int numIndices, int start = 0, int instanceCount = 1);
void afDraw(int numVertices, int start = 0, int instanceCount = 1);

inline void afSetTextureName(const TextureContext& tex, const char* name)
{
}

class AFRenderStates {
	uint32_t flags = AFRS_NONE;
	VkPipeline pipeline = 0;
	VkPipelineLayout pipelineLayout = 0;
public:
	VkPipelineLayout GetPipelineLayout() { return pipelineLayout; }
	void Create(const char* shaderName, int numInputElements = 0, const InputElement* inputElements = nullptr, uint32_t flags = AFRS_NONE, int numSamplers = 0, const SamplerType samplerTypes[] = nullptr);
	void Apply();
	void Destroy();
	bool IsReady() { return pipeline != 0; }
};

#include "AFDynamicQuadListVertexBuffer.inl"

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
	TextureContext depthStencil;
	VkSemaphore semaphore = 0;
	VkCommandPool commandPool = 0;
	VkRenderPass renderPass = 0;
	VkPipelineCache pipelineCache = 0;
	uint32_t frameIndex = 0;
	RECT rc = {};
	VkViewport viewport;
	VkRect2D scissor;
public:
	VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
	VkPhysicalDevice physicalDevice = nullptr;
	VkDevice GetDevice() { return device; }
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
	void BeginScene();
	void Flush();
	VkPipeline CreatePipeline(const char* name, VkPipelineLayout pipelineLayout, uint32_t numAttributes, const VkVertexInputAttributeDescription attributes[], uint32_t flags);
};
extern DeviceManVK deviceMan;
