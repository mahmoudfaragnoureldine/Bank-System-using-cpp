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
    std::ifstream accIn("data/accounts.csv");
    if (!accIn)
    {
        std::ofstream accOut("data/accounts.csv");
        accOut << "account_id,name,balance_cents\n";
    }

    std::ifstream ledgerIn("data/ledger.csv");
    if (!ledgerIn)
    {
        std::ofstream ledgerOut("data/ledger.csv");
        ledgerOut << "ts_iso,tx_id,type,from,to,amount_cents,note\n";
    }
}

/*
 * Loads accounts and transaction history from CSV files.
 */
void BankSystem::LoadFiles()
{
    std::ifstream accIn("data/accounts.csv");
    std::string line;
    std::getline(accIn, line); // skip header

    while (std::getline(accIn, line))
    {
        std::stringstream ss(line);
        std::string id, name, balanceStr;
        std::getline(ss, id, ',');
        std::getline(ss, name, ',');
        std::getline(ss, balanceStr);

        if (!balanceStr.empty() &&
            std::all_of(balanceStr.begin(), balanceStr.end(), ::isdigit))
        {
            accounts.push_back({id, name, std::stoll(balanceStr)});
        }
    }

    std::ifstream ledgerIn("data/ledger.csv");
    std::getline(ledgerIn, line); // skip header

    while (std::getline(ledgerIn, line))
    {
        std::stringstream ss(line);
        Transaction tx;
        std::string amountStr;

        std::getline(ss, tx.ts_iso, ',');
        std::getline(ss, tx.tx_id, ',');
        std::getline(ss, tx.type, ',');
        std::getline(ss, tx.from_id, ',');
        std::getline(ss, tx.to_id, ',');
        std::getline(ss, amountStr, ',');
        std::getline(ss, tx.note);

        if (!amountStr.empty() &&
            std::all_of(amountStr.begin(), amountStr.end(), ::isdigit))
        {
            tx.amount_cents = std::stoll(amountStr);
            transactions.push_back(tx);
        }
    }
}

/*
 * Persists accounts and transactions back to disk.
 */
void BankSystem::SaveFiles()
{
    std::ofstream accOut("data/accounts.csv");
    accOut << "account_id,name,balance_cents\n";
    for (const auto &acc : accounts)
    {
        accOut << acc.id << "," << acc.name << "," << acc.balance << "\n";
    }

    std::ofstream ledgerOut("data/ledger.csv");
    ledgerOut << "ts_iso,tx_id,type,from,to,amount_cents,note\n";
    for (const auto &tx : transactions)
    {
        ledgerOut << tx.ts_iso << "," << tx.tx_id << "," << tx.type << ","
                  << tx.from_id << "," << tx.to_id << ","
                  << tx.amount_cents << "," << tx.note << "\n";
    }
}

/*
 * Returns current local time formatted as ISO 8601.
 */
std::string BankSystem::getCurrentISOTime()
{
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm *tm = std::localtime(&t);

    std::ostringstream oss;
    oss << std::put_time(tm, "%Y-%m-%dT%H:%M:%S");
    return oss.str();
}

/*
 * Generates a zero-padded unique transaction ID.
 */
std::string BankSystem::generateTxId()
{
    std::ostringstream oss;
    oss << std::setw(10) << std::setfill('0') << ++tx_counter;
    return oss.str();
}

/*
 * Main command loop for the banking system.
 * Handles user input and dispatches supported operations.
 */
void BankSystem::Run()
{
    std::string command;

    while (true)
    {
        std::cout << "\nEnter command (list, create, deposit, withdraw, transfer, balance, statement, quit): ";
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
            Account acc{};
            std::cout << "Enter account id: ";
            std::cin >> acc.id;

            std::cout << "Enter name: ";
            std::cin >> acc.name;

            std::cout << "Enter initial balance (cents): ";
            std::cin >> acc.balance;

            accounts.push_back(acc);
            SaveFiles();

            std::cout << "Account created successfully.\n";
        }

        /* ===================== DEPOSIT ===================== */
        else if (command == "deposit")
        {
            std::string id, note;
            long long amount{};
            bool found{false};

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

                    std::cout << "Deposit successful.\n";
                    break;
                }
            }

            if (!found)
                std::cout << "Account not found.\n";
        }

        /* ===================== WITHDRAW ===================== */
        else if (command == "withdraw")
        {
            std::string id, note;
            long long amount{};
            bool found{false};

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

                    if (acc.balance < amount)
                    {
                        std::cout << "Not enough balance.\n";
                        break;
                    }

                    acc.balance -= amount;

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

                    std::cout << "Withdraw successful.\n";
                    break;
                }
            }

            if (!found)
                std::cout << "Account not found.\n";
        }

        /* ===================== TRANSFER ===================== */
        else if (command == "transfer")
        {
            std::string from_id, to_id, note;
            long long amount{};
            bool foundFrom{false}, foundTo{false};

            std::cout << "From account id: ";
            std::cin >> from_id;

            std::cout << "To account id: ";
            std::cin >> to_id;

            std::cout << "Enter amount (cents): ";
            std::cin >> amount;

            std::cout << "Enter note: ";
            std::cin.ignore();
            std::getline(std::cin, note);

            for (auto &from : accounts)
            {
                if (from.id == from_id)
                {
                    foundFrom = true;

                    if (from.balance < amount)
                    {
                        std::cout << "Not enough balance.\n";
                        break;
                    }

                    for (auto &to : accounts)
                    {
                        if (to.id == to_id)
                        {
                            foundTo = true;

                            from.balance -= amount;
                            to.balance += amount;

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

                            std::cout << "Transfer successful.\n";
                            break;
                        }
                    }
                    break;
                }
            }

            if (!foundFrom)
                std::cout << "From account not found.\n";
            else if (!foundTo)
                std::cout << "To account not found.\n";
        }

        /* ===================== BALANCE ===================== */
        else if (command == "balance")
        {
            std::string id;
            bool found{false};

            std::cout << "Account id: ";
            std::cin >> id;

            for (const auto &acc : accounts)
            {
                if (acc.id == id)
                {
                    std::cout << "Current balance: " << acc.balance << " cents.\n";
                    found = true;
                    break;
                }
            }

            if (!found)
                std::cout << "Account not found.\n";
        }

        /* ===================== STATEMENT ===================== */
        else if (command == "statement")
        {
            std::string id;
            int limit{};
            bool found{false};

            std::cout << "Account id: ";
            std::cin >> id;

            std::cout << "Enter limit: ";
            std::cin >> limit;

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
                std::cout << "Account not found.\n";
                continue;
            }

            for (int i = static_cast<int>(transactions.size()) - 1;
                 i >= 0 && limit > 0;
                 --i)
            {
                if (transactions[i].from_id == id ||
                    transactions[i].to_id == id)
                {
                    std::cout << transactions[i].ts_iso << ", "
                              << transactions[i].tx_id << ", "
                              << transactions[i].type << ", "
                              << transactions[i].from_id << ", "
                              << transactions[i].to_id << ", "
                              << transactions[i].amount_cents << ", "
                              << transactions[i].note << "\n";
                    --limit;
                }
            }
        }

        /* ===================== EXIT ===================== */
        else if (command == "quit")
        {
            std::cout << "Exiting system...\n";
            break;
        }

        else
        {
            std::cout << "Unknown command.\n";
        }
    }
}
