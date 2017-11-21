------ READ ------
char buf[1024];
read(0, buf, 128);

------ WRITE ------
write(1, "I am here", 9);


------ OPENDIR -----
DIR * dir = opendir("/rootfs/etc1/");
if(dir == NULL) {
    puts("Directory does not exist");
} else {
    puts("Directory exists");
}

------ READDIR -----
DIR * dir = opendir("/rootfs/bin/");
if(dir == NULL) {
    puts("Directory does not exist");
} else {
    puts("Directory exists");
}

int k;
dentry* curr_dentry = NULL;
while((curr_dentry = readdir(dir)) != NULL) {
    for (k = 0; k < strlen(curr_dentry->name); k++) {
        putchar(curr_dentry->name[k]);
    }
    putchar(' ');
}
closedir(dir);

------ FILE OPEN and CLOSE -----
int fd = open("/rootfs/bin/ls", O_RDONLY);
close(fd);
char c = fd + '0';
putchar(c);

------ GETCWD and CHDIR
char buf[1024];
getcwd(buf, 1024);
puts(buf);

chdir("/../../../rootfs/bin/../etc/../rootfs/etc");

getcwd(buf, 1024);
puts(buf);

    
/* Check sys_mmap, page fault and sys_munmap */
Case 1:
char *p = sys_mmap((void *)0x1000, 100, RW_FLAG); // 1 page
strcpy(p, "Hello");
puts("After sys_mmap and page fault: - ");
puts(p);
sys_munmap(p, 100);
strcpy(p, "World");
puts("After sys_unmap (This should not be printed): - ");
puts(p);

Case 2:
char *p = sys_mmap((void *)0x1000, 4097, RW_FLAG); // 2 pages
strcpy(p, "Hello");
puts("After sys_mmap and page fault: - ");
puts(p);
sys_munmap(p, 100);
p = (char *)0x2000;
strcpy(p, "World");
puts("After sys_unmap (This should be printed): - ");
puts(p);

Case 3:
char *p = sys_mmap((void *)0x1000, 8193, RW_FLAG); // 3 pages
strcpy(p, "Hello");
puts("After sys_mmap and page fault: - ");
puts(p);
sys_munmap(p + 0x1000, 100);
p = (char *)0x3000;
strcpy(p, "World");
puts("After sys_unmap (This should be printed): - ");
puts(p);

Case 4:
char *p = sys_mmap((void *)0x1000, 4097, RW_FLAG); // 3 pages
strcpy(p, "Hello");
puts("After sys_mmap and page fault: - ");
puts(p);
sys_munmap(p + 0x1000, 100);
strcpy(p, "World");
puts("After sys_unmap (This should be printed): - ");
puts(p);
