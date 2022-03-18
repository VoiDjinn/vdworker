#include "VDTaskLoadResource.h"

#include "core/error_macros.h"

VDTaskLoadResource::VDTaskLoadResource() {}

void VDTaskLoadResource::_bind_methods(){
	ClassDB::bind_method(D_METHOD("set_resource_path", "path"), &VDTaskLoadResource::set_resource_path);
}

void VDTaskLoadResource::init()
{
	this->loader = ResourceLoader::load_interactive(this->resource_path);
	if(!this->loader.is_null()){
		this->max_load_progress = this->loader->get_stage_count();
	}
}

bool VDTaskLoadResource::execute() {
	bool done = false;
	Error progress = this->loader->poll();
	if(progress == Error::ERR_FILE_EOF){
		done = true;
		update_progress(1);
		result = this->loader->get_resource();
	}
	else if(progress == Error::OK){
		update_progress(this->loader->get_stage() / max_load_progress);
	}
	return done;
}

void VDTaskLoadResource::finish()
{
	if(this->loader.is_valid()){
		this->loader.unref();
	}
	if(this->result.is_valid()){
		this->emit_signal("result_returned", result);
		call_result_callback(result.ptr());
	}
}

void VDTaskLoadResource::set_resource_path(String path){
	resource_path = path;
}
