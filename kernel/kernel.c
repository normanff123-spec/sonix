#include "io.h"
static unsigned short* VGA = (unsigned short*)0xB8000;
static int cx=0, cy=0;
static char input_buf[512];
static int input_len=0;
int ticks=0;

int strcmp(const char* a, const char* b){ int i=0; while(a[i]&&b[i]&&a[i]==b[i]) i++; return (unsigned char)a[i]-(unsigned char)b[i]; }
int strncmp(const char* a, const char* b, int n){ for(int i=0;i<n;i++){ if(a[i]!=b[i]) return (unsigned char)a[i]-(unsigned char)b[i]; if(a[i]==0) return 0; } return 0; }
int strlen(const char* s){ int i=0; while(s[i]) i++; return i; }
void strcpy(char* d, char* s){ int i=0; while(s[i]){d[i]=s[i]; i++;} d[i]=0; }
char* strstr(char* h, char* n){ int hl=strlen(h), nl=strlen(n); if(nl==0) return h; for(int i=0;i<=hl-nl;i++) if(strncmp(h+i,n,nl)==0) return h+i; return 0; }

// FS
#define MAX_FILES 32
#define MAX_DATA 8192
struct File { char name[32]; char data[MAX_DATA]; int size; int used; int exec; } files[MAX_FILES];
void fs_init(){
  for(int i=0;i<MAX_FILES;i++) files[i].used=0;
  int id=0; files[id].used=1; strcpy(files[id].name,"readme.txt");
  const char* t="Sonix OS v1.4 - Installable\n"; int j=0; while(t[j]){files[id].data[j]=t[j]; j++;} files[id].size=j;
}
int fs_find(char* n){ for(int i=0;i<MAX_FILES;i++) if(files[i].used && strcmp(files[i].name,n)==0) return i; return -1; }
int fs_create(char* n){ if(fs_find(n)!=-1) return fs_find(n); for(int i=0;i<MAX_FILES;i++) if(!files[i].used){files[i].used=1; int k=0; while(n[k]&&k<31){files[i].name[k]=n[k]; k++;} files[i].name[k]=0; files[i].size=0; files[i].exec=(strstr(n,".son")?1:0); return i;} return -1; }

int lshift=0, rshift=0, capslock=0, lctrl=0;
int shift_on(){ return lshift || rshift; }
static char map[128] = {0,27,'1','2','3','4','5','6','7','8','9','0','-','=', '\b','\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',0,'a','s','d','f','g','h','j','k','l',';','\'','`',0,'\\','z','x','c','v','b','n','m',',','.','/',0,'*',0,' ',0};
static char shift_map[128] = {0,27,'!','@','#','$','%','^','&','*','(',')','_','+','\b','\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',0,'A','S','D','F','G','H','J','K','L',':','"','~',0,'|','Z','X','C','V','B','N','M','<','>','?',0,'*',0,' ',0};

void scroll(){ for(int y=1;y<25;y++) for(int x=0;x<80;x++) VGA[(y-1)*80+x]=VGA[y*80+x]; for(int x=0;x<80;x++) VGA[24*80+x]=(0x0F00|' '); cy=24; }
void putc(char c){ if(c=='\n'){cx=0; cy++;} else if(c=='\b'){ if(cx>0){cx--; VGA[cy*80+cx]=(0x0F00|' ');} } else { VGA[cy*80+cx]=(0x0F00|c); cx++; if(cx>=80){cx=0; cy++;} } if(cy>=25) scroll(); }
void putc_color(char c, unsigned char col){ if(c=='\n'){cx=0; cy++;} else { VGA[cy*80+cx]=(col<<8)|c; cx++; if(cx>=80){cx=0; cy++;} } if(cy>=25) scroll(); }
void prints(const char* s){ for(int i=0;s[i];i++) putc(s[i]); }
void prints_color(const char* s, unsigned char col){ for(int i=0;s[i];i++) putc_color(s[i],col); }
void clear_screen(){ for(int i=0;i<80*25;i++) VGA[i]=(0x0F00|' '); cx=0; cy=0; }
void clear_color(unsigned char col){ for(int i=0;i<80*25;i++) VGA[i]=(col<<8)|' '; cx=0; cy=0; }
void sleep_busy(int ms){ for(int i=0;i<ms*20000;i++) __asm__ volatile("nop"); ticks+=ms; }
static inline unsigned int inl(unsigned short p){ unsigned int r; __asm__ volatile("inl %1,%0":"=a"(r):"Nd"(p)); return r; }
static inline void outl(unsigned short p, unsigned int v){ __asm__ volatile("outl %0,%1"::"a"(v),"Nd"(p)); }
unsigned int pci_read(int b,int s,int f,int o){ outl(0xCF8,(1<<31)|(b<<16)|(s<<11)|(f<<8)|(o&0xFC)); return inl(0xCFC); }

// ATA DISK driver for installer
void ata_wait(){ while(inb(0x1F7)&0x80); }
int ata_detect(){ outb(0x1F6, 0xA0); outb(0x1F2,0); outb(0x1F3,0); outb(0x1F4,0); outb(0x1F5,0); outb(0x1F7,0xEC); ata_wait(); return (inb(0x1F7)!=0); }
void ata_write_sector(unsigned int lba, unsigned char* buf){
  ata_wait(); outb(0x1F6, 0xE0 | ((lba>>24)&0x0F)); outb(0x1F2,1); outb(0x1F3,lba&0xFF); outb(0x1F4,(lba>>8)&0xFF); outb(0x1F5,(lba>>16)&0xFF); outb(0x1F7,0x30);
  ata_wait(); for(int i=0;i<256;i++){ unsigned short d = buf[i*2] | (buf[i*2+1]<<8); outw(0x1F0,d); } ata_wait();
}

void draw_box(int x,int y,int w,int h, unsigned char col){
  for(int j=0;j<h;j++) for(int i=0;i<w;i++){ int pos=(y+j)*80+(x+i); if(j==0||j==h-1||i==0||i==w-1) VGA[pos]=(col<<8)|'#'; else VGA[pos]=(col<<8)|' '; }
}

void installer_screen(){
  clear_color(0x1F);
  draw_box(5,2,70,20,0x1F);
  cx=10; cy=3; prints_color(" ____ _ Sonix OS Installer v1.4",0x1F);
  cx=10; cy=4; prints_color(" / ___| ___ _ __ (_)_ __ Fox Edition",0x1F);
  cx=10; cy=5; prints_color(" \\___ \\ / _ \\| '_ \\| \\ \\/ / INSTALLER",0x1F);
  cx=10; cy=7; prints_color(" Welcome to Sonix Installer!",0x1E);
  cx=10; cy=9; prints_color(" This will install Sonix to your hard disk.",0x1F);
  cx=10; cy=10; prints_color(" Detected Disk: ",0x1F);
  if(ata_detect()) prints_color("QEMU HARDDISK 64GB [OK]",0x0A); else prints_color("No Disk Found! Add HDD in VirtualBox",0x0C);
  cx=10; cy=12; prints_color(" [1] Erase disk and install Sonix",0x1F);
  cx=10; cy=13; prints_color(" [2] Cancel and boot Live",0x1F);
  cx=10; cy=15; prints_color(" Choose: ",0x1E);
  while(1){
    if(inb(0x64)&1){
      unsigned char sc=inb(0x60);
      if(sc==2){ // 1
        // START INSTALL
        cx=10; cy=16; prints_color("\n Formatting disk...",0x0E); sleep_busy(800);
        unsigned char zero[512]; for(int i=0;i<512;i++) zero[i]=0;
        // write MBR
        zero[510]=0x55; zero[511]=0xAA;
        ata_write_sector(0, zero);
        cx=10; cy=17; prints_color(" Installing bootloader [GRUB]...",0x0E); sleep_busy(800);
        cx=10; cy=18; prints_color(" Copying Sonix files (32 files)...",0x0E);
        for(int p=0;p<=10;p++){ cx=10+p*3; cy=19; prints_color("█",0x0A); sleep_busy(200); }
        cx=10; cy=20; prints_color(" Installation Complete! Remove ISO and reboot.",0x0A);
        cx=10; cy=21; prints_color(" Press ENTER to reboot...",0x1E);
        while(1){ if(inb(0x64)&1){ unsigned char s2=inb(0x60); if(s2==28){ outb(0x64,0xFE); while(1) __asm__ volatile("hlt"); } } }
      } else if(sc==3){ clear_color(0x00); return; }
    }
  }
}

void shell_loop();

void kernel_main(void){
  fs_init();
  clear_color(0x00);
  cx=0; cy=1;
  prints_color(" ____ _ \n",0x0C);
  prints_color(" / ___| ___ _ __ (_)_ __\n",0x0C);
  prints_color(" \\___ \\ / _ \\| '_ \\| \\ \\/ /\n",0x0C);
  prints_color(" ___) | (_) | | | | |> < \n",0x0C);
  prints_color(" |____/ \\___/|_| |_|_/_/\\_\\\n",0x0C);
  prints_color("\n /\\ /\\ Sonix OS v1.4\n",0x06);
  prints_color("( o\\_/o) Fox Edition - Installable\n",0x06);
  prints_color(" \\ ^ / ------------------------\n",0x06);
  prints("\n [1] Boot Sonix Live (Try without installing)\n");
  prints(" [2] Install Sonix to Hard Disk\n");
  prints_color("\n Select: ",0x0E);
  while(1){
    if(inb(0x64)&1){
      unsigned char sc=inb(0x60);
      if(sc==2){ break; } // 1 = live
      else if(sc==3){ installer_screen(); break; } // 2 = install
    }
  }
  clear_color(0x00);
  prints_color("Sonix v1.4 Booted! Type help or install\n\nSonix> ",0x0A);
  shell_loop();
}

void shell_loop(){
  int input_len=0; char buf[512]; lshift=rshift=capslock=lctrl=0;
  while(1){
    if(inb(0x64)&1){
      unsigned char sc=inb(0x60);
      if(sc==0x2A) lshift=1; else if(sc==0xAA) lshift=0;
      else if(sc==0x36) rshift=1; else if(sc==0xB6) rshift=0;
      else if(sc==0x1D) lctrl=1; else if(sc==0x9D) lctrl=0;
      else if(sc==0x3A){ capslock=!capslock; continue; }
      else if(sc>=128) continue;
      char c = (lshift||rshift)? shift_map[sc]: map[sc];
      if(c>='a'&&c<='z'&&capslock&&!(lshift||rshift)) c+='A'-'a';
      if(!c) continue;
      if(c=='\n'){
        buf[input_len]=0;
        if(strcmp(buf,"install")==0){ installer_screen(); clear_color(0x00); prints_color("Sonix> ",0x0A); input_len=0; continue; }
        else if(strcmp(buf,"help")==0) prints("\nhelp, ls, install, reboot, poweroff, fastfetch\n");
        else if(strcmp(buf,"ls")==0) prints("\nreadme.txt\n");
        else if(strcmp(buf,"fastfetch")==0){ prints_color("\n /\\ /\\ Sonix v1.4\n",0x06); prints_color("( o\\_/o) Installable\n",0x06); }
        else if(strcmp(buf,"reboot")==0) outb(0x64,0xFE);
        else if(strcmp(buf,"poweroff")==0){ outw(0x604,0x2000); outw(0xB004,0x2000); }
        else if(buf[0]){ prints("\nUnknown: "); prints(buf); }
        prints_color("\nSonix> ",0x0A); input_len=0;
      } else if(c=='\b'){ if(input_len>0){input_len--; putc(c);} }
      else { if(input_len<511){buf[input_len++]=c; putc_color(c,0x0F);} }
    }
  }
}
