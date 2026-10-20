#include "engine/resource.hpp"

using namespace fluffy_fox;

Resource::Resource(const std::string& id) :
	resource_id_(id),
	loaded_(false)
{

}

const std::string& Resource::get_id() const
{
	return resource_id_;
}

bool Resource::is_loaded() const
{
	return loaded_;
}

bool Resource::load()
{
	loaded_ = do_load();
	return loaded_;
}

void Resource::unload()
{
	do_unload();
	loaded_ = false;
}
