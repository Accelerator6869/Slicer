/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Daniel Haehn, UPenn
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef vtkSlicerSceneViewsModuleLogic_h
#define vtkSlicerSceneViewsModuleLogic_h

// SlicerLogic includes
#include "vtkSlicerBaseLogic.h"

// MRMLLogic includes
#include "vtkMRMLAbstractLogic.h"

#include "vtkSlicerSceneViewsModuleLogicExport.h"
//#include "qSlicerSceneViewsModuleExport.h"

#include "vtkSlicerModuleLogic.h"

// MRML includes
class vtkMRMLSceneViewNode;

// VTK includes
class vtkImageData;

// STD includes
#include <string>

class VTK_SLICER_SCENEVIEWS_MODULE_LOGIC_EXPORT vtkSlicerSceneViewsModuleLogic :
  public vtkSlicerModuleLogic
{
public:

  static vtkSlicerSceneViewsModuleLogic *New();
  vtkTypeMacro(vtkSlicerSceneViewsModuleLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Initialize listening to MRML events
  void SetMRMLSceneInternal(vtkMRMLScene * newScene) override;

  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  void RegisterNodes() override;

  /// Create a sceneView..
  void CreateSceneView(const char* name, const char* description, int screenshotType, vtkImageData* screenshot);

  /// Modify an existing sceneView.
  void ModifySceneView(std::string id, const char* name, const char* description, int screenshotType, vtkImageData* screenshot);

  /// Return the name of an existing sceneView.
  std::string GetSceneViewName(const char* id);

  /// Return the description of an existing sceneView.
  std::string GetSceneViewDescription(const char* id);

  /// Return the screenshotType of an existing sceneView.
  int GetSceneViewScreenshotType(const char* id);

  /// Return the screenshot of an existing sceneView.
  vtkImageData* GetSceneViewScreenshot(const char* id);

  /// Restore a sceneView.
  /// If removeNodes flag is false, don't restore the scene if it will remove data.
  /// The method will return with false if restore failed because nodes were not allowed
  /// to be removed.
  /// RemoveNodes defaults to true for backward compatibility.
  bool RestoreSceneView(const char* id, bool removeNodes = true);

  /// Move sceneView up
  const char* MoveSceneViewUp(const char* id);

  /// Move sceneView up
  const char* MoveSceneViewDown(const char* id);

  /// Remove a scene view node
  void RemoveSceneViewNode(vtkMRMLSceneViewNode *sceneViewNode);

protected:

  vtkSlicerSceneViewsModuleLogic();

  ~vtkSlicerSceneViewsModuleLogic() override;

  void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;
  void OnMRMLSceneEndImport() override;
  void OnMRMLSceneEndRestore() override;
  void OnMRMLSceneEndClose() override;

  void OnMRMLNodeModified(vtkMRMLNode* node) override;

private:

  std::string m_StringHolder;

private:
  vtkSlicerSceneViewsModuleLogic(const vtkSlicerSceneViewsModuleLogic&) = delete;
  void operator=(const vtkSlicerSceneViewsModuleLogic&) = delete;
};

#endif
