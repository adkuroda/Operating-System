
//============================================================================
// p5test.c : use regular C library calls to check what the answers should be
// when run against the BFS filesystem
// ============================================================================

#include "p5test.h"

// ============================================================================
// Check that 'size' bytes, starting at buf[start] hold the value 'val'.
// 'testnum' is the test number - used for reporting
// ============================================================================
void check(int testnum, i8* buf, int start, int size, int val) {
  for (int i = start; i < start + size; ++i) {
    if (buf[i] != val) {
      printf("TEST %d : BAD  : buf[%d] = %d but should be %d \n", 
        testnum, i, buf[i], val);
      return;
    }
  }
  printf("TEST %d : GOOD \n", testnum);
}



// ============================================================================
// Check that 'actual' == 'expected' for test 'testnum'
// ============================================================================
void checkCursor(int testnum, int expected, int actual) {
  if (actual == expected) {
    printf("TEST %d : GOOD \n", testnum);
  } else {
    printf("TEST %d : BAD  : cursor = %d but should be %d \n", 
        testnum, actual, expected);
  }
}



// ============================================================================
// Create file "P5", holding 50 blocks, inside of BFSDISK, and populate
// ============================================================================
void createP5() {

  i32 fd = fsCreate("P5");

  i8 buf[BYTESPERBLOCK];

  // Write 100 blocks.  Every byte in block 'b' the value 'b'

  for (int b = 0; b < 50; ++b) {
    memset(buf, b, BYTESPERBLOCK);
    fsWrite(fd, BYTESPERBLOCK, buf);
  }

  fsClose(fd);
}



// ============================================================================
// TEST 1 : Small read (100 bytes) from cursor = 0
// ============================================================================
void test1(i32 fd) {
  i8 buf[BUFSIZE];                  // buffer for reads and writes

  fsSeek(fd, 0, SEEK_SET);     

  i32 curs = fsTell(fd);
  checkCursor(1, 0, curs);

  memset(buf, 0, BUFSIZE);
  i32 ret = fsRead(fd, 100, buf);   // read 100 bytes from cursor = 0
  assert(ret == 100);

  curs = fsTell(fd);
  checkCursor(1, 100, curs);

  check(1, buf, 0, 100, 0);
}


// ============================================================================
// TEST 2 : Small read (200 bytes) from 30 bytes into block 1
// ============================================================================
void test2(i32 fd) {
  i8 buf[BUFSIZE];                  // buffer for reads and writes

  fsSeek(fd, 512 + 30, SEEK_SET);     

  i32 curs = fsTell(fd);
  checkCursor(2, 512 + 30, curs);

  memset(buf, 0, BUFSIZE);
  i32 ret = fsRead(fd, 200, buf);   // read 200 bytes from current cursor
  assert(ret == 200);

  curs = fsTell(fd);
  checkCursor(2, 512 + 30 + 200, curs);

  check(2, buf, 0, 200, 1);
}



// ============================================================================
// TEST 3 : Large, spanning read (1,000 bytes) from start of block 20
//          512*20, 488*21
// ============================================================================
void test3(i32 fd) {
  i8 buf[BUFSIZE];                  // buffer for reads and writes

  fsSeek(fd, 20 * BYTESPERBLOCK, SEEK_SET);     

  i32 curs = fsTell(fd);
  checkCursor(3, 20 * 512, curs);

  memset(buf, 0, BUFSIZE);
  i32 ret = fsRead(fd, 1000, buf);  // read 1,000 bytes from current cursor
  assert(ret == 1000);

  curs = fsTell(fd);
  checkCursor(3, 20 * 512 + 1000, curs);

  check(3, buf,   0, 512, 20);
  check(3, buf, 512, 488, 21);
}


// ============================================================================
// TEST 4 : Small write (77 bytes) starting at 10 bytes into block 7
//          10*7, 77*77, 425*7
// ============================================================================
void test4(i32 fd) {
  i8 buf[BUFSIZE];                  // buffer for reads and writes

  fsSeek(fd, 7 * BYTESPERBLOCK + 10, SEEK_SET);     

  i32 curs = fsTell(fd);
  checkCursor(4, 7 * 512 + 10, curs);

  memset(buf, 0, BUFSIZE);
  memset(buf, 77, 77);
  
  fsWrite(fd, 77, buf);

  curs = fsTell(fd);
  checkCursor(4, 7 * 512 + 10 + 77, curs);

  fsSeek(fd, 7 * BYTESPERBLOCK, SEEK_SET);     

  i32 ret = fsRead(fd, BYTESPERBLOCK, buf);
  assert(ret == BYTESPERBLOCK);

  check(4, buf, 0,  10,  7);
  check(4, buf, 10, 77,  77);
  check(4, buf, 87, 425, 7);   
}



// ============================================================================
// TEST 5 : Large, spanning write (900 bytes) starting at 50 bytes into 
//          block 10
//          50*10, 462*88, 438*88, 74*11
// ============================================================================
void test5(i32 fd) {
  i8 buf[BUFSIZE];                  // buffer for reads and writes

  fsSeek(fd, 10 * BYTESPERBLOCK + 50, SEEK_SET);     

  i32 curs = fsTell(fd);
  checkCursor(5, 10 * 512 + 50, curs);

  memset(buf,  0, BUFSIZE);
  memset(buf, 88, 900);
  
  fsWrite(fd, 900, buf);

  curs = fsTell(fd);
  checkCursor(5, 10 * 512 + 50 + 900, curs);

  fsSeek(fd, 10 * BYTESPERBLOCK, SEEK_SET);     

  curs = fsTell(fd);
  checkCursor(5, 10 * 512, curs);

  i32 ret = fsRead(fd, 2 * BYTESPERBLOCK, buf);
  assert(ret == 2 * BYTESPERBLOCK);

  curs = fsTell(fd);
  checkCursor(5, 12 * 512, curs);

  check(5, buf, 0,    50, 10);
  check(5, buf, 50,  462, 88);
  check(5, buf, 512, 438, 88);   
  check(5, buf, 950,  74, 11);
}



// ============================================================================
// TEST 6 : Large, extending write (700 bytes) starting at block 49
//          512*99, 188*99, 324*0
// ============================================================================
void test6(i32 fd) {
  i8 buf[BUFSIZE];                  // buffer for reads and writes

  fsSeek(fd, 49 * BYTESPERBLOCK, SEEK_SET);     

  i32 curs = fsTell(fd);
  checkCursor(6, 49 * 512, curs);

  memset(buf, 0, BUFSIZE);
  memset(buf, 99, 700);
  
  fsWrite(fd, 700, buf);

  curs = fsTell(fd);
  checkCursor(6, 49 * 512 + 700, curs);

  fsSeek(fd, 49 * BYTESPERBLOCK, SEEK_SET);     

  curs = fsTell(fd);
  checkCursor(6, 49 * 512, curs);

  i32 ret = fsRead(fd, 2 * BYTESPERBLOCK, buf);
  assert(ret == 700);

  curs = fsTell(fd);
  checkCursor(6, 49 * 512 + 700, curs);

  check(6, buf,   0, 512, 99);
  check(6, buf, 512, 188, 99);
  check(6, buf, 700, 324,  0);    // technically beyond EOF
}



void p5test() {

  i32 fd = fsOpen("P5");    // open "P5" for testing

  test1(fd);
  test2(fd);
  test3(fd);
  test4(fd);
  test5(fd);
  test6(fd);

  fsClose(fd);

}







/************************************************************/
/*              BELOW ARE TESTING CASES WE CONDUCTED:       */
/*             COMMENT ABOVE AND UMCOMMENT BELOW TO RUN     */
/*                THE TEST CASES WE CREATED                 */
/************************************************************/
// // ============================================================================
// // p5test.c : use regular C library calls to check what the answers should be
// // when run against the BFS filesystem
// // ============================================================================

// #include "p5test.h"

// // ============================================================================
// // Check that 'size' bytes, starting at buf[start] hold the value 'val'.
// // 'testnum' is the test number - used for reporting
// // ============================================================================
// void check(int testnum, i8* buf, int start, int size, int val) {
//   for (int i = start; i < start + size; ++i) {
//     if (buf[i] != val) {
//       printf("TEST %d : BAD  : buf[%d] = %d but should be %d \n", 
//         testnum, i, buf[i], val);
//       return;
//     }
//   }
//   printf("TEST %d : GOOD \n", testnum);
// }



// // ============================================================================
// // Check that 'actual' == 'expected' for test 'testnum'
// // ============================================================================
// void checkCursor(int testnum, int expected, int actual) {
//   if (actual == expected) {
//     printf("TEST %d : GOOD \n", testnum);
//   } else {
//     printf("TEST %d : BAD  : cursor = %d but should be %d \n", 
//         testnum, actual, expected);
//   }
// }



// // ============================================================================
// // Create file "P5", holding 50 blocks, inside of BFSDISK, and populate
// // ============================================================================
// void createP5() {

//   i32 fd = fsCreate("P5");

//   i8 buf[BYTESPERBLOCK];

//   // Write 100 blocks.  Every byte in block 'b' the value 'b'

//   for (int b = 0; b < 50; ++b) {
//     memset(buf, b, BYTESPERBLOCK);
//     fsWrite(fd, BYTESPERBLOCK, buf);
//   }

//   fsClose(fd);
// }



// // ============================================================================
// // TEST 1 : Small read (100 bytes) from cursor = 0
// // ============================================================================
// void test1(i32 fd) {
//   i8 buf[BUFSIZE];                  // buffer for reads and writes

//   fsSeek(fd, 0, SEEK_SET);     

//   i32 curs = fsTell(fd);
//   checkCursor(1, 0, curs);

//   memset(buf, 0, BUFSIZE);
//   i32 ret = fsRead(fd, 100, buf);   // read 100 bytes from cursor = 0
//   assert(ret == 100);

//   curs = fsTell(fd);
//   checkCursor(1, 100, curs);

//   check(1, buf, 0, 100, 0);
// }


// // ============================================================================
// // TEST 2 : Small read (200 bytes) from 30 bytes into block 1
// // ============================================================================
// void test2(i32 fd) {
//   i8 buf[BUFSIZE];                  // buffer for reads and writes

//   fsSeek(fd, 512 + 30, SEEK_SET);     

//   i32 curs = fsTell(fd);
//   checkCursor(2, 512 + 30, curs);

//   memset(buf, 0, BUFSIZE);
//   i32 ret = fsRead(fd, 200, buf);   // read 200 bytes from current cursor
//   assert(ret == 200);

//   curs = fsTell(fd);
//   checkCursor(2, 512 + 30 + 200, curs);

//   check(2, buf, 0, 200, 1);
// }



// // ============================================================================
// // TEST 3 : Large, spanning read (1,000 bytes) from start of block 20
// //          512*20, 488*21
// // ============================================================================
// void test3(i32 fd) {
//   i8 buf[BUFSIZE];                  // buffer for reads and writes

//   fsSeek(fd, 20 * BYTESPERBLOCK, SEEK_SET);     

//   i32 curs = fsTell(fd);
//   checkCursor(3, 20 * 512, curs);

//   memset(buf, 0, BUFSIZE);
//   i32 ret = fsRead(fd, 1000, buf);  // read 1,000 bytes from current cursor
//   assert(ret == 1000);

//   curs = fsTell(fd);
//   checkCursor(3, 20 * 512 + 1000, curs);

//   check(3, buf,   0, 512, 20);
//   check(3, buf, 512, 488, 21);
// }


// // ============================================================================
// // TEST 4 : Small write (77 bytes) starting at 10 bytes into block 7
// //          10*7, 77*77, 425*7
// // ============================================================================
// void test4(i32 fd) {
//   i8 buf[BUFSIZE];                  // buffer for reads and writes

//   fsSeek(fd, 7 * BYTESPERBLOCK + 10, SEEK_SET);     

//   i32 curs = fsTell(fd);
//   checkCursor(4, 7 * 512 + 10, curs);

//   memset(buf, 0, BUFSIZE);
//   memset(buf, 77, 77);
  
//   fsWrite(fd, 77, buf);

//   curs = fsTell(fd);
//   checkCursor(4, 7 * 512 + 10 + 77, curs);

//   fsSeek(fd, 7 * BYTESPERBLOCK, SEEK_SET);     

//   i32 ret = fsRead(fd, BYTESPERBLOCK, buf);
//   assert(ret == BYTESPERBLOCK);

//   check(4, buf, 0,  10,  7);
//   check(4, buf, 10, 77,  77);
//   check(4, buf, 87, 425, 7);   
// }



// // ============================================================================
// // TEST 5 : Large, spanning write (900 bytes) starting at 50 bytes into 
// //          block 10
// //          50*10, 462*88, 438*88, 74*11
// // ============================================================================
// void test5(i32 fd) {
//   i8 buf[BUFSIZE];                  // buffer for reads and writes

//   fsSeek(fd, 10 * BYTESPERBLOCK + 50, SEEK_SET);     

//   i32 curs = fsTell(fd);
//   checkCursor(5, 10 * 512 + 50, curs);

//   memset(buf,  0, BUFSIZE);
//   memset(buf, 88, 900);
  
//   fsWrite(fd, 900, buf);

//   curs = fsTell(fd);
//   checkCursor(5, 10 * 512 + 50 + 900, curs);

//   fsSeek(fd, 10 * BYTESPERBLOCK, SEEK_SET);     

//   curs = fsTell(fd);
//   checkCursor(5, 10 * 512, curs);

//   i32 ret = fsRead(fd, 2 * BYTESPERBLOCK, buf);
//   assert(ret == 2 * BYTESPERBLOCK);

//   curs = fsTell(fd);
//   checkCursor(5, 12 * 512, curs);

//   check(5, buf, 0,    50, 10);
//   check(5, buf, 50,  462, 88);
//   check(5, buf, 512, 438, 88);   
//   check(5, buf, 950,  74, 11);
// }



// // ============================================================================
// // TEST 6 : Large, extending write (700 bytes) starting at block 49
// //          512*99, 188*99, 324*0
// // ============================================================================
// void test6(i32 fd) {
//   i8 buf[BUFSIZE];                  // buffer for reads and writes

//   fsSeek(fd, 49 * BYTESPERBLOCK, SEEK_SET);     

//   i32 curs = fsTell(fd);
//   checkCursor(6, 49 * 512, curs);

//   memset(buf, 0, BUFSIZE);
//   memset(buf, 99, 700);
  
//   fsWrite(fd, 700, buf);

//   curs = fsTell(fd);
//   checkCursor(6, 49 * 512 + 700, curs);

//   fsSeek(fd, 49 * BYTESPERBLOCK, SEEK_SET);     

//   curs = fsTell(fd);
//   checkCursor(6, 49 * 512, curs);

//   i32 ret = fsRead(fd, 2 * BYTESPERBLOCK, buf);
//   assert(ret == 700);

//   curs = fsTell(fd);
//   checkCursor(6, 49 * 512 + 700, curs);

//   check(6, buf,   0, 512, 99);
//   check(6, buf, 512, 188, 99);
//   check(6, buf, 700, 324,  0);    // technically beyond EOF
// }


// // ============================================================================
// // TEST 7 : Writing beyond current file 
// // ============================================================================

// void test7(i32 fd) {
//   i8 buf[BUFSIZE];                  // buffer for reads and writes

//   fsSeek(fd, 51 * BYTESPERBLOCK, SEEK_SET);     

//   i32 curs = fsTell(fd);
//   checkCursor(7,  51 * BYTESPERBLOCK, curs);

//   memset(buf, 0, BUFSIZE);
//   memset(buf, 11, 512);
//   fsWrite(fd, 512, buf);

//   curs = fsTell(fd);
//   checkCursor(7, 51 * 512 + 512, curs);

//   fsSeek(fd, 51 * BYTESPERBLOCK, SEEK_SET);     

//   curs = fsTell(fd);
//   checkCursor(7, 51 * 512, curs);

//   i32 ret = fsRead(fd, 512, buf);
//   assert(ret == 512);

//   curs = fsTell(fd);
//   checkCursor(7, 51 * 512 + 512, curs);

//   check(7, buf,   0, 512, 11);
//   check(7, buf, 0, 512, 11);
//   check(7, buf, 700, 324,  0);   
// }


// // ============================================================================
// // TEST 8: Writing beyond current file mutliple blocks
// // ============================================================================

// void test8(i32 fd) {
//   i8 buf[BUFSIZE];                  // buffer for reads and writes

//   fsSeek(fd, 51 * BYTESPERBLOCK + 10, SEEK_SET);     

//   i32 curs = fsTell(fd);
//   checkCursor(8,  51 * BYTESPERBLOCK + 10, curs);

//   memset(buf, 0, BUFSIZE);  
//   memset(buf, 11, 512);
//   fsWrite(fd, 512+ 10, buf);

//   curs = fsTell(fd);
//   checkCursor(8, 52* BYTESPERBLOCK + 20, curs);

//   fsSeek(fd, 51 * BYTESPERBLOCK, SEEK_SET);     

//   curs = fsTell(fd);
//   checkCursor(8, 51 * 512, curs);

//   i32 ret = fsRead(fd, 522, buf);
//   assert(ret == 522);

//   curs = fsTell(fd);
//   checkCursor(8, 51 * 512 + 512 + 10, curs);

//   check(8, buf,   0, 512, 11);
//   check(8, buf, 0, 512, 11);
//   check(8, buf, 700, 324,  0);   
// }



// // Made by Nathaniel Fincham
// // ============================================================================
// // TEST 9 : Small Read (50 bytes) from cursor = 50
// // ============================================================================
// void test9(i32 fd) {
//   i8 buf[BUFSIZE];                  // buffer for reads and writes

//   fsSeek(fd, 50, SEEK_SET);     

//   i32 curs = fsTell(fd);
//   checkCursor(9, 50, curs);

//   memset(buf, 0, BUFSIZE);
//   i32 ret = fsRead(fd, 50, buf);   // read 50 bytes from cursor = 50
//   assert(ret == 50);

//   curs = fsTell(fd);
//   checkCursor(9, 100, curs);

//   check(9, buf, 50, 50, 0);
// }



// // Made by Nathaniel Fincham
// // ============================================================================
// // TEST 10 : Small Read (50 bytes) from cursor = 50
// //           Close File Directory then Reopen
// //           Read from Same location
// // ============================================================================
// void test10(i32 fd) {
//   i8 buf[BUFSIZE];                  // buffer for reads and writes

//   fsSeek(fd, 50, SEEK_SET);     

//   i32 curs = fsTell(fd);
//   checkCursor(10, 50, curs);

//   memset(buf, 0, BUFSIZE);
//   i32 ret = fsRead(fd, 50, buf);   // read 50 bytes from cursor = 50
//   assert(ret == 50);

//   curs = fsTell(fd);
//   checkCursor(10, 100, curs);

//   check(10, buf, 50, 50, 0);

//   fsClose(fd);    // Close file directory

//   fd = fsOpen("P5"); // Reopen

//   // Redo Same test
//   fsSeek(fd, 50, SEEK_SET);     

//   curs = fsTell(fd);
//   checkCursor(10, 50, curs);

//   memset(buf, 0, BUFSIZE);
//   ret = fsRead(fd, 50, buf);   // read 50 bytes from cursor = 50
//   assert(ret == 50);

//   curs = fsTell(fd);
//   checkCursor(10, 100, curs);

//   check(10, buf, 50, 50, 0);
// }



// // Made by Nathaniel Fincham
// // ============================================================================
// // TEST 11 : Small write (77 bytes) starting at 30 bytes into block 8
// //          10*7, 77*77, 425*7
// //          Close file directory then reopen
// //          Read to find expected values
// // ============================================================================
// void test11(i32 fd) {
//   i8 buf[BUFSIZE];                  // buffer for reads and writes

//   fsSeek(fd, 8 * BYTESPERBLOCK + 30, SEEK_SET);     

//   i32 curs = fsTell(fd);
//   checkCursor(11, 8 * 512 + 30, curs);

//   memset(buf, 0, BUFSIZE);
//   memset(buf, 77, 77);
  
//   fsWrite(fd, 77, buf);

//   curs = fsTell(fd);
//   checkCursor(11, 8 * 512 + 30 + 77, curs);

//   fsSeek(fd, 8 * BYTESPERBLOCK, SEEK_SET);     

//   i32 ret = fsRead(fd, BYTESPERBLOCK, buf);
//   assert(ret == BYTESPERBLOCK);

//   check(11, buf, 0,  30,  8);
//   check(11, buf, 30, 77,  77);
//   check(11, buf, 107, 405, 8);

//   fsClose(fd);    // Close file directory
//   // Reopen File directory
//   fd = fsOpen("P5");

//   fsSeek(fd, 8 * BYTESPERBLOCK, SEEK_SET);     

//   ret = fsRead(fd, BYTESPERBLOCK, buf);
//   assert(ret == BYTESPERBLOCK);

//   check(11, buf, 0,  30,  8);
//   check(11, buf, 30, 77,  77);
//   check(11, buf, 107, 405, 8);     
// }



// // Made by Nathaniel Fincham
// // ============================================================================
// // TEST 12 : Large spanning write (750 bytes) from block 3
// //           Small Read of what is in block 4 (750-512 bytes)
// // ============================================================================
// void test12(i32 fd) {
//   i8 buf[BUFSIZE];                  // buffer for reads and writes

//   fsSeek(fd, 3 * BYTESPERBLOCK, SEEK_SET);     

//   i32 curs = fsTell(fd);
//   checkCursor(12, 3 * 512, curs);

//   memset(buf,  0, BUFSIZE);
//   memset(buf, 23, 750);
  
//   fsWrite(fd, 750, buf);

//   curs = fsTell(fd);
//   checkCursor(12, 3 * 512 + 750, curs);

//   fsSeek(fd, 3 * BYTESPERBLOCK, SEEK_SET);     

//   curs = fsTell(fd);
//   checkCursor(12, 3 * 512, curs);

//   i32 ret = fsRead(fd, 2 * BYTESPERBLOCK, buf);
//   assert(ret == 2 * BYTESPERBLOCK);

//   curs = fsTell(fd);
//   checkCursor(12, (3 * 512) + (2 * BYTESPERBLOCK), curs);

//   check(12, buf, 0,    50, 23);
//   check(12, buf, 50,  462, 23);
//   check(12, buf, 512, 237, 23);   
//   check(12, buf, 750,  74, 4); // extends beyond the size of the file

//   // Shut down and Open back up FD
//   fsClose(fd);
//   fd = fsOpen("P5");


//   // read
//   fsSeek(fd, 4 * BYTESPERBLOCK, SEEK_SET);     

//   curs = fsTell(fd);
//   checkCursor(12, 4 * BYTESPERBLOCK, curs);

//   memset(buf, 0, BUFSIZE);
//   ret = fsRead(fd, 238, buf);   // read 238 bytes from cursor = 238; all equal to 23
//   assert(ret == 238);

//   curs = fsTell(fd);
//   checkCursor(12, 4 * BYTESPERBLOCK + 238, curs);
  
//   // check end of buffer and one beyond
//   // Expected first should be 23
//   // Expected one beyond the read should be 0
//   check(12, buf, 188, 50, 23);
//   check(12, buf, 238, 0, 0);
// }

// // Create new file and write to it. This should allow to 
// // write to new file and read from it.
// // Made by Adilet Kuroda

// void test14(i32 fd) {
//   i8 buf[BUFSIZE];
//   memset(buf, 11, BUFSIZE);
//   fsSeek(fd, 100, SEEK_SET);
//   i32 cur = fsTell(fd);
//   checkCursor(14, 100, cur);
//   fsWrite(fd, 600, buf);
//   cur = fsTell(fd);
//   checkCursor(14, 700, cur);
//   fsSeek(fd, 100, SEEK_SET);
//   cur = fsTell(fd);
//   checkCursor(14, 100, cur);
//   i32 ret = fsRead(fd, 600, buf);
//   assert(ret == 600);
//   cur = fsTell(fd);
//   checkCursor(14, 700, cur);
//   check(14, buf, 0, 600, 11);
//   fsSeek(fd, 0, SEEK_SET);
//   cur = fsTell(fd);
//   checkCursor(14, 0, cur);
//   memset(buf, 0, BUFSIZE);
//   fsWrite(fd, 100, buf);
//   cur = fsTell(fd);
//   checkCursor(14, 100, cur);
//   fsSeek(fd, 0, SEEK_SET);
//   cur = fsTell(fd);
//   checkCursor(14, 0, cur);
//   ret = fsRead(fd, 100, buf);
//   check(14, buf, 0, 100, 0);
 
// }
// // Test after closing the file and opening again. Make sure 
// // data persists at the correct location  in given disk. 
// // Made by Adilet Kuroda

// void test15(i32 fd) {
//   i8 buf[BUFSIZE];
//   fsSeek(fd, 100, SEEK_SET);
//   i32 cur = fsTell(fd);
//   checkCursor(15, 100, cur);
//   i32 ret = fsRead(fd, 600, buf);
//   assert(ret == 600);

//   cur = fsTell(fd);
//   checkCursor(15, 700, cur);
//   check(15, buf, 0, 600, 11);

//   fsSeek(fd, 0, SEEK_SET);
//   cur = fsTell(fd);
//   checkCursor(14, 0, cur);
//   ret = fsRead(fd, 100, buf);
//   assert(ret == 100);
//   check(15, buf, 0, 100, 0);
 
// }



// void p5test() {

//   i32 fd = fsOpen("P5");    // open "P5" for testing

//   test1(fd);
//   test2(fd);
//   test3(fd);
//   test4(fd);
//   test5(fd);
//   test6(fd);
//   test7(fd);
//   test8(fd);
//   test9(fd);
//   test10(fd);
//   test11(fd);
//   test12(fd);
//   fsClose(fd);
//   printf("TesFiles created\n");
//   i32 fd1 = fsCreate("TestFile");
//   test14(fd1);
//   fsClose(fd1);
//   printf("TestFile closed\n");
//   printf("TestFile opened again\n");
//   fd1 = fsOpen("TestFile");
//   test15(fd1);

//   fsClose(fd1);




// }


// // // Made by Nathaniel Fincham
// // // ============================================================================
// // // TEST 13 : Small Read and Write (50 bytes) from cursor = 50; from forked processes
// // // ============================================================================
// // void test13(i32 fd) {
// //   printf("TEST 13\n");
// //   i8 buf[BUFSIZE];                  // buffer for reads and writes

// //   fsSeek(fd, 50, SEEK_SET);

// //   int pid = fork();     

// //   if (pid == 0){
// //     i32 curs = fsTell(fd);
// //     checkCursor(13, 50, curs);

// //     memset(buf, 0, BUFSIZE);
// //     i32 ret = fsRead(fd, 50, buf);   // read 50 bytes from cursor = 50
// //     assert(ret == 50);

// //     curs = fsTell(fd);
// //     checkCursor(13, 100, curs);

// //     check(13, buf, 50, 50, 0);
// //   }
// //   else{
// //     int x = 100;

// //     i32 curs = fsTell(fd);
// //     checkCursor(x + 13, 50, curs);

// //     // write
// //     memset(buf,  0, BUFSIZE);
// //     memset(buf, 23, 50);
// //     fsSeek(fd, 50, SEEK_SET);
      
// //     curs = fsTell(fd);
// //     checkCursor(x + 13, 50, curs);

// //     fsWrite(fd, 50, buf);

// //     fsSeek(fd, 50, SEEK_SET);
// //     memset(buf, 0, BUFSIZE);
// //     i32 ret = fsRead(fd, 50, buf);   // read 50 bytes from cursor = 50
// //     assert(ret == 50);

// //     check(x + 13, buf, 0, 50, 23);
// //   }
// // }

// // Additional Test Case by Adilet Kuroda
// // Create new file and write to it
// // Read from it