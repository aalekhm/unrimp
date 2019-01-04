/*********************************************************\
 * Copyright (c) 2012-2019 The Unrimp Team
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
#include <RendererRuntime/Public/Resource/CompositorNode/Pass/CompositorPassFactory.h>


//[-------------------------------------------------------]
//[ Classes                                               ]
//[-------------------------------------------------------]
class CompositorPassFactoryFirst final : public RendererRuntime::CompositorPassFactory
{


//[-------------------------------------------------------]
//[ Public methods                                        ]
//[-------------------------------------------------------]
public:
	inline CompositorPassFactoryFirst()
	{
		// Nothing here
	}

	inline virtual ~CompositorPassFactoryFirst() override
	{
		// Nothing here
	}


//[-------------------------------------------------------]
//[ Protected virtual RendererRuntime::ICompositorPassFactory methods ]
//[-------------------------------------------------------]
protected:
	[[nodiscard]] virtual RendererRuntime::ICompositorResourcePass* createCompositorResourcePass(const RendererRuntime::CompositorTarget& compositorTarget, RendererRuntime::CompositorPassTypeId compositorPassTypeId) const override;
	[[nodiscard]] virtual RendererRuntime::ICompositorInstancePass* createCompositorInstancePass(const RendererRuntime::ICompositorResourcePass& compositorResourcePass, const RendererRuntime::CompositorNodeInstance& compositorNodeInstance) const override;


//[-------------------------------------------------------]
//[ Protected methods                                     ]
//[-------------------------------------------------------]
protected:
	explicit CompositorPassFactoryFirst(const CompositorPassFactoryFirst&) = delete;
	CompositorPassFactoryFirst& operator=(const CompositorPassFactoryFirst&) = delete;


};
