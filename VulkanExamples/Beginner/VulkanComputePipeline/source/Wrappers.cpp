/*

*


*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
* DEALINGS IN THE SOFTWARE.
*/

#include "Wrappers.h"
#include "VulkanApplication.h"

void CommandBufferMgr::allocCommandBuffer(const VkDevice* device, const VkCommandPool cmdPool, VkCommandBuffer* cmdBuf, const VkCommandBufferAllocateInfo* commandBufferInfo)
{
	// Dependency on the intialize SwapChain Extensions and initialize CommandPool
	VkResult result;

	// If command information is available use it as it is.
	if (commandBufferInfo) {
		result = vkAllocateCommandBuffers(*device, commandBufferInfo, cmdBuf);
		assert(!result);
		return;
	}

	// Default implementation, create the command buffer
	// allocation info and use the supplied parameter into it
	VkCommandBufferAllocateInfo cmdInfo = {};
	cmdInfo.sType		= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdInfo.pNext		= NULL;
	cmdInfo.commandPool = cmdPool;
	cmdInfo.level		= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdInfo.commandBufferCount = (uint32_t) sizeof(cmdBuf) / sizeof(VkCommandBuffer);;

	result = vkAllocateCommandBuffers(*device, &cmdInfo, cmdBuf);
	assert(!result);
}

void CommandBufferMgr::beginCommandBuffer(VkCommandBuffer cmdBuf, VkCommandBufferBeginInfo* inCmdBufInfo)
{
	// Dependency on  the initialieCommandBuffer()
	VkResult  result;
	// If the user has specified the custom command buffer use it
	if (inCmdBufInfo) {
		result = vkBeginCommandBuffer(cmdBuf, inCmdBufInfo);
		assert(result == VK_SUCCESS);
		return;
	}

	// Otherwise, use the default implementation.
	// Note: if required to specify VkCommandBufferInheritanceInfo, then set it out side in VkCommandBufferBeginInfo. Do not use defualt implmentation
	//VkCommandBufferInheritanceInfo cmdBufInheritInfo = {};
	//cmdBufInheritInfo.sType					= VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
	//cmdBufInheritInfo.pNext					= NULL;
	//cmdBufInheritInfo.renderPass			= VK_NULL_HANDLE;
	//cmdBufInheritInfo.subpass				= 0;
	//cmdBufInheritInfo.framebuffer			= VK_NULL_HANDLE;
	//cmdBufInheritInfo.occlusionQueryEnable	= VK_FALSE;
	//cmdBufInheritInfo.queryFlags			= 0;
	//cmdBufInheritInfo.pipelineStatistics	= 0;
	
	VkCommandBufferBeginInfo cmdBufInfo = {};
	cmdBufInfo.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufInfo.pNext				= NULL;
	cmdBufInfo.flags				= 0;
	cmdBufInfo.pInheritanceInfo		= NULL; // &cmdBufInheritInfo;

	result = vkBeginCommandBuffer(cmdBuf, &cmdBufInfo);

	assert(result == VK_SUCCESS);
}

void CommandBufferMgr::endCommandBuffer(VkCommandBuffer commandBuffer)
{
	VkResult  result;
	result = vkEndCommandBuffer(commandBuffer);
	assert(result == VK_SUCCESS);
}

void CommandBufferMgr::submitCommandBuffer(const VkQueue& queue, const VkCommandBuffer* commandBuffer, const VkSubmitInfo* inSubmitInfo, const VkFence& fence)
{
	VkResult result;
	
	// If Subimt information is avialable use it as it is, this assumes that 
	// the commands are already specified in the structure, hence ignore command buffer 
	if (inSubmitInfo){
		result = vkQueueSubmit(queue, 1, inSubmitInfo, fence);
		assert(!result);

		result = vkQueueWaitIdle(queue);
		assert(!result);
		return;
	}

	// Otherwise, create the submit information with specified buffer commands
	VkSubmitInfo submitInfo = {};
	submitInfo.sType				= VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext				= NULL;
	submitInfo.waitSemaphoreCount	= 0;
	submitInfo.pWaitSemaphores		= NULL;
	submitInfo.pWaitDstStageMask	= NULL;
	submitInfo.commandBufferCount	= (uint32_t)sizeof(commandBuffer)/sizeof(VkCommandBuffer);
	submitInfo.pCommandBuffers		= commandBuffer;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores	= NULL;

	result = vkQueueSubmit(queue, 1, &submitInfo, fence);
	assert(!result);

	result = vkQueueWaitIdle(queue);
	assert(!result);
}

// PPM parser implementation
PpmParser::PpmParser()
{
	isValid			= false;
	imageWidth		= 0;
	imageHeight		= 0;
	ppmFile			= "invalid file name";
	dataPosition	= 0;
}

PpmParser::~PpmParser()
{

}

int32_t PpmParser::getImageWidth()
{
	return imageWidth;
}

int32_t PpmParser::getImageHeight()
{
	return imageHeight;
}

bool PpmParser::getHeaderInfo(const char *filename)
{
	tex2D = new gli::texture2D(gli::load(filename));
	imageHeight = static_cast<uint32_t>(tex2D[0].dimensions().x);
	imageWidth  = static_cast<uint32_t>(tex2D[0].dimensions().y);
	return true;
}

bool PpmParser::loadImageData(int rowPitch, uint8_t *data)
{
	uint8_t* dataTemp = (uint8_t*)tex2D->data();
	for (int y = 0; y < imageHeight; y++)
	{
		size_t imageSize = imageWidth * 4;
		memcpy(data, dataTemp, imageSize);
		dataTemp += imageSize;

		// Advance row by row pitch information
		data += rowPitch;
	}

	return true;
}

void* readFile(const char *spvFileName, size_t *fileSize) {

	FILE *fp = fopen(spvFileName, "rb");
	if (!fp) {
		return NULL;
	}

	size_t retval;
	long int size;

	fseek(fp, 0L, SEEK_END);
	size = ftell(fp);

	fseek(fp, 0L, SEEK_SET);

	void* spvShader = malloc(size+1); // Plus for NULL character '\0'
	memset(spvShader, 0, size+1);

	retval = fread(spvShader, size, 1, fp);
	assert(retval == 1);

	*fileSize = size;
	fclose(fp);
	return spvShader;
}

#include "VulkanDevice.h"
// This method allocates a big chunk of memory block, it can be used for suballocation purposes
VkBool32 allocateMemory(VulkanDevice* deviceObj, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void * data, VkDeviceMemory * memory)
{
	VkMemoryRequirements memReqs;
	VkMemoryAllocateInfo memAllocInfo = {};
	memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAllocInfo.pNext = NULL;
	memAllocInfo.memoryTypeIndex = 0;
	memAllocInfo.allocationSize = 0;

	// Create buffer resource states using VkBufferCreateInfo
	VkBufferCreateInfo bufInfo = {};
	bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufInfo.pNext = NULL;
	bufInfo.usage = usageFlags;
	bufInfo.size = size;
	bufInfo.flags = 0;

	VkBuffer buffer; // Temporary buffer object used for querying the memory requirements
	vkCreateBuffer(deviceObj->device, &bufInfo, nullptr, &buffer);

	vkGetBufferMemoryRequirements(deviceObj->device, buffer, &memReqs);
	memAllocInfo.allocationSize = memReqs.size;
	deviceObj->memoryTypeFromProperties(memoryPropertyFlags, &memAllocInfo.memoryTypeIndex);
	vkAllocateMemory(deviceObj->device, &memAllocInfo, nullptr, memory);

	if (data)
	{
		int32_t *mappedMem;
		vkMapMemory(deviceObj->device, *memory, 0, size, 0, (void**)&mappedMem);
		memcpy(mappedMem, data, size);
		vkUnmapMemory(deviceObj->device, *memory);
	}

	return VK_TRUE;
}

// double check the if the pointer are required, objects are already handles
VkBool32 createBuffer(VulkanDevice*	deviceObj, VkDeviceMemory * memory, VkBufferUsageFlags usageFlags, std::vector<BufferList>& vkBufferList)
{
	for (int i = 0; i < vkBufferList.size(); i++)
	{
		VkBufferCreateInfo bufInfo = {};
		bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufInfo.pNext = NULL;
		bufInfo.usage = usageFlags;
		bufInfo.size = vkBufferList[i].size;
		bufInfo.flags = 0;

		vkCreateBuffer(deviceObj->device, &bufInfo, nullptr, &vkBufferList[i].buffer);
		vkBindBufferMemory(deviceObj->device, vkBufferList[i].buffer, *memory, vkBufferList[i].index);
	}

	return VK_TRUE;
}

VkBool32 createBuffer(VulkanDevice*	deviceObj, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void * data, VkBuffer * buffer, VkDeviceMemory * memory)
{
	VkMemoryRequirements memReqs;

	VkMemoryAllocateInfo memAllocInfo = {};
	memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAllocInfo.pNext = NULL;
	memAllocInfo.memoryTypeIndex = 0;
	memAllocInfo.allocationSize = 0;

	//VkBufferCreateInfo bufferCreateInfo = vkTools::initializers::bufferCreateInfo(usageFlags, size);
	// Create buffer resource states using VkBufferCreateInfo
	VkBufferCreateInfo bufInfo = {};
	bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufInfo.pNext = NULL;
	bufInfo.usage = usageFlags;
	bufInfo.size = size;
	bufInfo.flags = 0;

	vkCreateBuffer(deviceObj->device, &bufInfo, nullptr, buffer);

	vkGetBufferMemoryRequirements(deviceObj->device, *buffer, &memReqs);
	memAllocInfo.allocationSize = memReqs.size;
	deviceObj->memoryTypeFromProperties(memReqs.memoryTypeBits, &memAllocInfo.memoryTypeIndex);
	vkAllocateMemory(deviceObj->device, &memAllocInfo, nullptr, memory);
	if (data != nullptr)
	{
		void *mapped;
		vkMapMemory(deviceObj->device, *memory, 0, size, 0, &mapped);
		memcpy(mapped, data, size);
		vkUnmapMemory(deviceObj->device, *memory);
	}
	vkBindBufferMemory(deviceObj->device, *buffer, *memory, 0);

	return true;
}
