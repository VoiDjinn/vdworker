#include "VDTaskScript.h"

VDTaskScript::VDTaskScript(){}

void VDTaskScript::_bind_methods() {
	ClassDB::bind_method(D_METHOD("update_progress", "progress"), &VDTaskScript::fire_update_progress);

	BIND_VMETHOD(MethodInfo("_on_init"));
	BIND_VMETHOD(MethodInfo("_on_execute"));
	BIND_VMETHOD(MethodInfo("_on_finish"));
}

void VDTaskScript::_notification(int what){}

void VDTaskScript::init()
{
	call("_on_init");
}

bool VDTaskScript::execute()
{
	return call("_on_execute");
}

void VDTaskScript::finish()
{
	call("_on_finish");
}

void VDTaskScript::fire_update_progress ( float progress ) // TODO: check name
{
	update_progress ( progress );
}
