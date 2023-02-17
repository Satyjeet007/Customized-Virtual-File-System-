
//****************************************************************
//                         Welcome To                            *
//                                                               *
//	        Customized Virtual File System Application           *
//                                                               *
//                                                               *
//****************************************************************


////////////////////////
//                 
//   Header Files
//
////////////////////////

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<iostream>

////////////////////////
//
// Defined The Macros
//
////////////////////////

#define MAXINODE 50   // Maximum Files To Be Created 50 (Inodes)

#define READ 1         
#define WRITE 2     // Give permission as 3 For Both Read & Write(1+2) (its like 0777)   

#define MAXFILESIZE 2048    // Maximum Size Of A File (2048 = 2kb)

#define REGULAR 1           // is Regular File (TXT File)
#define SPECIAL 2           

#define START 0              // File Offset(lseek)
#define CURRENT 1               
#define END 2


////////////////////////////////
//
// Created SuperBlock Structure 
//
////////////////////////////////

typedef struct superblock
{
    int TotalInodes;            // used to store count of total inodes                         
    int FreeInode;              // used to store how many inodes vacant(unused) are  
                                
}SUPERBLOCK , *PSUPERBLOCK;     


////////////////////////////////
//
// Created Inode Structure
//
////////////////////////////////

typedef struct inode        // 86 bytes allocated for this block
{
    char FileName[50];      // file name stored
    int InodeNumber;        //  inode number
    int FileSize;           //  2024 bytes 
    int FileActualSize;     //  allocated when we write into it
    int FileType;           //  type of file (TXT)
    char *Buffer;           
    int LinkCount;          //  linking count
    int ReferenceCount;     //  reference count
    int permission;         //  read + write permission
    struct inode *next;     //  self referential structure

}INODE , *PINODE , **PPINODE;


////////////////////////////////
//
// Created FileTable Structure
//
////////////////////////////////

typedef struct filetable
{
    int readoffset;     //  From Where To Read
    int writeoffset;    //  From Where To Write
    int count;
    int mode;           //  mode of the file (Read or Write or Read + Write)

    PINODE ptrinode;    // A Pointer Whitch Points to inode

}FILETABLE , *PFILETABLE;


////////////////////////////////////////////////////////
//
// Created UFDT Structure (User File Descriptor Table)
//
////////////////////////////////////////////////////////


typedef struct ufdt             
{
    PFILETABLE ptrfiletable;    // A Pointer Which Points To The File Table

}UFDT;

UFDT UFDTArr[50];               //UFDT (User File Descriptor Table) Array Of Pointers
SUPERBLOCK SUPERBLOCKobj;       // Global Object Of SuperBlock
PINODE head = NULL;             // Global pointer 


////////////////////////////////////////////////////////////////////////////////////////
//
//	Function Name	: 	man
//	Input			: 	char * 
//	Output			: 	None
//	Description 	: 	It Used To Display The Description For Each Commands
//	Author			: 	Satyjeet Patil
//	Date			:	27/01/2023
//
////////////////////////////////////////////////////////////////////////////////////////

void man(char *name)
{
    if(name == NULL) 
    {
        return;
    }

    if(strcmp(name,"create") == 0)
    {
        printf("Description : Used to create new regular file \n");
        printf("Usage : create File_Name Permission \n");
    }
    else if(strcmp(name,"read") == 0)
    {   
        printf("Description : Used to read data from regular file \n");
        printf("Usage : read File_Name No_Of_Bytes_To_Read \n");
    }
    else if(strcmp(name,"write") == 0)
    {   
        printf("Description : Used to write into regular file \n");
        printf("Usage : write File_Name\n After this enter the data that we want to write \n");
    }
    else if(strcmp(name,"ls") == 0)
    {
        printf("Description : Used to list all information of files \n");
        printf("Usage : ls \n");
    }
    else if(strcmp(name,"stat") == 0)
    {   
        printf("Description : Used to display information of file \n");
        printf("Usage : stat File_Name \n");
    }
    else if(strcmp(name,"fstat") == 0)
    {   
        printf("Description : Used to display information of file \n");
        printf("Usage : stat File_Descriptor\n \n");
    }
    else if(strcmp(name,"truncate") == 0)
    {   
        printf("Description : Used to remove data from file \n");
        printf("Usage : truncate File_Name \n");
    }
    else if(strcmp(name,"open") == 0)
    {   
        printf("Description : Used to open existing file \n");
        printf("Usage : open File_Name mode \n");
    }
    else if(strcmp(name,"close") == 0)
    {   
        printf("Description : Used to close opened file \n");
        printf("Usage : close File_Name \n");
    }
    else if(strcmp(name,"closeall") == 0)
    {   
        printf("Description : Used to close all opened file \n");
        printf("Usage : closeall \n");
    }
    else if(strcmp(name,"lseek") == 0)
    {   
        printf("Description : Used to change file offset \n");
        printf("Usage : lseek File_Name ChangeInOffset StartPoint \n");
    }
    else if(strcmp(name,"rm") == 0)
    {   
        printf("Description : Used to delete the file \n");
        printf("Usage : rm File_Name \n");
    }
    else
    {
        printf("ERROR : No manual entry available \n");
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Function Name	: 	DisplayHelp
//	Input			: 	None
//	Output			: 	None
//	Description 	: 	It Used To Display All List Of Commands + Some Information About The Commands
//	Author			: 	Satyjeet Patil
//	Date			:	27/01/2023
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DisplayHelp()
{
    printf("ls : To List Out All Files \n");
    printf("clear : To Clear Console \n");
    printf("open : To Open The File \n");
    printf("close : To Close The File \n");
    printf("closeall : To Close All Opened Files \n");
    printf("read : To Read The Contents From File \n");
    printf("write : To Write Contents Into File \n");
    printf("stat : To Display Information Of File Using Name \n");
    printf("fstat : To Display Information Of File Using File Descriptor \n");
    printf("truncate : To Remove All Data From File \n");
    printf("rm : To Delete The File \n");
    printf("exit : To Terminate File System \n");
}

int GetFDFromName(char *name)
{
    int i = 0;
    
    while(i < 50)
    {
        if(UFDTArr[i].ptrfiletable != NULL)
        {
            if(strcmp((UFDTArr[i].ptrfiletable->ptrinode->FileName),name) == 0)
            {
                break;
            }
        }   

        i++;
    }

    if(i == 50)
    {
        return -1;
    }
    else
    {
        return i;
    }
}

PINODE Get_Inode(char * name)
{
    PINODE temp = head;
    int i = 0;

    if(name == NULL)
    {
        return NULL;
    }

    while(temp != NULL)
    {
        if(strcmp(name,temp->FileName) == 0)
        {
            break;              //whitch is used to check wather same file name exitis or not
        }
        temp = temp->next;    
    }    
    return temp;
}

void CreateDILB()
{
    int i = 1;
    PINODE newn = NULL;
    PINODE temp = head;

    while(i <= MAXINODE)
    {
        newn = (PINODE)malloc(sizeof(INODE));       // 86 bytes allocated for this block (inode)

        newn->LinkCount = 0;
        newn->ReferenceCount = 0;
        newn->FileType = 0;                 // setting the defult value to the Some veriable of inode
        newn->FileSize = 0;

        newn->Buffer = NULL;
        newn->next = NULL;                  // Pointers to Defult value

        newn->InodeNumber = i;

        if(temp == NULL)
        {
            head = newn;
            temp = head;
        }
        else
        {
            temp->next = newn;
            temp = temp->next;            
        }
        i++;
    }
    printf("DILB created successfully \n");
}

void InitianliseSuperBlock()
{
    int i = 0;
    //           50
    while(i < MAXINODE)
    {
        UFDTArr[i].ptrfiletable = NULL; /////////////////
        i++;
    }

    SUPERBLOCKobj.TotalInodes = MAXINODE;
    SUPERBLOCKobj.FreeInode = MAXINODE;

    printf("Inside InitianliseSuperBlock \n");
}
//              Hello.txt           3
int CreateFile(char *name , int permission)
{
    int i = 0;          // you can set to 3 (specific reason)

    PINODE temp = head;

    if((name == NULL) || (permission == 0) || (permission > 3))
    {
        return -1;
    }

    if(SUPERBLOCKobj.FreeInode == 0)
    {
        return -2;
    }
    
    (SUPERBLOCKobj.FreeInode)--;

    if(Get_Inode(name) != NULL)
    {
        return -3;
    }

    while(temp != NULL)
    {
        if(temp->FileType == 0)
        {
            break;              // checking file type is 0 or not
        }                    
        temp = temp->next;            
    }

    while(i < 50)
    {
        if(UFDTArr[i].ptrfiletable == NULL)
        {
            break;                       // to check wather ptrfiletable pointer is NULL or not
        }                                
        i++;
    }

    UFDTArr[i].ptrfiletable = (PFILETABLE)malloc(sizeof(FILETABLE));    //Allocateing Memory For The FileTable(Fd)

    UFDTArr[i].ptrfiletable->count = 1;
    UFDTArr[i].ptrfiletable->mode = permission;
    UFDTArr[i].ptrfiletable->readoffset = 0;                            
    UFDTArr[i].ptrfiletable->writeoffset = 0;           //Initianliseing The File Table

    UFDTArr[i].ptrfiletable->ptrinode = temp;

    strcpy(UFDTArr[i].ptrfiletable->ptrinode->FileName,name);   //(strcpy)copies string2, including the ending null character, to the location that is specified by string1.
    UFDTArr[i].ptrfiletable->ptrinode->FileType = REGULAR;
    UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount = 1;
    UFDTArr[i].ptrfiletable->ptrinode->LinkCount = 1;           //ReInitianliseing The Veriable Of inode
    UFDTArr[i].ptrfiletable->ptrinode->FileSize = MAXFILESIZE;
    UFDTArr[i].ptrfiletable->ptrinode->FileActualSize = 0;
    UFDTArr[i].ptrfiletable->ptrinode->permission = permission;
    UFDTArr[i].ptrfiletable->ptrinode->Buffer = (char *)malloc(MAXFILESIZE);

    return i;
}
//  rm_File("Demo.txt")
int rm_File(char *name)
{
    int fd = 0;

    fd = GetFDFromName(name);
    if(fd == -1)
        return -1;

    (UFDTArr[fd].ptrfiletable->ptrinode->LinkCount)--;

    if(UFDTArr[fd].ptrfiletable->ptrinode->LinkCount == 0)
    {
        UFDTArr[fd].ptrfiletable->ptrinode->FileType = 0;
        //free(UFDTArr[fd].ptrfiletable->ptrinode->Buffer);
        free(UFDTArr[fd].ptrfiletable);
    }    

    UFDTArr[fd].ptrfiletable = NULL;
    (SUPERBLOCKobj.FreeInode)++;
    printf("File Successfully Deleted\n");
}

int ReadFile(int fd , char *arr , int isize)
{
    int read_size = 0;

    if(UFDTArr[fd].ptrfiletable == NULL)
    {
        return -1;
    }

    if(UFDTArr[fd].ptrfiletable->mode != READ && UFDTArr[fd].ptrfiletable->mode != READ+WRITE)
    {
        return -2;
    }

    if(UFDTArr[fd].ptrfiletable->ptrinode->permission != READ && UFDTArr[fd].ptrfiletable->ptrinode->permission 
    != READ + WRITE)
    {
        return -2;
    }    

    if(UFDTArr[fd].ptrfiletable->readoffset == UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)
    {
        return -3;
    }

    if(UFDTArr->ptrfiletable->ptrinode->FileType != REGULAR)
    {
        return -4;
    }

    read_size = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) - (UFDTArr[fd].ptrfiletable->readoffset);

    if(read_size < isize)
    {
        strncpy(arr,(UFDTArr[fd].ptrfiletable->ptrinode->Buffer) + (UFDTArr[fd].ptrfiletable->readoffset),read_size);

        UFDTArr[fd].ptrfiletable->readoffset = UFDTArr[fd].ptrfiletable->readoffset + read_size;
    }
    else
    {
        strncpy(arr,(UFDTArr[fd].ptrfiletable->ptrinode->Buffer) + (UFDTArr[fd].ptrfiletable->readoffset),isize);
        (UFDTArr[fd].ptrfiletable->readoffset) = (UFDTArr[fd].ptrfiletable->readoffset) + isize;
    }
    
    return isize;
}
///             fd       Data      Lenght of Data
int WriteFile(int fd , char *arr , int isize)
{
    if(((UFDTArr[fd].ptrfiletable->mode) != WRITE) && ((UFDTArr[fd].ptrfiletable->mode) != READ + WRITE))
    {
        return -1;
    }

    if(((UFDTArr[fd].ptrfiletable->ptrinode->permission) != WRITE) && ((UFDTArr[fd].ptrfiletable->ptrinode->permission)
    != READ + WRITE))
    {
        return -1;
    }

    if((UFDTArr[fd].ptrfiletable->writeoffset) == MAXFILESIZE)
    {
        return -2;
    }

    if((UFDTArr[fd].ptrfiletable->ptrinode->FileType) != REGULAR)
    {
        return -3;
    }

    strncpy((UFDTArr[fd].ptrfiletable->ptrinode->Buffer) + (UFDTArr[fd].ptrfiletable->writeoffset),arr,isize);       //copies count characters of string2 to string1

    (UFDTArr[fd].ptrfiletable->writeoffset) = (UFDTArr[fd].ptrfiletable->writeoffset) + isize;

    (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + isize;

    return isize; 
}
//           hello.txt        3
int OpenFile(char *name , int mode)
{
    int i = 0;
    PINODE temp = NULL;

    if(name == NULL || mode <= 0)
    {
        return -1;
    }    

    temp = Get_Inode(name);

    if(temp == NULL)
    {
        return -2;
    }

    if(temp->permission < mode)
    {
        return -3;
    }

    while(i < 50)
    {
        if(UFDTArr[i].ptrfiletable == NULL)
        {
            break;
        }
        i++;            
    }

    UFDTArr[i].ptrfiletable = (PFILETABLE)malloc(sizeof(FILETABLE));
    
    if(UFDTArr[i].ptrfiletable == NULL)
    {
        return -1;  
    } 
    
    UFDTArr[i].ptrfiletable->count = 1;
    UFDTArr[i].ptrfiletable->mode = mode;

    if(mode == READ + WRITE)
    {
        UFDTArr[i].ptrfiletable->readoffset = 0;
        UFDTArr[i].ptrfiletable->writeoffset = 0;
    }
    else if(mode == READ)
    {
        UFDTArr[i].ptrfiletable->readoffset = 0;
    }      
    else if(mode == WRITE)
    {
        UFDTArr[i].ptrfiletable->writeoffset = 0;
    }

    UFDTArr[i].ptrfiletable->ptrinode = temp;
    (UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)++;

    return i;
}

void CloseFileByName(int fd)
{
    UFDTArr[fd].ptrfiletable->readoffset = 0;
    UFDTArr[fd].ptrfiletable->writeoffset = 0;
    (UFDTArr[fd].ptrfiletable->ptrinode->ReferenceCount)--;
}

int CloseFileByName(char *name)
{
    int i = 0;

    i = GetFDFromName(name);

    if(i == -1)
        return -1;

    UFDTArr[i].ptrfiletable->readoffset = 0;
    UFDTArr[i].ptrfiletable->writeoffset = 0;
    (UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)--;    

    return 0;
}

void CloseAllFile()
{
    int i = 0;

    while(i < 50)
    {
        if(UFDTArr[i].ptrfiletable != NULL)
        {
            UFDTArr[i].ptrfiletable->readoffset = 0;
            UFDTArr[i].ptrfiletable->writeoffset = 0;
            (UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)--;
            break;
        }
        i++;
    }
}

int LseekFile(int fd , int size , int from)
{
    if((fd < 0) || (from > 2))  return -1;

    if(UFDTArr[fd].ptrfiletable == NULL)  return -1;

    if((UFDTArr[fd].ptrfiletable->mode == READ) || (UFDTArr[fd].ptrfiletable->mode == READ + WRITE))
    {
        if(from == CURRENT)
        {
            if(((UFDTArr[fd].ptrfiletable->readoffset) + size) > UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)
                return -1;

            if(((UFDTArr[fd].ptrfiletable->readoffset) + size) < 0) return -1;
            (UFDTArr[fd].ptrfiletable->readoffset) = (UFDTArr[fd].ptrfiletable->readoffset) + size;   
        }
    }
    else if(from == START)
    {
        if(size > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))   return -1;
        
        if(size < 0)  return -1;
        (UFDTArr[fd].ptrfiletable->readoffset) = size;
    }
    else if(from == END)
    {
        if((UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + size > MAXFILESIZE)
            return -1;

        if(((UFDTArr[fd].ptrfiletable->readoffset) + size) < 0) return -1;
        (UFDTArr[fd].ptrfiletable->readoffset) = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + size;    
    }
    else if(UFDTArr[fd].ptrfiletable->mode == WRITE)
    {
        if(from == CURRENT)
        {
            if(((UFDTArr[fd].ptrfiletable->writeoffset) + size ) > MAXFILESIZE) return -1;
            
            if(((UFDTArr[fd].ptrfiletable->writeoffset) + size ) < 0)   return -1;
            
            if(((UFDTArr[fd].ptrfiletable->writeoffset) + size) > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
                
                (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) = (UFDTArr[fd].ptrfiletable->writeoffset
                ) + size;
            (UFDTArr[fd].ptrfiletable->writeoffset) = (UFDTArr[fd].ptrfiletable->writeoffset)+ size;    
        }
        else if(from == START)
        {
            if(size > MAXFILESIZE)   return -1;
            if(size < 0)   return -1;
            if(size > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
                (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) = size;
            (UFDTArr[fd].ptrfiletable->writeoffset) = size;    
        }
        else if(from == END)
        {
            if((UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + size > MAXFILESIZE)   return -1;
            if(((UFDTArr[fd].ptrfiletable->writeoffset) + size ) < 0) return -1;
            (UFDTArr[fd].ptrfiletable->writeoffset) = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + size;
        }
    }
    printf("Successfully Changed\n");
}

void Is_file()
{
    int i = 0;
    PINODE temp = head;

    if(SUPERBLOCKobj.FreeInode == MAXINODE)
    {
        printf("Error : There are no files\n");
        return;
    }

    printf("\nFile Name\tInode number\tfile size\tLink count\n");
    printf("-------------------------------------------------------\n");
    
    while(temp != NULL)
    {
        if(temp->FileType != 0)
        {
            printf("%s\t\t%d\t\t%d\t\t%d\n",temp->FileName,temp->InodeNumber,temp->FileActualSize,temp->LinkCount);
        }
        temp = temp->next;
    }
    
    printf("\n-------------------------------------------------------\n");
}

int fstat_file(int fd)
{
    PINODE temp = head;
    int i = 0;

    if(fd < 0)
    {
        return -1;
    }    

    if(UFDTArr[fd].ptrfiletable == NULL)
    {
        return -2;
    }    

    temp = UFDTArr[fd].ptrfiletable->ptrinode;

    printf("\n--------Statistical Information about file--------\n\n");
    printf("File name : %s\n",temp->FileName);
    printf("Inode Number %d\n",temp->InodeNumber);
    printf("File size : %d\n",temp->FileSize);
    printf("Actual File size : %d\n",temp->FileActualSize);
    printf("Link count : %d\n",temp->LinkCount);
    printf("Reference count : %d\n",temp->ReferenceCount);

    if(temp->permission == 1)
    {
        printf("File Permission : Read only\n");
    }
    else if(temp->permission == 2)
    {
        printf("File Permission : Read & Write \n");   
    }
    else if(temp->permission == 3)
    {     
        printf("File Permission : Read & Write\n");
    }                
    printf("\n---------------------------------------------------\n");   

    return 0;      
}

int stat_file(char *name)
{
    PINODE temp = head;
    int i = 0;

    if(name == NULL)
    {
        return -1;
    }    

    while(temp != NULL)
    {
        if(strcmp(name,temp->FileName) == 0)
        {
            break;
        }
        temp = temp->next;    
    }

    if(temp == NULL)
    {
        return -2;
    }

    printf("\n--------Statistical Information about file--------\n\n");
    printf("File name : %s\n",temp->FileName);
    printf("Inode Number %d\n",temp->InodeNumber);
    printf("File size : %d\n",temp->FileSize);
    printf("Actual File size : %d\n",temp->FileActualSize);
    printf("Link count : %d\n",temp->LinkCount);
    printf("Reference count : %d\n",temp->ReferenceCount);

    if(temp->permission == 1)
    {
        printf("File Permission : Read only\n");
    }
    else if(temp->permission == 2)
    {
        printf("File Permission : Read \n");
    }
    else if(temp->permission == 3)
    {     
        printf("File Permission : Read & Write\n");
    }            
    printf("\n---------------------------------------------------\n");   

    return 0;
}

int truncate_File(char *name)
{
    int fd = GetFDFromName(name);

    if(fd == -1)
        return -1;

    memset(UFDTArr[fd].ptrfiletable->ptrinode->Buffer , 0 , 1024);    // File Size Can be Change
    UFDTArr[fd].ptrfiletable->readoffset = 0;
    UFDTArr[fd].ptrfiletable->writeoffset = 0;
    UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize = 0;    
}

int main()
{
    char *ptr = NULL;           
    int ret = 0 , fd = 0 , count = 0;
    char command[4][80] , str[80] , arr[MAXFILESIZE];

    InitianliseSuperBlock();
    CreateDILB();        //DILB (Disk inode list block) 
    
    //Shell
    while(1)    //Infinite Listening Loop  (Firatach Rahanara , Aik Nar Ra , Gool Gool Firnara)
    {
        fflush(stdin);  //Used To Flush The Data(clear Input Buffer)
        strcpy(str,"");  // Used To cleen the data

        printf("\nMarvellous VFS : > ");

        //   kasat kiti  kuthe
        fgets(str , 80 , stdin);    // we can also use scanf("%[^'\n']s",str);

        //return value of sscanf is how many tokens are there
        count = sscanf(str,"%s %s %s %s" ,command[0],command[1],command[2],command[3]);     //Tokenization(sequence of strings into pieces such as words)
        
        if(count == 1)
        {
            if(strcmp(command[0] , "ls") == 0)      
            {
                Is_file();
            }
            else if(strcmp(command[0] , "closeall") == 0)
            {
                CloseAllFile();
                printf("All files closed successfully \n");
                continue;
            }
            else if(strcmp(command[0] , "clear") == 0)
            {
                system("clear");  //Used To Clear The Console
                continue;
            }
            else if(strcmp(command[0] , "help") == 0)
            {
                DisplayHelp();
                continue;
            }
            else if(strcmp(command[0] , "exit") == 0)
            {
                printf("Terminating The Customized Virtual File System \n");
                break;
            }
            else
            {
                printf("\nERROR : Command not found !!!!\n");
                continue;
            }
        }
        else if(count == 2)
        {
            if(strcmp(command[0] , "stat") == 0)
            {
                ret == stat_file(command[1]);
                if(ret == -1)
                {
                   printf("ERROR : Incorrect parameters\n");
                    continue; 
                }
                
                if(ret == -2)
                {
                    printf("ERROR : There is no such file\n");
                    continue;
                }
            }
            else if(strcmp(command[0] , "fstat") == 0)
            {
                ret = fstat_file(atoi(command[1]));             //atoi(Converts A Character String To An Integer Value)
                
                if(ret == -1)
                {
                    printf("ERROR : Incorrect parameters\n");
                    continue;
                }    
                
                if(ret == -2)
                {
                    printf("ERROR : There is no such file\n");
                    continue;
                }   
            }
            else if(strcmp(command[0] , "close") == 0)
            {
                ret = CloseFileByName(command[1]);
                
                if(ret == -1)
                {
                    printf("ERROR : There is no such file\n");
                    continue;
                }    
            }
            else if(strcmp(command[0] , "rm") == 0)
            {
                ret = rm_File(command[1]);
                
                if(ret == -1)
                {
                    printf("ERROR : There is no such file\n");
                    continue;
                }    
            }
            else if(strcmp(command[0] , "man") == 0)
            {
                man(command[1]);
                continue;           
            }
            else if(strcmp(command[0] , "write") == 0)
            {
                fd = GetFDFromName(command[1]);
                
                if(fd == -1)
                {
                    printf("ERROR : Incorrect parameters\n");
                    continue;
                }

                printf("Enter The Data : \n");
                scanf("%[^\n]",arr);

                ret = strlen(arr);
                
                if(ret == 0)
                {
                    printf("ERROR : Incorrect parameters\n");
                    continue;
                }

                ret = WriteFile(fd,arr,ret);
                
                if(ret == -1)
                {
                    printf("ERROR : Permission denied\n");
                }   
    
                if(ret == -2)
                {
                    printf("ERROR : There is no sufficient memory to write \n");
                }   
                
                if(ret == -3)
                {
                    printf("ERROR : It is not regular file \n");
                }
                    
                if(ret > 0)
				{
					printf("Sucessfully : %d bytes written\n", ret);
				}
                
            }
            else if(strcmp(command[0] , "truncate") == 0)
            {
                ret = truncate_File(command[1]);
                
                if(ret == -1)
                {
                    printf("ERROR : Incorrect parameters\n");
                }   
                
            }
            else
            {
                printf("\nERROR : Command not found !!!!\n");
                continue;
            }
        }
        else if(count == 3)
        {
            if(strcmp(command[0] , "create") == 0)      // return value of strcmp is 0 if the two strings are equal
            {   
                // ret = CreateFile(Hello.txt , 3);
                ret = CreateFile(command[1] , atoi(command[2]));      //atoi(Converts A Character String To An Integer Value)
                
                if(ret >= 0)
                {
                    printf("File is successfully created with file descriptor : %d\n",ret);
                }
                if(ret == -1)
                {
                    printf("ERROR : Incorrect parameters\n");
                }
                if(ret == -2)
                {
                    printf("ERROR : There is no inodes\n");
                }   
                if(ret == -3)
                {
                    printf("ERROR : File already exists\n");
                }
                if(ret == -4)
                {
                    printf("ERROR : Memory allocation failure\n");
                }
                continue;
            }
            else if(strcmp(command[0] , "open") == 0)
            {
                ret = OpenFile(command[1] , atoi(command[2]));
                
                if(ret >= 0) 
                {
                    printf("File is successfully opened with file descriptor : %d\n",ret);
                }
                if(ret == -1)
                {
                    printf("ERROR : Incorrect parameters\n");
                }
                if(ret == -2)
                {
                    printf("ERROR : File not present\n");
                }
                if(ret == -3)
                {
                    printf("ERROR : Permission denied\n");
                }
                continue;
            }
            else if(strcmp(command[0] , "read") == 0)
            {
                fd = GetFDFromName(command[1]);
                
                if(fd == -1)
                {
                    printf("ERROR : Incorrect parameters\n");
                    continue;
                }

                ptr = (char *)malloc(sizeof(atoi(command[2])) +1);

                if(ptr == NULL)
                {
                    printf("ERROR : Memory allocation failure\n");
                    continue;
                }

                ret = ReadFile(fd , ptr , atoi(command[2]));

                if(ret == -1)
                {
                    printf("ERROR : File not existing\n");
                }
                if(ret == -2)
                {
                    printf("ERROR : Permission denied\n");
                }
                if(ret == -3)
                {
                    printf("ERROR : Reached at end of file\n");
                }
                if(ret == -4)
                {
                    printf("ERROR : It is not regular file\n");
                }
                if(ret == -5)
                {
                    printf("ERROR : File is not opened\n");
                }
                if(ret == 0)
                {
                    printf("ERROR : File empty\n");
                }
                if(ret > 0)
                {
                    write(2,ptr,ret);
                }
                continue;    
            }
            else
            {
                printf("\nERROR : Command not found !!!!\n");
                continue;
            }
        }
        else if(count == 4)
        {
            if(strcmp(command[0] , "lseek") == 0)
            {
                fd = GetFDFromName(command[1]);
                if(fd == -1)
                {                    
                    printf("ERROR : Incorrect parameters\n");
                    continue;
                }

                ret = LseekFile(fd , atoi(command[2]) , atoi(command[3]));

                if(ret == -1)
                {
                    printf("ERROR : Unable to perform lseek\n");
                }
                if(ret == -2)
                {
                    printf("ERROR : File is not opened\n");
                }
            }
            else
            {
                printf("\nERROR : Command not found !!!!\n");
                continue;
            }
        }
        else
        {
            printf("\nERROR : Command not found !!!!\n");
            continue;
        } 

    }//End of while loop

    return 0;       

}//End of main funcation  