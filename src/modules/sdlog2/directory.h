#pragma once

#include <stdint.h>

__BEGIN_DECLS

/*
 * Public declarations.
 */

__EXPORT extern const char sdlog2_root[];

typedef enum {
	SDLOG2_FILE_LOG,
	SDLOG2_FILE_PREFLIGHT,
	SDLOG2_FILE_POSTFLIGHT,
	SDLOG2_FILE_KIND_MAX
} sdlog2_file_kind_t;

__EXPORT int /* boolean result -- name defined or not */
sdlog2_filename(char filepath[/*PATH_MAX*/], const char dir[], sdlog2_file_kind_t kind);

__EXPORT uint32_t /* result == limit is found nothing, result < limit is an existing number */
sdlog2_dir_find_closest_number_lt(char full_path[/*PATH_MAX*/], uint32_t limit, const char root[]);

__EXPORT int /* boolean result -- found or not */
sdlog2_dir_find_by_number(char full_path[/*PATH_MAX*/], uint32_t number, const char root[]);

/*
 * src/module/sdlog2 internal functions.
 */

uint64_t __attribute__ (( visibility ("protected") ))
sdlog2_dir_size_recursive(const char path[]);

void __attribute__ (( visibility ("protected") ))
sdlog2_dir_remove_recursive(const char path[]);

void __attribute__ (( visibility ("protected") ))
sdlog2_dir_remove_oldest(const char root[]);

__END_DECLS
