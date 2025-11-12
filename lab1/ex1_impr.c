/***********************************************************************************************************************
 * Copyright [2015-2021] Renesas Electronics Corporation and/or its licensors. All Rights Reserved.
 * 
 * This file is part of Renesas SynergyTM Software Package (SSP)
 *
 * The contents of this file (the "contents") are proprietary and confidential to Renesas Electronics Corporation
 * and/or its licensors ("Renesas") and subject to statutory and contractual protections.
 *
 * This file is subject to a Renesas SSP license agreement. Unless otherwise agreed in an SSP license agreement with
 * Renesas: 1) you may not use, copy, modify, distribute, display, or perform the contents; 2) you may not use any name
 * or mark of Renesas for advertising or publicity purposes or in connection with your use of the contents; 3) RENESAS
 * MAKES NO WARRANTY OR REPRESENTATIONS ABOUT THE SUITABILITY OF THE CONTENTS FOR ANY PURPOSE; THE CONTENTS ARE PROVIDED
 * "AS IS" WITHOUT ANY EXPRESS OR IMPLIED WARRANTY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, AND NON-INFRINGEMENT; AND 4) RENESAS SHALL NOT BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, OR
 * CONSEQUENTIAL DAMAGES, INCLUDING DAMAGES RESULTING FROM LOSS OF USE, DATA, OR PROJECTS, WHETHER IN AN ACTION OF
 * CONTRACT OR TORT, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THE CONTENTS. Third-party contents
 * included in this file may be subject to different terms.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : hal_entry.c
* Description  : This is a very simple example application that blinks all LEDs on a board.
***********************************************************************************************************************/

#include "hal_data.h"
#include "stdio.h"
#include "string.h"
#include "images.h"


#define N 10    /*Frame dimension for QCIF format*/
#define M 10     /*Frame dimension for QCIF format*/

#define B 5      /*Block size*/

#define p 7       /*Search space. Restricted in a [-p,p] region around the
                    original location of the block.*/


void phods_motion_estimation(const int current[N][M], const int previous[N][M],
        int vectors_x[N/B][M/B],int vectors_y[N/B][M/B])
{
  int x, y, i, j, k, l, p1, p2, q2, distx, disty, S, min1, min2, bestx, besty;

  distx = 0;
  disty = 0;


  /*For all blocks in the current frame*/
  for(x=0; x<N/B; x++)
  {
    for(y=0; y<M/B; y++)
    {
     /*Initialize the vector motion matrices*/
     //1st Optimization: Move initialization of motion vectors inside the loop
      vectors_x[x][y] = 0;
      vectors_y[x][y] = 0;
      //3rd Optimization: Reduce multiplications by doing one initially, while also probably keeping B on case
      //int Bx = B*x;
      //int By = B*y;
      S = 4;

      while(S > 0)
      {
        min1 = 255*B*B;
        min2 = 255*B*B;

        /*For all candidate blocks in X dimension*/
        for(i=-S; i<S+1; i+=S)
        {
          distx = 0;

          /*For all pixels in the block*/
          for(k=0; k<B; k++)
          {
            for(l=0; l<B; l++)
            {
              p1 = current[B*x+k][B*y+l];

              if((B*x + vectors_x[x][y] + i + k) < 0 ||
                  (B*x + vectors_x[x][y] + i + k) > (N-1) ||
                  (B*y + vectors_y[x][y] + l) < 0 ||
                  (B*y + vectors_y[x][y] + l) > (M-1))
              {
                p2 = 0;
              } else {
                p2 = previous[B*x+vectors_x[x][y]+i+k][B*y+vectors_y[x][y]+l];
              }
              
              if(p1-p2>0){
                  distx += p1-p2;
              }
              else{
                  distx += p2-p1;
              }

            }
          }

          if(distx < min1)
          {
            min1 = distx;
            bestx = i;
          }
        }

        /*For all candidate blocks in Y dimension*/
        for(i=-S; i<S+1; i+=S)
        {
          disty = 0;

          /*For all pixels in the block*/
          for(k=0; k<B; k++)
          {
            for(l=0; l<B; l++)
            {
              p1 = current[B*x+k][B*y+l];

              if((B*x + vectors_x[x][y] + k) <0 ||
                  (B*x + vectors_x[x][y] + k) > (N-1) ||
                  (B*y + vectors_y[x][y] + i + l) < 0 ||
                  (B*y + vectors_y[x][y] + i + l) > (M-1))
              {
                q2 = 0;
              } else {
                q2 = previous[B*x+vectors_x[x][y]+k][B*y+vectors_y[x][y]+i+l];
              }

              if(p1-q2>0){
                   disty += p1-q2;
               }
               else{
                   disty += q2-p1;
               }
            }
          }

          if(disty < min2)
          {
            min2 = disty;
            besty = i;
          }
        }

        /*
        //2nd Impovement: Combine the two for loops into a single loop, removing redundant calculations
        for(i=-S; i<S+1; i+=S)
        {
          distx = 0;

          //For all pixels in the block
          for(k=0; k<B; k++)
          {
            for(l=0; l<B; l++)
            {
              p1 = current[B*x+k][B*y+l];

              if((B*x + vectors_x[x][y] + i + k) < 0 ||
                  (B*x + vectors_x[x][y] + i + k) > (N-1) ||
                  (B*y + vectors_y[x][y] + l) < 0 ||
                  (B*y + vectors_y[x][y] + l) > (M-1))
              {
                p2 = 0;
              } else {
                p2 = previous[B*x+vectors_x[x][y]+i+k][B*y+vectors_y[x][y]+l];
              }

              if((B*x + vectors_x[x][y] + k) <0 ||
                  (B*x + vectors_x[x][y] + k) > (N-1) ||
                  (B*y + vectors_y[x][y] + i + l) < 0 ||
                  (B*y + vectors_y[x][y] + i + l) > (M-1))
              {
                q2 = 0;
              } else {
                q2 = previous[B*x+vectors_x[x][y]+k][B*y+vectors_y[x][y]+i+l];
              }
              
              //5th Optimization: Use abs instead of if-else statements
              //distx += abs(p1 - p2);
              //disty += abs(p1 - q2);


              if(p1-p2>0){
                  distx += p1-p2;
              }
              else{
                  distx += p2-p1;
                }
              if(p1-q2>0){
                   disty += p1-q2;
               }
              else{
                  disty += q2-p1;
              }

            }
          }

          if(distx < min1)
          {
            min1 = distx;
            bestx = i;
          }
          if(disty < min2)
          {
            min2 = disty;
            besty = i;
          }
        }
        */

        //4th Optimization: Change S updating method to reduce divisions
        //S = S >> 1;
        S = S/2;
        vectors_x[x][y] += bestx;
        vectors_y[x][y] += besty;
      }
    }
  }

}

/*******************************************************************************************************************//**
 * @brief  Blinky example application
 *
 * Blinks all leds at a rate of 1 second using the software delay function provided by the BSP.
 * Only references two other modules including the BSP, IOPORT.
 *
 **********************************************************************************************************************/
void hal_entry(void) {


	// Code to initialize the DWT->CYCCNT register
    CoreDebug->DEMCR |= 0x01000000;
       ITM->LAR = 0xC5ACCE55;
       DWT->CYCCNT = 0;
       DWT->CTRL |= 1;
	   /* Initialize your variables here */

	   
      int motion_vectors_x[N/B][M/B], motion_vectors_y[N/B][M/B], i, j;
        

      uint32_t cycles[10];
      uint32_t start, end, min, max, mid;

      //1st calculation is done seperately to initiate min and max values, so that everything is done in a single loop
      start = DWT->CYCCNT;

      phods_motion_estimation(current,previous,motion_vectors_x,motion_vectors_y);

      end = DWT->CYCCNT;
      cycles[0] = end - start;
      min = cycles[0];
      max = cycles[0];
      mid = cycles[0];

      for(i=1; i<10; i++){

        start = DWT->CYCCNT;
		 
        phods_motion_estimation(current,previous,motion_vectors_x,motion_vectors_y);

        end = DWT->CYCCNT;
        cycles[i] = end - start;
        if(cycles[i] < min){
            min = cycles[i];
        }

        if(cycles[i] > max){
            max = cycles[i];
        }

        mid += cycles[i];
      }


      mid = mid/10;


      freq = (float)SystemCoreClock;
      printf("Timing results for PHODS Motion Estimation algorithm:\n");
      printf("Minimum Time: %.2f ms\n", (min/freq)*1000);
      printf("Maximum Time: %.2f ms\n", (max/freq)*1000);
      printf("Average Time: %.2f ms\n", (mid/freq)*1000);
      while(1){

      }



}
