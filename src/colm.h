#ifndef __COLM_COLM_H
#define __COLM_COLM_H

#ifdef __cplusplus
extern "C" {
#endif

struct colm_data;
struct colm_tree;
struct colm_kid;
struct colm_program;
struct colm_sections;
struct colm_tree;
struct colm_location;

extern struct colm_sections colm_object;

typedef unsigned long colm_value_t;

struct colm_tree
{
	/* First four will be overlaid in other structures. */
	short id;
	unsigned short flags;
	long refs;
	struct colm_kid *child;

	struct colm_data *tokdata;

	/* FIXME: this needs to go somewhere else. Will do for now. */
	unsigned short prod_num;
};

struct colm_print_args
{
	void *arg;
	int comm;
	int attr;
	int trim;

	void (*out)( struct colm_print_args *args, const char *data, int length );
	void (*open_tree)( struct colm_program *prg, struct colm_tree **sp, 
		struct colm_print_args *args, struct colm_kid *parent, struct colm_kid *kid );
	void (*print_term)( struct colm_program *prg, struct colm_tree **sp, 
		struct colm_print_args *args, struct colm_kid *kid );
	void (*close_tree)( struct colm_program *prg, struct colm_tree **sp, 
		struct colm_print_args *args, struct colm_kid *parent, struct colm_kid *kid );
};

void colm_print_null( struct colm_program *prg, struct colm_tree **sp,
		struct colm_print_args *args, struct colm_kid *parent, struct colm_kid *kid );
void colm_print_term_tree( struct colm_program *prg, struct colm_tree **sp,
		struct colm_print_args *print_args, struct colm_kid *kid );

struct colm_tree **colm_vm_root( struct colm_program *prg );
struct colm_tree *colm_return_val( struct colm_program *prg );
void colm_print_tree_args( struct colm_program *prg, struct colm_tree **sp,
		struct colm_print_args *print_args, struct colm_tree *tree );

int colm_repeat_end( struct colm_tree *tree );
int colm_list_last( struct colm_tree *tree );

struct colm_tree *colm_get_rhs_val( struct colm_program *prg, struct colm_tree *tree, int *a );
struct colm_tree *colm_get_attr( struct colm_tree *tree, long pos );
struct colm_tree *colm_get_global( struct colm_program *prg, long pos );
struct colm_tree *colm_get_repeat_next( struct colm_tree *tree );
struct colm_tree *colm_get_repeat_val( struct colm_tree *tree );
struct colm_location *colm_find_location( struct colm_program *prg, struct colm_tree *tree );

/* Debug realms. To turn on, pass to colm_set_debug before invocation. */
#define COLM_DBG_BYTECODE    0x00000001
#define COLM_DBG_PARSE       0x00000002
#define COLM_DBG_MATCH       0x00000004
#define COLM_DBG_COMPILE     0x00000008
#define COLM_DBG_POOL        0x00000010
#define COLM_DBG_PRINT       0x00000020
#define COLM_DBG_INPUT       0x00000040
#define COLM_DBG_SCAN        0x00000080

#define COLM_RN_NEITHER 0x00
#define COLM_RN_DATA    0x01
#define COLM_RN_LOC     0x02
#define COLM_RN_BOTH    0x03

/*
 * Primary Interface.
 */

/* Allocate a program. Takes program static data as arg. Normally this is
 * &colm_object. */
struct colm_program *colm_new_program( struct colm_sections *rtd );

/* Enable debug realms for a program. */
void colm_set_debug( struct colm_program *prg, long active_realm );

/* Run a top-level colm program. */
void colm_run_program( struct colm_program *prg, int argc, const char **argv );

/* Run a single exported colm function. */
struct colm_tree *colm_run_func( struct colm_program *prg, int frame_id,
		const char **params, int param_count );

/* Delete a colm program. Clears all memory. */
int colm_delete_program( struct colm_program *prg );

/* Set the pointer to the reduce struct used. */
void colm_set_reduce_ctx( struct colm_program *prg, void *ctx );

const char **colm_extract_fns( struct colm_program *prg );

#ifdef __cplusplus
}
#endif

#endif
