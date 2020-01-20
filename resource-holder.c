#if 0 /* -*- mode: c; c-file-style: "stroustrup"; tab-width: 8; -*-
 set -euf; trg=${0##*''/}; trg=test-${trg%.c}; rm -f "$trg"
 #case ${1-} in '') set x -O2; shift; esac
 case ${1-} in '') set x -O0 -ggdb; shift; esac
 set -x; exec ${CC:-gcc} -std=c99 -DTEST -DTEST_SMALL "$@" -o "$trg" "$0"
 exit $?
 */
#endif
/*
 * $ resource-holder.c $
 *
 * Author: Tomi Ollila -- too ät iki piste fi
 *
 *      Copyright (c) 2015-2016 Tomi Ollila
 *	  All rights reserved
 *
 * Created: Thu 17 Dec 2015 21:04:16 EET too
 * Last modified: Mon 20 Jan 2020 20:20:20 +0200 too
 */

/* SPDX-License-Identifier: BSD-2-Clause */

// hint: gcc -dM -E -xc /dev/null | grep -i gnuc
// also: clang -dM -E -xc /dev/null | grep -i gnuc
#if defined (__GNUC__)

#if 0 // use of -Wpadded gets complicated, 32 vs 64 bit systems
#pragma GCC diagnostic warning "-Wpadded"
#endif

#if 1
#if __GNUC__ >= 5
#pragma GCC diagnostic warning "-Wsuggest-attribute=pure"
#pragma GCC diagnostic warning "-Wsuggest-attribute=const"
#pragma GCC diagnostic warning "-Wsuggest-attribute=noreturn"
#pragma GCC diagnostic warning "-Wsuggest-attribute=format"
#endif
#endif

#pragma GCC diagnostic warning "-Wall"
#pragma GCC diagnostic warning "-Wextra"

#if __GNUC__ >= 8 // impractically strict in gccs 5, 6 and 7
#pragma GCC diagnostic warning "-Wpedantic"
#endif

#if __GNUC__ >= 7
// gcc manual says all kind of /* fall.*through */ regexp's work too
// but perhaps only when cpp does not filter comments out. thus...
#define FALL_THROUGH __attribute__ ((fallthrough))
#else
#define FALL_THROUGH ((void)0)
#endif

#ifndef __cplusplus
#pragma GCC diagnostic warning "-Wstrict-prototypes"
#pragma GCC diagnostic warning "-Wbad-function-cast"
#pragma GCC diagnostic warning "-Wold-style-definition"
#pragma GCC diagnostic warning "-Wmissing-prototypes"
#pragma GCC diagnostic warning "-Wnested-externs"
#endif

// -Wformat=2 ¡currently! (2020-0202) equivalent of the following 4
#pragma GCC diagnostic warning "-Wformat"
#pragma GCC diagnostic warning "-Wformat-nonliteral"
#pragma GCC diagnostic warning "-Wformat-security"
#pragma GCC diagnostic warning "-Wformat-y2k"

#pragma GCC diagnostic warning "-Wcast-align"
#pragma GCC diagnostic warning "-Wpointer-arith"
#pragma GCC diagnostic warning "-Wwrite-strings"
#pragma GCC diagnostic warning "-Wcast-qual"
#pragma GCC diagnostic warning "-Wshadow"
#pragma GCC diagnostic warning "-Wmissing-include-dirs"
#pragma GCC diagnostic warning "-Wundef"

#ifndef __clang__ // XXX revisit -- tried with clang 3.8.0
#pragma GCC diagnostic warning "-Wlogical-op"
#endif

#ifndef __cplusplus // supported by c++ compiler but perhaps not worth having
#pragma GCC diagnostic warning "-Waggregate-return"
#endif

#pragma GCC diagnostic warning "-Wmissing-declarations"
#pragma GCC diagnostic warning "-Wredundant-decls"
#pragma GCC diagnostic warning "-Winline"
#pragma GCC diagnostic warning "-Wvla"
#pragma GCC diagnostic warning "-Woverlength-strings"
#pragma GCC diagnostic warning "-Wuninitialized"

//ragma GCC diagnostic error "-Wfloat-equal"
//ragma GCC diagnostic error "-Werror"
//ragma GCC diagnostic error "-Wconversion"

// avoiding known problems (turning some settings set above to ignored)...
#if __GNUC__ == 4
#ifndef __clang__
#pragma GCC diagnostic ignored "-Winline" // gcc 4.4.6 ...
#pragma GCC diagnostic ignored "-Wuninitialized" // gcc 4.4.6, 4.8.5 ...
#endif
#endif

#endif /* defined (__GNUC__) */

// note: these days compilers default to 'gnu'11 or newer. c99 still used here
#define _DEFAULT_SOURCE /* linux, glibc 2.19 or newer */
#define _SVID_SOURCE /* linux, glibc older than 2.19 */

#include <unistd.h>
#include <stddef.h> // for offsetof
#include <stdbool.h>
#include <sys/mman.h>

#ifndef NULL
#define NULL ((void *)0)
#endif

#if TEST
#define DEBUG 1
#define dz do { char dbuf[8192]; char * dptr = dbuf; int dbgl; (void)dbgl; \
    di(__LINE__) dc(':') spc
#define ds(s) dbgl = strlen(s); memcpy(dptr, s, dbgl); dptr += dbgl;
#define da(a) memcpy(dptr, a, sizeof a - 1); dptr += sizeof a - 1;
#define dc(c) *dptr++ = c;
#define spc *dptr++ = ' ';
#define dot *dptr++ = '.';
#define cmm *dptr++ = ','; spc
#define cln *dptr++ = ':'; spc
#define dnl *dptr++ = '\n';
#define du(u) dptr += sprintf(dptr, "%lu", (unsigned long)i);
#define di(i) dptr += sprintf(dptr, "%ld", (long)i);
#define dp(p) dptr += sprintf(dptr, "%p", p);
#define dx(x) dptr += sprintf(dptr, "%lx", (long)x);

#define df ds(__func__) dc('(')
#define dfc dc(')')
#define dss(s) da(#s) da(": \"") ds(s) dc('"')
#define dsi(i) da(#i) da(": ") di(i)
#define dsp(p) da(#p) da(": ") dp(p)

#define dw dnl write(2, dbuf, dptr - dbuf); } while (0)
#define dws spc write(2, dbuf, dptr - dbuf); } while (0)
#else

#define DEBUG 0
#define dz do {
#define ds(s)
#define da(a)
#define dc(c)
#define spc
#define dot
#define cmm
#define dnl
#define du(u)
#define di(i)
#define dx(x)

#define df
#define dfc
#define dsi(i)
#define dss(s)

#define dw } while (0)
#define dws } while (0)
#endif
#define dw0 } while (0)

#ifndef TEST_SMALL
#define TEST_SMALL 0
#endif

#if TEST_SMALL
static int S_page_size = 128;
#else
// 0 -> use page size. if 4096, ~250 items on 64-bit system, ~500 on 32-bit
static int S_page_size = 0;
#endif

#if 0
#include <stdio.h>
#include <ctype.h>
static
void hexdump(const void * addr, unsigned len)
{
    const unsigned char * s = (const unsigned char *)addr;
    while (len > 0) {
	unsigned l = len > 16? 16: len;
	unsigned i;
	printf("%p ", s);
	for (i = 0; i < l; i++)
	    printf("%02x ", s[i]);
	for (; i < 16; i++)
	    printf("   ");
	//printf(" ");
	for (i = 0; i < l; i++)
	    printf("%c", isprint(s[i])? s[i]: '.');
	printf("\n");
	s += l;
	len -= l;
    }
}
#endif

#if defined (__GNUC__) && __GNUC__ >= 5
#pragma GCC diagnostic push
// we're interested to see how struct resource_holder aligns...
#pragma GCC diagnostic warning "-Wpadded"
#endif
struct resource_holder
{
    struct resource_holder * prev; // circular, points to last in use
    struct resource_holder * next; // linked list to all allocated holders
    int items;
    int frees;
//    int sizetest; // w/ this, on 64-bit system, end_of_buffer is 8-aligned.
    char * end_of_buffer;
    struct {
	void (*func)(void * data);
	void * data;
    } item[];
};
#if defined (__GNUC__) && __GNUC__ >= 5
#pragma GCC diagnostic pop
#endif

/*
  Possible resource-holder-configuration (-> next, <-> both & `----^ prev):

  | first | <-> | second | <-> | third | <-> | fourth | -> NULL
      `----------------------------^
  In this case first & second are full (freed possible), third is half-full
  and fourth has been use, but no longer. only one "extra" is kept
  (note that e.g. release code probably supports more than one "extra").
 */

// private freed "marker" function
static void _rh_freed(void * data)
{
    (void)data;
    //dz df dss(data) dfc dw;
}

#if TEST_SMALL
#include <stdio.h>
static void _print_holders(struct resource_holder * curr)
{
    struct resource_holder * first = curr;
    int n = 0;
    while (curr) {
	n++;
#if 0
	int d = ((char *)first - (char *)curr) & 0xfffffff;
	printf("%4d  -%07x  %d  %d  %c  ", n, d, curr->items, curr->frees,
#else
	printf("%4d   %d   %d  %c  ", n, curr->items, curr->frees,
	       first->prev == curr? 'l': ' ');
#endif
	for (int i = 0; i < curr->items; i++)
	    printf("%c", (curr->item[i].func != _rh_freed)?
		    (int)(intptr_t)curr->item[i].data & 0x7f: '.');
	printf("\n");
	curr = curr->next;
    }
}
#endif

#define _munmap(addr) munmap(addr, S_page_size);

static struct resource_holder * _alloc_new(struct resource_holder * curr)
{
    struct resource_holder * next;
    if (S_page_size == 0)
	S_page_size = sysconf(_SC_PAGESIZE);

    if (curr && curr->next != NULL)
	next = curr->next;
    else {
	next = mmap(NULL, S_page_size, PROT_READ|PROT_WRITE,
		    MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
	next->next = NULL;
	if (curr)
	    curr->next = next;
    }
    next->prev = curr;
    next->items = 0;
    next->frees = 0;
    next->end_of_buffer = (char *)next + S_page_size
	- (offsetof(struct resource_holder, item[1]) -
	   offsetof(struct resource_holder, item[0]));
    return next;
}

// if any of the holders have free nodes (starting) from current,
// gc the most recent holder... (in test make one fully empty node
// in between (although that will be unmapped, so check for 1

#define eprintf(...) fprintf(stderr, __VA_ARGS__)
#if 0
#include <stdlib.h>
#define assert4(l, e, r, f) do { if (!(l e r)) { \
	eprintf("Assertion: " #l " [" #f "] " #e " " #r " [" #f "] failed.\n",\
		l, r); exit(1); }} while (0)
#endif
static
struct resource_holder * _may_gc_alloc_new(struct resource_holder * holder)
{
    struct resource_holder * curr = holder;
    while (curr->frees == 0 && curr != holder->prev)
	curr = curr->next;

    if (curr->frees == 0)
	return _alloc_new(holder->prev);

    int toi = 0, fromi = -1;
    while (toi < curr->items) {
	if (curr->item[toi].func == _rh_freed) {
	    fromi = toi + 1;
	    break;
	}
	toi++;
    }
    //assert4(fromi, >=, 0, %d);
    struct resource_holder * from = curr;
    do {
	while (fromi < from->items) {
	    if (from->item[fromi].func != _rh_freed)
		break;
	    fromi++;
	}
	if (fromi >= from->items) {
	    from = from->next;
	    fromi = 0;
	    continue;
	}
	curr->item[toi].func = from->item[fromi].func;
	curr->item[toi].data = from->item[fromi].data;
	fromi++;
	toi++;
	if (toi >= curr->items) {
	    curr->frees = 0;
	    curr = curr->next;
	    toi = 0;
	}
    } while (from);
    curr->items = toi;
    curr->frees = 0;
    from = curr->next;
    if (from) {
	from->items = from->frees = 0;
	struct resource_holder * tmp = from->next;
	from->next = NULL;
	while (tmp) {
	    from = tmp->next;
	    _munmap(tmp);
	    tmp = from;
	}
    }
    return curr;
}

struct resource_holder * resource_holder_create(void);
struct resource_holder * resource_holder_create(void)
{
    struct resource_holder * holder = _alloc_new(NULL);
    holder->prev = holder;
    return holder;
}

void resource_holder_release_all(struct resource_holder * holder);
void resource_holder_release_all(struct resource_holder * holder)
{
    struct resource_holder * curr = holder->prev; // prev means last here
    if (curr->next != NULL) {
	struct resource_holder * tmp = curr->next;
	while (tmp->next != NULL)
	    tmp = tmp->next;
	do {
	    struct resource_holder * prev = tmp->prev;
	    _munmap(tmp);
	    tmp = prev;
	} while (tmp != curr);
    }
    while (1) {
	for (int i = curr->items - 1; i >= 0; i--)
	    curr->item[i].func(curr->item[i].data);
	if (curr == holder)
	    break;
	struct resource_holder * prev = curr->prev;
	_munmap(curr);
	curr = prev;
    }
    holder->next = NULL;
    holder->prev = holder;
    holder->items = holder->frees = 0;
}

void resource_holder_destroy(struct resource_holder * holder);
void resource_holder_destroy(struct resource_holder * holder)
{
    resource_holder_release_all(holder);
    holder->prev = NULL;
    _munmap(holder);
}


void resource_holder_add(struct resource_holder * holder,
			 void (*func)(void * data), void * data);
void resource_holder_add(struct resource_holder * holder,
			 void (*func)(void * data), void * data)
{
    struct resource_holder * curr = holder->prev; // prev means last here
    if (curr->end_of_buffer - (char *)&curr->item[curr->items] < 0)
	holder->prev = curr = curr->next?
	    _alloc_new(curr): _may_gc_alloc_new(holder);
    int i = curr->items;
    curr->item[i].func = func;
    curr->item[i].data = data;
    curr->items++;
}

void resource_holder_pop(struct resource_holder * holder, const bool call,
			 void (**funcp)(void * data), void ** datap);
void resource_holder_pop(struct resource_holder * holder, const bool call,
			 void (**funcp)(void * data), void ** datap)
{
    struct resource_holder * curr = holder->prev; // prev means last here
    if (curr->items == 0) { // only applicable on top-level empty holder
	if (funcp) *funcp = NULL;
	if (datap) *datap = NULL;
	return;
    }
    int i = curr->items - 1;
    while (curr->item[i].func == _rh_freed) {
	curr->items--;
	curr->frees--;
	if (i == 0) {
	    if (curr == holder) {
		if (funcp) *funcp = NULL;
		if (datap) *datap = NULL;
		return;
	    }
	    if (curr->next) {
		_munmap(curr->next);
		curr->next = NULL;
	    }
	    // curr->items = curr->frees == 0; test, assert, etc.
	    curr = curr->prev;
	    holder->prev = curr;
	    i = curr->items - 1;
	}
    }
    if (funcp) *funcp = curr->item[i].func;
    if (datap) *datap = curr->item[i].data;
    if (call)
	curr->item[i].func(curr->item[i].data);
    if (i > 0 || curr == holder)
	curr->items = i;
    else {
	if (curr->next) {
	    _munmap(curr->next);
	    curr->next = NULL;
	}
	curr = curr->prev;
	holder->prev = curr;
    }
}

#define RHRF_UPTO 1
#define RHRF_ALLMATCHES 2
#define RHRF_MATCHFUNC 4
#define RHRF_MATCHDATA 8

bool resource_holder_release(struct resource_holder * holder,
			     void (*func)(void * data), void * data,
			     const int count, const unsigned flags);
bool resource_holder_release(struct resource_holder * holder,
			     void (*func)(void * data), void * data,
			     const int count, const unsigned flags)
{
    struct resource_holder * curr = holder->prev; // prev means last here
    const bool _fo = flags & RHRF_MATCHFUNC;
    const bool _do = flags & RHRF_MATCHDATA;
    bool rv = false;
    while (1) {
	for (int i = curr->items - 1; i >= 0; i--) {
	    const bool fm = _do || (curr->item[i].func == func);
	    const bool dm = _fo || (curr->item[i].data == data);
	    if (fm && dm) {
		if (count == 0)
		    goto zc;
		int c = count > 0? count: -count;
		while (1) {
		    if (count > 0)
			curr->item[i].func(curr->item[i].data);

		    if (flags & RHRF_UPTO) {
			curr->items--;
			if (curr->item[i].func == _rh_freed)
			    curr->frees--;
			if (curr->items == 0) {
			    if (curr->next != NULL) {
				_munmap(curr->next);
				curr->next = NULL;
			    }
			    holder->prev = curr->prev;
			}
		    }
		    else {
			if (curr->item[i].func != _rh_freed) {
			    curr->item[i].func = _rh_freed;
			    curr->item[i].data = (void*)0;
			    curr->frees++;
			}
		    }
		    c--;
		    if (c == 0)
			break;
		    if (i == 0) {
			if (curr == holder)
			    return true;
			curr = curr->prev;
			i = curr->items - 1;
		    }
		    else
			i--;
		}
	    zc:
		if (!(flags & RHRF_ALLMATCHES))
		    return true;
		rv = true;
	    }
	    else if (flags & RHRF_UPTO) {
		if (curr->item[i].func != _rh_freed)
		    curr->item[i].func(curr->item[i].data);
		else
		    curr->frees--;
		curr->items--;
		if (curr->items == 0) {
		    if (curr->next != NULL) {
			_munmap(curr->next);
			curr->next = NULL;
		    }
		    holder->prev = curr->prev;
		}
	    }
	}
	if (curr == holder)
	    break;
	curr = curr->prev;
    }
    return rv;
}

// test code, hacky ad-hoc code, mainly to provide interface for external
// test script.
#if TEST
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static void do_noting(void * data) // typo intentional, noone says h anyway...
{
    (void)data;
}

static int req_xchr(const char * s)
{
    if (strlen(s) > 1) {
	eprintf("'%s': not one character\n", s);
	return 0;
    }
    int chr = *s;
    if ((chr < '0' || chr > '9') && (chr < 'a' || chr > 'z')) {
	eprintf("'%c': character not in 0-9a-z\n", chr);
	return 0;
    }
    return chr;
}

static void * voidit(int i) {
    return (void *)((char *)0 + i);
}

static char * nstrtok(char * buf, char * p)
{
    //dz df dsp(buf) cmm dsp(p) dfc dw;
    //hexdump(buf, 16);
    if (p == NULL)
	return NULL;
    for (p--; p - buf >= 0 && *p; p--) {
#if 0
	printf("  %d", *p);
	printf("\n");
#endif
    }
    if (p - buf >= 0)
	*p = ' ';
    return strtok(NULL, " \n");
}

//int main(int argc, char ** argv)
int main(void)
{
    //printf("%ld\n", offsetof(struct resource_holder, item[0]));
    //printf("%ld\n", offsetof(struct resource_holder, item[1]));

    struct resource_holder * holder = resource_holder_create();

    //assert4(argc, >, 4, %d);

    while (!feof(stdin)) {
	char buf[1024];
	//memset(buf, 0, 32);
	printf(">>> ");
	fflush(stdout);
	if (fgets(buf, sizeof buf, stdin) == NULL)
	    break;
	for (int r = 0; r < 2; r++) {
	    char * p = strtok(buf, " \n");
	    while (1) {
		if (p == NULL)
		    break;
		if (strlen(p) > 1) {
		    eprintf("'%s': unknown command (too long)\n", p);
		    goto newline;
		}
		int chr, cnt;
		//eprintf("cmd: %s %d\n", p, *p);
		switch (*p) {
		case 'a':
		    //hexdump(buf, 32);
		    p = nstrtok(buf, p);
		    if (p == NULL) {
			eprintf("(a)dd requires chr and possibly count\n");
			goto newline;
		    }
		    chr = req_xchr(p);
		    if (chr == 0)
			goto newline;
		    p = nstrtok(buf, p);
		    cnt = 1;
		    if (p && isdigit(*p) && atoi(p) > 0) {
			cnt = atoi(p);
			p = nstrtok(buf, p);
		    }
		    //dz dsi(chr) cmm dsi(cnt) dw;
		    if (r)
			for (int i = 0; i < cnt; i++)
			    resource_holder_add(holder, do_noting, voidit(chr));
		    break;
		case 'r':
		    p = nstrtok(buf, p);
		    if (p == NULL) {
			eprintf(
			    "(r)el requires chr, count and possibly flags\n");
			goto newline;
		    }
		    chr = req_xchr(p);
		    if (chr == 0)
			goto newline;
		    p = nstrtok(buf, p);
		    if (p == NULL) {
			eprintf("(r)el count undefined\n");
			goto newline;
		    }
		    cnt = atoi(p);
		    p = nstrtok(buf, p);
		    int flgs = 0;
		    if (p && isdigit(*p)) {
			flgs = atoi(p);
			p = nstrtok(buf, p);
		    }
		    //dz dsi(chr) cmm dsi(cnt) cmm dsi(flgs) dw;
		    if (r)
			resource_holder_release(holder, do_noting, voidit(chr),
						cnt, flgs);
		    break;
		case 'p':
		    if (r)
			resource_holder_pop(holder, 1, NULL, NULL);
		    p = nstrtok(buf, p);
		    break;
		case 'q':
		    goto out;
		default:
		    eprintf("'%c': unknown command\n", *p);
		    goto newline;
		}
	    }
	}
	_print_holders(holder);
    newline:;
    }
out:
    resource_holder_destroy(holder);
    return 0;
}
#endif
