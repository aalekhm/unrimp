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
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "RendererRuntime/Public/DebugGui/DebugGuiHelper.h"
#include "RendererRuntime/Public/Core/Math/Transform.h"
#include "RendererRuntime/Public/Core/Math/EulerAngles.h"
#include "RendererRuntime/Public/Resource/Scene/SceneNode.h"
#include "RendererRuntime/Public/Resource/Scene/SceneResource.h"
#include "RendererRuntime/Public/Resource/Scene/Item/Camera/CameraSceneItem.h"
#include "RendererRuntime/Public/Resource/Scene/Item/Mesh/SkeletonMeshSceneItem.h"
#include "RendererRuntime/Public/Resource/Skeleton/SkeletonResourceManager.h"
#include "RendererRuntime/Public/Resource/Skeleton/SkeletonResource.h"
#include "RendererRuntime/Public/Resource/CompositorWorkspace/CompositorWorkspaceInstance.h"
#include "RendererRuntime/Public/IRendererRuntime.h"

#include <imguizmo/ImGuizmo.h>

#include <imgui/imgui.h>

// Disable warnings in external headers, we can't fix them
PRAGMA_WARNING_PUSH
	PRAGMA_WARNING_DISABLE_MSVC(4201)	// warning C4201: nonstandard extension used: nameless struct/union
	PRAGMA_WARNING_DISABLE_MSVC(4464)	// warning C4464: relative include path contains '..'
	#include <glm/gtc/type_ptr.hpp>
PRAGMA_WARNING_POP

// Disable warnings in external headers, we can't fix them
PRAGMA_WARNING_PUSH
	PRAGMA_WARNING_DISABLE_MSVC(4365)	// warning C4365: 'argument': conversion from 'long' to 'unsigned int', signed/unsigned mismatch
	PRAGMA_WARNING_DISABLE_MSVC(5026)	// warning C5026: 'std::_Generic_error_category': move constructor was implicitly defined as deleted
	PRAGMA_WARNING_DISABLE_MSVC(5027)	// warning C5027: 'std::_Generic_error_category': move assignment operator was implicitly defined as deleted
	PRAGMA_WARNING_DISABLE_MSVC(4571)	// warning C4571: Informational: catch(...) semantics changed since Visual C++ 7.1; structured exceptions (SEH) are no longer caught
	PRAGMA_WARNING_DISABLE_MSVC(4625)	// warning C4625: 'std::codecvt_base': copy constructor was implicitly defined as deleted
	PRAGMA_WARNING_DISABLE_MSVC(4626)	// warning C4626: 'std::codecvt<char16_t,char,_Mbstatet>': assignment operator was implicitly defined as deleted
	PRAGMA_WARNING_DISABLE_MSVC(4668)	// warning C4668: '_M_HYBRID_X86_ARM64' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
	PRAGMA_WARNING_DISABLE_MSVC(4774)	// warning C4774: 'sprintf_s' : format string expected in argument 3 is not a string literal
	#include <string>
	#include <unordered_set>
PRAGMA_WARNING_POP


//[-------------------------------------------------------]
//[ Anonymous detail namespace                            ]
//[-------------------------------------------------------]
namespace
{
	namespace detail
	{


		//[-------------------------------------------------------]
		//[ Global definitions                                    ]
		//[-------------------------------------------------------]
		static const ImVec4 GREEN_COLOR(0.0f, 1.0f, 0.0f, 1.0f);
		static const ImVec4 YELLOW_COLOR(1.0f, 1.0f, 0.0f, 1.0f);
		static const ImVec4 RED_COLOR(1.0f, 0.0f, 0.0f, 1.0f);


		//[-------------------------------------------------------]
		//[ Global functions                                      ]
		//[-------------------------------------------------------]
		bool objectSpaceToScreenSpacePosition(const glm::vec3& objectSpacePosition, const glm::mat4& objectSpaceToClipSpaceMatrix, ImVec2& screenSpacePosition)
		{
			glm::vec4 position = objectSpaceToClipSpaceMatrix * glm::vec4(objectSpacePosition, 1.0f);
			const bool inFront = (position.z >= 0.0f);
			position *= 0.5f / position.w;
			position += glm::vec4(0.5f, 0.5f, 0.0f, 0.0f);
			position.y = 1.0f - position.y;
			const ImGuiIO& imGuiIO = ImGui::GetIO();
			position.x *= imGuiIO.DisplaySize.x;
			position.y *= imGuiIO.DisplaySize.y;
			screenSpacePosition = ImVec2(position.x, position.y);

			// In front of camera?
			return inFront;
		}

		void draw3DLine(const glm::mat4& objectSpaceToClipSpaceMatrix, const glm::vec3& objectSpaceStartPosition, const glm::vec3& objectSpaceEndPosition, const ImColor& color, float thickness, ImDrawList& imDrawList)
		{
			// TODO(co) Add near plane clip
			ImVec2 screenSpaceStartPosition;
			ImVec2 screenSpaceEndPosition;
			const bool screenSpaceStartPositionVisible = objectSpaceToScreenSpacePosition(objectSpaceStartPosition, objectSpaceToClipSpaceMatrix, screenSpaceStartPosition);
			const bool screenSpaceEndPositionVisible = objectSpaceToScreenSpacePosition(objectSpaceEndPosition, objectSpaceToClipSpaceMatrix, screenSpaceEndPosition);
			if (screenSpaceStartPositionVisible || screenSpaceEndPositionVisible)
			{
				imDrawList.AddLine(screenSpaceStartPosition, screenSpaceEndPosition, color, thickness);
			}
		}


//[-------------------------------------------------------]
//[ Anonymous detail namespace                            ]
//[-------------------------------------------------------]
	} // detail
}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Private static data                                   ]
	//[-------------------------------------------------------]
	uint32_t DebugGuiHelper::mDrawTextCounter = 0;


	//[-------------------------------------------------------]
	//[ Public static methods                                 ]
	//[-------------------------------------------------------]
	void DebugGuiHelper::drawText(const char* text, float x, float y, bool drawBackground)
	{
		if (!drawBackground)
		{
			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1.0f, 0.0f, 0.0f, 0.0f));
		}
		ImGui::Begin(("RendererRuntime::DebugGuiManager::drawText_" + std::to_string(mDrawTextCounter)).c_str(), nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoFocusOnAppearing);
			ImGui::Text("%s", text);
			ImGui::SetWindowPos(ImVec2(x, y));
		ImGui::End();
		if (!drawBackground)
		{
			ImGui::PopStyleColor();
		}
		++mDrawTextCounter;
	}

	void DebugGuiHelper::drawGizmo(const CameraSceneItem& cameraSceneItem, GizmoSettings& gizmoSettings, Transform& transform)
	{
		// Setup ImGuizmo
		if (ImGui::RadioButton("Translate", gizmoSettings.currentGizmoOperation == GizmoOperation::TRANSLATE))
		{
			gizmoSettings.currentGizmoOperation = GizmoOperation::TRANSLATE;
		}
		ImGui::SameLine();
		if (ImGui::RadioButton("Rotate", gizmoSettings.currentGizmoOperation == GizmoOperation::ROTATE))
		{
			gizmoSettings.currentGizmoOperation = GizmoOperation::ROTATE;
		}
		ImGui::SameLine();
		if (ImGui::RadioButton("Scale", gizmoSettings.currentGizmoOperation == GizmoOperation::SCALE))
		{
			gizmoSettings.currentGizmoOperation = GizmoOperation::SCALE;
		}
		{ // Show and edit rotation quaternion using Euler angles in degree
			glm::vec3 eulerAngles = glm::degrees(EulerAngles::matrixToEuler(glm::mat3_cast(transform.rotation)));
			ImGui::InputFloat3("Tr", glm::value_ptr(transform.position), 3);
			ImGui::InputFloat3("Rt", glm::value_ptr(eulerAngles), 3);
			ImGui::InputFloat3("Sc", glm::value_ptr(transform.scale), 3);
			transform.rotation = EulerAngles::eulerToQuaternion(glm::radians(eulerAngles));
		}
		if (gizmoSettings.currentGizmoOperation != GizmoOperation::SCALE)
		{
			if (ImGui::RadioButton("Local", gizmoSettings.currentGizmoMode == GizmoMode::LOCAL))
			{
				gizmoSettings.currentGizmoMode = GizmoMode::LOCAL;
			}
			ImGui::SameLine();
			if (ImGui::RadioButton("World", gizmoSettings.currentGizmoMode == GizmoMode::WORLD))
			{
				gizmoSettings.currentGizmoMode = GizmoMode::WORLD;
			}
		}
		ImGui::Checkbox("", &gizmoSettings.useSnap);
		ImGui::SameLine();
		switch (gizmoSettings.currentGizmoOperation)
		{
			case GizmoOperation::TRANSLATE:
				ImGui::InputFloat3("Snap", &gizmoSettings.snap[0]);
				break;

			case GizmoOperation::ROTATE:
				ImGui::InputFloat("Angle Snap", &gizmoSettings.snap[0]);
				break;

			case GizmoOperation::SCALE:
				ImGui::InputFloat("Scale Snap", &gizmoSettings.snap[0]);
				break;
		}
		{ // Let ImGuizmo do its thing
			glm::mat4 matrix;
			transform.getAsMatrix(matrix);
			ImGuizmo::OPERATION operation = static_cast<ImGuizmo::OPERATION>(gizmoSettings.currentGizmoOperation);
			ImGuizmo::MODE mode = (operation == ImGuizmo::SCALE) ? ImGuizmo::LOCAL : static_cast<ImGuizmo::MODE>(gizmoSettings.currentGizmoMode);
			const ImGuiIO& imGuiIO = ImGui::GetIO();
			ImGuizmo::SetRect(0, 0, imGuiIO.DisplaySize.x, imGuiIO.DisplaySize.y);
			ImGuizmo::Manipulate(glm::value_ptr(cameraSceneItem.getWorldSpaceToViewSpaceMatrix()), glm::value_ptr(cameraSceneItem.getViewSpaceToClipSpaceMatrix(static_cast<float>(imGuiIO.DisplaySize.x) / imGuiIO.DisplaySize.y)), operation, mode, glm::value_ptr(matrix), nullptr, gizmoSettings.useSnap ? &gizmoSettings.snap[0] : nullptr);
			transform = Transform(matrix);
		}
	}

	void DebugGuiHelper::drawSkeleton(const CameraSceneItem& cameraSceneItem, const SkeletonMeshSceneItem& skeletonMeshSceneItem)
	{
		// Get skeleton resource instance
		const SkeletonResource* skeletonResource = skeletonMeshSceneItem.getSceneResource().getRendererRuntime().getSkeletonResourceManager().tryGetById(skeletonMeshSceneItem.getSkeletonResourceId());
		if (nullptr != skeletonResource)
		{
			// Get transform data
			glm::mat4 objectSpaceToWorldSpace;
			skeletonMeshSceneItem.getParentSceneNodeSafe().getGlobalTransform().getAsMatrix(objectSpaceToWorldSpace);
			const ImGuiIO& imGuiIO = ImGui::GetIO();
			const glm::mat4 objectSpaceToClipSpaceMatrix = cameraSceneItem.getViewSpaceToClipSpaceMatrix(static_cast<float>(imGuiIO.DisplaySize.x) / imGuiIO.DisplaySize.y) * cameraSceneItem.getWorldSpaceToViewSpaceMatrix() * objectSpaceToWorldSpace;

			// Get skeleton data
			const uint8_t numberOfBones = skeletonResource->getNumberOfBones();
			const uint8_t* boneParentIndices = skeletonResource->getBoneParentIndices();
			const glm::mat4* globalBoneMatrices = skeletonResource->getGlobalBoneMatrices();

			// Draw skeleton hierarchy as lines
			// -> Update ImGui style to not have a visible round border
			ImGui::PushStyleColor(ImGuiCol_WindowBg, 0);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			if (ImGui::Begin("skeleton", nullptr, ImGui::GetIO().DisplaySize, 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus))
			{
				static const ImColor WHITE_COLOR(255, 255, 255);
				ImDrawList* imDrawList = ImGui::GetWindowDrawList();
				ImVec2 parentBonePosition;
				ImVec2 bonePosition;
				for (uint8_t boneIndex = 1; boneIndex < numberOfBones; ++boneIndex)
				{
					::detail::draw3DLine(objectSpaceToClipSpaceMatrix, globalBoneMatrices[boneParentIndices[boneIndex]][3], globalBoneMatrices[boneIndex][3], WHITE_COLOR, 6.0f, *imDrawList);
				}
			}
			ImGui::End();
			ImGui::PopStyleVar();
			ImGui::PopStyleColor();
		}
	}

	void DebugGuiHelper::drawGrid(const CameraSceneItem& cameraSceneItem, float cellSize, float yPosition)
	{
		if (ImGui::Begin("grid", nullptr, ImGui::GetIO().DisplaySize, 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus))
		{
			const int32_t NUMBER_OF_LINES_PER_DIRECTION = 10;
			static const ImColor GREY_COLOR(0.5f, 0.5f, 0.5f, 1.0f);
			ImDrawList* imDrawList = ImGui::GetWindowDrawList();
			const ImGuiIO& imGuiIO = ImGui::GetIO();
			const glm::mat4 objectSpaceToClipSpaceMatrix = cameraSceneItem.getViewSpaceToClipSpaceMatrix(static_cast<float>(imGuiIO.DisplaySize.x) / imGuiIO.DisplaySize.y) * cameraSceneItem.getWorldSpaceToViewSpaceMatrix();

			// Keep the grid fixed at camera
			const glm::vec3& cameraPosition = cameraSceneItem.getParentSceneNodeSafe().getTransform().position;
			const glm::vec3 centerPosition(Math::makeMultipleOf(cameraPosition.x, cellSize), yPosition, Math::makeMultipleOf(cameraPosition.z, cellSize));

			// Lines along z axis
			for (int32_t z = -NUMBER_OF_LINES_PER_DIRECTION; z <= NUMBER_OF_LINES_PER_DIRECTION; ++z)
			{
				const float thickness = (0 == z || NUMBER_OF_LINES_PER_DIRECTION == std::abs(z)) ? 4.0f : 1.0f;
				::detail::draw3DLine(objectSpaceToClipSpaceMatrix, centerPosition + glm::vec3(-NUMBER_OF_LINES_PER_DIRECTION * cellSize, 0.0f, z * cellSize), centerPosition + glm::vec3(NUMBER_OF_LINES_PER_DIRECTION * cellSize, 0.0f, z * cellSize), GREY_COLOR, thickness, *imDrawList);
			}

			// Lines along x axis
			for (int32_t x = -NUMBER_OF_LINES_PER_DIRECTION; x <= NUMBER_OF_LINES_PER_DIRECTION; ++x)
			{
				const float thickness = (0 == x || NUMBER_OF_LINES_PER_DIRECTION == std::abs(x)) ? 4.0f : 1.0f;
				::detail::draw3DLine(objectSpaceToClipSpaceMatrix, centerPosition + glm::vec3(x * cellSize, 0.0f, -NUMBER_OF_LINES_PER_DIRECTION * cellSize), centerPosition + glm::vec3(x * cellSize, 0.0f, NUMBER_OF_LINES_PER_DIRECTION * cellSize), GREY_COLOR, thickness, *imDrawList);
			}
		}
		ImGui::End();
	}


	//[-------------------------------------------------------]
	//[ Private static methods                                ]
	//[-------------------------------------------------------]
	void DebugGuiHelper::drawMetricsWindow(bool& open, CompositorWorkspaceInstance* compositorWorkspaceInstance)
	{
		if (ImGui::Begin("Metrics", &open))
		{
			// Frames per second (FPS)
			const float framesPerSecond = ImGui::GetIO().Framerate;
			ImVec4 color = ::detail::GREEN_COLOR;
			if (framesPerSecond < 60.0f)
			{
				color = ::detail::RED_COLOR;
			}
			else if (framesPerSecond < 90.0f)
			{
				// HTC Vive refresh rate: 90 Hz (11.11 ms per frame), everything below isn't OK
				color = ::detail::YELLOW_COLOR;
			}
			ImGui::PushStyleColor(ImGuiCol_Text, color);
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / framesPerSecond, framesPerSecond);
			ImGui::PopStyleColor();

			// Optional compositor workspace instance metrics
			if (nullptr != compositorWorkspaceInstance)
			{
				// Please note that one renderable manager can be inside multiple render queue index ranges
				// -> Since this metrics debugging code isn't performance critical we're using already available data to extract the
				//    information we want to display instead of letting the core system gather additional data it doesn't need to work
				size_t numberOfRenderables = 0;
				std::unordered_set<const RenderableManager*> processedRenderableManager;
				for (const CompositorWorkspaceInstance::RenderQueueIndexRange& renderQueueIndexRanges : compositorWorkspaceInstance->getRenderQueueIndexRanges())
				{
					for (const RenderableManager* renderableManager : renderQueueIndexRanges.renderableManagers)
					{
						if (processedRenderableManager.find(renderableManager) == processedRenderableManager.cend())
						{
							processedRenderableManager.insert(renderableManager);
							numberOfRenderables += renderableManager->getRenderables().size();
						}
					}
				}
				ImGui::Text("Rendered renderable managers %d", static_cast<int>(processedRenderableManager.size()));
				ImGui::Text("Rendered renderables %d", static_cast<int>(numberOfRenderables));

				// Command buffer metrics
				const Renderer::CommandBuffer& commandBuffer = compositorWorkspaceInstance->getCommandBuffer();
				#ifdef RENDERER_STATISTICS
					const uint32_t numberOfCommands = commandBuffer.getNumberOfCommands();
				#else
					uint32_t numberOfCommands = 0;
					{
						const uint8_t* commandPacketBuffer = commandBuffer.getCommandPacketBuffer();
						Renderer::ConstCommandPacket constCommandPacket = commandPacketBuffer;
						while (nullptr != constCommandPacket)
						{
							// Count command packet
							++numberOfCommands;

							{ // Next command
								const uint32_t nextCommandPacketByteIndex = Renderer::CommandPacketHelper::getNextCommandPacketByteIndex(constCommandPacket);
								constCommandPacket = (~0u != nextCommandPacketByteIndex) ? &commandPacketBuffer[nextCommandPacketByteIndex] : nullptr;
							}
						}
					}
				#endif
				if (ImGui::TreeNode("EmittedCommands", "Emitted commands: %d", numberOfCommands))
				{
					// Loop through all commands and count them
					uint32_t numberOfCommandFunctions[Renderer::CommandDispatchFunctionIndex::NumberOfFunctions] = {};
					const uint8_t* commandPacketBuffer = commandBuffer.getCommandPacketBuffer();
					Renderer::ConstCommandPacket constCommandPacket = commandPacketBuffer;
					while (nullptr != constCommandPacket)
					{
						// Count command packet
						++numberOfCommandFunctions[Renderer::CommandPacketHelper::loadCommandDispatchFunctionIndex(constCommandPacket)];

						{ // Next command
							const uint32_t nextCommandPacketByteIndex = Renderer::CommandPacketHelper::getNextCommandPacketByteIndex(constCommandPacket);
							constCommandPacket = (~0u != nextCommandPacketByteIndex) ? &commandPacketBuffer[nextCommandPacketByteIndex] : nullptr;
						}
					}

					// Print the number of emitted command functions
					static constexpr const char* commandFunction[Renderer::CommandDispatchFunctionIndex::NumberOfFunctions] =
					{
						// Command buffer
						"ExecuteCommandBuffer",
						// Graphics
						"SetGraphicsRootSignature",
						"SetGraphicsPipelineState",
						"SetGraphicsResourceGroup",
						"SetGraphicsVertexArray",
						"SetGraphicsViewports",
						"SetGraphicsScissorRectangles",
						"SetGraphicsRenderTarget",
						"ClearGraphics",
						"DrawGraphics",
						"DrawIndexedGraphics",
						// Compute
						"SetComputeRootSignature",
						"SetComputePipelineState",
						"SetComputeResourceGroup",
						"DispatchCompute",
						// Resource
						"SetTextureMinimumMaximumMipmapIndex",
						"ResolveMultisampleFramebuffer",
						"CopyResource",
						// Debug
						"SetDebugMarker",
						"BeginDebugEvent",
						"EndDebugEvent"
					};
					for (uint32_t i = 0; i < Renderer::CommandDispatchFunctionIndex::NumberOfFunctions; ++i)
					{
						ImGui::Text("%s: %d", commandFunction[i], numberOfCommandFunctions[i]);
					}
					ImGui::TreePop();
				}

				// Renderer statistics
				#ifdef RENDERER_STATISTICS
					const Renderer::Statistics& statistics = static_cast<const Renderer::IRenderer&>(compositorWorkspaceInstance->getRendererRuntime().getRenderer()).getStatistics();
					if (ImGui::TreeNode("RendererResources", "Renderer resources: %d", statistics.getNumberOfCurrentResources()))
					{
						ImGui::Text("Root signatures: %d", statistics.currentNumberOfRootSignatures.load());
						ImGui::Text("Resource groups: %d", statistics.currentNumberOfResourceGroups.load());
						ImGui::Text("Graphics programs: %d", statistics.currentNumberOfGraphicsPrograms.load());
						ImGui::Text("Vertex arrays: %d", statistics.currentNumberOfVertexArrays.load());
						ImGui::Text("Render passes: %d", statistics.currentNumberOfRenderPasses.load());
						ImGui::Text("Swap chains: %d", statistics.currentNumberOfSwapChains.load());
						ImGui::Text("Framebuffers: %d", statistics.currentNumberOfFramebuffers.load());
						ImGui::Text("Index buffers: %d", statistics.currentNumberOfIndexBuffers.load());
						ImGui::Text("Vertex buffers: %d", statistics.currentNumberOfVertexBuffers.load());
						ImGui::Text("Texture buffers: %d", statistics.currentNumberOfTextureBuffers.load());
						ImGui::Text("Structured buffers: %d", statistics.currentNumberOfStructuredBuffers.load());
						ImGui::Text("Indirect buffers: %d", statistics.currentNumberOfIndirectBuffers.load());
						ImGui::Text("Uniform buffers: %d", statistics.currentNumberOfUniformBuffers.load());
						ImGui::Text("1D textures: %d", statistics.currentNumberOfTexture1Ds.load());
						ImGui::Text("2D textures: %d", statistics.currentNumberOfTexture2Ds.load());
						ImGui::Text("2D texture arrays: %d", statistics.currentNumberOfTexture2DArrays.load());
						ImGui::Text("3D textures: %d", statistics.currentNumberOfTexture3Ds.load());
						ImGui::Text("Cubes textures: %d", statistics.currentNumberOfTextureCubes.load());
						ImGui::Text("Graphics pipeline states: %d", statistics.currentNumberOfGraphicsPipelineStates.load());
						ImGui::Text("Compute pipeline states: %d", statistics.currentNumberOfComputePipelineStates.load());
						ImGui::Text("Sampler states: %d", statistics.currentNumberOfSamplerStates.load());
						ImGui::Text("Vertex shaders: %d", statistics.currentNumberOfVertexShaders.load());
						ImGui::Text("Tessellation control shaders: %d", statistics.currentNumberOfTessellationControlShaders.load());
						ImGui::Text("Tessellation evaluation shaders: %d", statistics.currentNumberOfTessellationEvaluationShaders.load());
						ImGui::Text("Geometry shaders: %d", statistics.currentNumberOfGeometryShaders.load());
						ImGui::Text("Fragment shaders: %d", statistics.currentNumberOfFragmentShaders.load());
						ImGui::Text("Compute shaders: %d", statistics.currentNumberOfComputeShaders.load());
						ImGui::TreePop();
					}
				#endif
			}
		}
		ImGui::End();
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime