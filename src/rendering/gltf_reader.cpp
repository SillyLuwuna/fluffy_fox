#include "rendering/gltf_reader.hpp"
#include <iostream>

using namespace fluffy_fox::rendering;

GltfReader::GltfReader() :
	loaded_(false)
{
}

GltfReader::~GltfReader()
{

}

bool GltfReader::load(const std::string& file)
{
	tinygltf3::ErrorStack errors;
	tg3_error_code err = tinygltf3::parse_file(model_, errors, file.c_str());

	if (err != TG3_OK)
	{
		print_error(errors);
		return false;
	}

	return true;
}

void GltfReader::print_error(tinygltf3::ErrorStack& errors)
{
	std::string error;
	for (uint32_t i = 0; i < errors.count(); i++)
	{
		error += "[" + std::to_string((int)errors.entry(i)->severity) + "] ";
		error += errors.entry(i)->message ? errors.entry(i)->message : "(null)";
	}
	std::cout << error << "\n";
}

i32 GltfReader::find_attribute(const tg3_primitive& primitive, const char* name)
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
