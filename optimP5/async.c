#include "scheduler.h"
#include <aio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h> 
#include <stdlib.h>
#include <stdio.h>
ssize_t read_wrap(int fd, void * buf, size_t count)
{
	struct aiocb cb;
	memset(&cb, 0, sizeof(struct aiocb));
	//set the offset to the 0 relative to current file position.
	if(fd!=0)cb.aio_offset = lseek(fd,0,SEEK_CUR);
	cb.aio_nbytes = count;
	cb.aio_fildes = fd;
	cb.aio_buf = buf;
	if(aio_read(&cb)==-1)
	{
		return -1;
	}
	while(aio_error(&cb)==EINPROGRESS)
	{
		yield();
	}
	/*	reposition the offset of the file associated with the file descriptor "fd" to the "count" offset
	 *	according to the current file position.
	 */
	if(fd!=0)lseek(fd,count,SEEK_CUR);
	return aio_return(&cb);
}
