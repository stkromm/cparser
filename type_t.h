/*
 * This file is part of cparser.
 * Copyright (C) 2007-2008 Matthias Braun <matze@braunis.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */
#ifndef TYPE_T_H
#define TYPE_T_H

#include <stdbool.h>
#include <assert.h>

#include <libfirm/firm_types.h>

#include "type.h"
#include "symbol.h"
#include "token_t.h"
#include "ast_t.h"
#include "adt/obst.h"

extern struct obstack *type_obst;

typedef enum type_kind_t {
	TYPE_INVALID,
	TYPE_ERROR,
	TYPE_ATOMIC,
	TYPE_COMPLEX,
	TYPE_IMAGINARY,
	TYPE_COMPOUND_STRUCT,
	TYPE_COMPOUND_UNION,
	TYPE_ENUM,
	TYPE_FUNCTION,
	TYPE_POINTER,
	TYPE_ARRAY,
	TYPE_BITFIELD,
	TYPE_BUILTIN,
	TYPE_TYPEDEF,
	TYPE_TYPEOF,
} type_kind_t;

typedef enum type_modifier_t {
	TYPE_MODIFIER_NONE              = 0,
	TYPE_MODIFIER_TRANSPARENT_UNION = 1 << 0,
} type_modifier_t;
typedef unsigned short type_modifiers_t;

struct type_base_t {
	type_kind_t       kind;
	il_size_t         size;           /**< The size of this type. */
	type_qualifiers_t qualifiers;
	type_modifiers_t  modifiers;
	il_alignment_t    alignment;      /**< The extra alignment of the type, 0 for default. */

	ir_type          *firm_type;
};

struct atomic_type_t {
	type_base_t         base;
	atomic_type_kind_t  akind;
};

struct complex_type_t {
	type_base_t         base;
	atomic_type_kind_t  akind;
};

struct imaginary_type_t {
	type_base_t         base;
	atomic_type_kind_t  akind;
};

struct builtin_type_t {
	type_base_t  base;
	symbol_t    *symbol;
	type_t      *real_type;
};

struct pointer_type_t {
	type_base_t  base;
	type_t      *points_to;
};

struct array_type_t {
	type_base_t   base;
	type_t       *element_type;
	expression_t *size_expression;
	size_t        size;

	ir_node      *size_node; /**< used by ast2firm phase */

	bool          is_static         : 1; /**< a [static] type */
	bool          is_variable       : 1; /**< a [*] type */
	bool          has_implicit_size : 1;
	bool          size_constant     : 1; /**< size expression is constant */
	bool          is_vla            : 1; /**< it's a variable length array */
};

/**
 * An entry in the parameter list of a function type.
 */
struct function_parameter_t {
	type_t               *type;  /**< The parameter type. */
	function_parameter_t *next;  /**< Points to the next type inthe parameter list.*/
};

/** Linkage specifications. */
typedef enum linkage_kind_t {
	LINKAGE_INVALID,
	LINKAGE_C,       /**< C linkage. */
	LINKAGE_CXX      /**< C++ linkage. */
} linkage_kind_t;

/** Calling conventions. */
typedef enum cc_kind_t {
	CC_DEFAULT,      /**< default calling convention. */
	CC_CDECL,        /**< cdecl calling convention. */
	CC_STDCALL,      /**< stdcall calling convention. */
	CC_FASTCALL,     /**< fastcall calling convention. */
	CC_THISCALL      /**< thiscall calling convention. */
} cc_kind_t;

/**
 * A function type.
 */
struct function_type_t {
	type_base_t           base;
	type_t               *return_type;        /**< The return type. */
	function_parameter_t *parameters;         /**< A list of the parameter types. */
	linkage_kind_t        linkage;
	cc_kind_t             calling_convention; /**< The specified calling convention. */
	bool                  variadic : 1;
	bool                  unspecified_parameters : 1;
	bool                  kr_style_parameters : 1;
};

struct compound_type_t {
	type_base_t  base;
	bool         packed : 1; /**< Set if packed was specified. */
	/** the declaration of the compound type, the scope of the declaration
	 *  contains the compound entries. */
	compound_t  *compound;
};

struct enum_type_t {
	type_base_t  base;
	/** the enum entity. You can find the enum entries by walking the
	 *  enum->base.next list until you don't find ENTITY_ENUM_VALUE entities
	 *  anymore */
	enum_t      *enume;
};

struct typedef_type_t {
	type_base_t  base;
	typedef_t   *typedefe;
	type_t      *resolved_type;
};

struct typeof_type_t {
	type_base_t   base;
	expression_t *expression;
	type_t       *typeof_type;
	type_t       *resolved_type;
};

struct bitfield_type_t {
	type_base_t   base;
	type_t       *base_type;
	expression_t *size_expression; /**< The expression for the bit size. */
	il_size_t     bit_size;        /**< Size of this bitfield in bits. */
};

union type_t {
	type_kind_t      kind;
	type_base_t      base;
	atomic_type_t    atomic;
	complex_type_t   complex;
	imaginary_type_t imaginary;
	builtin_type_t   builtin;
	pointer_type_t   pointer;
	array_type_t     array;
	function_type_t  function;
	compound_type_t  compound;
	enum_type_t      enumt;
	typedef_type_t   typedeft;
	bitfield_type_t  bitfield;
	typeof_type_t    typeoft;
};

type_t *make_atomic_type(atomic_type_kind_t type, type_qualifiers_t qualifiers);
type_t *make_complex_type(atomic_type_kind_t type, type_qualifiers_t qualifiers);
type_t *make_imaginary_type(atomic_type_kind_t type, type_qualifiers_t qualifiers);
type_t *make_pointer_type(type_t *points_to, type_qualifiers_t qualifiers);
type_t *make_array_type(type_t *element_type, size_t size,
                        type_qualifiers_t qualifiers);

type_t *duplicate_type(const type_t *type);

static inline bool is_typeref(const type_t *type)
{
	return type->kind == TYPE_TYPEDEF || type->kind == TYPE_TYPEOF;
}

static inline bool is_type_atomic(const type_t *type, atomic_type_kind_t atype)
{
	assert(!is_typeref(type));

	if(type->kind != TYPE_ATOMIC)
		return false;
	const atomic_type_t *atomic_type = &type->atomic;

	return atomic_type->akind == atype;
}

static inline bool is_type_pointer(const type_t *type)
{
	assert(!is_typeref(type));
	return type->kind == TYPE_POINTER;
}

static inline bool is_type_array(const type_t *type)
{
	assert(!is_typeref(type));
	return type->kind == TYPE_ARRAY;
}

static inline bool is_type_function(const type_t *type)
{
	assert(!is_typeref(type));
	return type->kind == TYPE_FUNCTION;
}

static inline bool is_type_union(const type_t *type)
{
	assert(!is_typeref(type));
	return type->kind == TYPE_COMPOUND_UNION;
}

static inline bool is_type_struct(const type_t *type)
{
	assert(!is_typeref(type));
	return type->kind == TYPE_COMPOUND_STRUCT;
}

static inline bool is_type_builtin(const type_t *type)
{
	assert(!is_typeref(type));
	return type->kind == TYPE_BUILTIN;
}

static inline bool is_type_compound(const type_t *type)
{
	assert(!is_typeref(type));
	return type->kind == TYPE_COMPOUND_STRUCT
		|| type->kind == TYPE_COMPOUND_UNION;
}

static inline bool is_type_valid(const type_t *type)
{
	assert(!is_typeref(type));
	return type->kind != TYPE_ERROR;
}

#endif
