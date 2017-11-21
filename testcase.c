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

