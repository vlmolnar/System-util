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

//This is the struct that gets printed out. The different attributes are retrieved
//from the stat system call
//Name is retrieved from getdents
typedef struct myFile {
  //Fron stat
  mode_t    st_mode;    /* protection */
  nlink_t   st_nlink;   /* number of hard links */
  uid_t     st_uid;     /* user ID of owner */
  gid_t     st_gid;     /* group ID of owner */
  off_t     st_size;    /* total size, in bytes */
  //date&time
  int date;
  //From getdents
  unsigned char  d_type;   /* File type */
  char*      d_name; /* Filename (null-terminated) */
} MyFile;

// Maybe?
// typedef struct fileName {
//   char*      d_name;
//   fileName*   next;
// } FileName;


//Taken from getdents man page
struct linux_dirent {
           long           d_ino;
           off_t          d_off;
           unsigned short d_reclen;
           char           d_name[];
       };
//End of copy-paste

//Taken from the stat man page
// struct stat {
//                dev_t     st_dev;         /* ID of device containing file */
//                ino_t     st_ino;         /* Inode number */
//                mode_t    st_mode;        /* File type and mode */
//                nlink_t   st_nlink;       /* Number of hard links */
//                uid_t     st_uid;         /* User ID of owner */
//                gid_t     st_gid;         /* Group ID of owner */
//                dev_t     st_rdev;        /* Device ID (if special file) */
//                off_t     st_size;        /* Total size, in bytes */
//                blksize_t st_blksize;     /* Block size for filesystem I/O */
//                blkcnt_t  st_blocks;      /* Number of 512B blocks allocated */
//
//                /* Since Linux 2.6, the kernel supports nanosecond
//                   precision for the following timestamp fields.
//                   For the details before Linux 2.6, see NOTES. */
//
//                struct timespec st_atim;  /* Time of last access */
//                struct timespec st_mtim;  /* Time of last modification */
//                struct timespec st_ctim;  /* Time of last status change */
//            //
//            #define st_atime st_atim.tv_sec      /* Backward compatibility */
//            #define st_mtime st_mtim.tv_sec
//            #define st_ctime st_ctim.tv_sec
//            };
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

////Implementation of open Linux system call
int my_open(const char *path) {
  return open(path, O_RDONLY | O_DIRECTORY);
}

//Implementation of write Linux system call
int my_write(long handle, char* text, size_t text_len) {
  //Taken from starter code
  return syscall( WRITE_SYSCALL, handle, text, text_len );
}

//Writes file data to console
int write_ls(char* filename, struct stat *file_stat) {
  //Print permissions https://stackoverflow.com/questions/10323060/printing-file-permissions-like-ls-l-using-stat2-in-c
  fprintf(stderr, "Stat size in func: %d ", (int)file_stat->st_mode);
  fprintf(stderr, "%d ", (int)file_stat->st_nlink); //done
  fprintf(stderr, "%d ", (int)file_stat->st_uid); //done
  fprintf(stderr, "%d ", (int)file_stat->st_gid); //done
  fprintf(stderr, "%d ", (int)file_stat->st_size); //done
  fprintf(stderr, "%s\n", filename); //done
  return 0;

  //Ref:
  // mode_t    st_mode;    /* protection */
  // nlink_t   st_nlink;   /* number of hard links */
  // uid_t     st_uid;     /* user ID of owner */
  // gid_t     st_gid;     /* group ID of owner */
  // off_t     st_size;    /* total size, in bytes */
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
        fprintf(stderr, "File: %s\n", d->d_name);
        char* filename = (char*) d->d_name;

        //Skips hidden files and directories (files that have names starting with '.')
        if (filename[0] != '.') {
          fprintf(stderr, "%s\n", "Filename's not hidden");
          /*---STAT---*/
          struct stat file_stat;

          int val = my_stat(filename, &file_stat);
          // fprintf(stderr, "Stat size: %d\n", (int)file_stat.st_size);
          write_ls(filename, &file_stat);
        }

        bpos += d->d_reclen;    //Taken from man page
      }

}
