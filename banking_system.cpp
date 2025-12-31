#include <stdio.h>   // Standard I/O library for functions
#include <stdlib.h>  // Standard library for memory allocation, process control
#include <string.h>  // String handling functions 
#include <dirent.h>  // Directory handling functions for listing files in a directory
#include <time.h>    // Time handling functions for working with timestamps


// Function to hash password using simple XOR
void hashPassword(const char* input, char* output) {
    for (int i = 0; i < strlen(input); i++) {
        output[i] = input[i] ^ 0xAA;  
    }
    output[strlen(input)] = '\0';   
}



// Log system-level events (account creation, deletion etc.)
void logEvent(const char* message) {
    FILE* flog = fopen("system.log", "a");
    time_t now = time(NULL);  // Current timestamp
    fprintf(flog, "[%s] %s\n", strtok(ctime(&now), "\n"), message);
    fclose(flog);
}



// Log account transactions (deposit/withdraw) to a separate file
void recordTransaction(int accNo, const char* action, float amount) {
    char filename[50];
    sprintf(filename, "trans_%d.txt", accNo);  // Format transaction log filename

    FILE* ftrans = fopen(filename, "a");
    time_t now = time(NULL);
    fprintf(ftrans, "[%s] %s: %.2f\n", strtok(ctime(&now), "\n"), action, amount);
    fclose(ftrans);
}



// Authenticate account using password
int authenticate(int accNo) {
    char input[50], stored[50], encInput[50];
    char passFile[50];
    sprintf(passFile, "pass_%d.txt", accNo);

    FILE* fpass = fopen(passFile, "r");
    if (!fpass) {
        printf("Password file not found.\n");
        return 0;
    }

    printf("Enter Password: ");
    scanf("%s", input);  // Input password
    hashPassword(input, encInput);  // Hash the input for comparison

    fscanf(fpass, "%s", stored);
    fclose(fpass);

    if (strcmp(encInput, stored) == 0) {
        return 1;  // Successful authentication
    } 
    
    else {
        printf("Incorrect password.\n");
        char message[100];
        sprintf(message, "Failed login attempt on account %d", accNo);
        logEvent(message);
        return 0;  // Failed authentication
    }
}



// Create a new account
void createAccount() {
    struct Account {
        int accountNumber;
        char name[100];
        float balance;
    } acc;

    char filename[50], balanceFile[50], passFile[50];
    char password[50], encPass[50];


    // Input account details
    printf("Enter Account Number: ");
    scanf("%d", &acc.accountNumber);

    sprintf(filename, "account_%d.txt", acc.accountNumber);
    FILE* fcheck = fopen(filename, "r");
    if (fcheck) {
        printf("Account already exists!\n");
        fclose(fcheck);
        return;
    }

    printf("Enter Name: ");
    scanf("%s", acc.name);

    printf("Enter Initial Balance: ");
    scanf("%f", &acc.balance);

    printf("Set Password: ");
    scanf("%s", password);
    hashPassword(password, encPass);  // Encrypt password


    // Create account text file with name and number
    FILE* ftxt = fopen(filename, "w");
    fprintf(ftxt, "%d\n%s\n", acc.accountNumber, acc.name);
    fclose(ftxt);


    // Create balance binary file
    sprintf(balanceFile, "balance_%d.dat", acc.accountNumber);
    FILE* fbal = fopen(balanceFile, "wb");
    fwrite(&acc.balance, sizeof(float), 1, fbal);
    fclose(fbal);


    // Save password to separate file
    sprintf(passFile, "pass_%d.txt", acc.accountNumber);
    FILE* fpass = fopen(passFile, "w");
    fprintf(fpass, "%s", encPass);
    fclose(fpass);


    // Log the event
    char message[100];
    sprintf(message, "Created account %d", acc.accountNumber);
    logEvent(message);

    printf("Account created successfully.\n");
}



// View account details
void viewAccount() {
    int accNo;
    char filename[50], balanceFile[50];
    char name[100];
    float balance;

    printf("Enter Account Number to View: ");
    scanf("%d", &accNo);
    if (!authenticate(accNo)) return;


    // Prepare file paths
    sprintf(filename, "account_%d.txt", accNo);
    sprintf(balanceFile, "balance_%d.dat", accNo);


    // Open and read account information
    FILE* ftxt = fopen(filename, "r");
    if (!ftxt) {
        printf("Account does not exist!\n");
        return;
    }
    fscanf(ftxt, "%*d\n%99s", name);  // Skip number, read name
    fclose(ftxt);


    // Read balance
    FILE* fbal = fopen(balanceFile, "rb");
    fread(&balance, sizeof(float), 1, fbal);
    fclose(fbal);


    // Display account data
    printf("\nAccount Number: %d\nName: %s\nBalance: %.2f\n", accNo, name, balance);
}



// Deposit money
void deposit() {
    int accNo;
    float amount, balance;
    char balanceFile[50];

    printf("Enter Account Number to Deposit: ");
    scanf("%d", &accNo);
    if (!authenticate(accNo)) return;

    sprintf(balanceFile, "balance_%d.dat", accNo);
    FILE* fbal = fopen(balanceFile, "rb+");
    fread(&balance, sizeof(float), 1, fbal);

    printf("Enter Amount to Deposit: ");
    scanf("%f", &amount);

    balance += amount;
    fseek(fbal, 0, SEEK_SET);
    fwrite(&balance, sizeof(float), 1, fbal);
    fclose(fbal);

    printf("Deposit successful. New balance: %.2f\n", balance);
    recordTransaction(accNo, "Deposit", amount);
}



// Withdraw money
void withdraw() {
    int accNo;
    float amount, balance;
    char balanceFile[50];

    printf("Enter Account Number to Withdraw: ");
    scanf("%d", &accNo);
    if (!authenticate(accNo)) return;

    sprintf(balanceFile, "balance_%d.dat", accNo);
    FILE* fbal = fopen(balanceFile, "rb+");
    fread(&balance, sizeof(float), 1, fbal);

    printf("Enter Amount to Withdraw: ");
    scanf("%f", &amount);

    if (balance < amount) {
        printf("Insufficient balance!\n");
    } else {
        balance -= amount;
        fseek(fbal, 0, SEEK_SET);
        fwrite(&balance, sizeof(float), 1, fbal);
        printf("Withdrawal successful. New balance: %.2f\n", balance);
        recordTransaction(accNo, "Withdrawal", amount);
    }

    fclose(fbal);
}



// List all accounts
void listAccounts() {
    DIR* dir = opendir(".");
    struct dirent* entry;
    printf("\nList of Accounts:\n");

    while ((entry = readdir(dir)) != NULL) {
        if (strncmp(entry->d_name, "account_", 8) == 0 && strstr(entry->d_name, ".txt")) {
            int accNo;
            char name[100];
            FILE* f = fopen(entry->d_name, "r");
            fscanf(f, "%d\n%99s", &accNo, name);
            fclose(f);
            printf("%d\t%s\n", accNo, name);
        }
    }

    closedir(dir);
}



// Delete an account and all associated files
void deleteAccount() {
    int accNo;
    char filename[50], balanceFile[50], passFile[50], transFile[50];

    printf("Enter Account Number to Delete: ");
    scanf("%d", &accNo);
    if (!authenticate(accNo)) return;


    // File paths
    sprintf(filename, "account_%d.txt", accNo);
    sprintf(balanceFile, "balance_%d.dat", accNo);
    sprintf(passFile, "pass_%d.txt", accNo);
    sprintf(transFile, "trans_%d.txt", accNo);


    // Remove files
    if (remove(filename) == 0 && remove(balanceFile) == 0 && remove(passFile) == 0) {
        remove(transFile); 
        printf("Account deleted successfully.\n");

        char message[100];
        sprintf(message, "Deleted account %d", accNo);
        logEvent(message);
    } else {
        printf("Account deletion failed.\n");
    }
}



// Update account holder's name
void updateAccount() {
    int accNo;
    char filename[50], newName[100];

    printf("Enter Account Number to Update: ");
    scanf("%d", &accNo);
    if (!authenticate(accNo)) return;

    sprintf(filename, "account_%d.txt", accNo);
    FILE* f = fopen(filename, "r+");

    int tempAcc;
    fscanf(f, "%d\n", &tempAcc);  // Read and skip old data

    printf("Enter New Name: ");
    scanf("%s", newName);

    fseek(f, 0, SEEK_SET);  // Overwrite file from start
    fprintf(f, "%d\n%s\n", accNo, newName);
    fclose(f);

    printf("Name updated successfully.\n");

    char message[100];
    sprintf(message, "Updated name for account %d", accNo);
    logEvent(message);
}



// Main Menu Loop
int main() {
    int choice;
    do {
        printf("\n=== Banking System Menu ===\n");
        printf("1. Create Account\n");
        printf("2. View Account\n");
        printf("3. Deposit\n");
        printf("4. Withdraw\n");
        printf("5. List All Accounts\n");
        printf("6. Delete Account\n");
        printf("7. Update Account Name\n");
        printf("8. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);


        // Handle user menu choices
        switch (choice) {
            case 1: createAccount(); break;
            case 2: viewAccount(); break;
            case 3: deposit(); break;
            case 4: withdraw(); break;
            case 5: listAccounts(); break;
            case 6: deleteAccount(); break;
            case 7: updateAccount(); break;
            case 8: printf("Exiting...\n"); break;
            default: printf("Invalid choice!\n");
        }
    } while (choice != 8);  // Loop until user exits

    return 0;
}

