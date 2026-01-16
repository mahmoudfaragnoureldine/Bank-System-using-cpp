#include "BankSystem.h"

void BankSystem::Init()
{
    ensureFilesExist();
    LoadFiles();
}

/*
 * Ensures required data files exist.
 * Creates CSV files with headers if missing.
 */
void BankSystem::ensureFilesExist()
{
    // accounts file
    std::ifstream accIn("../data/accounts.csv");
    if (!accIn)
    {
        std::ofstream accOut("../data/accounts.csv");
        if (accOut.is_open())
        {
            accOut << "account_id,name,balance_cents\n";
            accOut.close();
            std::cout << "accounts.csv is created and written successfully!\n";
        }
        else
            std::cout << "Failed to create accounts.csv!" << std::endl; 
    }

    // ledger file
    std::ifstream ledgerIn("../data/ledger.csv");
    if (!ledgerIn)
    {
        std::ofstream ledgerOut("../data/ledger.csv");
        if (ledgerOut.is_open())
        {
            ledgerOut << "ts_iso,tx_id,type,from,to,amount_cents,note\n";
            ledgerOut.close();
            std::cout << "ledger.csv is created and written successfully!\n";
        }
        else
            std::cout << "Failed to create ledger.csv!" << std::endl; 
    }
}

/*
 * Loads accounts and transaction history from CSV files.
 */
void BankSystem::LoadFiles()
{
    // accounts.csv
    std::ifstream accIn("../data/accounts.csv");
    if (!accIn)
    {
        std::cout << "Failed to open accounts.csv for reading!\n";
        return;
    } 

    std::string accline;
    std::getline(accIn, accline); // skip header
    
    while (std::getline(accIn, accline))   // retun false when reaching end of the file
    {
        /* parse line */
        std::stringstream parseLine(accline);
        std::string id, name, balanceStr;
        std::getline(parseLine, id, ',');
        std::getline(parseLine, name, ',');
        std::getline(parseLine, balanceStr);
        //long long balance = std::stoll(balanceStr);
        if (!balanceStr.empty() && std::all_of(balanceStr.begin(), balanceStr.end(), ::isdigit)) 
        {
            long long balance = std::stoll(balanceStr);
            accounts.push_back({id, name, balance});
        } 
        else
        {
            std::cout << "Invalid amount in account.csv: " << balanceStr << "\n";
        }
    }

    // ledger.csv
    std::ifstream ledgerIn("../data/ledger.csv");
    if (!ledgerIn)
    {
        std::cout << "Failed to open ledger.csv for reading!\n";
        return;
    }
    
    std::string ledgline;
    std::getline(ledgerIn, ledgline); // skip header
    
    while (std::getline(ledgerIn, ledgline))   // retun false when reaching end of the file
    {
        /* parse line */
        std::stringstream parseLine(ledgline);
        std::string ts_iso, tx_id, type, from_id, to_id, amount_centsStr, note;
        std::getline(parseLine, ts_iso, ',');
        std::getline(parseLine, tx_id, ',');
        std::getline(parseLine, type, ',');
        std::getline(parseLine, from_id, ',');
        std::getline(parseLine, to_id, ',');
        std::getline(parseLine, amount_centsStr, ',');
        std::getline(parseLine, note);
        //long long amount_cents = std::stoll(amount_centsStr);
        if (!amount_centsStr.empty() && std::all_of(amount_centsStr.begin(), amount_centsStr.end(), ::isdigit)) 
        {
            long long amount_cents = std::stoll(amount_centsStr);
            transactions.push_back({ts_iso, tx_id, type, from_id, to_id, amount_cents, note});
        } 
        else
        {
            std::cout << "Invalid amount in ledger.csv: " << amount_centsStr << "\n";
        }
    }
}

/*
 * Persists accounts and transactions back to disk.
 */
void BankSystem::SaveFiles() {
    // 1) accounts.csv
    std::ofstream accWrite("../data/accounts.csv");
    if (!accWrite)
    {
        std::cout << "Failed to open accounts.csv for writing!\n";
        return;
    }
    // write header
    accWrite << "account_id,name,balance_cents\n";
    // loop in every account
    for (const auto &acc : accounts)
    {
        accWrite << acc.id << "," << acc.name << "," << acc.balance << std::endl;
    }
    
    // 2) ledger.csv
    std::ofstream ledgWrite("../data/ledger.csv");
    if(!ledgWrite)
    {
        std::cout << "Failed to open ledge.csv for writing!\n";
        return;
    }
    // write header
    ledgWrite << "ts_iso,tx_id,type,from,to,amount_cents,note\n";
    // loop in every transaction
    for (const auto &tx : transactions)
    {
        ledgWrite << tx.ts_iso << "," 
            << tx.tx_id << "," 
            << tx.type << "," 
            << tx.from_id << "," 
            << tx.to_id << "," 
            << tx.amount_cents << "," 
            << tx.note << std::endl;
    }
    
}

/*
 * Returns current local time formatted as ISO 8601.
 */
std::string BankSystem::getCurrentISOTime() {
    auto now = std::chrono::system_clock::now();    // get current time as time_point
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);   // from time_point to time_t   
    std::tm* local = std::localtime(&now_c);    // from time_t to struct tm
    std::ostringstream oss;
    oss << std::put_time(local, "%Y-%m-%dT%H:%M:%S");  // ISO
    std::string ts_iso = oss.str();
    return ts_iso;
}

/*
 * Generates a zero-padded unique transaction ID.
 */
std::string BankSystem::generateTxId() {
    tx_counter++;
    std::ostringstream oss;
    oss << std::setw(10) << std::setfill('0') << tx_counter;
    return oss.str();
}

/*
 * Main command loop for the banking system.
 * Handles user input and dispatches supported operations.
 */
void BankSystem::Run() {
    std::string command;

    while (true)
    {
        std::cout << "Enter command (list, create, deposit, withdraw, transfer, balance, statement, quit): ";
        std::cin >> command;

        /* ===================== LIST ACCOUNTS ===================== */
        if (command == "list")
        {
            for (const auto &acc : accounts)
            {
                std::cout << acc.id << " | "
                        << acc.name << " | "
                        << acc.balance << " cents\n";
            }
        }

        /* ===================== CREATE ACCOUNT ===================== */
        else if (command == "create")
        {
            Account acc;
            std::cout << "Enter account id: ";
            std::cin >> acc.id;

            std::cout << "Enter name: ";
            std::cin >> acc.name;

            std::cout << "Enter initial balance (cents): ";
            std::cin >> acc.balance;

            accounts.push_back(acc);
            SaveFiles();

            std::cout << "Account created successfully!\n";
        }

        /* ===================== DEPOSIT ===================== */
        else if (command == "deposit")
        {
            std::string id;
            long long amount;
            std::string note;
            bool found = false;

            std::cout << "Account id: ";
            std::cin >> id;

            std::cout << "Enter amount (cents): ";
            std::cin >> amount;

            std::cout << "Enter note: ";
            std::cin.ignore();
            std::getline(std::cin, note);

            for (auto &acc : accounts)
            {
                if (acc.id == id)
                {
                    acc.balance += amount;
                    found = true;
                    
                    // add the transaction
                    Transaction tx;
                    tx.ts_iso = getCurrentISOTime();
                    tx.tx_id = generateTxId();
                    tx.type = "deposit";
                    tx.from_id = "";
                    tx.to_id = id;
                    tx.amount_cents = amount;
                    tx.note = note;

                    transactions.push_back(tx);
                    SaveFiles();

                    std::cout << "Deposit successful!\n";
                    break;
                }
            }

            if (!found)
            {
                std::cout << "Account not found!\n";
            }
        }
        
        /* ===================== WITHDRAW ===================== */
        else if (command == "withdraw")
        {
            std::string id;
            long long amount;
            std::string note;
            bool found = false;
            bool enoughBalance = false;

            std::cout << "Account id: ";
            std::cin >> id;

            std::cout << "Enter amount (cents): ";
            std::cin >> amount;

            std::cout << "Enter note: ";
            std::cin.ignore();
            std::getline(std::cin, note);

            for (auto &acc : accounts)
            {
                if (acc.id == id)
                {
                    found = true;
                    if (acc.balance >= amount)
                    {
                        enoughBalance = true;
                        acc.balance -= amount;
                    
                        // add the transaction
                        Transaction tx;
                        tx.ts_iso = getCurrentISOTime();
                        tx.tx_id = generateTxId();
                        tx.type = "withdraw";
                        tx.from_id = id;
                        tx.to_id = "";
                        tx.amount_cents = amount;
                        tx.note = note;

                        transactions.push_back(tx);
                        SaveFiles();

                        std::cout << "Withdraw successful!\n";
                        break;
                    }
                    
                }
            }
            if (!found)
            {
                std::cout << "Account not found!\n";
            }
            else if (!enoughBalance)
            {
                std::cout << "Not enough balance\n";
            }
        }

        /* ===================== TRANSFER ===================== */
        else if (command == "transfer")
        {
            std::string from_id;
            std::string to_id;
            long long amount;
            std::string note;
            bool foundFrom = false;
            bool foundTo = false;
            bool enoughBalance = false;

            std::cout << "Enter From account id: ";
            std::cin >> from_id;

            std::cout << "Enter To account id: ";
            std::cin >> to_id;

            std::cout << "Enter amount (cents): ";
            std::cin >> amount;

            std::cout << "Enter note: ";
            std::cin.ignore();
            std::getline(std::cin, note);

            for (auto &accFrom : accounts)
            {
                if (accFrom.id == from_id)
                {
                    foundFrom = true;
                    for (auto &accTo : accounts)
                    {
                        if (accTo.id == to_id)
                        {
                            foundTo = true;
                            if (accFrom.balance >= amount)
                            {
                                enoughBalance = true;
                                accFrom.balance -= amount;
                                accTo.balance += amount;

                                Transaction tx;
                                tx.ts_iso = getCurrentISOTime();
                                tx.tx_id = generateTxId();
                                tx.type = "transfer";
                                tx.from_id = from_id;
                                tx.to_id = to_id;
                                tx.amount_cents = amount;
                                tx.note = note;

                                transactions.push_back(tx);
                                SaveFiles();

                                std::cout << "Transfer successful!\n";
                            } 
                            break;
                        }
                    } 
                    break;
                }
            }
            if (!foundFrom)
            {
                std::cout << "From Account not fount\n";
            }
            else if (!foundTo)
            {
                std::cout << "To Account not found\n";
            }
            else if (!enoughBalance)
            {
                std::cout << "Not enough balance\n";
            }
        }
        
        /* ===================== BALANCE ===================== */
        else if (command == "balance")
        {
            std::string id;
            bool found = false;

            std::cout << "Enter account id: ";
            std::cin >> id;
            
            for (const auto &acc : accounts)
            {
                if (acc.id == id)
                {
                    found = true;
                    std::cout << "Current balance: " << acc.balance << " cents.\n";
                    break;
                }
            }
            if (!found)
                std::cout << "Account not found!\n";
        }
        
        /* ===================== STATEMENT ===================== */
        else if (command == "statement")
        {
            std::string id;
            int limit;
            bool found = false;
            bool foundtransction = false;

            std::cout << "Enter acount id: ";
            std::cin >> id;

            std::cout << "Enter limit: ";
            std::cin>> limit;

            for (const auto &acc : accounts)
            {
                if (acc.id == id)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                std::cout << "Account not found!\n";
            }
            for (int i = transactions.size() - 1; i >= 0 && limit > 0; i--)
            {
                if (transactions[i].from_id == id || transactions[i].to_id == id)
                {
                    foundtransction = true;
                    std::cout << transactions[i].ts_iso << ", " << transactions[i].tx_id << ", " << transactions[i].type 
                        << ", " << transactions[i].from_id << ", " << transactions[i].to_id 
                        << ", " << transactions[i].amount_cents << ", " << transactions[i].note << "\n"; 
                    --limit;
                }
            }
            if (!foundtransction)
            {
                std::cout << "No transactions found for this account.\n";
            }  
        }

        /* ===================== EXIT ===================== */
        else if (command == "quit")
        {
            std::cout << "Exiting system...\n";
            break;
        }
        else
            std::cout << "Unknown command...\n";
           
    }
    
}
