#include "Bot.h"
#include <iostream>
#include <fstream>
#include <chrono>

Bot::Bot()
{
}

void Bot::startBot()
{
    Algo algo;
    currentTime = orderBook.getEarliestTime();
    
    setupWallet();

    std::cout<<"Press 1 to continue."<<std::endl;
    std::string input;
    try
    {
        getline(std::cin, input);
        if(input == "1")
        {
            marketAnalysis();
            log();
        }
    }
    catch(const std::exception& e)
    {
        // 
    }
}

void Bot::setupWallet()
{
    //get user to input amount for wallet
    std::cout<<"Enter amount of BTC to be initialized in wallet: "<<std::endl;
    std::string amount;
    try
    {
        getline(std::cin, amount);
        wallet.insertCurrency("BTC",stod(amount));

        std::vector<OrderBookEntry> entries = orderBook.getOrders(OrderBookType::ask, botCurrencies[0], currentTime);
        // initialBalanceUSDT = stod(amount)*orderBook.getAvgPrice(entries);
        // std::cout<<"Wallet is worth "<< initialBalanceUSDT << " USDT now."<<std::endl;
        std::cout<<"Wallet is worth "<< amount << " BTC now."<<std::endl;
        std::cout<<"Bot will run until "<< (profitPercent-1)*100 << "% profit has been achieved."<<std::endl;
    }
    catch(const std::exception& e)
    {
        // 
    }
}

void Bot::marketAnalysis()
{   
    auto startTime = std::chrono::system_clock::now();
    int counter=0;
    int walletAmount = wallet.getCurrencyAmount("BTC");

        //loop until profit has reached
        while(!wallet.containsCurrency("BTC", walletAmount*profitPercent)) //0.01% increase (benchmark)
        // while(counter < 435)
        {
                //keep track of how many loops
                std::cout<<"Counter: "<<counter<<std::endl; 
                analysis();
                //gather data till enough for analysis
                if(counter>windowSize)
                {
                    loopThruExchangeAskPrices(); 
                    loopThruExchangeBidPrices();
                }
                nextTimeframe(); 
                counter++;
        }
        //only sort orders at the end based on timestamps
        orderBook.sortOrders(); 
        std::cout<<"Profit of " << (profitPercent-1)*100 << "% has been achieved. Bot has finished running. \n"<<std::endl;

        //keep track of time taken for the bot
        auto endTime = std::chrono::system_clock::now();
        auto timeTaken = (std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count() / 1000);
        std::cout<<"Total time taken: " << timeTaken << " seconds." << std::endl;
}

/**analyse the market for SMA and slope gradient*/
void Bot::analysis()
{
    //seperate into asks and bids entries to find min ask price and max bid price
    std::vector<OrderBookEntry> asks = orderBook.getOrders(OrderBookType::ask, botCurrencies[0], currentTime);
    std::vector<OrderBookEntry> bids = orderBook.getOrders(OrderBookType::bid, botCurrencies[0], currentTime);
    //calculate SMA and slope gradients
    getMinAskAndMaxBidForMarket(asks, bids);
}  

//takes in current counter of cycles the bot has gone through
void Bot::calcSMA(int n)    
{
    //adds the min price of the latest OrderBookEntries to sumAskSMA and sumBidSMA
    sumAskSMA += priceVecAsks[n];   
    sumBidSMA += priceVecBids[n];   

    //if the no. of cycles the bot has gone through > windowSize, 
    if(n >= windowSize)     
    {
        //check for the oldest index within the vector to subtract
        int replaceIndex = n - windowSize;     
        //subtracts the min and max price of the oldest OrderBookEntries     
        sumAskSMA -= priceVecAsks[replaceIndex];    
        sumBidSMA -= priceVecBids[replaceIndex];    
    }

    //when the bot has gone through the same number of cycles as the value of windowSize or more
    if(n >= (windowSize-1))  
    {
        //calculates the current SMA values by dividing with the number of elements (windowSize)
        askSMA = sumAskSMA / windowSize;    
        bidSMA = sumBidSMA / windowSize;    
    }
        // cout << "ask SMA: " << askSMA <<endl;
        // cout << "bid SMA: " << bidSMA <<endl;
}

/**calculate sub-section of the slopes in the graph to determine gradient of slope*/
//takes in current counter of cycles the bot has gone through
void Bot::calcMovingSlope(int n)     
{
    std::vector<double> subVecAsks, subVecBids, subTimeAskX, subTimeBidX;

        //if the no. of cycles the bot has gone through > windowSize, 
        if(n >= windowSize)         
        {
            //gets the correct starting index for windowSize amount of values
            int startIndex = n - windowSize;    
            //extracts sub vector of windowSize values for asks and bids
            subVecAsks = getSubVector(priceVecAsks, startIndex, n);     
            subVecBids = getSubVector(priceVecBids, startIndex, n);  
            //extracts sub vector of windowSize values for ask and bid time   
            subTimeAskX = getSubVector(askTimeX, startIndex, n);        
            subTimeBidX = getSubVector(bidTimeX, startIndex, n);        
        }

        //when the bot has gone through the same number of cycles as the value of windowSize or more
        if(n >= (windowSize-1))     
        {
            //calculate slope gradient for asks and bids
            slopeAsk = algo.slope(subTimeAskX, subVecAsks);             
            slopeBid = algo.slope(subTimeBidX, subVecBids);             
            //calculate y-intercept for asks and bids
            interceptAsk = algo.intercept(subTimeAskX, subVecAsks, slopeAsk);   
            interceptBid = algo.intercept(subTimeBidX, subVecBids, slopeBid);   
            // cout<<"slopeAsk : "<<slopeAsk<<endl;
            // cout<<"slopeBid : "<<slopeBid<<endl;
        }
}

/**go through diff currency ASK prices in the market*/
void Bot::loopThruExchangeAskPrices() 
{
    for(std::string p : botCurrencies)
    {
        std::vector<OrderBookEntry> entries = orderBook.getOrders(OrderBookType::ask, p, currentTime);
        for(int k=0; k<entries.size(); k++)
        {
            //if price lower than askSMA and trend of prices going down
            if(entries[k].price <= askSMA && negativeSlope(slopeAsk)) 
            { 
                //if the current offer is better than prev one, then remove the prev order 
                //so if there is a streak of prices going down, take the lowest once in the curve for maximum value
                if(entries[k].price < prevAskPrice) 
                    orderBook.removeOrder();        
                
                //enter bot bid based on the entries[k] data, exact match to listing in exchange
                enterBotBid(p, entries[k].price, entries[k].amount); 
            }
            prevAskPrice = entries[k].price;    
        }
    }
}

/**go through diff currency BID prices in the market*/
void Bot::loopThruExchangeBidPrices() 
{
    for(std::string p : botCurrencies)
    {
        std::vector<OrderBookEntry> entries = orderBook.getOrders(OrderBookType::bid, p, currentTime);
        for(int k=0; k<entries.size(); k++)
        {
            //if price greater than bidSMA and trend of prices going up
            if(entries[k].price >= bidSMA && positiveSlope(slopeBid)) 
            {
                //if the current offer is better than prev one, then remove the prev order 
                //so if there is a streak of prices going down, take the highest once in the curve for maximum profits
                if(entries[k].price > prevBidPrice) 
                    orderBook.removeOrder();  

                //enter bot ask based on the entries[k] data, exact match to listing in exchange
                enterBotAsk(p, entries[k].price, entries[k].amount); 
            }
            prevBidPrice = entries[k].price;    
        }
    }
}

/**enter bot's ASK in the exchange*/
void Bot::enterBotAsk(std::string product, double price, double amount)
{
    std::vector<std::string> splitProductName = CSVReader::tokenise(product, '/');
    std::string outgoingCurr = splitProductName[0];
    std::string incomingCurr = splitProductName[1];

    OrderBookEntry obe = CSVReader::stringsToOBE(std::to_string(price),std::to_string(amount), currentTime, product, OrderBookType::ask);
    obe.username = "simuser";

    if (wallet.canFulfillOrder(obe))
    {
        std::cout << "Selling " << amount << " "<< outgoingCurr << " for: " << price*amount << incomingCurr << std::endl;
        orderBook.insertOrder(obe); //wont sort yet (moved to a diff function)
        //data log for bot asks placed
        historicalDataAsksPlaced.push_back(obe);

    }
    else 
        std::cout << "Bot::wallet has insufficient funds to place ASK. " << std::endl;
}

/**enter bot's BID in the exchange*/
void Bot::enterBotBid(std::string product, double price, double amount)
{
        std::vector<std::string> splitProductName = CSVReader::tokenise(product, '/');
        std::string outgoingCurr = splitProductName[0];
        std::string incomingCurr = splitProductName[1];

        OrderBookEntry obe = CSVReader::stringsToOBE(std::to_string(price),std::to_string(amount), currentTime, product, OrderBookType::bid);
        obe.username = "simuser";

        if (wallet.canFulfillOrder(obe))
        {
            std::cout << "Buying " << amount << " "<< outgoingCurr << " for: " << price*amount << incomingCurr << std::endl;
            orderBook.insertOrder(obe); //wont sort yet (moved to a diff function)
            //data log for bot bids placed
            historicalDataBidsPlaced.push_back(obe);
        }
        else 
            std::cout << "Bot::wallet has insufficient funds to place BID. " << std::endl;
}

/**MerkelMain's gotoNextTimeFrame*/
void Bot::nextTimeframe()
{
    //removing the p:products part from Merkel
    //only one currency in it for now
    for (std::string p : botCurrencies) 
    {
        std::cout << "matching " << p << std::endl;
        std::vector<OrderBookEntry> sales =  orderBook.matchAsksToBids(p, currentTime);
        std::cout << "Sales: " << sales.size() << std::endl;
        for (OrderBookEntry& sale : sales)
        {
            std::cout << "Sale price: " << sale.price << " amount " << sale.amount << std::endl; 
            if (sale.username == "simuser")
            {
                if(wallet.processSale(sale)) //bool
                {
                    std::cout<<"Sale successful."<<std::endl;
                    //data log for sales
                    historicalSales.push_back(sale); 
                }
            }
        }
    }
    std::cout<<currentTime<<std::endl;
    walletSummary();
    std::cout<<"Going to next Timeframe::"<<std::endl;
    std::cout<<"====================================================================================\n"<<std::endl;
    currentTime = orderBook.getNextTime(currentTime);
}

//record of the bids and asks it has made (and successful sales) >> print to file
void Bot::log() 
{
    //print out wallet details
    walletSummary();
    //user's choice to print out transaction logs
    std::cout<<"Print out transaction logs?"<<std::endl;
    std::cout<<"Press y to continue, n to exit."<<std::endl;
    std::string input;
    getline(std::cin, input);
    try
    {
        if(input == "Y" || input == "y")
        {
            printHistoricalDataAskFile();
            printHistoricalDataBidFile();
            printHistoricalSalesFile();
        }

        else if (input == "N" || input == "n")
        {
            std::cout<<"Bot terminated."<<std::endl;
        }
    }
    catch(const std::exception& e)
    {
        // std::cerr << e.what() << '\n';
    }
}

/**gets and prints out profit of exchange*/
void Bot::walletSummary()
{
    // std::vector<OrderBookEntry> entries = orderBook.getOrders(OrderBookType::ask, botCurrencies[0], currentTime);
    // double BTC_left_in_wallet = wallet.getCurrencyAmount("BTC");
    // endBalanceUSDT = BTC_left_in_wallet*orderBook.getAvgPrice(entries) + wallet.getCurrencyAmount("USDT");
    // std::cout<<"Wallet is worth "<< endBalanceUSDT << " USDT now."<<std::endl;
    // std::cout<<"Profit of: " << (endBalanceUSDT/initialBalanceUSDT * 100) -100 << "%"<<std::endl;

    //prints out wallet's assets at the end of each timestamp
    std::cout<<"Wallet's balance:"<<std::endl;
    std::cout<<wallet.toString()<<std::endl; 
}


//HELPER FNS BELOW
double Bot::getAvgPriceForAsk(std::string& timestamp, std::string& product) //BTC/USDT_ASK_TIMESTAMP
{
    std::vector<OrderBookEntry> entries = orderBook.getOrders(OrderBookType::ask, product, timestamp);
    if(entries.empty()) return 0;
    else return orderBook.getAvgPrice(entries);
}

double Bot::getAvgPriceForBid(std::string& timestamp, std::string& product) 
{
    std::vector<OrderBookEntry> entries = orderBook.getOrders(OrderBookType::bid, product, timestamp);
    if(entries.empty())
    {
        return 0;
    }
    else return orderBook.getAvgPrice(entries);
}

//to push back minPrice of current timestamp to vector
void Bot::getMinAskAndMaxBidForMarket(std::vector<OrderBookEntry>& asks, std::vector<OrderBookEntry>& bids) //BTC/USDT_ASK_TIMESTAMP
{
    //checker for empty timestamps
    if(!(asks.empty()) && !(bids.empty())) 
    {
        //push back min ask price and max ask bid to instance vector
        //push back time counter to instance vector
        askTimeX.push_back(marketCounter); 
        priceVecAsks.push_back(orderBook.getLowPrice(asks));
        bidTimeX.push_back(marketCounter);  
        priceVecBids.push_back(orderBook.getHighPrice(bids));
        // std::cout<<"last ask min: "<<priceVecAsks.back()<<std::endl;
        // std::cout<<"last bid max: "<<priceVecBids.back()<<std::endl;

        calcSMA(marketCounter);
        calcMovingSlope(marketCounter);
        marketCounter++;
    } 
}

/**get a section of an array to be used as the moving window of prices*/        
std::vector<double> Bot::getSubVector(std::vector<double> &priceVec, int startIndex, int endIndex)
{
   auto first = priceVec.begin() + startIndex;
   auto last = priceVec.begin() + endIndex + 1;
   std::vector<double> vector(first, last);
   return vector;
}

/**check for a positive slope; increment in prices of linear graph*/
bool Bot::positiveSlope(double slope)
{
    double positiveSlope;
    slopeAskVec.push_back(slope);
    //gets the trend of the graph based on windowSize number of values
    if(slopeAskVec.size() >= windowSize)
    {
        for(int i=slopeAskVec.size()-1; i > slopeAskVec.size()-(1+windowSize); i--) //5 slope analysis
        {
            positiveSlope += slopeAskVec[i];
        }
    }
    //returns true if is going up
    return positiveSlope>0;
}

/**check for a negative slope; decrement in prices of linear graph*/
bool Bot::negativeSlope(double slope)
{
    double negativeSlope;
    slopeBidVec.push_back(slope);
    //gets the trend of the graph based on windowSize number of values
    if(slopeBidVec.size() >= windowSize)
    {
        for(int i=slopeBidVec.size()-1; i > slopeBidVec.size()-(1+windowSize); i--) //5 slope analysis
        {
            negativeSlope += slopeBidVec[i];
        }
    }
    //returns true if is going down
    return negativeSlope<0;
}

/**gets average successful ask sale price*/
double Bot::getAvgAskSalePrice()
{
    double sumAskSalePrice, sumAskCounter;
    for(OrderBookEntry&e : historicalSales)
    {
        if(e.orderType == OrderBookType::asksale) 
        {
            sumAskSalePrice += e.price;
            sumAskCounter++;
        }
    }
    return sumAskSalePrice / sumAskCounter;
}

/**gets average successful bid sale price*/
double Bot::getAvgBidSalePrice()
{
    double sumBidSalePrice, sumBidCounter;
    for(OrderBookEntry&e : historicalSales)
    {
        if(e.orderType == OrderBookType::bidsale)
        {
            sumBidSalePrice += e.price;
            sumBidCounter++;
        }
    }
    return sumBidSalePrice / sumBidCounter;
}

/**prints out all asks placed by bot to new file*/
void Bot::printHistoricalDataAskFile()
{
    //writing to file for all asks placed by bot
    ofstream historicalDataAskfile;
    historicalDataAskfile.open("historicalDataAsksPlaced.txt");

    // //print to console
    std::cout<<"Asks placed: "<<std::endl;
    std::cout<<"PRODUCT | ASKS | TIMESTAMP                 |PRICE      |AMOUNT"<< std::endl;

    //write to file
    historicalDataAskfile<<"Asks placed: \n" ;
    historicalDataAskfile<<"PRODUCT | ASKS | TIMESTAMP                 |PRICE      |AMOUNT\n";
    for(OrderBookEntry& e : historicalDataAsksPlaced)
    {
        historicalDataAskfile << e.toString() + "\n";
        std::cout<< e.toString() <<std::endl;
    }
    historicalDataAskfile.close();
}

/**prints out all bids placed by bot to new file*/
void Bot::printHistoricalDataBidFile()
{
    //writing to file for all bids placed by bot
    ofstream historicalDataBidfile;
    historicalDataBidfile.open ("historicalDataBidsPlaced.txt");

    //print to console
    std::cout<<"Bids placed: "<<std::endl;
    std::cout<<"PRODUCT | BIDS | TIMESTAMP                 |PRICE      |AMOUNT"<< std::endl;

    //write to file
    historicalDataBidfile<<"Bids placed: \n";
    historicalDataBidfile<<"PRODUCT | BIDS | TIMESTAMP                 |PRICE      |AMOUNT\n";
    for(OrderBookEntry& e : historicalDataBidsPlaced)
    {
        historicalDataBidfile << e.toString() + "\n";
        std::cout<< e.toString() <<std::endl;
    }
    historicalDataBidfile.close();
}

/**prints out all sales and average successful sale price for asks/bids placed by bot to new file*/
void Bot::printHistoricalSalesFile()
{
    //writing to file for all successful sales
    ofstream historicalSalesFile;
    historicalSalesFile.open ("historicalSales.txt");

    //print to console
    std::cout<<"Succesful sales: "<<std::endl;
    std::cout<<"Average asksale price: "<< getAvgAskSalePrice() << std::endl;
    std::cout<<"Average bidsale price: "<< getAvgBidSalePrice() << std::endl;
    std::cout<<"PRODUCT |ORDERTYPE |TIMESTAMP                 |PRICE      |AMOUNT"<< std::endl;

    //write to file
    historicalSalesFile<<"Succesful sales: \n";
    historicalSalesFile<<"Average asksale price: " + std::to_string(getAvgAskSalePrice()) + "\n";
    historicalSalesFile<<"Average bidsale price: " + std::to_string(getAvgBidSalePrice()) + "\n";
    historicalSalesFile<<"PRODUCT |ORDERTYPE |TIMESTAMP                 |PRICE      |AMOUNT\n";
    
    for(OrderBookEntry& e : historicalSales)
    {
        historicalSalesFile << e.toString() + "\n";
        std::cout<< e.toString() <<std::endl;
    }
    historicalSalesFile.close();
}