#include "VDTaskChangeScene.h"

#include "core/error_macros.h"

VDTaskChangeScene::VDTaskChangeScene() {}

void VDTaskChangeScene::_bind_methods()
{
     ClassDB::bind_method ( D_METHOD ( "register_for_task_result", "task" ), &VDTaskChangeScene::register_for_task_result );
     ClassDB::bind_method ( D_METHOD ( "_handle_node_ready" ), &VDTaskChangeScene::_handle_node_ready );
}

void VDTaskChangeScene::sceneResultCallback ( void * resultUserData, void * result )
{
     Ref<VDTaskChangeScene> this_task = Ref<VDTaskChangeScene> ( ( VDTaskChangeScene * ) resultUserData );
     this_task->scene = Ref<PackedScene> ( ( PackedScene * ) result );
//      Ref<VDTaskChangeScene> this_task = Ref<VDTaskChangeScene> ( resultUserData );
//      this_task->scene = Ref<PackedScene> ( result );
}

void VDTaskChangeScene::init()
{
     phase = 0;
     done = false;
}

bool VDTaskChangeScene::execute()
{
     switch ( phase ) {
     case 0:
          if ( scene.is_valid() && scene->can_instance() ) {
               scene_node = scene->instance();
          }
          update_progress ( 0.2 );
          phase++;
          break;
     case 1:
          if ( scene_node ) {
               update_progress ( 0.4 );
               phase++;
          }
          break;
     case 2:
          scene_node->connect ( "ready", this, "_handle_node_ready", Vector<Variant>(), CONNECT_ONESHOT );
          SceneTree::get_singleton()->call_deferred ( "_change_scene", scene_node );
//           SceneTree::get_singleton()->change_scene_to ( scene );
          update_progress ( 0.6 );
          get_owning_job()->pause();
          break;
     case 3:
          update_progress ( 0.8 );
          phase++;
          break;
     case 4:
          update_progress ( 1.0 );
          done = true;
          break;
     }
     return done;
}

void VDTaskChangeScene::finish() {}

void VDTaskChangeScene::_handle_node_ready()
{
     phase++;
     get_owning_job()->start();
}

void VDTaskChangeScene::register_for_task_result ( Ref<VDTask> task )
{
     task->set_result_callback ( VDTaskChangeScene::sceneResultCallback, this );
}

void VDTaskChangeScene::set_target_scene ( Ref<PackedScene> target_scene )
{
     this->scene = target_scene;
}

Ref<PackedScene> VDTaskChangeScene::get_target_scene() const
{
     return scene;
}
