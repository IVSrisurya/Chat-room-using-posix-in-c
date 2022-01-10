    // create some work to do
    struct data work1;
    struct data work2;
    struct data work3;
    struct data work4;
    struct data work5;

    printf("Enter 2 values to add: ");
    scanf("%d", &work1.a);
    scanf("%d", &work1.b);

    printf("Enter 2 values to subtract: ");
    scanf("%d", &work2.a);
    scanf("%d", &work2.b);

    printf("Enter 2 values to multiply: ");
    scanf("%d", &work3.a);
    scanf("%d", &work3.b);

    printf("Enter 2 values to divide: ");
    scanf("%d", &work4.a);
    scanf("%d", &work4.b);

    printf("\n");

    // initialize the thread pool
    pool_init();

    // submit the work to the queue
    pool_submit(&add,&work1);
    pool_submit(&sub,&work2);
    pool_submit(&mult,&work3);
    pool_submit(&div,&work4);
