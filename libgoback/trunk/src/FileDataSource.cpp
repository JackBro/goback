#include "FileDataSource.h"

	//constructor
	FileDataSource::FileDataSource() {
		_fd=-1;
		_writable=false;
	}

	/*
	 * Open the file in read-write.
	 */
	bool FileDataSource::requestWrite() {
		if (_writable) return true;

		int fdw = ::open(_filename, O_RDWR);

		if (fdw == -1) return false;
		_writable=true;
		::close(_fd);
		_fd=fdw;
		return true;
	}

	/*
	 * Open the file in read-write, and let insert.
	 */
	bool FileDataSource::requestInsert() {
		return requestWrite();
	}

	bool FileDataSource::requestRemove() {
		return requestWrite();
	}

	/*
	 * Flush the writes.
	 */
	bool FileDataSource::flushWrites() {
		if (fsync(_fd) == 0) return true;
		return false;
	}

	/*
	 * Open file in read-only
	 */
	bool FileDataSource::open(const char * filename) {
		//open file
		_fd = ::open(filename, O_RDONLY);

		//check errors
		if (_fd == -1 ) return false;
		
		_filename = strdup(filename);

		return true;
	}

	/*
	 * Close the file.
	 */
	bool FileDataSource::close() {
		if (::close(_fd) == 0) return true;
		_fd=-1;
		return false;
	}

	/* 
	 * Put at buff, size bytes, from offset.
	 * This function doesn't modify the current seek.
	 * If offset is null, read from current seek, and modify it.
	 */
	int FileDataSource::readBytes(char * buff, int size, unsigned int offset) {
		int result=0;

		//check file opened
		if (_fd == -1 ) return 0;

		if (offset != -1 ) {
			int seek = lseek(_fd, 0, SEEK_CUR);
			//seek the offset
			if (lseek(_fd, offset, SEEK_SET) == offset)
				result=read(_fd, buff, size);
			//restore seek
			lseek(_fd, seek, SEEK_SET);
		} else {
			result=read(_fd, buff, size);
		}
		return result;
	}

	/*
	 * Replace at offset, size bytes, with buff.
	 * This fucntion doesn't modify the current seek.
	 * If offset is null, replace at current seek and modify it.
	 */
	int FileDataSource::replaceBytes(const char * buff, int size, unsigned int offset) {
		int result=0;

		//check file opened and writable
		if (_fd == -1 || !_writable ) return 0;

		if (offset != -1) {
			//seek the offset
			if (lseek(_fd, offset, SEEK_SET) == offset)
				result=write(_fd, buff, size);
		} else {
			result=write(_fd, buff, size);
		}
		return result;
	}

	/*
	 * Insert size bytes from buff to the offset.
	 * This is for small sizes. ~mbytes.
	 */
	int FileDataSource::insertBytes(const char * buff, int size, unsigned int offset) {
		//check file opened and writable
		if (_fd == -1 || !_writable ) return 0;

		int fdtmp = ::open(_filename, O_RDONLY);

		int buffsize = (size<4096)?4096:size;
		void * bufftmp = malloc(buffsize);
		
		//set the seek and the end of file
		int filesize=lseek(_fd, 0, SEEK_END);
		lseek(_fd, -size, SEEK_END);

		//add a chunk of size bytes at the end of file
		int bytes = read(_fd, bufftmp, size);
		write(_fd, bufftmp, bytes);

		//move the content
		int i=0;
		while ((filesize-offset-buffsize*i-size) > buffsize){
			lseek(_fd, filesize-(buffsize*(i+1))-size, SEEK_SET);
			bytes=read(_fd, bufftmp, buffsize);
			lseek(_fd, filesize-(buffsize*(i+1)), SEEK_SET);
			write(_fd, bufftmp, bytes);
			i++;
		}

		//there are more bytes to move?
		bytes=filesize-offset-buffsize*i-size;
		if (bytes > 0) {
			lseek(_fd, filesize-buffsize*i-size-bytes, SEEK_SET);
			read(_fd, bufftmp, bytes);
			lseek(_fd, filesize-buffsize*i-bytes, SEEK_SET);
			write(_fd, bufftmp, bytes);
		}
		//clean
		free(bufftmp);
		::close(fdtmp);

		//insert the bytes
		lseek(_fd, offset, SEEK_SET);
		return write(_fd, buff, size);
	}

	/*
	 * Remove size bytes at offset.
	 * This is for small sizes. ~mbytes.
	 */
	int FileDataSource::removeBytes(int size, unsigned int offset) {
		//check file opened and writable
		if (_fd == -1 || !_writable ) return 0;

		int buffsize = (size<4096)?4096:size;
		void * bufftmp = malloc(buffsize);
		int fdtmp = ::open(_filename, O_RDONLY);
		int filesize = lseek(_fd,0,SEEK_END);
		lseek(_fd, offset, SEEK_SET);
		lseek(fdtmp, offset+size, SEEK_SET);

		int bytes;
		while ((bytes=read(fdtmp, bufftmp, buffsize)) > 0) {
			write(_fd, bufftmp, bytes);
		}

		free(bufftmp);
		::close(fdtmp);

		ftruncate(_fd, filesize-size);
	
		return size;
	}