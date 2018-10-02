/* Starter code for a system tool using only system calls. In this practical, we will make no use of
 * standard C library calls -- we will only use system calls provided by the Linux kernel, and do
 * everything else ourselves. The only exception is localtime(), which can be used to convert seconds
 * to sensible dates.
 *
 * Kasim Terzic, Sep 2018
 *
 * See:
 *
 *  https://blog.packagecloud.io/eng/2016/04/05/the-definitive-guide-to-linux-system-calls/
 *  https://en.wikibooks.org/wiki/X86_Assembly/Interfacing_with_Linux
 *  http://ibiblio.org/gferg/ldp/GCC-Inline-Assembly-HOWTO.html
 *
 *  man 2 syscalls
 *  man 2 stat opendir readdir write localtime inode getdents
 */

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <dirent.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>

// A complete list of linux system call numbers can be found in: /usr/include/asm/unistd_64.h
#define WRITE_SYSCALL 1
// #define GETDENTS_SYSCALL 74  //Does not work
#define BUF_SIZE 1024

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

//Custom implementation of the string.h function strlen
int str_length(char* s) {
  if (s == NULL) return 0;

  int length = 0;
  char* sp = s;

  while (*sp) {
    length += 1;
    sp += 1;
  }
  return length;
}

// //Implementation of the getdents system call, returns linked list of filenames
// FileName* getdents_sys(FileName* filenames, int argc, char** argv) {
//
//   FileName* last_fn = filenames;
//
//   //Taken from the man page with slight tweaks
//     int fd, nread;            //Variables to monitor syscall success/failure
//     char buf[BUF_SIZE];       //Buffer
//     struct linux_dirent *d;   //Linked list of linux_dirent structs
//     int bpos;                 //
//     // char d_type;              //Type of file (directory, file, etc.)
//
//     fd = open(argc > 1 ? argv[1] : ".", O_RDONLY | O_DIRECTORY);
//          if (fd == -1) {
//           fprintf(stderr ,"Something went wrong, fd is: %d\n", fd);
//          }
//
//          for ( ; ; ) {  //TODO change to while (1)?
//            nread = syscall(SYS_getdents, fd, buf, BUF_SIZE);
//
//              if (nread == -1) {
//                //ERROR, changed from original
//                  fprintf(stderr ,"Something went wrong, nread is: %d\n", nread);
//               } else {
//                 //All works fine
//                   fprintf(stderr ,"All is well, nread is: %d\n", nread);
//                   break;
//                }
//         }
//         for (bpos = 0; bpos < nread;) {
//           d = (struct linux_dirent *) (buf + bpos);
//           fprintf(stderr, "%8ld  ", d->d_ino);  //Prints inode number
//           fprintf(stderr, "File: %s\n", d->d_name);
//           //TODO add
//           bpos += d->d_reclen;
//          }
//     //End of copy-paste
// }

int main(int argc, char** argv) {
  /*
  *Logic*
  * Use getdents yo retrieve file name: http://man7.org/linux/man-pages/man2/getdents.2.html
  * Use stat to retrieve file meta data from file name: http://man7.org/linux/man-pages/man2/stat.2.html
  * Write retrieved data to console
  *
  *Example*
  * -rw-rw-r-- 1 20846 20846  167 Sep 28 17:06 Makefile
  * -rw-rw-r-- 1 20846 20846 3922 Sep 28 17:06 myls.c
  */

  // FileName* fn = NULL;
  // fn = getdents_sys(fn, argc, argv);

  /*---STAT---*/
  struct stat sb;

  if (lstat(argv[1], &sb) == -1) {
      fprintf(stderr, "Error: Cannot retrie data on filename %s\n", argv[1]);
      return -1;
   }


  // /*---GETDENTS---*/
  // //Taken from the man page with slight tweaks
  //   int fd, nread;            //Variables to monitor syscall success/failure
  //   char buf[BUF_SIZE];       //Buffer
  //   struct linux_dirent *d;   //Linked list of linux_dirent structs
  //   int bpos;                 //
  //   // char d_type;              //Type of file (directory, file, etc.)
  //
  //   fd = open(argc > 1 ? argv[1] : ".", O_RDONLY | O_DIRECTORY);
  //        if (fd == -1) {
  //         fprintf(stderr ,"Error: Something went wrong, fd is: %d\n", fd);
  //         return -1;
  //        }
  //
  //        for ( ; ; ) {  //TODO change to while (1)?
  //          nread = syscall(SYS_getdents, fd, buf, BUF_SIZE);
  //
  //            if (nread == -1) {
  //              //ERROR, changed from original
  //                fprintf(stderr ,"Something went wrong, nread is: %d\n", nread);
  //             } else {
  //               //All works fine
  //                 fprintf(stderr ,"All is well, nread is: %d\n", nread);
  //                 break;
  //              }
  //       }
  //       for (bpos = 0; bpos < nread;) {
  //         d = (struct linux_dirent *) (buf + bpos);
  //         // fprintf(stderr, "%8ld  ", d->d_ino);  //Prints inode number
  //         fprintf(stderr, "File: %s\n", d->d_name);
  //
  //         bpos += d->d_reclen;
  //        }
  //   //End of copy-paste





//Taken from starter code
  // I am using long ints here to avoid converting them for asm (rxx registers are 64 bits)

  char* text = "Hello world! We come in peace!\n";    // String to print
  size_t len = 31;     // Length of our string, which we need to pass to write syscall
  long handle = 1;     // 1 for stdout, 2 for stderr, file handle from open() for files
  long ret = -1;       // Return value received from the system call

  // First way is to use the wrapper function defined in POSIX. This is the "normal" way
  // of doing things. You may wish to use this in the first instance to get a basic working
  // solution before refactoring it to call the system calls directly
  ret = write(handle, text, len);

  // Using the Linux syscall function for generic calls to the kernel
  // Note -- first argument is the system call number for write. Also note how similar
  // this is to the wrapper above
  ret = syscall( WRITE_SYSCALL, handle, text, len );


  return ret;
}
