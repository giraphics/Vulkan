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

#include "VulkanDevice.h"
#include "VulkanInstance.h"
#include "VulkanApplication.h"

VulkanDevice::VulkanDevice(VkPhysicalDevice* physicalDevice) 
{
	gpu = physicalDevice;
}

VulkanDevice::~VulkanDevice() 
{
}

// Note: This function requires queue object to be in existence before
VkResult VulkanDevice::createDevice(std::vector<const char *>& layers, std::vector<const char *>& extensions)
{
	layerExtension.appRequestedLayerNames		= layers;
	layerExtension.appRequestedExtensionNames	= extensions;

	// Create Device with available queue information.

	VkResult result;
	float queuePriorities[1]			= { 1.0 };
	VkDeviceQueueCreateInfo queueInfo	= {};
	queueInfo.queueFamilyIndex			= computeQueueIndex; // Graphics with Compute Queue
	queueInfo.sType						= VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueInfo.pNext						= NULL;
	queueInfo.queueCount				= 1;
	queueInfo.pQueuePriorities			= queuePriorities;


	vkGetPhysicalDeviceFeatures(*gpu, &deviceFeatures);

	VkPhysicalDeviceFeatures setEnabledFeatures = {VK_FALSE};
	setEnabledFeatures.samplerAnisotropy = deviceFeatures.samplerAnisotropy;

	VkDeviceCreateInfo deviceInfo		= {};
	deviceInfo.sType					= VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.pNext					= NULL;
	deviceInfo.queueCreateInfoCount		= 1;
	deviceInfo.pQueueCreateInfos		= &queueInfo;
	deviceInfo.enabledLayerCount		= 0;
	deviceInfo.ppEnabledLayerNames		= NULL;											// Device layers are deprecated
	deviceInfo.enabledExtensionCount	= (uint32_t)extensions.size();
	deviceInfo.ppEnabledExtensionNames	= extensions.size() ? extensions.data() : NULL;
	deviceInfo.pEnabledFeatures			= &setEnabledFeatures;

	result = vkCreateDevice(*gpu, &deviceInfo, NULL, &device);
	assert(result == VK_SUCCESS);

	return result;
}

bool VulkanDevice::memoryTypeFromProperties(uint32_t typeBits, VkFlags requirementsMask, uint32_t *typeIndex)
{
	// Search memtypes to find first index with those properties
	for (uint32_t i = 0; i < 32; i++) {
		if ((typeBits & 1) == 1) {
			// Type is available, does it match user properties?
			if ((memoryProperties.memoryTypes[i].propertyFlags & requirementsMask) == requirementsMask) {
				*typeIndex = i;
				return true;
			}
		}
		typeBits >>= 1;
	}
	// No memory types matched, return failure
	return false;
}

bool VulkanDevice::memoryTypeFromProperties(VkFlags requirementsMask, uint32_t *typeIndex)
{
	// Search memtypes to find first index with those properties
	for (uint32_t i = 0; i < 32; i++) {
		// Type is available, does it match user properties?
		if ((memoryProperties.memoryTypes[i].propertyFlags & requirementsMask) == requirementsMask) {
			*typeIndex = i;
			return true;
		}
	}
	// No memory types matched, return failure
	return false;
}

void VulkanDevice::getPhysicalDeviceQueuesAndProperties()
{
	// Query queue families count with pass NULL as second parameter.
	vkGetPhysicalDeviceQueueFamilyProperties(*gpu, &queueFamilyCount, NULL);
	
	// Allocate space to accomodate Queue properties.
	queueFamilyProps.resize(queueFamilyCount);

	// Get queue family properties
	vkGetPhysicalDeviceQueueFamilyProperties(*gpu, &queueFamilyCount, queueFamilyProps.data());
}

VkResult VulkanDevice::getGraphicsQueueHandle()
{
	//	1. Get the number of Queues supported by the Physical device
	//	2. Get the properties each Queue type or Queue Family
	//			There could be 4 Queue type or Queue families supported by physical device - 
	//			Graphics Queue	- VK_QUEUE_GRAPHICS_BIT 
	//			Compute Queue	- VK_QUEUE_COMPUTE_BIT
	//			DMA				- VK_QUEUE_TRANSFER_BIT
	//			Sparse memory	- VK_QUEUE_SPARSE_BINDING_BIT
	//	3. Get the index ID for the required Queue family, this ID will act like a handle index to queue.

	bool found = false;
	// 1. Iterate number of Queues supported by the Physical device
	for (unsigned int i = 0; i < queueFamilyCount; i++){
		// 2. Get the Graphics Queue type
		//		There could be 4 Queue type or Queue families supported by physical device - 
		//		Graphics Queue		- VK_QUEUE_GRAPHICS_BIT 
		//		Compute Queue		- VK_QUEUE_COMPUTE_BIT
		//		DMA/Transfer Queue	- VK_QUEUE_TRANSFER_BIT
		//		Sparse memory		- VK_QUEUE_SPARSE_BINDING_BIT

		if (queueFamilyProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT){
			// 3. Get the handle/index ID of graphics queue family.
			found				= true;
			graphicsQueueIndex	= i;
			return VK_SUCCESS;
		}
	}

	return VK_ERROR_INITIALIZATION_FAILED;
}

VkResult VulkanDevice::getComputeQueueHandle()
{
	// first try and find a queue that has just the compute bit set
	for (uint32_t i = 0; i < queueFamilyCount; i++) {
		// mask out the sparse binding bit that we aren't caring about (yet!) and the transfer bit
		const VkQueueFlags maskedFlags = (~(VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT) & queueFamilyProps[i].queueFlags);

		if (!(VK_QUEUE_GRAPHICS_BIT & maskedFlags) && (VK_QUEUE_COMPUTE_BIT & maskedFlags)) {
			computeQueueIndex = i;
			return VK_SUCCESS;
		}
	}

	// lastly get any queue that'll work for us
	for (uint32_t i = 0; i < queueFamilyCount; i++) {
		// mask out the sparse binding bit that we aren't caring about (yet!) and the transfer bit
		const VkQueueFlags maskedFlags = (~(VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT) &
			queueFamilyProps[i].queueFlags);

		if (VK_QUEUE_COMPUTE_BIT & maskedFlags) {
			computeQueueIndex = i;
			return VK_SUCCESS;
		}
	}

	return VK_ERROR_INITIALIZATION_FAILED;
}

VkResult VulkanDevice::getTransferQueueHandle()
{
	// first try and find a queue that has just the transfer bit set
	for (uint32_t i = 0; i < queueFamilyCount; i++) {
		// mask out the sparse binding bit that we aren't caring about (yet!)
		const VkQueueFlags maskedFlags = (~VK_QUEUE_SPARSE_BINDING_BIT & queueFamilyProps[i].queueFlags);

		if (!((VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT) & maskedFlags) &&
			(VK_QUEUE_TRANSFER_BIT & maskedFlags)) {
			transferQueueIndex = i;
			return VK_SUCCESS;
		}
	}

	// otherwise we'll prefer using a compute-only queue,
	// remember that having compute on the queue implicitly enables transfer!
	for (uint32_t i = 0; i < queueFamilyCount; i++) {
		// mask out the sparse binding bit that we aren't caring about (yet!)
		const VkQueueFlags maskedFlags = (~VK_QUEUE_SPARSE_BINDING_BIT & queueFamilyProps[i].queueFlags);

		if (!(VK_QUEUE_GRAPHICS_BIT & maskedFlags) && (VK_QUEUE_COMPUTE_BIT & maskedFlags)) {
			transferQueueIndex = i;
			return VK_SUCCESS;
		}
	}

	// lastly get any queue that'll work for us (graphics, compute or transfer bit set)
	for (uint32_t i = 0; i < queueFamilyCount; i++) {
		// mask out the sparse binding bit that we aren't caring about (yet!)
		const VkQueueFlags maskedFlags = (~VK_QUEUE_SPARSE_BINDING_BIT & queueFamilyProps[i].queueFlags);

		if ((VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT) & maskedFlags) {
			transferQueueIndex = i;
			return VK_SUCCESS;
		}
	}

	return VK_ERROR_INITIALIZATION_FAILED;
}

void VulkanDevice::destroyDevice()
{
	vkDestroyDevice(device, NULL);
}


//Queue related functions
void VulkanDevice::getDeviceQueue()
{
	// Parminder: this depends on intialiing the SwapChain to 
	// get the graphics queue with presentation support
	vkGetDeviceQueue(device, graphicsQueueWithPresentIndex, 0, &queue);

	// Get the compute queue
	vkGetDeviceQueue(device, computeQueueIndex, 0, &queueComupte);
}
