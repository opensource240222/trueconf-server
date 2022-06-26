/*
 * © 2012, 2013, 2014, 2015, Artem Boldarev <artem.boldarev@gmail.com>
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef _PARG_H
#define _PARG_H

/* Plan9-like simple command line parser */

#include <stdlib.h> /* for exit() */

/* Begin arguments parsing.
 * argc - arguments count
 * argv - pointer to arguments
 * argv0 - pointer, will be point to argv[0] - executable name
 * So, it look like:
 * ARGBEGIN(int argc, char **argv, char *argv0)
 */
#define ARGBEGIN \
	{ \
		char __arg_ch; \
		int __arg_argc;\
		char **__arg_argv; \
		__arg_argc = argc, __arg_argv = argv; \
		if ( argv0 == NULL ) \
			argv0 = __arg_argv[0]; \
		for ( __arg_argc--, __arg_argv++; \
				__arg_argv[0] && __arg_argv[0][0] == '-' && __arg_argv[0][1] != '\0'; \
				__arg_argc--, __arg_argv++ ) \
		{ \
			char *__arg_args, *__arg_argt; \
			__arg_args = &__arg_argv[0][1]; \
			if ( __arg_args[0] == '-' && __arg_args[1] == '\0' ) \
			{ \
				__arg_argc--; __arg_argv++; \
				break; \
			} \
			__arg_ch = 0; \
			while (*__arg_args && (__arg_args+=((__arg_ch = __arg_args[0]) != '\0' ? 1 : 0)) ) \
			{ \
				switch ( __arg_ch )

/* End arguments parsing */
#define ARGEND \
			} \
		} \
	argc = __arg_argc; \
	argv = __arg_argv; \
	}


/* Get current argument name */
#define ARGC() (__arg_ch)

/* Get current argument parameters */
#define ARGF() (__arg_argt=__arg_args, __arg_args=(char *)"", \
		(*__arg_argt ? __arg_argt : __arg_argv[1] ? \
		 (__arg_argc--, *++__arg_argv) : NULL ) )

/* Get current argument parameters, end execute function (for error handling) */
#define EARGF(x) (__arg_argt=__arg_args, __arg_args=(char *)"", \
		(*__arg_argt ? __arg_argt : __arg_argv[1] ? \
		 (__arg_argc--, *++__arg_argv) : (char *)((x), exit(1), NULL) ) )

/* Check if string is command line flag */
#define ISARG(s) ( s[0] == '-' && s[1] != '\0' )

#endif /* _PARG_H */

