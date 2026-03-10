#pragma once

#include <functional>
#include <string>
namespace util
{
	template<typename T>
	class NamedFunction
	{
		private:
			std::function<T> m_func;
			std::string m_name;

		public:
			NamedFunction(const std::function<T>& func, const std::string& name);
			const std::string& get_name() const;

			template<typename... Args>
			typename std::result_of<std::function<T>(Args...)>::type
			operator()(Args&&... args);

			bool operator==(const NamedFunction& other) const;
	};

	struct NamedFunctionHash
	{
		template<typename T>
		std::size_t operator()(const NamedFunction<T>& func) const;
	};
}

#include "named_function.ipp"
