#pragma once
#include "Algo.h"
#include "OrderBook.h"
#include "Wallet.h"

#include <string>
#include <vector>

class Bot
{
    public:
        Bot();
        /**gets the bot running; main() function*/
        void startBot();

    private:
        /**prompts user to enter deposit into the wallet*/
        void setupWallet();
        /**method that contains the main running functions of the bot; the loop content*/
        void marketAnalysis();
            /**analyse the market for SMA and slope gradient*/
            void analysis();
            /**calculate SMA(simple moving average) of both ASK and BID prices in the market*/
            void calcSMA(int n);
            /**calculate sub-section of the slopes in the graph to determine gradient of slope*/
            void calcMovingSlope(int counter);
            /**go thru diff currency ASK prices*/
            void loopThruExchangeAskPrices();
            /**go thru diff currency BID prices*/
            void loopThruExchangeBidPrices();
            /**MerkelMain's gotoNextTimeFrame*/
            void nextTimeframe();

        /**enter bot's ASK in the exchange*/
        void enterBotAsk(std::string product, double price, double amount);
        /**enter bot's BID in the exchange*/
        void enterBotBid(std::string product, double price, double amount);
        /**prints out all ASKS and BIDS and SALES*/
        void log();
            /**gets and prints out profit of exchange*/
            void walletSummary();


        //HELPER FNS
        /**get average ASK price from the current timestamp*/
        double getAvgPriceForAsk(std::string& timestamp, std::string& product); //for market
        /**get average BID price from the current timestamp*/
        double getAvgPriceForBid(std::string& timestamp, std::string& product); //for market
        /**get minimum ASK price and maximum BID price from the current timestamp to calculate SMA*/
        void getMinAskAndMaxBidForMarket(vector<OrderBookEntry>& asks, vector<OrderBookEntry>& bids);
        /**get a section of an array to be used as the moving window of prices*/        
        std::vector<double> getSubVector(std::vector<double> &priceVec, int startIndex, int endIndex);
        /**check for a positive slope; increment in prices of linear graph*/
        bool positiveSlope(double slope);
        /**check for a negative slope; decrement in prices of linear graph*/
        bool negativeSlope(double slope);
        /**gets average successful ask sale price*/
        double getAvgAskSalePrice();
        /**gets average successful bid sale price*/
        double getAvgBidSalePrice();
        /**prints out all asks placed by bot to new file*/
        void printHistoricalDataAskFile();
        /**prints out all bids placed by bot to new file*/
        void printHistoricalDataBidFile();
        /**prints out all sales and average successful sale price for asks/bids placed by bot to new file*/
        void printHistoricalSalesFile();

        //vars
        Algo algo;
        Wallet wallet;
        OrderBook orderBook{"20200601.csv"}; //read CSV file name
        std::string currentTime;  //current Timestamp variable
        std::vector<std::string> botCurrencies = {"BTC/USDT"};  //single product for now

        int windowSize = 10; //double to store moving window size for SMA and moving slope (LR/SMA)
        double marketCounter; //counter to check for empty entries
        double profitPercent = 1.001; //set the profit limit for the bot's looping

        //LR
        std::vector<double> priceVecAsks, priceVecBids, askTimeX, bidTimeX; //vector to store values for LR (analysis)
        double slopeAsk, slopeBid; //double to store slope gradient for ask/bid (LR)
        double interceptAsk, interceptBid; //double to store y-intercept for ask/bid (LR)
        std::vector<double> slopeAskVec, slopeBidVec; //vector to store all the slope gradients (LR)
        
        // double predictAskValue, predictBidValue; //double to store predicted values and ask/bid prices //not used now

        //SMA
        double askSMA=0, bidSMA=0; //double to store current SMA for ask/bid (SMA)
        double sumAskSMA, sumBidSMA; //double to store sum of ask/bid over a period of time (SMA)

        //For withdrawing
        double prevAskPrice, prevBidPrice;
        
        //LOG
        double initialBalanceUSDT, endBalanceUSDT; //wallet currency for log
        std::vector<OrderBookEntry> historicalDataAsksPlaced, historicalDataBidsPlaced, historicalSales; //vectors to store transaction histories

};
