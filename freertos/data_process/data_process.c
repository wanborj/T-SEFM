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
    int start_time, end_time;
    freopen("../execution_time_of_servant.data", "r", stdin);
    freopen("data_process.data", "w", stdout);
    
    while( scanf("%d", &id1) != EOF )
    {
        scanf("%d", &start_time);
        scanf("%d", &end_time);
        scanf("%d", &id2);

        if( id2 == (id1 + 10) * 3 )
        {
            printf("%d %d\n", id1, (end_time - start_time)); // transit to micro second.
        }
        else
        {
            //printf("Servant %d had been preempted\n", id1);
            //printf("id: %d ; start_time %d\n", id1, start_time);
            //printf("id: %d ; end_time   %d\n", id2, end_time);
        }
    }

    fclose(stdin);
    fclose(stdout);

    return 0;
}
