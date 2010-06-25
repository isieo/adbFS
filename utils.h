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

queue<string> exec_command(string);
vector<string> make_array(string);
void string_replacer(string&,const string,string);

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
