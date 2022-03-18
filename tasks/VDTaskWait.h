#ifndef VDTASKWAIT_H
#define VDTASKWAIT_H

#include "../VDWorker.h"

#include "scene/main/scene_tree.h"
#include "scene/main/timer.h"

class VDTaskWait : public VDTask {
	GDCLASS(VDTaskWait, VDTask);

  float progress;
  Timer* timer;

  float step_time  =  1.0;
  float total_time = 10.0;

	protected:
		static void _bind_methods();
		void _notification(int what);

    void _handle_timeout();

	public:
		VDTaskWait();

    virtual void init() override;
    virtual void finish() override;
    virtual bool execute() override;

    void set_step_time(float step_time);
    float get_step_time();

    void set_total_time(float total_time);
    float get_total_time();

    Timer* get_internal_timer();//with great power comes great responsibility

};


#endif

