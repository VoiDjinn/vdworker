#ifndef VDTASKLOADRESOURCE_H
#define VDTASKLOADRESOURCE_H

#include "../VDWorker.h"
#include "core/io/resource_loader.h"

class VDTaskLoadResource : public VDTask {
	GDCLASS(VDTaskLoadResource, VDTask);

	Ref<ResourceInteractiveLoader> loader;

	protected:
		static void _bind_methods();

		String resource_path;
		int max_load_progress = 0;
		Ref<Resource> result;

	public:
		VDTaskLoadResource();

		virtual void init() override;
		virtual bool execute() override;
		virtual void finish() override;

		void set_resource_path(String path);
};

#endif
