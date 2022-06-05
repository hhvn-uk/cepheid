#include <stddef.h>

#define MAXGROUPS 256
#define TODO

/* -----
 * List functions
 */

/* Set *ret to a list of groups, and return the length of the list.
 * The _f variant accepts a filter function (zero = ignore entry):
 *  dblistgroups(..., ...) is equivalent to dblistgroups_f(..., ..., NULL, NULL)
 *  The pointer fdata is passed to the filter function.
 * On failure, set *ret to NULL, and return 0;
 * The data returned by this function must be free'd using dblistfree()
 * Errors may be those of scandir(3) or:
 * 	EINVAL	ret, dir or group are NULL
 * 	ENOMEM	allocation function failed
 */
size_t dblistgroups(char ***ret, char *dir);
size_t dblistgroups_f(char ***ret, char *dir, int (*filter)(void *data, char *path), void *fdata);

/* Set *ret to a list of keys, and return the length of the list.
 * The _f variant accepts a filter function (zero = ignore entry):
 *  dblistkeys(..., ...) is equivalent to dblistkeys_f(..., ..., NULL, NULL)
 *  The pointer fdata is passed to the filter function.
 * On failure, set *ret to NULL, and return 0;
 * The data returned by this function must be free'd using dblistfree()
 * Errors:
 * 	EINVAL	ret, dir or group are NULL
 * 	ENOENT	no such group
 * 	ENOMEM	allocation function failed
 */
size_t dblistkeys(char ***ret, char *dir, char *group);
size_t dblistkeys_f(char ***ret, char *dir, char *group, int (*filter)(void *data, char *path), void *fdata);

/* Returns a duplicated list, or NULL for failure.
 * The data returned by this function must be free'd using dblistfree()
 * Errors:
 * 	EINVAL	list is NULL, or len is 0
 * 	ENOMEM	allocation function failed
 */
char **dblistdup(char **list, size_t len);

/* Free memory allocated by:
 * dblistgroups(), dblistkeys(), dblistdup() */
void dblistfree(char **list, size_t len);

/* Returns 1 if lists are equal, and 0 if not. */
int dblisteq(char **l1, size_t s1, char **l2, size_t s2);

/* -----
 * Manipulating values
 */

/* Returns a value, or NULL for failure.
 * Errors:
 * 	EINVAL	dir, group, or key are NULL
 * 	ENOENT	no such group
 */
char *dbget(char *dir, char *group, char *key);

/* Set a value.
 * Returns 0 for success, or -1 for failure.
 * Errors:
 * 	EINVAL	dir, group, or key is NULL
 * 	ENOENT	no such group
 * 	ENOMEM	allocation function failed
 */
int dbset(char *dir, char *group, char *key, char *val);

/* -----
 * Deleting groups/pairs
 */

/* Free a group and delete it from storage.
 * Returns 0 for success or -1 for failure.
 * Errors are that of unlink(3) or:
 * 	EINVAL	dir or group are NULL
 * 	ENOENT	no such group
 */
int dbdelgroup(char *dir, char *group);

/* Free a pair and remove it from the group.
 * dbwrite() or dbwritegroup() must still be called on
 * the parent group to remove this pair from storage.
 * Returns 0 for success or -1 for failure.
 * Errors:
 * 	EINVAL	dir, group or key is NULL
 * 	ENOENT	no such group/key
 */
int dbdelpair(char *dir, char *group, char *key);

/* -----
 * Writing db data to storage
 */

/* Write a group to file.
 * Returns 0 for success or -1 for failure.
 * Errors are that of mkdir(3), fopen(3), or:
 * 	EINVAL	dir or group are NULL
 * 	ENOENT	no such group
 */
int dbwritegroup(char *dir, char *group);

/* Writes all groups in a directory to file.
 * Returns 0 for success or -1 for failure.
 * Every write is attempted even if one fails.
 * Errors are that of mkdir(3), fopen(3), or:
 * 	EINVAL	dir is NULL
 */
int dbwrite(char *dir);

/* -----
 * Cleanup
 */

/* Discards and frees the content of groups in a directory. */
void dbfree(char *dir);

/* Discards and frees the content of a group. */
void dbfreegroup(char *dir, char *group);

/* Discards and frees everything used by db.c.
 * Data from dblistgroups(), dblistkeys() and dblistdup() are excluded:
 * dblistfree() must be used on the data returned by these functions. */
void dbcleanup(void);
