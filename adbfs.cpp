#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <queue>
#include <vector>
#include <map>

using namespace std;

struct fileCache{
    time_t timestamp;
    queue<string> statOutput;
};

void string_replacer(string&,const string,string);
queue<string> adb_push(string, string);
queue<string> adb_pull(string, string);
queue<string> adb_shell(string);
queue<string> shell(string);
queue<string> exec_command(string);
vector<string> make_array(string);
map<string,fileCache> fileData;

vector<string> make_array(string data){
    vector<string> result;
    string delimiters = " ";
    string::size_type lastPos = data.find_first_not_of(delimiters, 0);
    string::size_type pos     = data.find_first_of(delimiters, lastPos);

    while (string::npos != pos || string::npos != lastPos)
    {
        result.push_back(data.substr(lastPos, pos - lastPos));
        lastPos = data.find_first_not_of(delimiters, pos);
        pos = data.find_first_of(delimiters, lastPos);
    }
    return result;
}

queue<string> shell(string command)
{
    string actual_command;
    string_replacer(command,"\\","\\\\");
    string_replacer(command,"'","\\'");
    string_replacer(command,"`","\\`");
    actual_command.append(command);

    return exec_command(actual_command);
}

queue<string> adb_shell(string command)
{
    string actual_command;
    string_replacer(command,"\\","\\\\");
    string_replacer(command,"'","\\'");
    string_replacer(command,"`","\\`");
    actual_command = "adb shell ";
    actual_command.append(command);
    //actual_command.append("'");

    return exec_command(actual_command);
}

queue<string> adb_pull(string remote_source, string local_destination)
{
    string actual_command;
    string_replacer(remote_source,"\\","\\\\");
    string_replacer(remote_source,"'","\\'");
    string_replacer(remote_source,"`","\\`");
    string_replacer(local_destination,"\\","\\\\");
    string_replacer(local_destination,"'","\\'");
    string_replacer(local_destination,"`","\\`");
    actual_command = "adb pull '";
    actual_command.append(remote_source);
    actual_command.append("' '");
    actual_command.append(local_destination);
    actual_command.append("'");

    return exec_command(actual_command);
}

queue<string> adb_push(string local_source, string remote_destination)
{
    string actual_command;
    string_replacer(remote_destination,"\\","\\\\");
    string_replacer(remote_destination,"'","\\'");
    string_replacer(remote_destination,"`","\\`");
    string_replacer(local_source,"\\","\\\\");
    string_replacer(local_source,"'","\\'");
    string_replacer(local_source,"`","\\`");
    actual_command = "adb push '";
    actual_command.append(local_source);
    actual_command.append("' '");
    actual_command.append(remote_destination);
    actual_command.append("'");

    return exec_command(actual_command);
}

queue<string> exec_command(string command)
{
    queue<string> output;
    FILE *fp = popen(command.c_str(), "r" );

    char buff[1000];
    string tmp_string;
    while ( fgets( buff, sizeof buff, fp ) != NULL && !feof(fp) )
    {
        tmp_string.assign(buff);
        tmp_string.erase(tmp_string.size()-2);
        output.push(tmp_string);
    }

    pclose( fp );

    return output;
}

void string_replacer( string &source, const string find, string replace ) {
	size_t j;
	for ( ; (j = source.find( find,j)) != string::npos ; ) {
		source.replace( j, find.length(), replace );
        j = j + replace.length();
	}
}

int xtoi(const char* xs, unsigned int* result)
{
 size_t szlen = strlen(xs);
 int i, xv, fact;

 if (szlen > 0)
 {
  // Converting more than 32bit hexadecimal value?
  if (szlen>8) return 2; // exit

  // Begin conversion here
  *result = 0;
  fact = 1;

  // Run until no more character to convert
  for(i=szlen-1; i>=0 ;i--)
  {
   if (isxdigit(*(xs+i)))
   {
    if (*(xs+i)>=97)
    {
     xv = ( *(xs+i) - 97) + 10;
    }
    else if ( *(xs+i) >= 65)
    {
     xv = (*(xs+i) - 65) + 10;
    }
    else
    {
     xv = *(xs+i) - 48;
    }
    *result += (xv * fact);
    fact *= 16;
   }
   else
   {
    // Conversion was abnormally terminated
    // by non hexadecimal digit, hence
    // returning only the converted with
    // an error value 4 (illegal hex character)
    return 4;
   }
  }
 }

 // Nothing to convert
 return 1;
}
static int adb_getattr(const char *path, struct stat *stbuf)
{
    int res = 0;
    memset(stbuf, 0, sizeof(struct stat));
    queue<string> output;
    string path_string;
    path_string.assign(path);
    string_replacer(path_string," ","\\ ");
    
    if (fileData.find(path_string) ==  fileData.end() || fileData[path_string].timestamp + 30 > time(NULL)){
        string command = "stat -t \"";
        command.append(path_string);
        command.append("\"");
        cout << command<<"\n";
        output = adb_shell(command);
        fileData[path_string].statOutput = output;
        fileData[path_string].timestamp = time(NULL);
    }else{
        output = fileData[path_string].statOutput;
        cout << "from cache " << output.front() <<"\n";
    }
    vector<string> output_chunk = make_array(output.front());
    if (output_chunk.size() < 13){
        return -ENOENT;
    }
    while (output_chunk.size() > 15){
        output_chunk.erase( output_chunk.begin());
    }
    /*
    stat -t Explained:
    file name (%n)
    total size (%s)
    number of blocks (%b)
    raw mode in hex (%f)
    UID of owner (%u)
    GID of file (%g)
    device number in hex (%D)
    inode number (%i)
    number of hard links (%h)
    major devide type in hex (%t)
    minor device type in hex (%T)
    last access time as seconds since the Unix Epoch (%X)
    last modification as seconds since the Unix Epoch (%Y)
    last change as seconds since the Unix Epoch (%Z)
    I/O block size (%o)
    */
    //stbuf->st_dev = atoi(output_chunk[1].c_str());     /* ID of device containing file */
    stbuf->st_ino = atoi(output_chunk[7].c_str());     /* inode number */
    unsigned int raw_mode;
    xtoi(output_chunk[3].c_str(),&raw_mode);
    stbuf->st_mode = raw_mode;    /* protection */
    stbuf->st_nlink = 1;   /* number of hard links */
    stbuf->st_uid = atoi(output_chunk[4].c_str());     /* user ID of owner */
    stbuf->st_gid = atoi(output_chunk[5].c_str());     /* group ID of owner */

    unsigned int device_id;
    xtoi(output_chunk[6].c_str(),&device_id);
    stbuf->st_rdev = device_id;    // device ID (if special file)

    stbuf->st_size = atoi(output_chunk[1].c_str());    /* total size, in bytes */
    stbuf->st_blksize = atoi(output_chunk[14].c_str()); /* blocksize for filesystem I/O */
    stbuf->st_blocks = atoi(output_chunk[2].c_str());  /* number of blocks allocated */
    stbuf->st_atime = atol(output_chunk[11].c_str());   /* time of last access */
    stbuf->st_mtime = atol(output_chunk[12].c_str());   /* time of last modification */
    stbuf->st_ctime = atol(output_chunk[13].c_str());   /* time of last status change */

    return res;
}



static int adb_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi)
{
	(void) offset;
	(void) fi;
    string path_string;
    path_string.assign(path);
    string_replacer(path_string," ","\\ ");
    
    queue<string> output;
    string command = "ls -1a \"";
    command.append(path_string);
    command.append("\"");
    output = adb_shell(command);

    while (output.size() > 0){
        filler(buf, output.front().c_str(), NULL, 0);
        output.pop();
    }


	return 0;
}


static int adb_open(const char *path, struct fuse_file_info *fi)
{
    string path_string;
    string local_path_string;
    path_string.assign(path);
    local_path_string.assign("/tmp/adbfs");
    local_path_string.append(path_string);
    string_replacer(local_path_string," ","\\ ");
    
    queue<string> output;
    string command = "stat -t \"";
    command.append(path_string);
    command.append("\"");
    cout << command<<"\n";
    output = adb_shell(command);
    vector<string> output_chunk = make_array(output.front());
    if (output_chunk.size() < 13){
        return -ENOENT;
    }

    adb_pull(path_string,local_path_string);
    

    return 0;
}

static int adb_read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi)
{
    int fd;
    int res;
    string local_path_string;
    local_path_string.assign("/tmp/adbfs");
    local_path_string.append(path);


    fd = open(local_path_string.c_str(), O_RDONLY);
    
    if(fd == -1)
        return -errno;
        
    res = pread(fd, buf, size, offset);
        if(res == -1)
            res = -errno;
        
    
    return size;
}


static int adb_access(const char *path, int mask) {
    //###cout << "###access[path=" << path << "]" <<  endl;
    return 0;
}

static struct fuse_operations adbfs_oper;

int main(int argc, char *argv[])
{
    memset(&adbfs_oper, sizeof(adbfs_oper), 0);
    adbfs_oper.readdir= adb_readdir;
    adbfs_oper.getattr= adb_getattr;
    adbfs_oper.access= adb_access;
    adbfs_oper.open= adb_open;
    adbfs_oper.read= adb_read;
	return fuse_main(argc, argv, &adbfs_oper, NULL);
}
