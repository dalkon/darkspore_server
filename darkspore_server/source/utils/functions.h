
#ifndef _UTILS_FUNCTIONS_HEADER
#define _UTILS_FUNCTIONS_HEADER

// Include
#include <cstdint>
#include <vector>
#include <string>

// utils
namespace utils {
	namespace detail {
		template<class StrT, class DelimT, class StrRT = std::remove_cvref_t<StrT>>
		auto explode_string(StrT str, DelimT delim, int32_t limit) {
			std::vector<StrRT> strings;

			size_t start = 0, end = 0;
			while (--limit != -1 && (end = str.find(delim, start)) != StrRT::npos) {
				strings.push_back(str.substr(start, end - start));
				if constexpr (std::is_class_v<DelimT>) {
					start = end + delim.size();
				} else {
					start = end + sizeof(delim);
				}
			}

			strings.push_back(str.substr(start));
			return strings;
		}
	}

	uint64_t get_unix_time();

	std::string get_file_text(const std::string& path);

	void string_replace(std::string& str, const std::string& old_str, const std::string& new_str);

	std::vector<std::string> explode_string(const std::string& str, char delim, int32_t limit = -1);
	std::vector<std::string> explode_string(const std::string& str, const std::string& delim, int32_t limit = -1);

	std::vector<std::string_view> explode_string(std::string_view str, char delim, int32_t limit = -1);
	std::vector<std::string_view> explode_string(std::string_view str, std::string_view delim, int32_t limit = -1);
}

#endif
