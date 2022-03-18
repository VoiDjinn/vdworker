#ifndef VDTASKSCRIPT_H
#define VDTASKSCRIPT_H

#include "../VDWorker.h"

class VDTaskScript : public VDTask {
	GDCLASS(VDTaskScript, VDTask);
	protected:
		static void _bind_methods();
		void _notification(int what);
	public:
		VDTaskScript();

		virtual void init() override;
		virtual bool execute() override;
		virtual void finish() override;
		void fire_update_progress ( float progress );
};

#endif

