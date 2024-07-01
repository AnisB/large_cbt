#pragma once

#ifdef assert
#undef assert
#endif

// Function that handles a fail
void __handle_fail(const char* msg, const char* file_name, int line);

// Assert functions
#define assert_fail_msg(msg) __handle_fail(msg, __FILE__, __LINE__)
#define assert_fail() assert_fail_msg("")
#define assert_msg(condition, msg) if(!(condition)) assert_fail_msg(msg)
#define assert(condition) assert_msg(condition, "")
#define not_implemented() assert_fail_msg("function not implemented")
