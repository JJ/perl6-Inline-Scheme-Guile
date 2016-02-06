#include <libguile.h>
#include <stdio.h>

static void dump_scm( SCM scm )
	{
	printf("null              X: %d\n", scm_is_null              ( scm ));
	printf("bool              X: %d\n", scm_is_bool              ( scm ));
	printf("false             X: %d\n", scm_is_false             ( scm ));
	printf("true              X: %d\n", scm_is_true              ( scm ));
	printf("integer           X: %d\n", scm_is_integer           ( scm ));
	printf("string            X: %d\n", scm_is_string            ( scm ));
	printf("symbol            X: %d\n", scm_is_symbol            ( scm ));
	printf("array              : %d\n", scm_is_array             ( scm ));
	printf("bitvector          : %d\n", scm_is_bitvector         ( scm ));
	printf("bytevector         : %d\n", scm_is_bytevector        ( scm ));
	printf("complex           X: %d\n", scm_is_complex           ( scm ));
	printf("dynamic_state      : %d\n", scm_is_dynamic_state     ( scm ));
/*
	printf("exact              : %d\n", scm_is_exact             ( scm ));
*/
	printf("generalized_vector : %d\n", scm_is_generalized_vector( scm ));
/*
	printf("inexact            : %d\n", scm_is_inexact           ( scm ));
*/
	printf("keyword           X: %d\n", scm_is_keyword           ( scm ));
	printf("number            X: %d\n", scm_is_number            ( scm ));
	printf("rational          X: %d\n", scm_is_rational          ( scm ));
	printf("real              X: %d\n", scm_is_real              ( scm ));
	//printf("signed_integer     : %d\n", scm_is_signed_integer    ( scm ));
	printf("simple_vector      : %d\n", scm_is_simple_vector     ( scm ));
	//printf("typed_array        : %d\n", scm_is_typed_array       ( scm ));
	printf("uniform_vector     : %d\n", scm_is_uniform_vector    ( scm ));
	//printf("unsigned_integer   : %d\n", scm_is_unsigned_integer  ( scm ));
	printf("vector             : %d\n", scm_is_vector            ( scm ));
	printf("pair               : %d\n", scm_is_pair              ( scm ));
        printf("\n");
	}

typedef enum
	{
	VECTOR_START  = -256,
	VECTOR_END    = -255,

	UNKNOWN_TYPE  = -2,
	VOID          = -1,
	ZERO          = 0,
	TYPE_NIL      = 1, // Yes, redundant, but matching the Perl...
	TYPE_BOOL     = 2,
	TYPE_INTEGER  = 3,
	TYPE_STRING   = 4,
	TYPE_DOUBLE   = 5,
	TYPE_RATIONAL = 6,
	TYPE_COMPLEX  = 7,
	TYPE_SYMBOL   = 8,
	TYPE_KEYWORD  = 9,
	}
	cons_cell_type;

typedef struct
	{
	double real_part;
	double imag_part;
	}
	complex_value;

typedef struct
	{
	double numerator_part;
	double denominator_part;
	}
	rational_value;

typedef struct
	{
	cons_cell_type type;
	union
		{
		long  int_content;
		char* string_content;
		double double_content;
		rational_value rational_content;
		complex_value complex_content;
		};
	}
	cons_cell;

static size_t _num_cells(); // Forward declaration

static size_t _num_vector_cells( SCM scm )
	{
	size_t num_cells = 2; // VECTOR_START and VECTOR_END
	int num_values = scm_c_vector_length( scm );
	int i;

	// Note _num_vector_cells calls _num_cells, so it should do the
	// recursive thing.
	for ( i = 0; i < num_values; i++ )
		{
		SCM _scm = scm_c_vector_ref( scm, i );
		num_cells += _num_cells( _scm );
		}

	return num_cells;
	}

static size_t _num_cells( SCM scm )
	{
	if ( scm_is_bool(     scm ) ) return 1; // '#t', '#f', '#nil'
	if ( scm_is_integer(  scm ) ) return 1; // '2'
	if ( scm_is_string(   scm ) ) return 1; // '"foo"'
	if ( scm_is_symbol(   scm ) ) return 1; // "'a"
	if ( scm_is_keyword(  scm ) ) return 1; // '#:foo'
	if ( scm_is_real(     scm ) ) return 1; // '-1.2'
	if ( scm_is_rational( scm ) ) return 1; // '-1/2'
	if ( scm_is_complex(  scm ) ) return 1; // '1+2i'

	if ( scm_is_vector(   scm ) ) return _num_vector_cells( scm ); // #(1 2)

	if ( scm_is_true( scm ) ) return 1;
printf("Uncovered cell type!\n");
	return 0;
	}

static size_t _count_cells( SCM scm )
	{
	size_t num_cells = 1; // Start with the sentinel
	int num_values = scm_c_nvalues( scm );
	int i;

	for ( i = 0; i < num_values; i++ )
		{
		SCM _scm = scm_c_value_ref( scm, i );
		num_cells += _num_cells( _scm );
		}

	return num_cells;
	}

static size_t _scm_to_cell( SCM scm, cons_cell* cell ); // Forward declaration

static size_t _scm_vector_to_cell( SCM scm, cons_cell* cell )
	{
	int num_values = scm_c_vector_length( scm );
	int i;

	cell->type = VECTOR_START;
	cell++;
	for ( i = 0; i < num_values; i++ )
		{
		SCM _scm = scm_c_vector_ref( scm, i );
		cell += _scm_to_cell( _scm, cell );
		}

	cell->type = VECTOR_END;
	cell++;
	return num_values + 2;
	}

static size_t _scm_to_cell( SCM scm, cons_cell* cell )
	{
	if ( scm_is_bool( scm ) )
		{
		if ( scm_is_false( scm ) )
			{
			// '#nil' is null, bool, false
			//
			if ( scm_is_null( scm ) )
				{
//printf("Nil\n");
				cell->type = TYPE_NIL;
				}
			// '#f' is not null, bool, false
			//
			else
				{
//printf("False\n");
				cell->type = TYPE_BOOL;
				cell->int_content = 0;
				}
			}
		// '#t' is not null, bool, not false, true
		//
		else
			{
//printf("True\n");
			cell->type = TYPE_BOOL;
			cell->int_content = 1;
			}
		return 1;
		}

	// '2' is an integer
	//
	if ( scm_is_integer( scm ) )
		{
//printf("Integer\n");
		cell->type = TYPE_INTEGER;
		cell->int_content = scm_to_int( scm );
		return 1;
		}

	// '""' is an string
	//
	if ( scm_is_string( scm ) )
		{
//printf("String\n");
		cell->type = TYPE_STRING;
		cell->string_content = scm_to_locale_string( scm );
		return 1;
		}

	// "'a" is an symbol
	//
	if ( scm_is_symbol( scm ) )
		{
//printf("Symbol\n");
		cell->type = TYPE_SYMBOL;
		cell->string_content =
			scm_to_locale_string( scm_symbol_to_string( scm ) );
		return 1;
		}

	// '#:a" is an keyword
	//
	if ( scm_is_keyword( scm ) )
		{
//printf("keyword\n");
		cell->type = TYPE_KEYWORD;
		cell->string_content =
			scm_to_locale_string( scm_symbol_to_string( scm_keyword_to_symbol( scm ) ) );
		return 1;
		}

	// '-1.2' is a real
	//
	if ( scm_is_real( scm ) )
		{
//printf("Real\n");
		cell->type = TYPE_DOUBLE;
		cell->double_content = scm_to_double( scm );
		return 1;
		}

	// '-1/2' is a rational (and complex, so test before complex)
	//
	if ( scm_is_rational( scm ) )
		{
//printf("rational\n");
		cell->type = TYPE_RATIONAL;
		cell->rational_content.numerator_part =
			scm_to_double( scm_numerator( scm ) );
		cell->rational_content.denominator_part =
			 scm_to_double( scm_denominator( scm ) );
		return 1;
		}

	// '-1i+2' is a complex
	//
	if ( scm_is_complex( scm ) )
		{
//printf("complex\n");
		cell->type = TYPE_COMPLEX;
		cell->complex_content.real_part =
			scm_c_real_part( scm );
		cell->complex_content.imag_part =
			 scm_c_imag_part( scm );
		return 1;
		}

	// '#(1 2 3)' is a vector, remember it can include other things.
	//
	if ( scm_is_vector( scm ) )
		{
//printf("vector\n");
		return _scm_vector_to_cell( scm, cell );
		}

	// '' is true and only 1 value, apparently.
	//
	if ( scm_is_true( scm ) )
		{
//printf("Void (fallback)\n");
		cell->type = VOID;
		return 1;
		}
printf("Unknown cell type, not advancing pointer.\n");
	return 0;
	}

static void _walk_scm( SCM scm, cons_cell* result )
	{
	int num_values = scm_c_nvalues( scm );
	int i;

	for ( i = 0; i < num_values; i++ )
		{
		SCM _scm = scm_c_value_ref( scm, i );
		result += _scm_to_cell( _scm, result );
		}
	result->type = ZERO;
	}

void* _run( void* expression )
	{
	SCM str = scm_from_utf8_string( (char*) expression );
	SCM scm = scm_eval_string( str );

	// Sigh, special-case void lists.
	if ( scm_c_nvalues( scm ) == 0 )
		{
		cons_cell* result = malloc( sizeof( cons_cell ) * 2 );
		result[0].type = VOID;
		result[1].type = ZERO;
		return result;
		}

	size_t num_values = _count_cells( scm );
	cons_cell* result = malloc( sizeof( cons_cell ) * num_values );

	_walk_scm( scm, result );

	return result;
	}

void* __dump( void* _expression )
	{
	char* expression = (char*) _expression;
	SCM str = scm_from_utf8_string( expression );
	SCM scm = scm_eval_string( str );

	printf("SCM object from '%s' returns %d cells\n",
		expression,
		(int)scm_c_nvalues( scm ));
	dump_scm( scm );
	printf("SCM 0th cell\n");
	dump_scm( scm_c_value_ref( scm, 0 ) );

	return expression;
	}

void _dump( const char* expression )
	{
	(void)scm_with_guile( __dump, (void*)expression );
	}

void run( const char* expression, void (*unmarshal(void*)) )
	{
	cons_cell* cells = scm_with_guile( _run, (void*)expression );
	cons_cell* head = cells;

	while( head->type != ZERO )
		{
		unmarshal(head++);
		}

	free(cells);
	}
