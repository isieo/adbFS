/*
 *      Redistribution and use in source and binary forms, with or without
 *      modification, are permitted provided that the following conditions are
 *      met:
 *      
 *      * Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *      * Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following disclaimer
 *        in the documentation and/or other materials provided with the
 *        distribution.
 *      * Neither the name of the  nor the names of its
 *        contributors may be used to endorse or promote products derived from
 *        this software without specific prior written permission.
 *      
 *      THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *      "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *      LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *      A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *      OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *      SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *      LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *      DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *      THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *      (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *      OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#define FUSE_USE_VERSION 26
#include "utils.h"

using namespace std;

queue<string> adb_push(string, string);
queue<string> adb_pull(string, string);
queue<string> adb_shell(string);
queue<string> shell(string);
map<string,fileCache> fileData;
map<int,bool> filePendingWrite;
map<string,bool> fileTruncated;
void clearTmpDir();

queue<string> shell(string command)
{
    string actual_command;
    string_replacer(command,"\\","\\\\");
    string_replacer(command,"'","\\'");
    string_replacer(command,"`","\\`");
    actual_command.append(command);

    return exec_command(actual_command);
}

void clearTmpDir(){
    shell("rm -rf /tmp/adbfs");
    mkdir("/tmp/adbfs/",0755);
}


queue<string> adb_shell(string command)
{
    string actual_command;
    string_replacer(command,"\\","\\\\");
    string_replacer(command,"(","\\(");
    string_replacer(command,")","\\)");
    string_replacer(command,"'","\\'");
    string_replacer(command,"`","\\`");
    string_replacer(command,"|","\\|");
    string_replacer(command,"&","\\&");
    string_replacer(command,";","\\;");
    string_replacer(command,"<","\\<");
    string_replacer(command,">","\\>");
    string_replacer(command,"*","\\*");
    string_replacer(command,"#","\\#");
    string_replacer(command,"%","\\%");
    string_replacer(command,"=","\\=");
    string_replacer(command,"~","\\~");
    string_replacer(command,"/[0;0m","");
    string_replacer(command,"/[1;32m","");
    string_replacer(command,"/[1;34m","");
    string_replacer(command,"/[1;36m","");
 
    actual_command = "adb shell busybox ";
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

    cout << "Adb command : " <<actual_command << "\n";
    return exec_command(actual_command);
}



static int adb_getattr(const char *path, struct stat *stbuf)
{
    int res = 0;
    memset(stbuf, 0, sizeof(struct stat));
    queue<string> output;
    string path_string;
    path_string.assign(path);
    //string_replacer(path_string," ","\\ ");

    if (true || fileData.find(path_string) ==  fileData.end() || fileData[path_string].timestamp + 30 > time(NULL)){
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
    stbuf->st_mode = raw_mode | 0700;    /* protection */
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
    string local_path_string;
    path_string.assign(path);
    local_path_string.assign("/tmp/adbfs/");
    string_replacer(path_string,"/","-");
    local_path_string.append(path_string);
    path_string.assign(path);
    //mkdir(local_path_string.c_str(),0755);
    string_replacer(path_string," ","\\ ");

    queue<string> output;
    string command = "ls -1a --color=none \"";
    command.append(path_string);
    command.append("\"");
    output = adb_shell(command);
    vector<string> output_chunk = make_array(output.front());
    if (output_chunk.size() >6){
        return -ENOENT;
    }
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
    local_path_string.assign("/tmp/adbfs/");
    string_replacer(path_string,"/","-");
    local_path_string.append(path_string);
    path_string.assign(path);
    //string_replacer(local_path_string," ","\\ ");
    if (!fileTruncated[path_string]){
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
    }else{
        fileTruncated[path_string] = false;
    }

    fi->fh = open(local_path_string.c_str(), fi->flags);

    return 0;
}

static int adb_read(const char *path, char *buf, size_t size, off_t offset,
    struct fuse_file_info *fi)
{
    int fd;
    int res;
    fd = fi->fh; //open(local_path_string.c_str(), O_RDWR);
    if(fd == -1)
        return -errno;
    res = pread(fd, buf, size, offset);
    //close(fd);
    if(res == -1)
        res = -errno;

    return size;
}

static int adb_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    string path_string;
    string local_path_string;
    path_string.assign(path);
    //local_path_string.assign("/tmp/adbfs/");
    //local_path_string.append(path_string);
    //string_replacer(local_path_string," ","\\ ");

    int fd = fi->fh; //open(local_path_string.c_str(), O_CREAT|O_RDWR|O_TRUNC);

    filePendingWrite[fd] = true;

    int res = pwrite(fd, buf, size, offset);
    //close(fd);
    //adb_push(local_path_string,path_string);
    //adb_shell("sync");
    if (res == -1)
        res = -errno;
    return res;
}


static int adb_flush(const char *path, struct fuse_file_info *fi) {
    string path_string;
    string local_path_string;
    path_string.assign(path);
    local_path_string.assign("/tmp/adbfs/");
    string_replacer(path_string,"/","-");
    local_path_string.append(path_string);
    path_string.assign(path);
    int flags = fi->flags;
    int fd = fi->fh;
    cout << "flag is: "<< flags <<"\n";
    if (filePendingWrite[fd]) {
        filePendingWrite[fd] = false;
        adb_push(local_path_string,path_string);
        adb_shell("sync");
    }
    return 0;
}

static int adb_release(const char *path, struct fuse_file_info *fi) {
    int fd = fi->fh;
    filePendingWrite.erase(filePendingWrite.find(fd));
    close(fd);
    return 0;
}

static int adb_access(const char *path, int mask) {
    //###cout << "###access[path=" << path << "]" <<  endl;
    return 0;
}

static int adb_utimens(const char *path, const struct timespec ts[2]) {
    string path_string;
    string local_path_string;
    path_string.assign(path);
    fileData[path_string].timestamp = fileData[path_string].timestamp + 50;
    local_path_string.assign("/tmp/adbfs/");
    string_replacer(path_string,"/","-");
    local_path_string.append(path_string);
    path_string.assign(path);

    queue<string> output;
    string command = "touch \"";
    command.append(path_string);
    command.append("\"");
    cout << command<<"\n";
    adb_shell(command);

    return 0;
}

static int adb_truncate(const char *path, off_t size) {
    string path_string;
    string local_path_string;
    path_string.assign(path);
    fileData[path_string].timestamp = fileData[path_string].timestamp + 50;
    local_path_string.assign("/tmp/adbfs/");
    string_replacer(path_string,"/","-");
    local_path_string.append(path_string);
    path_string.assign(path);

    queue<string> output;
    string command = "stat -t \"";
    command.append(path_string);
    command.append("\"");
    cout << command<<"\n";
    output = adb_shell(command);
    vector<string> output_chunk = make_array(output.front());
    if (output_chunk.size() < 13){
        adb_pull(path_string,local_path_string);
    }

    fileTruncated[path_string] = true;

    cout << "truncate[path=" << local_path_string << "][size=" << size << "]" << endl;

    return truncate(local_path_string.c_str(),size);
}

static int adb_mknod(const char *path, mode_t mode, dev_t rdev) {
    string path_string;
    string local_path_string;
    path_string.assign(path);
    local_path_string.assign("/tmp/adbfs/");
    string_replacer(path_string,"/","-");
    local_path_string.append(path_string);
    path_string.assign(path);

    cout << "mknod for " << local_path_string << "\n";
    mknod(local_path_string.c_str(),mode, rdev);
    adb_push(local_path_string,path_string);
    adb_shell("sync");

    fileData[path_string].timestamp = fileData[path_string].timestamp + 50;

    return 0;
}

static int adb_mkdir(const char *path, mode_t mode) {
    string path_string;
    string local_path_string;
    path_string.assign(path);
    fileData[path_string].timestamp = fileData[path_string].timestamp + 50;
    local_path_string.assign("/tmp/adbfs/");
    string_replacer(path_string,"/","-");
    local_path_string.append(path_string);
    path_string.assign(path);
    string command;
    command.assign("mkdir '");
    command.append(path_string);
    command.append("'");
    adb_shell(command);
    return 0;
}

static int adb_rename(const char *from, const char *to) {
    string local_from_string,local_to_string ="/tmp/adbfs/";

    local_from_string.append(from);
    local_to_string.append(to);
    string command = "mv '";
    command.append(from);
    command.append("' '");
    command.append(to);
    command.append("'");
    cout << "Renaming " << from << " to " << to <<"\n";
    adb_shell(command);
    return 0;
}

static int adb_rmdir(const char *path) {
    string path_string;
    string local_path_string;
    path_string.assign(path);
    fileData[path_string].timestamp = fileData[path_string].timestamp + 50;
    local_path_string.assign("/tmp/adbfs/");
    string_replacer(path_string,"/","-");
    local_path_string.append(path_string);
    path_string.assign(path);

    string command = "rmdir '";
    command.append(path_string);
    command.append("'");
    adb_shell(command);

    //rmdir(local_path_string.c_str());
    return 0;
}

static int adb_unlink(const char *path) {
    string path_string;
    string local_path_string;
    path_string.assign(path);
    fileData[path_string].timestamp = fileData[path_string].timestamp + 50;
    local_path_string.assign("/tmp/adbfs/");
    string_replacer(path_string,"/","-");
    local_path_string.append(path_string);
    path_string.assign(path);

    string command = "rm '";
    command.append(path_string);
    command.append("'");
    adb_shell(command);

    unlink(local_path_string.c_str());
    return 0;
}

static struct fuse_operations adbfs_oper;

int main(int argc, char *argv[])
{
    clearTmpDir();
    memset(&adbfs_oper, sizeof(adbfs_oper), 0);
    adbfs_oper.readdir= adb_readdir;
    adbfs_oper.getattr= adb_getattr;
    adbfs_oper.access= adb_access;
    adbfs_oper.open= adb_open;
    adbfs_oper.open= adb_open;
    adbfs_oper.flush = adb_flush;
    adbfs_oper.release = adb_release;
    adbfs_oper.read= adb_read;
    adbfs_oper.write = adb_write;
    adbfs_oper.utimens = adb_utimens;
    adbfs_oper.truncate = adb_truncate;
    adbfs_oper.mknod = adb_mknod;
    adbfs_oper.mkdir = adb_mkdir;
    adbfs_oper.rename = adb_rename;
    adbfs_oper.rmdir = adb_rmdir;
    adbfs_oper.unlink = adb_unlink;
    return fuse_main(argc, argv, &adbfs_oper, NULL);
}
