#ifndef __DECLARATION_H__
#define __DECLARATION_H__

size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);
void init_ramdisk() ;
size_t get_ramdisk_size() ;

// void naive_uload(PCB *pcb, const char *filename);

size_t serial_write(const void *buf, size_t offset, size_t len);
size_t events_read(void *buf, size_t offset, size_t len) ;
size_t dispinfo_read(void *buf, size_t offset, size_t len);
size_t fb_write(const void *buf, size_t offset, size_t len);
void init_device() ;


#endif