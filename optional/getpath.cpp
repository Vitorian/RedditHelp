
#include <filesystem>
#include <optional>

bool GetPath(const std::string& value, std::filesystem::path& path)
{
	if (value.empty())
		return false;
	path = std::filesystem::absolute(value);
	return true;
}

std::optional<std::filesystem::path> GetPath(const std::string& value)
{
	if (value.empty())
		return {};
	return std::make_optional(std::filesystem::absolute(value));
}
