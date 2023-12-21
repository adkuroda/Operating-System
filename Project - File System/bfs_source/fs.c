// ============================================================================
// fs.c - user FileSytem API
// Created by: Adilet Kuroda and Nathaniel Fincham
// Test cases we created are commented out. In case you want to run them,
// uncomment them and run the test program. 
// There is a script file called run.sh that runs all the test cases. At the end 
// of the script, it removes the BFSDISK and copies the original BFSDISK to the
// current directory so that the next time you run the test program, it will
// clear the BFSDISK and start from the beginning.
// ============================================================================

#include "bfs.h"
#include "fs.h"

// ============================================================================
// Close the file currently open on file descriptor 'fd'.
// ============================================================================
i32 fsClose(i32 fd) { 
  i32 inum = bfsFdToInum(fd);
  bfsDerefOFT(inum);
  return 0; 
}



// ============================================================================
// Create the file called 'fname'.  Overwrite, if it already exsists.
// On success, return its file descriptor.  On failure, EFNF
// ============================================================================
i32 fsCreate(str fname) {
  i32 inum = bfsCreateFile(fname);
  if (inum == EFNF) return EFNF;
  return bfsInumToFd(inum);
}



// ============================================================================
// Format the BFS disk by initializing the SuperBlock, Inodes, Directory and 
// Freelist.  On succes, return 0.  On failure, abort
// ============================================================================
i32 fsFormat() {
  FILE* fp = fopen(BFSDISK, "w+b");
  if (fp == NULL) FATAL(EDISKCREATE);

  i32 ret = bfsInitSuper(fp);               // initialize Super block
  if (ret != 0) { fclose(fp); FATAL(ret); }

  ret = bfsInitInodes(fp);                  // initialize Inodes block
  if (ret != 0) { fclose(fp); FATAL(ret); }

  ret = bfsInitDir(fp);                     // initialize Dir block
  if (ret != 0) { fclose(fp); FATAL(ret); }

  ret = bfsInitFreeList();                  // initialize Freelist
  if (ret != 0) { fclose(fp); FATAL(ret); }

  ret = bfsInitOFT();                  	   // initialize OFT
  if (ret != 0) { fclose(fp); FATAL(ret); }

  fclose(fp);
  return 0;
}


// ============================================================================
// Mount the BFS disk.  It must already exist
// ============================================================================
i32 fsMount() {
  FILE* fp = fopen(BFSDISK, "rb");
  if (fp == NULL) FATAL(ENODISK);           // BFSDISK not found
  fclose(fp);
  return 0;
}



// ============================================================================
// Open the existing file called 'fname'.  On success, return its file 
// descriptor.  On failure, return EFNF
// ============================================================================
i32 fsOpen(str fname) {
  i32 inum = bfsLookupFile(fname);        // lookup 'fname' in Directory
  if (inum == EFNF) return EFNF;
  return bfsInumToFd(inum);
}



// ============================================================================
// Read 'numb' bytes of data from the cursor in the file currently fsOpen'd on
// File Descriptor 'fd' into 'buf'.  On success, return actual number of bytes
// read (may be less than 'numb' if we hit EOF).  On failure, abort
// ============================================================================
i32 fsRead(i32 fd, i32 numb, void* buf) {
  // printf("numb %d\n", numb);
  // Bound check; assume file is opened properly
  if (numb < 0) FATAL(ENEGNUMB);
  // get the inode number
  i32 inum = bfsFdToInum(fd);
  // get the current position of the cursor on the file
  i32 cur_pos = fsTell(fd); 
  // check if the cursor is out of bound
  if (cur_pos < 0) FATAL(EBADCURS);
  // check if past the end of the file
  if (cur_pos > fsSize(fd)) FATAL(EBADCURS);
  // ret is the number of bytes we can read
  int ret = numb;
  if (ret + cur_pos > fsSize(fd)){
    ret = fsSize(fd) - cur_pos;
    numb = fsSize(fd) - cur_pos;
  }
  // to offset the cursor after reading
  i32 offset = ret + cur_pos;
  // to obtain starting block number 
  i32 fnb = cur_pos / BYTESPERBLOCK; 
  // to obtain the starting byte number for reading
  i32 start_byte = cur_pos % BYTESPERBLOCK; 
  // this will temporarily store the data read from the disk
  i8 temp[BYTESPERBLOCK];
  // this is a pointer to the temp array
  i8 * temp_prt = (i8*)temp;
  // this is a pointer to the buf array to help copy the data
  i8 * buf_prt = (i8*)buf;
  while(fnb < MAXFBN && numb > 0){
    //printf("reading nunb %d\n", numb);
    // read entire block
    bfsRead(inum, fnb, temp);
    // if the reading does not start from the beginning of the block
    if (start_byte != 0){
      // In case when the number of bytes to read is less than 
      //the number of bytes left in the block
      if (numb < BYTESPERBLOCK - start_byte){
        // copy the data from the temp array to the buf array
        memcpy(buf_prt, temp_prt + start_byte, numb);
        numb = 0;
      }
      else{
        // copy the data from the temp array to the buf array
        memcpy(buf_prt, temp_prt + start_byte, BYTESPERBLOCK - start_byte);
        buf_prt += BYTESPERBLOCK - start_byte;
        numb -= BYTESPERBLOCK - start_byte;
        start_byte = 0;
      }
    }
    else{
      // In case when the number of bytes to read is less than
      // the number of bytes in the block
      if (numb < BYTESPERBLOCK){
        // copy the data from the temp array to the buf array
        memcpy(buf_prt, temp_prt, numb);
        numb = 0;
      }
      else{
        // copy the data from the temp array to the buf array
        memcpy(buf_prt, temp_prt, BYTESPERBLOCK);
        buf_prt += BYTESPERBLOCK;
        numb -= BYTESPERBLOCK;
      }
    }
    fnb ++;
    // reset the temp pointer
    temp_prt = (i8*)temp;
  }
  // update the cursor position
  fsSeek(fd, offset, SEEK_SET);
  //printf("ret = %d\n", ret);
  return ret;
  //FATAL(ENYI);                                  // Not Yet Implemented!
}


// ============================================================================
// Move the cursor for the file currently open on File Descriptor 'fd' to the
// byte-offset 'offset'.  'whence' can be any of:
//
//  SEEK_SET : set cursor to 'offset'
//  SEEK_CUR : add 'offset' to the current cursor
//  SEEK_END : add 'offset' to the size of the file
//
// On success, return 0.  On failure, abort
// ============================================================================
i32 fsSeek(i32 fd, i32 offset, i32 whence) {

  if (offset < 0) FATAL(EBADCURS);
 
  i32 inum = bfsFdToInum(fd);
  i32 ofte = bfsFindOFTE(inum);
  
  switch(whence) {
    case SEEK_SET:
      g_oft[ofte].curs = offset;
      break;
    case SEEK_CUR:
      g_oft[ofte].curs += offset;
      break;
    case SEEK_END: {
        i32 end = fsSize(fd);
        g_oft[ofte].curs = end + offset;
        break;
      }
    default:
        FATAL(EBADWHENCE);
  }
  return 0;
}



// ============================================================================
// Return the cursor position for the file open on File Descriptor 'fd'
// ============================================================================
i32 fsTell(i32 fd) {
  return bfsTell(fd);
}



// ============================================================================
// Retrieve the current file size in bytes.  This depends on the highest offset
// written to the file, or the highest offset set with the fsSeek function.  On
// success, return the file size.  On failure, abort
// ============================================================================
i32 fsSize(i32 fd) {
  i32 inum = bfsFdToInum(fd);
  return bfsGetSize(inum);
}



// ============================================================================
// Write 'numb' bytes of data from 'buf' into the file currently fsOpen'd on
// filedescriptor 'fd'.  The write starts at the current file offset for the
// destination file.  On success, return 0.  On failure, abort
// ============================================================================

i32 fsWrite(i32 fd, i32 numb, void* buf) {
  if (numb < 0){
    FATAL(ENEGNUMB);
  }
  // get the current position of the cursor on the file
  i32 cur_pos = fsTell(fd);
  // get the inode number
  i32 inum = bfsFdToInum(fd);
  // get the size of the file
  i32 size = fsSize(fd);
  i32 diff = 0;
  // determine the difference between the current position and the size of the file
  if (cur_pos > size){
    diff = cur_pos - size;
  }
  // to set the cursor once the writing is done
  i32 offset = cur_pos + numb;
  // to obtain the last block number
  i32 fnb_last = (size + 1)/BYTESPERBLOCK;
  // to obtain the current block number
  i32 fnb_cur = cur_pos/BYTESPERBLOCK;
  // to obtain the number of blocks needed to write
  i32 num_block = (numb + diff - 1)/BYTESPERBLOCK + 1;
  // to extend the file if needed
  if(fnb_cur + num_block > fnb_last){
    bfsExtend(inum, fnb_last+ num_block);
    bfsSetSize(inum, offset);
  }else{
    if(cur_pos + numb > size){
      bfsSetSize(inum, cur_pos + numb);
    }
  }
  // to obtain the starting byte number for writing
  i32 start_byte = cur_pos % BYTESPERBLOCK;
  // to obtain the number of bytes left in the block
  i32 count = numb;
  // this is a pointer to the buf array to help copy the data
  i8 * buf_start = (i8*)buf;
  //printf("cur_pos %d\n", cur_pos);
  while(count > 0){
    //printf("count %d\n", count);
    i8 temp[BYTESPERBLOCK];
    // to ensure that only the bytes left in the block are written
    bfsRead(inum, fnb_cur, temp);
    // in case when the number of bytes to write is less than the number of 
    //bytes left in the block
    if (count < BYTESPERBLOCK){
      // to check if the writing does not start from the beginning of the block
      if (start_byte != 0){
        memcpy(temp + start_byte, buf_start, count);
      }
      // only write certain number of bytes
      else{
        memcpy(temp, buf_start, count);
        buf_start += count;
      }
      // obtain the disk block number
      i32 dbn = bfsFbnToDbn(inum, fnb_cur);
      // write the data to the disk
      bioWrite(dbn, temp);
      // update the number of bytes left to write
      count = 0;
    }
    else{
      // to check if the writing does not start from the beginning of the block
      if (start_byte != 0){
        memcpy(temp + start_byte, buf_start, BYTESPERBLOCK - start_byte);
        count -= BYTESPERBLOCK;
        count += start_byte;
        buf_start += BYTESPERBLOCK;
        buf_start -= start_byte;
        start_byte = 0;
      }
      else{
        memcpy(temp, buf_start, BYTESPERBLOCK);
        buf_start += BYTESPERBLOCK;
        count -= BYTESPERBLOCK;
      }
      // obtain the disk block number
      i32 dbn = bfsFbnToDbn(inum, fnb_cur);
      // write the data to the disk
      bioWrite(dbn, temp);
      // update the number of bytes left to write
    }
    // update the file block number
    fnb_cur ++;
  } 
  // update the cursor position
  fsSeek(fd, offset, SEEK_SET);
  return 0;
}
