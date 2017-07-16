/*
* Learning Vulkan - ISBN: 9781786469809
*
* Author: Parminder Singh, parminder.vulkan@gmail.com
* Linkedin: https://www.linkedin.com/in/parmindersingh18
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

#include "VulkanShader.h"
#include "VulkanApplication.h"
#include "VulkanDevice.h"


void VulkanShader::buildShaderModuleWithSPV(uint32_t *shaderText, size_t shaderSPVSize, const char* entryPoint, VkShaderStageFlagBits shaderStageFlag)
{
	VulkanDevice* deviceObj = VulkanApplication::GetInstance()->deviceObj;

	VkResult  result;

	VkPipelineShaderStageCreateInfo shaderStage;
	// Fill in the control structure to push the necessary
	// details of the shader.
	shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStage.pNext = NULL;
	shaderStage.pSpecializationInfo = NULL;
	shaderStage.flags = 0;
	shaderStage.stage = shaderStageFlag;
	shaderStage.pName = entryPoint;

	VkShaderModuleCreateInfo moduleCreateInfo;
	moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleCreateInfo.pNext = NULL;
	moduleCreateInfo.flags = 0;
	moduleCreateInfo.codeSize = shaderSPVSize;
	moduleCreateInfo.pCode = shaderText;
	result = vkCreateShaderModule(deviceObj->device, &moduleCreateInfo, NULL, &shaderStage.module);
	assert(result == VK_SUCCESS);

	shaderStagesVector.push_back(shaderStage);
}

void VulkanShader::destroyShaders()
{
	VulkanDevice* deviceObj = VulkanApplication::GetInstance()->deviceObj;
	for (int i = 0; i < shaderStagesVector.size(); i++)
	{
		vkDestroyShaderModule(deviceObj->device, shaderStagesVector[i].module, NULL);
	}
}

#ifdef AUTO_COMPILE_GLSL_TO_SPV

// Helper function intaking the GLSL vertex and fragment shader. 
// It prepares the shaders to be consumed in the SPIR-V format 
// with the help of glslang library helper functions.
void VulkanShader::buildShader(const char *shaderText, const char* entryPoint, VkShaderStageFlagBits shaderStageFlag)
{
	VulkanDevice* deviceObj = VulkanApplication::GetInstance()->deviceObj;

	VkResult  result;
	bool  retVal;
	VkPipelineShaderStageCreateInfo shaderStage;
	// Fill in the control structure to push the necessary
	// details of the shader.
	std::vector<unsigned int> shaderSPV;
	shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStage.pNext = NULL;
	shaderStage.pSpecializationInfo = NULL;
	shaderStage.flags = 0;
	shaderStage.stage = shaderStageFlag;
	shaderStage.pName = entryPoint;

	glslang::InitializeProcess();

	retVal = GLSLtoSPV(shaderStageFlag, shaderText, shaderSPV);
	assert(retVal);

	VkShaderModuleCreateInfo moduleCreateInfo;
	moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleCreateInfo.pNext = NULL;
	moduleCreateInfo.flags = 0;
	moduleCreateInfo.codeSize = shaderSPV.size() * sizeof(unsigned int);
	moduleCreateInfo.pCode = shaderSPV.data();
	result = vkCreateShaderModule(deviceObj->device, &moduleCreateInfo, NULL, &shaderStage.module);
	assert(result == VK_SUCCESS);

	shaderStagesVector.push_back(shaderStage);

	glslang::FinalizeProcess();
}

//
// Compile a given string containing GLSL into SPV for use by VK
// Return value of false means an error was encountered.
//
bool VulkanShader::GLSLtoSPV(const VkShaderStageFlagBits shaderType, const char *pshader, std::vector<unsigned int> &spirv)
{
	glslang::TProgram* program = new glslang::TProgram;
	const char *shaderStrings[1];
	TBuiltInResource Resources;
	initializeResources(Resources);

	// Enable SPIR-V and Vulkan rules when parsing GLSL
	EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);

	EShLanguage stage = getLanguage(shaderType);
	glslang::TShader* shader = new glslang::TShader(stage);

	shaderStrings[0] = pshader;
	shader->setStrings(shaderStrings, 1);

	if (!shader->parse(&Resources, 100, false, messages)) {
		puts(shader->getInfoLog());
		puts(shader->getInfoDebugLog());
		return false;
	}

	program->addShader(shader);

	// Link the program and report if errors...
	if (!program->link(messages)) {
		puts(shader->getInfoLog());
		puts(shader->getInfoDebugLog());
		return false;
	}

	glslang::GlslangToSpv(*program->getIntermediate(stage), spirv);
	delete program;
	delete shader;
	return true;
}

EShLanguage VulkanShader::getLanguage(const VkShaderStageFlagBits shaderType)
{
	switch (shaderType) {
	case VK_SHADER_STAGE_VERTEX_BIT:
		return EShLangVertex;

	case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
		return EShLangTessControl;

	case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
		return EShLangTessEvaluation;

	case VK_SHADER_STAGE_GEOMETRY_BIT:
		return EShLangGeometry;

	case VK_SHADER_STAGE_FRAGMENT_BIT:
		return EShLangFragment;

	case VK_SHADER_STAGE_COMPUTE_BIT:
		return EShLangCompute;

	default:
		printf("Unknown shader type specified: %d. Exiting!", shaderType);
		exit(1);
	}
}


void VulkanShader::initializeResources(TBuiltInResource &Resources)
{
	Resources.maxLights = 32;
	Resources.maxClipPlanes = 6;
	Resources.maxTextureUnits = 32;
	Resources.maxTextureCoords = 32;
	Resources.maxVertexAttribs = 64;
	Resources.maxVertexUniformComponents = 4096;
	Resources.maxVaryingFloats = 64;
	Resources.maxVertexTextureImageUnits = 32;
	Resources.maxCombinedTextureImageUnits = 80;
	Resources.maxTextureImageUnits = 32;
	Resources.maxFragmentUniformComponents = 4096;
	Resources.maxDrawBuffers = 32;
	Resources.maxVertexUniformVectors = 128;
	Resources.maxVaryingVectors = 8;
	Resources.maxFragmentUniformVectors = 16;
	Resources.maxVertexOutputVectors = 16;
	Resources.maxFragmentInputVectors = 15;
	Resources.minProgramTexelOffset = -8;
	Resources.maxProgramTexelOffset = 7;
	Resources.maxClipDistances = 8;
	Resources.maxComputeWorkGroupCountX = 65535;
	Resources.maxComputeWorkGroupCountY = 65535;
	Resources.maxComputeWorkGroupCountZ = 65535;
	Resources.maxComputeWorkGroupSizeX = 1024;
	Resources.maxComputeWorkGroupSizeY = 1024;
	Resources.maxComputeWorkGroupSizeZ = 64;
	Resources.maxComputeUniformComponents = 1024;
	Resources.maxComputeTextureImageUnits = 16;
	Resources.maxComputeImageUniforms = 8;
	Resources.maxComputeAtomicCounters = 8;
	Resources.maxComputeAtomicCounterBuffers = 1;
	Resources.maxVaryingComponents = 60;
	Resources.maxVertexOutputComponents = 64;
	Resources.maxGeometryInputComponents = 64;
	Resources.maxGeometryOutputComponents = 128;
	Resources.maxFragmentInputComponents = 128;
	Resources.maxImageUnits = 8;
	Resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
	Resources.maxCombinedShaderOutputResources = 8;
	Resources.maxImageSamples = 0;
	Resources.maxVertexImageUniforms = 0;
	Resources.maxTessControlImageUniforms = 0;
	Resources.maxTessEvaluationImageUniforms = 0;
	Resources.maxGeometryImageUniforms = 0;
	Resources.maxFragmentImageUniforms = 8;
	Resources.maxCombinedImageUniforms = 8;
	Resources.maxGeometryTextureImageUnits = 16;
	Resources.maxGeometryOutputVertices = 256;
	Resources.maxGeometryTotalOutputComponents = 1024;
	Resources.maxGeometryUniformComponents = 1024;
	Resources.maxGeometryVaryingComponents = 64;
	Resources.maxTessControlInputComponents = 128;
	Resources.maxTessControlOutputComponents = 128;
	Resources.maxTessControlTextureImageUnits = 16;
	Resources.maxTessControlUniformComponents = 1024;
	Resources.maxTessControlTotalOutputComponents = 4096;
	Resources.maxTessEvaluationInputComponents = 128;
	Resources.maxTessEvaluationOutputComponents = 128;
	Resources.maxTessEvaluationTextureImageUnits = 16;
	Resources.maxTessEvaluationUniformComponents = 1024;
	Resources.maxTessPatchComponents = 120;
	Resources.maxPatchVertices = 32;
	Resources.maxTessGenLevel = 64;
	Resources.maxViewports = 16;
	Resources.maxVertexAtomicCounters = 0;
	Resources.maxTessControlAtomicCounters = 0;
	Resources.maxTessEvaluationAtomicCounters = 0;
	Resources.maxGeometryAtomicCounters = 0;
	Resources.maxFragmentAtomicCounters = 8;
	Resources.maxCombinedAtomicCounters = 8;
	Resources.maxAtomicCounterBindings = 1;
	Resources.maxVertexAtomicCounterBuffers = 0;
	Resources.maxTessControlAtomicCounterBuffers = 0;
	Resources.maxTessEvaluationAtomicCounterBuffers = 0;
	Resources.maxGeometryAtomicCounterBuffers = 0;
	Resources.maxFragmentAtomicCounterBuffers = 1;
	Resources.maxCombinedAtomicCounterBuffers = 1;
	Resources.maxAtomicCounterBufferSize = 16384;
	Resources.maxTransformFeedbackBuffers = 4;
	Resources.maxTransformFeedbackInterleavedComponents = 64;
	Resources.maxCullDistances = 8;
	Resources.maxCombinedClipAndCullDistances = 8;
	Resources.maxSamples = 4;
	Resources.limits.nonInductiveForLoops = 1;
	Resources.limits.whileLoops = 1;
	Resources.limits.doWhileLoops = 1;
	Resources.limits.generalUniformIndexing = 1;
	Resources.limits.generalAttributeMatrixVectorIndexing = 1;
	Resources.limits.generalVaryingIndexing = 1;
	Resources.limits.generalSamplerIndexing = 1;
	Resources.limits.generalVariableIndexing = 1;
	Resources.limits.generalConstantMatrixVectorIndexing = 1;

}
#endif
