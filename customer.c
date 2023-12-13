#include "local.h"

    int MINIMUM_SHOPPING_TIME, MAXIMUM_WAITING_TIME;
    int MAXIMUM_SHOPPING_TIME;

    int iamAlive = 0;

int readSuperMarketData(Item items[], int *itemCount){
    FILE* file = fopen("data.txt","r"); //open file

    if (file == NULL) {
        printf("CUSTOMER: ERROR OPENING THE FILE\n");
        return 0;
    }

    *itemCount = 0;;

    // read dat from the file into the array of items
    while(fscanf(file, "%s %d %f", items[*itemCount].name,&items[*itemCount].quantity,&items[*itemCount].price) == 3){
        // increment the item count
        (*itemCount)++;

        // check if the array is full
        if(*itemCount >= MAX_ITEMS){
            printf("CUSTOMER: TOO many items in the file.\n");
            break;
        }
    }

    // close the file
    fclose(file);
    return 1;
}

void printItemsInCart(ShoppingCart *cart){
    for(int i = 0;i<cart->itemCount; i++){
        printf("%s\n", cart->items[i].name);
    }

}

void addToCart(ShoppingCart *cart, Item*item){
    if(cart->itemCount < MAX_ITEMS){
        cart->items[cart->itemCount] = *item;
        cart->itemCount++;
    }
    else{
        printf("CUSTOMER: The cart is full.\n");
    }
}

void simulateShopping(ShoppingCart *cart,Item items[], int itemCount ){
    // simulate customer shopping for a random time (e.g 5 to 10 seconds)
    // int shoppingTime = rand() % 6 + 5; // how many items the customer will buy
      srand(time(NULL));
    //int shoppingTime = rand() % 6 +5; // how many items the customer will buy
     int shoppingTime = getRandom(MINIMUM_SHOPPING_TIME,MAXIMUM_WAITING_TIME);
     printf("GENERATED SHOPPING TIME:%d\n",shoppingTime);
    for(int time = 0; time< shoppingTime; time++){
        // generate random item index
        int randomItemIndedx = rand() % itemCount;

        // check if the item is in stock
        if(items[randomItemIndedx].quantity > 0){
            // decrement quantity by one and add item to the cart
            items[randomItemIndedx].quantity--;
            addToCart(cart, &items[randomItemIndedx]);
            printf("CUSTOMER {%d}: Added %s to the cart.\n", getpid(),items[randomItemIndedx].name);
        }

        else{
            printf("CUSTOMER: %s is out of stock.\n", items[randomItemIndedx].name);
        }
        // delay to simulate customer shopping speed
        sleep(rand() % 3 +1);
    }
    printf("CUSTOMER: ID = %d Has just finished shopping! \n", (int)getpid());

}

// double check this function since abd used struct, not typedef
int bestCashier(int cashiersNumber,int weights[]){
    pid_t ppid = getppid();
    if (ppid == -1){
         perror("CUSTOMER: Error getting parent id\n");
        exit(EXIT_FAILURE);
    }
    printf("CUSTOMER{%d}: is currently looking for best cashier!\n", (int)getpid());

    
    struct MEMORY * memory[cashiersNumber]; // to hold cashier status
    char          *shmptr;
    // connect to shared memory for each cashier
    for(int i =0 ; i < cashiersNumber; i++){

        int shmId = shmget(((int)getppid() + i), 0, 0);
        if(shmId == -1){
            printf("IN loop iteration {%d} i failed to connect to shmem %d!\n", i, ((int)getppid() + i));
            perror("CUSTOMER: Error connecting to shared memory");
            exit(EXIT_FAILURE);
        }
        if ( (shmptr = (struct MEMORY *) shmat(shmId, NULL, 0)) == (char *) -1 ) {
            perror("shmptr -- parent -- attach");
            exit(1);
        }
        memory[i] = (struct MEMORY *)shmptr;
        printf("CUSTOMER{%d}: Just read Shmem %d\n Size %d Number of items%d time to scan %d behaviour %d\n", (int)getpid(), i, memory[i]->queueSize, memory[i]->numberOfItems , memory[i]->timeToScan, memory[i]->behaviour);
    }

    /* The Process of finding the best cashier */
    int evaluation[cashiersNumber]; 
    for(int i =0; i<cashiersNumber;i++){
        evaluation[i] = memory[i]->queueSize*weights[0] + memory[i]->numberOfItems * weights[1] + memory[i]->timeToScan * weights[2] + memory[i]->behaviour*weights[3];
    }
    int max = evaluation[0], index = 0;
    for(int i=0;i<cashiersNumber;i++){
        if(max < evaluation[i]){
            max=evaluation[i];
            index = i;
        }
    }


    printf("CUSTOMER{%d}: I have decided to go with cashier index = [%d] \n", (int)getpid(), index);
    return index;
}

void leaveQueue(int signum){
    if(iamAlive == 0){ //
            kill(getppid(), SIGUSR1);
            printf("CUSTOMER {%d}: Can't wait in the queue %d\n", getpid(),getpid());
            sleep(5);
            connectTOGUIQueue(1);//connect to gui queue, pass 1 (leaving)
            exit(EXIT_FAILURE);
        }
      else{
        // mayble add another connection to GUI queue also here
        printf("customer {%d} ALARM HAS RING, BUT IAM NOT LEAVING SINCE  ITS MY TURN\n",getpid());
      }  
    }


void connectTOGUIQueue(int flag){
     // send MESSAGE TO MESSAGE GUI
    //FIRST GET KEY
    // SECOND CONNECT TO THE MESSAGE QUEUE
    // SEND THE MESSAGE TO THE QUEUE
    __key_t key2 = ftok(".",GUISEED);
     if (key2 == -1){
        perror("CUSTOMER: Error creating key  GUI QUEUE.\n");
        exit(EXIT_FAILURE);
    }

    //connect 
    int msgid2 = msgget(key2, 0); // get msg queue id
    if (msgid2 == -1){
         perror("CUSTOMER: Error making the message queue for the GUI\n");
        exit(EXIT_FAILURE);
    }
    //create the message
    MESSAGEGUI guiMessage;
    guiMessage.customerId =(int) getpid();
    guiMessage.cashierId = 0; // I think it must be modified, otherwise how we can show to which cashier should the customer go
    guiMessage.flag = flag;
    guiMessage.msgtype = SERVER;
    guiMessage.sentBy = 1;
    guiMessage.total = 0;

    // send the message
    int error = msgsnd(msgid2,&guiMessage,sizeof(guiMessage),0);
    if(error == -1){
        perror("CUSTOMER: Error sending the message to the GUI queue\n");
        exit(EXIT_FAILURE);
    }
    
}


// here check 'SEED', and index-1 (since I have started from 1 not 0)
void connect_to_the_message_queue(int index, ShoppingCart cart){
    // create key
    __key_t key = ftok(".", SEED + index );
    if (key == -1){
        perror("CUSTOMER: Error creating key\n");
        exit(EXIT_FAILURE);
    }

    int msgid = msgget(key, 0); // get msg queue id
    if (msgid == -1){
         perror("CUSTOMER: Error making the message queue\n");
        exit(EXIT_FAILURE);
    }

    // create a message
    printf("CUSTOMER: Id = %d CONNECTED TO MSGQID = %d\n", getpid(), msgid);
    MESSAGE msg;
    msg.msg_type = SERVER;
    msg.cart = cart;
    msg.clientId = getpid(); // get customer process ID


    // send the message to the cashier
    int err = msgsnd(msgid, &msg, sizeof(msg), 0);
    if(err == -1){
         perror("CUSTOMER: Error sending the message to the cashier queue\n");
        exit(EXIT_FAILURE);
    }

    connectTOGUIQueue(2); // connect to the GUI queue, and pass the floag = 0(not leaving)


    if(sigset(SIGALRM, leaveQueue)){ // to handle the alarm signal
        perror("Sigset can not set SIGALRM");
        exit(SIGALRM);
    }
    printf("CUSTOMER: Id = %d has just sent a message and now im sleeping !\n", getpid());
    alarm(MAXIMUM_WAITING_TIME); 
   
    while(1){
        pause();
    }
}

    

    void stillAlive(int signum){
        iamAlive = 1; // don't leave the supermarket
        printf("CUSTOMER {%d} yes I am still availible\n\n",getpid());
        while(1){
            pause();
        }
    }

    void recieveCashierMessage(int signum){
        printf("CUSTOMER: Customer %d has finished.\n", getpid());
        exit(EXIT_SUCCESS);
    }


int main(int args, char*argv[]){
    
    connectTOGUIQueue(0);



    int thresholds[12];
    int count;
    count = readThresholds(thresholds);
    MAXIMUM_SHOPPING_TIME = thresholds[3];
    MINIMUM_SHOPPING_TIME = thresholds[2];
    MAXIMUM_WAITING_TIME = thresholds[6];

    printf("MAXIMUM SHOPPING TIME: %d", MAXIMUM_SHOPPING_TIME);
    printf("MINIMUN SHOPPING TIME: %d", MINIMUM_SHOPPING_TIME);
    printf("Waiting TIME: %d", MAXIMUM_WAITING_TIME);



    prctl(PR_SET_PDEATHSIG, SIGHUP); // GET A SIGNAL WHEN PARENT IS KILLED
    if(sigset(SIGUSR1, stillAlive) == -1){ // to handle the signal sent by the cashier to check if the customer is still alive
        perror("Sigset can not set SIGUSR1");
        exit(SIGUSR1);
    }
    if(sigset(SIGUSR2, recieveCashierMessage) == -1){
        perror("Sigset can not set SIGUSR2");
        exit(SIGUSR2);
    }
    int numberOfCashier = 0; // passed by argument
  

    if (args < 2){
        perror("CUSTOMER: Number of args is less than 2\n");
        exit(EXIT_FAILURE);
    }
    else{
        numberOfCashier = atoi(argv[1]);
    }

    Item items[MAX_ITEMS]; // array of items to read the file
    int itemCount; // number of items bought;
    // read super market data from file
    if(!readSuperMarketData(items, &itemCount)){
        return -1;
    }

    // seed the random number generator with the current time
    srand(time(NULL));
    //SIMULATE SHOPPING
    printf("CUSTOMER: {%d} has started shopping\n",getpid());
    ShoppingCart cart;
    cart.itemCount = 0;
    simulateShopping(&cart, items, itemCount);

    // choose the best cashier
    int wieghts[4];
    wieghts[0] = -1;
    wieghts[1] = -1;
    wieghts[2] = -2;
    wieghts[3] = 1;
    int best_cashier_index = bestCashier(numberOfCashier, wieghts);

    // connect to the message queue
    connect_to_the_message_queue(best_cashier_index, cart);

    return 0;
}
