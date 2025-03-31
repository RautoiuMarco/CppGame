#include "PasswordManager.h"
#include <regex>

PASSWORD_API bool VerifyPassword(const std::string& password)
{
	std::regex passwordRegex("(?=.*[a-z])(?=.*[A-Z])(?=.*[0-9])(?=.*[^a-zA-Z0-9]).+");
	return std::regex_match(password, passwordRegex);
}
