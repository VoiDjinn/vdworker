#include "VDTaskWait.h"

VDTaskWait::VDTaskWait(){}

void VDTaskWait::_bind_methods()
{
	ClassDB::bind_method(D_METHOD("_handle_timeout"), &VDTaskWait::_handle_timeout);

  ClassDB::bind_method(D_METHOD("set_step_time", "step_time"), &VDTaskWait::set_step_time);
  ClassDB::bind_method(D_METHOD("get_step_time"),              &VDTaskWait::get_step_time);

  ClassDB::bind_method(D_METHOD("set_total_time", "total_time"), &VDTaskWait::set_total_time);
  ClassDB::bind_method(D_METHOD("get_total_time"),               &VDTaskWait::get_total_time);

  ADD_PROPERTY(PropertyInfo(Variant::REAL, "step_time"),  "set_step_time",  "get_step_time");
  ADD_PROPERTY(PropertyInfo(Variant::REAL, "total_time"), "set_total_time", "get_total_time");

}

void VDTaskWait::_notification(int what){}

void VDTaskWait::init()
{
	progress = 0.0;
	timer = memnew(Timer);
  timer->set_wait_time(step_time);
	timer->connect("timeout", this,"_handle_timeout");
	SceneTree::get_singleton()->get_current_scene()->add_child(timer);

}

void VDTaskWait::finish()
{
	timer->disconnect("timeout",this,"_handle_timeout");
	SceneTree::get_singleton()->get_current_scene()->remove_child(timer);
  memdelete(timer);
	timer = NULL;
}

bool VDTaskWait::execute()
{
	if (progress < 1.0)
  {
		timer->start();
		get_owning_job()->pause();
		return false;
	}
  else
  {
		return true;
  }
}

void VDTaskWait::_handle_timeout()
{
	progress += (step_time  / total_time);
	update_progress(progress);
	if (progress >= 1.0)
  {
		get_owning_job()->start();
  }
}

void VDTaskWait::set_step_time(float step_time)
{
  this->step_time = step_time;
  if (timer != NULL)
  {
    timer->set_wait_time(step_time);
  }
}

float VDTaskWait::get_step_time()
{
  return step_time;
}

void VDTaskWait::set_total_time(float total_time)
{
  this->total_time = total_time;
}

float VDTaskWait::get_total_time()
{
  return total_time;
}

Timer* VDTaskWait::get_internal_timer()
{
  return timer;
}
