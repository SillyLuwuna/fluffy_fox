#pragma once

#include <fluff/types.hpp>
using namespace fluff::types;

namespace fluffy_fox
{
	class ComponentTypeIDSystem
	{
	private:
		static inline u64 next_type_id = 0;

	public:
		template<typename T>
		static u64 get_type_id()
		{
			static u64 type_id = next_type_id++;
			return type_id;
		}
	};
}
