/*
* Learning Vulkan - ISBN: 9781786469809
*
* Author: Parminder Singh, parminder.vulkan@gmail.com
* Linkedin: https://www.linkedin.com/in/parmindersingh18
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the Software),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED AS IS, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
* DEALINGS IN THE SOFTWARE.
*/

#version 450

layout (std140, binding = 0) uniform bufferVals {	// UNIFORM_BLOCK_BINDING_INDEX
    mat4 mvp;
} myBufferVals;

layout (location = 0) in vec4 pos;
layout (location = 1) in vec4 inColor;

// Instanced attributes
layout (location = 2) in mat4 instancePos;
layout (location = 6) in vec4 instanceRot;
//layout (location = 4) in float instanceScale;
//layout (location = 5) in int instanceTexIndex;

layout (location = 0) out vec4 outColor;

void main() {
   outColor      = inColor;
   //vec4 posTemp = pos;
   //posTemp.x += instancePos.x;
   //posTemp.y += instancePos.y;
   gl_Position   = myBufferVals.mvp * instancePos * (pos + instanceRot);
   gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0;
}
