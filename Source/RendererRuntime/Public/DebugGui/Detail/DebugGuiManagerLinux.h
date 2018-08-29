/*********************************************************\
 * Copyright (c) 2012-2018 The Unrimp Team
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
\*********************************************************/


//[-------------------------------------------------------]
//[ Header guard                                          ]
//[-------------------------------------------------------]
#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "RendererRuntime/Public/DebugGui/DebugGuiManager.h"


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Classes                                               ]
	//[-------------------------------------------------------]
	class DebugGuiManagerLinux final : public DebugGuiManager
	{


	//[-------------------------------------------------------]
	//[ Friends                                               ]
	//[-------------------------------------------------------]
		friend class RendererRuntimeImpl;


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	public:
		RENDERERRUNTIME_API_EXPORT void onWindowResize(uint32_t width, uint32_t heigth);
		RENDERERRUNTIME_API_EXPORT void onKeyInput(uint32_t keySym, char character, bool pressed);
		RENDERERRUNTIME_API_EXPORT void onMouseMoveInput(int x, int y);
		RENDERERRUNTIME_API_EXPORT void onMouseButtonInput(uint32_t button, bool pressed);
		RENDERERRUNTIME_API_EXPORT void onMouseWheelInput(bool scrollUp);


	//[-------------------------------------------------------]
	//[ Protected virtual RendererRuntime::DebugGuiManager methods ]
	//[-------------------------------------------------------]
	protected:
		virtual void initializeImGuiKeyMap() override;
		virtual void onNewFrame(Renderer::IRenderTarget& renderTarget) override;


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	private:
		inline explicit DebugGuiManagerLinux(IRendererRuntime& rendererRuntime) :
			DebugGuiManager(rendererRuntime),
			mWindowWidth(0),
			mWindowHeigth(0),
			mTime(0)
		{
			// Nothing here
		}

		inline virtual ~DebugGuiManagerLinux() override
		{
			// Nothing here
		}

		explicit DebugGuiManagerLinux(const DebugGuiManagerLinux&) = delete;
		DebugGuiManagerLinux& operator=(const DebugGuiManagerLinux&) = delete;


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		uint32_t mWindowWidth;
		uint32_t mWindowHeigth;
		uint64_t mTime;	//< Holds the time in microseconds


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime