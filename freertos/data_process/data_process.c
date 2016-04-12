/*************************************************************************
	> File Name: data_proccess.c
	> Author: 
	> Mail: 
	> Created Time: Tue 12 Apr 2016 06:04:50 PM CST
 ************************************************************************/

#include<stdio.h>

int main()
{
    int id1, id2;
    long long int start_time, end_time;
    freopen("../execution_time_of_servant.data", "r", stdin);
    freopen("data_process.data", "w", stdout);
    
    while( scanf("%d", &id1) != EOF )
    {
        scanf("%lld", &start_time);
        scanf("%d", &id2);
        scanf("%lld", &end_time);

        if( id2 == (id1 + 10) * 3 )
        {
            printf("%d %lf\n", id1, (end_time - start_time)/1000000.0); // transit to micro second.
        }
        else
        {
            fprintf(stdout, "Servant %d had been preempted\n", id1);
        }
    }

    fclose(stdin);
    fclose(stdout);

    return 0;
}
