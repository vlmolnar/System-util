#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <dirent.h>

//To be removed
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>

// A complete list of linux system call numbers can be found in: /usr/include/asm/unistd_64.h
#define WRITE_SYSCALL 1
// #define GETDENTS_SYSCALL 74  //Does not work
// #define BUF_SIZE 1024

//Taken from getdents man page
struct linux_dirent {
           long           d_ino;
           off_t          d_off;
           unsigned short d_reclen;
           char           d_name[];
       };
//End of copy-paste

//Custom implementation of the string.h function strlen, measures length of a string
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

//A function to convert int variables to strings
void int_to_char(int i, char* cs) {
  //Ref: https://stackoverflow.com/questions/8671845/iterating-through-digits-in-integer-in-c

  char* csp = cs;
  int digit = 1000000000;

  //Checks if int i has single digit
  if (i < 10) {
    printf("%s\n", "Here");
    *cs = i + '0';
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
  printf("Int '%d' to char: %s\n", i, cs);
  printf("Size is: %d\n", my_strlen(cs));
}

//Implementation of getdents Linux system call
int my_getdents(long handle, char* text, size_t text_len) {
  // struct linux_dirent
  return syscall(SYS_getdents, handle, text, text_len);
}

//Implementation of stat Linux system call
int my_stat(char* filename, struct stat *buf) {
  // Stat function header: stat(const char *path, struct stat *buf);
  return stat(filename, buf);
}

//Implementation of open Linux system call
int my_open(const char *path) {
  return open(path, O_RDONLY | O_DIRECTORY);
}

//Implementation of write Linux system call
int my_write(char* text) {
  long handle = 1;  //1 stands for stdout
  size_t text_len = my_strlen(text);
  //Taken from starter code
  return syscall( WRITE_SYSCALL, handle, text, text_len );
}

//Implementation of write to stderr to use for error messages and debugging
int my_write_err(char* text) {
  long handle = 2;  //2 stands for stderr
  size_t text_len = my_strlen(text);
  //Taken from starter code
  return syscall( WRITE_SYSCALL, handle, text, text_len );
}

  // A function that prints out file permissions from a stat struct
void write_permissions(struct stat* file_stat) {
  //Ref: https://stackoverflow.com/questions/10323060/printing-file-permissions-like-ls-l-using-stat2-in-c
  //TODO replace macros function
  my_write(S_ISDIR(file_stat->st_mode) ? "d" : "-");
  my_write(file_stat->st_mode & S_IRUSR ? "r" : "-");
  my_write(file_stat->st_mode & S_IWUSR ? "w" : "-");
  my_write(file_stat->st_mode & S_IXUSR ? "x" : "-");
  my_write(file_stat->st_mode & S_IRGRP ? "r" : "-");
  my_write(file_stat->st_mode & S_IWGRP ? "w" : "-");
  my_write(file_stat->st_mode & S_IXGRP ? "x" : "-");
  my_write(file_stat->st_mode & S_IROTH ? "r" : "-");
  my_write(file_stat->st_mode & S_IWOTH ? "w" : "-");
  my_write(file_stat->st_mode & S_IXOTH ? "x" : "-");

}

//A function that converts int to month name abbreviations, and prints it out
void write_month(int month) {
  switch (month) {
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
    default:my_write_err("Err");
  }
}

//Writes file data to console
int write_ls(char* filename, struct stat *file_stat) {
  //Print permissions Ref: https://stackoverflow.com/questions/10323060/printing-file-permissions-like-ls-l-using-stat2-in-c
  // Ref: https://jameshfisher.com/2017/02/24/what-is-mode_t.html
  // int to char Ref: https://stackoverflow.com/questions/2279379/how-to-convert-integer-to-char-in-c
  // fprintf(stderr, "%d ", (int)file_stat->st_mode);

  struct timespec stat_time = file_stat->st_mtim;
  struct tm *mod_time = localtime(&stat_time.tv_sec);

  write_permissions(file_stat);
  fprintf(stderr, " %d ", (int)file_stat->st_nlink);
  fprintf(stderr, " %d", (int)file_stat->st_uid);
  fprintf(stderr, " %d", (int)file_stat->st_gid);
  fprintf(stderr, " %d ", (int)file_stat->st_size);

  //Date and time of last modification
  // fprintf(stderr, " %d", mod_time->tm_mon);
  write_month(mod_time->tm_mon);
  fprintf(stderr, " %d", mod_time->tm_mday);
  fprintf(stderr, " %d:%d", mod_time->tm_hour, mod_time->tm_min);

  fprintf(stderr, " %s\n", filename); //done

  return 0;


  //Ref:
  // mode_t    st_mode;    /* protection */
  // nlink_t   st_nlink;   /* number of hard links */
  // uid_t     st_uid;     /* user ID of owner */
  // gid_t     st_gid;     /* group ID of owner */
  // off_t     st_size;    /* total size, in bytes */
  // struct timespec st_mtim;  /* Time of last modification */

  //Example format:
  // drwxr-xr-x 2 20846 20846     2 Oct 10 11:45 folder1/

}


// // Writes to stderr, used for debugging
// my_write_err(long handle, char* text, size_t text_len) {
//
// }

int main(int argc, char** argv) {
  /*
  *Logic*
  * Use getdents to retrieve file name: http://man7.org/linux/man-pages/man2/getdents.2.html
  * Use stat to retrieve file meta data from file name: http://man7.org/linux/man-pages/man2/stat.2.html
  * Write retrieved data to console
  *
  *Example*
  * -rw-rw-r-- 1 20846 20846  167 Sep 28 17:06 Makefile
  * -rw-rw-r-- 1 20846 20846 3922 Sep 28 17:06 myls.c
  */

  //TODO remove this
  // char test[11];
  // int_to_char(0,test);
  // int_to_char(5,test);
  // int_to_char(3675,test);

  /*---GETDENTS---*/
  //Taken from the man page with slight tweaks
    int fd, nread;            //Variables to monitor syscall success/failure
    size_t buf_size = 1024;   //Size of buffer to retrieve data in
    char buf[buf_size];       //Buffer
    struct linux_dirent *d;   //Linked list of linux_dirent structs
    int bpos;                 //
    // char d_type;              //Type of file (directory, file, etc.)

    // fd = open(argc > 1 ? argv[1] : ".", O_RDONLY | O_DIRECTORY);
    fd = my_open(argc > 1 ? argv[1] : ".");   //Changed from man
         if (fd == -1) {
          fprintf(stderr ,"Error: Something went wrong, fd is: %d\n", fd);
          return -1;
         }

         for ( ; ; ) {
           // nread = syscall(SYS_getdents, fd, buf, BUF_SIZE);
           nread = my_getdents(fd, buf, buf_size);

             if (nread == -1) {
               //ERROR, changed from original
                 fprintf(stderr ,"Something went wrong, nread is: %d\n", nread);
              } else {
                //All works fine
                  fprintf(stderr ,"All is well, nread is: %d\n", nread);
                  break;
               }
        }

      for (bpos = 0; bpos < nread;) {
        d = (struct linux_dirent *) (buf + bpos);
        //End of copy-paste

        // fprintf(stderr, "%8ld  ", d->d_ino);  //Prints inode number
        // fprintf(stderr, "File: %s\n", d->d_name);
        char* filename = (char*) d->d_name;

        //Skips hidden files and directories (files that have names starting with '.')
        if (filename[0] != '.') {
          // fprintf(stderr, "%s\n", "Filename's not hidden");
          /*---STAT---*/
          struct stat file_stat;

          int val = my_stat(filename, &file_stat);
          // fprintf(stderr, "Stat size: %d\n", (int)file_stat.st_size);
          write_ls(filename, &file_stat);
        }

        bpos += d->d_reclen;    //Taken from man page
      }

}
