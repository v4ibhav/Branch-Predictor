#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sim_bp.h"
#include <math.h>
#include <iostream>
#include <iomanip>
#define BI 0
#define GS 1
#define HY 2

#define forn(i,n) for(int i= 0;i<n;i++)
typedef unsigned long int ll;

using namespace std;

//global variables
int BI_Index, GS_Index,HY_Index;
int M1,M2,N,K;
int flag;
int Count_mispredict;
int Total_prediction;

int shifter;
int Table_Size,Table_Size_b,Table_Size_h,Table_Size_g;


// globally accesible arrays to store predictor values//
int *bimodal_table;
int *gshare_table;
int *hybrid_table;

//create a table according to the type of predict table needed
void predictTable_init(ll kK,ll mM1, ll mM2, ll nN,ll type)
{
    shifter = 0;
    if(type == BI )
    {
        // cout<<"we should not be inside the BI"<<endl;

        flag = 0;
        Count_mispredict = 0;
        Table_Size = pow(2,mM2);
        M2 = mM2;
        bimodal_table = new int[Table_Size];
        forn(i,Table_Size)
        {
            bimodal_table[i]= 2;
        }

    }
    if(type == GS)
    {
        flag = 1;
        // cout<<"we should not be inside the GS"<<endl;
        Count_mispredict =0;
        Table_Size = pow(2,mM1);
        M1 = mM1;
        N = nN;
        // cout<<"\n table size is "<<Table_Size;
        gshare_table = new int[Table_Size];
        forn(i,Table_Size)
        {
            gshare_table[i] = 2;
        }
    }
    if(type == HY)
    {
        flag =2;
        K=kK;
        N=nN;
        M1=mM1;
        M2=mM2;
        Count_mispredict = 0;
        //initialize the table of gshare in hybrid predcitor
        Table_Size_g = pow(2,M1);
        gshare_table = new int[Table_Size_g];
        //same variable of table size as of now
        forn(i,Table_Size_g)
        {
            gshare_table[i] = 2;
        }

        //initialize the tabel of bimodal in hybrid predictor;
        Table_Size_b = pow(2,M2);
        bimodal_table = new int[Table_Size_b];
        forn(i,Table_Size_b)
        {
        // cout<<"we are inside "<<endl;
            bimodal_table[i] = 2;
        }

        //inititalize the table which selector that is hybrid 
        Table_Size_h = pow(2,K);
        hybrid_table = new int[Table_Size_h];
        forn(i,Table_Size_h)
        {
            hybrid_table[i] =1;
        }


    }
}


//get the correct index
void fetch_Index(ll addr)
{
    ll Mask_Ind;
    ll Mask_bhr;
    ll Mask_temp;
    ll Mask_Selector;
    int index;
    addr = addr>>2;
    if(flag == BI)
    {
       Mask_Ind = pow(2,M2)-1;
       BI_Index = addr& Mask_Ind;
    }
    if(flag == GS)
    {
        int diff = M1-N;

        Mask_Ind = pow(2,M1)-1;
        Mask_bhr = pow(2,N)-1;
        Mask_temp = pow(2,diff)-1;

        index = addr & Mask_Ind;
        
        int temp = index & Mask_temp;
        index = (index>>diff)^(shifter & Mask_bhr);
        index = (index<<diff)|temp;
        GS_Index = index;
        
    }
    if(flag == HY)
    {
        //calculating index for Gshare
        int diff = M1-N;    

        Mask_Ind = pow(2,M1)-1;
        Mask_bhr = pow(2,N)-1;
        Mask_temp = pow(2,diff)-1;

        index = addr & Mask_Ind;
        
        int temp = index & Mask_temp;
        index = (index>>diff)^(shifter & Mask_bhr);
        index = (index<<diff)|temp;
        GS_Index = index;
        // cout<<"GS index is : "<<GS_Index<<endl;

        //calculating index for bimodal predictor
        Mask_Ind = pow(2,M2)-1;
        BI_Index = addr & Mask_Ind;


        //calculating index for the hybrid table

        Mask_Selector = pow(2,K)-1;
        HY_Index = addr & Mask_Selector;


    }
}
//get the prediction done at fetch step
void predict(ll index,char outcome)
{   
    int actual;
    if(outcome == 't')
    {
        actual = 1;
    }
    else
        actual = 0;

    
    int prediction = 0;
    int pred_BI=0;
    int pred_GS = 0;


    if(flag == BI)
    {
        if(bimodal_table[BI_Index] >=2)
        {
            prediction = 1;
        }

        if(actual == 1 && bimodal_table[BI_Index]<3)
        {
            bimodal_table[BI_Index]++;
            // cout<<"\n mein yaha hu";
        }
        else if(actual == 0 && bimodal_table[BI_Index]>0)
        {
            bimodal_table[BI_Index]--;

        }

        if(prediction != actual)
        {
            Count_mispredict++;
            // cout<<"yeh kya hua "<<endl;
        }
    }

    if(flag == GS)
    {
        if(gshare_table[GS_Index]>=2)
        {
            prediction = 1;
        }
        if(actual == 1 && gshare_table[GS_Index]<3)
        {
            gshare_table[GS_Index]++;
            // cout<<"\n mein yaha hu";

        }
        else if(actual == 0 && gshare_table[GS_Index]>0)
        {
            gshare_table[GS_Index]--;
        }
        shifter>>=1;
        int temp = actual<<(N-1);
        shifter |=temp;

        if(prediction!=actual)
        {
            Count_mispredict++;
        }
    }

    if(flag == HY)
    {

       /*
       (1) Obtain two predictions, one from the gshare predictor (follow steps 1 and 2 in Section 3.1.2)
and one from the bimodal predictor (follow steps 1 and 2 in Section 3.1.1).
(2) Determine the branch’s index into the chooser table. The index for the chooser table is bit
k+1 to bit 2 of the branch PC (i.e., as before, discard the lowest two bits of the PC).
(3) Make an overall prediction. Use index to get the branch’s chooser counter from the chooser
table. If the chooser counter value is greater than or equal to 2, then use the prediction that was
obtained from the gshare predictor, otherwise use the prediction that was obtained from the
bimodal predictor.
(4) Update the selected branch predictor based on the branch’s actual outcome. Only the branch
predictor that was selected in step 3, above, is updated (if gshare was selected, follow step 3 in
Section 3.1.2, otherwise follow step 3 in Section 3.1.1).
(5) Note that the gshare’s global branch history register must always be updated, even if bimodal
was selected (follow step 4 in Section 3.1.2).
       */ 

        //step 1 obtain prediction
        if(bimodal_table[BI_Index]>=2)
        {
            pred_BI = 1;
        }
        if(gshare_table[GS_Index]>=2)
        {
            pred_GS = 1;
        }

        //step 3

        if(hybrid_table[HY_Index]>=2)
        {
            if(actual == 1 && gshare_table[GS_Index]<3)
            {
            gshare_table[GS_Index]++;
            // cout<<"\n mein yaha hu";
            }
            else if(actual == 0 && gshare_table[GS_Index]>0)
            {
                gshare_table[GS_Index]--;
            }

            if(pred_GS != actual)
            {
                Count_mispredict++;
            }
        }
        else
        {
            if(actual == 1 && bimodal_table[BI_Index]<3)
            {
                bimodal_table[BI_Index]++;
            }
            else if(actual == 0 && bimodal_table[BI_Index] >0)
            {
                bimodal_table[BI_Index]--;
            }

            if(pred_BI != actual)
            {
                Count_mispredict++;
            }
        }

        //step 5
        shifter>>=1;
        int temp = actual<<(N-1);
        shifter |=temp;

        //step 6
        /*
        Results from predictors:
both incorrect
or
both correct
gshare correct,
bimodal incorrect
bimodal correct,
gshare incorrect
Chooser counter
update policy: no change (but satu increment rates at 3) (but saturates at 0*/

        if(pred_GS == actual && pred_BI != actual)
        {
            if(hybrid_table[HY_Index]<3)
            {
                hybrid_table[HY_Index]++;
            }
        }
        else if(pred_GS != actual && pred_BI == actual)
        {
            if(hybrid_table[HY_Index]>0)
            {
                hybrid_table[HY_Index]--;
            }
        }        
    } 
}

/*  argc holds the number of command line arguments
    argv[] holds the commands themselves

    Example:-
    sim bimodal 6 gcc_trace.txt
    argc = 4
    argv[0] = "sim"
    argv[1] = "bimodal"
    argv[2] = "6"
    ... and so on
*/
int main (int argc, char* argv[])
{
    FILE *FP;               // File handler
    char *trace_file;       // Variable that holds trace file name;
    bp_params params;       // look at sim_bp.h header file for the the definition of struct bp_params
    char outcome;           // Variable holds branch outcome
    unsigned long int addr; // Variable holds the address read from input file
    
    if (!(argc == 4 || argc == 5 || argc == 7))
    {
        printf("Error: Wrong number of inputs:%d\n", argc-1);
        exit(EXIT_FAILURE);
    }
    
    params.bp_name  = argv[1];
    
    // strtoul() converts char* to unsigned long. It is included in <stdlib.h>
    if(strcmp(params.bp_name, "bimodal") == 0)              // Bimodal
    {
        if(argc != 4)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc-1);
            exit(EXIT_FAILURE);
        }
        params.M2       = strtoul(argv[2], NULL, 10);
        trace_file      = argv[3];
        printf("COMMAND\n%s %s %lu %s\n", argv[0], params.bp_name, params.M2, trace_file);
        predictTable_init(0,0,params.M2,0,0);
    }
    else if(strcmp(params.bp_name, "gshare") == 0)          // Gshare
    {
        if(argc != 5)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc-1);
            exit(EXIT_FAILURE);
        }
        params.M1       = strtoul(argv[2], NULL, 10);
        params.N        = strtoul(argv[3], NULL, 10);
        trace_file      = argv[4];
        printf("COMMAND\n%s %s %lu %lu %s\n", argv[0], params.bp_name, params.M1, params.N, trace_file);
        predictTable_init(0,params.M1,0,params.N,1);

    }
    else if(strcmp(params.bp_name, "hybrid") == 0)          // Hybrid
    {
        if(argc != 7)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc-1);
            exit(EXIT_FAILURE);
        }
        params.K        = strtoul(argv[2], NULL, 10);
        params.M1       = strtoul(argv[3], NULL, 10);
        params.N        = strtoul(argv[4], NULL, 10);
        params.M2       = strtoul(argv[5], NULL, 10);
        trace_file      = argv[6];
        printf("COMMAND\n%s %s %lu %lu %lu %lu %s\n", argv[0], params.bp_name, params.K, params.M1, params.N, params.M2, trace_file);
        // cout<<"here"<<endl;
        predictTable_init(params.K,params.M1,params.M2,params.N,2);
        // cout<<"there"<<endl;

        // predictTable_init(0,params.M1,0,params.N,1);


    }
    else
    {
        printf("Error: Wrong branch predictor name:%s\n", params.bp_name);
        exit(EXIT_FAILURE);
    }
    
    // Open trace_file in read mode
    FP = fopen(trace_file, "r");
    if(FP == NULL)
    {
        // Throw error and exit if fopen() failed
        printf("Error: Unable to open file %s\n", trace_file);
        exit(EXIT_FAILURE);
    }
    
    //variable intialization section



    //end


    char str[2];
    while(fscanf(FP, "%lx %s", &addr, str) != EOF)
    {
        
        outcome = str[0];
        // if (outcome == 't')
            // printf("%lx %s\n", addr, "t");           // Print and test if file is read correctly
        // else if (outcome == 'n')
            // printf("%lx %s\n", addr, "n");          // Print and test if file is read correctly
        /*************************************
            Add branch predictor code here
        **************************************/

        //m1 is needed to bitmasked
        //K=0,M1=0,M2=value,N=0 for bimodal

        //pseudo code for bimodal predictor
        /*
        * initialize the BTB
        * get the bits to find in the BTB
        *   if inside the table then output prdiction
        *   else update with a predetermined predcition value check if the predetermined prediction is true or 
        *       if true strengthen the prediction 
        *       else weaken the prediction 
        * return bimodal is done 
        */

        
        //intialize the prediction table 
        
        //table is created according to the type of predictor needed 
        if(flag == BI)
        {
            fetch_Index(addr);
            predict(BI_Index,outcome);

        }
        else if(flag == GS)
        {
            fetch_Index(addr);
            predict(GS_Index,outcome);
        }
        //code for bimodal as of now
        //predict
        else if(flag == HY)
        {
            fetch_Index(addr);
            predict(HY_Index, outcome);
        }
        Total_prediction++;
        
    }
    // cout<<Count_mispredict<<"\t"<<Total_prediction;
    double percentage = ((double)Count_mispredict/(double)Total_prediction)*100;
    
    if(flag == BI)
    {
        cout<<"OUTPUT"<<endl;

        cout<<" number of predictions:\t"<<Total_prediction<<endl;
        cout<<" number of mispredictions:\t"<<Count_mispredict<<endl;
        // cout<<"misprediction rate:\t"<<setprecision(4)<<percentage<<endl;
        printf(" misprediction rate:\t%.2f%c",percentage,'%');
        printf("\n");

        cout<<"FINAL BIMODAL CONTENTS";

        // cout<<"\ttable size:  "<<Table_Size;
        forn(i,Table_Size)
            {   cout<<endl;
                cout<<" " <<i<<"\t"<<bimodal_table[i];
            }
    }
    if (flag == GS)
    {
        cout<<"OUTPUT"<<endl;

        cout<<"number of predictions:\t"<<Total_prediction<<endl;
        cout<<"number of mispredictions:\t"<<Count_mispredict<<endl;
        // cout<<"misprediction rate:\t"<<setprecision(4)<<percentage<<endl;
        printf("misprediction rate:\t%.2f%c",percentage,'%');
        printf("\n");
        cout<<"FINAL GSHARE CONTENTS";

        forn(i,Table_Size)
        {
            cout<<endl;
            cout<<i<<"\t"<<gshare_table[i];
        }
    }
    if (flag == HY)
    {
        cout<<"OUTPUT"<<endl;
        cout<<"number of predictions:\t"<<Total_prediction<<endl;
        cout<<"number of mispredictions:\t"<<Count_mispredict<<endl;
        printf("misprediction rate:\t%.2f%c",percentage,'%');
        printf("\n");


        cout<<"FINAL CHOOSER CONTENTS";
        forn(i,Table_Size_h)
        {
            cout<<endl;
            cout<<i<<"\t"<<hybrid_table[i];
        }
        // cout<<endl;
        cout<<"FINAL GSHARE CONTENTS";
        forn(i,Table_Size_g)
        {
            cout<<endl;
            cout<<i<<"\t"<<gshare_table[i];
        }
        // cout<<endl;
        cout<<"FINAL BIMODAL CONTENTS";
        forn(i,Table_Size_b)
        {
            cout<<endl;
            cout<<i<<"\t"<<bimodal_table[i];
        }
    }
    
    // return 0;
}
