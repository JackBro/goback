#include "DataSource.h"

class FileDataSource : public DataSource {
public:
	FileDataSource();
	bool requestWrite();
	bool requestInsert();
	bool requestRemove();
	bool flushWrites();
	bool open(const char * filename);
	bool close();
	int readBytes(char * buff, int size, unsigned int offset = -1);
	int replaceBytes(const char * buff, int size, unsigned int offset = -1);
	int insertBytes(const char * buff, int size, unsigned int offset);
	int removeBytes(int size, unsigned int offset);

private:
	int _fd;
	bool _writable;
	char * _filename;
};
