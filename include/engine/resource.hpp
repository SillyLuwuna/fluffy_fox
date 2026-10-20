#pragma once

#include <string>

namespace fluffy_fox
{
	class Resource
	{
	private:
		std::string resource_id_;
		bool loaded_;

	public:
		explicit Resource(const std::string& id);
		virtual ~Resource() = default;

		const std::string& get_id() const;
		bool is_loaded() const;

		bool load();
		void unload();
	
	protected:
		virtual bool do_load() = 0;
		virtual void do_unload() = 0;
	};
}
