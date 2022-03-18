#include "register_types.h"
#include "core/class_db.h"
#include "core/engine.h"

#include "VDWorker.h"
#include "tasks/VDTaskScript.h"
#include "tasks/VDTaskLoadResource.h"
#include "tasks/VDTaskChangeScene.h"
#include "tasks/VDTaskWait.h"

void register_vdworker_types() {
  ClassDB::register_class<VDWorker>();
  VDWorker::singleton.instance();
  Engine::get_singleton()->add_singleton(Engine::Singleton("VDWorker", VDWorker::get_singleton()));

  ClassDB::register_class<VDJob>();
  ClassDB::register_virtual_class<VDTask>();
  ClassDB::register_class<VDTaskScript>();
  ClassDB::register_class<VDTaskLoadResource>();
  ClassDB::register_class<VDTaskChangeScene>();
  ClassDB::register_class<VDTaskWait>();
}

void unregister_vdworker_types() {
  VDWorker::singleton.unref();
}
