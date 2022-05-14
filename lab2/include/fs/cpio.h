typedef{
   char	   c_magic[6];
   char	   c_ino[8];
   char	   c_mode[8];
   char	   c_uid[8];
   char	   c_gid[8];
   char	   c_nlink[8];
   char	   c_mtime[8];
   char	   c_filesize[8];
   char	   c_devmajor[8];
   char	   c_devminor[8];
   char	   c_rdevmajor[8];
   char	   c_rdevminor[8];
   char	   c_namesize[8];
   char	   c_check[8];
} cpio_newc_header __attribute__((packed));;

typedef {
   uint32_t    c_ino;
   uint32_t	   c_mode;
   uint32_t	   c_uid;
   uint32_t	   c_gid;
   uint32_t	   c_nlink;
   uint32_t	   c_mtime;
   uint32_t	   c_filesize;
   uint32_t	   c_devmajor;
   uint32_t	   c_devminor;
   uint32_t	   c_rdevmajor;
   uint32_t	   c_rdevminor;
   uint32_t	   c_namesize;
   uint32_t	   c_check;
   char*       filename;
   uint8_t     data;
}f_entry;
