/*Assignment 4 Aging algorithm */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>


#define MAXREF 1000
#define MAXCOUNT 20000
#define MAX_STRING_LEN 32
#define MAX_PAGE_NUMBER 1000000
#define RESET_AGE 0
typedef enum { false, true } bool;

//
// PgFrameEntry : Physical Page Frame Entry
// @reverse_pg : virual page number
// @age : Age of the physical frame

struct PgFrameEntry
{
  int reverse_pg;
  int age;
};

struct PgFrameEntry *pf_table;

int    pfn_avail;

static void init_pf_table(struct PgFrameEntry *pf_table, int total_pfn);


// PgTableEntry : Page Table Entry
// @pfn : physical page frame
// @prsent : valid 

struct PgTableEntry
{
  int pfn;
  bool present;
};

struct PgTableEntry pg_table[MAXREF];

int pg_pattern[MAXCOUNT];

static 
int init_pattern(char *filename) {
   int i = 0;

   FILE *fp =NULL;

   char  buf[MAX_STRING_LEN];

   int total_entries = -1;

   fp= fopen(filename,"r");
   if (!fp) {
  	 perror(" Error Opening File");
	 return -1;
   }
		 
   while ((fgets(buf, MAX_STRING_LEN, fp) != NULL) && (i < MAXCOUNT)) {
	int pg_no = atoi(buf);

	if (pg_no > MAX_PAGE_NUMBER - 1) {
		printf("Illegal Input\n");
		goto close;
	} else  {
   		pg_pattern[i]= pg_no;
 		assert(pg_no > 0);
	}

        i++;
  };

 total_entries = i;

 close:

 fclose(fp);
 return total_entries;
}

static
void init_pf_table(struct PgFrameEntry *pf_table, int total_pfn) {
	int i =0;
   for ( i = 0; i  < total_pfn; i++ ) {
	pf_table[i].reverse_pg = -1;
	pf_table[i].age = RESET_AGE;
   }
}
	
static 
int get_pfn(struct PgFrameEntry *pf_table, int total_pfn) {
   int i = 0;

   // Fast Path	
   if (!pfn_avail) {
	return -1;
   }	

   for (i = 0; i  < total_pfn; i++ ) {
	if (pf_table[i].reverse_pg == -1)
		break;
   }

   pfn_avail--;
   return i;		
}

// Called on Eviction
static
int select_candidate_pfn(struct PgFrameEntry *pf_table, int total_pfn) {
  
   int candidate_pfn = 0;
   int max_age = pf_table[candidate_pfn].age;
    int i=0;
   // Selction 
   for ( i = 1; i < total_pfn; i++) {
	if (pf_table[i].age > max_age) {
		max_age = pf_table[i].age;
		candidate_pfn = i;
	}
   }
   return candidate_pfn;	
}

static
void evict_pfn(struct PgFrameEntry *pf_table, int candidate_pfn) {
     
	int pageno ;
	assert(candidate_pfn >= 0);
	assert(candidate_pfn < MAX_PAGE_NUMBER);

	pageno = pf_table[candidate_pfn].reverse_pg;
	pg_table[pageno].pfn = -1;
	pg_table[pageno].present = false;
}

static
void update_age(struct PgFrameEntry *pf_table, int total_pfn) {
	int i=0;
	for (i = 0; i < total_pfn; i++) {
		if (pf_table[i].reverse_pg != -1)
			pf_table[i].age++;
	}
}

static 
void write_to_file(int PgFault_counter,char *outputfilename){

	char *C ="";
	FILE *fd;
	fd=fopen(outputfilename,"a");
	fprintf(fd,"\n%s",C);
	fprintf(fd,"%d",PgFault_counter);

	fclose(fd);
}

struct stats {
	int pg_faults;
	int total_pfn;
};



int main(int argc, char *argv[])
{
   int ret = 0;	

   int max_entries = 0;

   int total_pfn =0;
   int i = 0 ,pfn=0;

   int ref_counter =0;
   int page_fault_counter =0;

   struct stats pg_stats = { 0 , 0};

   if (argc <= 3) {
	printf ("Argument List small\n"
                "Usage : cmd <MaxPageFrames> <patternfile> <outputdatafile>\n");
        return -1; 
   }
	  	
   total_pfn = atoi(argv[1]);

   pg_stats.total_pfn = pfn_avail = total_pfn;
	
   pf_table = (struct PgFrameEntry *)malloc(sizeof(struct PgFrameEntry)* total_pfn);
   if (pf_table == NULL) {
	perror("No Memory\n");
	return -ENOMEM;
   }

   init_pf_table(pf_table, total_pfn);	
	
   ret = init_pattern(argv[2]);
   if (ret < 0) {
	printf("Terminating Program\n");
	return -1;
   } else  {
   	max_entries = ret < MAXCOUNT ? ret : MAXCOUNT;
   }	

   #if 0
   printf("Total references  : %d\n", ret);
   printf("No of page frames : %d\n", total_pfn);
   #endif	
  
   
   do {
 	int page_no = pg_pattern[i] - 1;
	ref_counter++;
	assert(page_no >= 0);
	assert(page_no < MAXREF);
	if (!pg_table[page_no].present) {
		pg_stats.pg_faults++;
        
		page_fault_counter++;


		pfn = get_pfn(pf_table, total_pfn);
		if (pfn < 0) {
			// Evict
			pfn = select_candidate_pfn(pf_table, total_pfn);			
			evict_pfn(pf_table, pfn);
		}
			
		 pf_table[pfn].reverse_pg = page_no;
		 pf_table[pfn].age = RESET_AGE;


		 pg_table[page_no].pfn = pfn;
		 pg_table[page_no].present = true;

	} else {
		int pfn = pg_table[page_no].pfn;
		pf_table[pfn].age = RESET_AGE;
	}

	update_age(pf_table, total_pfn);

    		if(ref_counter == MAXREF){
	         
			  write_to_file(page_fault_counter , argv[3]);
		   // printf("\n Refcount : %d \t pfFault :%d\n",ref_counter,page_fault_counter);
			//  page_fault_counter =0;
			  ref_counter =0;

		}
	
        i++;

   } while (i < max_entries);

   printf("Stats : \n Page Frame Number :%d\n Total Page Faults :%d \n Total Page Access :%d \n ", total_pfn,pg_stats.pg_faults, max_entries);

   return 0;
}

