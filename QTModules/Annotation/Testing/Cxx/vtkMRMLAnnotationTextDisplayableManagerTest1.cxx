/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// MRMLDisplayableManager includes
#include <vtkMRMLDisplayableManagerFactory.h>
#include <vtkMRMLDisplayableManagerGroup.h>
#include <vtkMRMLViewDisplayableManager.h>
#include <vtkMRMLCameraDisplayableManager.h>
#include <vtkThreeDViewInteractorStyle.h>

// MRMLLogic includes
#include <vtkMRMLApplicationLogic.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLViewNode.h>
#include <vtkMRMLAnnotationTextNode.h>

// VTK includes
#include <vtkRegressionTestImage.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h> 
#include <vtkSmartPointer.h>
#include <vtkErrorCode.h>
#include <vtkInteractorEventRecorder.h>
#include <vtkWindowToImageFilter.h>
#include <vtkPNGWriter.h>

// STD includes
#include <cstdlib>
#include <iterator>

#include "TestingMacros.h"

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()


//----------------------------------------------------------------------------
class vtkRenderRequestCallback : public vtkCommand
{
public:
  static vtkRenderRequestCallback *New()
    { return new vtkRenderRequestCallback; }
  void SetRenderer(vtkRenderer *renderer)
    { this->Renderer =  renderer; }
  int GetRenderRequestCount()
    { return this->RenderRequestCount; }
  virtual void Execute(vtkObject*, unsigned long , void* )
    {
    this->Renderer->GetRenderWindow()->Render();
    this->RenderRequestCount++;
    //std::cout << "RenderRequestCount [" << this->RenderRequestCount << "]" << std::endl;
    }
protected:
  vtkRenderRequestCallback():Renderer(0), RenderRequestCount(0){}
  vtkRenderer * Renderer;
  int           RenderRequestCount;
};

//----------------------------------------------------------------------------
int vtkMRMLAnnotationTextDisplayableManagerTest1(int argc, char* argv[])
{


  /*
   *
   * Setup the test scenario.
   *
   */
  // Renderer, RenderWindow and Interactor
  vtkRenderer* rr = vtkRenderer::New();
  vtkRenderWindow* rw = vtkRenderWindow::New();
  vtkRenderWindowInteractor* ri = vtkRenderWindowInteractor::New();
  rw->SetSize(600, 600);

  rw->SetMultiSamples(0); // Ensure to have the same test image everywhere

  rw->AddRenderer(rr);
  rw->SetInteractor(ri);

  // Set Interactor Style
  vtkThreeDViewInteractorStyle * iStyle = vtkThreeDViewInteractorStyle::New();
  ri->SetInteractorStyle(iStyle);
  iStyle->Delete();

  // MRML scene
  vtkMRMLScene* scene = vtkMRMLScene::New();

  // Application logic - Handle creation of vtkMRMLSelectionNode and vtkMRMLInteractionNode
  vtkMRMLApplicationLogic* applicationLogic = vtkMRMLApplicationLogic::New();
  applicationLogic->SetMRMLScene(scene);
  // Pass through event handling once without observing the scene
  // allows any dependent nodes to be created
  applicationLogic->ProcessMRMLEvents(scene, vtkCommand::ModifiedEvent, 0);
  applicationLogic->SetAndObserveMRMLScene(scene);

  // Add ViewNode
  vtkMRMLViewNode * viewNode = vtkMRMLViewNode::New();
  vtkMRMLNode * nodeAdded = scene->AddNode(viewNode);
  viewNode->Delete();
  if (!nodeAdded)
    {
    std::cerr << "Failed to add vtkMRMLViewNode" << std::endl;
    return EXIT_FAILURE;
    }

  /*
   *
   * Create the DisplayableManager factory and register the DisplayableManager
   *
   * Perform some tests to ensure registration succeeded.
   *
   */
  // Factory
  vtkMRMLDisplayableManagerFactory * factory = vtkMRMLDisplayableManagerFactory::New();

  // Check if GetRegisteredDisplayableManagerCount returns 0
  if (factory->GetRegisteredDisplayableManagerCount() != 0)
    {
    std::cerr << "Expected RegisteredDisplayableManagerCount: 0" << std::endl;
    std::cerr << "Current RegisteredDisplayableManagerCount:"
        << factory->GetRegisteredDisplayableManagerCount() << std::endl;
    return EXIT_FAILURE;
    }

  factory->RegisterDisplayableManager("vtkMRMLAnnotationTextDisplayableManager");

  /*
   *
   * Check if factory registered the displayable Manager
   *
   */
  // Check if GetRegisteredDisplayableManagerCount returns 2
  if (factory->GetRegisteredDisplayableManagerCount() != 1)
    {
    std::cerr << "Expected RegisteredDisplayableManagerCount: 1" << std::endl;
    std::cerr << "Current RegisteredDisplayableManagerCount:"
        << factory->GetRegisteredDisplayableManagerCount() << std::endl;
    return EXIT_FAILURE;
    }

  vtkMRMLDisplayableManagerGroup * displayableManagerGroup =
      factory->InstantiateDisplayableManagers(rr);

  if (!displayableManagerGroup)
    {
    std::cerr << "Failed to instantiate Displayable Managers using "
        << "InstantiateDisplayableManagers" << std::endl;
    return EXIT_FAILURE;
    }


  if (displayableManagerGroup->GetDisplayableManagerCount() != 1)
    {
    std::cerr << "Check displayableManagerGroup->GetDisplayableManagerCount()" << std::endl;
    std::cerr << "Expected DisplayableManagerCount: 1" << std::endl;
    std::cerr << "Current DisplayableManagerCount:"
      << displayableManagerGroup->GetDisplayableManagerCount() << std::endl;
    return EXIT_FAILURE;
    }

  // RenderRequest Callback
  vtkRenderRequestCallback * renderRequestCallback = vtkRenderRequestCallback::New();
  renderRequestCallback->SetRenderer(rr);
  displayableManagerGroup->AddObserver(vtkCommand::UpdateEvent, renderRequestCallback);

  // Assign ViewNode
  displayableManagerGroup->SetMRMLViewNode(viewNode);

  // Check if RenderWindowInteractor has NOT been changed
  if (displayableManagerGroup->GetInteractor() != ri)
    {
    std::cerr << "Expected RenderWindowInteractor:" << ri << std::endl;
    std::cerr << "Current RenderWindowInteractor:"
        << displayableManagerGroup->GetInteractor() << std::endl;
    return EXIT_FAILURE;
    }

  // Interactor style should be vtkThreeDViewInteractorStyle
  vtkInteractorObserver * currentInteractoryStyle = ri->GetInteractorStyle();
  if (!vtkThreeDViewInteractorStyle::SafeDownCast(currentInteractoryStyle))
    {
    std::cerr << "Expected interactorStyle: vtkThreeDViewInteractorStyle" << std::endl;
    std::cerr << "Current RenderWindowInteractor: "
      << (currentInteractoryStyle ? currentInteractoryStyle->GetClassName() : "Null") << std::endl;
    return EXIT_FAILURE;
    }

  /*
   *
   * Start testing the actual functionality.
   *
   * Testing plan
   * 1. Create text annotations and fire vtkMRMLScene::NodeAddedEvent events through MRMLScene->AddNode
   * 2. The following tests are executed two times: 1. for single text annotation, 2. for multiple text annotations
   *  2a. Fire vtkCommand::ModifiedEvent
   *  2b. Fire vtkMRMLTransformableNode::TransformModifiedEvent
   *  2c. Fire vtkMRMLAnnotationNode::LockModifiedEvent to Lock
   *  2d. Fire vtkMRMLAnnotationNode::LockModifiedEvent to Unlock
   * 3. Delete text annotations and fire vtkMRMLScene::NodeRemovedEvent events through MRMLScene->RemoveNode
   * 4. Close MRML Scene
   *
   */

  //--------------------------------------------------------------------------------------
  // TEST 1:
  // Add one TextNode to scene and see if widget appears!
  //

  // fail if widgets are in Renderwindow
  if (rr->GetViewProps()->GetNumberOfItems()>0) {
    std::cerr << "Expected number of items in renderer: 0" << std::endl;
    std::cerr << "Current number of items in renderer: " << rr->GetViewProps()->GetNumberOfItems() << std::endl;
    return EXIT_FAILURE;
  }

  vtkMRMLAnnotationTextNode* textNode1 = vtkMRMLAnnotationTextNode::New();
  // the next line fires the event
  textNode1->Initialize(scene);
  textNode1->SetName("AnnotationText1Testing");

  // fail if widget did not appear
  if (rr->GetViewProps()->GetNumberOfItems()!=1) {
    std::cerr << "Expected number of items in renderer: 1" << std::endl;
    std::cerr << "Current number of items in renderer: " << rr->GetViewProps()->GetNumberOfItems() << std::endl;
    return EXIT_FAILURE;
  }

  //--------------------------------------------------------------------------------------
  // TEST 2:
  // Add multiple TextNodes to scene and see if widgets appear!
  //

  vtkMRMLAnnotationTextNode* textNode2 = vtkMRMLAnnotationTextNode::New();
  // the next line fires the event
  textNode2->Initialize(scene);
  textNode2->SetName("AnnotationText2Testing");

  vtkMRMLAnnotationTextNode* textNode3 = vtkMRMLAnnotationTextNode::New();
  // the next line fires the event
  textNode3->Initialize(scene);
  textNode3->SetName("AnnotationText3Testing");

  vtkMRMLAnnotationTextNode* textNode4 = vtkMRMLAnnotationTextNode::New();
  // the next line fires the event
  textNode4->Initialize(scene);
  textNode4->SetName("AnnotationText4Testing");


  // fail if widget did not appear
  if (rr->GetViewProps()->GetNumberOfItems()!=4) {
    std::cerr << "Expected number of items in renderer: 4" << std::endl;
    std::cerr << "Current number of items in renderer: " << rr->GetViewProps()->GetNumberOfItems() << std::endl;
    return EXIT_FAILURE;
  }

  //--------------------------------------------------------------------------------------
  // TEST 3:
  // Hide the added TextNodes and see if change gets propagated!
  //
  // fires OnMRMLAnnotationTextNodeModifiedEvent
  textNode4->SetVisible(0);
  // fail if widget did not appear
  if (rr->GetViewProps()->GetNumberOfItems()!=3) {
    std::cerr << "Expected number of items in renderer: 3" << std::endl;
    std::cerr << "Current number of items in renderer: " << rr->GetViewProps()->GetNumberOfItems() << std::endl;
    return EXIT_FAILURE;
  }

  // fires OnMRMLAnnotationTextNodeModifiedEvent
  textNode3->SetVisible(0);
  // fail if widget did not appear
  if (rr->GetViewProps()->GetNumberOfItems()!=2) {
    std::cerr << "Expected number of items in renderer: 2" << std::endl;
    std::cerr << "Current number of items in renderer: " << rr->GetViewProps()->GetNumberOfItems() << std::endl;
    return EXIT_FAILURE;
  }

  // fires OnMRMLAnnotationTextNodeModifiedEvent
  textNode2->SetVisible(0);
  // fail if widget did not appear
  if (rr->GetViewProps()->GetNumberOfItems()!=1) {
    std::cerr << "Expected number of items in renderer: 1" << std::endl;
    std::cerr << "Current number of items in renderer: " << rr->GetViewProps()->GetNumberOfItems() << std::endl;
    return EXIT_FAILURE;
  }

  // fires OnMRMLAnnotationTextNodeModifiedEvent
  textNode1->SetVisible(0);
  // fail if widget did not appear
  if (rr->GetViewProps()->GetNumberOfItems()!=0) {
    std::cerr << "Expected number of items in renderer: 0" << std::endl;
    std::cerr << "Current number of items in renderer: " << rr->GetViewProps()->GetNumberOfItems() << std::endl;
    return EXIT_FAILURE;
  }




  /*
  // get the fourth widget
  vtkPropCollection* props = rr->GetViewProps();
  props->InitTraversal();
  vtkProp* curProp = 0;
  for(int i = 0; i < props->GetNumberOfItems(); i++)
    {
    std::cout << props->GetNumberOfItems();
    curProp = props->GetNextProp();
    //std::cout << curProp->GetPickable() << std::endl;
    //std::cout << curProp->GetDragable() << std::endl;
    std::cout << i << curProp->GetVisibility() << std::endl;
    }

  // fail if fourth widget did not hide
  if (curProp->GetVisibility()!=0) {
    std::cerr << "Expected visibility of last text node in renderer: 0" << std::endl;
    std::cerr << "Current visibility of last text node in renderer: " << curProp->GetVisibility() << std::endl;
    return EXIT_FAILURE;
  }





  vtkPropCollection* props = rr->GetViewProps();
  props->InitTraversal();
  for(int i = 0; i < props->GetNumberOfItems(); i++)
    {
    vtkProp* curProp = props->GetNextProp();
    //std::cout << curProp->GetPickable() << std::endl;
    //std::cout << curProp->GetDragable() << std::endl;
    std::cout << curProp->GetVisibility() << std::endl;
    }


  textNode1->SetVisible(0);
  //textNode1->SetLocked(1);
  textNode2->SetVisible(0);
  //textNode2->SetLocked(1);
  textNode3->SetVisible(0);
  //textNode3->SetLocked(1);
*/
  // cleanup

  scene->RemoveNode(textNode1);
  scene->RemoveNode(textNode2);
  scene->RemoveNode(textNode3);
  scene->RemoveNode(textNode4);

  textNode1->Delete();
  textNode2->Delete();
  textNode3->Delete();
  textNode4->Delete();
  renderRequestCallback->Delete();
  if (displayableManagerGroup) { displayableManagerGroup->Delete(); }
  factory->Delete();
  applicationLogic->Delete();
  scene->Delete();
  rr->Delete();
  rw->Delete();
  ri->Delete();


  return EXIT_SUCCESS;
}

