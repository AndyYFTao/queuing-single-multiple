#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <queue>
#include <fstream>
using namespace std;

//declare the number of service windows as K, the total count of customers as N, the number of simulations required as S
int K,N,S;

//define a struct to represent time by hour and minute
struct thetime
{
    int h;  //hour
    int m;  //minute
};

//define all attributes of a customer in a struct
struct customers
{
    struct thetime arrive_time;     //arrival time
    struct thetime wait_time;       //waiting time
    struct thetime start_time;      //service starting time
    int business_time;              //duration of the corresponding customer service(Assumed as integers and between 1-30 mins)
    struct thetime end_time;        //ending time of the service
    int in_bank_number;             //queuing number obtained upon arrival
};

//Function declaration
void customers_time(struct customers &c, int index);
void customer_sort(customers customer[]);

void customers_in_queue(queue<customers> cus_queue[],customers customer[],int each_queue_cus_number[], char label);

void leave_queue(queue<customers> cus_queue[],customers customer);
int judge_queue_in_M(queue<customers> cus_queue[],customers &customer,int each_queue_cus_number[],int index);
int judge_queue_in_S(queue<customers> cus_queue[],customers &customer,int each_queue_cus_number[],int index);

void output (customers customer[], int each_queue_cus_number[], ofstream &oufile);

int main()
{
    //interactively receive the required inputs
    cout<<"Number of Windows (NW): ";
    cin>>K;
    cout<<"Number of Customers (NC): ";
    cin>>N;
    cout<<"Number of Simulations (NS): ";
    cin>>S;

    //file output stream to create two new files and write the simulation data in them
    ofstream oufile_m;//record the multiple-line data
    ofstream oufile_s;//record the single-line data
    oufile_m.open ("simulation_data_m.txt", ios::app);//write in by appending one row each time in the following for-loop
    oufile_s.open ("simulation_data_s.txt", ios::app);

    if(oufile_m.fail()||oufile_s.fail())
        cout << "File failed to open!" << endl;//signify file creation failure
    else
    {
        oufile_m<<"NW: " << K << " NC: " << N << " NS: " << S << endl;
        oufile_s<<"NW: " << K << " NC: " << N << " NS: " << S << endl;
        char label;//to be used later to distinguish between multiple-line cases and single-line cases
        oufile_m << "Multiple case:" << endl;
        oufile_s << "Single case:" << endl;

        //loop of simulation
        for (int i = 0; i<S; i++)
        {
            customers customer_m[N];//an array of user-defined structures to represent N customers' information
            queue<customers> cus_queue_m[K];//an array of queues to model the queues of each service window
            int each_queue_cus_number[K];//count the number of queuing customers at each window (K windows in total)

            //initialization of attributes of each customer
            for(int j=0; j<N; j++)
                customers_time(customer_m[j],i+j);

            //initialization of numbers of queuing customers at each window
            for(int j=0; j<K; j++)
                each_queue_cus_number[j]=0;

            //sort according to customers' arrival time
            customer_sort(customer_m);

            label = 'M';//multiple-line cases

            //assign queuing numbers upon arrival
            for(int j=0; j<N; j++)
                customer_m[j].in_bank_number = j + 1;

            //put customers in queues
            customers_in_queue(cus_queue_m,customer_m,each_queue_cus_number,label);

            output(customer_m,each_queue_cus_number, oufile_m);


            //analogous procedure for single-line cases
            customers customer_s[N];
            queue<customers> cus_queue_s[K];

            for(int j=0; j<N; j++)
                customers_time(customer_s[j],i+j);

            for(int j=0; j<K; j++)
                each_queue_cus_number[j]=0;

            customer_sort(customer_s);

            label = 'S';
            for(int j=0; j<N; j++)
                customer_s[j].in_bank_number = j + 1;

            customers_in_queue(cus_queue_s,customer_s,each_queue_cus_number,label);

            output(customer_s,each_queue_cus_number, oufile_s);
        }
    }
    return 0;
}

//initialization via random generation
void customers_time(struct customers &c, int index)
{
    //set indistinct seeds for different customers using indistinct index parameters
    srand(index);
    //randomly generate customers' arrival time and service durations
    c.arrive_time.h=9+rand()%8;
    c.arrive_time.m=rand()%60;
    c.business_time=rand()%30+1;
}

//sort all customers in ascending order of randomly generated arrival time
void customer_sort(customers customer[])
{
    int max_time_index;   //record the customer index with the latest arrival time
    customers max_time_cus,swap_cus;
    //selection sort
    for(int i=N-1; i>0; i--)
    {
        max_time_cus=customer[i];
        max_time_index=i;
        //locate the customer who arrives the latest
        for(int j=0; j<i; j++)
        {
            if((customer[j].arrive_time.h)*60+customer[j].arrive_time.m > (max_time_cus.arrive_time.h)*60+max_time_cus.arrive_time.m)
            {
                max_time_cus=customer[j];
                max_time_index=j;
            }
        }
        if(i!=max_time_index)
        {
            //the swap part of selection sort
            swap_cus=customer[i];
            customer[i]=max_time_cus;
            customer[max_time_index]=swap_cus;
        }
    }
}

//for multiple-line cases, judge which queue has the fewest people waiting
int judge_queue_in_M(queue<customers> cus_queue[],customers &customer,int each_queue_cus_number[],int index)
{
    //record waiting time of each window in an array
    int each_queue_wait_time[K];

    for(int i=0; i<K; i++)
    {
        //the waiting time of an individual depends on the ending service time of the previous customer in its queue
        int wait_h=cus_queue[i].back().end_time.h-customer.arrive_time.h;

        int wait_m;
        if (wait_h == 0)
            wait_m=cus_queue[i].back().end_time.m-customer.arrive_time.m;
        else //wait_h > 0
            wait_m=cus_queue[i].back().end_time.m-customer.arrive_time.m + 60;

        each_queue_wait_time[i]=wait_h*60+wait_m;
    }

    //determine the queue with the fewest people waiting
    int min_cus_number_index=0;
    for(int j=1; j<K; j++)
    {
        if(cus_queue[j].size() < cus_queue[min_cus_number_index].size())
            min_cus_number_index=j;
    }
    //update data
    customer.wait_time.h=each_queue_wait_time[min_cus_number_index]/60;
    customer.wait_time.m=each_queue_wait_time[min_cus_number_index]%60;
    customer.start_time.h=cus_queue[min_cus_number_index].back().end_time.h;
    customer.start_time.m=cus_queue[min_cus_number_index].back().end_time.m;
    customer.end_time.h=customer.start_time.h+(customer.start_time.m+customer.business_time)/60;
    customer.end_time.m=(customer.start_time.m+customer.business_time)%60;

    //push the customer in queue
    //if a customer's starting time (not necessarily arrival time) is later than the bank's closing time
    //then he/she would not join any queue or receive any service
    if((customer.start_time.h)*60+customer.start_time.m < 17*60)
    {
        cus_queue[min_cus_number_index].push(customer);
        each_queue_cus_number[min_cus_number_index]++;
    }
    return min_cus_number_index;
}

//for single-line cases, judge which queue has the least waiting time
int judge_queue_in_S(queue<customers> cus_queue[],customers &customer,int each_queue_cus_number[],int index)
{
    //analogous procedure for single-line cases
    int each_queue_wait_time[K];

    for(int i=0; i<K; i++)
    {
        int wait_h=cus_queue[i].back().end_time.h-customer.arrive_time.h;
        each_queue_wait_time[i]=wait_h*60+cus_queue[i].back().end_time.m-customer.arrive_time.m;
    }

    //determine the queue with the least waiting time
    int min_time_queue_index=0;
    for(int j=1; j<K; j++)
    {
        if(each_queue_wait_time[j] < each_queue_wait_time[min_time_queue_index])
            min_time_queue_index=j;
    }

    customer.wait_time.h=each_queue_wait_time[min_time_queue_index]/60;
    customer.wait_time.m=each_queue_wait_time[min_time_queue_index]%60;
    customer.start_time.h=cus_queue[min_time_queue_index].back().end_time.h;
    customer.start_time.m=cus_queue[min_time_queue_index].back().end_time.m;
    customer.end_time.h=customer.start_time.h+(customer.start_time.m+customer.business_time)/60;
    customer.end_time.m=(customer.start_time.m+customer.business_time)%60;

    if((customer.start_time.h)*60+customer.start_time.m < 17*60)
    {
        cus_queue[min_time_queue_index].push(customer);
        each_queue_cus_number[min_time_queue_index]++;
    }
    return min_time_queue_index;
}

//when the next customer arrives, determine whether those customers currently in queues have ended
//services and move out of queues accordingly
void leave_queue(queue<customers> cus_queue[],customers customer)
{
    for(int i=0; i<K; i++)
    {
        if(!cus_queue[i].empty())
        {
            while((cus_queue[i].front().start_time.h)*60+cus_queue[i].front().start_time.m+
                    cus_queue[i].front().business_time <= (customer.arrive_time.h)*60+customer.arrive_time.m)
            {
                cus_queue[i].pop();
                if(cus_queue[i].empty())
                    break;
            }
        }
    }
}

//put customers in queues
void customers_in_queue(queue<customers> cus_queue[],customers customer[],int each_queue_cus_number[], char label)
{
    int queue_number;//locate an empty window
    for(int i=0; i<N; i++)
    {
        bool queue_free=false;

        //move those in the front of queues out if necessary
        leave_queue(cus_queue,customer[i]);

        for(int j=0; j<K; j++)
        {
            //when there are available windows
            if(cus_queue[j].empty())
            {
                //update data and join queues
                customer[i].wait_time.h=0;
                customer[i].wait_time.m=0;
                customer[i].start_time.h=customer[i].arrive_time.h;
                customer[i].start_time.m=customer[i].arrive_time.m;
                customer[i].end_time.h=customer[i].start_time.h+(customer[i].start_time.m+customer[i].business_time)/60;
                customer[i].end_time.m=(customer[i].start_time.m+customer[i].business_time)%60;
                cus_queue[j].push(customer[i]);
                each_queue_cus_number[j]++;
                queue_free=true;
                break;
            }
        }
        //when there are no available windows
        if(queue_free==false)
        {
            if (label=='M')//multiple-line cases
                queue_number = judge_queue_in_M(cus_queue,customer[i],each_queue_cus_number,i);   //judge which queues to join
            else
                queue_number = judge_queue_in_S(cus_queue,customer[i],each_queue_cus_number,i);
        }
    }
}

void output (customers customer[], int each_queue_cus_number[], ofstream &oufile)
{
    int sum_cus_wait_time=(customer[0].wait_time.h)*60+customer[0].wait_time.m;
    int actual_cus_numbers=0;
    for(int i=0; i<K; i++)
        actual_cus_numbers+=each_queue_cus_number[i];

    for(int i=1; i<actual_cus_numbers; i++)
        sum_cus_wait_time+=(customer[i].wait_time.h)*60+customer[i].wait_time.m;

    oufile << (double)sum_cus_wait_time/actual_cus_numbers << endl;
}




