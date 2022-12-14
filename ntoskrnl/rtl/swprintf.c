/* $Id: swprintf.c 13311 2005-01-26 13:58:37Z ion $
 *
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS kernel
 * FILE:            ntoskrnl/rtl/swprintf.c
 * PURPOSE:         Unicode sprintf functions
 * 
 * PROGRAMMERS:     David Welch
 *                  Eric Kohl
 */

/*
 * TODO:
 *   - Verify the implementation of '%Z'.
 */

/*
 *  linux/lib/vsprintf.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

/* vsprintf.c -- Lars Wirzenius & Linus Torvalds. */
/*
 * Wirzenius wrote this portably, Torvalds fucked it up :-)
 */

#include <ntoskrnl.h>
#include <internal/ctype.h>
#define NDEBUG
#include <internal/debug.h>


#if defined(__GNUC__)
typedef          long long SWPRINT_INT64;
typedef unsigned long long SWPRINT_UINT64;
#else
typedef          __int64   SWPRINT_INT64;
typedef unsigned __int64   SWPRINT_UINT64;
#endif


#define ZEROPAD	1		/* pad with zero */
#define SIGN	2		/* unsigned/signed long */
#define PLUS	4		/* show plus */
#define SPACE	8		/* space if plus */
#define LEFT	16		/* left justified */
#define SPECIAL	32		/* 0x */
#define LARGE	64		/* use 'ABCDEF' instead of 'abcdef' */


#if defined(__GNUC__)

#define do_div(n,base) ({ \
int __res; \
__res = ((SWPRINT_UINT64) n) % (unsigned) base; \
n = ((SWPRINT_UINT64) n) / (unsigned) base; \
__res; })

#elif defined(_MSC_VER)

static __inline int do_foo_div(SWPRINT_INT64* n, int base)
{
	int __res = (int)(((SWPRINT_UINT64) *n) % (unsigned) base);
	*n        = (int)(((SWPRINT_UINT64) *n) / (unsigned) base);
	return __res;
}
#define do_div(n,base) do_foo_div(&n,base)

#else
#error Unknown compiler for this special compiler trickery
#endif



static int skip_atoi(const wchar_t **s)
{
	int i=0;

	while (iswdigit(**s))
		i = i*10 + *((*s)++) - L'0';
	return i;
}


static wchar_t *
number(wchar_t * buf, wchar_t * end, SWPRINT_INT64 num, int base, int size, int precision, int type)
{
	wchar_t c,sign, tmp[66];
	const wchar_t *digits;
	const wchar_t *small_digits = L"0123456789abcdefghijklmnopqrstuvwxyz";
	const wchar_t *large_digits = L"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	int i;

	digits = (type & LARGE) ? large_digits : small_digits;
	if (type & LEFT)
		type &= ~ZEROPAD;
	if (base < 2 || base > 36)
		return 0;
	c = (type & ZEROPAD) ? L'0' : L' ';
	sign = 0;
	if (type & SIGN) {
		if (num < 0) {
			sign = L'-';
			num = -num;
			size--;
		} else if (type & PLUS) {
			sign = L'+';
			size--;
		} else if (type & SPACE) {
			sign = ' ';
			size--;
		}
	}
	if (type & SPECIAL) {
		if (base == 16)
			size -= 2;
		else if (base == 8)
			size--;
	}
	i = 0;
	if (num == 0)
		tmp[i++] = L'0';
	else while (num != 0)
		tmp[i++] = digits[do_div(num,base)];
	if (i > precision)
		precision = i;
	size -= precision;
	if (!(type&(ZEROPAD+LEFT))) {
		while(size-->0) {
			if (buf <= end)
				*buf = L' ';
			++buf;
		}
	}
	if (sign) {
		if (buf <= end)
			*buf = sign;
		++buf;
	}
	if (type & SPECIAL) {
		if (base==8) {
			if (buf <= end)
				*buf = L'0';
			++buf;
		} else if (base==16) {
			if (buf <= end)
				*buf = L'0';
			++buf;
			if (buf <= end)
				*buf = digits[33];
			++buf;
		}
	}
	if (!(type & LEFT)) {
		while (size-- > 0) {
			if (buf <= end)
				*buf = c;
			++buf;
		}
	}
	while (i < precision--) {
		if (buf <= end)
			*buf = L'0';
		++buf;
	}
	while (i-- > 0) {
		if (buf <= end)
			*buf = tmp[i];
		++buf;
	}
	while (size-- > 0) {
		if (buf <= end)
			*buf = L' ';
		++buf;
	}
	return buf;
}

static wchar_t*
string(wchar_t* buf, wchar_t* end, const char* s, int len, int field_width, int precision, int flags)
{
	int i;
	if (s == NULL)
	{
		s = "<NULL>";
		len = 6;
	}
	else
	{
		if (len == -1)
		{
			len = 0;
			while ((unsigned int)len < (unsigned int)precision && s[len])
				len++;
		}
		else
		{
			if ((unsigned int)len > (unsigned int)precision)
				len = precision;
		}
	}
	if (!(flags & LEFT))
		while (len < field_width--)
		{
			if (buf <= end)
				*buf = L' ';
			++buf;
		}
	for (i = 0; i < len; ++i)
	{
		if (buf <= end)
			*buf = *s++;
		++buf;
	}
	while (len < field_width--)
	{
		if (buf <= end)
			*buf = L' ';
		++buf;
	}
	return buf;
}

static wchar_t* 
stringw(wchar_t* buf, wchar_t* end, const wchar_t* sw, int len, int field_width, int precision, int flags)
{
	int i;
	if (sw == NULL)
	{
		sw = L"<NULL>";
		len = 6;
	}
	else
	{
		if (len == -1)
		{
			len = 0;
			while ((unsigned int)len < (unsigned int)precision && sw[len])
				len++;
		}
		else
		{
			if ((unsigned int)len > (unsigned int)precision)
				len = precision;
		}
	}
	if (!(flags & LEFT))
		while (len < field_width--)
		{
			if (buf <= end)
				*buf = L' ';
			buf++;
		}
	for (i = 0; i < len; ++i)
	{
		if (buf <= end)
			*buf = *sw++;
		buf++;
	}
	while (len < field_width--)
	{
		if (buf <= end)
			*buf = L' ';
		buf++;
	}
	return buf;
}

int _vsnwprintf(wchar_t *buf, size_t cnt, const wchar_t *fmt, va_list args)
{
	int len;
	SWPRINT_UINT64 num;
	int base;
	wchar_t * str, * end;
	const char *s;
	const wchar_t *sw;

	int flags;		/* flags to number() */

	int field_width;	/* width of output field */
	int precision;		/* min. # of digits for integers; max
				   number of chars for from string */
	int qualifier;		/* 'h', 'l', 'L', 'w' or 'I' for integer fields */

	str = buf;
	end = buf + cnt - 1;
	if (end < buf - 1) {
		end = ((void *) -1);
		cnt = end - buf + 1;
	}

	for ( ; *fmt ; ++fmt) {
		if (*fmt != L'%') {
			if (str <= end)
				*str = *fmt;
			++str;
			continue;
		}

		/* process flags */
		flags = 0;
		repeat:
			++fmt;		/* this also skips first '%' */
			switch (*fmt) {
				case L'-': flags |= LEFT; goto repeat;
				case L'+': flags |= PLUS; goto repeat;
				case L' ': flags |= SPACE; goto repeat;
				case L'#': flags |= SPECIAL; goto repeat;
				case L'0': flags |= ZEROPAD; goto repeat;
			}

		/* get field width */
		field_width = -1;
		if (iswdigit(*fmt))
			field_width = skip_atoi(&fmt);
		else if (*fmt == L'*') {
			++fmt;
			/* it's the next argument */
			field_width = va_arg(args, int);
			if (field_width < 0) {
				field_width = -field_width;
				flags |= LEFT;
			}
		}

		/* get the precision */
		precision = -1;
		if (*fmt == L'.') {
			++fmt;
			if (iswdigit(*fmt))
				precision = skip_atoi(&fmt);
			else if (*fmt == L'*') {
				++fmt;
				/* it's the next argument */
				precision = va_arg(args, int);
			}
			if (precision < 0)
				precision = 0;
		}

		/* get the conversion qualifier */
		qualifier = -1;
		if (*fmt == L'h' || *fmt == L'l' || *fmt == L'L' || *fmt == L'w') {
			qualifier = *fmt;
			++fmt;
		} else if (*fmt == L'I' && *(fmt+1) == L'6' && *(fmt+2) == L'4') {
			qualifier = *fmt;
			fmt += 3;
		}

		/* default base */
		base = 10;

		switch (*fmt) {
		case L'c':
			if (!(flags & LEFT))
				while (--field_width > 0) {
					if (str <= end)
						*str = L' ';
					++str;
				}
			if (qualifier == 'h') {
				if (str <= end)
					*str = (wchar_t) va_arg(args, int);
				++str;
			} else {
				if (str <= end)
					*str = (wchar_t) va_arg(args, int);
				++str;
			}
			while (--field_width > 0) {
				if (str <= end)
					*str = L' ';
				++str;
			}
			continue;

		case L'C':
			if (!(flags & LEFT))
				while (--field_width > 0) {
					if (str <= end)
						*str = L' ';
					++str;
				}
			if (qualifier == 'l' || qualifier == 'w') {
				if (str <= end)
					*str = (wchar_t) va_arg(args, int);
				++str;
			} else {
				if (str <= end)
					*str = (wchar_t) va_arg(args, int);
				++str;
			}
			while (--field_width > 0) {
				if (str <= end)
					*str = L' ';
				++str;
			}
			continue;

		case L's':
			if (qualifier == 'h') {
				/* print ascii string */
				s = va_arg(args, char *);
				str = string(str, end, s, -1,  field_width, precision, flags);
			} else {
				/* print unicode string */
				sw = va_arg(args, wchar_t *);
				str = stringw(str, end, sw, -1, field_width, precision, flags);
			}
			continue;

		case L'S':
			if (qualifier == 'l' || qualifier == 'w') {
				/* print unicode string */
				sw = va_arg(args, wchar_t *);
				str = stringw(str, end, sw, -1, field_width, precision, flags);
			} else {
				/* print ascii string */
				s = va_arg(args, char *);
				str = string(str, end, s, -1,  field_width, precision, flags);
			}
			continue;

		case L'Z':
			if (qualifier == 'h') {
				/* print counted ascii string */
				PANSI_STRING pus = va_arg(args, PANSI_STRING);
				if ((pus == NULL) || (pus->Buffer == NULL)) {
					s = NULL;
					len = -1;
				} else {
					s = pus->Buffer;
					len = pus->Length;
				}
				str = string(str, end, s, len,  field_width, precision, flags);
			} else {
				/* print counted unicode string */
				PUNICODE_STRING pus = va_arg(args, PUNICODE_STRING);
				if ((pus == NULL) || (pus->Buffer == NULL)) {
					sw = NULL;
					len = -1;
				} else {
					sw = pus->Buffer;
					len = pus->Length / sizeof(WCHAR);
				}
				str = stringw(str, end, sw, len,  field_width, precision, flags);
			}
			continue;

		case L'p':
			if (field_width == -1) {
				field_width = 2*sizeof(void *);
				flags |= ZEROPAD;
			}
			str = number(str, end,
				(unsigned long) va_arg(args, void *), 16,
				field_width, precision, flags);
			continue;

		case L'n':
			/* FIXME: What does C99 say about the overflow case here? */
			if (qualifier == 'l') {
				long * ip = va_arg(args, long *);
				*ip = (str - buf);
			} else {
				int * ip = va_arg(args, int *);
				*ip = (str - buf);
			}
			continue;

		/* integer number formats - set up the flags and "break" */
		case L'o':
			base = 8;
			break;

		case L'b':
			base = 2;
			break;

		case L'X':
			flags |= LARGE;
		case L'x':
			base = 16;
			break;

		case L'd':
		case L'i':
			flags |= SIGN;
		case L'u':
			break;

		default:
			if (*fmt != L'%') {
				if (str <= end)
					*str = L'%';
				++str;
			}
			if (*fmt) {
				if (str <= end)
					*str = *fmt;
				++str;
			} else
				--fmt;
			continue;
		}

		if (qualifier == L'I')
			num = va_arg(args, SWPRINT_UINT64);
		else if (qualifier == L'l') {
			if (flags & SIGN)
				num = va_arg(args, long);
			else
				num = va_arg(args, unsigned long);
		}
		else if (qualifier == L'h') {
			if (flags & SIGN)
				num = va_arg(args, int);
			else
				num = va_arg(args, unsigned int);
		}
		else {
			if (flags & SIGN)
				num = va_arg(args, int);
			else
				num = va_arg(args, unsigned int);
		}
		str = number(str, end, num, base, field_width, precision, flags);
	}
	if (str <= end)
		*str = L'\0';
	else if (cnt > 0)
		/* don't write out a null byte if the buf size is zero */
		*end = L'\0';
	return str-buf;
}


/*
 * @implemented
 */
int swprintf(wchar_t *buf, const wchar_t *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	i=_vsnwprintf(buf,INT_MAX,fmt,args);
	va_end(args);
	return i;
}


/*
 * @implemented
 */
int _snwprintf(wchar_t *buf, size_t cnt, const wchar_t *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	i=_vsnwprintf(buf,cnt,fmt,args);
	va_end(args);
	return i;
}


int vswprintf(wchar_t *buf, const wchar_t *fmt, va_list args)
{
	return _vsnwprintf(buf,INT_MAX,fmt,args);
}

/* EOF */
