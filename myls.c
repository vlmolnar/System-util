#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <dirent.h>


//Constants to store system call numbers
#define WRITE_SYSCALL 1
#define OPEN_SYSCALL 2
#define STAT_SYSCALL 4
#define GETDENTS_SYSCALL 78

//Struct taken from getdents man page
struct linux_dirent {
     long           d_ino;
     off_t          d_off;
     unsigned short d_reclen;
     char           d_name[];
};
//End of copy-paste


/*
* Custom implementation of the string.h function strlen, measures length of a string
*/
int my_strlen(char* s) {
    if (s == NULL) return 0;

    int length = 0;
    char* sp = s;

    while (*sp) {
        length += 1;
        sp += 1;
    }
    return length;
}

/*
* Custom implementation of the string.h function strcat, concatenates two
* strings and places resulting string in a third variable
*/
void my_strcat(char* str1, char* str2, char* str_dest) {
  //Ref: https://stackoverflow.com/questions/2488563/strcat-implementation
    int i;

    //Copies first string into destination
    for (i = 0; str1[i] != '\0'; i++) {
      str_dest[i] = str1[i];

    }

    //Copies second string to the destination
    for (int j = 0; str2[j] != '\0'; j++) {
      str_dest[i] = str2[j];
      i += 1;
    }
    str_dest[i] = '\0'; //Null-terminates string
}

/*
* A function to convert int variables to strings
*/
void my_int_to_str(int i, char* cs) {
    //Ref: https://stackoverflow.com/questions/8671845/iterating-through-digits-in-integer-in-c

    char* csp = cs;
    int digit = 1000000000;   //Works on 10-digit ints at most

    //Checks if int i has single digit, changed from ref
    if (i < 10) {
        *csp = i + '0';
        csp+=1;
    } else {
        //Finds greatest decimal of number
        while (digit > i*10) {
            digit /= 10;
      }

      //Converts digit to char and adds it to char array *cs
      while (digit /= 10) {
          // printf("Value is: %s\n", );
          *csp = ((i / digit) % 10) + '0' ;
          csp+=1;
      }
    }

    *csp = '\0';  //Null-terminates string

}

/*
* Implementation of getdents Linux system call
*/
int my_getdents(long handle, char* text, size_t text_len) {
    long ret = -1;
    // return syscall(SYS_getdents, handle, text, text_len);

    asm( "movq %1, %%rax\n\t"
         "movq %2, %%rdi\n\t"
         "movq %3, %%rsi\n\t"
         "movq %4, %%rdx\n\t"
         "syscall\n\t"
         "movq %%rax, %0\n\t" :
         "=r"(ret) :
         "r"((long)GETDENTS_SYSCALL),"r"(handle), "r"(text), "r"(text_len) :
         "%rax","%rdi","%rsi","%rdx","memory" );

    return ret;
}

/*
* Implementation of stat Linux system call
*/
int my_stat(char* filename, struct stat *buf) {
    long ret = -1;
    // return stat(filename, buf);

    asm( "movq %1, %%rax\n\t"
         "movq %2, %%rdi\n\t"
         "movq %3, %%rsi\n\t"
         "syscall\n\t"
         "movq %%rax, %0\n\t" :
         "=r"(ret) :
         "r"((long)STAT_SYSCALL),"r"(filename), "r"(buf):
         "%rax","%rdi","%rsi","%rdx","memory" );

    return ret;
}

/*
* Implementation of open Linux system call
*/
int my_open(const char *path) {
    long ret = -1;
    long flags = O_RDONLY | O_DIRECTORY;
    // return open(path, flags);

    asm( "movq %1, %%rax\n\t"
         "movq %2, %%rdi\n\t"
         "movq %3, %%rsi\n\t"
         "syscall\n\t"
         "movq %%rax, %0\n\t" :
         "=r"(ret) :
         "r"((long)OPEN_SYSCALL),"r"(path), "r"(flags):
         "%rax","%rdi","%rsi","%rdx","memory" );

    return ret;
}

/*
* Implementation of write Linux system call
*/
int my_write(char* text) {
    long handle = 1;  //1 stands for stdout
    long ret = -1; //Syscall return value
    size_t text_len = my_strlen(text);

    //Taken from starter code
    // return syscall( WRITE_SYSCALL, handle, text, text_len );

    asm( "movq %1, %%rax\n\t"
         "movq %2, %%rdi\n\t"
         "movq %3, %%rsi\n\t"
         "movq %4, %%rdx\n\t"
         "syscall\n\t"
         "movq %%rax, %0\n\t" :
         "=r"(ret) :
         "r"((long)WRITE_SYSCALL),"r"(handle), "r"(text), "r"(text_len) :
         "%rax","%rdi","%rsi","%rdx","memory" );

    return ret;

}

/*
* Implementation of write to stderr to use for error messages and debugging
*/
int my_write_err(char* text) {
    long handle = 2;  //2 stands for stderr
    long ret = -1; //Syscall return value
    size_t text_len = my_strlen(text);

    //Taken from starter code
    // return syscall( WRITE_SYSCALL, handle, text, text_len );

    asm( "movq %1, %%rax\n\t"
         "movq %2, %%rdi\n\t"
         "movq %3, %%rsi\n\t"
         "movq %4, %%rdx\n\t"
         "syscall\n\t"
         "movq %%rax, %0\n\t" :
         "=r"(ret) :
         "r"((long)WRITE_SYSCALL),"r"(handle), "r"(text), "r"(text_len) :
         "%rax","%rdi","%rsi","%rdx","memory" );

    return ret;
}

/*
* A function that prints out file permissions from a stat struct
*/
void write_permissions(struct stat* file_stat, char d_type) {
    //Ref: https://stackoverflow.com/questions/10323060/printing-file-permissions-like-ls-l-using-stat2-in-c
    // Following code taken from reference:
    my_write((d_type == DT_DIR) ? "d" : "-"); //Modified from reference
    my_write(file_stat->st_mode & S_IRUSR ? "r" : "-");
    my_write(file_stat->st_mode & S_IWUSR ? "w" : "-");
    my_write(file_stat->st_mode & S_IXUSR ? "x" : "-");
    my_write(file_stat->st_mode & S_IRGRP ? "r" : "-");
    my_write(file_stat->st_mode & S_IWGRP ? "w" : "-");
    my_write(file_stat->st_mode & S_IXGRP ? "x" : "-");
    my_write(file_stat->st_mode & S_IROTH ? "r" : "-");
    my_write(file_stat->st_mode & S_IWOTH ? "w" : "-");
    my_write(file_stat->st_mode & S_IXOTH ? "x" : "-");
    //End of copy-paste

}

/*
* A function that converts int to month name abbreviations, and prints it out
*/
void write_date(struct tm *mod_time) {
    char printable[10];

    my_write(" ");


    my_write(" ");
    switch (mod_time->tm_mon) {
        case 0: my_write("Jan"); break;
        case 1: my_write("Feb"); break;
        case 2: my_write("Mar"); break;
        case 3: my_write("Apr"); break;
        case 4: my_write("May"); break;
        case 5: my_write("Jun"); break;
        case 6: my_write("Jul"); break;
        case 7: my_write("Aug"); break;
        case 8: my_write("Sep"); break;
        case 9: my_write("Oct"); break;
        case 10: my_write("Nov"); break;
        case 11: my_write("Dec"); break;
        default: my_write_err("--Err--");
    }

    my_write(" ");
    my_int_to_str(mod_time->tm_mday, printable);
    my_write(printable);

    my_write(" ");
    my_int_to_str(mod_time->tm_hour, printable);
    my_write(printable);

    my_write(":");
    my_int_to_str(mod_time->tm_min, printable);
    my_write(printable);
}

/*
* Writes file data to console
*/
int write_ls(char* filename, struct stat *file_stat, char d_type) {

    struct timespec stat_time = file_stat->st_mtim;
    struct tm *mod_time = localtime(&stat_time.tv_sec);
    char printable[10];

    //Permissions
    write_permissions(file_stat, d_type);

    //Links
    my_int_to_str((int)file_stat->st_nlink, printable);
    my_write(" ");
    my_write(printable);

    //User ID
    my_int_to_str((int)file_stat->st_uid, printable);
    my_write(" ");
    my_write(printable);

    //Group ID
    my_int_to_str((int)file_stat->st_gid, printable);
    my_write(" ");
    my_write(printable);

    //File size
    my_int_to_str((int)file_stat->st_size, printable);
    my_write(" ");
    my_write(printable);

    //Modification date
    write_date(mod_time);


    //File name
    my_write(" ");
    my_write(filename);
    my_write("\n");

    return 0;

}

/*
* Main method, contains calls to open, getdents and stat syscalls
*/
int main(int argc, char** argv) {

    //Taken from getdents man page with slight tweaks
    int fd, nread;            //Variables to monitor syscall success/failure
    size_t buf_size = 1024;   //Size of buffer to retrieve data in
    char buf[buf_size];       //Buffer
    struct linux_dirent *d;   //Linked list of linux_dirent structs
    int bpos;                 //Incremented until everything in buffer is processed
    char d_type;              //File type such as regular, directory, etc.

    char* filename;           //Name of file, changed from man
    char* filepath = argc > 1 ? argv[1] : "."; //Path to file without file name, changed from man

    //--OPEN--
    // fd = my_open(argc > 1 ? argv[1] : ".");   //Changed from man
    fd = my_open(filepath);   //Changed from man
         if (fd == -1) {
             my_write_err("Error: Something went wrong when opening current directory\n");
             return -1;
         }

         for ( ; ; ) {
             //--GETDENTS--
             nread = my_getdents(fd, buf, buf_size);  //Changed from man

               if (nread == -1) {
                    //ERROR, changed from man
                    my_write_err("Error: Something went wrong while calling GETDENTS system call\n");
                    return -1;
                } else {
                    //All works fine
                    break;
                }
        }

      for (bpos = 0; bpos < nread;) {
          d = (struct linux_dirent *) (buf + bpos);
          d_type = *(buf + bpos + d->d_reclen - 1);
          //End of getdents man copy-paste

          filename = (char*) d->d_name;

          //Skips hidden files and directories, including current and parent directory
          //(all files that have names starting with '.')
          if (filename[0] != '.') {
              //--STAT--
              struct stat file_stat;

              //TODO string concatentation: https://stackoverflow.com/questions/2488563/strcat-implementation
              //call my_stat with base + filename, file_stat

              char full_path[30];
              my_strcat(filepath, "/", full_path);
              my_strcat(full_path, filename, full_path);

              int val = my_stat(full_path, &file_stat);
              // my_write_err(filename);
              if (val) {
                  my_write_err("Error: Something went wrong while calling STAT system call\n");
                  return -1;
              }
              write_ls(filename, &file_stat, d_type);
          }
          bpos += d->d_reclen;    //Taken from stat man page
      }
}
