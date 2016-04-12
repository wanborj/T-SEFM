/*************************************************************************
	> File Name: data_analysis.c
	> Author: 
	> Mail: 
	> Created Time: Tue 12 Apr 2016 06:23:56 PM CST
 ************************************************************************/

#include<stdio.h>

int count[21];
double max[21];
double min[21];
double average[21];
double variation[21];
void init()
{
    int i;
    for( i = 0; i < 21; i ++ )
    {
        count[i] = 0;
        max[i] = 0;
        min[i] = 10000.0;
        average[i] = 0.0;
        variation[i] = 0.0;
    }
}

void get_average_and_max()
{
    int id, i;
    double duration;
    freopen("data_process.data", "r",stdin);
    while(scanf("%d %lf\n", &id, &duration) != EOF)
    {
        //printf("id: %d; duration: %lf\n", id, duration);
        
        count[id] ++; 
        if( max[id] < duration )
        {
            max[id] = duration;
        }

        if( min[id] > duration )
        {
            min[id] = duration;
            printf("the min execution time is %lf\n", min[id]);
        }
        average[id] += duration;
    }
    for( i = 0; i < 21; ++ i )
    {
        average[i] = average[i] / count[i];
    }
    fclose(stdin);
}

void get_variation()
{
    int id, i;
    double duration; 
    double temp[21];
    for( i = 0; i < 21; ++ i )
    {
        temp[i] = 0;
    }

    freopen("data_process.data", "r",stdin);

    while(scanf("%d %lf", &id, &duration) != EOF )
    {
        variation[id] += (duration - average[id]) * (duration - average[id]);         
    }

    for( i = 0; i < 21; ++ i )
    {
        variation[i] /= count[i];
    }

    fclose(stdin);
}

void output()
{
    int i;
    freopen("data_analysis.data", "w",stdout);
    
    for( i = 0; i < 21; ++ i )
    {
        printf("#######################\n");
        printf("Servant %d:\n", i);

        printf("the Max execution time: %lf\n", max[i]);
        printf("the Min execution time: %lf\n", min[i]);

        printf("the Count is : %d\n", count[i]);
        printf("the Average execution time: %lf\n", average[i]);
        printf("the Variation execution time: %lf\n", variation[i]);
        
        printf("#######################\n");
    }

    fclose(stdout);
}

int main()
{
    init();
    get_average_and_max();
    get_variation();
    output();

    return 0;
}
