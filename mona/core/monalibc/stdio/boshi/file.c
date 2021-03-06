#include "mona_file.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "private_vars.h"
#include "filelist.h"

int __mlibc_stdio_parse_mode(const char *s)
{
	int r = 0;
	if( s == NULL ) return 0;
	switch(*s++)
	{	case 'r': r |= F_READ; break;
		case 'w': r |= F_WRITE|F_CREATE|F_TRUNC; break;
		case 'a': r |= F_APPEND|F_WRITE|F_CREATE; break;
	default: return 0; } do { switch(*s)
	{	case '+': r |= F_READ|F_WRITE; break;
		case 'b': r |= F_BINARY; break;
		default: break; }} while( *s++ != '\0' );
	return r;
}

struct cache_operators_ fullbuf_operators;
struct cache_operators_ linebuf_operators;
extern struct __mlibc_file_operators_ __file_ops;
void* opening_files = NULL;

FILE* __mlibc_fopen(const char *path, const char *mode)
{
	FILE* file = NULL;

	file = (FILE*)malloc(sizeof(FILE));
	if( file == NULL )
	{
		errno = ENOMEM;
		return NULL;
	}
	memset(file, 0, sizeof(FILE));

	file->mode = __mlibc_stdio_parse_mode(mode);
	if( file->mode == 0 )
	{
		errno = EINVAL;
		free(file);
		return NULL;
	}

	file->fileops = __file_ops;

	file->file = file->fileops.open(file, path, file->mode);
	if( !file->fileops.is_valid_file(file, file->file) )
	{
		errno = EUNKNOWN;
		free(file);
		return NULL;
	}

	file->end = file->fileops.seek(file, 0, SEEK_END);
	file->fileops.seek(file, 0 , SEEK_SET);

	file->offset = 0;
	file->cachers = fullbuf_operators;
	file->ungetcbuf = EOF;

	__monalibc_filelist_add(openeing_files, file);

	return file;
}

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

int __mlibc_fseek(FILE *f, long offset, int whence)
{
	switch( whence )
	{
		case SEEK_SET: f->offset = offset; break;
		case SEEK_CUR: f->offset += offset; break;
		case SEEK_END: f->offset = f->end + offset;
			if( f->offset < 0 ) f->offset = 0;
			if( f->offset < f->cache.offset
				+ (f->cache.current-f->cache.base) )
					f->offset = f->cache.offset
					+ (f->cache.current-f->cache.base);
			break;
		defalut: errno = EINVAL; return -1;
	}

	f->mode &= ~F_EOF;
	return 0;
}

int __mlibc_fflush(FILE *f)
{
	f->cachers.flush(f);
	return 0;
}

void __mlibc_fclose(FILE *f)
{
	__mlibc_fflush(f);

	f->fileops.close(f, f->file);
	if( f->mode & F_BUFSELF) free(f->cache.base);
	__monalibc_filelist_remove_by_element(openeing_files, file);
	free(f);
	return;
}

int __mlibc_ungetc(int c, FILE *f)
{
	if( f->ungetcbuf != EOF ) f->offset--;
	f->ungetcbuf = c;
	return c;
}

void __mlibc_init_cache(FILE *file, char *buf, size_t size, int mode)
{
	file->cache.base = buf;
	if( file->cache.base == NULL )
	{
		__mlibc_init_cache(file, malloc(size), size, mode|F_BUFSELF);
		return;
	}
	file->cache.current = file->cache.base;
	file->cache.end = file->cache.base + (size - 1);
	file->cache.size = size;
	file->cache.offset = 0;

	file->mode |= mode|F_BUFALCD;

	return;
}

int __mlibc_is_out_of_cache(FILE *f)
{
	int x = 0, p1 = 0, p2 = 0;
	x = f->offset;
	p1 = f->cache.offset;
//	p2 = f->cache.offset + f->cache.size;
	p2 = f->cache.offset + (f->cache.end-f->cache.base);
	if( x < p1 || p2 <= x ) return 1;
	return 0;
}

int __mlibc_cache_flush(FILE *f)
{
	int result = 0;
	if( f->mode & F_BUFDIRTY )
	{
		f->fileops.seek(f, f->cache.offset, SEEK_SET);
		/* NEED TO CHECK THE ERROR */
		result = f->fileops.write(f, f->cache.base, f->cache.current-f->cache.base);
	}
	f->cache.current = f->cache.base;
	return result;
}

int __mlibc_is_eof(FILE *f)
{
//	puts(__func__);
//	_logprintf("current = %x, end = %x\n", f->cache.current, f->cache.end);
	if( !(f->mode & F_BUFEOF) ) return 0;
	if( f->cache.current >= f->cache.end ) return 1;
	return 0;
}

void __mlibc_move_cache_current_ptr(FILE* f)
{
	if( __mlibc_is_out_of_cache(f) ) return;
	f->cache.current = f->cache.base + (f->offset-f->cache.offset);
}

/** fully buffer */

void __mlibc_init_cache_fully(FILE *f, void *p, size_t size)
{
	__mlibc_init_cache(f, p, size, F_FULLBUF);
}

int __mlibc_cache_load_fully(FILE *f)
{
//	_logprintf("%s\n", __func__);
	size_t readsize = 0;

	if( !(f->mode & F_READ) )
	{
		f->cache.current = f->cache.base;
		f->cache.end = f->cache.end;
		f->cache.offset = f->offset;
		return 0;
	}

//	_logprintf("\tseek : offset = %x\n", f->offset);
	int result = f->fileops.seek(f, f->offset, SEEK_SET);
//	_logprintf("\t     : result = %x\n", result);
//	_logprintf("\tread : size = %d\n", f->cache.size);
	readsize = f->fileops.read(f, f->cache.base, f->cache.size);
	if( readsize == -1 )
	{
		return -1;
	}
//	_logprintf("\t     : readsize = %d\n", readsize);
	f->cache.current = f->cache.base;
	f->cache.end = f->cache.base + readsize;
	f->mode &= ~F_BUFEOF;
	f->cache.offset = f->offset;
//	_logprintf("\tcur = %x, end = %x, off = %x\n", f->cache.current, f->cache.end, f->cache.offset);
	if( readsize < f->cache.size )
	{
		f->mode |= F_BUFEOF;
	}
	return readsize;
}

struct cache_operators_ fullbuf_operators =
{
	(int(*)(void*,void*,size_t))__mlibc_init_cache_fully,
	(int(*)(void*))__mlibc_is_out_of_cache,
	(int(*)(void*))__mlibc_is_eof,
	(int(*)(void*))__mlibc_cache_flush,
	(int(*)(void*))__mlibc_cache_load_fully,
	(void(*)(void*))__mlibc_move_cache_current_ptr
};

/** line buffer */

void __mlibc_init_cache_line(FILE *f, void *p, size_t size)
{
	__mlibc_init_cache(f, p, size, F_LINEBUF);
}

int __mlibc_get_line_iter(FILE *f, char *buf, size_t size, size_t rsize)
{
	if( size <= 0 ) {
		return rsize;
	} else { switch(f->fileops.read(f, buf, 1)) {
		case 0: /* End of file */
			f->mode |= F_BUFEOF; return rsize;
		case -1: /* Error */
			return rsize;
	}}
	if( *buf == '\n') return rsize;
	return __mlibc_get_line_iter(f, buf+1, size-1, rsize+1);
}

int __mlibc_get_line(FILE *f)
{
//	puts(__func__);
	size_t readsize = 0;

	if( !(f->mode & F_READ) )
	{
		f->cache.current = f->cache.base;
		f->cache.end = f->cache.end;
		f->cache.offset = f->offset;
		return 0;
	}

//	printf("offset = %d\n", f->offset);
	int result = f->fileops.seek(f, f->offset, SEEK_SET);
	f->mode &= ~F_BUFEOF;
	readsize = __mlibc_get_line_iter(f, f->cache.base, f->cache.size, 0);
//	printf("readsize = %d\n", readsize);
	f->cache.current = f->cache.base;
	f->cache.end = f->cache.base + readsize;
	f->cache.offset = f->offset;
	return readsize;
}

struct cache_operators_ linebuf_operators =
{
	(int(*)(void*,void*,size_t))__mlibc_init_cache_line,
	(int(*)(void*))__mlibc_is_out_of_cache,
	(int(*)(void*))__mlibc_is_eof,
	(int(*)(void*))__mlibc_cache_flush,
	(int(*)(void*))__mlibc_get_line,
	(void(*)(void*))__mlibc_move_cache_current_ptr
};

