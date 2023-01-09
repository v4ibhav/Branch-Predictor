#ifndef SIM_BP_H
#define SIM_BP_H



typedef struct bp_params{
    unsigned long int K;  //K is the number of pc bits used to index the chooser table.
    unsigned long int M1; //M1 number of Pc bits 
    unsigned long int M2; // The number of Pc bits used to index bimodal table
    unsigned long int N;  //glibal branch history register bits
    char*             bp_name;
}bp_params;

// Put additional data structures here as per your requirement.
//no need of anymore data structures.

#endif
