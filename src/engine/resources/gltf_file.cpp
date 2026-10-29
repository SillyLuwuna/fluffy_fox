#include "engine/resources/gltf_file.hpp"
#include <iostream>

using namespace fluffy_fox;

GltfFile::GltfFile(const std::string& id) :
	Resource(id)
{ }

GltfFile::~GltfFile()
{
	unload();
}

bool GltfFile::do_load()
{
	tinygltf3::ErrorStack errors;
	tg3_error_code err = tinygltf3::parse_file(model_, errors, get_id().c_str());

	if (err != TG3_OK)
	{
		print_error(errors);
		return false;
	}

	return true;
}


void GltfFile::do_unload()
{
	if (!is_loaded()) return;

	model_ = {};
}

void GltfFile::print_error(tinygltf3::ErrorStack& errors)
{
	std::string error;
	for (uint32_t i = 0; i < errors.count(); i++)
	{
		error += "[" + std::to_string((int)errors.entry(i)->severity) + "] ";
		error += errors.entry(i)->message ? errors.entry(i)->message : "(null)";
	}
	std::cout << error << "\n";
}

i32 GltfFile::find_attribute(const tg3_primitive& primitive, const char* name)
{
	for (u32 i = 0; i < primitive.attributes_count; i++)
	{
		if (tg3_str_equals_cstr(primitive.attributes[i].key, name))
		{
			return primitive.attributes[i].value;
		}
	}

	return -1;
}
