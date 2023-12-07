#include "local.h"

int readSuperMarketData(Item items[], int *itemCount){
    FILE* file = fopen("data.txt","r"); //open file

    if (file == NULL) {
        printf("ERROR OPENING THE FILE\n");
        return 0;
    }

    *itemCount = 0;;

    // read dat from the file into the array of items
    while(fscanf(file, "%s %d %f", items[*itemCount].name,&items[*itemCount].quantity,&items[*itemCount].price) == 3){
        // increment the item count
        (*itemCount)++;

        // check if the array is full
        if(*itemCount >= MAX_ITEMS){
            printf("TOO many items in the file.\n");
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
        printf("The cart is full.\n");
    }
}

void simulateShopping(ShoppingCart *cart,Item items[], int itemCount ){
    // simulate customer shopping for a random time (e.g 5 to 10 seconds)
    int shoppingTime = rand() % 6 +5; // how many items the customer will buy
    for(int time = 0; time< shoppingTime; time++){
        // generate random item index
        int randomItemIndedx = rand() % itemCount;

        // check if the item is in stock
        if(items[randomItemIndedx].quantity > 0){
            // decrement quantity by one and add item to the cart
            items[randomItemIndedx].quantity--;
            addToCart(cart, &items[randomItemIndedx]);
            printf("Added %s to the cart.\n", items[randomItemIndedx].name);
        }

        else{
            printf("%s is out of stock.\n", items[randomItemIndedx].name);
        }
        // delay to simulate customer shopping speed
        sleep(rand() % 3 +1);
    }

}

void leaveQueue(int signum){
    kill(getppid(), SIGUSR2);
    printf("Can't wait in the queue %d", getpid());
    exit(EXIT_FAILURE);
}

void stillAlive(int signum){
    while(1){
        pause();
    }
}

void recieveCashierMessage(int signum){
    printf("Customer %d has finished.\n", getpid());
    exit(EXIT_SUCCESS);
}

// double check this function since abd used struct, not typedef
int bestCashier(int cashiersNumber,int weights[]){
    // for loop, read from each cashier shared memory
    pid_t ppid = getppid(); // parent pid
    if (ppid == -1){
         perror("Error getting parent id\n");
        exit(EXIT_FAILURE);
    }

    int shmId;
    struct MEMORY * memory[cashiersNumber]; // to hild cashier status

    // connect to shared memory for each cashier
    for(int i =0; i < cashiersNumber; i++){
        shmId = shmget(((int)ppid) + i, sizeof(memory), IPC_CREAT | 0666);
        if(shmId == -1){
            perror("Error connecting to shared memory\n");
            exit(EXIT_FAILURE);
        }
        // make the 'memory point to the shared memory'
        memory[i] = (struct MESSAGE *)shamt((shmId),NULL,0);
    }

    // let's compare cashiers
    int evaluation[cashiersNumber]; // to calculate rating of each cashier
    for(int i =0; i<cashiersNumber;i++){
        evaluation[i] = memory[i]->queueSize*weights[0] + memory[i]->numberOfItems * weights[1] + memory[i]->timeToScan * weights[2] + memory[i]->behaviour*weights[3];
    }

    int max = evaluation[0], index = 0;

    // find the highest evaluation
    for(int i=0;i<cashiersNumber;i++){
        if(max < evaluation[i]){
            max=evaluation[i];
            index = i;
        }
    }




    return index; // this must be modified to return the best cashier index

}



// here check 'SEED', and index-1 (since I have started from 1 not 0)
void connect_to_the_message_queue(int index, ShoppingCart cart){
    // create key
    __key_t key = ftok(".", SEED + index );
    if (key == -1){
        perror("Error creating key\n");
        exit(EXIT_FAILURE);
    }

    int msgid = msgget(key, IPC_CREAT | 0666); // get msg queue id
    if (msgid == -1){
         perror("Error making the message queue\n");
        exit(EXIT_FAILURE);
    }

    // create a message
    MESSAGE msg;
    msg.msg_type = index;
    msg.cart = cart;
    msg.clientId = getpid(); // get customer process ID

    // send the message to the cashier
    int err = msgsnd(msgid, &msg, sizeof(msg) - sizeof(long), 0);
    if(err == -1){
         perror("Error sending the message\n");
        exit(EXIT_FAILURE);
    }

    signal(SIGALRM, leaveQueue); // to handle the alarm signal
    alarm(20); // wait 20 second in the queue
   
    while(1){
        pause();
    }

    signal(SIGUSR1, stillAlive); // to handle the signal sent by the cashier to check if the customer is still alive

    signal(SIGUSR2, recieveCashierMessage);
    // to handle the signal sent by the cashier that the payment went well.


}


int main(int args, char*argv[]){
    prctl(PR_SET_PDEATHSIG, SIGHUP); // GET A SIGNAL WHEN PARENT IS KILLED
    int numberOfCashier = 0; // passed by argument

    if (args < 2){
        perror("Number of args is less than 2\n");
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
    ShoppingCart cart;
    cart.itemCount = 0;
    simulateShopping(&cart, items, itemCount);

    // choose the best cashier
    int wieghts[4];
    wieghts[0] = -1;
    wieghts[1] = -1;
    wieghts[2] = 2;
    wieghts[3] = 1;
    int best_cashier_index = bestCashier(numberOfCashier, wieghts);

    // connect to the message queue
    connect_to_the_message_queue(best_cashier_index, cart);

    return 0;



}
