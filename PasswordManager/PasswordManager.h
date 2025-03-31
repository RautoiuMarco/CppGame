#pragma once
#include <string>

#ifdef PASSWORDMANAGER_EXPORTS
#define PASSWORD_API __declspec(dllexport)
#else
#define PASSWORD_API __declspec(dllimport)
#endif

extern "C" PASSWORD_API bool VerifyPassword(const std::string& password);