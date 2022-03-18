#ifndef VDTASKCHANGESCENE_H
#define VDTASKCHANGESCENE_H

#include "../VDWorker.h"
#include "scene/main/node.h"
#include "scene/main/scene_tree.h"
#include "scene/resources/packed_scene.h"

class VDTaskChangeScene : public VDTask {
	GDCLASS(VDTaskChangeScene, VDTask);

	static void sceneResultCallback(void * resultUserData, void * result);

	protected:
		static void _bind_methods();

		Ref<PackedScene> scene;
		Node *scene_node;
		bool done = false;
		int phase = 0;

		void _handle_node_ready();

	public:
		VDTaskChangeScene();

		void set_target_scene(Ref<PackedScene> target_scene);
		Ref<PackedScene> get_target_scene() const;
		void register_for_task_result(Ref<VDTask> task);

		virtual void init() override;
		virtual bool execute() override;
		virtual void finish() override;
};

#endif
