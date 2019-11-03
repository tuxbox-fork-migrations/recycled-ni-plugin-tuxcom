/*
	TuxCom - TuxBox-Commander Plugin

	Copyright (C) 2004 'dbluelle' (dbluelle@blau-weissoedingen.de)

	Homepage: http://www.blau-weissoedingen.de/dreambox/

	License: GPL

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA
*/

#include <config.h>
#define _FILE_OFFSET_BITS 64
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <errno.h>
#include <locale.h>
#include <fcntl.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/dir.h>
#include <sys/stat.h>
//#include <plugin.h>
#if defined(HAVE_SPARK_HARDWARE) || defined(HAVE_DUCKBOX_HARDWARE)
#include <linux/stmfb.h>
#endif

#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_CACHE_H
#include FT_CACHE_SMALL_BITMAPS_H


#include <linux/input.h>

#define MENUROWS      10
#define MENUITEMS     10
#define _MENUSIZE       59
#define _MINBOX        380
#define _BUTTONWIDTH   114
#define _BUTTONHEIGHT  30
int MENUSIZE, MINBOX, BUTTONWIDTH, BUTTONHEIGHT;
#define COLORBUTTONS  4

#define LEFTFRAME    0
#define RIGHTFRAME   1

#ifndef CONFIGDIR
#define CONFIGDIR "/var/tuxbox/config"
#endif
#ifndef FONTDIR
#define FONTDIR	"/share/fonts"
#endif
#define DEFAULT_PATH "/"
static const char *charset = " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789#!$%&?*()@\\/=<>+-_,.;:";

#define FILEBUFFER_SIZE (100 * 1024) // Edit files up to 100k
#define FTPBUFFER_SIZE  (200 * 1024) // FTP Download Buffer size

#define MSG_VERSION    "Tuxbox Commander Version 1.22"
#define MSG_COPYRIGHT  "� dbluelle 2004-2007"

// rc codes
#define	RC_0			'0'
#define	RC_1			'1'
#define	RC_2			'2'
#define	RC_3			'3'
#define	RC_4			'4'
#define	RC_5			'5'
#define	RC_6			'6'
#define	RC_7			'7'
#define	RC_8			'8'
#define	RC_9			'9'

#define	RC_RIGHT	0x0191
#define	RC_LEFT		0x0192
#define	RC_UP			0x0193
#define	RC_DOWN		0x0194
#define	RC_PLUS		0x0195
#define	RC_MINUS	0x0196

#define	RC_OK				0x0D
#define	RC_STANDBY	0x1C
#define RC_ESC			RC_HOME

#define	RC_HOME			0x01B1
#define	RC_MUTE			0x01B2
#define	RC_HELP			0x01B3
#define	RC_DBOX			0x01B4

#define	RC_GREEN	0x01A1
#define	RC_YELLOW	0x01A2
#define	RC_RED		0x01A3
#define	RC_BLUE		0x01A4

#define RC_PAUSE	RC_HELP
#define RC_ALTGR	0x12
#define RC_BS			0x7F
#define RC_POS1		RC_HOME
#define RC_END		0x13
#define RC_INS		0x10
#define RC_ENTF		0x11
#define RC_STRG		0x00
#define RC_LSHIFT	0x0E
#define RC_RSHIFT	0x0E
#define RC_ALT		0x0F
#define RC_NUM		RC_DBOX
#define RC_ROLLEN	0x00
#define RC_F5			RC_DBOX
#define RC_F6			RC_HELP
#define RC_F7			RC_MUTE
#define RC_F8			0x01C8
#define RC_F9			0x01C9
#define RC_F10		0x01CA
#define RC_RET		0x0D
#define RC_RET1		0x01CC
#define RC_CAPSLOCK	0x01CD
#define RC_ON			0x01CE

#define RC_F1		RC_RED
#define RC_F2		RC_GREEN
#define RC_F3		RC_YELLOW
#define RC_F4		RC_BLUE
#define RC_PAGEUP	RC_PLUS
#define RC_PAGEDOWN	RC_MINUS

int rctable[] =
{
   0x00, RC_ESC, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '�', '�', RC_BS, 0x09,
   'q',  'w', 'e', 'r', 't', 'z', 'u', 'i', 'o', 'p', '�', '+', RC_RET, RC_STRG, 'a', 's',
   'd',  'f', 'g', 'h', 'j', 'k', 'l', '�', '�', '^', RC_LSHIFT, '#', 'y', 'x', 'c', 'v',
   'b',  'n', 'm', ',', '.', '-', RC_RSHIFT, 0x00, RC_ALT, 0x20, RC_CAPSLOCK,RC_F1,RC_F2,RC_F3,RC_F4,RC_F5,
   RC_F6,RC_F7,RC_F8,RC_F9,RC_F10,RC_NUM,RC_ROLLEN,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, RC_STANDBY, 0x00, 0x00, 0x00, 0x00, '<', RC_OK, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, RC_ALTGR, 0x00, RC_POS1, RC_UP, RC_PAGEUP, RC_LEFT, RC_RIGHT, RC_END, RC_DOWN,RC_PAGEDOWN,RC_INS,RC_ENTF,
   0x00, RC_MUTE, RC_MINUS, RC_PLUS, RC_STANDBY, 0x00, 0x00, RC_PAUSE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
int rcshifttable[] =
{
   0x00, RC_ESC, '!', '"', '�', '$', '%', '&', '/', '(', ')', '=', '?', '`', 0x08, 0x09,
   'Q',  'W', 'E', 'R', 'T', 'Z', 'U', 'I', 'O', 'P', '�', '*', RC_RET1, RC_STRG, 'A', 'S',
   'D',  'F', 'G', 'H', 'J', 'K', 'L', '�', '�', '�', RC_LSHIFT, 0x27, 'Y', 'X', 'C', 'V',
   'B',  'N', 'M', ';', ':', '_', RC_RSHIFT, 0x00, RC_ALT, 0x20, RC_CAPSLOCK,RC_F1,RC_F2,RC_F3,RC_F4,RC_F5,
   RC_F6,RC_F7,RC_F8,RC_F9,RC_F10,RC_NUM,RC_ROLLEN,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, RC_STANDBY, 0x00, 0x00, 0x00, 0x00, '>'
};
int rcaltgrtable[] =
{
   0x00, RC_ESC, 0x00, '�', '�', 0x00, 0x00, 0x00, '{', '[', ']', '}', '\\', 0x00, 0x00, 0x00,
   '@',  0x00, '�', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, '~', RC_RET1, RC_STRG, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, RC_LSHIFT, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00,  0x00, '�', 0x00, 0x00, 0x00, RC_RSHIFT, 0x00, RC_ALT, 0x20, RC_CAPSLOCK,RC_F1,RC_F2,RC_F3,RC_F4,RC_F5,
   RC_F6,RC_F7,RC_F8,RC_F9,RC_F10,RC_NUM,RC_ROLLEN,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, RC_STANDBY, 0x00, 0x00, 0x00, 0x00, '|'
};

// kb codes

#define KBC_UP		0x01
#define KBC_DOWN	0x02
#define KBC_RIGHT	0x03
#define KBC_LEFT	0x04
#define KBC_INS		0x05
#define KBC_DEL		0x06
#define KBC_POS1	0x07
#define KBC_BACKSPACE	0x7F
#define KBC_END		0x0A
#define KBC_PAGEUP	0x0B
#define KBC_PAGEDOWN	0x0C
#define KBC_RETURN	0x0D

#define KBLCKFILE "/tmp/keyboard.lck"										//! file to lock keyboard-conversion


//freetype stuff
#define FONT FONTDIR "/neutrino.ttf"
// if font is not in usual place, we look here:
#define FONT2 FONTDIR "/pakenham.ttf"

enum {LANG_INT,LANG_DE, LANG_IT, LANG_SV, LANG_PT};
enum {RC_NORMAL,RC_EDIT};
enum {LEFT, CENTER, RIGHT};
enum {VERY_SMALL, SMALL, BIG};

FT_Library		library;
FTC_Manager		manager;
FTC_SBitCache		cache;
FTC_SBit		sbit;
FTC_ImageTypeRec	desc;
FT_Face			face;
FT_UInt			prev_glyphindex;
FT_Bool			use_kerning;



enum {OK, OKCANCEL, OKHIDDENCANCEL,YESNOCANCEL,NOBUTTON,OVERWRITECANCEL,OVERWRITESKIPCANCEL,CANCELRUN};
enum {YES, NO, HIDDEN,CANCEL, OVERWRITE, SKIP, OVERWRITEALL,SKIPALL,EDIT, RENAME, SEARCHRESULT, EDITOR};
enum {GZIP,BZIP2,COMPRESS,TAR,FTP};

#define _FONTHEIGHT_VERY_SMALL 20
#define _FONTHEIGHT_SMALL      24
#define _FONTHEIGHT_BIG        32
#define _FONT_OFFSET           5
#define _FONT_OFFSET_BIG       6
#define BORDERSIZE            5
int FONTHEIGHT_VERY_SMALL, FONTHEIGHT_SMALL, FONTHEIGHT_BIG, FONT_OFFSET, FONT_OFFSET_BIG;
//framebuffer stuff

enum {FILL, GRID};
enum {TRANSP, WHITE, BLACK, BLUE1, BLUE2, ORANGE, GREEN, YELLOW, RED, GRAY,GREEN2,GRAY2, BLUE_TRANSP, GRAY_TRANSP, BLUE3};

uint32_t *lfb = NULL, *lbb = NULL;

struct fb_fix_screeninfo fix_screeninfo;
struct fb_var_screeninfo var_screeninfo;

static uint32_t bgra[] = {
	0x00000000, 0xffffffff, 0xff000000, 0xff000080,
	0xff0080ff, 0xffffc000, 0xff00d000, 0xffe8e800,
	0xffff0000, 0xffb0b0b0, 0xff00ff00, 0xff505050,
	0xc0000080, 0xc0505050, 0xff0040ff };

#define CONFIG_FILE CONFIGDIR "/tuxcom.conf"


int trans_map     [] = {BLUE1,BLUE_TRANSP,TRANSP};
int trans_map_mark[] = {GRAY2,GRAY_TRANSP,GRAY_TRANSP};

unsigned short rccode;
char kbcode;

//some data

int avs, saa, fnc_old, saa_old, screenmode;
int rc, fb, kb;
int sx, ex, sy, ey;
int PosX, PosY, StartX, StartY, FrameWidth, NameWidth, SizeWidth;
int curframe, cursort, curvisibility, singleview, lastnoncur;
int tool[MENUITEMS*2];
int colortool[COLORBUTTONS];
int overwriteall, skipall;
int textuppercase;

int framerows;
int viewx;
int viewy;
int menuitemwidth;
int menuitemnumber;

char tmpzipdir[256];
char szClipboard[256];
char szSearchstring[FILENAME_MAX];
char szTextSearchstring[FILENAME_MAX];
char szPass[20];
long commandsize;


FILE *conf;
int language, langselect, autosave, filesize_in_byte;

#define ACTION_NOACTION 0
#define ACTION_PROPS    1
#define ACTION_RENAME   2
#define ACTION_VIEW     3
#define ACTION_EDIT     4
#define ACTION_COPY     5
#define ACTION_MOVE     6
#define ACTION_MKDIR    7
#define ACTION_DELETE   8
#define ACTION_MKFILE   9
#define ACTION_MKLINK   10

#define ACTION_EXEC      1
#define ACTION_MARKER    2
#define ACTION_SORT      3
#define ACTION_REFRESH   4
#define ACTION_DELLINE   5
#define ACTION_INSLINE   6
#define ACTION_CLEAR     7
#define ACTION_UPPERCASE 8
#define ACTION_LOWERCASE 9
#define ACTION_KILLPROC  10
#define ACTION_TOLINUX   11
#define ACTION_MARKTEXT  12
#define ACTION_INSTEXT   13




#define BTN_OK            0
#define BTN_CANCEL        1
#define BTN_HIDDEN        2
#define BTN_YES           3
#define BTN_NO            4
#define BTN_OVERWRITE     5
#define BTN_SKIP          6
#define BTN_OVERWRITEALL  7
#define BTN_SKIPALL       8
#define BTN_RENAME        9
#define BTN_ASK           10
#define BTN_AUTO          11
#define BTN_GERMAN        12
#define BTN_ENGLISH       13
#define BTN_ITALIAN       14
#define BTN_SWEDISH       15
#define BTN_PORTUGUES     16

#define SORT_UP    1
#define SORT_DOWN -1

#define SELECT_NOCHANGE 0
#define SELECT_UPDIR    1
#define SELECT_ROOTDIR  2

#define SHOW_NO_OUTPUT    0
#define SHOW_OUTPUT       1
#define SHOW_SEARCHRESULT 2

#define REPEAT_TIMER 3

#define INI_VERSION 1

#define NUM_LANG 5

#define MAINMENU 8

enum {MSG_EXEC              ,
      MSG_EXEC_NOT_POSSIBLE ,
      MSG_COPY              ,
      MSG_COPY_MULTI        ,
      MSG_COPY_PROGRESS     ,
      MSG_COPY_NOT_POSSIBLE ,
      MSG_MOVE              ,
      MSG_MOVE_MULTI        ,
      MSG_MOVE_PROGRESS     ,
      MSG_DELETE            ,
      MSG_DELETE_MULTI      ,
      MSG_DELETE_PROGRESS   ,
      MSG_RENAME            ,
      MSG_MKDIR             ,
      MSG_MKFILE            ,
      MSG_MKLINK            ,
      MSG_COMMAND           ,
      MSG_SAVE              ,
      MSG_FILE_EXISTS       ,
      MSG_LINE              ,
      MSG_READ_ZIP_DIR      ,
      MSG_EXTRACT           ,
      MSG_FTP_NOCONN        ,
      MSG_FTP_CONN          ,
      MSG_FTP_ERROR         ,
      MSG_FTP_READDIR       ,
      MSG_KILLPROC          ,
      MSG_PROCESSID         ,
      MSG_PROCESSUSER       ,
      MSG_PROCESSNAME       ,
      MSG_CANCELDOWNLOAD    ,
      MSG_APPENDDOWNLOAD    ,
      MSG_SEARCHFILES       ,
      MSG_SAVESETTINGS		};

enum {INFO_COPY     ,
      INFO_MOVE     ,
      INFO_EXEC     ,
      INFO_MARKER   ,
      INFO_PROC     ,
      INFO_PASS1    ,
      INFO_PASS2    ,
      INFO_PASS3    ,
      INFO_PASS4    ,
      INFO_SEARCH1  ,
      INFO_SEARCH2  ,
      INFO_SAVED    ,
      INFO_ACCESSED ,
      INFO_MODIFIED ,
      INFO_CREATED  ,
      INFO_DATETIME };


char *numberchars[] = {  "0 #!$%&?*()@\\",
                 		 "1/=<>+-_,.;:" ,
                 		 "abc2",
                 		 "def3",
                 		 "ghi4",
                 		 "jkl5",
                 		 "mno6",
                 		 "pqrs7",
                 		 "tuv8",
                 		 "wxyz9" };

char *info[]   = { "(select 'hidden' to copy in background)"               ,"('versteckt' w�hlen zum Kopieren im Hintergrund)"              ,"(Seleziona 'nascosto' per copiare in background)"              ,"(v�lj 'g�md' f�r att kopiera i bakgrunden)"        ,"(Seleccionar 'Escondido' para copiar em background)"  ,
                   "(select 'hidden' to move in background)"               ,"('versteckt' w�hlen zum Verschieben im Hintergrund)"           ,"(Seleziona 'nascosto' per muovere in background)"              ,"(v�lj 'g�md' f�r att flytta i bakgrund)"           ,"(Seleccionar 'Escondido' para mover em background)"   ,
                   "(select 'hidden' to execute in background)"            ,"('versteckt' w�hlen zum Ausf�hren im Hintergrund)"             ,"(Seleziona 'nascosto' per eseguire in background)"             ,"(v�lj 'hidden' f�r att starta i bakgrunden)"       ,"(Seleccionar 'Escondido' para executar em background)",
                   "selected:%d"                                           ,"markiert:%d"                                                   ,"Seleziona:%d"                                                  ,"vald:%d"                                           ,"Seleccionado:%d"                                      ,
				   "Warning: killing a process can make your box unstable!","Warnung: Prozesse beenden kann die Box instabil werden lassen!","Attenzione: fermare un processo pu� rendere il DB instabile!"  ,"Varning: d�da en process kan g�ra din box ostabil!","Matar um processo pode pox a BOX instavel"            ,
				   "Please enter your password"                            ,"Bitte Passwort eingeben"                                       ,"Per fovore inserire la password"                               ,"Skriv in ditt l�senord"                            ,"Por favor introduza password"                         ,
				   "Please enter new password"                             ,"Bitte neues Passwort eingeben"                                 ,"Per fovore inserire la nuova password"                         ,"Skriv in ditt nya l�senord"                        ,"Por favor imtroduza nova password"                    ,
				   "Please enter new password again"                       ,"Bitte neues Passwort wiederholen"                              ,"Per fovore inserire la nuova password di nuovo"                ,"Skriv in ditt nya l�senord igen"                   ,"Por favor introduza nova password outra vez"          ,
				   "password has been changed"                             ,"Passwort wurde ge�ndert"                                       ,"La password � stata cambiata"                                  ,"l�senordet har �ndrats"                            ,"Password foi alterada"                                ,
				   "searching..."							               ,"Suche l�uft..."                                                ,"Ricerca in corso..."                                           ,"S�ker..."                                          ,"Procurando..."                                        ,
				   "search result"									       ,"Suchergebnis"                                                  ,"Risultato della ricerca"                                       ,"S�kresultat"                                       ,"Resultado da pesquisa"                                ,
				   "settings saved"                                        ,"Einstellungen gespeichert"                                     ,"Impostazioni salvate"                                          ,"Inst�llningar sparade"                             ,"Gravar configuracoes"                                 ,
				   "last access"                                           ,"letzter Zugriff"                                               ,"last access"                                                   ,"Senast �ppnad"                                     ,"Ultimo acesso"                                        ,
				   "last modified"                                         ,"letzte �nderung"                                               ,"last modified"                                                 ,"Senast modifierad"                                 ,"Modificado a ultima vez"                              ,
				   "created"                                               ,"Erstellung"                                                    ,"created"                                                       ,"skapad"                                            ,"Criado"                                               ,
				   "%m/%d/%Y %H:%M:%S"                                     ,"%d.%m.%Y %H:%M:%S"                                             ,"%m/%d/%Y %H:%M:%S"                                             ,"%Y-%m-%d %H:%M:%S"                                 ,"%m/%d/%Y %H:%M:%S"                                    };

char *msg[]   = { "Execute '%s' ?"                             ,"'%s' ausf�hren ?"                                ,"Eseguire '%s'  ?"                                ,"Starta '%s' ?"                        ,"Executa '%s' ?"                                   ,
                  "Cannot execute file '%s'"                   ,"Kann '%s' nicht ausf�hren"                       ,"Impossibile eseguire il file '%s' "              ,"Kan inte starta fil"                  ,"Nao pode executar o ficheiro '%s'"                ,
                  "Copy '%s' to '%s' ?"                        ,"'%s' nach '%s' kopieren ?"                       ,"Copiare '%s' a '%s'  ?"                          ,"Kopiera '%s' till '%s'?"              ,"Copia '%s' para '%s' ?"                           ,
                  "Copy %d file(s) to '%s' ?"                  ,"%d Datei(en) nach '%s' kopieren ?"               ,"Copiare %d file in '%s'  ?"                      ,"Kopiera %d fil(er) till '%s'?"        ,"Copiar %d ficheiros '%s' para '%s' ?"             ,
                  "Copying file '%s' to '%s'..."               ,"kopiere '%s' nach '%s' ..."                      ,"Sto copiando file '%s' in '%s' ..."              ,"Kopierar filen '%s' till '%s'..."     ,"Copiando ficheiro '%s' para '%s'"                 ,
                  "Cannot copy to same Directory"              ,"kann nicht in das gleiche Verzeichnis kopieren"  ,"Impossibile copiare alla stessa directory"       ,"Kan inte kopiera till samma mapp"     ,"Nao pode copiar para a mesma Directoria"          ,
                  "Move '%s' to '%s' ?"                        ,"'%s' nach '%s' verschieben ?"                    ,"Muovere '%s' in '%s' ?"                          ,"Flytta '%s' till '%s' ?"              ,"Mover '%s' para '%s' ?"                           ,
				  "Move %d file(s) to '%s' ?"                  ,"%d Datei(en) nach '%s' verschieben ?"            ,"Muovere %d file in '%s' ?"                       ,"Flytta %d fil(er) till '%s' ?"        ,"Mover %d ficheiros para '%s' ..."                 ,
				  "Moving file '%s' to '%s'..."                ,"verschiebe '%s' nach '%s' ..."                   ,"Sto muovendo file '%s' in '%s' ..."              ,"Flyttar filen '%s' till '%s'..."      ,"Movendo os ficheiros '%s' para '%s' ..."          ,
				  "Delete '%s' ?"                              ,"'%s' l�schen ?"                                  ,"Cancellare '%s' ?"                               ,"Radera '%s' ?"                        ,"Apagando '%s' ?"                                  ,
				  "Delete %d files ?"                          ,"%d Datei(en) l�schen ?"                          ,"Cancellare i %d file ?"                          ,"Radera %d fil(er) ?"                  ,"Apagando %d ficheiros ?"                          ,
				  "Deleting file '%s'..."                      ,"l�sche Datei '%s' ..."                           ,"Sto cancellando i file '%s' ..."                 ,"Raderar filen '%s'..."                ,"Apagando ficheiros '%s' :"                        ,
				  "rename file '%s' :"                         ,"Datei '%s' umbenennen:"                          ,"Rinominare il file '%s' :"                       ,"Byt namn p� filen '%s'"               ,"Mudar nome do ficheiro '%s' :"                    ,
				  "create new directory in '%s'"               ,"neues Verzeichnis in '%s' erstellen"             ,"Creare una nuova directory nella directory '%s'" ,"Skapa ny mapp i mappen '%s'"          ,"Criar novo directorio en '%s'"                    ,
				  "create new file in directory '%s'"          ,"neue Datei in Verzeichnis '%s' erstellen"        ,"Creare un nuovo file '%s' nella directory"       ,"Skapa ny fil i mappen '%s'"           ,"Criar novo ficheiro no directorio '%s'"           ,
				  "create link to '%s%s\' in directory '%s'"   ,"Verweis auf '%s%s' in Verzeichnis '%s' erstellen","Creare un link a '%s%s' nella directory '%s' "   ,"Skapa l�nk till '%s%s\' i mappen '%s'","Criar uma ligacao para '%s%s\' no directorio '%s'",
				  "execute linux command"                      ,"Linux-Kommando ausf�hren"                        ,"Eseguire un comando linux"                       ,"Exekvera Linux kommando"              ,"Executar comando linux"                           ,
				  "save changes to '%s' ?"                     ,"�nderungen an '%s' speichern ?"                  ,"Salvare i cambiamenti a '%s' ?"                  ,"Spara �ndringar till '%s'?"           ,"Gravar alteracoes em '%s'"                        ,
				  "file '%s' already exists"                   ,"Datei '%s' existiert bereits"                    ,"Il file '%s' esiste gi�"                         ,"Filen '%s' finns redan"               ,"Ficheiro '%s' ja existe"                          ,
				  "line %d of %d%s"                            ,"Zeile %d von %d%s"                               ,"Linea %d di %d%s"                                ,"Linje %d av %d%s"                     ,"Linha %d de %d%s"                                 ,
				  "reading archive directory..."               ,"Lese Archiv-Verzeichnis..."                      ,"Sto leggendo la directory dell'archivio..."      ,"L�ser arkivmapp..."                   ,"Ler arquivo de directorio"                        ,
				  "extracting from file '%s'..."               ,"Entpacke aus Datei '%s'"                         ,"Sto estraendo dal file '%s'"                     ,"Extraherar fr�n filen '%s'..."        ,"Extrair do ficheiro '%s' ..."                     ,
				  "no connection to"                           ,"Keine Verbindung zu"                             ,"Nessuna connessione"                             ,"Ingen anslutning till"                ,"Nao ha ligacao para"                              ,
				  "connecting to"                              ,"Verbinde mit"                                    ,"Mi sto connettendo"                              ,"Ansluten till"                        ,"A ligar a"                                        ,
				  "error in ftp command '%s%s'"                ,"Fehler bei FTP-Kommando '%s%s'"                  ,"Errore nel comando ftp '%s%s'"                   ,"Fel i FTP-kommando '%s%s'"            ,"Erro no comando ftp '%s%s'"                       ,
				  "reading directory"                          ,"Lese Verzeichnis"                                ,"Sto leggendo la directory"                       ,"l�ser mappinformation"                ,"Ler directorio"                                   ,
				  "Do you really want to kill process '%s'?"   ,"Wollen sie wirklich den Prozess '%s' beenden?"   ,"Vuoi davvero fermare il processo '%s' ?"         ,"Vill du verkligen d�da process '%s'?" ,"Quer mesmo matar este processo '%s'"              ,
				  "process id"                                 ,"Prozess ID"                                      ,"ID processo"                                     ,"process ID"                           ,"ID do processo"                                   ,
				  "owner"                                      ,"Besitzer"                                        ,"Proprietario"                                    ,"�gare"                                ,"Proprietario"                                     ,
				  "process"                                    ,"Prozess"                                         ,"Processo"                                        ,"process"                              ,"Processo"                                         ,
				  "cancel download ?"                          ,"Download abbrechen ?"                            ,"Cancellare Download ?"                           ,"avbryt nedladdning ?"                 ,"Parar download ?"                                 ,
				  "append to file '%s' ?"                      ,"An Datei '%s' anh�ngen ?"                        ,"Aggiungere al file '%s' ?"                       ,"L�gg till i fil '%s' ?"               ,"Acrescentar ao ficheiro '%s' ?"                   ,
				  "search in directory %s for file:"           ,"In Verzeichnis %s suchen nach Datei:"            ,"Sto cercando il file %s:"                        ,"s�k i mappen %s efter fil:"           ,"Procurar no directorio %s pelo ficheiro:"         ,
				  "save current settings ?"                    ,"Einstellungen speichern ?"                       ,"Salvare le impostazioni correnti ?"              ,"spara nuvarande inst�llningar ?"      ,"Gravar configuracao corrente"                     };

char *menuline[]  = { ""      , ""       ,""      ,""       ,""      ,
                      "rights", "Rechte" ,"Attrib","r�tti." ,"Attrib",
                      "rename", "umben." ,"Rinom.","byt na.","Renom.",
                      "view"  , "Ansicht","Vedi"  ,"visa"   ,"Ver"   ,
                      "edit"  , "bearb." ,"Edita" ,"�ndra"  ,"Edita" ,
                      "copy"  , "kopier.","Copia" ,"kopiera","Copia" ,
                      "move"  , "versch.","Muovi" ,"flytta" ,"Mover" ,
                      "mkdir" , "mkdir"  ,"mkdir" ,"mkdir"  ,"mkdir" ,
                      "delete", "l�schen","Canc." ,"radera" ,"Apagar",
                      "touch" , "neu"    ,"Crea"  ,"touch"  ,"Cria"  ,
                      "link"  , "Verw."  ,"Link"  ,"l�nk"   ,"Ligar" };

char *editorline[]= { ""      , ""       ,""         ,""       ,""      ,
                      ""      , ""       ,""         ,""       ,""      ,
                      ""      , ""       ,""         ,""       ,""      ,
                      "mark"  , "mark."  ,"Seleziona","markera","Marcar",
                      ""      , ""       ,""         ,""       ,""      ,
                      "copy"  , "kopier.","Copia"    ,"kopiera","Copia" ,
                      "move"  , "versch.","Muovi"    ,"flytta" ,"mover" ,
                      ""      , ""       ,""         ,""       ,""      ,
                      "delete", "l�schen","Cancella" ,"radera" ,"Apagar",
                      ""      , ""       ,""         ,""       ,""      ,
                      ""      , ""       ,""         ,""       ,""      };

char *colorline[] = { ""               , ""                     ,""                ,""                 ,""                  ,
                      "execute command", "Kommando ausf�hren"   ,"Esegui comando"  ,"starta kommando"  ,"Executar comando"  ,
                      "toggle marker"  , "Datei markieren"      ,"Seleziona"       ,"v�xla markering"  ,"Seleccionar"       ,
                      "sort directory" , "Verzeichnis sortieren","Ordina directory","sortera mapp"     ,"Ordenar Directorio",
                      "refresh view"   , "Ansicht aktualisieren","Rivisualizza"    ,"uppdatera vy"     ,"Fazer refresh"     ,
                      "delete line"    , "Zeile l�schen"        ,"Cancella riga"   ,"radera linje"     ,"Apagar linha"      ,
                      "insert line"    , "Zeile einf�gen"       ,"Inserisci riga"  ,"l�gg till linje"  ,"Inserir linha"     ,
                      "clear input"    , "Eingabe l�schen"      ,"Cancella ins."   ,"rensa inmatning"  ,"Cancelar insercao" ,
                      "set uppercase"  , "Grossbuchstaben"      ,"Imposta su"      ,"s�tt versaler"    ,"Por letra grande"  ,
                      "set lowercase"  , "Kleinbuchstaben"      ,"Imposta gi�"     ,"s�tt gemener"     ,"Por letra pequena" ,
                      "kill process"   , "Prozess beenden"      ,"Ferma processo"  ,"d�da process"     ,"Matar processo"    ,
                      "to linux format", "in Linux-Format"      ,"A formato linux" ,"till Linux format","Formato linux"     ,
                      "mark text"      , "Text markieren"       ,"Marca testo"     ,"markera text"     ,"Marcar texto"      ,
                      "insert text"    , "Text einf�gen"        ,"Inserisci testo" ,"l�gg till text"   ,"inserir Texto"     };

char *mbox[]     = { "OK"           , "OK"                ,"OK"                ,"OK"             ,"OK"           ,
                     "Cancel"       , "Abbrechen"         ,"Annulla"           ,"Avbryt"         ,"Cancelar"     ,
                     "Hidden"       , "Versteckt"         ,"Nascosto"          ,"G�md"           ,"Esconder"     ,
                     "yes"          , "ja"                ,"Si"                ,"ja"             ,"Sim"          ,
                     "no"           , "nein"              ,"No"                ,"nej"            ,"Nao"          ,
                     "overwrite"    , "�berschr."         ,"Sovrascrivi"       ,"skriv �ver"     ,"Sobrepor"     ,
                     "skip"         , "�berspringen"      ,"Salta"             ,"hoppa �ver"     ,"Saltar"       ,
                     "overwrite all", "alle �berschreiben","Sovrascivi tutto"  ,"skriv �ver alla","Sobrepor tudo",
                     "skip all"     , "alle �berspringen" ,"Salta tutto"       ,"hoppa �ver alla","Saltar tudo"  ,
                     "rename"       , "umben."            ,"Rinomina"          ,"byt namn"       ,"Mudar nome"   ,
                     "ask"          , "nachfragen"        ,"Chiedi"            ,"fr�ga"          ,"Pergunta"     ,
                     "auto"			, "automatisch"       ,"automatico"        ,"auto"           ,"Auto"         ,
                     "Deutsch"      , "Deutsch"           ,"Deutsch"           ,"Deutsch"        ,"Deutsch"      ,
                     "english"      , "english"           ,"english"           ,"english"        ,"english"      ,
                     "Italiano"     , "Italiano"          ,"Italiano"          ,"Italiano"       ,"Italiano"     ,
                     "svenska"      , "svenska"           ,"svenska"           ,"svenska"        ,"svenska"      ,
                     "Portugues"    , "Portugues"         ,"Portugues"         ,"Portugues"      ,"Portugues"    };

char *props[]    = { "read"   , "lesen"    ,"Lettura"   ,"l�s"     ,"Ler"       ,
                     "write"  , "schreiben","Scrittura" ,"skriv"   ,"Escrever"  ,
                     "execute", "ausf�hren","Esecuzione","exekvera","Exececutar"};

char *ftpstr[]   = { "host"     , "Adresse"    ,"Host"       ,"serveraddress","Host"      ,
                     "port"     , "Port"       ,"Porta"      ,"port"         ,"Porta"     ,
                     "user"     , "Nutzer"     ,"Utente"     ,"anv�ndare"    ,"User"      ,
                     "password" , "Passwort"   ,"Password"   ,"l�senord"     ,"Password"  ,
                     "directory", "Verzeichnis","Directory"  ,"mapp"         ,"Directorio"};

char *mainmenu[] = { "search files"                       , "Dateien suchen"                            ,"Cerca file"                                ,"s�k filer"                           ,"Procurar ficheiros"                 ,
                     "taskmanager"                        , "Prozess�bersicht"                          ,"Taskmanager"                               ,"Process�versikt"                     ,"List de processos"                  ,
                     "toggle 16:9 mode"                   , "16:9-Modus setzen"                         ,"Passa a modalit� 16:9"                     ,"v�xla 16:9 l�ge"                     ,"Mudar para 16:9"                    ,
                     "set password"                       , "Passwort setzen"                           ,"Imposta password"                          ,"s�tt l�senord"                       ,"Por password"                       ,
                     "show filesizes in byte <%s>"        , "Dateigr�ssen in Byte anzeigen <%s>"        ,"show filesizes in byte <%s>"               ,"show filesizes in byte <%s>"         ,"show filesizes in byte <%s>"        ,
                     "language/Sprache/Lingua/Spr�k: <%s>", "Sprache/language/Lingua/Spr�k: <%s>"       ,"Lingua/language/Sprache/Spr�k: <%s>"       ,"Lingua/language/Sprache/Spr�k: <%s>" ,"Lingua/language/Sprache/Spr�k: <%s>",
                     "save settings on exit: <%s>"        , "Einstellungen beim Beenden speichern: <%s>","Salvare le impostazioni in uscita: <%s>"   ,"spara inst�llningar vid avslut: <%s>","Gravar e sair: <%s>"                ,
                     "save settings now"                  , "Einstellungen jetzt speichern"             ,"Salvare le impostazioni adesso"            ,"spara inst�llningar nu"              ,"Gravar configuracoes agora"         };

struct fileentry
{
    char name[256];
	struct stat   fentry;
};
struct zipfileentry
{
    char name[FILENAME_MAX];
	struct stat   fentry;
	struct zipfileentry * next;
};
struct marker
{
    char name[256];
	struct marker* next;
};


struct frameinfo
{
	char           			path[FILENAME_MAX];
	int            			writable;
	int			  			sort;
	int           			markcount;
	unsigned long long  	marksize;
	long          			first;
	long		  			selected;
	long			count;
	unsigned long long  	size;
	struct fileentry*		flist;
	struct marker * 		mlist;
	int						ziptype;
	char           			zipfile[FILENAME_MAX];
	char           			zippath[FILENAME_MAX];
	struct zipfileentry*	allziplist;
	FILE*                   ftpconn;
	struct sockaddr_in      s_in;
	char 					ftphost[512];
	int  					ftpport;
	char 					ftpuser[100];
	char 					ftppass[100];

};



struct frameinfo finfo[2];

//functions

void				SetPassword();
void 				RenderBox(int sx, int sy, int ex, int ey, int mode, int color);
void 	          	RenderFrame(int frame);
void 	          	RenderMenuLine(int highlight, int refresh);
void 	          	FillDir(int frame, int selmode);
struct fileentry* 	GetSelected(int frame);
void 				SetSelected(int frame, const char* szFile);
void 	          	GetSizeString(char* sizeString, unsigned long long size, int forcebytes);
int 	          	MessageBox(const char* msg1,const char* msg2, int mode);
int 	          	GetInputString(int width, int maxchars, char* str, char * msg, int pass);
void	          	ClearEntries(int frame);
void 				ClearZipEntries(int frame);
void	          	ClearMarker(int frame);
void	          	RenameMarker(int frame, const char* szOld, const char* szNew);
void	          	ToggleMarker(int frame);
int               	IsMarked(int frame, int pos);
int 			  	CheckOverwrite(struct fileentry* pfe, int mode, char* szNew);
void	          	ReadSettings();
void	          	WriteSettings();
void	          	DoExecute(char* szAction, int showoutput);
int 				DoCopy(struct fileentry* pfe, int typ, int checkmode, char* szZipCommand);
void 				DoZipCopyEnd();
int 				DoMove(struct fileentry* pfe, int typ, int checktype);
void	          	DoViewFile();
void	          	DoEditFile(char* szFile, char* szTitle, int writable);
void	          	DoTaskManager();
int			DoEditString(int x, int y, int width, unsigned int maxchars, char* str, int vsize, int back, int pass);
int 	          	ShowProperties();
void 		 	  	RenderButtons(int he, int mode);
int 			  	flistcmp(struct fileentry * p1, struct fileentry * p2);
struct fileentry* 	getfileentry(int frame, int pos);
struct fileentry* 	FindFile(int frame, const char* szFile);
void 			  	sortframe(int frame, char* szSel);
void 			  	ShowFile(FILE* pipe, char* szAction);
void 			  	ReadZip(int typ);
int					CheckZip(char* szName);
FILE*				OpenPipe(char* szAction);
void 				OpenFTP();
void 				ReadFTPDir(int frame, char* seldir);
int					FTPcmd(int frame, const char *s1, const char *s2, char *buf);
void 				DoEditFTP(char* szFile,char* szTitle);
void 				DoMainMenu();
void 				DoSearchFiles();
