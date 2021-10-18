#include "Wallet.h"
#include <iostream>
#include "MerkelMain.h"
#include "Bot.h"
#include "Algo.h"

int main()
{   

    // MerkelMain app{};
    // app.init();

    std::cout<<"Enter 1 to start simulation."<<std::endl;
    std::string line;
    try
    {
        std::getline(std::cin, line);
    }
    catch(const std::exception& e)
    {
        
    }
    if(line == "1")
    {
        std::cout<<"Loading BTC/USDT Trading Bot.."<<std::endl;
        Bot bot{};
        bot.startBot();
    }

    else
    {
    }
    return 0;
}