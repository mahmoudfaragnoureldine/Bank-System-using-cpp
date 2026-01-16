#if !defined(BankSystem_h)
#define BankSystem_h

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <cctype>

/*
 * Represents a bank account with a unique identifier,
 * owner name, and balance stored in cents.
 */
class Account
{
    public:
        std::string id;
        std::string name;
        long long balance{};
};

/*
 * Represents a single transaction entry in the ledger.
 */
class Transaction
{
    public:
        std::string ts_iso;        // ISO 8601 timestamp
        std::string tx_id;         // Unique transaction ID
        std::string type;          // deposit / withdraw / transfer
        std::string from_id;       // source account (if applicable)
        std::string to_id;         // destination account (if applicable)
        long long amount_cents{};  // transaction amount
        std::string note;          // user note
};

/*
 * Core banking system handling accounts, transactions,
 * persistence, and command-based interaction.
 */
class BankSystem
{
private:
    long long tx_counter{0};
    std::vector<Account> accounts;
    std::vector<Transaction> transactions;

    void ensureFilesExist();
    void LoadFiles();
    void SaveFiles();

    std::string getCurrentISOTime();
    std::string generateTxId();

public:
    void Init();
    void Run();
};

#endif // BankSystem_h
