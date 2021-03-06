/*
Copyright (c) 2008 Shotaro Tsuji

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "mona_file.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "private_vars.h"
#include "filelist.h"

/** common functions */
void __mlibc_move_cache_current_ptr(FILE* f);

size_t __mlibc_fread_iter(char* p, size_t size, size_t readsize, FILE* f)
{
//	puts(__func__);
	char *q = NULL;

	if( size <= 0 ){ return readsize; }
	if( f->mode & F_EOF ){ return readsize; }
	if( f->cachers.is_out_of_cache(f) )
	{
		if( f->cachers.flush(f) < 0 ||
				f->cachers.load(f) < 0 )
		{
			f->mode |= F_ERROR;
			return readsize;
		}
	}
	if( f->cachers.is_eof(f) ){ f->mode |= F_EOF; return readsize; }
	if( f->ungetcbuf == EOF )
	{
		q = f->cache.current++;
		*p = *q;
	}
	else
	{
		*p = (char)f->ungetcbuf;
		f->cache.current++;
	}
	f->offset = f->offset + 1;
	return __mlibc_fread_iter(p+1, size-1, readsize+1, f);
}

size_t __mlibc_fread(void *buf, size_t size, size_t nmemb, FILE *f)
{
//	_logprintf("%s\n", __func__);
	if( !(f->mode & F_READ) )
	{
		f->mode |= F_ERROR;
		return (size_t)0;
	}

	if( !(f->mode & F_BUFALCD) )
	{
		f->cachers.init(f, NULL, BUFSIZ);
		if( f->cachers.load(f) < 0 )
		{
			f->mode |= F_ERROR;
			return (size_t)0;
		}
	}

	__mlibc_move_cache_current_ptr(f);
	return __mlibc_fread_iter((char*)buf, size*nmemb, 0, f);
}

size_t __mlibc_fwrite_iter(char* p, size_t size, size_t writtensize, FILE* f)
{
//	puts(__func__);
	char *q = NULL;
	if( size <= 0 ) return writtensize;
	if( f->cachers.is_out_of_cache(f) )
	{
		if( f->cachers.flush(f) < 0 ||
				f->cachers.load(f) < 0 )
		{
			f->mode |= F_ERROR;
			return writtensize;
		}
	}
	if( f->cachers.is_eof(f) ){ f->mode |= F_EOF; return writtensize; }
	q = f->cache.current++;
	f->cache.end = f->cache.current;
	*q = *p;
	f->offset = f->offset + 1;
	f->mode |= F_BUFDIRTY;
	return __mlibc_fwrite_iter(p+1, size-1, writtensize+1, f);
}

size_t __mlibc_fwrite(void *buf, size_t size, size_t nmemb, FILE *f)
{
	if( !(f->mode & F_WRITE) )
	{
		f->mode |= F_ERROR;
		return (size_t)0;
	}

	if( !(f->mode & F_BUFALCD) )
	{
		f->cachers.init(f, NULL, BUFSIZ);
		if( f->cachers.load(f) < 0 )
		{
			return 0;
		}
	}

	if( f->mode & F_APPEND ) __mlibc_fseek(f, 0, SEEK_END);

	__mlibc_move_cache_current_ptr(f);

	return __mlibc_fwrite_iter((char*)buf, size*nmemb, 0, f);
}

